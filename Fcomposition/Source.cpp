#include <functional>
#include <type_traits>
 
#include <iostream>


template<typename T>
using std_fun = decltype(std::function{ std::declval<T>() });

template<typename T>
constexpr size_t arg_count = 0;

template<typename R, typename... Args>
constexpr size_t arg_count<std::function<R(Args...)>> = sizeof...(Args);

template <size_t begin_arg_count, typename ... Ts>
constexpr auto take_first(Ts... args) {
    return take_first_impl<begin_arg_count, std::tuple<>, Ts...>(std::make_tuple(), args...);
}

template <size_t begin_arg_count, typename Tuple, typename I0, typename ... Tail>
constexpr auto take_first_impl(Tuple tup, I0 i0, Tail... tail) {
    if constexpr (begin_arg_count > 0) {
        return take_first_impl<
            begin_arg_count - 1,
            decltype(std::tuple_cat(tup, std::make_tuple(i0))),
            Tail...>
            (std::tuple_cat(tup, std::make_tuple(i0)), tail...);
    }
    else {
        return tup;
    }
}

template <size_t end_arg_count, typename I0, typename ... Tail>
constexpr auto take_last(I0 i0, Tail... tail) {
    if constexpr (end_arg_count == sizeof...(Tail) + 1) {
        return  std::make_tuple(i0, tail...);
    }
    else {
        return take_last<end_arg_count, Tail...>(tail...);
    }
}

template<typename T, typename U>
constexpr auto operator &(T f, U g) requires requires {std::function{ f }; std::function{ g }; } {
    return [=] <typename... Ts>(Ts... args) {
        constexpr size_t begin_arg_count = arg_count<std_fun<T>>;
        constexpr size_t end_arg_count = arg_count<std_fun<U>>;
        
        auto begin_args = take_first<begin_arg_count, Ts...>(args...);
        auto end_args = take_last<end_arg_count, Ts...>(args...);

        return std::apply(f, begin_args) && std::apply(g, end_args);

    };
}
 
template<typename T, typename U>
constexpr auto operator &&(T f, U g) requires requires {std::function{ f }; std::function{ g }; } {
    return[=] <typename... Ts>(Ts... args) requires (sizeof...(Ts) == std::max(arg_count<std_fun<T>>, arg_count<std_fun<U>>)){
        constexpr size_t f_arg_count = arg_count<std_fun<T>>;
        constexpr size_t g_arg_count = arg_count<std_fun<U>>;

        if constexpr (f_arg_count == g_arg_count) {
            return f(args...) && g(args...);
        }
        else if constexpr (f_arg_count > g_arg_count) {
            return f(args...) && std::apply(g, take_first<g_arg_count, Ts...>(args...));

        }
        else {
            return std::apply(f, take_first<f_arg_count, Ts...>(args...)) && g(args...);
        }
    };
}


template<typename T, typename U>
constexpr auto operator |(T f, U g) requires requires {std::function{ f }; std::function{ g }; } {
    return[=] <typename... Ts>(Ts... args) {
        constexpr size_t begin_arg_count = arg_count<std_fun<T>>;
        constexpr size_t end_arg_count = arg_count<std_fun<U>>;

        auto begin_args = take_first<begin_arg_count, Ts...>(args...);
        auto end_args = take_last<end_arg_count, Ts...>(args...);

        return std::apply(f, begin_args) || std::apply(g, end_args);

    };
}

template<typename T, typename U>
constexpr auto operator ||(T f, U g) requires requires {std::function{ f }; std::function{ g }; } {
    return[=] <typename... Ts>(Ts... args) requires (sizeof...(Ts) == std::max(arg_count<std_fun<T>>, arg_count<std_fun<U>>)) {
        constexpr size_t f_arg_count = arg_count<std_fun<T>>;
        constexpr size_t g_arg_count = arg_count<std_fun<U>>;

        if constexpr (f_arg_count == g_arg_count) {
            return f(args...) || g(args...);
        }
        else if constexpr (f_arg_count > g_arg_count) {
            return f(args...) || std::apply(g, take_first<g_arg_count, Ts...>(args...));

        }
        else {
            return std::apply(f, take_first<f_arg_count, Ts...>(args...)) || g(args...);
        }
    };
}

int main() { 

    auto f = [](bool i, bool i1, bool i2) {
        return i && i1 && i2;
    };


    auto g = [](bool i, bool i1) {
        return i && i1;
    };

    auto h = f && g;
   



    std::cout << h(1, 1, 1);

    


}
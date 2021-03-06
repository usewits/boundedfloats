# Bounded Floats
A simple header only library to numerically assess worst case floating point rounding errors. This is done by keeping track of the lowest and highest attainable value of a floating point at all times. Whenever a floating point operation is performed, the new minimum and maximum values are computed, taking rounding errors into account. The true (infinite precision) value always lies in between min and max.

Easy to use! Just replace `double` with `bounded_double`. In some cases your code has to be adjusted a little. For example if you use operators with a double on the left hand side, or apply a fancy function on a bounded double.

Disclaimers: This code is licenced under three clause BSD. This code was not battle tested. Efficiency could be greatly improved (expensive function calls everywhere!).
Note that the `apply` function can not be used in arbitrary situations.



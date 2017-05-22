#include <limits>
#include <cmath>
#include <ostream>
#include <istream>
#include <functional>
#include <assert.h>


class bounded_double {
    double typical;//typical behaves exactly as a regular floating point would
    double min;//min is smaller than the true value at all times
    double max;//max is bigger than the true value at all times

    static double down(double x);//take pre/postdecessor
    static double up(double x);

  public:
    bounded_double(const bounded_double& rhs) {
        *this = rhs;
    }

    bounded_double(double value) {
        *this = value;
    }

    bounded_double(double min, double typical, double max) {
        this->min = min;
        this->typical = typical;
        this->max = max;
    }


    bounded_double() {
        *this = std::numeric_limits<double>::quiet_NaN();
    }

    bounded_double apply(double (*f)(double) ) const;
    bounded_double apply(double (*f)(double, double), const bounded_double& rhs) const;

    static bounded_double apply(double (*f)(double), const bounded_double& lhs) {
        return lhs.apply(f);
    }
    static bounded_double apply(double (*f)(double, double), const bounded_double& lhs, const bounded_double& rhs) {
        return lhs.apply(f, rhs);
    }

    double get_min() {
        return min;
    }
    double get_max() {
        return max;
    }
    double get_typical() {
        return typical;
    }

    bounded_double operator=(const bounded_double& rhs) {
        min = rhs.min;
        max = rhs.max;
        typical = rhs.typical;
    }

    bounded_double operator+(const bounded_double& rhs) {
        return apply([] (const double a, const double b) -> double {return a+b;}, rhs);
    }
    bounded_double operator-(const bounded_double& rhs) {
        return apply([] (const double a, const double b) -> double {return a-b;}, rhs);
    }
    bounded_double operator*(const bounded_double& rhs) {
        return apply([] (const double a, const double b) -> double {return a*b;}, rhs);
    }
    bounded_double operator/(const bounded_double& rhs) {
		if(rhs.min <= 0 && rhs.max >= 0) {//Extremes may not be on boundary if division by 0 is possible
    		bounded_double result;
			result.min = -std::numeric_limits<double>::infinity();
			result.max = std::numeric_limits<double>::infinity();
			result.typical = typical/rhs.typical;
			return result;
		} else {
        	return apply([] (const double a, const double b) -> double {return a/b;}, rhs);
    }

    bounded_double operator+=(const bounded_double& rhs) {
        *this = apply([] (const double a, const double b) -> double {return a+b;}, rhs);
    }
    bounded_double operator-=(const bounded_double& rhs) {
        *this = apply([] (const double a, const double b) -> double {return a-b;}, rhs);
    }
    bounded_double operator*=(const bounded_double& rhs) {
        *this = apply([] (const double a, const double b) -> double {return a*b;}, rhs);
    }
    bounded_double operator/=(const bounded_double& rhs) {
		if(rhs.min <= 0 && rhs.max >= 0) {//Extremes may not be on boundary if division by 0 is possible
			min = -std::numeric_limits<double>::infinity();
			max = std::numeric_limits<double>::infinity();
			typical /= rhs.typical;
		} else {
        	*this = apply([] (const double a, const double b) -> double {return a/b;}, rhs);
		}
    }


    bounded_double operator=(const double value) {
        min = down(value);
        max = up(value);
        typical = value;
    }

    bounded_double operator+(const double value) {
        return apply([] (const double a, const double b) -> double {return a+b;}, bounded_double(value));
    }
    bounded_double operator-(const double value) {
        return apply([] (const double a, const double b) -> double {return a-b;}, bounded_double(value));
    }
    bounded_double operator*(const double value) {
        return apply([] (const double a, const double b) -> double {return a*b;}, bounded_double(value));
    }
    bounded_double operator/(const double value) {
        return apply([] (const double a, const double b) -> double {return a/b;}, bounded_double(value));
    }

    bounded_double operator+=(const double value) {
        *this = apply([] (const double a, const double b) -> double {return a+b;}, bounded_double(value));
    }
    bounded_double operator-=(const double value) {
        *this = apply([] (const double a, const double b) -> double {return a-b;}, bounded_double(value));
    }
    bounded_double operator*=(const double value) {
        *this = apply([] (const double a, const double b) -> double {return a*b;}, bounded_double(value));
    }
    bounded_double operator/=(const double value) {
        *this = apply([] (const double a, const double b) -> double {return a/b;}, bounded_double(value));
    }


    bounded_double operator-() {
        return (*this)*(-1);
    }


    bool operator>(const bounded_double& rhs) {
        return typical > rhs.typical;
    }
    bool operator<(const bounded_double& rhs) {
        return typical < rhs.typical;
    }
    bool operator<=(const bounded_double& rhs) {
        return typical <= rhs.typical;
    }
    bool operator>=(const bounded_double& rhs) {
        return typical >= rhs.typical;
    }

    bool operator>(double value) {
        return typical > value;
    }
    bool operator<(double value) {
        return typical < value;
    }
    bool operator<=(double value) {
        return typical <= value;
    }
    bool operator>=(double value) {
        return typical >= value;
    }

    friend std::ostream& operator<< (std::ostream& stream, const bounded_double& rhs) {
        int original_precision = stream.precision();
        double error_size = std::max((rhs.typical-rhs.min), (rhs.max-rhs.typical));
        int significant_digits = ceil(log(rhs.typical)-log(error_size));

			//Only show significant digits, unless fewer digits are requested
        stream.precision(std::min(significant_digits, original_precision));
        stream << rhs.typical;
        stream.precision(1);
        stream << "(±" << error_size <<")";
        stream.precision(original_precision);
        return stream;
    }

    friend std::istream& operator>> (std::istream& stream, bounded_double& rhs) {
        double value;
        stream >> value;
        rhs = value;
        return stream;
    }
};

double bounded_double::down(double x) {
    //The existance of infinity and -inifinity is platform dependent..
    return std::nextafter(x, -std::numeric_limits<double>::infinity());
}

double bounded_double::up(double x) {
    return std::nextafter(x, std::numeric_limits<double>::infinity());
}


//asserts f is monotomic [increasing | decreasing] between min and max
bounded_double bounded_double::apply(double (*f)(const double) ) const {
    bounded_double result;

    double left_res = f(min);
    double right_res = f(max);
    
    result.min = down(std::min(left_res,right_res));
    result.max = up(std::max(left_res,right_res));
   
    result.typical = f(typical);
    return result;
}

//asserts f attains its maximum and minimum on boundary points
bounded_double bounded_double::apply(double (*f)(const double, const double), const bounded_double& rhs) const {
    bounded_double result;
    result.min = std::numeric_limits<double>::infinity();
    result.max = -std::numeric_limits<double>::infinity();

    double lval, rval;
    for(int type = 0; type < (1<<2); type++) {//Plug in the four extreme points
        if(type & (1 << 0)) lval = min;
        else lval = max;
        if(type & (1 << 1)) rval = rhs.min;
        else rval = rhs.max;

        double fres = f(lval, rval);
        if(down(fres) < result.min)
            result.min = down(fres);
        if(  up(fres) > result.max)
            result.max = up(fres);
    }
    result.typical = f(typical, rhs.typical);
    return result;
}


/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <math.h>

#define PREC_float 1
#define PREC_double 2
#define PREC_long_double 3

template<class T>
bool is_equal(T x, T y, sal_Int16 _nPrec)
{
    // due to the fact that this check looks only if both values are equal
    // we only need to look on one value

    // 14 digits will announce the checkPrecisionSize

    sal_Int32 nPRECISION;
    switch(_nPrec)
    {
    case PREC_float:
        nPRECISION = 6;
        break;
    case PREC_double:
        nPRECISION = 14;
        break;
    case PREC_long_double:
        nPRECISION = 20;
        break;
    default:
        nPRECISION = 2;
    }

    if (x < 0)
    {
        x = -x;
    }
    if (y < 0)
    {
        y = -y;
    }

    // LLA: due to a bug in printf with '%f' and long double within linux environment
    //      we have to use %lf instead.

    if (_nPrec != PREC_long_double)
    {
        printf("double equal: %.20f\n", x);
        printf("              %.20f\n", y);
    }
    //here nPrecOfN is the number after dot
    sal_Int32 nBeforeDot = sal_Int32( log10(x) );
    if ( nBeforeDot < 0)
    {
         nBeforeDot = 0;
    }
    //printf("nPRECISION is  %d\n", nPRECISION);
    sal_Int32 nPrecOfN = -nPRECISION + nBeforeDot;

    if (_nPrec != PREC_long_double)
        printf("nPrecOfN is  %d\n", nPrecOfN);

    long double nPrec = pow(0.1, -nPrecOfN);

    if (_nPrec != PREC_long_double)
        printf("        prec: %.20f\n", nPrec);

    long double nDelta = fabs( x - y ) ;

    if (_nPrec != PREC_long_double)
    {
        printf("       delta: %.20f\n", nDelta);
        printf("       nPrec: %.20f\n", nPrec);
        printf("delta must be less or equal to prec!\n\n");
    }

    if (nDelta > nPrec)
    {
        // values are not equal
        return false;
    }

    // values are equal
    return true;
}

bool is_float_equal(float x, float y)
{
    return is_equal<float>(x, y, PREC_float);
}
bool is_double_equal(double x, double y)
{
    return is_equal<double>(x, y, PREC_double);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "samplelib.hxx"
#include <sal/types.h>
/*


*/

extern "C"
SampleLib_Api* SAL_CALL initSampleLibApi(void)
{
    static SampleLib_Api aApi= {0,0};
    if (!aApi.funcA)
    {
        aApi.funcA= &funcA;
        aApi.funcB= &funcB;
        return (&aApi);
    }
    else
    {
        return (&aApi);
    }

}


sal_Int32 SAL_CALL funcA( sal_Int32 a)
{
    return a;
}


double SAL_CALL funcB( double a)
{
    return a;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#ifndef __SAMPLELIB_HXX_
#define __SAMPLELIB_HXX_

#include <sal/types.h>

struct SampleLib_Api
{
    sal_Int32 (SAL_CALL *funcA)( sal_Int32 );
    double (SAL_CALL *funcB)( double );
};


typedef SampleLib_Api* (SAL_CALL *InitSampleLib_Api)(void);

#define SAMPLELIB_INIT_FUNCTION_NAME "initSampleLibApi"


sal_Int32 SAL_CALL funcA( sal_Int32 a);
double SAL_CALL funcB( double a);


#endif

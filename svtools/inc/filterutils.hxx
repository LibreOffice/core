/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef SVTOOLS_INC_FILTERUTILS_HXX
#define SVTOOLS_INC_FILTERUTILS_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/uno/RuntimeException.hpp>
namespace svt
{
    class BinFilterUtils
    {
    private:
        BinFilterUtils();    // never implemented

    public:
        SVT_DLLPUBLIC static  rtl::OUString CreateOUStringFromUniStringArray( const char* pcCharArr, sal_uInt32 nBufSize );
        SVT_DLLPUBLIC static  rtl::OUString CreateOUStringFromStringArray( const char* pcCharArr, sal_uInt32 nBufSize );
    };

//........................................................................
} // namespace svt
//........................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

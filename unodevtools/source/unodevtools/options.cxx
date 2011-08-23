/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <stdio.h>

#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/process.h"

#include "com/sun/star/uno/RuntimeException.hpp"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace unodevtools {

//-------------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
static void out( const sal_Char * pText )
{
    fprintf( stderr, pText );
}
#endif

//-------------------------------------------------------------------------------
sal_Bool readOption( OUString * pValue, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const OUString & aArg)
    throw (RuntimeException)
{
    const OUString dash = OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
    if(aArg.indexOf(dash) != 0)
        return sal_False;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return sal_False;

    if (aOpt.equalsIgnoreAsciiCase( aArg.copy(1) )) {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= (sal_Int32)rtl_getAppCommandArgCount() ||
            pValue->copy(1).equals(dash))
        {
            OUStringBuffer buf( 32 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("incomplete option \"-") );
            buf.appendAscii( pOpt );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" given!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
        } else {
#if OSL_DEBUG_LEVEL > 1
            out( "\n> identified option -" );
            out( pOpt );
            out( " = " );
            OString tmp = OUStringToOString(*pValue, RTL_TEXTENCODING_ASCII_US);
              out( tmp.getStr() );
#endif
            ++(*pnIndex);
            return sal_True;
        }
    } else if (aArg.indexOf(aOpt) == 1) {
        *pValue = aArg.copy(1 + aOpt.getLength());
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option -" );
        out( pOpt );
        out( " = " );
        OString tmp = OUStringToOString(*pValue, RTL_TEXTENCODING_ASCII_US);
        out( tmp.getStr() );
#endif
        ++(*pnIndex);

        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------------
sal_Bool readOption( sal_Bool * pbOpt, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const OUString & aArg)
{
    const OUString dashdash(RTL_CONSTASCII_USTRINGPARAM("--"));
    const OUString dash(RTL_CONSTASCII_USTRINGPARAM("-"));
    OUString aOpt = OUString::createFromAscii(pOpt);

    if((aArg.indexOf(dash) == 0 && aOpt.equalsIgnoreAsciiCase(aArg.copy(1))) ||
       (aArg.indexOf(dashdash) == 0 && aOpt.equalsIgnoreAsciiCase(aArg.copy(2))) )
    {
        ++(*pnIndex);
        *pbOpt = sal_True;
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option --" );
        out( pOpt );
#endif
        return sal_True;
    }
    return sal_False;
}

} // end of namespace unodevtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

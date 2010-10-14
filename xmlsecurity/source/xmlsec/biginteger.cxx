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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <xmlsecurity/biginteger.hxx>

#include <sal/types.h>
//For reasons that escape me, this is what xmlsec does when size_t is not 4
#if SAL_TYPES_SIZEOFPOINTER != 4
#    define XMLSEC_NO_SIZE_T
#endif
#include <xmlsec/xmlsec.h>
#include <xmlsec/bn.h>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star::uno ;
using ::rtl::OUString ;

Sequence< sal_Int8 > numericStringToBigInteger ( OUString numeral )
{
    if( numeral.getStr() != NULL )
    {
        xmlChar* chNumeral ;
        const xmlSecByte* bnInteger ;
        xmlSecSize length ;
        xmlSecBn bn ;

        rtl::OString onumeral = rtl::OUStringToOString( numeral , RTL_TEXTENCODING_ASCII_US ) ;

        chNumeral = xmlStrndup( ( const xmlChar* )onumeral.getStr(), ( int )onumeral.getLength() ) ;

        if( xmlSecBnInitialize( &bn, 0 ) < 0 ) {
            xmlFree( chNumeral ) ;
            return Sequence< sal_Int8 >();
        }

        if( xmlSecBnFromDecString( &bn, chNumeral ) < 0 ) {
            xmlFree( chNumeral ) ;
            xmlSecBnFinalize( &bn ) ;
            return Sequence< sal_Int8 >();
        }

        xmlFree( chNumeral ) ;

        length = xmlSecBnGetSize( &bn ) ;
        if( length <= 0 ) {
            xmlSecBnFinalize( &bn ) ;
            return Sequence< sal_Int8 >();
        }

        bnInteger = xmlSecBnGetData( &bn ) ;
        if( bnInteger == NULL ) {
            xmlSecBnFinalize( &bn ) ;
            return Sequence< sal_Int8 >();
        }

        Sequence< sal_Int8 > integer( length ) ;
        for( unsigned int i = 0 ; i < length ; i ++ )
        {
            integer[i] = *( bnInteger + i ) ;
        }

        xmlSecBnFinalize( &bn ) ;
        return integer ;
    }

    return Sequence< sal_Int8 >();
}

OUString bigIntegerToNumericString ( Sequence< sal_Int8 > integer )
{
    OUString aRet ;

    if( integer.getLength() ) {
        xmlSecBn bn ;
        xmlChar* chNumeral ;

        if( xmlSecBnInitialize( &bn, 0 ) < 0 )
            return aRet ;

        if( xmlSecBnSetData( &bn, ( const unsigned char* )&integer[0], integer.getLength() ) < 0 ) {
            xmlSecBnFinalize( &bn ) ;
            return aRet ;
        }

        chNumeral = xmlSecBnToDecString( &bn ) ;
        if( chNumeral == NULL ) {
            xmlSecBnFinalize( &bn ) ;
            return aRet ;
        }

        aRet = OUString::createFromAscii( ( const char* )chNumeral ) ;

        xmlSecBnFinalize( &bn ) ;
        xmlFree( chNumeral ) ;
    }

    return aRet ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

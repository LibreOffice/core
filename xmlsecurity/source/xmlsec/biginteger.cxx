/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <xmlsecurity/biginteger.hxx>

#include "xmlsecurity/xmlsec-wrapper.h"
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star::uno ;

Sequence< sal_Int8 > numericStringToBigInteger ( const OUString& numeral )
{
    if( numeral.getStr() != nullptr )
    {
        xmlChar* chNumeral ;
        const xmlSecByte* bnInteger ;
        xmlSecSize length ;
        xmlSecBn bn ;

        OString onumeral = OUStringToOString( numeral , RTL_TEXTENCODING_ASCII_US ) ;

        chNumeral = xmlStrndup( reinterpret_cast<const xmlChar*>(onumeral.getStr()), ( int )onumeral.getLength() ) ;

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
        if( bnInteger == nullptr ) {
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

OUString bigIntegerToNumericString ( const Sequence< sal_Int8 >& integer )
{
    OUString aRet ;

    if( integer.getLength() ) {
        xmlSecBn bn ;
        xmlChar* chNumeral ;

        if( xmlSecBnInitialize( &bn, 0 ) < 0 )
            return aRet ;

        if( xmlSecBnSetData( &bn, reinterpret_cast<const unsigned char*>(integer.getConstArray()), integer.getLength() ) < 0 ) {
            xmlSecBnFinalize( &bn ) ;
            return aRet ;
        }

        chNumeral = xmlSecBnToDecString( &bn ) ;
        if( chNumeral == nullptr ) {
            xmlSecBnFinalize( &bn ) ;
            return aRet ;
        }

        aRet = OUString::createFromAscii( reinterpret_cast<char*>(chNumeral) ) ;

        xmlSecBnFinalize( &bn ) ;
        xmlFree( chNumeral ) ;
    }

    return aRet ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

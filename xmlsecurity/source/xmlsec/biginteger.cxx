/*************************************************************************
 *
 *  $RCSfile: biginteger.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-26 06:15:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <xmlsec/xmlsec.h>
#include <xmlsec/bn.h>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star::uno ;
using ::rtl::OUString ;

Sequence< sal_Int8 > numericStringToBigInteger (
    OUString numeral
) {
    if( numeral.getStr() != NULL ) {
        xmlChar* chNumeral ;
        const xmlSecByte* bnInteger ;
        xmlSecSize length ;
        xmlSecBn bn ;

        rtl::OString onumeral = rtl::OUStringToOString( numeral , RTL_TEXTENCODING_ASCII_US ) ;

        chNumeral = xmlStrndup( ( const xmlChar* )onumeral.getStr(), ( int )onumeral.getLength() ) ;

        if( xmlSecBnInitialize( &bn, 0 ) < 0 ) {
            xmlFree( chNumeral ) ;
            return NULL ;
        }

        if( xmlSecBnFromDecString( &bn, chNumeral ) < 0 ) {
            xmlFree( chNumeral ) ;
            xmlSecBnFinalize( &bn ) ;
            return NULL ;
        }

        xmlFree( chNumeral ) ;

        length = xmlSecBnGetSize( &bn ) ;
        if( length <= 0 ) {
            xmlSecBnFinalize( &bn ) ;
            return NULL ;
        }

        bnInteger = xmlSecBnGetData( &bn ) ;
        if( bnInteger == NULL ) {
            xmlSecBnFinalize( &bn ) ;
            return NULL ;
        }

        Sequence< sal_Int8 > integer( length ) ;
        for( unsigned int i = 0 ; i < length ; i ++ )
            integer[i] = *( bnInteger + i ) ;
        //  integer[i] = *( bb + sizeof( bb ) - len + i ) ;

        xmlSecBnFinalize( &bn ) ;
        return integer ;
    }

    return NULL ;
}

OUString bigIntegerToNumericString (
    Sequence< sal_Int8 > integer
) {
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


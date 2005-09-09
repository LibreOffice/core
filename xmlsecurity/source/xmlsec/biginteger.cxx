/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biginteger.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:24:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <xmlsecurity/biginteger.hxx>

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
        {
            integer[i] = *( bnInteger + i ) ;
        }

        xmlSecBnFinalize( &bn ) ;
        return integer ;
    }

    return NULL ;
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


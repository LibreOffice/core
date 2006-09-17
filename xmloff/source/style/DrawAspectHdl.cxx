/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawAspectHdl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:42:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmltoken.hxx>
#endif

#ifndef _XMLOFF_DRAWASPECTHDL_HXX
#include "DrawAspectHdl.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePropHdl
//

SvXMLEnumMapEntry __READONLY_DATA pXML_DrawAspect_Enum[] =
{
    { XML_CONTENT,          1   },
    { XML_THUMBNAIL,        2   },
    { XML_ICON,             4   },
    { XML_PRINT,            8   },
    { XML_TOKEN_INVALID, 0 }
};

DrawAspectHdl::~DrawAspectHdl()
{
    // nothing to do
}

sal_Bool DrawAspectHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nAspect = 0;

    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    OUString aToken;
    while( aTokenEnum.getNextToken( aToken ) )
    {
        sal_uInt16 nVal;
        if( rUnitConverter.convertEnum( nVal, aToken, pXML_DrawAspect_Enum ) )
        {
            nAspect = nAspect | (sal_Int32)nVal;
        }
    }

    rValue <<= nAspect;

    return nAspect != 0;
}

sal_Bool DrawAspectHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;

    sal_Int32 nAspect;
    if( rValue >>= nAspect )
    {
        if( (nAspect & 1) != 0 )
            aOut.append( GetXMLToken(XML_CONTENT) );

        if( (nAspect & 2) != 0 )
        {
            if( aOut.getLength() )
                aOut.append( sal_Unicode(' ') );
            aOut.append( GetXMLToken(XML_THUMBNAIL) );
        }

        if( (nAspect & 4) != 0 )
        {
            if( aOut.getLength() )
                aOut.append( sal_Unicode(' ') );
            aOut.append( GetXMLToken(XML_ICON) );
        }

        if( (nAspect & 8) != 0 )
        {
            if( aOut.getLength() )
                aOut.append( sal_Unicode(' ') );
            aOut.append( GetXMLToken(XML_PRINT) );
        }

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

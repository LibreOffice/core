/*************************************************************************
 *
 *  $RCSfile: DrawAspectHdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-29 21:07:17 $
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

sal_Bool DrawAspectHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
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

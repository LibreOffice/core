/*************************************************************************
 *
 *  $RCSfile: HatchStyle.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:19:05 $
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

#ifndef _XMLOFF_HATCHSTYLE_HXX
#include "HatchStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_HATCH_NAME,
    XML_TOK_HATCH_DISPLAY_NAME,
    XML_TOK_HATCH_STYLE,
    XML_TOK_HATCH_COLOR,
    XML_TOK_HATCH_DISTANCE,
    XML_TOK_HATCH_ROTATION,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aHatchAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_HATCH_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_HATCH_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_HATCH_STYLE },
    { XML_NAMESPACE_DRAW, XML_COLOR, XML_TOK_HATCH_COLOR },
    { XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, XML_TOK_HATCH_DISTANCE },
    { XML_NAMESPACE_DRAW, XML_ROTATION, XML_TOK_HATCH_ROTATION },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HatchStyle_Enum[] =
{
    { XML_HATCHSTYLE_SINGLE,    drawing::HatchStyle_SINGLE },
    { XML_HATCHSTYLE_DOUBLE,    drawing::HatchStyle_DOUBLE },
    { XML_HATCHSTYLE_TRIPLE,    drawing::HatchStyle_TRIPLE },
    { XML_TOKEN_INVALID, 0 }
};


//-------------------------------------------------------------
// Import
//-------------------------------------------------------------

XMLHatchStyleImport::XMLHatchStyleImport( SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLHatchStyleImport::~XMLHatchStyleImport()
{
}

sal_Bool XMLHatchStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    sal_Bool bRet = sal_False;

    sal_Bool bHasName  = sal_False;
    sal_Bool bHasStyle = sal_False;
    sal_Bool bHasColor = sal_False;
    sal_Bool bHasDist  = sal_False;
    OUString aDisplayName;

    drawing::Hatch aHatch;
    aHatch.Style = drawing::HatchStyle_SINGLE;
    aHatch.Color = 0;
    aHatch.Distance = 0;
    aHatch.Angle = 0;

    SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );
    SvXMLNamespaceMap rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
            case XML_TOK_HATCH_NAME:
                {
                    rStrName = rStrValue;
                    bHasName = sal_True;
                }
                break;
            case XML_TOK_HATCH_DISPLAY_NAME:
                aDisplayName = rStrValue;
                break;
            case XML_TOK_HATCH_STYLE:
                {
                    sal_uInt16 eValue;
                    if( bHasStyle = rUnitConverter.convertEnum( eValue, rStrValue, pXML_HatchStyle_Enum ) )
                        aHatch.Style = (drawing::HatchStyle) eValue;
                }
                break;
            case XML_TOK_HATCH_COLOR:
                {
                    Color aColor;
                    if( bHasColor = rUnitConverter.convertColor( aColor, rStrValue ) )
                        aHatch.Color = (sal_Int32)( aColor.GetColor() );
                }
                break;
            case XML_TOK_HATCH_DISTANCE:
                bHasDist = rUnitConverter.convertMeasure( (sal_Int32&)aHatch.Distance, rStrValue );
                break;
            case XML_TOK_HATCH_ROTATION:
                {
                    sal_Int32 nValue;
                    rUnitConverter.convertNumber( nValue, rStrValue, 0, 360 );
                    aHatch.Angle = sal_Int16( nValue );
                }
                break;

            default:
                DBG_WARNING( "Unknown token at import hatch style" )
                ;
        }
    }

    rValue <<= aHatch;

    if( aDisplayName.getLength() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_HATCH_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasStyle && bHasColor && bHasDist;

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

#ifndef SVX_LIGHT

XMLHatchStyleExport::XMLHatchStyleExport( SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLHatchStyleExport::~XMLHatchStyleExport()
{
}

sal_Bool XMLHatchStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    drawing::Hatch aHatch;

    if( rStrName.getLength() )
    {
        if( rValue >>= aHatch )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            SvXMLUnitConverter& rUnitConverter =
                rExport.GetMM100UnitConverter();

            // Style
            if( !rUnitConverter.convertEnum( aOut, aHatch.Style, pXML_HatchStyle_Enum ) )
            {
                bRet = sal_False;
            }
            else
            {
                // Name
                sal_Bool bEncoded = sal_False;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                      rExport.EncodeStyleName( rStrName,
                                                                &bEncoded ) );
                if( bEncoded )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                            rStrName );

                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

                // Color
                rUnitConverter.convertColor( aOut, Color( aHatch.Color ) );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, aStrValue );

                // Distance
                rUnitConverter.convertMeasure( aOut, aHatch.Distance );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, aStrValue );

                // Angle
                rUnitConverter.convertNumber( aOut, sal_Int32( aHatch.Angle ) );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ROTATION, aStrValue );

                // Do Write
                SvXMLElementExport rElem( rExport, XML_NAMESPACE_DRAW, XML_HATCH,
                                          sal_True, sal_False );
            }
        }
    }

    return bRet;
}

#endif // #ifndef SVX_LIGHT

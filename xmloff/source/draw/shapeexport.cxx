/*************************************************************************
 *
 *  $RCSfile: shapeexport.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-26 19:45:14 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _SDXMLEXP_HXX
#include "sdxmlexp.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

XMLShapeExport::XMLShapeExport(SvXMLExport& rExp,
                                SvXMLExportPropertyMapper *pExtMapper )
:   rExport( rExp ),
    mnNextUniqueShapeId(1),
    msZIndex( RTL_CONSTASCII_USTRINGPARAM("ZOrder") ),
    msEmptyPres( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ),
    msModel( RTL_CONSTASCII_USTRINGPARAM("Model") ),
    msStartShape( RTL_CONSTASCII_USTRINGPARAM("StartShape") ),
    msEndShape( RTL_CONSTASCII_USTRINGPARAM("EndShape") )
{
    // construct PropertyHandlerFactory
    xSdPropHdlFactory = new XMLSdPropHdlFactory;

    // construct PropertySetMapper
    UniReference < XMLPropertySetMapper > xMapper =
        new XMLPropertySetMapper(
            (XMLPropertyMapEntry*)aXMLSDProperties, xSdPropHdlFactory);
    xPropertySetMapper = new SvXMLExportPropertyMapper( xMapper );
    if( pExtMapper )
    {
        UniReference < SvXMLExportPropertyMapper > xExtMapper( pExtMapper );
        xPropertySetMapper->ChainExportMapper( xExtMapper );
    }

    rExp.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)),
        GetPropertySetMapper(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX)));
    rExp.GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_PRESENTATION_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME)),
        GetPropertySetMapper(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX)));
}

XMLShapeExport::~XMLShapeExport()
{
}

// This method collects all automatic styles for the given XShape
void XMLShapeExport::collectShapeAutoStyles(const uno::Reference< drawing::XShape >& xShape)
{
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // filter propset
        OUString aParentName; // parent maybe the pool in later versions
        OUString aNewName;
        std::vector< XMLPropertyState > xPropStates = GetPropertySetMapper()->Filter( xPropSet );

        if(xPropStates.size())
        {
            // there are filtered properties -> hard attributes
            // try to find this style in AutoStylePool
            aNewName = GetExport().GetAutoStylePool()->Find(
                XML_STYLE_FAMILY_SD_GRAPHICS_ID, aParentName, xPropStates);

            if(!aNewName.getLength())
            {
                // Style did not exist, add it to AutoStalePool
                GetExport().GetAutoStylePool()->Add(
                    XML_STYLE_FAMILY_SD_GRAPHICS_ID, aParentName, xPropStates);
            }
        }
    }

    // prep text styles
    uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY);
    if(xText.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bIsEmptyPresObj = sal_False;

        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(msEmptyPres) )
        {
            uno::Any aAny = xPropSet->getPropertyValue(msEmptyPres);
            aAny >>= bIsEmptyPresObj;
        }

        if(!bIsEmptyPresObj)
        {
            GetExport().GetTextParagraphExport()->collectTextAutoStyles( xText );
        }
    }

    // check for calc ole
    const OUString aShapeType( xShape->getShapeType() );
    if( aShapeType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.OLE2Shape" )) ||
        aShapeType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.CalcShape" )) )
    {
        uno::Reference< chart::XChartDocument > xChartDoc;
        uno::Any aAny( xPropSet->getPropertyValue(msModel) );
        aAny >>= xChartDoc;
        if( xChartDoc.is() )
        {
            GetExport().GetChartExport()->collectAutoStyles( xChartDoc );
        }
    }

    // check for connector
    if( aShapeType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.ConnectorShape" ) ) )
    {
        uno::Reference< drawing::XShape > xConnection;

        // create shape ids for export later
        if( xPropSet->getPropertyValue( msStartShape ) >>= xConnection )
        {
            createShapeId( xConnection );
        }
        if( xPropSet->getPropertyValue( msEndShape ) >>= xConnection )
        {
            createShapeId( xConnection );
        }
    }
}

// This method exports the given XShape
void XMLShapeExport::exportShape(const uno::Reference< drawing::XShape >& xShape,
                                 sal_Int32 nFeatures /* = SEF_DEFAULT */,
                                 com::sun::star::awt::Point* pRefPoint /* = NULL */ )
{
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aParentName; // parent maybe the pool in later versions
        OUString aNewName;
        std::vector< XMLPropertyState > xPropStates( GetPropertySetMapper()->Filter( xPropSet ) );

        if(xPropStates.size())
            aNewName = GetExport().GetAutoStylePool()->Find(
                XML_STYLE_FAMILY_SD_GRAPHICS_ID, aParentName, xPropStates);

        // compute the shape type
        XmlShapeType eShapeType(XmlShapeTypeNotYetSet);
        SdXMLExport::ImpCalcShapeType(xShape, eShapeType);

        SdXMLExport::ImpWriteSingleShapeStyleInfo(GetExport(), xShape,
            XML_STYLE_FAMILY_SD_GRAPHICS_ID, aNewName, eShapeType, nFeatures, pRefPoint);
    }
}

void XMLShapeExport::exportAutoStyles()
{
    // export all autostyle infos

    // ...for graphic
//  if(IsFamilyGraphicUsed())
    {
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_GRAPHICS_ID,
            GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap());
    }

    // ...for presentation
//  if(IsFamilyPresentationUsed())
    {
        GetExport().GetAutoStylePool()->exportXML(
            XML_STYLE_FAMILY_SD_PRESENTATION_ID,
            GetExport().GetDocHandler(),
            GetExport().GetMM100UnitConverter(),
            GetExport().GetNamespaceMap());
    }
}

/** creates a unique id for this shape, this id is saved and exported with this shape later
    with the exportShape method. Its ok to call this twice with the same shape */
void XMLShapeExport::createShapeId( const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName( msZIndex ) )
    {
        sal_Int32 nIndex = 0;
        xPropSet->getPropertyValue( msZIndex ) >>= nIndex;

        const std::vector<XMLShapeIdHint>::size_type nCount = maUsedShapeIds.size();
        for( std::vector<XMLShapeIdHint>::size_type i = 0; i < nCount; i++ )
        {
            if( maUsedShapeIds[i].mnShapeIndex == nIndex )
                return; // we already have an id for this shape;
        }

        XMLShapeIdHint aNewHint;
        aNewHint.mnShapeIndex = nIndex;
        aNewHint.mnShapeId = mnNextUniqueShapeId++;
        maUsedShapeIds.push_back( aNewHint );
    }
    else
    {
        DBG_ERROR( "createShapeId failed!" );
    }
}

/** returns the unique id for this shape. It returns -1 if the was no createShapeId call
    for this shape yet. */
sal_Int32 XMLShapeExport::getShapeId( const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( xPropSet.is() && xPropSet->getPropertySetInfo()->hasPropertyByName( msZIndex ) )
    {
        sal_Int32 nIndex = 0;
        xPropSet->getPropertyValue( msZIndex ) >>= nIndex;

        const std::vector<XMLShapeIdHint>::size_type nCount = maUsedShapeIds.size();
        for( std::vector<XMLShapeIdHint>::size_type i = 0; i < nCount; i++ )
        {
            if( maUsedShapeIds[i].mnShapeIndex == nIndex )
                return maUsedShapeIds[i].mnShapeId;
        }
    }

    return -1;
}

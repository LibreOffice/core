/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-20 05:35:38 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmlmetae.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include <GradientStyle.hxx>
#endif
#ifndef _XMLOFF_HATCHSTYLE_HXX
#include <HatchStyle.hxx>
#endif
#ifndef _XMLOFF_IMAGESTYLE_HXX
#include <ImageStyle.hxx>
#endif
#ifndef _XMLOFF_TRANSGRADIENTSTYLE_HXX
#include <TransGradientStyle.hxx>
#endif
#ifndef _XMLOFF_MARKERSTYLE_HXX
#include <MarkerStyle.hxx>
#endif
#ifndef _XMLOFF_DASHSTYLE_HXX
#include <DashStyle.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

sal_Char __READONLY_DATA sXML_0_9[] = "0.9";

void SvXMLExport::_InitCtor()
{
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_OFFICE, sXML_np_office,
                               sXML_n_office, XML_NAMESPACE_OFFICE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_STYLE, sXML_np_style,
                               sXML_n_style, XML_NAMESPACE_STYLE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_FO, sXML_np_fo,
                               sXML_n_fo, XML_NAMESPACE_FO );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_XLINK, sXML_np_xlink,
                               sXML_n_xlink, XML_NAMESPACE_XLINK );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_DC, sXML_np_dc,
                               sXML_n_dc, XML_NAMESPACE_DC );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_META, sXML_np_meta,
                               sXML_n_meta, XML_NAMESPACE_META );

    // namespaces for documents
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_TEXT, sXML_np_text,
                               sXML_n_text, XML_NAMESPACE_TEXT );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_DRAW, sXML_np_draw,
                               sXML_n_draw, XML_NAMESPACE_DRAW );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_SVG, sXML_np_svg,
                               sXML_n_svg, XML_NAMESPACE_SVG );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_CHART, sXML_np_chart,
                               sXML_n_chart, XML_NAMESPACE_CHART );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_TABLE, sXML_np_table,
                               sXML_n_table, XML_NAMESPACE_TABLE );
    pNamespaceMap->AddAtIndex( XML_NAMESPACE_NUMBER, sXML_np_number,
                                  sXML_n_number, XML_NAMESPACE_NUMBER );

    xAttrList = (xml::sax::XAttributeList*)pAttrList;
}

SvXMLExport::SvXMLExport(
        const OUString &rFileName,
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        MapUnit eDfltUnit   ) :
    pImpl( 0 ),
    sCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    sWS( OUString::createFromAscii( sXML_WS ) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY )
{
    _InitCtor();
}

SvXMLExport::SvXMLExport(
        const OUString &rFileName,
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const Reference< XModel >& rModel,
        sal_Int16 eDfltUnit ) :
    pImpl( 0 ),
    sCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    sWS( OUString::createFromAscii( sXML_WS ) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit) ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    xModel( rModel ),
    pNumExport(0L),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY)
{
    _InitCtor();
    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(rHandler, xNumberFormatsSupplier);
}

SvXMLExport::~SvXMLExport()
{
    delete pNamespaceMap;
    delete pUnitConv;
}

void SvXMLExport::AddAttributeASCII( sal_uInt16 nPrefixKey,
                                     const sal_Char *pName,
                                       const sal_Char *pValue )
{
    OUString sName( OUString::createFromAscii( pName ) );
    OUString sValue( OUString::createFromAscii( pValue ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             sCDATA, sValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const sal_Char *pName,
                              const OUString& rValue )
{
    OUString sName( OUString::createFromAscii( pName ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             sCDATA, rValue );
}

#ifdef USED
void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const OUString& rName,
                              const OUString& rValue )
{
    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey, rName ),
                             sCDATA, rValue );
}
#endif

void SvXMLExport::AddAttributeList( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( xAttrList.is())
        pAttrList->AppendAttributeList( xAttrList );
}

void SvXMLExport::ClearAttrList()
{
    pAttrList->Clear();
}

#ifndef PRODUCT
void SvXMLExport::CheckAttrList()
{
    DBG_ASSERT( !pAttrList->getLength(),
                "XMLExport::CheckAttrList: list is not empty" );
}
#endif

void SvXMLExport::ImplExportMeta()
{
    CheckAttrList();

    {
        // <office:meta>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, sXML_meta,
                                sal_True, sal_True );

        _ExportMeta();
    }
}

void SvXMLExport::ImplExportStyles( sal_Bool bUsed )
{
    CheckAttrList();

//  AddAttributeASCII( XML_NAMESPACE_NONE, sXML_id, sXML_styles_id );
    {
        // <style:styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, sXML_styles,
                                sal_True, sal_True );

        _ExportStyles( sal_False );
    }

//  AddAttributeASCII( XML_NAMESPACE_NONE, sXML_id, sXML_auto_styles_id );
    {
        // <style:automatic-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  sXML_automatic_styles, sal_True, sal_True );

#if 0
        AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
        AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_href, sXML_styles_href );
        AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onLoad );
        AddAttribute( XML_NAMESPACE_XLINK, sXML_role,
                         pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                OUString::createFromAscii(sXML_stylesheet)) );
        {
            // <style:use-styles>
            SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                      sXML_use_styles, sal_True, sal_True );
        }
#endif
        _ExportAutoStyles();
    }

    {
        // <style:master-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, sXML_master_styles,
                                sal_True, sal_True );

        _ExportMasterStyles();
    }

#if 0
    AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
    AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_href, sXML_auto_styles_href );
    AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onLoad );
    AddAttribute( XML_NAMESPACE_XLINK, sXML_role,
                  pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                OUString::createFromAscii(sXML_stylesheet) ) );
    {
        // <style:use-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  sXML_use_styles, sal_True, sal_True );
    }
#endif
}

void SvXMLExport::ImplExportContent()
{
    CheckAttrList();

    {
        // <office:body ...>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, sXML_body,
                                  sal_True, sal_True );

        _ExportContent();
    }
}

sal_uInt32 SvXMLExport::exportDoc( const sal_Char *pClass )
{
    xHandler->startDocument();

    // <?xml version="1.0" encoding="UTF-8"?>
//  xHandler->processingInstruction( S2U( sXML_xml ), S2U( sXML_xml_pi ) );

    // <office:document ...>
    CheckAttrList();

    // office:class = ...
    if( pClass )
        AddAttributeASCII( XML_NAMESPACE_OFFICE, sXML_class, pClass );

    // office:version = ...
    if( !bExtended )
        AddAttributeASCII( XML_NAMESPACE_OFFICE, sXML_version, sXML_0_9 );

    sal_uInt16 nPos = pNamespaceMap->GetFirstIndex();
    while( USHRT_MAX != nPos )
    {
        pAttrList->AddAttribute( pNamespaceMap->GetAttrNameByIndex( nPos ),
                                 sCDATA,
                                 pNamespaceMap->GetNameByIndex( nPos ) );
        nPos = pNamespaceMap->GetNextIndex( nPos );
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, sXML_document,
                                sal_True, sal_True );

        // meta information
        ImplExportMeta();

        // styles
        ImplExportStyles( sal_False );

        // contnt
        ImplExportContent();
    }


    xHandler->endDocument();

    return 0;
}

void SvXMLExport::_ExportMeta()
{
    SfxXMLMetaExport aMeta( GetDocHandler(), xModel );
    aMeta.Export( GetNamespaceMap() );
}

void SvXMLExport::_ExportStyles( sal_Bool bUsed )
{
    uno::Reference< lang::XMultiServiceFactory > xFact( GetModel(), uno::UNO_QUERY );
    if( xFact.is())
    {
        // export (fill-)gradient-styles
        try
        {
            uno::Reference< container::XNameAccess > xGradient( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GradientTable") ) ), uno::UNO_QUERY );
            if( xGradient.is() )
            {
                XMLGradientStyle aGradientStyle( &xHandler, *pNamespaceMap, *pUnitConv );

                if( xGradient->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xGradient->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xGradient->getByName( rStrName );

                            aGradientStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export (fill-)hatch-styles
        try
        {
            uno::Reference< container::XNameAccess > xHatch( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.HatchTable") ) ), uno::UNO_QUERY );
            if( xHatch.is() )
            {
                XMLHatchStyle aHatchStyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xHatch->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xHatch->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xHatch->getByName( rStrName );

                            aHatchStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export (fill-)bitmap-styles
        try
        {
            uno::Reference< container::XNameAccess > xBitmap( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.BitmapTable") ) ), uno::UNO_QUERY );
            if( xBitmap.is() )
            {
                XMLImageStyle aImageStyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xBitmap->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xBitmap->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xBitmap->getByName( rStrName );

                            aImageStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export transparency-gradient -styles
        try
        {
            uno::Reference< container::XNameAccess > xTransGradient( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TransparencyGradientTable") ) ), uno::UNO_QUERY );
            if( xTransGradient.is() )
            {
                XMLTransGradientStyle aTransGradientstyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xTransGradient->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xTransGradient->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xTransGradient->getByName( rStrName );

                            aTransGradientstyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export marker-styles
        try
        {
            uno::Reference< container::XNameAccess > xMarker( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MarkerTable") ) ), uno::UNO_QUERY );
            if( xMarker.is() )
            {
                XMLMarkerStyle aMarkerStyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xMarker->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xMarker->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xMarker->getByName( rStrName );

                            aMarkerStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export dash-styles
        try
        {
            uno::Reference< container::XNameAccess > xDashes( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DashTable") ) ), uno::UNO_QUERY );
            if( xDashes.is() )
            {
                XMLDashStyle aDashStyle( &xHandler, *pNamespaceMap, *pUnitConv );

                if( xDashes->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xDashes->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xDashes->getByName( rStrName );

                            aDashStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}
    }
}

XMLTextParagraphExport* SvXMLExport::CreateTextParagraphExport()
{
    return new XMLTextParagraphExport( *this, *(GetAutoStylePool().get()) );
}

XMLShapeExport* SvXMLExport::CreateShapeExport()
{
    return new XMLShapeExport(*this);
}

SvXMLAutoStylePoolP* SvXMLExport::CreateAutoStylePool()
{
    return new SvXMLAutoStylePoolP();
}

XMLPageExport* SvXMLExport::CreatePageExport()
{
    return new XMLPageExport( *this );
}

SchXMLExportHelper* SvXMLExport::CreateChartExport()
{
    return new SchXMLExportHelper(*this,*GetAutoStylePool().get());
}

OUString SvXMLExport::getDataStyleName(const sal_Int32 nNumberFormat) const
{
    OUString sTemp;
    if(pNumExport)
        sTemp = pNumExport->GetStyleName(nNumberFormat);
    return sTemp;
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const sal_Char *pLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside )
{
    OUString sLName( OUString::createFromAscii(pLName) );
    aName = rExp.GetNamespaceMap().GetQNameByKey( nPrefixKey, sLName );

    if( bIWSOutside )
        rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
    rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList() );
    rExport.ClearAttrList();
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const OUString& rLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside )
{
    aName = rExp.GetNamespaceMap().GetQNameByKey( nPrefixKey, rLName );

    if( bIWSOutside )
        rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
    rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList() );
    rExport.ClearAttrList();
}

SvXMLElementExport::~SvXMLElementExport()
{
    if( bIgnWS )
        rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
    rExport.GetDocHandler()->endElement( aName );
}



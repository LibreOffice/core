/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.106 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:12:38 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <rtl/uuid.h>
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XBINARYSTREAMRESOLVER_HPP_
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXINVALIDCHARACTEREXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXInvalidCharacterException.hpp>
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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

#ifndef _XMLOFF_XMLBASICE_HXX
#include "xmlscripte.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif

#ifndef _XMLOFF_XMLSTARBASICEXPORTHANDLER_HXX
#include "XMLStarBasicExportHandler.hxx"
#endif

#ifndef _XMLOFF_XMLSCRIPTEXPORTHANDLER_HXX
#include "XMLScriptExportHandler.hxx"
#endif

#ifndef _XMLOFF_SETTINGSEXPORTHELPER_HXX
#include "SettingsExportHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
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
#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include "XMLFontAutoStylePool"
#endif
#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#include "XMLImageMapExport.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64EXPORT_HXX_
#include "XMLBase64Export.hxx"
#endif
#ifndef _XMLOFF_XMLERROR_HXX_
#include "xmlerror.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _XMLOFF_XMLFILTERSERVICENAMES_H
#include "XMLFilterServiceNames.h"
#endif
#ifndef _XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#include "XMLEmbeddedObjectExportFilter.hxx"
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;

sal_Char __READONLY_DATA sXML_1_0[] = "1.0";

#define LOGFILE_AUTHOR "mb93740"

#define XML_MODEL_SERVICE_WRITER    "com.sun.star.text.TextDocument"
#define XML_MODEL_SERVICE_CALC      "com.sun.star.sheet.SpreadsheetDocument"
#define XML_MODEL_SERVICE_DRAW      "com.sun.star.drawing.DrawingDocument"
#define XML_MODEL_SERVICE_IMPRESS   "com.sun.star.presentation.PresentationDocument"
#define XML_MODEL_SERVICE_MATH      "com.sun.star.formula.FormulaProperties"
#define XML_MODEL_SERVICE_CHART     "com.sun.star.chart.ChartDocument"

#define XML_USEPRETTYPRINTING "UsePrettyPrinting"

struct XMLServiceMapEntry_Impl
{
    const sal_Char *sModelService;
    sal_Int32      nModelServiceLen;
    const sal_Char *sFilterService;
    sal_Int32      nFilterServiceLen;
};

#define SERVICE_MAP_ENTRY( app ) \
    { XML_MODEL_SERVICE_##app, sizeof(XML_MODEL_SERVICE_##app)-1, \
      XML_EXPORT_FILTER_##app, sizeof(XML_EXPORT_FILTER_##app)-1 }

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( WRITER ),
    SERVICE_MAP_ENTRY( CALC ),
    SERVICE_MAP_ENTRY( IMPRESS ),// Impress supports DrawingDocument, too, so
    SERVICE_MAP_ENTRY( DRAW ),   // it must appear before Draw
    SERVICE_MAP_ENTRY( MATH ),
    SERVICE_MAP_ENTRY( CHART ),
    { 0, 0, 0, 0 }
};

//==============================================================================

class SvXMLExportEventListener : public cppu::WeakImplHelper1<
                            com::sun::star::lang::XEventListener >
{
private:
    SvXMLExport*    pExport;

public:
                            SvXMLExportEventListener(SvXMLExport* pExport);
    virtual                 ~SvXMLExportEventListener();

                            // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& rEventObject) throw(::com::sun::star::uno::RuntimeException);
};

SvXMLExportEventListener::SvXMLExportEventListener(SvXMLExport* pTempExport)
    : pExport(pTempExport)
{
}

SvXMLExportEventListener::~SvXMLExportEventListener()
{
}

// XEventListener
void SAL_CALL SvXMLExportEventListener::disposing( const lang::EventObject& rEventObject )
    throw(uno::RuntimeException)
{
    if (pExport)
        pExport->DisposingModel();
}

//==============================================================================

void SvXMLExport::SetDocHandler( const uno::Reference< xml::sax::XDocumentHandler > &rHandler )
{
    xHandler = rHandler;
    xExtHandler = uno::Reference<xml::sax::XExtendedDocumentHandler>( xHandler, UNO_QUERY );
}


void SvXMLExport::_InitCtor()
{
    if( (getExportFlags() & ~EXPORT_OASIS) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
        pNamespaceMap->Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    }
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO), XML_NAMESPACE_FO );
    }
    if( (getExportFlags() & (EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( (getExportFlags() & EXPORT_SETTINGS) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( (getExportFlags() & (EXPORT_META|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_DC), GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
        pNamespaceMap->Add( GetXMLToken(XML_NP_META), GetXMLToken(XML_N_META), XML_NAMESPACE_META );
    }
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    // namespaces for documents
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_TEXT), GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );
        pNamespaceMap->Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
        pNamespaceMap->Add( GetXMLToken(XML_NP_DR3D), GetXMLToken(XML_N_DR3D), XML_NAMESPACE_DR3D );
        pNamespaceMap->Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG),  XML_NAMESPACE_SVG );
        pNamespaceMap->Add( GetXMLToken(XML_NP_CHART), GetXMLToken(XML_N_CHART), XML_NAMESPACE_CHART );
        pNamespaceMap->Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
        pNamespaceMap->Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );
        pNamespaceMap->Add( GetXMLToken(XML_NP_OOOW), GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
        pNamespaceMap->Add( GetXMLToken(XML_NP_OOOC), GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
    }
    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_MATH), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
        pNamespaceMap->Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    }
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_SCRIPT), GetXMLToken(XML_N_SCRIPT), XML_NAMESPACE_SCRIPT );
        pNamespaceMap->Add( GetXMLToken(XML_NP_DOM), GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
    }

    xAttrList = (xml::sax::XAttributeList*)pAttrList;

    sPicturesPath = OUString( RTL_CONSTASCII_USTRINGPARAM( "#Pictures/" ) );
    sObjectsPath = OUString( RTL_CONSTASCII_USTRINGPARAM( "#./" ) );
    sGraphicObjectProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
    sEmbeddedObjectProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ) );

    if (xModel.is() && ! mxEventListener.is())
    {
        mxEventListener.set( new SvXMLExportEventListener(this));
        xModel->addEventListener(mxEventListener);
    }
}

// #110680#
SvXMLExport::SvXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    MapUnit eDfltUnit, const enum XMLTokenEnum eClass, sal_uInt16 nExportFlags )
:   pImpl( 0 ),
    // #110680#
    mxServiceFactory(xServiceFactory),
    meClass( eClass ),
    sWS( GetXMLToken(XML_WS) ),
    pNamespaceMap( new SvXMLNamespaceMap ),

    // #110680#
    // pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit ) ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit, getServiceFactory() ) ),

    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    pNumExport(0L),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    pXMLErrors( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( nExportFlags ),
    mnErrorFlags( ERROR_NO )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();
}

// #110680#
SvXMLExport::SvXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const OUString &rFileName,
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    MapUnit eDfltUnit   )
:   pImpl( 0 ),
    // #110680#
    mxServiceFactory(xServiceFactory),
    meClass( XML_TOKEN_INVALID ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),

    // #110680#
    // pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit ) ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit, getServiceFactory() ) ),

    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    pNumExport(0L),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    pXMLErrors( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( 0 ),
    mnErrorFlags( ERROR_NO )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(*this, xNumberFormatsSupplier);
}

// #110680#
SvXMLExport::SvXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const OUString &rFileName,
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    const Reference< XModel >& rModel,
    sal_Int16 eDfltUnit )
:   pImpl( 0 ),
    // #110680#
    mxServiceFactory(xServiceFactory),
    meClass( XML_TOKEN_INVALID ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),

    // #110680#
    // pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit) ) ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit), getServiceFactory() ) ),

    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    xModel( rModel ),
    pNumExport(0L),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    pXMLErrors( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( 0 ),
    mnErrorFlags( ERROR_NO )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(*this, xNumberFormatsSupplier);
}

// #110680#
SvXMLExport::SvXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const OUString &rFileName,
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    const Reference< XModel >& rModel,
    const Reference< document::XGraphicObjectResolver >& rEmbeddedGraphicObjects,
    sal_Int16 eDfltUnit )
:   pImpl( 0 ),
    // #110680#
    mxServiceFactory(xServiceFactory),
    meClass( XML_TOKEN_INVALID ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),

    // #110680#
    // pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit) ) ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit), getServiceFactory() ) ),

    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    xGraphicResolver( rEmbeddedGraphicObjects ),
    xModel( rModel ),
    pNumExport(0L),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    pXMLErrors( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( 0 ),
    mnErrorFlags( ERROR_NO )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(*this, xNumberFormatsSupplier);
}

SvXMLExport::~SvXMLExport()
{
    delete pXMLErrors;
    delete pImageMapExport;
    delete pEventExport;
    delete pNamespaceMap;
    delete pUnitConv;
    if (pProgressBarHelper || pNumExport)
    {
        if (xExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                if (pProgressBarHelper)
                {
                    OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                    OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                    OUString sRepeat(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSREPEAT));
                    if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                        xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                    {
                        sal_Int32 nProgressMax(pProgressBarHelper->GetReference());
                        sal_Int32 nProgressCurrent(pProgressBarHelper->GetValue());
                        uno::Any aAny;
                        aAny <<= nProgressMax;
                        xExportInfo->setPropertyValue(sProgressMax, aAny);
                        aAny <<= nProgressCurrent;
                        xExportInfo->setPropertyValue(sProgressCurrent, aAny);
                    }
                    if (xPropertySetInfo->hasPropertyByName(sRepeat))
                        xExportInfo->setPropertyValue(sRepeat, cppu::bool2any(pProgressBarHelper->GetRepeat()));
                }
                if (pNumExport && (mnExportFlags & (EXPORT_AUTOSTYLES | EXPORT_STYLES)))
                {
                    OUString sWrittenNumberFormats(RTL_CONSTASCII_USTRINGPARAM(XML_WRITTENNUMBERSTYLES));
                    if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
                    {
                        uno::Sequence<sal_Int32> aWasUsed;
                        pNumExport->GetWasUsed(aWasUsed);
                        uno::Any aAny;
                        aAny <<= aWasUsed;
                        xExportInfo->setPropertyValue(sWrittenNumberFormats, aAny);
                    }
                }
            }
        }
        delete pProgressBarHelper;
        delete pNumExport;
    }

    xmloff::token::ResetTokens();

    if (mxEventListener.is() && xModel.is())
        xModel->removeEventListener(mxEventListener);
}

///////////////////////////////////////////////////////////////////////

// XExporter
void SAL_CALL SvXMLExport::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    xModel = uno::Reference< frame::XModel >::query( xDoc );
    if( !xModel.is() )
        throw lang::IllegalArgumentException();
    if (xModel.is() && ! mxEventListener.is())
    {
        mxEventListener.set( new SvXMLExportEventListener(this));
        xModel->addEventListener(mxEventListener);
    }

    if(!xNumberFormatsSupplier.is() )
    {
        xNumberFormatsSupplier = xNumberFormatsSupplier.query( xModel );
        if(xNumberFormatsSupplier.is() && xHandler.is())
            pNumExport = new SvXMLNumFmtExport(*this, xNumberFormatsSupplier);
    }
    if (xExportInfo.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM(XML_USEPRETTYPRINTING));
            if (xPropertySetInfo->hasPropertyByName(sUsePrettyPrinting))
            {
                uno::Any aAny = xExportInfo->getPropertyValue(sUsePrettyPrinting);
                if (::cppu::any2bool(aAny))
                    mnExportFlags |= EXPORT_PRETTY;
                else
                    mnExportFlags &= ~EXPORT_PRETTY;
            }
            if (pNumExport && (mnExportFlags & (EXPORT_AUTOSTYLES | EXPORT_STYLES)))
            {
                OUString sWrittenNumberFormats(RTL_CONSTASCII_USTRINGPARAM(XML_WRITTENNUMBERSTYLES));
                if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
                {
                    uno::Any aAny = xExportInfo->getPropertyValue(sWrittenNumberFormats);
                    uno::Sequence<sal_Int32> aWasUsed;
                    if(aAny >>= aWasUsed)
                        pNumExport->SetWasUsed(aWasUsed);
                }
            }
        }
    }

    // namespaces for user defined attributes
    Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY );
    if( xFactory.is() )
    {
        try
        {
            Reference < XInterface > xIfc =
                xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM(
                                            "com.sun.star.xml.NamespaceMap")) );
            if( xIfc.is() )
            {
                Reference< XNameAccess > xNamespaceMap( xIfc, UNO_QUERY );
                if( xNamespaceMap.is() )
                {
                    Sequence< OUString > aPrefixes( xNamespaceMap->getElementNames() );

                    OUString* pPrefix = aPrefixes.getArray();
                    const sal_Int32 nCount = aPrefixes.getLength();
                    sal_Int32 nIndex;
                    OUString aURL;

                    for( nIndex = 0; nIndex < nCount; nIndex++, *pPrefix++ )
                    {
                        if( xNamespaceMap->getByName( *pPrefix ) >>= aURL )
                            _GetNamespaceMap().Add( *pPrefix, aURL, XML_NAMESPACE_UNKNOWN );
                    }
                }
            }
        }
        catch( com::sun::star::uno::Exception& )
        {
        }
    }
}

// XInitialize
void SAL_CALL SvXMLExport::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // #93186# we need to queryInterface every single Any with any expected outcome. This variable hold the queryInterface results.

    const sal_Int32 nAnyCount = aArguments.getLength();
    const uno::Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        Reference<XInterface> xValue;
        *pAny >>= xValue;

        // status indicator
        uno::Reference<task::XStatusIndicator> xTmpStatus( xValue, UNO_QUERY );
        if ( xTmpStatus.is() )
            xStatusIndicator = xTmpStatus;

        // graphic resolver
        uno::Reference<document::XGraphicObjectResolver> xTmpGraphic(
            xValue, UNO_QUERY );
        if ( xTmpGraphic.is() )
            xGraphicResolver = xTmpGraphic;

        // object resolver
        uno::Reference<document::XEmbeddedObjectResolver> xTmpObjectResolver(
            xValue, UNO_QUERY );
        if ( xTmpObjectResolver.is() )
            xEmbeddedResolver = xTmpObjectResolver;

        // document handler
        uno::Reference<xml::sax::XDocumentHandler> xTmpDocHandler(
            xValue, UNO_QUERY );
        if( xTmpDocHandler.is() )
        {
            xHandler = xTmpDocHandler;
            *pAny >>= xExtHandler;

            if (xNumberFormatsSupplier.is() && pNumExport == NULL)
                pNumExport = new SvXMLNumFmtExport(*this, xNumberFormatsSupplier);
        }

        // property set to transport data across
        uno::Reference<beans::XPropertySet> xTmpPropertySet(
            xValue, UNO_QUERY );
        if( xTmpPropertySet.is() )
            xExportInfo = xTmpPropertySet;

    }

    if( xExportInfo.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo =
            xExportInfo->getPropertySetInfo();
        OUString sPropName(
                RTL_CONSTASCII_USTRINGPARAM("BaseURI" ) );
        if( xPropertySetInfo->hasPropertyByName(sPropName) )
        {
            uno::Any aAny = xExportInfo->getPropertyValue(sPropName);
            aAny >>= sOrigFileName;
        }
        OUString sRelPath;
        sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("StreamRelPath" ) );
        if( xPropertySetInfo->hasPropertyByName(sPropName) )
        {
            uno::Any aAny = xExportInfo->getPropertyValue(sPropName);
            aAny >>= sRelPath;
        }
        OUString sName;
        sPropName = OUString( RTL_CONSTASCII_USTRINGPARAM("StreamName" ) );
        if( xPropertySetInfo->hasPropertyByName(sPropName) )
        {
            uno::Any aAny = xExportInfo->getPropertyValue(sPropName);
            aAny >>= sName;
        }
        if( sOrigFileName.getLength() && sName.getLength() )
        {
            INetURLObject aBaseURL( sOrigFileName );
            if( sRelPath.getLength() )
                aBaseURL.insertName( sRelPath );
            aBaseURL.insertName( sName );
            sOrigFileName = aBaseURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
        }
    }

}

// XFilter
sal_Bool SAL_CALL SvXMLExport::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor ) throw(uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogContext, "xmloff", LOGFILE_AUTHOR,
                                "SvXMLExport::filter" );

    // check for xHandler first... should have been supplied in initialize
    if( !xHandler.is() )
        return sal_False;

    try
    {
        const sal_uInt32 nTest =
            EXPORT_META|EXPORT_STYLES|EXPORT_CONTENT|EXPORT_SETTINGS;
        if( (mnExportFlags & nTest) == nTest && !sOrigFileName.getLength() )
        {
            // evaluate descriptor only for flat files and if a base URI
            // has not been provided already
            const sal_Int32 nPropCount = aDescriptor.getLength();
            const beans::PropertyValue* pProps = aDescriptor.getConstArray();

            for( sal_Int32 nIndex = 0; nIndex < nPropCount; nIndex++, pProps++ )
            {
                const OUString& rPropName = pProps->Name;
                const Any& rValue = pProps->Value;

                if( rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FileName" ) ) )
                {
                    if( !(rValue >>= sOrigFileName ) )
                        return sal_False;
                }
                else if (rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FilterName" ) ) )
                {
                    if( !(rValue >>= sFilterName ) )
                        return sal_False;
                }
            }
        }

#ifdef TIMELOG
        if (GetModel().is())
        {
            // print a trace message with the URL
            ByteString aUrl( (String) GetModel()->getURL(),
                             RTL_TEXTENCODING_ASCII_US );
            RTL_LOGFILE_CONTEXT_TRACE1( aLogContext, "%s", aUrl.GetBuffer() );

            // we also want a trace message with the document class
            ByteString aClass( (String)GetXMLToken(meClass),
                               RTL_TEXTENCODING_ASCII_US );
            RTL_LOGFILE_CONTEXT_TRACE1( aLogContext, "class=\"%s\"",
                                        aClass.GetBuffer() );
        }
#endif

        exportDoc( meClass );
    }
    catch( uno::Exception e )
    {
        // We must catch exceptions, because according to the
        // API definition export must not throw one!
        Sequence<OUString> aSeq(0);
        SetError( XMLERROR_FLAG_ERROR | XMLERROR_FLAG_SEVERE | XMLERROR_API,
                  aSeq, e.Message, NULL );
    }

    // return true only if no error occured
    return (GetErrorFlags() & (ERROR_DO_NOTHING|ERROR_ERROR_OCCURED)) == 0;
}

void SAL_CALL SvXMLExport::cancel() throw(uno::RuntimeException)
{
    // stop export
    Sequence<OUString> aEmptySeq;
    SetError(XMLERROR_CANCEL|XMLERROR_FLAG_SEVERE, aEmptySeq);
}

::rtl::OUString SAL_CALL SvXMLExport::getName(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return sFilterName;
}

void SAL_CALL SvXMLExport::setName( const ::rtl::OUString& aName )
    throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing, because it is not possible to set the FilterName
}


// XServiceInfo
OUString SAL_CALL SvXMLExport::getImplementationName(  ) throw(uno::RuntimeException)
{
    OUString aStr;
    return aStr;
}

sal_Bool SAL_CALL SvXMLExport::supportsService( const OUString& rServiceName ) throw(uno::RuntimeException)
{
    return
        rServiceName.equalsAsciiL(
            "com.sun.star.document.ExportFilter",
            sizeof("com.sun.star.document.ExportFilter")-1 ) ||
        rServiceName.equalsAsciiL(
            "com.sun.star.xml.XMLExportFilter",
            sizeof("com.sun.star.xml.XMLExportFilter")-1);
}

uno::Sequence< OUString > SAL_CALL SvXMLExport::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aSeq(2);
    OUString* pSeq = aSeq.getArray();
    aSeq[0] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ExportFilter"));
    aSeq[1] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.XMLExportFilter"));
    return aSeq;
}

///////////////////////////////////////////////////////////////////////

void SvXMLExport::AddAttributeASCII( sal_uInt16 nPrefixKey,
                                     const sal_Char *pName,
                                       const sal_Char *pValue )
{
    OUString sName( OUString::createFromAscii( pName ) );
    OUString sValue( OUString::createFromAscii( pValue ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             sValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const sal_Char *pName,
                              const OUString& rValue )
{
    OUString sName( OUString::createFromAscii( pName ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const OUString& rName,
                              const OUString& rValue )
{
    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey, rName ),
                             rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                const OUString& rValue )
{
    pAttrList->AddAttribute(
        pNamespaceMap->GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                enum XMLTokenEnum eValue)
{
    pAttrList->AddAttribute(
        pNamespaceMap->GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        GetXMLToken(eValue) );
}

void SvXMLExport::AddAttribute( const ::rtl::OUString& rQName,
                                const ::rtl::OUString& rValue )
{
      pAttrList->AddAttribute(
        rQName,
        rValue );
}

void SvXMLExport::AddAttribute( const ::rtl::OUString& rQName,
                                enum ::xmloff::token::XMLTokenEnum eValue )
{
      pAttrList->AddAttribute(
        rQName,
        GetXMLToken(eValue) );
}

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
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_META,
                                sal_True, sal_True );

        _ExportMeta();
    }
}

void SvXMLExport::ImplExportSettings()
{
    CheckAttrList();

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_SETTINGS,
                                sal_True, sal_True );
        XMLSettingsExportHelper aSettingsExportHelper(*this);
        _ExportViewSettings(aSettingsExportHelper);
        _ExportConfigurationSettings(aSettingsExportHelper);
    }
}

void SvXMLExport::ImplExportStyles( sal_Bool bUsed )
{
    CheckAttrList();

//  AddAttribute( XML_NAMESPACE_NONE, XML_ID, XML_STYLES_ID );
    {
        // <style:styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_STYLES,
                                sal_True, sal_True );

        _ExportStyles( sal_False );
    }
}

void SvXMLExport::ImplExportAutoStyles( sal_Bool bUsed )
{
//  AddAttributeASCII( XML_NAMESPACE_NONE, XML_ID, XML_AUTO_STYLES_ID );
    {
        // <style:automatic-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  XML_AUTOMATIC_STYLES, sal_True, sal_True );

#if 0
        AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, XML_STYLES_HREF );
        AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        AddAttribute( XML_NAMESPACE_XLINK, XML_ROLE,
                         pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                               GetXMLToken(XML_STYLESHEET)) );
        {
            // <style:use-styles>
            SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                      XML_USE_STYLES, sal_True, sal_True );
        }
#endif
        _ExportAutoStyles();
    }
}

void SvXMLExport::ImplExportMasterStyles( sal_Bool bUsed )
{
    {
        // <style:master-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,
                                sal_True, sal_True );

        _ExportMasterStyles();
    }

#if 0
    AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, XML_AUTO_STYLES_HREF );
    AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
    AddAttribute( XML_NAMESPACE_XLINK, XML_ROLE,
                  pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                                GetXMLToken(XML_STYLESHEET) ) );
    {
        // <style:use-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  XML_USE_STYLES, sal_True, sal_True );
    }
#endif
}

void SvXMLExport::ImplExportContent()
{
    CheckAttrList();

    CheckAttrList();

    {
        // <office:body ...>
        SetBodyAttributes();
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_BODY,
                                  sal_True, sal_True );
        {
            XMLTokenEnum eClass = meClass;
            if( XML_TEXT_GLOBAL == eClass )
            {
                AddAttribute( XML_NAMESPACE_TEXT, XML_GLOBAL,
                      GetXMLToken( XML_TRUE ) );
                eClass = XML_TEXT;
            }
            SvXMLElementExport aElem( *this, meClass != XML_TOKEN_INVALID,
                                      XML_NAMESPACE_OFFICE, eClass,
                                        sal_True, sal_True );

            _ExportContent();
        }
    }
}

void SvXMLExport::SetBodyAttributes()
{
}

sal_uInt32 SvXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    bool bOwnGraphicResolver = false;
    bool bOwnEmbeddedResolver = false;

    if( !xGraphicResolver.is() || !xEmbeddedResolver.is() )
    {
        Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY );
        if( xFactory.is() )
        {
            try
            {
                if( !xGraphicResolver.is() )
                {
                    xGraphicResolver = Reference< XGraphicObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.document.ExportGraphicObjectResolver"))));
                    bOwnGraphicResolver = xGraphicResolver.is();
                }

                if( !xEmbeddedResolver.is() )
                {
                    xEmbeddedResolver = Reference< XEmbeddedObjectResolver >::query(
                        xFactory->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.document.ExportEmbeddedObjectResolver"))));
                    bOwnEmbeddedResolver = xEmbeddedResolver.is();
                }
            }
            catch( com::sun::star::uno::Exception& )
            {
            }
        }
    }
    if( (getExportFlags() & EXPORT_OASIS) == 0 )
    {
        Reference< lang::XMultiServiceFactory > xFactory = getServiceFactory();
        if( xFactory.is() )
        {
            try
            {
                ::comphelper::PropertyMapEntry aInfoMap[] =
                {
                    { "Class", sizeof("Class")-1, 0,
                        &::getCppuType((::rtl::OUString*)0),
                          PropertyAttribute::MAYBEVOID, 0},
                    { NULL, 0, 0, NULL, 0, 0 }
                };
                Reference< XPropertySet > xConvPropSet(
                    ::comphelper::GenericPropertySet_CreateInstance(
                            new ::comphelper::PropertySetInfo( aInfoMap ) ) );

                Any aAny;
                aAny <<= GetXMLToken( eClass );
                xConvPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("Class")), aAny );

                Reference< XPropertySet > xPropSet =
                    xExportInfo.is()
                        ?  PropertySetMerger_CreateInstance( xExportInfo,
                                                          xConvPropSet )
                        : xExportInfo;

                Sequence<Any> aArgs( 2 );
                aArgs[0] <<= xHandler;
                aArgs[1] <<= xPropSet;

                // get filter component
                Reference< xml::sax::XDocumentHandler > xTmpDocHandler(
                    xFactory->createInstanceWithArguments(
                    OUString::createFromAscii("com.sun.star.comp.Oasis2OOoTransformer"),
                                aArgs), UNO_QUERY);
                OSL_ENSURE( xTmpDocHandler.is(),
                    "can't instantiate OASIS transformer component" );
                if( xTmpDocHandler.is() )
                {
                    xHandler = xTmpDocHandler;
                    xExtHandler = uno::Reference<xml::sax::XExtendedDocumentHandler>( xHandler, UNO_QUERY );
                }
            }
            catch( com::sun::star::uno::Exception& )
            {
            }
        }
    }


    xHandler->startDocument();

    // <?xml version="1.0" encoding="UTF-8"?>
//  xHandler->processingInstruction( S2U( sXML_xml ), S2U( sXML_xml_pi ) );

    // <office:document ...>
    CheckAttrList();

    // namespace attributes
    // ( The namespace decls should be first attributes in the element;
    //   some faulty XML parsers (JAXP1.1) have a problem with this,
    //   also it's more elegant )
    sal_uInt16 nPos = pNamespaceMap->GetFirstKey();
    while( USHRT_MAX != nPos )
    {
        pAttrList->AddAttribute( pNamespaceMap->GetAttrNameByKey( nPos ),
                                 pNamespaceMap->GetNameByKey( nPos ) );
        nPos = pNamespaceMap->GetNextKey( nPos );
    }



    // office:version = ...
    if( !bExtended )
        AddAttribute( XML_NAMESPACE_OFFICE, XML_VERSION,
                      OUString::createFromAscii(sXML_1_0) );

    {
        enum XMLTokenEnum eRootService = XML_TOKEN_INVALID;
        const sal_Int32 nExportMode = mnExportFlags & (EXPORT_META|EXPORT_STYLES|EXPORT_CONTENT|EXPORT_SETTINGS);
        if( EXPORT_META == nExportMode )
        {
            // export only meta
            eRootService = XML_DOCUMENT_META;
        }
        else if ( EXPORT_SETTINGS == nExportMode )
        {
            // export only settings
            eRootService = XML_DOCUMENT_SETTINGS;
        }
        else if( EXPORT_STYLES == nExportMode )
        {
            // export only styles
            eRootService = XML_DOCUMENT_STYLES;
        }
        else if( EXPORT_CONTENT == nExportMode )
        {
            // export only content
            eRootService = XML_DOCUMENT_CONTENT;
        }
        else
        {
            // the god'ol one4all element
            eRootService = XML_DOCUMENT;
            // office:mimetype = ... (only for stream containing the content)
            if( eClass != XML_TOKEN_INVALID )
            {
                OUString aTmp( RTL_CONSTASCII_USTRINGPARAM("application/x-vnd.oasis.openoffice.") );
                aTmp += GetXMLToken( eClass );
                AddAttribute( XML_NAMESPACE_OFFICE, XML_MIMETYPE, aTmp );
            }
        }

//      if( (getExportFlags() & EXPORT_NODOCTYPE) == 0 &&
//          xExtHandler.is() )
//      {
//          OUStringBuffer aDocType(
//               GetXMLToken(XML_XML_DOCTYPE_PREFIX).getLength() +
//              GetXMLToken(XML_XML_DOCTYPE_SUFFIX).getLength() + 30 );
//
//          aDocType.append( GetXMLToken(XML_XML_DOCTYPE_PREFIX) );
//          aDocType.append( GetNamespaceMap().GetQNameByKey(
//                         XML_NAMESPACE_OFFICE, GetXMLToken(eRootService) ) );
//          aDocType.append( GetXMLToken(XML_XML_DOCTYPE_SUFFIX) );
//          xExtHandler->unknown( aDocType.makeStringAndClear() );
//      }

        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, eRootService, sal_True, sal_True );

        // meta information
        if( mnExportFlags & EXPORT_META )
            ImplExportMeta();

        // settings
        if( mnExportFlags & EXPORT_SETTINGS )
            ImplExportSettings();

        // scripts
        if( mnExportFlags & EXPORT_SCRIPTS )
            _ExportScripts();

        // font declerations
        if( mnExportFlags & EXPORT_FONTDECLS )
            _ExportFontDecls();

        // styles
        if( mnExportFlags & EXPORT_STYLES )
            ImplExportStyles( sal_False );

        // autostyles
        if( mnExportFlags & EXPORT_AUTOSTYLES )
            ImplExportAutoStyles( sal_False );

        // masterstyles
        if( mnExportFlags & EXPORT_MASTERSTYLES )
            ImplExportMasterStyles( sal_False );

        // contnt
        if( mnExportFlags & EXPORT_CONTENT )
            ImplExportContent();
    }


    xHandler->endDocument();

    if( bOwnGraphicResolver )
    {
        Reference< XComponent > xComp( xGraphicResolver, UNO_QUERY );
        xComp->dispose();
    }

    if( bOwnEmbeddedResolver )
    {
        Reference< XComponent > xComp( xEmbeddedResolver, UNO_QUERY );
        xComp->dispose();
    }

    return 0;
}

void SvXMLExport::_ExportMeta()
{
    SfxXMLMetaExport aMeta( *this, xModel );
    aMeta.Export();
}

void SvXMLExport::_ExportViewSettings(const XMLSettingsExportHelper& rSettingsExportHelper)
{
    uno::Sequence<beans::PropertyValue> aProps;
    GetViewSettings(aProps);
    uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetModel(), uno::UNO_QUERY);
    if(xViewDataSupplier.is())
    {
        uno::Reference<container::XIndexAccess> xIndexAccess;
        xViewDataSupplier->setViewData( xIndexAccess ); // make sure we get a newly created sequence
        xIndexAccess = xViewDataSupplier->getViewData();
        if(xIndexAccess.is())
        {
            sal_Int32 nOldLength(aProps.getLength());
            aProps.realloc(nOldLength + 1);
            beans::PropertyValue aProp;
            aProp.Name = OUString(RTL_CONSTASCII_USTRINGPARAM("Views"));
            aProp.Value <<= xIndexAccess;
            aProps[nOldLength] = aProp;
        }
    }
    OUString sViewSettings(GetXMLToken(XML_VIEW_SETTINGS));
    rSettingsExportHelper.exportSettings(aProps, sViewSettings);
}

void SvXMLExport::_ExportConfigurationSettings(const XMLSettingsExportHelper& rSettingsExportHelper)
{
    uno::Sequence<beans::PropertyValue> aProps;
    GetConfigurationSettings(aProps);
    OUString sConfigurationSettings(GetXMLToken(XML_CONFIGURATION_SETTINGS));
    rSettingsExportHelper.exportSettings(aProps, sConfigurationSettings);
}

void SvXMLExport::_ExportScripts()
{
// There is no script support at the moment, so we don't need this
    // <office:script>
//  SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_SCRIPTS,
//                          sal_True, sal_True );

//  XMLBasicExport aBasicExp( *this );
//  aBasicExp.Export();

    // export document events
//  Reference<document::XEventsSupplier> xEvents(GetModel(), UNO_QUERY);
//  GetEventExport().Export(xEvents, sal_True);
}

void SvXMLExport::_ExportFontDecls()
{
    if( mxFontAutoStylePool.is() )
        mxFontAutoStylePool->exportXML();
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
                XMLGradientStyleExport aGradientStyle( *this );

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
                XMLHatchStyleExport aHatchStyle( *this );

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
                XMLImageStyle aImageStyle;

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

                            aImageStyle.exportXML( rStrName, aValue, *this );
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
                XMLTransGradientStyleExport aTransGradientstyle( *this );

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
                XMLMarkerStyleExport aMarkerStyle( *this );

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
                XMLDashStyleExport aDashStyle( *this );

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
    return new SvXMLAutoStylePoolP(*this);
}

XMLPageExport* SvXMLExport::CreatePageExport()
{
    return new XMLPageExport( *this );
}

SchXMLExportHelper* SvXMLExport::CreateChartExport()
{
    return new SchXMLExportHelper(*this,*GetAutoStylePool().get());
}

XMLFontAutoStylePool* SvXMLExport::CreateFontAutoStylePool()
{
    return new XMLFontAutoStylePool( *this );
}

xmloff::OFormLayerXMLExport* SvXMLExport::CreateFormExport()
{
    return new xmloff::OFormLayerXMLExport(*this);
}

void SvXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& aProps)
{
}

void SvXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& aProps)
{
}

void SvXMLExport::addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat )
{
    if(pNumExport)
        pNumExport->SetUsed(nNumberFormat);
}

void SvXMLExport::exportDataStyles()
{
    if(pNumExport)
        pNumExport->Export(sal_False);
}

void SvXMLExport::exportAutoDataStyles()
{
    if(pNumExport)
        pNumExport->Export(sal_True);

    if (mxFormExport.is())
        mxFormExport->exportAutoControlNumberStyles();
}

OUString SvXMLExport::getDataStyleName(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat ) const
{
    OUString sTemp;
    if(pNumExport)
        sTemp = pNumExport->GetStyleName(nNumberFormat);
    return sTemp;
}

sal_Int32 SvXMLExport::dataStyleForceSystemLanguage(sal_Int32 nFormat) const
{
    return ( pNumExport != NULL )
                 ? pNumExport->ForceSystemLanguage( nFormat ) : nFormat;
}


OUString SvXMLExport::AddEmbeddedGraphicObject( const OUString& rGraphicObjectURL )
{
    OUString sRet( rGraphicObjectURL );
    if( 0 == rGraphicObjectURL.compareTo( sGraphicObjectProtocol,
                                          sGraphicObjectProtocol.getLength() ) &&
        xGraphicResolver.is() )
    {
        if( (getExportFlags() & EXPORT_EMBEDDED) == 0 )
            sRet = xGraphicResolver->resolveGraphicObjectURL( rGraphicObjectURL );
        else
            sRet = OUString();
    }
    else
        sRet = GetRelativeReference( sRet );

    return sRet;
}

sal_Bool SvXMLExport::AddEmbeddedGraphicObjectAsBase64( const OUString& rGraphicObjectURL )
{
    sal_Bool bRet = sal_False;

    if( (getExportFlags() & EXPORT_EMBEDDED) != 0 &&
        0 == rGraphicObjectURL.compareTo( sGraphicObjectProtocol,
                                          sGraphicObjectProtocol.getLength() ) &&
        xGraphicResolver.is() )
    {
        Reference< XBinaryStreamResolver > xStmResolver( xGraphicResolver, UNO_QUERY );

        if( xStmResolver.is() )
        {
            Reference< XInputStream > xIn( xStmResolver->getInputStream( rGraphicObjectURL ) );

            if( xIn.is() )
            {
                XMLBase64Export aBase64Exp( *this );
                bRet = aBase64Exp.exportOfficeBinaryDataElement( xIn );
            }
        }
    }

    return bRet;
}

OUString SvXMLExport::AddEmbeddedObject( const OUString& rEmbeddedObjectURL )
{
    OUString sRet;
    if( 0 == rEmbeddedObjectURL.compareTo( sEmbeddedObjectProtocol,
                sEmbeddedObjectProtocol.getLength() ) &&
        xEmbeddedResolver.is() )
    {
        sRet =
            xEmbeddedResolver->resolveEmbeddedObjectURL( rEmbeddedObjectURL );
    }

    return sRet;
}

sal_Bool SvXMLExport::AddEmbeddedObjectAsBase64( const OUString& rEmbeddedObjectURL )
{
    sal_Bool bRet = sal_False;
    if( 0 == rEmbeddedObjectURL.compareTo( sEmbeddedObjectProtocol,
                sEmbeddedObjectProtocol.getLength() ) &&
        xEmbeddedResolver.is() )
    {
        Reference < XNameAccess > xNA( xEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            Any aAny = xNA->getByName( rEmbeddedObjectURL );
            Reference < XInputStream > xIn;
            aAny >>= xIn;
            if( xIn.is() )
            {
                XMLBase64Export aBase64Exp( *this );
                bRet = aBase64Exp.exportOfficeBinaryDataElement( xIn );
            }
        }
    }

    return bRet;
}

OUString SvXMLExport::EncodeStyleName(
        const OUString& rName,
        sal_Bool *pEncoded ) const
{
    return GetMM100UnitConverter().encodeStyleName( rName, pEncoded );
}

ProgressBarHelper*  SvXMLExport::GetProgressBarHelper()
{
    if (!pProgressBarHelper)
    {
        pProgressBarHelper = new ProgressBarHelper(xStatusIndicator, sal_True);

        if (pProgressBarHelper && xExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSRANGE));
                OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                OUString sRepeat(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSREPEAT));
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent) &&
                    xPropertySetInfo->hasPropertyByName(sProgressRange))
                {
                    uno::Any aAny;
                    sal_Int32 nProgressMax(0);
                    sal_Int32 nProgressCurrent(0);
                    sal_Int32 nProgressRange(0);
                    aAny = xExportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        pProgressBarHelper->SetRange(nProgressRange);
                    aAny = xExportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        pProgressBarHelper->SetReference(nProgressMax);
                    aAny = xExportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        pProgressBarHelper->SetValue(nProgressCurrent);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                {
                    uno::Any aAny = xExportInfo->getPropertyValue(sRepeat);
                    if (aAny.getValueType() == getBooleanCppuType())
                        pProgressBarHelper->SetRepeat(::cppu::any2bool(aAny));
                    else
                        DBG_ERRORFILE("why is it no boolean?");
                }
            }
        }
    }
    return pProgressBarHelper;
}

XMLEventExport& SvXMLExport::GetEventExport()
{
    if( NULL == pEventExport)
    {
        // create EventExport on demand
        pEventExport = new XMLEventExport(*this, NULL);

        // and register standard handlers + names
        OUString sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic"));
        pEventExport->AddHandler(sStarBasic, new XMLStarBasicExportHandler());
        OUString sScript(RTL_CONSTASCII_USTRINGPARAM("Script"));
        pEventExport->AddHandler(sScript, new XMLScriptExportHandler());
        pEventExport->AddTranslationTable(aStandardEventTable);
    }

    return *pEventExport;
}

XMLImageMapExport& SvXMLExport::GetImageMapExport()
{
    // image map export, create on-demand
    if( NULL == pImageMapExport )
    {
        pImageMapExport = new XMLImageMapExport(*this);
    }

    return *pImageMapExport;
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLExport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvXMLExport* SvXMLExport::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
        return (SvXMLExport*)xUT->getSomething( SvXMLExport::getUnoTunnelId() );
    else
        return NULL;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLExport::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

sal_Bool SvXMLExport::ExportEmbeddedOwnObject( Reference< XComponent >& rComp )
{
    OUString sFilterService;

    Reference < lang::XServiceInfo > xServiceInfo( rComp, UNO_QUERY );
    if( xServiceInfo.is() )
    {
        const XMLServiceMapEntry_Impl *pEntry = aServiceMap;
        while( pEntry->sModelService )
        {
            OUString sModelService( pEntry->sModelService,
                                    pEntry->nModelServiceLen,
                                       RTL_TEXTENCODING_ASCII_US );
            if( xServiceInfo->supportsService( sModelService ) )
            {
                sFilterService = OUString( pEntry->sFilterService,
                                           pEntry->nFilterServiceLen,
                                              RTL_TEXTENCODING_ASCII_US );
                break;
            }
            pEntry++;
        }
    }

    OSL_ENSURE( sFilterService.getLength(), "no export filter for own object" );

    if( !sFilterService.getLength() )
        return sal_False;

    Reference < XDocumentHandler > xHdl =
        new XMLEmbeddedObjectExportFilter( xHandler );

    Sequence < Any > aArgs( 1 );
    aArgs[0] <<= xHdl;

    // #110680#
    // Reference< lang::XMultiServiceFactory > xServiceFactory = comphelper::getProcessServiceFactory();
    Reference< lang::XMultiServiceFactory > xServiceFactory = getServiceFactory();

    Reference< document::XExporter > xExporter(
        xServiceFactory->createInstanceWithArguments( sFilterService, aArgs),
                                               UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
                "can't instantiate export filter component for own object" );
    if( !xExporter.is() )
        return sal_False;

    xExporter->setSourceDocument( rComp );

    Reference<XFilter> xFilter( xExporter, UNO_QUERY );

    Sequence < PropertyValue > aMediaDesc( 0 );
    return xFilter->filter( aMediaDesc );
}

OUString SvXMLExport::GetRelativeReference(const OUString& rValue)
{
    // That's ugly, but it is a temporary solution only that will be
    // changed in CWS sab19
    OUString aOldBaseURL( INetURLObject::GetBaseURL() );
    sal_Bool bSet = sOrigFileName != aOldBaseURL;
    if( bSet )
        INetURLObject::SetBaseURL( sOrigFileName );

    OUString aRet( INetURLObject::AbsToRel( rValue ) );

    if( bSet )
        INetURLObject::SetBaseURL( aOldBaseURL );

    return aRet;
}

void SvXMLExport::StartElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSOutside )
{
    StartElement(pNamespaceMap->GetQNameByKey( nPrefix, GetXMLToken(eName) ), bIgnWSOutside);
}

void SvXMLExport::StartElement(const OUString& rName,
                        sal_Bool bIgnWSOutside )
{
    if ((mnErrorFlags & ERROR_DO_NOTHING) != ERROR_DO_NOTHING)
    {
        try
        {
            if( bIgnWSOutside && ((mnExportFlags & EXPORT_PRETTY) == EXPORT_PRETTY))
                xHandler->ignorableWhitespace( sWS );
            xHandler->startElement( rName, GetXAttrList() );
        }
        catch ( SAXInvalidCharacterException& e )
        {
            Sequence<OUString> aPars(1);
            aPars[0] = rName;
            SetError( XMLERROR_SAX|XMLERROR_FLAG_WARNING, aPars, e.Message, NULL );
        }
        catch ( SAXException& e )
        {
            Sequence<OUString> aPars(1);
            aPars[0] = rName;
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, NULL );
        }
    }
    ClearAttrList();
}

void SvXMLExport::Characters(const ::rtl::OUString& rChars)
{
    if ((mnErrorFlags & ERROR_DO_NOTHING) != ERROR_DO_NOTHING)
    {
        try
        {
            xHandler->characters(rChars);
        }
        catch ( SAXInvalidCharacterException& e )
        {
            Sequence<OUString> aPars(1);
            aPars[0] = rChars;
            SetError( XMLERROR_SAX|XMLERROR_FLAG_WARNING, aPars, e.Message, NULL );
        }
        catch ( SAXException& e )
        {
            Sequence<OUString> aPars(1);
            aPars[0] = rChars;
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, NULL );
        }
    }
}

void SvXMLExport::EndElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSInside )
{
    EndElement(pNamespaceMap->GetQNameByKey( nPrefix, GetXMLToken(eName) ), bIgnWSInside);
}

void SvXMLExport::EndElement(const OUString& rName,
                        sal_Bool bIgnWSInside )
{
    if ((mnErrorFlags & ERROR_DO_NOTHING) != ERROR_DO_NOTHING)
    {
        try
        {
            if( bIgnWSInside && ((mnExportFlags & EXPORT_PRETTY) == EXPORT_PRETTY))
                xHandler->ignorableWhitespace( sWS );
            xHandler->endElement( rName );
        }
        catch ( SAXException& e )
        {
            Sequence<OUString> aPars(1);
            aPars[0] = rName;
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, NULL );
        }
    }
}

void SvXMLExport::IgnorableWhitespace()
{
    if ((mnExportFlags & EXPORT_PRETTY) != EXPORT_PRETTY)
        return;

    if ((mnErrorFlags & ERROR_DO_NOTHING) != ERROR_DO_NOTHING)
    {
        try
        {
            xHandler->ignorableWhitespace( sWS );
        }
        catch ( SAXException& e )
        {
            Sequence<OUString> aPars(0);
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, NULL );
        }
    }
}


void SvXMLExport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams,
    const OUString& rExceptionMessage,
    const Reference<XLocator>& rLocator )
{
    // allow multi-threaded access to the cancel() method
    static ::vos::OMutex aMutex;
    ::vos::OGuard aGuard(aMutex);

    // maintain error flags
    if ( ( nId & XMLERROR_FLAG_ERROR ) != 0 )
        mnErrorFlags |= ERROR_ERROR_OCCURED;
    if ( ( nId & XMLERROR_FLAG_WARNING ) != 0 )
        mnErrorFlags |= ERROR_WARNING_OCCURED;
    if ( ( nId & XMLERROR_FLAG_SEVERE ) != 0 )
        mnErrorFlags |= ERROR_DO_NOTHING;

    // create error lsit on demand
    if ( pXMLErrors == NULL )
        pXMLErrors = new XMLErrors();

    // save error information
    pXMLErrors->AddRecord( nId, rMsgParams, rExceptionMessage, rLocator );
}

void SvXMLExport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams)
{
    OUString sEmpty;
    SetError( nId, rMsgParams, sEmpty, NULL );
}


XMLErrors* SvXMLExport::GetErrors()
{
    return pXMLErrors;
}

void SvXMLExport::DisposingModel()
{
    xModel.clear();
    mxEventListener.clear();
}

// #110680#
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SvXMLExport::getServiceFactory()
{
    // #110680#
    return mxServiceFactory;
}

//=============================================================================

void SvXMLElementExport::StartElement( SvXMLExport& rExp,
                                       sal_uInt16 nPrefixKey,
                                       const OUString& rLName,
                                       sal_Bool bIWSOutside )
{
    aName = rExp.GetNamespaceMap().GetQNameByKey(nPrefixKey, rLName);
    rExp.StartElement(aName, bIWSOutside);
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const sal_Char *pLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    OUString sLName( OUString::createFromAscii(pLName) );
    StartElement( rExp, nPrefixKey, sLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const OUString& rLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    StartElement( rExp, nPrefixKey, rLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        enum XMLTokenEnum eLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    StartElement( rExp, nPrefixKey, GetXMLToken(eLName), bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        const sal_Char *pLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
    {
        OUString sLName( OUString::createFromAscii(pLName) );
        StartElement( rExp, nPrefixKey, sLName, bIWSOutside );
    }
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        const OUString& rLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
        StartElement( rExp, nPrefixKey, rLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        enum XMLTokenEnum eLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
        StartElement( rExport, nPrefixKey, GetXMLToken(eLName), bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        const OUString& rQName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    aName = rQName;
    rExp.StartElement( rQName, bIWSOutside );
}

SvXMLElementExport::~SvXMLElementExport()
{
    if( bDoSomething )
    {
        rExport.EndElement( aName, bIgnWS );
    }
}


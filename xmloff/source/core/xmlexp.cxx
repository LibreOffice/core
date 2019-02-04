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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <stack>
#include <string.h>

#include <officecfg/Office/Common.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/xml/sax/SAXInvalidCharacterException.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/XMLSettingsExportContext.hxx>
#include <xmloff/families.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <XMLStarBasicExportHandler.hxx>
#include <XMLScriptExportHandler.hxx>
#include <xmloff/SettingsExportHelper.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/GradientStyle.hxx>
#include <xmloff/HatchStyle.hxx>
#include <xmloff/ImageStyle.hxx>
#include <TransGradientStyle.hxx>
#include <xmloff/MarkerStyle.hxx>
#include <xmloff/DashStyle.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <XMLImageMapExport.hxx>
#include <XMLBase64Export.hxx>
#include <xmloff/xmlerror.hxx>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <xmloff/XMLFilterServiceNames.h>
#include <xmloff/XMLEmbeddedObjectExportFilter.hxx>
#include <XMLBasicExportFilter.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/servicehelper.hxx>
#include <PropertySetMerger.hxx>

#include <svl/urihelper.hxx>
#include <xmloff/xformsexport.hxx>

#include <unotools/docinfohelper.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XMLOasisBasicExporter.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <RDFaExportHelper.hxx>

#include <comphelper/xmltools.hxx>
#include <comphelper/graphicmimetype.hxx>

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

sal_Char const sXML_1_2[] = "1.2";

#define XML_MODEL_SERVICE_WRITER    "com.sun.star.text.TextDocument"
#define XML_MODEL_SERVICE_CALC      "com.sun.star.sheet.SpreadsheetDocument"
#define XML_MODEL_SERVICE_DRAW      "com.sun.star.drawing.DrawingDocument"
#define XML_MODEL_SERVICE_IMPRESS   "com.sun.star.presentation.PresentationDocument"
#define XML_MODEL_SERVICE_MATH      "com.sun.star.formula.FormulaProperties"
#define XML_MODEL_SERVICE_CHART     "com.sun.star.chart.ChartDocument"

#define XML_USEPRETTYPRINTING       "UsePrettyPrinting"

#define XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE "vnd.sun.star.GraphicObject:"
#define XML_EMBEDDEDOBJECT_URL_BASE     "vnd.sun.star.EmbeddedObject:"

namespace {

struct XMLServiceMapEntry_Impl
{
    const sal_Char *sModelService;
    sal_Int32 const      nModelServiceLen;
    const sal_Char *sFilterService;
    sal_Int32 const      nFilterServiceLen;
};

}

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
    { nullptr, 0, nullptr, 0 }
};

class SettingsExportFacade : public ::xmloff::XMLSettingsExportContext
{
public:
    explicit SettingsExportFacade( SvXMLExport& i_rExport )
        :m_rExport( i_rExport )
    {
    }

    virtual ~SettingsExportFacade()
    {
    }

    virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                  const OUString& i_rValue ) override;
    virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                  enum ::xmloff::token::XMLTokenEnum i_eValue ) override;

    virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName ) override;
    virtual void    EndElement(   const bool i_bIgnoreWhitespace ) override;

    virtual void    Characters( const OUString& i_rCharacters ) override;

    virtual css::uno::Reference< css::uno::XComponentContext >
                    GetComponentContext() const override;
private:
    SvXMLExport&                    m_rExport;
    ::std::stack< OUString > m_aElements;
};

void SettingsExportFacade::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, const OUString& i_rValue )
{
    m_rExport.AddAttribute( XML_NAMESPACE_CONFIG, i_eName, i_rValue );
}

void SettingsExportFacade::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, enum ::xmloff::token::XMLTokenEnum i_eValue )
{
    m_rExport.AddAttribute( XML_NAMESPACE_CONFIG, i_eName, i_eValue );
}

void SettingsExportFacade::StartElement( enum ::xmloff::token::XMLTokenEnum i_eName )
{
    const OUString sElementName( m_rExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_CONFIG, GetXMLToken( i_eName ) ) );
    m_rExport.StartElement( sElementName, true/*i_bIgnoreWhitespace*/ );
    m_aElements.push( sElementName );
}

void SettingsExportFacade::EndElement( const bool i_bIgnoreWhitespace )
{
    const OUString sElementName( m_aElements.top() );
    m_rExport.EndElement( sElementName, i_bIgnoreWhitespace );
    m_aElements.pop();
}

void SettingsExportFacade::Characters( const OUString& i_rCharacters )
{
    m_rExport.GetDocHandler()->characters( i_rCharacters );
}

Reference< XComponentContext > SettingsExportFacade::GetComponentContext() const
{
    return m_rExport.getComponentContext();
}

class SvXMLExportEventListener : public cppu::WeakImplHelper<
                            css::lang::XEventListener >
{
private:
    SvXMLExport*    pExport;

public:
    explicit SvXMLExportEventListener(SvXMLExport* pExport);

                            // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& rEventObject) override;
};

SvXMLExportEventListener::SvXMLExportEventListener(SvXMLExport* pTempExport)
    : pExport(pTempExport)
{
}

// XEventListener
void SAL_CALL SvXMLExportEventListener::disposing( const lang::EventObject& )
{
    if (pExport)
    {
        pExport->DisposingModel();
        pExport = nullptr;
    }
}

class SvXMLExport_Impl
{
public:
    SvXMLExport_Impl();

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper  maInterfaceToIdentifierMapper;
    uno::Reference< uri::XUriReferenceFactory >         mxUriReferenceFactory;
    OUString                                            msPackageURI;
    OUString                                            msPackageURIScheme;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    bool                                                mbOutlineStyleAsNormalListStyle;
    bool                                                mbSaveBackwardCompatibleODF;

    uno::Reference< embed::XStorage >                   mxTargetStorage;

    SvtSaveOptions const                                maSaveOptions;

    /// name of stream in package, e.g., "content.xml"
    OUString mStreamName;

    OUString maSrcShellID;
    OUString maDestShellID;

    /// stack of backed up namespace maps
    /// long: depth at which namespace map has been backed up into the stack
    ::std::stack< ::std::pair< std::unique_ptr<SvXMLNamespaceMap>, long > > mNamespaceMaps;
    /// counts depth (number of open elements/start tags)
    long mDepth;

    ::std::unique_ptr< ::xmloff::RDFaExportHelper> mpRDFaHelper;

    bool                                                mbExportTextNumberElement;
    bool                                                mbNullDateInitialized;

    void SetSchemeOf( const OUString& rOrigFileName )
    {
        sal_Int32 nSep = rOrigFileName.indexOf(':');
        if( nSep != -1 )
            msPackageURIScheme = rOrigFileName.copy( 0, nSep );
    }
};

SvXMLExport_Impl::SvXMLExport_Impl()
:    mxUriReferenceFactory( uri::UriReferenceFactory::create(comphelper::getProcessComponentContext()) ),
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbOutlineStyleAsNormalListStyle( false ),
    mbSaveBackwardCompatibleODF( true ),
    mDepth( 0 ),
    mbExportTextNumberElement( false ),
    mbNullDateInitialized( false )
{
}

void SvXMLExport::SetDocHandler( const uno::Reference< xml::sax::XDocumentHandler > &rHandler )
{
    mxHandler = rHandler;
    mxExtHandler.set( mxHandler, UNO_QUERY );
}

void SvXMLExport::InitCtor_()
{
    // note: it is not necessary to add XML_NP_XML (it is declared implicitly)
    if( getExportFlags() & ~SvXMLExportFlags::OASIS )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    }
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::FONTDECLS) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );
    }
    if( getExportFlags() & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS|SvXMLExportFlags::SETTINGS) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( getExportFlags() & SvXMLExportFlags::SETTINGS )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( getExportFlags() & (SvXMLExportFlags::META|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_DC), GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_META), GetXMLToken(XML_N_META), XML_NAMESPACE_META );
    }
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::FONTDECLS) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    // namespaces for documents
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_DC),    GetXMLToken(XML_N_DC),      XML_NAMESPACE_DC );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_TEXT),  GetXMLToken(XML_N_TEXT),    XML_NAMESPACE_TEXT );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_DRAW),  GetXMLToken(XML_N_DRAW),    XML_NAMESPACE_DRAW );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_DR3D),  GetXMLToken(XML_N_DR3D),    XML_NAMESPACE_DR3D );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_SVG),   GetXMLToken(XML_N_SVG_COMPAT),  XML_NAMESPACE_SVG );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_CHART), GetXMLToken(XML_N_CHART),   XML_NAMESPACE_CHART );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_RPT),   GetXMLToken(XML_N_RPT),     XML_NAMESPACE_REPORT );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE),   XML_NAMESPACE_TABLE );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_NUMBER),GetXMLToken(XML_N_NUMBER),  XML_NAMESPACE_NUMBER );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_OOOW),  GetXMLToken(XML_N_OOOW),    XML_NAMESPACE_OOOW );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_OOOC),  GetXMLToken(XML_N_OOOC),    XML_NAMESPACE_OOOC );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_OF),    GetXMLToken(XML_N_OF),      XML_NAMESPACE_OF );

        if (getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        {
            mpNamespaceMap->Add(
                GetXMLToken(XML_NP_TABLE_EXT), GetXMLToken(XML_N_TABLE_EXT), XML_NAMESPACE_TABLE_EXT);
            mpNamespaceMap->Add(
                GetXMLToken(XML_NP_CALC_EXT), GetXMLToken(XML_N_CALC_EXT), XML_NAMESPACE_CALC_EXT);
            mpNamespaceMap->Add(
                GetXMLToken(XML_NP_DRAW_EXT), GetXMLToken(XML_N_DRAW_EXT), XML_NAMESPACE_DRAW_EXT);
            mpNamespaceMap->Add(
                GetXMLToken(XML_NP_LO_EXT), GetXMLToken(XML_N_LO_EXT),
                XML_NAMESPACE_LO_EXT);
            mpNamespaceMap->Add( GetXMLToken(XML_NP_FIELD), GetXMLToken(XML_N_FIELD), XML_NAMESPACE_FIELD );
        }
    }
    if( getExportFlags() & (SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_MATH), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    }
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_SCRIPT), GetXMLToken(XML_N_SCRIPT), XML_NAMESPACE_SCRIPT );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_DOM), GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
    }
    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_XFORMS_1_0), GetXMLToken(XML_N_XFORMS_1_0), XML_NAMESPACE_XFORMS );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_XSD), GetXMLToken(XML_N_XSD), XML_NAMESPACE_XSD );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_XSI), GetXMLToken(XML_N_XSI), XML_NAMESPACE_XSI );
        mpNamespaceMap->Add( GetXMLToken(XML_NP_FORMX), GetXMLToken(XML_N_FORMX), XML_NAMESPACE_FORMX );
    }

    // RDFa: needed for content and header/footer styles
    if( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_XHTML),
            GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
    }
    // GRDDL: to convert RDFa and meta.xml to RDF
    if( getExportFlags() & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        mpNamespaceMap->Add( GetXMLToken(XML_NP_GRDDL),
            GetXMLToken(XML_N_GRDDL), XML_NAMESPACE_GRDDL );
    }
    // CSS Text Level 3 for distributed text justification.
    if ( getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES) )
    {
        mpNamespaceMap->Add(
            GetXMLToken(XML_NP_CSS3TEXT), GetXMLToken(XML_N_CSS3TEXT), XML_NAMESPACE_CSS3TEXT );
    }

    if (mxModel.is() && !mxEventListener.is())
    {
        mxEventListener.set( new SvXMLExportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }

    // Determine model type (#i51726#)
    DetermineModelType_();

    // cl: but only if we do export to current oasis format, old openoffice format *must* always be compatible
    if( getExportFlags() & SvXMLExportFlags::OASIS )
    {
        mpImpl->mbSaveBackwardCompatibleODF =
            officecfg::Office::Common::Save::Document::
            SaveBackwardCompatibleODF::get( comphelper::getProcessComponentContext() );
    }
}

// Shapes in Writer cannot be named via context menu (#i51726#)
void SvXMLExport::DetermineModelType_()
{
    meModelType = SvtModuleOptions::EFactory::UNKNOWN_FACTORY;

    if ( mxModel.is() )
    {
        meModelType = SvtModuleOptions::ClassifyFactoryByModel( mxModel );
    }
}

SvXMLExport::SvXMLExport(
    sal_Int16 const eDefaultMeasureUnit /*css::util::MeasureUnit*/,
    const uno::Reference< uno::XComponentContext >& xContext,
    OUString const & implementationName,
    const enum XMLTokenEnum eClass, SvXMLExportFlags nExportFlags )
:   mpImpl( new SvXMLExport_Impl ),
    m_xContext(xContext), m_implementationName(implementationName),
    mxAttrList( new SvXMLAttributeList ),
    mpNamespaceMap( new SvXMLNamespaceMap ),
    maUnitConv( xContext, util::MeasureUnit::MM_100TH, eDefaultMeasureUnit ),
    meClass( eClass ),
    mnExportFlags( nExportFlags ),
    mnErrorFlags( SvXMLErrorFlags::NO ),
    msWS( GetXMLToken(XML_WS) ),
    mbSaveLinkedSections(true),
    mbAutoStylesCollected(false)
{
    SAL_WARN_IF( !xContext.is(), "xmloff.core", "got no service manager" );
    InitCtor_();
}

SvXMLExport::SvXMLExport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName,
    const OUString &rFileName,
    sal_Int16 const eDefaultMeasureUnit /*css::util::MeasureUnit*/,
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler)
:   mpImpl( new SvXMLExport_Impl ),
    m_xContext(xContext), m_implementationName(implementationName),
    mxHandler( rHandler ),
    mxExtHandler( rHandler, uno::UNO_QUERY ),
    mxAttrList( new SvXMLAttributeList ),
    msOrigFileName( rFileName ),
    mpNamespaceMap( new SvXMLNamespaceMap ),
    maUnitConv( xContext, util::MeasureUnit::MM_100TH, eDefaultMeasureUnit ),
    meClass( XML_TOKEN_INVALID ),
    mnExportFlags( SvXMLExportFlags::NONE ),
    mnErrorFlags( SvXMLErrorFlags::NO ),
    msWS( GetXMLToken(XML_WS) ),
    mbSaveLinkedSections(true),
    mbAutoStylesCollected(false)
{
    SAL_WARN_IF( !xContext.is(), "xmloff.core", "got no service manager" );
    mpImpl->SetSchemeOf( msOrigFileName );
    InitCtor_();

    if (mxNumberFormatsSupplier.is())
        mpNumExport.reset( new SvXMLNumFmtExport(*this, mxNumberFormatsSupplier) );
}

SvXMLExport::SvXMLExport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName,
    const OUString &rFileName,
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    const Reference< XModel >& rModel,
    FieldUnit const eDefaultFieldUnit,
    SvXMLExportFlags nExportFlag)
:   mpImpl( new SvXMLExport_Impl ),
    m_xContext(xContext), m_implementationName(implementationName),
    mxModel( rModel ),
    mxHandler( rHandler ),
    mxExtHandler( rHandler, uno::UNO_QUERY ),
    mxNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    mxAttrList( new SvXMLAttributeList ),
    msOrigFileName( rFileName ),
    mpNamespaceMap( new SvXMLNamespaceMap ),
    maUnitConv( xContext,
                util::MeasureUnit::MM_100TH,
                SvXMLUnitConverter::GetMeasureUnit(eDefaultFieldUnit) ),
    meClass( XML_TOKEN_INVALID ),
    mnExportFlags( nExportFlag ),
    mnErrorFlags( SvXMLErrorFlags::NO ),
    msWS( GetXMLToken(XML_WS) ),
    mbSaveLinkedSections(true),
    mbAutoStylesCollected(false)
{
    SAL_WARN_IF(!xContext.is(), "xmloff.core", "got no service manager" );
    mpImpl->SetSchemeOf( msOrigFileName );
    InitCtor_();

    if (mxNumberFormatsSupplier.is())
        mpNumExport.reset( new SvXMLNumFmtExport(*this, mxNumberFormatsSupplier) );
}

SvXMLExport::~SvXMLExport()
{
    mpXMLErrors.reset();
    mpImageMapExport.reset();
    mpEventExport.reset();
    mpNamespaceMap.reset();
    if (mpProgressBarHelper || mpNumExport)
    {
        if (mxExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                if (mpProgressBarHelper)
                {
                    OUString sProgressMax(XML_PROGRESSMAX);
                    OUString sProgressCurrent(XML_PROGRESSCURRENT);
                    OUString sRepeat(XML_PROGRESSREPEAT);
                    if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                        xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                    {
                        sal_Int32 nProgressMax(mpProgressBarHelper->GetReference());
                        sal_Int32 nProgressCurrent(mpProgressBarHelper->GetValue());
                        mxExportInfo->setPropertyValue(sProgressMax, uno::Any(nProgressMax));
                        mxExportInfo->setPropertyValue(sProgressCurrent, uno::Any(nProgressCurrent));
                    }
                    if (xPropertySetInfo->hasPropertyByName(sRepeat))
                        mxExportInfo->setPropertyValue(sRepeat, css::uno::makeAny(mpProgressBarHelper->GetRepeat()));
                }
                if (mpNumExport && (mnExportFlags & (SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::STYLES)))
                {
                    OUString sWrittenNumberFormats(XML_WRITTENNUMBERSTYLES);
                    if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
                    {
                        mxExportInfo->setPropertyValue(sWrittenNumberFormats, Any(mpNumExport->GetWasUsed()));
                    }
                }
            }
        }
        mpProgressBarHelper.reset();
        mpNumExport.reset();
    }

    if (mxEventListener.is() && mxModel.is())
        mxModel->removeEventListener(mxEventListener);
}

// XExporter
void SAL_CALL SvXMLExport::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
{
    mxModel.set( xDoc, UNO_QUERY );
    if( !mxModel.is() )
        throw lang::IllegalArgumentException();
    if (mxModel.is() && ! mxEventListener.is())
    {
        mxEventListener.set( new SvXMLExportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }

    if(!mxNumberFormatsSupplier.is() )
    {
        mxNumberFormatsSupplier.set(mxModel, css::uno::UNO_QUERY);
        if(mxNumberFormatsSupplier.is() && mxHandler.is())
            mpNumExport.reset( new SvXMLNumFmtExport(*this, mxNumberFormatsSupplier) );
    }
    if (mxExportInfo.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxExportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            OUString sUsePrettyPrinting(XML_USEPRETTYPRINTING);
            if (xPropertySetInfo->hasPropertyByName(sUsePrettyPrinting))
            {
                uno::Any aAny = mxExportInfo->getPropertyValue(sUsePrettyPrinting);
                if (::cppu::any2bool(aAny))
                    mnExportFlags |= SvXMLExportFlags::PRETTY;
                else
                    mnExportFlags &= ~SvXMLExportFlags::PRETTY;
            }

            if (mpNumExport && (mnExportFlags & (SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::STYLES)))
            {
                OUString sWrittenNumberFormats(XML_WRITTENNUMBERSTYLES);
                if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
                {
                    uno::Any aAny = mxExportInfo->getPropertyValue(sWrittenNumberFormats);
                    uno::Sequence<sal_Int32> aWasUsed;
                    if(aAny >>= aWasUsed)
                        mpNumExport->SetWasUsed(aWasUsed);
                }
            }
        }
    }

    if ( mpImpl->mbSaveBackwardCompatibleODF )
        mnExportFlags |= SvXMLExportFlags::SAVEBACKWARDCOMPATIBLE;
    else
        mnExportFlags &= ~SvXMLExportFlags::SAVEBACKWARDCOMPATIBLE;

    // namespaces for user defined attributes
    Reference< XMultiServiceFactory > xFactory( mxModel,    UNO_QUERY );
    if( xFactory.is() )
    {
        try
        {
            Reference < XInterface > xIfc =
                xFactory->createInstance("com.sun.star.xml.NamespaceMap");
            if( xIfc.is() )
            {
                Reference< XNameAccess > xNamespaceMap( xIfc, UNO_QUERY );
                if( xNamespaceMap.is() )
                {
                    Sequence< OUString > aPrefixes( xNamespaceMap->getElementNames() );
                    for( OUString const & prefix : aPrefixes )
                    {
                        OUString aURL;
                        if( xNamespaceMap->getByName( prefix ) >>= aURL )
                            GetNamespaceMap_().Add( prefix, aURL );
                    }
                }
            }
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    // Determine model type (#i51726#)
    DetermineModelType_();
}

// XInitialize
void SAL_CALL SvXMLExport::initialize( const uno::Sequence< uno::Any >& aArguments )
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
            mxStatusIndicator = xTmpStatus;

        // graphic storage handler
        uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler(xValue, UNO_QUERY);
        if (xGraphicStorageHandler.is())
            mxGraphicStorageHandler = xGraphicStorageHandler;

        // object resolver
        uno::Reference<document::XEmbeddedObjectResolver> xTmpObjectResolver(
            xValue, UNO_QUERY );
        if ( xTmpObjectResolver.is() )
            mxEmbeddedResolver = xTmpObjectResolver;

        // document handler
        uno::Reference<xml::sax::XDocumentHandler> xTmpDocHandler(
            xValue, UNO_QUERY );
        if( xTmpDocHandler.is() )
        {
            mxHandler = xTmpDocHandler;
            *pAny >>= mxExtHandler;

            if (mxNumberFormatsSupplier.is() && mpNumExport == nullptr)
                mpNumExport.reset( new SvXMLNumFmtExport(*this, mxNumberFormatsSupplier) );
        }

        // property set to transport data across
        uno::Reference<beans::XPropertySet> xTmpPropertySet(
            xValue, UNO_QUERY );
        if( xTmpPropertySet.is() )
            mxExportInfo = xTmpPropertySet;
    }

    if( !mxExportInfo.is() )
        return;

    uno::Reference< beans::XPropertySetInfo > xPropertySetInfo =
        mxExportInfo->getPropertySetInfo();
    OUString sPropName(
            "BaseURI"  );
    if( xPropertySetInfo->hasPropertyByName(sPropName) )
    {
        uno::Any aAny = mxExportInfo->getPropertyValue(sPropName);
        aAny >>= msOrigFileName;
        mpImpl->msPackageURI = msOrigFileName;
        mpImpl->SetSchemeOf( msOrigFileName );
    }
    OUString sRelPath;
    sPropName = "StreamRelPath";
    if( xPropertySetInfo->hasPropertyByName(sPropName) )
    {
        uno::Any aAny = mxExportInfo->getPropertyValue(sPropName);
        aAny >>= sRelPath;
    }
    OUString sName;
    sPropName = "StreamName";
    if( xPropertySetInfo->hasPropertyByName(sPropName) )
    {
        uno::Any aAny = mxExportInfo->getPropertyValue(sPropName);
        aAny >>= sName;
    }
    if( !msOrigFileName.isEmpty() && !sName.isEmpty() )
    {
        INetURLObject aBaseURL( msOrigFileName );
        if( !sRelPath.isEmpty() )
            aBaseURL.insertName( sRelPath );
        aBaseURL.insertName( sName );
        msOrigFileName = aBaseURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
    }
    mpImpl->mStreamName = sName; // Note: may be empty (XSLT)

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    const OUString sOutlineStyleAsNormalListStyle(
            "OutlineStyleAsNormalListStyle" );
    if( xPropertySetInfo->hasPropertyByName( sOutlineStyleAsNormalListStyle ) )
    {
        uno::Any aAny = mxExportInfo->getPropertyValue( sOutlineStyleAsNormalListStyle );
        aAny >>= mpImpl->mbOutlineStyleAsNormalListStyle;
    }

    OUString sTargetStorage( "TargetStorage" );
    if( xPropertySetInfo->hasPropertyByName( sTargetStorage ) )
        mxExportInfo->getPropertyValue( sTargetStorage ) >>= mpImpl->mxTargetStorage;

    const OUString sExportTextNumberElement(
            "ExportTextNumberElement" );
    if( xPropertySetInfo->hasPropertyByName( sExportTextNumberElement ) )
    {
        uno::Any aAny = mxExportInfo->getPropertyValue( sExportTextNumberElement );
        aAny >>= mpImpl->mbExportTextNumberElement;
    }


}

// XFilter
sal_Bool SAL_CALL SvXMLExport::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
{
    // check for xHandler first... should have been supplied in initialize
    if( !mxHandler.is() )
        return false;

    try
    {
        const sal_Int32 nPropCount = aDescriptor.getLength();

        const SvXMLExportFlags nTest =
            SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SETTINGS;
        if( (mnExportFlags & nTest) == nTest && msOrigFileName.isEmpty() )
        {
            // evaluate descriptor only for flat files and if a base URI
            // has not been provided already

            const beans::PropertyValue* pProps = aDescriptor.getConstArray();
            for( sal_Int32 nIndex = 0; nIndex < nPropCount; nIndex++, pProps++ )
            {
                const OUString& rPropName = pProps->Name;
                const Any& rValue = pProps->Value;

                if ( rPropName == "FileName" )
                {
                    if( !(rValue >>= msOrigFileName ) )
                        return false;
                }
                else if ( rPropName == "FilterName" )
                {
                    if( !(rValue >>= msFilterName ) )
                        return false;
                }
            }
        }

        const beans::PropertyValue* pProps = aDescriptor.getConstArray();
        for (sal_Int32 nIndex = 0; nIndex < nPropCount; ++nIndex, ++pProps)
        {
            const OUString& rPropName = pProps->Name;
            const Any& rValue = pProps->Value;

            if (rPropName == "SourceShellID")
            {
                if (!(rValue >>= mpImpl->maSrcShellID))
                    return false;
            }
            else if (rPropName == "DestinationShellID")
            {
                if (!(rValue >>= mpImpl->maDestShellID))
                    return false;
            }
            else if( rPropName == "ImageFilter")
            {
                if (!(rValue >>= msImgFilterName))
                    return false;
            }
        }


        exportDoc( meClass );
    }
    catch(const uno::Exception& e)
    {
        // We must catch exceptions, because according to the
        // API definition export must not throw one!
        css::uno::Any ex(cppu::getCaughtException());
        OUString sMessage( ex.getValueTypeName() + ": \"" + e.Message + "\"");
        if (e.Context.is())
        {
            const char* pContext = typeid(*e.Context.get()).name();
            sMessage += " (context: " + OUString::createFromAscii(pContext) + " )";
        }
        SetError( XMLERROR_FLAG_ERROR | XMLERROR_FLAG_SEVERE | XMLERROR_API,
                  Sequence<OUString>(), sMessage, nullptr );
    }

    // return true only if no error occurred
    return (mnErrorFlags & (SvXMLErrorFlags::DO_NOTHING|SvXMLErrorFlags::ERROR_OCCURRED)) == SvXMLErrorFlags::NO;
}

void SAL_CALL SvXMLExport::cancel()
{
    // stop export
    Sequence<OUString> aEmptySeq;
    SetError(XMLERROR_CANCEL|XMLERROR_FLAG_SEVERE, aEmptySeq);
}

OUString SAL_CALL SvXMLExport::getName(  )
{
    return msFilterName;
}

void SAL_CALL SvXMLExport::setName( const OUString& )
{
    // do nothing, because it is not possible to set the FilterName
}

// XServiceInfo
OUString SAL_CALL SvXMLExport::getImplementationName(  )
{
    return m_implementationName;
}

sal_Bool SAL_CALL SvXMLExport::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SvXMLExport::getSupportedServiceNames(  )
{
    uno::Sequence<OUString> aSeq(2);
    aSeq[0] = "com.sun.star.document.ExportFilter";
    aSeq[1] = "com.sun.star.xml.XMLExportFilter";
    return aSeq;
}

OUString
SvXMLExport::EnsureNamespace(OUString const & i_rNamespace)
{
    OUString const aPreferredPrefix("gen");
    OUString sPrefix;
    sal_uInt16 nKey( GetNamespaceMap_().GetKeyByName( i_rNamespace ) );
    if( XML_NAMESPACE_UNKNOWN == nKey )
    {
        // There is no prefix for the namespace, so
        // we have to generate one and have to add it.
        sPrefix = aPreferredPrefix;
        nKey = GetNamespaceMap_().GetKeyByPrefix( sPrefix );
        sal_Int32 n( 0 );
        OUStringBuffer buf;
        while( nKey != USHRT_MAX )
        {
            buf.append( aPreferredPrefix );
            buf.append( ++n );
            sPrefix = buf.makeStringAndClear();
            nKey = GetNamespaceMap_().GetKeyByPrefix( sPrefix );
        }

        if (mpImpl->mNamespaceMaps.empty()
            || (mpImpl->mNamespaceMaps.top().second != mpImpl->mDepth))
        {
            // top was created for lower depth... need a new namespace map!
            auto pNew = new SvXMLNamespaceMap( *mpNamespaceMap );
            mpImpl->mNamespaceMaps.push(
                ::std::make_pair(std::move(mpNamespaceMap), mpImpl->mDepth) );
            mpNamespaceMap.reset( pNew );
        }

        // add the namespace to the map and as attribute
        mpNamespaceMap->Add( sPrefix, i_rNamespace );
        buf.append( GetXMLToken(XML_XMLNS) );
        buf.append( ':' );
        buf.append( sPrefix );
        AddAttribute( buf.makeStringAndClear(), i_rNamespace );
    }
    else
    {
        // If there is a prefix for the namespace, reuse that.
        sPrefix = GetNamespaceMap_().GetPrefixByKey( nKey );
    }
    return sPrefix;
}

void SvXMLExport::AddAttributeASCII( sal_uInt16 nPrefixKey,
                                     const sal_Char *pName,
                                       const sal_Char *pValue )
{
    OUString sName( OUString::createFromAscii( pName ) );
    OUString sValue( OUString::createFromAscii( pValue ) );

    mxAttrList->AddAttribute(
        GetNamespaceMap_().GetQNameByKey( nPrefixKey, sName ), sValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const sal_Char *pName,
                              const OUString& rValue )
{
    OUString sName( OUString::createFromAscii( pName ) );

    mxAttrList->AddAttribute(
        GetNamespaceMap_().GetQNameByKey( nPrefixKey, sName ), rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const OUString& rName,
                              const OUString& rValue )
{
    mxAttrList->AddAttribute(
        GetNamespaceMap_().GetQNameByKey( nPrefixKey, rName ), rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                const OUString& rValue )
{
    mxAttrList->AddAttribute(
        GetNamespaceMap_().GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                enum XMLTokenEnum eValue)
{
    mxAttrList->AddAttribute(
        GetNamespaceMap_().GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        GetXMLToken(eValue) );
}

void SvXMLExport::AddAttribute( const OUString& rQName,
                                const OUString& rValue )
{
      mxAttrList->AddAttribute(
        rQName,
        rValue );
}

void SvXMLExport::AddAttribute( const OUString& rQName,
                                enum ::xmloff::token::XMLTokenEnum eValue )
{
      mxAttrList->AddAttribute(
        rQName,
        GetXMLToken(eValue) );
}

void SvXMLExport::AddLanguageTagAttributes( sal_uInt16 nPrefix, sal_uInt16 nPrefixRfc,
        const css::lang::Locale& rLocale, bool bWriteEmpty )
{
    if (rLocale.Variant.isEmpty())
    {
        // Per convention The BCP 47 string is always stored in Variant, if
        // that is empty we have a plain language-country combination, no need
        // to convert to LanguageTag first. Also catches the case of empty
        // locale denoting system locale.
        xmloff::token::XMLTokenEnum eLanguage, eCountry;
        eLanguage = XML_LANGUAGE;
        eCountry  = XML_COUNTRY;
        if (bWriteEmpty || !rLocale.Language.isEmpty())
            AddAttribute( nPrefix, eLanguage, rLocale.Language);
        if (bWriteEmpty || !rLocale.Country.isEmpty())
            AddAttribute( nPrefix, eCountry, rLocale.Country);
    }
    else
    {
        LanguageTag aLanguageTag( rLocale);
        AddLanguageTagAttributes( nPrefix, nPrefixRfc, aLanguageTag, bWriteEmpty);
    }
}

void SvXMLExport::AddLanguageTagAttributes( sal_uInt16 nPrefix, sal_uInt16 nPrefixRfc,
        const LanguageTag& rLanguageTag, bool bWriteEmpty )
{
    if (rLanguageTag.isIsoODF())
    {
        if (bWriteEmpty || !rLanguageTag.isSystemLocale())
        {
            AddAttribute( nPrefix, XML_LANGUAGE, rLanguageTag.getLanguage());
            if (rLanguageTag.hasScript() && getDefaultVersion() >= SvtSaveOptions::ODFVER_012)
                AddAttribute( nPrefix, XML_SCRIPT, rLanguageTag.getScript());
            if (bWriteEmpty || !rLanguageTag.getCountry().isEmpty())
                AddAttribute( nPrefix, XML_COUNTRY, rLanguageTag.getCountry());
        }
    }
    else
    {
        if (getDefaultVersion() >= SvtSaveOptions::ODFVER_012)
            AddAttribute( nPrefixRfc, XML_RFC_LANGUAGE_TAG, rLanguageTag.getBcp47());
        // Also in case of non-pure-ISO tag store best matching fo: attributes
        // for consumers not handling *:rfc-language-tag, ensuring that only
        // valid ISO codes are stored. Here the bWriteEmpty parameter has no
        // meaning.
        OUString aLanguage, aScript, aCountry;
        rLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, aCountry);
        if (!aLanguage.isEmpty())
        {
            AddAttribute( nPrefix, XML_LANGUAGE, aLanguage);
            if (!aScript.isEmpty() && getDefaultVersion() >= SvtSaveOptions::ODFVER_012)
                AddAttribute( nPrefix, XML_SCRIPT, aScript);
            if (!aCountry.isEmpty())
                AddAttribute( nPrefix, XML_COUNTRY, aCountry);
        }
    }
}

void SvXMLExport::AddAttributeList( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( xAttrList.is())
        mxAttrList->AppendAttributeList( xAttrList );
}

void SvXMLExport::ClearAttrList()
{
    mxAttrList->Clear();
}

#ifdef DBG_UTIL
void SvXMLExport::CheckAttrList()
{
    SAL_WARN_IF( mxAttrList->getLength(), "xmloff.core", "XMLExport::CheckAttrList: list is not empty" );
}
#endif

void SvXMLExport::ImplExportMeta()
{
    CheckAttrList();

    ExportMeta_();
}

void SvXMLExport::ImplExportSettings()
{
    CheckAttrList();

    ::std::vector< SettingsGroup > aSettings;
    sal_Int32 nSettingsCount = 0;

    // view settings
    uno::Sequence< beans::PropertyValue > aViewSettings;
    GetViewSettingsAndViews( aViewSettings );
    aSettings.emplace_back( XML_VIEW_SETTINGS, aViewSettings );
    nSettingsCount += aViewSettings.getLength();

    // configuration settings
    uno::Sequence<beans::PropertyValue> aConfigSettings;
    GetConfigurationSettings( aConfigSettings );
    aSettings.emplace_back( XML_CONFIGURATION_SETTINGS, aConfigSettings );
    nSettingsCount += aConfigSettings.getLength();

    // any document specific settings
    nSettingsCount += GetDocumentSpecificSettings( aSettings );

    {
        SvXMLElementExport aElem( *this,
                                nSettingsCount != 0,
                                XML_NAMESPACE_OFFICE, XML_SETTINGS,
                                true, true );

        SettingsExportFacade aSettingsExportContext( *this );
        XMLSettingsExportHelper aSettingsExportHelper( aSettingsExportContext );

        for (auto const& settings : aSettings)
        {
            if ( !settings.aSettings.getLength() )
                continue;

            const OUString& sSettingsName( GetXMLToken( settings.eGroupName ) );
            OUString sQName = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OOO, sSettingsName );
            aSettingsExportHelper.exportAllSettings( settings.aSettings, sQName );
        }
    }
}

void SvXMLExport::ImplExportStyles()
{
    CheckAttrList();

    {
        // <style:styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_STYLES,
                                true, true );

        ExportStyles_( false );
    }

    // transfer style names (+ families) TO other components (if appropriate)
    if( ( !( mnExportFlags & SvXMLExportFlags::CONTENT ) ) && mxExportInfo.is() )
    {
        static OUString sStyleNames( "StyleNames" );
        static OUString sStyleFamilies( "StyleFamilies" );
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxExportInfo->getPropertySetInfo();
        if ( xPropertySetInfo->hasPropertyByName( sStyleNames ) && xPropertySetInfo->hasPropertyByName( sStyleFamilies ) )
        {
            Sequence<sal_Int32> aStyleFamilies;
            Sequence<OUString> aStyleNames;
            mxAutoStylePool->GetRegisteredNames( aStyleFamilies, aStyleNames );
            mxExportInfo->setPropertyValue( sStyleNames, makeAny( aStyleNames ) );
            mxExportInfo->setPropertyValue( sStyleFamilies,
                                           makeAny( aStyleFamilies ) );
        }
    }
}

void SvXMLExport::ImplExportAutoStyles()
{
    // transfer style names (+ families) FROM other components (if appropriate)
    OUString sStyleNames( "StyleNames" );
    OUString sStyleFamilies( "StyleFamilies" );
    if( ( !( mnExportFlags & SvXMLExportFlags::STYLES ) )
        && mxExportInfo.is()
        && mxExportInfo->getPropertySetInfo()->hasPropertyByName( sStyleNames )
        && mxExportInfo->getPropertySetInfo()->hasPropertyByName( sStyleFamilies ) )
    {
        Sequence<sal_Int32> aStyleFamilies;
        mxExportInfo->getPropertyValue( sStyleFamilies ) >>= aStyleFamilies;
        Sequence<OUString> aStyleNames;
        mxExportInfo->getPropertyValue( sStyleNames ) >>= aStyleNames;
        mxAutoStylePool->RegisterNames( aStyleFamilies, aStyleNames );
    }

    {
        // <style:automatic-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  XML_AUTOMATIC_STYLES, true, true );

        ExportAutoStyles_();
    }
}

void SvXMLExport::ImplExportMasterStyles()
{
    // <style:master-styles>
    SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,
                            true, true );

    ExportMasterStyles_();
}

void SvXMLExport::ImplExportContent()
{
    CheckAttrList();

    CheckAttrList();

    {
        SvXMLElementExport aElement( *this, XML_NAMESPACE_OFFICE, XML_BODY,
                                  true, true );
        {
            XMLTokenEnum eClass = meClass;
            if( XML_TEXT_GLOBAL == eClass )
            {
                AddAttribute( XML_NAMESPACE_TEXT, XML_GLOBAL,
                      GetXMLToken( XML_TRUE ) );
                eClass = XML_TEXT;
            }
            if ( XML_GRAPHICS == eClass )
                eClass = XML_DRAWING;
            // <office:body ...>
            SetBodyAttributes();
            SvXMLElementExport aElem( *this, meClass != XML_TOKEN_INVALID,
                                      XML_NAMESPACE_OFFICE, eClass,
                                        true, true );

            ExportContent_();
        }
    }
}

void SvXMLExport::SetBodyAttributes()
{
}

static void
lcl_AddGrddl(SvXMLExport const & rExport, const SvXMLExportFlags /*nExportMode*/)
{
    // check version >= 1.2
    switch (rExport.getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall through
        case SvtSaveOptions::ODFVER_010: return;
        default: break;
    }

    // #i115030#: disabled, the XSLT is not finished, and not available via HTTP
#if 0
    if (SvXMLExportFlags::SETTINGS != nExportMode) // meta, content, styles
    {
        rExport.AddAttribute( XML_NAMESPACE_GRDDL, XML_TRANSFORMATION,
            OUString("http://FIXME") );
    }
#endif
}

void SvXMLExport::addChaffWhenEncryptedStorage()
{
    uno::Reference< embed::XEncryptionProtectedSource2 > xEncr(mpImpl->mxTargetStorage, uno::UNO_QUERY);

    if (xEncr.is() && xEncr->hasEncryptionData() && mxExtHandler.is())
    {
        mxExtHandler->comment(OStringToOUString(comphelper::xml::makeXMLChaff(), RTL_TEXTENCODING_ASCII_US));
    }
}

ErrCode SvXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    bool bOwnGraphicResolver = false;
    bool bOwnEmbeddedResolver = false;

    if (!mxGraphicStorageHandler.is() || !mxEmbeddedResolver.is())
    {
        Reference< XMultiServiceFactory > xFactory( mxModel,    UNO_QUERY );
        if( xFactory.is() )
        {
            try
            {
                if (!mxGraphicStorageHandler.is())
                {
                    mxGraphicStorageHandler.set(xFactory->createInstance( "com.sun.star.document.ExportGraphicStorageHandler"), UNO_QUERY);
                    bOwnGraphicResolver = mxGraphicStorageHandler.is();
                }

                if( !mxEmbeddedResolver.is() )
                {
                    mxEmbeddedResolver.set(
                        xFactory->createInstance( "com.sun.star.document.ExportEmbeddedObjectResolver" ), UNO_QUERY);
                    bOwnEmbeddedResolver = mxEmbeddedResolver.is();
                }
            }
            catch(const css::uno::Exception&)
            {
            }
        }
    }
    if( (getExportFlags() & SvXMLExportFlags::OASIS) == SvXMLExportFlags::NONE )
    {
        try
        {
            static ::comphelper::PropertyMapEntry const aInfoMap[] =
            {
                { OUString("Class"), 0,
                    ::cppu::UnoType<OUString>::get(),
                      PropertyAttribute::MAYBEVOID, 0},
                { OUString(), 0, css::uno::Type(), 0, 0 }
            };
            Reference< XPropertySet > xConvPropSet(
                ::comphelper::GenericPropertySet_CreateInstance(
                        new ::comphelper::PropertySetInfo( aInfoMap ) ) );

            xConvPropSet->setPropertyValue( "Class", Any(GetXMLToken( eClass )) );

            Reference< XPropertySet > xPropSet =
                mxExportInfo.is()
                ?  PropertySetMerger_CreateInstance( mxExportInfo,
                                                     xConvPropSet )
                : xConvPropSet;

            Sequence<Any> aArgs( 3 );
            aArgs[0] <<= mxHandler;
            aArgs[1] <<= xPropSet;
            aArgs[2] <<= mxModel;

            // get filter component
            Reference< xml::sax::XDocumentHandler > xTmpDocHandler(
                m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.comp.Oasis2OOoTransformer", aArgs, m_xContext),
                UNO_QUERY);
            SAL_WARN_IF(!xTmpDocHandler.is(), "xmloff.core", "can't instantiate OASIS transformer component" );
            if( xTmpDocHandler.is() )
            {
                mxHandler = xTmpDocHandler;
                mxExtHandler.set( mxHandler, UNO_QUERY );
            }
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    mxHandler->startDocument();

    addChaffWhenEncryptedStorage();

    // <office:document ...>
    CheckAttrList();

    // namespace attributes
    // ( The namespace decls should be first attributes in the element;
    //   some faulty XML parsers (JAXP1.1) have a problem with this,
    //   also it's more elegant )
    sal_uInt16 nPos = mpNamespaceMap->GetFirstKey();
    while( USHRT_MAX != nPos )
    {
        mxAttrList->AddAttribute( mpNamespaceMap->GetAttrNameByKey( nPos ),
                                  mpNamespaceMap->GetNameByKey( nPos ) );
        nPos = mpNamespaceMap->GetNextKey( nPos );
    }

    // office:version = ...
    const sal_Char* pVersion = nullptr;
    switch (getDefaultVersion())
    {
    case SvtSaveOptions::ODFVER_LATEST: pVersion = sXML_1_2; break;
    case SvtSaveOptions::ODFVER_012_EXT_COMPAT: pVersion = sXML_1_2; break;
    case SvtSaveOptions::ODFVER_012: pVersion = sXML_1_2; break;
    case SvtSaveOptions::ODFVER_011: pVersion = "1.1"; break;
    case SvtSaveOptions::ODFVER_010: break;

    default:
        SAL_WARN("xmloff.core", "xmloff::SvXMLExport::exportDoc(), unexpected odf default version!");
    }

    if (pVersion)
    {
        AddAttribute( XML_NAMESPACE_OFFICE, XML_VERSION,
                          OUString::createFromAscii(pVersion) );
    }

    {
        enum XMLTokenEnum eRootService = XML_TOKEN_INVALID;
        const SvXMLExportFlags nExportMode = mnExportFlags & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SETTINGS);

        lcl_AddGrddl(*this, nExportMode);

        if( SvXMLExportFlags::META == nExportMode )
        {
            // export only meta
            eRootService = XML_DOCUMENT_META;
        }
        else if ( SvXMLExportFlags::SETTINGS == nExportMode )
        {
            // export only settings
            eRootService = XML_DOCUMENT_SETTINGS;
        }
        else if( SvXMLExportFlags::STYLES == nExportMode )
        {
            // export only styles
            eRootService = XML_DOCUMENT_STYLES;
        }
        else if( SvXMLExportFlags::CONTENT == nExportMode )
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
                OUString aTmp( "application/vnd.oasis.opendocument." );
                aTmp += GetXMLToken( eClass );
                AddAttribute( XML_NAMESPACE_OFFICE, XML_MIMETYPE, aTmp );
            }
        }

        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, eRootService, true, true );

        // meta information
        if( mnExportFlags & SvXMLExportFlags::META )
            ImplExportMeta();

        // settings
        if( mnExportFlags & SvXMLExportFlags::SETTINGS )
            ImplExportSettings();

        // scripts
        if( mnExportFlags & SvXMLExportFlags::SCRIPTS )
            ExportScripts_();

        // font declarations
        if( mnExportFlags & SvXMLExportFlags::FONTDECLS )
            ExportFontDecls_();

        // styles
        if( mnExportFlags & SvXMLExportFlags::STYLES )
            ImplExportStyles();

        // autostyles
        if( mnExportFlags & SvXMLExportFlags::AUTOSTYLES )
            ImplExportAutoStyles();

        // masterstyles
        if( mnExportFlags & SvXMLExportFlags::MASTERSTYLES )
            ImplExportMasterStyles();

        // content
        if( mnExportFlags & SvXMLExportFlags::CONTENT )
            ImplExportContent();
    }

    mxHandler->endDocument();

    if( bOwnGraphicResolver )
    {
        uno::Reference<XComponent> xComp(mxGraphicStorageHandler, UNO_QUERY);
        xComp->dispose();
    }

    if( bOwnEmbeddedResolver )
    {
        Reference< XComponent > xComp( mxEmbeddedResolver, UNO_QUERY );
        xComp->dispose();
    }

    return ERRCODE_NONE;
}

void SvXMLExport::ResetNamespaceMap()
{
    mpNamespaceMap.reset( new SvXMLNamespaceMap );
}

OUString const & SvXMLExport::GetSourceShellID() const
{
    return mpImpl->maSrcShellID;
}

OUString const & SvXMLExport::GetDestinationShellID() const
{
    return mpImpl->maDestShellID;
}

void SvXMLExport::ExportMeta_()
{
    OUString generator( ::utl::DocInfoHelper::GetGeneratorString() );
    Reference< XDocumentPropertiesSupplier > xDocPropsSupplier(mxModel,
        UNO_QUERY);
    if (xDocPropsSupplier.is()) {
        Reference<XDocumentProperties> xDocProps(
            xDocPropsSupplier->getDocumentProperties());
        if (!xDocProps.is()) throw;
        // update generator here
        xDocProps->setGenerator(generator);
        rtl::Reference<SvXMLMetaExport> pMeta = new SvXMLMetaExport(*this, xDocProps);
        pMeta->Export();
    } else {
        // office:meta
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_META,
                                true, true );
        {
    // BM: #i60323# export generator even if xInfoProp is empty (which is the
    // case for charts). The generator does not depend on xInfoProp
            SvXMLElementExport anElem( *this, XML_NAMESPACE_META, XML_GENERATOR,
                                      true, true );
            Characters(generator);
        }
    }
}

void SvXMLExport::ExportScripts_()
{
    SvXMLElementExport aElement( *this, XML_NAMESPACE_OFFICE, XML_SCRIPTS, true, true );

    // export Basic macros (only for FlatXML)
    if ( mnExportFlags & SvXMLExportFlags::EMBEDDED )
    {
        OUString aValue( GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_OOO ) );
        aValue += ":Basic";
        AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE, aValue );

        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_SCRIPT, true, true );

        // initialize Basic
        if ( mxModel.is() )
        {
            Reference< beans::XPropertySet > xPSet( mxModel, UNO_QUERY );
            if ( xPSet.is() )
                xPSet->getPropertyValue("BasicLibraries");
        }

        Reference < XDocumentHandler > xHdl( new XMLBasicExportFilter( mxHandler ) );
        Reference< document::XXMLBasicExporter > xExporter = document::XMLOasisBasicExporter::createWithHandler( m_xContext, xHdl );

        Reference< XComponent > xComp( mxModel, UNO_QUERY );
        xExporter->setSourceDocument( xComp );
        Sequence< PropertyValue > aMediaDesc( 0 );
        xExporter->filter( aMediaDesc );
    }

    // export document events
    Reference< document::XEventsSupplier > xEvents( GetModel(), UNO_QUERY );
    GetEventExport().Export( xEvents );
}

void SvXMLExport::ExportFontDecls_()
{
    if( mxFontAutoStylePool.is() )
        mxFontAutoStylePool->exportXML();
}

void SvXMLExport::ExportStyles_( bool )
{
    uno::Reference< lang::XMultiServiceFactory > xFact( GetModel(), uno::UNO_QUERY );
    if( xFact.is())
    {
        // export (fill-)gradient-styles
        try
        {
            uno::Reference< container::XNameAccess > xGradient( xFact->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY );
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
                        catch(const container::NoSuchElementException&)
                        {
                        }
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }

        // export (fill-)hatch-styles
        try
        {
            uno::Reference< container::XNameAccess > xHatch( xFact->createInstance("com.sun.star.drawing.HatchTable"), uno::UNO_QUERY );
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
                        catch(const container::NoSuchElementException&)
                        {}
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }

        // export (fill-)bitmap-styles
        try
        {
            uno::Reference< container::XNameAccess > xBitmap( xFact->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY );
            if( xBitmap.is() )
            {
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

                            XMLImageStyle::exportXML( rStrName, aValue, *this );
                        }
                        catch(const container::NoSuchElementException&)
                        {
                        }
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }

        // export transparency-gradient -styles
        try
        {
            uno::Reference< container::XNameAccess > xTransGradient( xFact->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY );
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
                        catch(const container::NoSuchElementException&)
                        {
                        }
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }

        // export marker-styles
        try
        {
            uno::Reference< container::XNameAccess > xMarker( xFact->createInstance("com.sun.star.drawing.MarkerTable"), uno::UNO_QUERY );
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
                        catch(const container::NoSuchElementException&)
                        {
                        }
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }

        // export dash-styles
        try
        {
            uno::Reference< container::XNameAccess > xDashes( xFact->createInstance("com.sun.star.drawing.DashTable"), uno::UNO_QUERY );
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
                        catch(const container::NoSuchElementException&)
                        {
                        }
                    }
                }
            }
        }
        catch(const lang::ServiceNotRegisteredException&)
        {
        }
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

void SvXMLExport::collectAutoStyles()
{
}

XMLPageExport* SvXMLExport::CreatePageExport()
{
    return new XMLPageExport( *this );
}

SchXMLExportHelper* SvXMLExport::CreateChartExport()
{
    return new SchXMLExportHelper(*this, *GetAutoStylePool());
}

XMLFontAutoStylePool* SvXMLExport::CreateFontAutoStylePool()
{
    return new XMLFontAutoStylePool( *this );
}

xmloff::OFormLayerXMLExport* SvXMLExport::CreateFormExport()
{
    return new xmloff::OFormLayerXMLExport(*this);
}

void SvXMLExport::GetViewSettingsAndViews(uno::Sequence<beans::PropertyValue>& rProps)
{
    GetViewSettings(rProps);
    uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetModel(), uno::UNO_QUERY);
    if(xViewDataSupplier.is())
    {
        uno::Reference<container::XIndexAccess> xIndexAccess;
        xViewDataSupplier->setViewData( xIndexAccess ); // make sure we get a newly created sequence
        xIndexAccess = xViewDataSupplier->getViewData();
        bool bAdd = false;
        uno::Any aAny;
        if(xIndexAccess.is() && xIndexAccess->hasElements() )
        {
            sal_Int32 nCount = xIndexAccess->getCount();
            for (sal_Int32 i = 0; i < nCount; i++)
            {
                aAny = xIndexAccess->getByIndex(i);
                uno::Sequence<beans::PropertyValue> aProps;
                if( aAny >>= aProps )
                {
                    if( aProps.getLength() > 0 )
                    {
                        bAdd = true;
                        break;
                    }
                }
            }
        }

        if( bAdd )
        {
            sal_Int32 nOldLength(rProps.getLength());
            rProps.realloc(nOldLength + 1);
            beans::PropertyValue aProp;
            aProp.Name = "Views";
            aProp.Value <<= xIndexAccess;
            rProps[nOldLength] = aProp;
        }
    }
}

void SvXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>&)
{
}

void SvXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>&)
{
}

sal_Int32 SvXMLExport::GetDocumentSpecificSettings( ::std::vector< SettingsGroup >& )
{
    return 0;
}

void SvXMLExport::addDataStyle(const sal_Int32 nNumberFormat, bool /*bTimeFormat*/ )
{
    if(mpNumExport)
        mpNumExport->SetUsed(nNumberFormat);
}

void SvXMLExport::exportDataStyles()
{
    if(mpNumExport)
        mpNumExport->Export(false);
}

void SvXMLExport::exportAutoDataStyles()
{
    if(mpNumExport)
        mpNumExport->Export(true);

    if (mxFormExport.is())
        mxFormExport->exportAutoControlNumberStyles();
}

OUString SvXMLExport::getDataStyleName(const sal_Int32 nNumberFormat, bool /*bTimeFormat*/ ) const
{
    OUString sTemp;
    if(mpNumExport)
        sTemp = mpNumExport->GetStyleName(nNumberFormat);
    return sTemp;
}

void SvXMLExport::exportAnnotationMeta(const uno::Reference<drawing::XShape>&)
{
}

sal_Int32 SvXMLExport::dataStyleForceSystemLanguage(sal_Int32 nFormat) const
{
    return ( mpNumExport != nullptr )
                 ? mpNumExport->ForceSystemLanguage( nFormat ) : nFormat;
}

OUString SvXMLExport::AddEmbeddedXGraphic(uno::Reference<graphic::XGraphic> const & rxGraphic, OUString & rOutMimeType, OUString const & rRequestedName)
{
    OUString sURL;

    Graphic aGraphic(rxGraphic);
    OUString aOriginURL = aGraphic.getOriginURL();

    if (!aOriginURL.isEmpty())
    {
        sURL = GetRelativeReference(aOriginURL);
    }
    else
    {
        if (mxGraphicStorageHandler.is())
        {
            if (!(getExportFlags() & SvXMLExportFlags::EMBEDDED))
                sURL = mxGraphicStorageHandler->saveGraphicByName(rxGraphic, rOutMimeType, rRequestedName);
        }
    }
    return sURL;
}

bool SvXMLExport::GetGraphicMimeTypeFromStream(uno::Reference<graphic::XGraphic> const & rxGraphic, OUString & rOutMimeType)
{
    if (mxGraphicStorageHandler.is())
    {
        Reference<XInputStream> xInputStream(mxGraphicStorageHandler->createInputStream(rxGraphic));
        if (xInputStream.is())
        {
            rOutMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForImageStream(xInputStream);
            return true;
        }
    }

    return false;
}

bool SvXMLExport::AddEmbeddedXGraphicAsBase64(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    if ((getExportFlags() & SvXMLExportFlags::EMBEDDED) &&
        mxGraphicStorageHandler.is())
    {
        Reference<XInputStream> xInputStream(mxGraphicStorageHandler->createInputStream(rxGraphic));
        if (xInputStream.is())
        {
            Graphic aGraphic(rxGraphic);
            if (aGraphic.getOriginURL().isEmpty()) // don't add the base64 if the origin URL is set (image is from an external URL)
            {
                XMLBase64Export aBase64Exp(*this);
                return aBase64Exp.exportOfficeBinaryDataElement(xInputStream);
            }
        }
    }

    return false;
}

OUString SvXMLExport::AddEmbeddedObject( const OUString& rEmbeddedObjectURL )
{
    OUString sRet;
    bool bSupportedURL = rEmbeddedObjectURL.startsWith(XML_EMBEDDEDOBJECT_URL_BASE) ||
                         rEmbeddedObjectURL.startsWith(XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE);
    if (bSupportedURL && mxEmbeddedResolver.is())
    {
        sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL(rEmbeddedObjectURL);
    }
    else
        sRet = GetRelativeReference( rEmbeddedObjectURL );

    return sRet;
}

bool SvXMLExport::AddEmbeddedObjectAsBase64( const OUString& rEmbeddedObjectURL )
{
    bool bRet = false;
    bool bSupportedURL = rEmbeddedObjectURL.startsWith(XML_EMBEDDEDOBJECT_URL_BASE) ||
                         rEmbeddedObjectURL.startsWith(XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE);
    if (bSupportedURL && mxEmbeddedResolver.is())
    {
        Reference < XNameAccess > xNA( mxEmbeddedResolver, UNO_QUERY );
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
        bool *pEncoded ) const
{
    return GetMM100UnitConverter().encodeStyleName( rName, pEncoded );
}

ProgressBarHelper*  SvXMLExport::GetProgressBarHelper()
{
    if (!mpProgressBarHelper)
    {
        mpProgressBarHelper.reset( new ProgressBarHelper(mxStatusIndicator, true) );

        if (mxExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sProgressRange(XML_PROGRESSRANGE);
                OUString sProgressMax(XML_PROGRESSMAX);
                OUString sProgressCurrent(XML_PROGRESSCURRENT);
                OUString sRepeat(XML_PROGRESSREPEAT);
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent) &&
                    xPropertySetInfo->hasPropertyByName(sProgressRange))
                {
                    uno::Any aAny;
                    sal_Int32 nProgressMax(0);
                    sal_Int32 nProgressCurrent(0);
                    sal_Int32 nProgressRange(0);
                    aAny = mxExportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        mpProgressBarHelper->SetRange(nProgressRange);
                    aAny = mxExportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        mpProgressBarHelper->SetReference(nProgressMax);
                    aAny = mxExportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        mpProgressBarHelper->SetValue(nProgressCurrent);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                {
                    uno::Any aAny = mxExportInfo->getPropertyValue(sRepeat);
                    if (aAny.getValueType() == cppu::UnoType<bool>::get())
                        mpProgressBarHelper->SetRepeat(::cppu::any2bool(aAny));
                    else {
                        SAL_WARN("xmloff.core", "why is it no boolean?" );
                    }
                }
            }
        }
    }
    return mpProgressBarHelper.get();
}

XMLEventExport& SvXMLExport::GetEventExport()
{
    if( nullptr == mpEventExport)
    {
        // create EventExport on demand
        mpEventExport.reset( new XMLEventExport(*this) );

        // and register standard handlers + names
        mpEventExport->AddHandler("StarBasic", std::make_unique<XMLStarBasicExportHandler>());
        mpEventExport->AddHandler("Script", std::make_unique<XMLScriptExportHandler>());
        mpEventExport->AddTranslationTable(aStandardEventTable);
    }

    return *mpEventExport;
}

XMLImageMapExport& SvXMLExport::GetImageMapExport()
{
    // image map export, create on-demand
    if( nullptr == mpImageMapExport )
    {
        mpImageMapExport.reset( new XMLImageMapExport(*this) );
    }

    return *mpImageMapExport;
}

namespace
{
    class theSvXMLExportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvXMLExportUnoTunnelId> {};
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLExport::getUnoTunnelId() throw()
{
    return theSvXMLExportUnoTunnelId::get().getSeq();
}

SvXMLExport* SvXMLExport::getImplementation( const uno::Reference< uno::XInterface >& xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
    {
        return
            reinterpret_cast<SvXMLExport*>(
                sal::static_int_cast<sal_IntPtr>(
                    xUT->getSomething( SvXMLExport::getUnoTunnelId())));
    }
    else
        return nullptr;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLExport::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

void SvXMLExport::ExportEmbeddedOwnObject( Reference< XComponent > const & rComp )
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

    SAL_WARN_IF( !sFilterService.getLength(), "xmloff.core", "no export filter for own object" );

    if( sFilterService.isEmpty() )
        return;

    Reference < XDocumentHandler > xHdl =
        new XMLEmbeddedObjectExportFilter( mxHandler );

    Sequence < Any > aArgs( 1 );
    aArgs[0] <<= xHdl;

    Reference< document::XExporter > xExporter(
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(sFilterService, aArgs, m_xContext),
        UNO_QUERY);
    SAL_WARN_IF( !xExporter.is(), "xmloff.core", "can't instantiate export filter component for own object" );
    if( !xExporter.is() )
        return;

    xExporter->setSourceDocument( rComp );

    Reference<XFilter> xFilter( xExporter, UNO_QUERY );

    Sequence < PropertyValue > aMediaDesc( 0 );
    xFilter->filter( aMediaDesc );
}

OUString SvXMLExport::GetRelativeReference(const OUString& rValue)
{
    OUString sValue( rValue );
    // #i65474# handling of fragment URLs ("#....") is undefined
    // they are stored 'as is'
    uno::Reference< uri::XUriReference > xUriRef;
    if(!sValue.isEmpty() && sValue[0] != '#')
    {
        try
        {
            xUriRef = mpImpl->mxUriReferenceFactory->parse( rValue );
            if( xUriRef.is() && !xUriRef->isAbsolute() )
            {
                //#i61943# relative URLs need special handling
                INetURLObject aTemp( mpImpl->msPackageURI );
                bool bWasAbsolute = false;
                sValue = aTemp.smartRel2Abs(sValue, bWasAbsolute ).GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
            }
        }
        catch(const uno::Exception&)
        {
        }
    }
    if( xUriRef.is() )//no conversion for empty values or for fragments
    {
        //conversion for matching schemes only
        if( xUriRef->getScheme() == mpImpl->msPackageURIScheme )
        {
            sValue = INetURLObject::GetRelURL( msOrigFileName, sValue );
        }
    }
    return sValue;
}

void SvXMLExport::StartElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSOutside )
{
    StartElement(GetNamespaceMap_().GetQNameByKey( nPrefix,
        GetXMLToken(eName) ), bIgnWSOutside);
}

void SvXMLExport::StartElement(const OUString& rName,
                        bool bIgnWSOutside )
{
    if ((mnErrorFlags & SvXMLErrorFlags::DO_NOTHING) != SvXMLErrorFlags::DO_NOTHING)
    {
        try
        {
            if( bIgnWSOutside && ((mnExportFlags & SvXMLExportFlags::PRETTY) == SvXMLExportFlags::PRETTY))
                mxHandler->ignorableWhitespace( msWS );
            mxHandler->startElement( rName, GetXAttrList() );
        }
        catch (const SAXInvalidCharacterException& e)
        {
            Sequence<OUString> aPars { rName };
            SetError( XMLERROR_SAX|XMLERROR_FLAG_WARNING, aPars, e.Message, nullptr );
        }
        catch (const SAXException& e)
        {
            Sequence<OUString> aPars { rName };
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, nullptr );
        }
    }
    ClearAttrList();
    ++mpImpl->mDepth; // increment nesting depth counter
}

void SvXMLExport::Characters(const OUString& rChars)
{
    if ((mnErrorFlags & SvXMLErrorFlags::DO_NOTHING) != SvXMLErrorFlags::DO_NOTHING)
    {
        try
        {
            mxHandler->characters(rChars);
        }
        catch (const SAXInvalidCharacterException& e)
        {
            Sequence<OUString> aPars { rChars };
            SetError( XMLERROR_SAX|XMLERROR_FLAG_WARNING, aPars, e.Message, nullptr );
        }
        catch (const SAXException& e)
        {
            Sequence<OUString> aPars { rChars };
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, nullptr );
        }
    }
}

void SvXMLExport::EndElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSInside )
{
    EndElement(GetNamespaceMap_().GetQNameByKey( nPrefix, GetXMLToken(eName) ),
        bIgnWSInside);
}

void SvXMLExport::EndElement(const OUString& rName,
                        bool bIgnWSInside )
{
    // decrement nesting depth counter & (maybe) restore namespace map
    --mpImpl->mDepth;
    if (!mpImpl->mNamespaceMaps.empty() &&
        (mpImpl->mNamespaceMaps.top().second == mpImpl->mDepth))
    {
        mpNamespaceMap = std::move(mpImpl->mNamespaceMaps.top().first);
        mpImpl->mNamespaceMaps.pop();
    }
    SAL_WARN_IF(!mpImpl->mNamespaceMaps.empty() &&
        (mpImpl->mNamespaceMaps.top().second >= mpImpl->mDepth), "xmloff.core", "SvXMLExport: NamespaceMaps corrupted");

    if ((mnErrorFlags & SvXMLErrorFlags::DO_NOTHING) != SvXMLErrorFlags::DO_NOTHING)
    {
        try
        {
            if( bIgnWSInside && ((mnExportFlags & SvXMLExportFlags::PRETTY) == SvXMLExportFlags::PRETTY))
                mxHandler->ignorableWhitespace( msWS );
            mxHandler->endElement( rName );
        }
        catch (const SAXException& e)
        {
            Sequence<OUString> aPars { rName };
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, nullptr );
        }
    }
}

void SvXMLExport::IgnorableWhitespace()
{
    if ((mnExportFlags & SvXMLExportFlags::PRETTY) != SvXMLExportFlags::PRETTY)
        return;

    if ((mnErrorFlags & SvXMLErrorFlags::DO_NOTHING) != SvXMLErrorFlags::DO_NOTHING)
    {
        try
        {
            mxHandler->ignorableWhitespace( msWS );
        }
        catch (const SAXException& e)
        {
            Sequence<OUString> aPars(0);
            SetError( XMLERROR_SAX|XMLERROR_FLAG_ERROR|XMLERROR_FLAG_SEVERE,
                      aPars, e.Message, nullptr );
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
    static osl::Mutex aMutex;
    osl::MutexGuard aGuard(aMutex);

    // maintain error flags
    if ( ( nId & XMLERROR_FLAG_ERROR ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::ERROR_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_WARNING ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::WARNING_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_SEVERE ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::DO_NOTHING;

    // create error list on demand
    if ( mpXMLErrors == nullptr )
        mpXMLErrors.reset( new XMLErrors() );

    // save error information
    mpXMLErrors->AddRecord( nId, rMsgParams, rExceptionMessage, rLocator );
}

void SvXMLExport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams)
{
    SetError( nId, rMsgParams, "", nullptr );
}

void SvXMLExport::DisposingModel()
{
    mxModel.clear();
    // Shapes in Writer cannot be named via context menu (#i51726#)
    meModelType = SvtModuleOptions::EFactory::UNKNOWN_FACTORY;
    mxEventListener.clear();
}


::comphelper::UnoInterfaceToUniqueIdentifierMapper& SvXMLExport::getInterfaceToIdentifierMapper()
{
    return mpImpl->maInterfaceToIdentifierMapper;
}

// Written OpenDocument file format doesn't fit to the created text document (#i69627#)
bool SvXMLExport::writeOutlineStyleAsNormalListStyle() const
{
    return mpImpl->mbOutlineStyleAsNormalListStyle;
}

uno::Reference< embed::XStorage > const & SvXMLExport::GetTargetStorage()
{
    return mpImpl->mxTargetStorage;
}

/// returns the currently configured default version for ODF export
SvtSaveOptions::ODFDefaultVersion SvXMLExport::getDefaultVersion() const
{
    if( mpImpl )
        return mpImpl->maSaveOptions.GetODFDefaultVersion();

    // fatal error, use current version as default
    return SvtSaveOptions::ODFVER_012;
}

SvtSaveOptions::ODFSaneDefaultVersion SvXMLExport::getSaneDefaultVersion() const
{
    if( mpImpl )
        return mpImpl->maSaveOptions.GetODFSaneDefaultVersion();

    // fatal error, use current version as default
    return SvtSaveOptions::ODFSVER_LATEST;
}

void
SvXMLExport::AddAttributeIdLegacy(
        sal_uInt16 const nLegacyPrefix, OUString const& rValue)
{
    switch (getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall through
        case SvtSaveOptions::ODFVER_010: break;
        default: // ODF 1.2: xml:id
            AddAttribute(XML_NAMESPACE_XML, XML_ID, rValue);
    }
    // in ODF 1.1 this was form:id, anim:id, draw:id, or text:id
    // backward compatibility: in ODF 1.2 write _both_ id attrs
    AddAttribute(nLegacyPrefix, XML_ID, rValue);
    // FIXME: this function simply assumes that rValue is unique
}

void
SvXMLExport::AddAttributeXmlId(uno::Reference<uno::XInterface> const & i_xIfc)
{
    // check version >= 1.2
    switch (getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall through
        case SvtSaveOptions::ODFVER_010: return;
        default: break;
    }
    const uno::Reference<rdf::XMetadatable> xMeta(i_xIfc,
        uno::UNO_QUERY);
//FIXME not yet...
    if ( xMeta.is() )
    {
        const beans::StringPair mdref( xMeta->getMetadataReference() );
        if ( !mdref.Second.isEmpty() )
        {
            const OUString streamName = mpImpl->mStreamName;
            if ( !streamName.isEmpty() )
            {
                if ( streamName == mdref.First )
                {
                    AddAttribute( XML_NAMESPACE_XML, XML_ID, mdref.Second );
                }
                else
                {
                    SAL_WARN("xmloff.core","SvXMLExport::AddAttributeXmlId: invalid stream name");
                }
            }
            else
            {
                // FIXME: this is ugly
                // there is no stream name (e.g. XSLT, flat-xml format)!
                // but how do we ensure uniqueness in this case?
                // a) just omit styles.xml ids -- they are unlikely anyway...
                // b) somehow find out whether we are currently exporting styles
                //    or content, and prefix "s" or "c" => unique
                if ( mdref.First == "content.xml" )
                {
                    AddAttribute( XML_NAMESPACE_XML, XML_ID, mdref.Second );
                }
                else
                {
                    SAL_INFO("xmloff.core", "SvXMLExport::AddAttributeXmlId: no stream name given: dropping styles.xml xml:id");
                }
            }
        }
    }
}

void
SvXMLExport::AddAttributesRDFa(
    uno::Reference<text::XTextContent> const & i_xTextContent)
{
    // check version >= 1.2
    switch (getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall through
        case SvtSaveOptions::ODFVER_010: return;
        default: break;
    }

    const uno::Reference<rdf::XMetadatable> xMeta(
        i_xTextContent, uno::UNO_QUERY);
    if (!xMeta.is() || xMeta->getMetadataReference().Second.isEmpty())
    {
        return; // no xml:id => no RDFa
    }

    if (!mpImpl->mpRDFaHelper)
    {
        mpImpl->mpRDFaHelper.reset( new ::xmloff::RDFaExportHelper(*this) );
    }
    mpImpl->mpRDFaHelper->AddRDFa(xMeta);
}

bool SvXMLExport::exportTextNumberElement() const
{
    return mpImpl->mbExportTextNumberElement;
}

bool SvXMLExport::SetNullDateOnUnitConverter()
{
    // if the null date has already been set, don't set it again (performance)
    if (!mpImpl->mbNullDateInitialized)
        mpImpl->mbNullDateInitialized = GetMM100UnitConverter().setNullDate(GetModel());

    return mpImpl->mbNullDateInitialized;
}

OUString const & SvXMLExport::GetImageFilterName() const
{
    return msImgFilterName;
}

void SvXMLElementExport::StartElement(
    const sal_uInt16 nPrefixKey,
    const OUString& rLName,
    const bool bIgnoreWhitespaceOutside )
{
    maElementName = mrExport.GetNamespaceMap().GetQNameByKey(nPrefixKey, rLName);
    mrExport.StartElement(maElementName, bIgnoreWhitespaceOutside);
}

SvXMLElementExport::SvXMLElementExport(
    SvXMLExport& rExp,
    sal_uInt16 nPrefixKey,
    const sal_Char *pLName,
    bool bIWSOutside,
    bool bIWSInside )
    : mrExport( rExp )
    , maElementName()
    , mbIgnoreWhitespaceInside( bIWSInside )
    , mbDoSomething( true )
{
    const OUString sLName( OUString::createFromAscii( pLName ) );
    StartElement( nPrefixKey, sLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport(
    SvXMLExport& rExp,
    sal_uInt16 nPrefixKey,
    const OUString& rLName,
    bool bIWSOutside,
    bool bIWSInside )
    : mrExport( rExp )
    , maElementName()
    , mbIgnoreWhitespaceInside( bIWSInside )
    , mbDoSomething( true )
{
    StartElement( nPrefixKey, rLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport(
    SvXMLExport& rExp,
    sal_uInt16 nPrefixKey,
    enum XMLTokenEnum eLName,
    bool bIWSOutside,
    bool bIWSInside )
    : mrExport( rExp )
    , maElementName()
    , mbIgnoreWhitespaceInside( bIWSInside )
    , mbDoSomething( true )
{
    StartElement( nPrefixKey, GetXMLToken(eLName), bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport(
    SvXMLExport& rExp,
    bool bDoSth,
    sal_uInt16 nPrefixKey,
    enum XMLTokenEnum eLName,
    bool bIWSOutside,
    bool bIWSInside )
    : mrExport( rExp )
    , maElementName()
    , mbIgnoreWhitespaceInside( bIWSInside )
    , mbDoSomething( bDoSth )
{
    if ( mbDoSomething )
        StartElement( nPrefixKey, GetXMLToken( eLName ), bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport(
    SvXMLExport& rExp,
    const OUString& rQName,
    bool bIWSOutside,
    bool bIWSInside )
    : mrExport( rExp )
    , maElementName()
    , mbIgnoreWhitespaceInside( bIWSInside )
    , mbDoSomething( true )
{
    maElementName = rQName;
    rExp.StartElement( rQName, bIWSOutside );
}

SvXMLElementExport::~SvXMLElementExport()
{
    if ( mbDoSomething )
    {
        mrExport.EndElement( maElementName, mbIgnoreWhitespaceInside );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

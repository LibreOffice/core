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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include "xmlfilter.hxx"
#include "xmlReport.hxx"
#include <vcl/errinf.hxx>
#include "xmlHelper.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <connectivity/dbtools.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <unotools/mediadescriptor.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/XMLTextMasterStylesContext.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlmetai.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svtools/sfxecode.hxx>
#include "xmlEnums.hxx"
#include "xmlStyleImport.hxx"
#include <strings.hxx>
#include "xmlPropertyHandler.hxx"
#include <ReportDefinition.hxx>

namespace rptxml
{
using namespace ::com::sun::star::uno;
using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;
using namespace xmloff;
using namespace ::com::sun::star::util;

namespace {

class RptMLMasterStylesContext_Impl:
    public XMLTextMasterStylesContext
{
    ORptFilter& GetImport() { return static_cast<ORptFilter&>(XMLTextMasterStylesContext::GetImport()); }

public:


    RptMLMasterStylesContext_Impl( ORptFilter& rImport );

    RptMLMasterStylesContext_Impl(const RptMLMasterStylesContext_Impl&) = delete;
    RptMLMasterStylesContext_Impl& operator=(const RptMLMasterStylesContext_Impl&) = delete;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

}

RptMLMasterStylesContext_Impl::RptMLMasterStylesContext_Impl( ORptFilter& rImport ) :
    XMLTextMasterStylesContext( rImport )
{
}

void RptMLMasterStylesContext_Impl::endFastElement(sal_Int32 )
{
    FinishStyles( true );
    GetImport().FinishStyles();
}

    /// read a component (file + filter version)
static ErrCode ReadThroughComponent(
    const uno::Reference<XInputStream>& xInputStream,
    const uno::Reference<XComponent>& xModelComponent,
    const uno::Reference<XComponentContext> & rContext,
    const uno::Reference<XFastParser>& rFastParser )
{
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rContext.is(), "factory missing");

    // prepare Parser InputSource
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get filter
    SAL_WARN_IF( !rFastParser.is(), "reportdesign", "Can't instantiate filter component." );
    if( !rFastParser.is() )
        return ErrCode(1);

    // connect model and filter
    uno::Reference < XImporter > xImporter( rFastParser, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );

    // finally, parser the stream
    try
    {
        rFastParser->parseStream( aParserInput );
    }
    catch (const SAXParseException&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "");
        return ErrCode(1);
    }
    catch (const SAXException&)
    {
        return ErrCode(1);
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const IOException&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "");
        return ErrCode(1);
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "");
        return ErrCode(1);
    }

    // success!
    return ERRCODE_NONE;
}

/// read a component (storage version)
static ErrCode ReadThroughComponent(
    const uno::Reference< embed::XStorage >& xStorage,
    const uno::Reference<XComponent>& xModelComponent,
    const char* pStreamName,
    const uno::Reference<XComponentContext> & rxContext,
    const Reference<document::XGraphicStorageHandler> & rxGraphicStorageHandler,
    const Reference<document::XEmbeddedObjectResolver>& _xEmbeddedObjectResolver,
    const OUString& _sFilterName
    ,const uno::Reference<beans::XPropertySet>& _xProp)
{
    OSL_ENSURE( xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    if ( !xStorage )
        // TODO/LATER: better error handling
        return ErrCode(1);

    uno::Reference< io::XStream > xDocStream;

    try
    {
        // open stream (and set parser input)
        OUString sStreamName = OUString::createFromAscii(pStreamName);
        if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
        {
            // stream name not found! return immediately with OK signal
            return ERRCODE_NONE;
        }

        // get input stream
        xDocStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
    }
    catch (const packages::WrongPasswordException&)
    {
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (const uno::Exception&)
    {
        return ErrCode(1); // TODO/LATER: error handling
    }

    sal_Int32 nArgs = 0;
    if (rxGraphicStorageHandler.is())
        nArgs++;
    if( _xEmbeddedObjectResolver.is())
        nArgs++;
    if ( _xProp.is() )
        nArgs++;

    uno::Sequence< uno::Any > aFilterCompArgs( nArgs );
    auto aFilterCompArgsRange = asNonConstRange(aFilterCompArgs);

    nArgs = 0;
    if (rxGraphicStorageHandler.is())
        aFilterCompArgsRange[nArgs++] <<= rxGraphicStorageHandler;
    if( _xEmbeddedObjectResolver.is())
        aFilterCompArgsRange[ nArgs++ ] <<= _xEmbeddedObjectResolver;
    if ( _xProp.is() )
        aFilterCompArgsRange[ nArgs++ ] <<= _xProp;

    // the underlying SvXMLImport implements XFastParser, XImporter, XFastDocumentHandler
    Reference< XFastParser > xFastParser(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(_sFilterName, aFilterCompArgs, rxContext),
        uno::UNO_QUERY_THROW );
    uno::Reference< XInputStream > xInputStream = xDocStream->getInputStream();
    // read from the stream
    return ReadThroughComponent( xInputStream
                                ,xModelComponent
                                ,rxContext
                                ,xFastParser );
}


/** Imports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ORptImportHelper_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptFilter(context,
        SERVICE_SETTINGSIMPORTER,
        SvXMLImportFlags::SETTINGS ));
}

/** Imports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_XMLOasisContentImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptFilter(context,
        SERVICE_CONTENTIMPORTER,
        SvXMLImportFlags::AUTOSTYLES | SvXMLImportFlags::CONTENT | SvXMLImportFlags::SCRIPTS | SvXMLImportFlags::FONTDECLS ));
}

/** Imports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ORptStylesImportHelper_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptFilter(context,
        SERVICE_STYLESIMPORTER,
        SvXMLImportFlags::STYLES | SvXMLImportFlags::MASTERSTYLES | SvXMLImportFlags::AUTOSTYLES |
        SvXMLImportFlags::FONTDECLS ));
}

/** Imports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ORptMetaImportHelper_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptFilter(context,
        SERVICE_METAIMPORTER,
        SvXMLImportFlags::META ));
}


ORptFilter::ORptFilter( const uno::Reference< XComponentContext >& _rxContext, OUString const & rImplementationName, SvXMLImportFlags nImportFlags )
    :SvXMLImport(_rxContext, rImplementationName, nImportFlags)
{
    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_100TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);
    GetNamespaceMap().Add( u"_report"_ustr,
                        GetXMLToken(XML_N_RPT),
                        XML_NAMESPACE_REPORT );

    GetNamespaceMap().Add( u"__report"_ustr,
                        GetXMLToken(XML_N_RPT_OASIS),
                        XML_NAMESPACE_REPORT );

    m_xPropHdlFactory = new OXMLRptPropHdlFactory;
    m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylePropertyMap(true, false);
    m_xColumnStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetColumnStyleProps(), m_xPropHdlFactory, false);
    m_xRowStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetRowStyleProps(), m_xPropHdlFactory, false);
}


ORptFilter::~ORptFilter() noexcept
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_OReportFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ORptFilter(context,
        u"com.sun.star.comp.report.OReportFilter"_ustr,
        SvXMLImportFlags::ALL ));
}

sal_Bool SAL_CALL ORptFilter::filter( const Sequence< PropertyValue >& rDescriptor )
{
    vcl::Window*     pFocusWindow = Application::GetFocusWindow();
    bool    bRet = false;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if ( GetModel().is() )
        bRet = implImport( rDescriptor );

    if ( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

bool ORptFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
{
    OUString                     sFileName;
    uno::Reference< embed::XStorage >   xStorage;
    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier;

    for(const PropertyValue& rProp : rDescriptor)
    {
        if ( rProp.Name == "FileName" )
            rProp.Value >>= sFileName;
        else if ( rProp.Name == "Storage" )
            rProp.Value >>= xStorage;
        else if ( rProp.Name == "ComponentData" )
        {
            Sequence< PropertyValue > aComponent;
            rProp.Value >>= aComponent;
            const PropertyValue* pComponentIter = aComponent.getConstArray();
            const PropertyValue* pComponentEnd  = pComponentIter + aComponent.getLength();
            pComponentIter = std::find_if(pComponentIter, pComponentEnd,
                [](const PropertyValue& rComponent) { return rComponent.Name == "ActiveConnection"; });
            if (pComponentIter != pComponentEnd)
            {
                uno::Reference<sdbc::XConnection> xCon(pComponentIter->Value, uno::UNO_QUERY);
                xNumberFormatsSupplier = ::dbtools::getNumberFormats(xCon);
            }
        }
    }

    if ( !sFileName.isEmpty() )
    {
        tools::SvRef<SfxMedium> pMedium = new SfxMedium(
                sFileName, ( StreamMode::READ | StreamMode::NOCREATE ) );

        if( pMedium.is() )
        {
            try
            {
                xStorage = pMedium->GetStorage();
            }
            catch (const Exception&)
            {
            }
        }
    }
    bool bRet = xStorage.is();
    if ( bRet )
    {
        m_xReportDefinition.set(GetModel(),UNO_QUERY_THROW);

#if OSL_DEBUG_LEVEL > 1
        uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
        uno::Sequence< OUString> aSeq = xAccess->getElementNames();
        const OUString* pDebugIter = aSeq.getConstArray();
        const OUString* pDebugEnd      = pDebugIter + aSeq.getLength();
        for(;pDebugIter != pDebugEnd;++pDebugIter)
        {
            (void)*pDebugIter;
        }
#endif

        uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
        uno::Reference<document::XEmbeddedObjectResolver> xEmbeddedObjectResolver;
        uno::Reference< uno::XComponentContext > xContext = GetComponentContext();

        uno::Sequence<uno::Any> aArgs{ uno::Any(xStorage) };
        xGraphicStorageHandler.set(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(u"com.sun.star.comp.Svx.GraphicImportHelper"_ustr, aArgs, xContext),
                uno::UNO_QUERY);

        uno::Reference< lang::XMultiServiceFactory > xReportServiceFactory( m_xReportDefinition, uno::UNO_QUERY);
        aArgs.getArray()[0] <<= beans::NamedValue(u"Storage"_ustr, uno::Any(xStorage));
        xEmbeddedObjectResolver.set( xReportServiceFactory->createInstanceWithArguments(u"com.sun.star.document.ImportEmbeddedObjectResolver"_ustr,aArgs) , uno::UNO_QUERY);

        static constexpr OUString s_sOld = u"OldFormat"_ustr;
        static comphelper::PropertyMapEntry const pMap[] =
        {
            { u"OldFormat"_ustr , 1,    cppu::UnoType<sal_Bool>::get(),                 beans::PropertyAttribute::BOUND,     0 },
            { u"StreamName"_ustr, 0,    cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"PrivateData"_ustr,0,    cppu::UnoType<XInterface>::get(),  beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"BaseURI"_ustr,    0,    cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamRelPath"_ustr, 0, cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
        };
        utl::MediaDescriptor aDescriptor(rDescriptor);
        uno::Reference<beans::XPropertySet> xProp = comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(pMap));
        const OUString sVal( aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTBASEURL, OUString()) );
        assert(!sVal.isEmpty()); // needed for relative URLs
        xProp->setPropertyValue(u"BaseURI"_ustr, uno::Any(sVal));
        const OUString sHierarchicalDocumentName( aDescriptor.getUnpackedValueOrDefault(u"HierarchicalDocumentName"_ustr,OUString()) );
        xProp->setPropertyValue(u"StreamRelPath"_ustr, uno::Any(sHierarchicalDocumentName));

        uno::Reference<XComponent> xModel = GetModel();
        static constexpr OUString s_sMeta = u"meta.xml"_ustr;
        static constexpr OUString s_sStreamName = u"StreamName"_ustr;
        xProp->setPropertyValue(s_sStreamName, uno::Any(s_sMeta));
        ErrCode nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"meta.xml"
                                    ,GetComponentContext()
                                    ,xGraphicStorageHandler
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_METAIMPORTER
                                    ,xProp
                                    );


        try
        {
            xProp->setPropertyValue(s_sOld,uno::Any(!(xStorage->hasByName(s_sMeta) || xStorage->isStreamElement( s_sMeta ))));
        }
        catch (const uno::Exception&)
        {
            xProp->setPropertyValue(s_sOld,uno::Any(true));
        }

        if ( nRet == ERRCODE_NONE )
        {
            xProp->setPropertyValue(s_sStreamName, uno::Any(u"settings.xml"_ustr));
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"settings.xml"
                                    ,GetComponentContext()
                                    ,xGraphicStorageHandler
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_SETTINGSIMPORTER
                                    ,xProp
                                    );
        }
        if ( nRet == ERRCODE_NONE )
        {
            xProp->setPropertyValue(s_sStreamName, uno::Any(u"styles.xml"_ustr));
            nRet = ReadThroughComponent(xStorage
                                    ,xModel
                                    ,"styles.xml"
                                    ,GetComponentContext()
                                    ,xGraphicStorageHandler
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_STYLESIMPORTER
                                    ,xProp);
        }

        if ( nRet == ERRCODE_NONE )
        {
            xProp->setPropertyValue(s_sStreamName, uno::Any(u"content.xml"_ustr));
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"content.xml"
                                    ,GetComponentContext()
                                    ,xGraphicStorageHandler
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_CONTENTIMPORTER
                                    ,xProp
                                    );
        }


        bRet = nRet == ERRCODE_NONE;

        if ( bRet )
        {
            m_xReportDefinition->setModified(false);
        }
        else
        {
            if( nRet == ERRCODE_IO_BROKENPACKAGE && xStorage.is() )
                ; // TODO/LATER: no way to transport the error outside from the filter!
            else
            {
                // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly! But for now this is the only way!
                ErrorHandler::HandleError( nRet );
                if( nRet.IsWarning() )
                    bRet = true;
            }
        }
    }

    return bRet;
}

namespace {

class RptXMLDocumentSettingsContext : public SvXMLImportContext
{
public:
    RptXMLDocumentSettingsContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        if (nElement == XML_ELEMENT(OFFICE, XML_SETTINGS))
        {
            return new XMLDocumentSettingsContext(GetImport());
        }
        return nullptr;
    }
};

class RptXMLDocumentStylesContext : public SvXMLImportContext
{
public:
    RptXMLDocumentStylesContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        ORptFilter & rImport(static_cast<ORptFilter&>(GetImport()));
        switch (nElement)
        {
            case XML_ELEMENT(OFFICE, XML_FONT_FACE_DECLS):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateFontDeclsContext();
            case XML_ELEMENT(OFFICE, XML_MASTER_STYLES):
                {
                    SvXMLStylesContext* pStyleContext = new RptMLMasterStylesContext_Impl(rImport);
                    rImport.SetMasterStyles(pStyleContext);
                    return pStyleContext;
                }
            case XML_ELEMENT(OFFICE, XML_STYLES):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateStylesContext(false);
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
                // don't use the autostyles from the styles-document for the progress
                return rImport.CreateStylesContext(true);
        }
        return nullptr;
    }
};

}

css::uno::Reference< css::xml::sax::XFastContextHandler > RptXMLDocumentBodyContext::createFastChildContext(
        sal_Int32 nElement,
        const uno::Reference<xml::sax::XFastAttributeList> & xAttrList)
{
    ORptFilter & rImport(static_cast<ORptFilter&>(GetImport()));
    if (nElement == XML_ELEMENT(OFFICE, XML_REPORT) || nElement == XML_ELEMENT(OOO, XML_REPORT))
    {
        rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
        const SvXMLStylesContext* pAutoStyles = rImport.GetAutoStyles();
        if (pAutoStyles)
        {
            XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast<const XMLPropStyleContext *>(pAutoStyles->FindStyleChildContext(XmlStyleFamily::PAGE_MASTER, u"pm1"_ustr)));
            if (pAutoStyle)
            {
                pAutoStyle->FillPropertySet(rImport.getReportDefinition());
            }
        }
        return new OXMLReport(rImport, xAttrList, rImport.getReportDefinition());
    }
    return nullptr;
}

namespace {

class RptXMLDocumentContentContext : public SvXMLImportContext
{
public:
    RptXMLDocumentContentContext(SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override
    {
        ORptFilter & rImport(static_cast<ORptFilter&>(GetImport()));
        switch (nElement)
        {
            case XML_ELEMENT(OFFICE, XML_BODY):
                return new RptXMLDocumentBodyContext(rImport);
            case XML_ELEMENT(OFFICE, XML_FONT_FACE_DECLS):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateFontDeclsContext();
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                return rImport.CreateStylesContext(true);
        }
        return nullptr;
    }
};

}

SvXMLImportContext *ORptFilter::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateMetaContext( nElement );
            break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_CONTENT ):
            pContext = new RptXMLDocumentContentContext(*this);
            break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_STYLES ):
            pContext = new RptXMLDocumentStylesContext(*this);
            break;
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_SETTINGS ):
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new RptXMLDocumentSettingsContext(*this);
            break;
    }
    return pContext;
}

SvXMLImportContext* ORptFilter::CreateStylesContext( bool bIsAutoStyle )
{
    SvXMLImportContext* pContext = bIsAutoStyle ? GetAutoStyles() : GetStyles();
    if ( !pContext )
    {
        pContext = new OReportStylesContext(*this, bIsAutoStyle);
        if (bIsAutoStyle)
            SetAutoStyles(static_cast<SvXMLStylesContext*>(pContext));
        else
            SetStyles(static_cast<SvXMLStylesContext*>(pContext));
    }
    return pContext;
}

SvXMLImportContext *ORptFilter::CreateFontDeclsContext()
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, osl_getThreadTextEncoding() );
    SetFontDecls( pFSContext );
    return pFSContext;
}

XMLShapeImportHelper* ORptFilter::CreateShapeImport()
{
    return new XMLShapeImportHelper( *this,GetModel() );
}

void ORptFilter::FinishStyles()
{
    if( GetStyles() )
        GetStyles()->FinishStyles( true );
}

const OUString& ORptFilter::convertFormula(const OUString& _sFormula)
{
    return _sFormula;
}

void SAL_CALL ORptFilter::startDocument()
{
    m_xReportDefinition.set(GetModel(),UNO_QUERY_THROW);
    m_pReportModel = reportdesign::OReportDefinition::getSdrModel(m_xReportDefinition);
    OSL_ENSURE(m_pReportModel,"Report model is NULL!");

    SvXMLImport::startDocument();
}

void ORptFilter::endDocument()
{
    OSL_ENSURE( GetModel().is(), "model missing; maybe startDocument wasn't called?" );
    if( !GetModel().is() )
        return;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;
    // Clear the shape import to sort the shapes  (and not in the
    // destructor that might be called after the import has finished
    // for Java filters.
    if( HasShapeImport() )
        ClearShapeImport();

    // delegate to parent: takes care of error handling
    SvXMLImport::endDocument();
}

void ORptFilter::removeFunction(const OUString& _sFunctionName)
{
    m_aFunctions.erase(_sFunctionName);
}

void ORptFilter::insertFunction(const css::uno::Reference< css::report::XFunction > & _xFunction)
{
    m_aFunctions.emplace(_xFunction->getName(),_xFunction);
}

SvXMLImportContext* ORptFilter::CreateMetaContext(const sal_Int32 /*nElement*/)
{
    SvXMLImportContext* pContext = nullptr;

    if ( getImportFlags() & SvXMLImportFlags::META )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(), uno::UNO_QUERY_THROW);
        pContext = new SvXMLMetaDocumentContext(*this, xDPS->getDocumentProperties());
    }
    return pContext;
}

bool ORptFilter::isOldFormat() const
{
    bool bOldFormat = true;
    uno::Reference<beans::XPropertySet> xProp = getImportInfo();
    if ( xProp.is() )
    {
        static constexpr OUString s_sOld = u"OldFormat"_ustr;
        if ( xProp->getPropertySetInfo()->hasPropertyByName(s_sOld))
        {
            xProp->getPropertyValue(s_sOld) >>= bOldFormat;
        }
    }
    return bOldFormat;
}


}// rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

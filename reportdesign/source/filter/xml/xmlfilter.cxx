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

#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include "xmlfilter.hxx"
#include "xmlGroup.hxx"
#include "xmlReport.hxx"
#include "xmlHelper.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <connectivity/dbtools.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <unotools/mediadescriptor.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlmetai.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <osl/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include "xmlEnums.hxx"
#include "xmlStyleImport.hxx"
#include "xmlstrings.hrc"
#include "xmlPropertyHandler.hxx"
#include <xmloff/txtprmap.hxx>
#include "ReportDefinition.hxx"

namespace rptxml
{
using namespace ::com::sun::star::uno;
using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;
using namespace xmloff;
using namespace ::com::sun::star::util;

class RptMLMasterStylesContext_Impl:
    public XMLTextMasterStylesContext
{
    ORptFilter& m_rImport;

public:


    RptMLMasterStylesContext_Impl(
            ORptFilter& rImport, sal_uInt16 nPrfx,
            const OUString& rLName ,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList );

    RptMLMasterStylesContext_Impl(const RptMLMasterStylesContext_Impl&) = delete;
    RptMLMasterStylesContext_Impl& operator=(const RptMLMasterStylesContext_Impl&) = delete;
    virtual void EndElement() override;
};

RptMLMasterStylesContext_Impl::RptMLMasterStylesContext_Impl(
        ORptFilter& rImport, sal_uInt16 nPrfx,
        const OUString& rLName ,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextMasterStylesContext( rImport, nPrfx, rLName, xAttrList )
    ,m_rImport(rImport)
{
}

void RptMLMasterStylesContext_Impl::EndElement()
{
    FinishStyles( true );
    m_rImport.FinishStyles();
}

    /// read a component (file + filter version)
sal_Int32 ReadThroughComponent(
    const uno::Reference<XInputStream>& xInputStream,
    const uno::Reference<XComponent>& xModelComponent,
    const sal_Char* /*pStreamName*/,
    const uno::Reference<XComponentContext> & rContext,
    const uno::Reference< XDocumentHandler >& _xFilter,
    bool /*bEncrypted*/ )
{
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rContext.is(), "factory missing");

    // prepare ParserInputSrouce
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    uno::Reference< XParser > xParser = xml::sax::Parser::create(rContext);
    SAL_INFO( "reportdesign", "parser created" );
    // get filter
    OSL_ENSURE( _xFilter.is(), "Can't instantiate filter component." );
    if( !_xFilter.is() )
        return 1;

    // connect parser and filter
    xParser->setDocumentHandler( _xFilter );

    // connect model and filter
    uno::Reference < XImporter > xImporter( _xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch (const SAXParseException& r)
    {
#if OSL_DEBUG_LEVEL > 1
        OStringBuffer aError("SAX parse exception caught while importing:\n");
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        aError.append(r.LineNumber);
        aError.append(',');
        aError.append(r.ColumnNumber);
        OSL_FAIL(aError.getStr());
#else
        (void)r;
#endif
        return 1;
    }
    catch (const SAXException&)
    {
        return 1;
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const IOException&)
    {
        return 1;
    }
    catch (const Exception&)
    {
        return 1;
    }

    // success!
    return 0;
}

/// read a component (storage version)
sal_Int32 ReadThroughComponent(
    const uno::Reference< embed::XStorage >& xStorage,
    const uno::Reference<XComponent>& xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    const uno::Reference<XComponentContext> & rxContext,
    const Reference< document::XGraphicObjectResolver > & _xGraphicObjectResolver,
    const Reference<document::XEmbeddedObjectResolver>& _xEmbeddedObjectResolver,
    const OUString& _sFilterName
    ,const uno::Reference<beans::XPropertySet>& _xProp)
{
    OSL_ENSURE( xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    if ( xStorage.is() )
    {
        uno::Reference< io::XStream > xDocStream;
        bool bEncrypted = false;

        try
        {
            // open stream (and set parser input)
            OUString sStreamName = OUString::createFromAscii(pStreamName);
            if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
            {
                // stream name not found! Then try the compatibility name.
                // if no stream can be opened, return immediately with OK signal

                // do we even have an alternative name?
                if ( nullptr == pCompatibilityStreamName )
                    return 0;

                // if so, does the stream exist?
                sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
                if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
                    return 0;
            }

            // get input stream
            xDocStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );

            uno::Reference< beans::XPropertySet > xProps( xDocStream, uno::UNO_QUERY_THROW );
            xProps->getPropertyValue("Encrypted") >>= bEncrypted;
        }
        catch (const packages::WrongPasswordException&)
        {
            return ERRCODE_SFX_WRONGPASSWORD;
        }
        catch (const uno::Exception&)
        {
            return 1; // TODO/LATER: error handling
        }

        sal_Int32 nArgs = 0;
        if( _xGraphicObjectResolver.is())
            nArgs++;
        if( _xEmbeddedObjectResolver.is())
            nArgs++;
        if ( _xProp.is() )
            nArgs++;

        uno::Sequence< uno::Any > aFilterCompArgs( nArgs );

        nArgs = 0;
        if( _xGraphicObjectResolver.is())
            aFilterCompArgs[nArgs++] <<= _xGraphicObjectResolver;
        if( _xEmbeddedObjectResolver.is())
            aFilterCompArgs[ nArgs++ ] <<= _xEmbeddedObjectResolver;
        if ( _xProp.is() )
            aFilterCompArgs[ nArgs++ ] <<= _xProp;

        Reference< xml::sax::XDocumentHandler > xDocHandler(
            rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(_sFilterName, aFilterCompArgs, rxContext),
            uno::UNO_QUERY_THROW );
        uno::Reference< XInputStream > xInputStream = xDocStream->getInputStream();
        // read from the stream
        return ReadThroughComponent( xInputStream
                                    ,xModelComponent
                                    ,pStreamName
                                    ,rxContext
                                    ,xDocHandler
                                    ,bEncrypted );
    }

    // TODO/LATER: better error handling
    return 1;
}


uno::Reference< uno::XInterface > ORptImportHelper::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return static_cast< XServiceInfo* >(new ORptFilter(xContext, SvXMLImportFlags::SETTINGS ));
}

OUString ORptImportHelper::getImplementationName_Static(  )
{
    return OUString(SERVICE_SETTINGSIMPORTER);
}

Sequence< OUString > ORptImportHelper::getSupportedServiceNames_Static(  )
{
    Sequence< OUString > aSupported { SERVICE_IMPORTFILTER };
    return aSupported;
}

Reference< XInterface > ORptContentImportHelper::create(const Reference< XComponentContext > & xContext)
{
    return static_cast< XServiceInfo* >(new ORptFilter(xContext, SvXMLImportFlags::AUTOSTYLES | SvXMLImportFlags::CONTENT | SvXMLImportFlags::SCRIPTS |
        SvXMLImportFlags::FONTDECLS ));
}

OUString ORptContentImportHelper::getImplementationName_Static(  )
{
    return OUString(SERVICE_CONTENTIMPORTER);
}

Sequence< OUString > ORptContentImportHelper::getSupportedServiceNames_Static(  )
{
    Sequence< OUString > aSupported { SERVICE_IMPORTFILTER };
    return aSupported;
}


Reference< XInterface > ORptStylesImportHelper::create(Reference< XComponentContext > const & xContext)
{
    return static_cast< XServiceInfo* >(new ORptFilter(xContext,
        SvXMLImportFlags::STYLES | SvXMLImportFlags::MASTERSTYLES | SvXMLImportFlags::AUTOSTYLES |
        SvXMLImportFlags::FONTDECLS ));
}

OUString ORptStylesImportHelper::getImplementationName_Static(  )
{
    return OUString(SERVICE_STYLESIMPORTER);
}

Sequence< OUString > ORptStylesImportHelper::getSupportedServiceNames_Static(  )
{
    Sequence< OUString > aSupported { SERVICE_IMPORTFILTER };
    return aSupported;
}


Reference< XInterface > ORptMetaImportHelper::create(Reference< XComponentContext > const & xContext)
{
    return static_cast< XServiceInfo* >(new ORptFilter(xContext,
        SvXMLImportFlags::META));
}

OUString ORptMetaImportHelper::getImplementationName_Static(  )
{
    return OUString(SERVICE_METAIMPORTER);
}

Sequence< OUString > ORptMetaImportHelper::getSupportedServiceNames_Static(  )
{
    Sequence< OUString > aSupported { SERVICE_IMPORTFILTER };
    return aSupported;
}


ORptFilter::ORptFilter( const uno::Reference< XComponentContext >& _rxContext, SvXMLImportFlags nImportFlags )
    :SvXMLImport(_rxContext, getImplementationName_Static(), nImportFlags)
{
    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_100TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);
    GetNamespaceMap().Add( "_report",
                        GetXMLToken(XML_N_RPT),
                        XML_NAMESPACE_REPORT );

    GetNamespaceMap().Add( "__report",
                        GetXMLToken(XML_N_RPT_OASIS),
                        XML_NAMESPACE_REPORT );

    m_xPropHdlFactory = new OXMLRptPropHdlFactory;
    m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylePropertyMap(true, false);
    m_xColumnStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetColumnStyleProps(), m_xPropHdlFactory, false);
    m_xRowStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetRowStyleProps(), m_xPropHdlFactory, false);
    m_xTableStylesPropertySetMapper = new XMLTextPropertySetMapper( TextPropMap::TABLE_DEFAULTS, false );
}


ORptFilter::~ORptFilter() throw()
{
}

uno::Reference< XInterface > ORptFilter::create(uno::Reference< XComponentContext > const & xContext)
{
    return *(new ORptFilter(xContext));
}


OUString ORptFilter::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.report.OReportFilter");
}

uno::Sequence< OUString > ORptFilter::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { SERVICE_IMPORTFILTER };

    return aServices;
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

    const PropertyValue* pIter = rDescriptor.getConstArray();
    const PropertyValue* pEnd   = pIter + rDescriptor.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( pIter->Name == "FileName" )
            pIter->Value >>= sFileName;
        else if ( pIter->Name == "Storage" )
            pIter->Value >>= xStorage;
        else if ( pIter->Name == "ComponentData" )
        {
            Sequence< PropertyValue > aComponent;
            pIter->Value >>= aComponent;
            const PropertyValue* pComponentIter = aComponent.getConstArray();
            const PropertyValue* pComponentEnd  = pComponentIter + aComponent.getLength();
            for(;pComponentIter != pComponentEnd;++pComponentIter)
            {
                if ( pComponentIter->Name == "ActiveConnection" )
                {
                    uno::Reference<sdbc::XConnection> xCon(pComponentIter->Value,uno::UNO_QUERY);
                    xNumberFormatsSupplier = ::dbtools::getNumberFormats(xCon);
                    break;
                }
            }
        }
    }

    if ( !sFileName.isEmpty() )
    {
        uno::Reference<XComponent> xCom(GetModel(),UNO_QUERY);

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
        OSL_ENSURE(m_xReportDefinition.is(),"ReportDefinition is NULL!");
        if ( !m_xReportDefinition.is() )
            return false;

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

        Reference< document::XGraphicObjectResolver > xGraphicObjectResolver;
        uno::Reference<document::XEmbeddedObjectResolver> xEmbeddedObjectResolver;
        uno::Reference< uno::XComponentContext > xContext = GetComponentContext();

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= xStorage;
        xGraphicObjectResolver.set(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.comp.Svx.GraphicImportHelper", aArgs, xContext),
                uno::UNO_QUERY );

        uno::Reference< lang::XMultiServiceFactory > xReportServiceFactory( m_xReportDefinition, uno::UNO_QUERY);
        aArgs[0] <<= beans::NamedValue("Storage",uno::makeAny(xStorage));
        xEmbeddedObjectResolver.set( xReportServiceFactory->createInstanceWithArguments("com.sun.star.document.ImportEmbeddedObjectResolver",aArgs) , uno::UNO_QUERY);

        static const char s_sOld[] = "OldFormat";
        static comphelper::PropertyMapEntry const pMap[] =
        {
            { OUString("OldFormat") , 1,    cppu::UnoType<sal_Bool>::get(),                 beans::PropertyAttribute::BOUND,     0 },
            { OUString("StreamName"), 0,    cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("PrivateData"),0,    cppu::UnoType<XInterface>::get(),  beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("BaseURI"),    0,    cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamRelPath"), 0, cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        utl::MediaDescriptor aDescriptor(rDescriptor);
        uno::Reference<beans::XPropertySet> xProp = comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(pMap));
        const OUString sVal( aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTBASEURL(),OUString()) );
        assert(!sVal.isEmpty()); // needed for relative URLs
        xProp->setPropertyValue("BaseURI", uno::makeAny(sVal));
        const OUString sHierarchicalDocumentName( aDescriptor.getUnpackedValueOrDefault("HierarchicalDocumentName",OUString()) );
        xProp->setPropertyValue("StreamRelPath", uno::makeAny(sHierarchicalDocumentName));

        uno::Reference<XComponent> xModel(GetModel(),UNO_QUERY);
        static const char s_sMeta[] = "meta.xml";
        static const char s_sStreamName[] = "StreamName";
        xProp->setPropertyValue(s_sStreamName, uno::makeAny(OUString(s_sMeta)));
        sal_Int32 nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"meta.xml"
                                    ,"Meta.xml"
                                    ,GetComponentContext()
                                    ,xGraphicObjectResolver
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_METAIMPORTER
                                    ,xProp
                                    );


        try
        {
            xProp->setPropertyValue(s_sOld,uno::makeAny(!(xStorage->hasByName(s_sMeta) || xStorage->isStreamElement( s_sMeta ))));
        }
        catch (const uno::Exception&)
        {
            xProp->setPropertyValue(s_sOld,uno::makeAny(true));
        }

        if ( nRet == 0 )
        {
            xProp->setPropertyValue(s_sStreamName, uno::makeAny(OUString("settings.xml")));
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"settings.xml"
                                    ,"Settings.xml"
                                    ,GetComponentContext()
                                    ,xGraphicObjectResolver
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_SETTINGSIMPORTER
                                    ,xProp
                                    );
        }
        if ( nRet == 0 )
        {
            xProp->setPropertyValue(s_sStreamName, uno::makeAny(OUString("styles.xml")));
            nRet = ReadThroughComponent(xStorage
                                    ,xModel
                                    ,"styles.xml"
                                    ,"Styles.xml"
                                    ,GetComponentContext()
                                    ,xGraphicObjectResolver
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_STYLESIMPORTER
                                    ,xProp);
        }

        if ( nRet == 0 )
        {
            xProp->setPropertyValue(s_sStreamName, uno::makeAny(OUString("content.xml")));
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"content.xml"
                                    ,"Content.xml"
                                    ,GetComponentContext()
                                    ,xGraphicObjectResolver
                                    ,xEmbeddedObjectResolver
                                    ,SERVICE_CONTENTIMPORTER
                                    ,xProp
                                    );
        }


        bRet = nRet == 0;

        if ( bRet )
        {
            m_xReportDefinition->setModified(false);
        }
        else
        {
            switch( nRet )
            {
                case ERRCODE_IO_BROKENPACKAGE:
                    if( xStorage.is() )
                    {
                        // TODO/LATER: no way to transport the error outside from the filter!
                        break;
                    }
                    SAL_FALLTHROUGH;
                default:
                    {
                        // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly! But for now this is the only way!
                        ErrorHandler::HandleError( nRet );
                        if( nRet & ERRCODE_WARNING_MASK )
                            bRet = true;
                    }
            }
        }
    }

    return bRet;
}

SvXMLImportContext* ORptFilter::CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetDocElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DOC_SETTINGS:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new XMLDocumentSettingsContext( *this, nPrefix, rLocalName,xAttrList );
            break;
        case XML_TOK_DOC_REPORT:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            {
                const SvXMLStylesContext* pAutoStyles = GetAutoStyles();
                if ( pAutoStyles )
                {
                    XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext *>(pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_PAGE_MASTER,"pm1")));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(getReportDefinition().get());
                    }
                }
                pContext = new OXMLReport( *this, nPrefix, rLocalName,xAttrList,getReportDefinition() );
            }
            break;
        case XML_TOK_DOC_STYLES:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext( rLocalName, xAttrList, false);
            break;
        case XML_TOK_DOC_AUTOSTYLES:
            // don't use the autostyles from the styles-document for the progress
            if ( ! IsXMLToken( rLocalName, XML_DOCUMENT_STYLES ) )
                GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext( rLocalName, xAttrList, true);
            break;
        case XML_TOK_DOC_FONTDECLS:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateFontDeclsContext( rLocalName,xAttrList );
            break;
        case XML_TOK_DOC_MASTERSTYLES:
            {
                SvXMLStylesContext* pStyleContext = new RptMLMasterStylesContext_Impl(*this, nPrefix, rLocalName,xAttrList);//CreateMasterStylesContext( rLocalName,xAttrList );
                pContext = pStyleContext;
                SetMasterStyles( pStyleContext );
            }
            break;
        case XML_TOK_DOC_META:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateMetaContext( rLocalName,xAttrList );
            break;
        default:
            break;
    }

    if ( !pContext )
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

const SvXMLTokenMap& ORptFilter::GetDocElemTokenMap() const
{
    if ( !m_pDocElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OFFICE, XML_REPORT,             XML_TOK_DOC_REPORT      },
            { XML_NAMESPACE_OOO,    XML_REPORT,             XML_TOK_DOC_REPORT      },
            { XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS,    XML_TOK_DOC_FONTDECLS   },
            { XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,      XML_TOK_DOC_MASTERSTYLES    },
            { XML_NAMESPACE_OFFICE, XML_DOCUMENT_META,      XML_TOK_DOC_META        },
            XML_TOKEN_MAP_END
        };
        m_pDocElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDocElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetReportElemTokenMap() const
{
    if ( !m_pReportElemTokenMap.get() )
        m_pReportElemTokenMap.reset(OXMLHelper::GetReportElemTokenMap());
    return *m_pReportElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetSubDocumentElemTokenMap() const
{
    if ( !m_pSubDocumentElemTokenMap.get() )
        m_pSubDocumentElemTokenMap.reset(OXMLHelper::GetSubDocumentElemTokenMap());
    return *m_pSubDocumentElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetFunctionElemTokenMap() const
{
    if ( !m_pFunctionElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_REPORT, XML_NAME,           XML_TOK_FUNCTION_NAME   },
            { XML_NAMESPACE_REPORT, XML_FORMULA,        XML_TOK_FUNCTION_FORMULA},
            { XML_NAMESPACE_REPORT, XML_PRE_EVALUATED,  XML_TOK_PRE_EVALUATED   },
            { XML_NAMESPACE_REPORT, XML_INITIAL_FORMULA,XML_TOK_INITIAL_FORMULA   },
            { XML_NAMESPACE_REPORT, XML_DEEP_TRAVERSING,XML_TOK_DEEP_TRAVERSING   },
            XML_TOKEN_MAP_END
        };
        m_pFunctionElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pFunctionElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetFormatElemTokenMap() const
{
    if ( !m_pFormatElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_REPORT, XML_ENABLED     , XML_TOK_ENABLED           },
            { XML_NAMESPACE_REPORT, XML_FORMULA     , XML_TOK_FORMULA           },
            { XML_NAMESPACE_REPORT, XML_STYLE_NAME  , XML_TOK_FORMAT_STYLE_NAME },
            XML_TOKEN_MAP_END
        };
        m_pFormatElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pFormatElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetGroupElemTokenMap() const
{
    if ( !m_pGroupElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_REPORT, XML_START_NEW_COLUMN            ,   XML_TOK_START_NEW_COLUMN            },
            { XML_NAMESPACE_REPORT, XML_RESET_PAGE_NUMBER           ,   XML_TOK_RESET_PAGE_NUMBER           },
            { XML_NAMESPACE_REPORT, XML_PRINT_HEADER_ON_EACH_PAGE   ,   XML_TOK_PRINT_HEADER_ON_EACH_PAGE   },
            { XML_NAMESPACE_REPORT, XML_RESET_PAGE_NUMBER           ,   XML_TOK_RESET_PAGE_NUMBER           },
            { XML_NAMESPACE_REPORT, XML_SORT_EXPRESSION             ,   XML_TOK_SORT_EXPRESSION             },
            { XML_NAMESPACE_REPORT, XML_GROUP_EXPRESSION            ,   XML_TOK_GROUP_EXPRESSION            },
            { XML_NAMESPACE_REPORT, XML_GROUP_HEADER                ,   XML_TOK_GROUP_HEADER                },
            { XML_NAMESPACE_REPORT, XML_GROUP                       ,   XML_TOK_GROUP_GROUP                 },
            { XML_NAMESPACE_REPORT, XML_DETAIL                      ,   XML_TOK_GROUP_DETAIL                },
            { XML_NAMESPACE_REPORT, XML_GROUP_FOOTER                ,   XML_TOK_GROUP_FOOTER                },
            { XML_NAMESPACE_REPORT, XML_SORT_ASCENDING              ,   XML_TOK_SORT_ASCENDING              },
            { XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER               ,   XML_TOK_GROUP_KEEP_TOGETHER         },
            { XML_NAMESPACE_REPORT, XML_FUNCTION                    ,   XML_TOK_GROUP_FUNCTION              },
            XML_TOKEN_MAP_END
        };
        m_pGroupElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pGroupElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetReportElementElemTokenMap() const
{
    if ( !m_pElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_REPORT, XML_PRINT_REPEATED_VALUES       ,XML_TOK_PRINT_REPEATED_VALUES              },
            { XML_NAMESPACE_REPORT, XML_PRINT_WHEN_GROUP_CHANGE     ,XML_TOK_PRINT_WHEN_GROUP_CHANGE            },
            { XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION,XML_TOK_REP_CONDITIONAL_PRINT_EXPRESSION   },
            { XML_NAMESPACE_REPORT, XML_REPORT_COMPONENT            ,XML_TOK_COMPONENT                          },
            { XML_NAMESPACE_REPORT, XML_FORMAT_CONDITION            ,XML_TOK_FORMATCONDITION                    },
            XML_TOKEN_MAP_END
        };
        m_pElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetControlElemTokenMap() const
{
    if ( !m_pControlElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_FORM,   XML_LABEL               ,XML_TOK_LABEL              },
            { XML_NAMESPACE_FORM,   XML_PROPERTIES          ,XML_TOK_PROPERTIES         },
            { XML_NAMESPACE_FORM,   XML_SIZE                ,XML_TOK_SIZE               },
            { XML_NAMESPACE_FORM,   XML_IMAGE_DATA          ,XML_TOK_IMAGE_DATA         },
            { XML_NAMESPACE_REPORT, XML_SCALE               ,XML_TOK_SCALE              },
            { XML_NAMESPACE_REPORT, XML_REPORT_ELEMENT      ,XML_TOK_REPORT_ELEMENT     },
            { XML_NAMESPACE_REPORT, XML_FORMULA             ,XML_TOK_DATA_FORMULA       },
            { XML_NAMESPACE_REPORT, XML_PRESERVE_IRI        ,XML_TOK_PRESERVE_IRI       },
            { XML_NAMESPACE_REPORT, XML_SELECT_PAGE         ,XML_TOK_SELECT_PAGE        },
            XML_TOKEN_MAP_END
        };
        m_pControlElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pControlElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetControlPropertyElemTokenMap() const
{
    if ( !m_pControlElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_FORM,   XML_PROPERTY_NAME   ,XML_TOK_PROPERTY_NAME          },
            { XML_NAMESPACE_OOO,    XML_VALUE_TYPE      ,XML_TOK_VALUE_TYPE             },
            { XML_NAMESPACE_FORM,   XML_LIST_PROPERTY   ,XML_TOK_LIST_PROPERTY          },
            { XML_NAMESPACE_OOO,    XML_VALUE           ,XML_TOK_VALUE                  },
            { XML_NAMESPACE_OOO,    XML_CURRENCY        ,XML_TOK_CURRENCY               },
            { XML_NAMESPACE_OOO,    XML_DATE_VALUE      ,XML_TOK_DATE_VALUE             },
            { XML_NAMESPACE_OOO,    XML_TIME_VALUE      ,XML_TOK_TIME_VALUE             },
            { XML_NAMESPACE_OOO,    XML_STRING_VALUE    ,XML_TOK_STRING_VALUE           },
            { XML_NAMESPACE_OOO,    XML_BOOLEAN_VALUE   ,XML_TOK_BOOLEAN_VALUE          },
            XML_TOKEN_MAP_END
        };
        m_pControlElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pControlElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetComponentElemTokenMap() const
{
    if ( !m_pComponentElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DRAW,   XML_NAME                        ,   XML_TOK_NAME                        },
            { XML_NAMESPACE_DRAW,   XML_TEXT_STYLE_NAME             ,   XML_TOK_TEXT_STYLE_NAME             },
            { XML_NAMESPACE_REPORT, XML_TRANSFORM                   ,   XML_TOK_TRANSFORM                   },
            XML_TOKEN_MAP_END
        };
        m_pComponentElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pComponentElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetColumnTokenMap() const
{
    if ( !m_pColumnTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_TABLE,  XML_NAME                        ,   XML_TOK_NAME                        },
            { XML_NAMESPACE_TABLE,  XML_STYLE_NAME                  ,   XML_TOK_COLUMN_STYLE_NAME           },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMN                ,   XML_TOK_COLUMN                      },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROW                   ,   XML_TOK_ROW                         },
            { XML_NAMESPACE_TABLE,  XML_TABLE_CELL                  ,   XML_TOK_CELL                        },
            { XML_NAMESPACE_TABLE,  XML_TABLE_COLUMNS               ,   XML_TOK_TABLE_COLUMNS               },
            { XML_NAMESPACE_TABLE,  XML_TABLE_ROWS                  ,   XML_TOK_TABLE_ROWS                  },
            { XML_NAMESPACE_TABLE,  XML_COVERED_TABLE_CELL          ,   XML_TOK_COV_CELL                    },
            { XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_SPANNED      ,   XML_TOK_NUMBER_COLUMNS_SPANNED      },
            { XML_NAMESPACE_TABLE,  XML_NUMBER_ROWS_SPANNED         ,   XML_TOK_NUMBER_ROWS_SPANNED         },
            { XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION,   XML_TOK_CONDITIONAL_PRINT_EXPRESSION},
            XML_TOKEN_MAP_END
        };
        m_pColumnTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pColumnTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetSectionElemTokenMap() const
{
    if ( !m_pSectionElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_TABLE,  XML_TABLE                       ,   XML_TOK_TABLE                       },
            { XML_NAMESPACE_TABLE,  XML_NAME                        ,   XML_TOK_SECTION_NAME                },
            { XML_NAMESPACE_REPORT, XML_VISIBLE                     ,   XML_TOK_VISIBLE                     },
            { XML_NAMESPACE_REPORT, XML_FORCE_NEW_PAGE              ,   XML_TOK_FORCE_NEW_PAGE              },
            { XML_NAMESPACE_REPORT, XML_FORCE_NEW_COLUMN            ,   XML_TOK_FORCE_NEW_COLUMN            },
            { XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER               ,   XML_TOK_KEEP_TOGETHER               },
            { XML_NAMESPACE_REPORT, XML_REPEAT_SECTION              ,   XML_TOK_REPEAT_SECTION              },
            { XML_NAMESPACE_TABLE,  XML_STYLE_NAME                  ,   XML_TOK_SECT_STYLE_NAME             },
            { XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION           ,   XML_TOK_PAGE_PRINT_OPTION           },

            XML_TOKEN_MAP_END
        };
        m_pSectionElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pSectionElemTokenMap;
}

const SvXMLTokenMap& ORptFilter::GetCellElemTokenMap() const
{
    if ( !m_pCellElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_TEXT,   XML_P                           ,   XML_TOK_P                           },
            { XML_NAMESPACE_REPORT, XML_FIXED_CONTENT               ,   XML_TOK_FIXED_CONTENT               },
            { XML_NAMESPACE_REPORT, XML_FORMATTED_TEXT              ,   XML_TOK_FORMATTED_TEXT              },
            { XML_NAMESPACE_REPORT, XML_IMAGE                       ,   XML_TOK_IMAGE                       },
            { XML_NAMESPACE_REPORT, XML_SUB_DOCUMENT                ,   XML_TOK_SUB_DOCUMENT                },
            { XML_NAMESPACE_DRAW,   XML_CUSTOM_SHAPE                ,   XML_TOK_CUSTOM_SHAPE                },
            { XML_NAMESPACE_DRAW,   XML_FRAME                       ,   XML_TOK_FRAME                       },
            { XML_NAMESPACE_TEXT,   XML_PAGE_NUMBER                 ,   XML_TOK_PAGE_NUMBER                 },
            { XML_NAMESPACE_TEXT,   XML_PAGE_COUNT                  ,   XML_TOK_PAGE_COUNT                  },
            { XML_NAMESPACE_TEXT,   XML_TAB                         ,   XML_TOK_TEXT_TAB_STOP               },
            { XML_NAMESPACE_TEXT,   XML_LINE_BREAK                  ,   XML_TOK_TEXT_LINE_BREAK             },
            { XML_NAMESPACE_TEXT,   XML_S                           ,   XML_TOK_TEXT_S                      },
            XML_TOKEN_MAP_END
        };
        m_pCellElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pCellElemTokenMap;
}

SvXMLImportContext* ORptFilter::CreateStylesContext(const OUString& rLocalName,
                                     const uno::Reference< XAttributeList>& xAttrList, bool bIsAutoStyle )
{
    SvXMLImportContext* pContext = bIsAutoStyle ? GetAutoStyles() : GetStyles();
    if ( !pContext )
    {
        pContext = new OReportStylesContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, bIsAutoStyle);
        if (bIsAutoStyle)
            SetAutoStyles(static_cast<SvXMLStylesContext*>(pContext));
        else
            SetStyles(static_cast<SvXMLStylesContext*>(pContext));
    }
    return pContext;
}

SvXMLImportContext *ORptFilter::CreateFontDeclsContext(
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, XML_NAMESPACE_OFFICE,
                                      rLocalName, xAttrList,
                                      osl_getThreadTextEncoding() );
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
    OSL_ENSURE(m_xReportDefinition.is(),"ReportDefinition is NULL!");
    if ( m_xReportDefinition.is() )
    {
        m_pReportModel = reportdesign::OReportDefinition::getSdrModel(m_xReportDefinition);
        OSL_ENSURE(m_pReportModel,"Report model is NULL!");

        SvXMLImport::startDocument();
    }
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
    m_aFunctions.insert(TGroupFunctionMap::value_type(_xFunction->getName(),_xFunction));
}

SvXMLImportContext* ORptFilter::CreateMetaContext(const OUString& rLocalName,const uno::Reference<xml::sax::XAttributeList>&)
{
    SvXMLImportContext* pContext = nullptr;

    if ( (getImportFlags() & SvXMLImportFlags::META) )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(), uno::UNO_QUERY_THROW);
        pContext = new SvXMLMetaDocumentContext(*this,XML_NAMESPACE_OFFICE, rLocalName,xDPS->getDocumentProperties());
    }
    return pContext;
}

bool ORptFilter::isOldFormat() const
{
    bool bOldFormat = true;
    uno::Reference<beans::XPropertySet> xProp = getImportInfo();
    if ( xProp.is() )
    {
        static const char s_sOld[] = "OldFormat";
        if ( xProp->getPropertySetInfo()->hasPropertyByName(s_sOld))
        {
            xProp->getPropertyValue(s_sOld) >>= bOldFormat;
        }
    }
    return bOldFormat;
}


}// rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

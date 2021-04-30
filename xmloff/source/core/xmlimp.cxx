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

#include <config_wasm_strip.h>

#include <memory>
#include <optional>

#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <tools/urlobj.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <vcl/graph.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <XMLEventImportHelper.hxx>
#include <XMLStarBasicContextFactory.hxx>
#include <XMLScriptContextFactory.hxx>
#include <StyleMap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmlerror.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <comphelper/fileformat.h>
#include <comphelper/namecontainer.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/documentinfo.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/attributelist.hxx>
#include <unotools/fontcvt.hxx>
#include <fasttokenhandler.hxx>
#include <vcl/GraphicExternalLink.hxx>

#include <com/sun/star/rdf/XMetadatable.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include <RDFaImportHelper.hxx>

using ::com::sun::star::beans::XPropertySetInfo;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;

rtl::Reference< FastTokenHandler > SvXMLImport::xTokenHandler( new FastTokenHandler() );
std::unordered_map< sal_Int32, std::pair< OUString, OUString > > SvXMLImport::aNamespaceMap;
std::unordered_map< OUString, OUString > SvXMLImport::aNamespaceURIPrefixMap;
bool SvXMLImport::bIsNSMapsInitialized = false;

namespace {

class SvXMLImportEventListener : public cppu::WeakImplHelper< css::lang::XEventListener >
{
private:
    SvXMLImport*    pImport;

public:
    explicit SvXMLImportEventListener(SvXMLImport* pImport);

                            // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& rEventObject) override;
};

}

SvXMLImportEventListener::SvXMLImportEventListener(SvXMLImport* pTempImport)
    : pImport(pTempImport)
{
}

// XEventListener
void SAL_CALL SvXMLImportEventListener::disposing( const lang::EventObject& )
{
    if (pImport)
    {
        pImport->DisposingModel();
        pImport = nullptr;
    }
}

namespace
{

OUString
getBuildIdsProperty(uno::Reference<beans::XPropertySet> const& xImportInfo)
{
    if (xImportInfo.is())
    {
        try
        {
            Reference< XPropertySetInfo > const xSetInfo(
                    xImportInfo->getPropertySetInfo());
            if (xSetInfo.is() && xSetInfo->hasPropertyByName("BuildId"))
            {
                OUString aBuildId;
                xImportInfo->getPropertyValue("BuildId") >>= aBuildId;
                return aBuildId;
            }
        }
        catch (Exception const&)
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.core", "exception getting BuildId");
        }
    }
    return OUString();
}

class DocumentInfo
{
private:
    sal_uInt16 mnGeneratorVersion;

public:
    explicit DocumentInfo( const SvXMLImport& rImport )
        : mnGeneratorVersion( SvXMLImport::ProductVersionUnknown )
    {
        OUString const buildIds(
                getBuildIdsProperty(rImport.getImportInfo()));
        if (!buildIds.isEmpty())
        {
            sal_Int32 const ix = buildIds.indexOf(';');
            if (-1 != ix)
            {
                OUString const loVersion(buildIds.copy(ix + 1));
                if (!loVersion.isEmpty())
                {
                    if ('3' == loVersion[0])
                    {
                        mnGeneratorVersion = SvXMLImport::LO_3x;
                    }
                    else if ('4' == loVersion[0])
                    {
                        if (loVersion.getLength() > 1
                            && (loVersion[1] == '0' || loVersion[1] == '1'))
                        {
                            mnGeneratorVersion = SvXMLImport::LO_41x; // 4.0/4.1
                        }
                        else if (loVersion.getLength() > 1 && '2' == loVersion[1])
                        {
                            mnGeneratorVersion = SvXMLImport::LO_42x; // 4.2
                        }
                        else if (loVersion.getLength() > 1 && '3' == loVersion[1])
                        {
                            mnGeneratorVersion = SvXMLImport::LO_43x; // 4.3
                        }
                        else if (loVersion.getLength() > 1 && '4' == loVersion[1])
                        {
                            mnGeneratorVersion = SvXMLImport::LO_44x; // 4.4
                        }
                    }
                    else if ('5' == loVersion[0])
                    {
                        mnGeneratorVersion = SvXMLImport::LO_5x;
                    }
                    else if ('6' == loVersion[0])
                    {
                        if (loVersion.getLength() > 1
                            && (loVersion[1] == '0' || loVersion[1] == '1'
                                || loVersion[1] == '2'))
                        {
                            mnGeneratorVersion = SvXMLImport::LO_6x; // 6.0/6.1/6.2
                        }
                        else
                        {
                            mnGeneratorVersion = SvXMLImport::LO_63x; // 6.3/6.4
                        }
                    }
                    else if ('7' == loVersion[0])
                    {
                        mnGeneratorVersion = SvXMLImport::LO_7x;
                    }
                    else
                    {
                        SAL_INFO("xmloff.core", "unknown LO version: " << loVersion);
                    }
                    return; // ignore buildIds
                }
            }
        }
        sal_Int32 nUPD, nBuild;
        if ( !rImport.getBuildIds( nUPD, nBuild ) )
            return;

        if ( nUPD >= 640 && nUPD <= 645 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_1x;
        }
        else if ( nUPD == 680 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_2x;
        }
        else if ( nUPD == 300 && nBuild <= 9379 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_30x;
        }
        else if ( nUPD == 310 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_31x;
        }
        else if ( nUPD == 320 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_32x;
        }
        else if ( nUPD == 330 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_33x;
        }
        else if ( nUPD == 340 )
        {
            mnGeneratorVersion = SvXMLImport::OOo_34x;
        }
        else if (nUPD == 400 || nUPD == 401)
        {
            mnGeneratorVersion = SvXMLImport::AOO_40x;
        }
        else if (nUPD >= 410)
        {
            // effectively this means "latest", see use
            // in XMLGraphicsDefaultStyle::SetDefaults()!
            mnGeneratorVersion = SvXMLImport::AOO_4x;
        }
    }

    sal_uInt16 getGeneratorVersion() const
    {
        return mnGeneratorVersion;
    }
};

}

class SvXMLImport_Impl
{
public:
    FontToSubsFontConverter hBatsFontConv;
    FontToSubsFontConverter hMathFontConv;

    bool mbOwnGraphicResolver;
    bool mbOwnEmbeddedResolver;
    INetURLObject aBaseURL;
    INetURLObject aDocBase;

    /// name of stream in package, e.g., "content.xml"
    OUString mStreamName;

    std::optional<OUString> mxODFVersion;

    bool mbIsOOoXML;

    // Boolean, indicating that position attributes
    // of shapes are given in horizontal left-to-right layout. This is the case
    // for the OpenOffice.org file format. (#i28749#)
    bool mbShapePositionInHoriL2R;
    bool mbTextDocInOOoFileFormat;

    const uno::Reference< uno::XComponentContext > mxComponentContext;
    OUString implementationName;
    css::uno::Sequence< OUString > maSupportedServiceNames;

    uno::Reference< embed::XStorage > mxSourceStorage;

    std::unique_ptr< xmloff::RDFaImportHelper > mpRDFaHelper;

    std::unique_ptr< DocumentInfo > mpDocumentInfo;

    SvXMLImport_Impl( const uno::Reference< uno::XComponentContext >& rxContext,
                      OUString const & theImplementationName,
                      const css::uno::Sequence< OUString > & sSupportedServiceNames = {})
        : hBatsFontConv( nullptr )
        , hMathFontConv( nullptr )
        , mbOwnGraphicResolver( false )
        , mbOwnEmbeddedResolver( false )
        , mbIsOOoXML(false)
        // Convert drawing object positions from OOo file format to OASIS (#i28749#)
        , mbShapePositionInHoriL2R( false )
        , mbTextDocInOOoFileFormat( false )
        , mxComponentContext( rxContext )
        , implementationName(theImplementationName)
        , maSupportedServiceNames(sSupportedServiceNames)
    {
        SAL_WARN_IF(!mxComponentContext.is(), "xmloff.core", "SvXMLImport: no ComponentContext");
        if (!mxComponentContext.is()) throw uno::RuntimeException();
        if (!maSupportedServiceNames.hasElements())
            maSupportedServiceNames = { "com.sun.star.document.ImportFilter", "com.sun.star.xml.XMLImportFilter" };
    }

    sal_uInt16 getGeneratorVersion( const SvXMLImport& rImport )
    {
        if (!mpDocumentInfo)
        {
            mpDocumentInfo.reset( new DocumentInfo( rImport ) );
        }

        return mpDocumentInfo->getGeneratorVersion();
    }

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper maInterfaceToIdentifierMapper;
};

SvXMLImportContext *SvXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    assert(false);
    SAL_WARN( "xmloff.core", "CreateFastContext should be overridden, for element " << nElement);
    return new SvXMLImportContext( *this );
}

void SvXMLImport::InitCtor_()
{
    if( mnImportFlags != SvXMLImportFlags::NONE )
    {
        // implicit "xml" namespace prefix
        mxNamespaceMap->Add( GetXMLToken(XML_XML), GetXMLToken(XML_N_XML), XML_NAMESPACE_XML );
        mxNamespaceMap->Add( "_office", GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
        mxNamespaceMap->Add( "_office_ooo", GetXMLToken(XML_N_OFFICE_EXT), XML_NAMESPACE_OFFICE_EXT );
        mxNamespaceMap->Add( "_ooo", GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
        mxNamespaceMap->Add( "_style", GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
        mxNamespaceMap->Add( "_text", GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );
        mxNamespaceMap->Add( "_table", GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
        mxNamespaceMap->Add( "_table_ooo", GetXMLToken(XML_N_TABLE_EXT), XML_NAMESPACE_TABLE_EXT );
        mxNamespaceMap->Add( "_draw", GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
        mxNamespaceMap->Add( "_draw_ooo", GetXMLToken(XML_N_DRAW_EXT), XML_NAMESPACE_DRAW_EXT );
        mxNamespaceMap->Add( "_dr3d", GetXMLToken(XML_N_DR3D), XML_NAMESPACE_DR3D );
        mxNamespaceMap->Add( "_fo", GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );
        mxNamespaceMap->Add( "_xlink", GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
        mxNamespaceMap->Add( "_dc", GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
        mxNamespaceMap->Add( "_dom", GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
        mxNamespaceMap->Add( "_meta", GetXMLToken(XML_N_META), XML_NAMESPACE_META );
        mxNamespaceMap->Add( "_number", GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );
        mxNamespaceMap->Add( "_svg", GetXMLToken(XML_N_SVG_COMPAT), XML_NAMESPACE_SVG );
        mxNamespaceMap->Add( "_chart", GetXMLToken(XML_N_CHART), XML_NAMESPACE_CHART );
        mxNamespaceMap->Add( "_math", GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
        mxNamespaceMap->Add( "_form", GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
        mxNamespaceMap->Add( "_script", GetXMLToken(XML_N_SCRIPT), XML_NAMESPACE_SCRIPT );
        mxNamespaceMap->Add( "_config", GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
        mxNamespaceMap->Add( "_xforms", GetXMLToken(XML_N_XFORMS_1_0), XML_NAMESPACE_XFORMS );
        mxNamespaceMap->Add( "_formx", GetXMLToken( XML_N_FORMX ), XML_NAMESPACE_FORMX );
        mxNamespaceMap->Add( "_xsd", GetXMLToken(XML_N_XSD), XML_NAMESPACE_XSD );
        mxNamespaceMap->Add( "_xsi", GetXMLToken(XML_N_XSI), XML_NAMESPACE_XFORMS );
        mxNamespaceMap->Add( "_ooow", GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
        mxNamespaceMap->Add( "_oooc", GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
        mxNamespaceMap->Add( "_field", GetXMLToken(XML_N_FIELD), XML_NAMESPACE_FIELD );
        mxNamespaceMap->Add( "_of", GetXMLToken(XML_N_OF), XML_NAMESPACE_OF );
        mxNamespaceMap->Add( "_xhtml", GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
        mxNamespaceMap->Add( "_css3text", GetXMLToken(XML_N_CSS3TEXT), XML_NAMESPACE_CSS3TEXT );

        mxNamespaceMap->Add( "_calc_libo", GetXMLToken(XML_N_CALC_EXT), XML_NAMESPACE_CALC_EXT);
        mxNamespaceMap->Add( "_office_libo",
                             GetXMLToken(XML_N_LO_EXT), XML_NAMESPACE_LO_EXT);
    }

    if (mxNumberFormatsSupplier.is())
        mpNumImport = std::make_unique<SvXMLNumFmtHelper>(mxNumberFormatsSupplier, GetComponentContext());

    if (mxModel.is() && !mxEventListener.is())
    {
        mxEventListener.set(new SvXMLImportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }
}

SvXMLImport::SvXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName,
    SvXMLImportFlags nImportFlags,
    const css::uno::Sequence< OUString > & sSupportedServiceNames )
:   mpImpl( new SvXMLImport_Impl(xContext, implementationName, sSupportedServiceNames) ),
    mxNamespaceMap( SvXMLNamespaceMap() ),

    mpUnitConv( new SvXMLUnitConverter( xContext,
                util::MeasureUnit::MM_100TH, util::MeasureUnit::MM_100TH,
                SvtSaveOptions::ODFSVER_LATEST_EXTENDED) ),

    mnImportFlags( nImportFlags ),
    maNamespaceHandler( new SvXMLImportFastNamespaceHandler() ),
    mbIsFormsSupported( true ),
    mbIsTableShapeSupported( false ),
    mbNotifyMacroEventRead( false )
{
    SAL_WARN_IF( !xContext.is(), "xmloff.core", "got no service manager" );
    InitCtor_();
    mxParser = xml::sax::FastParser::create( xContext );
    setNamespaceHandler( maNamespaceHandler );
    setTokenHandler( xTokenHandler  );
    if ( !bIsNSMapsInitialized )
    {
        initializeNamespaceMaps();
        bIsNSMapsInitialized = true;
    }
    registerNamespaces();
    maNamespaceAttrList = new comphelper::AttributeList;
}

void SvXMLImport::cleanup() noexcept
{
    if (mxEventListener.is() && mxModel.is())
        mxModel->removeEventListener(mxEventListener);
    // clear context stacks first in case of parse error because the context
    // class dtors are full of application logic
    while (!maContexts.empty())
    {
        if (SvXMLStylesContext* pStylesContext = dynamic_cast<SvXMLStylesContext*>(maContexts.top().get()))
            pStylesContext->dispose();
        maContexts.pop();
    }
    if( mxTextImport )
        mxTextImport->dispose();
    mxTextImport.clear(); // XMLRedlineImportHelper needs model
    DisposingModel();
}

SvXMLImport::~SvXMLImport() noexcept
{
    cleanup();
}

bool SvXMLImport::addEmbeddedFont(const css::uno::Reference< css::io::XInputStream >& stream,
                                  const OUString& fontName, const char* extra,
                                  std::vector<unsigned char> const & key, bool eot)
{
    if (!mxEmbeddedFontHelper)
        mxEmbeddedFontHelper.reset(new EmbeddedFontsHelper);
    return mxEmbeddedFontHelper->addEmbeddedFont(stream, fontName, extra, key, eot);
}

const css::uno::Sequence<sal_Int8>& SvXMLImport::getUnoTunnelId() noexcept
{
    static const comphelper::UnoIdInit theSvXMLImportUnoTunnelId;
    return theSvXMLImportUnoTunnelId.getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLImport::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    return comphelper::getSomethingImpl(rId, this);
}

namespace
{
    class setFastDocumentHandlerGuard
    {
    private:
        css::uno::Reference<css::xml::sax::XFastParser> mxParser;
    public:
        setFastDocumentHandlerGuard(const css::uno::Reference<css::xml::sax::XFastParser>& Parser,
                                    const css::uno::Reference<css::xml::sax::XFastDocumentHandler>& Handler)
            : mxParser(Parser)
        {
            mxParser->setFastDocumentHandler(Handler);
        }
        //guarantee restoration of null document handler
        ~setFastDocumentHandlerGuard()
        {
            mxParser->setFastDocumentHandler(nullptr);
        }
    };
}

// XFastParser
void SAL_CALL SvXMLImport::parseStream( const xml::sax::InputSource& aInputSource )
{
    setFastDocumentHandlerGuard aDocumentHandlerGuard(mxParser, mxFastDocumentHandler.is() ? mxFastDocumentHandler : this);
    mxParser->parseStream(aInputSource);
}

void SAL_CALL SvXMLImport::setFastDocumentHandler( const uno::Reference< xml::sax::XFastDocumentHandler >& Handler )
{
    mxFastDocumentHandler = Handler;
}

void SAL_CALL SvXMLImport::setTokenHandler( const uno::Reference< xml::sax::XFastTokenHandler >& Handler )
{
    mxParser->setTokenHandler( Handler );
}

void SAL_CALL SvXMLImport::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken )
{
    mxParser->registerNamespace( NamespaceURL, NamespaceToken );
}

OUString SAL_CALL SvXMLImport::getNamespaceURL( const OUString& rPrefix )
{
    return mxParser->getNamespaceURL( rPrefix );
}

void SAL_CALL SvXMLImport::setErrorHandler( const uno::Reference< xml::sax::XErrorHandler >& Handler )
{
    mxParser->setErrorHandler( Handler );
}

void SAL_CALL SvXMLImport::setEntityResolver( const uno::Reference< xml::sax::XEntityResolver >& Resolver )
{
    mxParser->setEntityResolver( Resolver );
}

void SAL_CALL SvXMLImport::setLocale( const lang::Locale& rLocale )
{
    mxParser->setLocale( rLocale );
}

void SAL_CALL SvXMLImport::setNamespaceHandler( const uno::Reference< xml::sax::XFastNamespaceHandler >& Handler)
{
    mxParser->setNamespaceHandler( Handler );
}

void SAL_CALL SvXMLImport::setCustomEntityNames( const ::css::uno::Sequence< ::css::beans::Pair<::rtl::OUString, ::rtl::OUString> >& replacements )
{
    mxParser->setCustomEntityNames( replacements );
}

void SAL_CALL SvXMLImport::startDocument()
{
    SAL_INFO( "xmloff.core", "{ SvXMLImport::startDocument" );
    if (mxGraphicStorageHandler.is() && mxEmbeddedResolver.is())
        return;

    Reference< lang::XMultiServiceFactory > xFactory( mxModel,  UNO_QUERY );
    if( !xFactory.is() )
        return;

    try
    {
        if (!mxGraphicStorageHandler.is())
        {
            // #99870# Import... instead of Export...
            mxGraphicStorageHandler.set(
                xFactory->createInstance("com.sun.star.document.ImportGraphicStorageHandler"),
                UNO_QUERY);
            mpImpl->mbOwnGraphicResolver = mxGraphicStorageHandler.is();
        }

        if( !mxEmbeddedResolver.is() )
        {
            // #99870# Import... instead of Export...
            mxEmbeddedResolver.set(
                xFactory->createInstance("com.sun.star.document.ImportEmbeddedObjectResolver"),
                UNO_QUERY);
            mpImpl->mbOwnEmbeddedResolver = mxEmbeddedResolver.is();
        }
    }
    catch( css::uno::Exception& )
    {
    }
}

void SAL_CALL SvXMLImport::endDocument()
{
    SAL_INFO( "xmloff.core", "} SvXMLImport::endDocument" );
    //  #i9518# All the stuff that accesses the document has to be done here, not in the dtor,
    //  because the SvXMLImport dtor might not be called until after the document has been closed.

    GetTextImport()->MapCrossRefHeadingFieldsHorribly();

    if (mpImpl->mpRDFaHelper)
    {
        const uno::Reference<rdf::XRepositorySupplier> xRS(mxModel,
            uno::UNO_QUERY);
        if (xRS.is())
        {
            mpImpl->mpRDFaHelper->InsertRDFa( xRS );
        }
    }

    mpNumImport.reset();
    if (mxImportInfo.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            if (bool(mpProgressBarHelper))
            {
                OUString sProgressMax(XML_PROGRESSMAX);
                OUString sProgressCurrent(XML_PROGRESSCURRENT);
                OUString sRepeat(XML_PROGRESSREPEAT);
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                {
                    sal_Int32 nProgressMax(mpProgressBarHelper->GetReference());
                    sal_Int32 nProgressCurrent(mpProgressBarHelper->GetValue());
                    mxImportInfo->setPropertyValue(sProgressMax, uno::Any(nProgressMax));
                    mxImportInfo->setPropertyValue(sProgressCurrent, uno::Any(nProgressCurrent));
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                    mxImportInfo->setPropertyValue(sRepeat, css::uno::makeAny(mpProgressBarHelper->GetRepeat()));
                // pProgressBarHelper is deleted in dtor
            }
            OUString sNumberStyles(XML_NUMBERSTYLES);
            if (mxNumberStyles.is() && xPropertySetInfo->hasPropertyByName(sNumberStyles))
            {
                mxImportInfo->setPropertyValue(sNumberStyles, Any(mxNumberStyles));
            }
        }
    }

    if( mxFontDecls.is() )
        mxFontDecls->dispose();
    if( mxStyles.is() )
        mxStyles->dispose();
    if( mxAutoStyles.is() )
        mxAutoStyles->dispose();
    if( mxMasterStyles.is() )
        mxMasterStyles->dispose();

    // possible form-layer related knittings which can only be done when
    // the whole document exists
    if ( mxFormImport.is() )
        mxFormImport->documentDone();

    //  The shape import helper does the z-order sorting in the dtor,
    //  so it must be deleted here, too.
    mxShapeImport = nullptr;

    if( mpImpl->mbOwnGraphicResolver )
    {
        Reference<lang::XComponent> xComp(mxGraphicStorageHandler, UNO_QUERY);
        xComp->dispose();
    }

    if( mpImpl->mbOwnEmbeddedResolver )
    {
        Reference< lang::XComponent > xComp( mxEmbeddedResolver, UNO_QUERY );
        xComp->dispose();
    }
    mpStyleMap.clear();

    if ( bool( mpXMLErrors ) )
    {
        mpXMLErrors->ThrowErrorAsSAXException( XMLERROR_FLAG_SEVERE );
    }
}

std::optional<SvXMLNamespaceMap> SvXMLImport::processNSAttributes(
        std::optional<SvXMLNamespaceMap> & rpNamespaceMap,
        SvXMLImport *const pImport, // TODO???
        const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    std::optional<SvXMLNamespaceMap> pRewindMap;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if (pImport && rAttrName == "office:version" && !pImport->mpImpl->mxODFVersion)
        {
            pImport->mpImpl->mxODFVersion = xAttrList->getValueByIndex( i );

            // the ODF version in content.xml and manifest.xml must be the same starting from ODF1.2
            if (pImport->mpImpl->mStreamName == "content.xml"
                && !pImport->IsODFVersionConsistent(*pImport->mpImpl->mxODFVersion))
            {
                throw xml::sax::SAXException("Inconsistent ODF versions in content.xml and manifest.xml!",
                        uno::Reference< uno::XInterface >(),
                        uno::makeAny(
                            packages::zip::ZipIOException("Inconsistent ODF versions in content.xml and manifest.xml!" ) ) );
            }
        }
        else if( ( rAttrName.getLength() >= 5 ) &&
            ( rAttrName.startsWith( GetXMLToken(XML_XMLNS) ) ) &&
            ( rAttrName.getLength() == 5 || ':' == rAttrName[5] ) )
        {
            if( !pRewindMap )
            {
                pRewindMap = std::move(rpNamespaceMap);
                rpNamespaceMap.emplace(*pRewindMap);
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 )
                                 ? OUString()
                                 : rAttrName.copy( 6 ) );
            // Add namespace, but only if it is known.
            sal_uInt16 nKey = rpNamespaceMap->AddIfKnown( aPrefix, rAttrValue );
            // If namespace is unknown, try to match a name with similar
            // TC Id and version
            if( XML_NAMESPACE_UNKNOWN == nKey  )
            {
                OUString aTestName( rAttrValue );
                if( SvXMLNamespaceMap::NormalizeURI( aTestName ) )
                    nKey = rpNamespaceMap->AddIfKnown( aPrefix, aTestName );
            }
            // If that namespace is not known, too, add it as unknown
            if( XML_NAMESPACE_UNKNOWN == nKey  )
                rpNamespaceMap->Add( aPrefix, rAttrValue );

        }
    }
    return pRewindMap;
}


void SAL_CALL SvXMLImport::characters( const OUString& rChars )
{
    maContexts.top()->characters( rChars );
}

void SAL_CALL SvXMLImport::processingInstruction( const OUString&,
                                       const OUString& )
{
}

void SAL_CALL SvXMLImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& rLocator )
{
    mxLocator = rLocator;
}

// XFastContextHandler
void SAL_CALL SvXMLImport::startFastElement (sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    SAL_INFO("xmloff.core", "startFastElement " << SvXMLImport::getNameFromToken( Element ));
    if ( Attribs.is() && !mpImpl->mxODFVersion)
    {
        sax_fastparser::FastAttributeList& rAttribList =
            sax_fastparser::castToFastAttributeList( Attribs );
        auto aIter( rAttribList.find( XML_ELEMENT( OFFICE, XML_VERSION ) ) );
        if( aIter != rAttribList.end() )
        {
            mpImpl->mxODFVersion = aIter.toString();

            // the ODF version in content.xml and manifest.xml must be the same starting from ODF1.2
            if ( mpImpl->mStreamName == "content.xml" && !IsODFVersionConsistent( *mpImpl->mxODFVersion ) )
            {
                throw xml::sax::SAXException("Inconsistent ODF versions in content.xml and manifest.xml!",
                        uno::Reference< uno::XInterface >(),
                        uno::makeAny(
                            packages::zip::ZipIOException("Inconsistent ODF versions in content.xml and manifest.xml!" ) ) );
            }
        }
    }

    maNamespaceAttrList->Clear();

    maNamespaceHandler->addNSDeclAttributes( maNamespaceAttrList );
    std::optional<SvXMLNamespaceMap> pRewindMap = processNSAttributes(mxNamespaceMap, this, maNamespaceAttrList);

    SvXMLImportContextRef xContext;
    const bool bRootContext = maContexts.empty();
    if (!maContexts.empty())
    {
        const SvXMLImportContextRef & pHandler = maContexts.top();
        SAL_INFO("xmloff.core", "calling createFastChildContext on " << typeid(*pHandler.get()).name());
        auto tmp = pHandler->createFastChildContext( Element, Attribs );
        xContext = static_cast<SvXMLImportContext*>(tmp.get());
        assert((tmp && xContext) || (!tmp && !xContext));
    }
    else
        xContext.set( CreateFastContext( Element, Attribs ) );

    SAL_INFO_IF(!xContext.is(), "xmloff.core", "No fast context for element " << getNameFromToken(Element));
    if (bRootContext && !xContext)
    {
        OUString aName = getNameFromToken(Element);
        SetError( XMLERROR_FLAG_SEVERE | XMLERROR_UNKNOWN_ROOT,
                  { aName }, "Root element " + aName + " unknown", Reference<xml::sax::XLocator>() );
    }
    if ( !xContext )
        xContext.set( new SvXMLImportContext( *this ) );

    // Remember old namespace map.
    if( pRewindMap )
        xContext->PutRewindMap(std::move(pRewindMap));

    // Call a startElement at the new context.
    xContext->startFastElement( Element, Attribs );

    // Push context on stack.
    maContexts.push(xContext);
}

void SAL_CALL SvXMLImport::startUnknownElement (const OUString & rNamespace, const OUString & rName,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    SAL_INFO("xmloff.core", "startUnknownElement " << rNamespace << " " << rName);
    SvXMLImportContextRef xContext;
    const bool bRootContext = maContexts.empty();
    if (!maContexts.empty())
    {
        const SvXMLImportContextRef & pHandler = maContexts.top();
        SAL_INFO("xmloff.core", "calling createUnknownChildContext on " << typeid(*pHandler.get()).name());
        auto tmp = pHandler->createUnknownChildContext( rNamespace, rName, Attribs );
        xContext = static_cast<SvXMLImportContext*>(tmp.get());
        assert((tmp && xContext) || (!tmp && !xContext));
    }
    else
        xContext.set( CreateFastContext( -1, Attribs ) );

    SAL_WARN_IF(!xContext.is(), "xmloff.core", "No context for unknown-element " << rNamespace << " " << rName);
    if (bRootContext && !xContext)
    {
        SetError( XMLERROR_FLAG_SEVERE | XMLERROR_UNKNOWN_ROOT,
                  { rName }, "Root element " + rName + " unknown", Reference<xml::sax::XLocator>() );
    }
    if (!xContext)
    {
        if (!maContexts.empty())
            // This is pretty weird, but it's what the code did before I simplified it, and some parts of the
            // code rely on this behaviour e.g. DocumentBuilderContext
            xContext = maContexts.top();
        else
            xContext = new SvXMLImportContext( *this );
    }

    xContext->startUnknownElement( rNamespace, rName, Attribs );
    maContexts.push(xContext);
}

void SAL_CALL SvXMLImport::endFastElement (sal_Int32 Element)
{
    SAL_INFO("xmloff.core", "endFastElement " << SvXMLImport::getNameFromToken( Element ));
    if (maContexts.empty())
    {
        SAL_WARN("xmloff.core", "SvXMLImport::endFastElement: no context left");
        assert(false);
        return;
    }
    SvXMLImportContextRef xContext = std::move(maContexts.top());
    // Get a namespace map to rewind.
    std::optional<SvXMLNamespaceMap> pRewindMap = xContext->TakeRewindMap();
    maContexts.pop();
    xContext->endFastElement( Element );
    // Rewind a namespace map.
    if (pRewindMap)
        mxNamespaceMap = std::move(pRewindMap);
}

void SAL_CALL SvXMLImport::endUnknownElement (const OUString & rPrefix, const OUString & rLocalName)
{
    SAL_INFO("xmloff.core", "endUnknownElement " << rPrefix << " " << rLocalName);
    if (maContexts.empty())
    {
        SAL_WARN("xmloff.core", "SvXMLImport::endUnknownElement: no context left");
        assert(false);
        return;
    }
    SvXMLImportContextRef xContext = std::move(maContexts.top());
    maContexts.pop();
    xContext->endUnknownElement( rPrefix, rLocalName );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    SvXMLImport::createFastChildContext (sal_Int32,
    const uno::Reference< xml::sax::XFastAttributeList > &)
{
    return this;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    SvXMLImport::createUnknownChildContext (const OUString &, const OUString &,
    const uno::Reference< xml::sax::XFastAttributeList > &)
{
    return this;
}

void SvXMLImport::SetStatistics(const uno::Sequence< beans::NamedValue> &)
{
    GetProgressBarHelper()->SetRepeat(false);
    GetProgressBarHelper()->SetReference(0);
}

// XImporter
void SAL_CALL SvXMLImport::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
{
    mxModel.set( xDoc, UNO_QUERY );
    if( !mxModel.is() )
        throw lang::IllegalArgumentException();

    try
    {
        uno::Reference<document::XStorageBasedDocument> const xSBDoc(mxModel, uno::UNO_QUERY);
        uno::Reference<embed::XStorage> const xStor(xSBDoc.is() ? xSBDoc->getDocumentStorage()
                                                                : nullptr);
        if (xStor.is())
        {
            mpImpl->mbIsOOoXML =
                ::comphelper::OStorageHelper::GetXStorageFormat(xStor)
                   < SOFFICE_FILEFORMAT_8;
        }
    }
    catch (uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.core");
    }
    if (!mxEventListener.is())
    {
        mxEventListener.set(new SvXMLImportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }

    SAL_WARN_IF( bool(mpNumImport), "xmloff.core", "number format import already exists." );
    mpNumImport.reset();
}

// XFilter
sal_Bool SAL_CALL SvXMLImport::filter( const uno::Sequence< beans::PropertyValue >& )
{
    return false;
}

void SAL_CALL SvXMLImport::cancel(  )
{
}

// XInitialize
void SAL_CALL SvXMLImport::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    for( const auto& rAny : aArguments )
    {
        Reference<XInterface> xValue;
        rAny >>= xValue;

        uno::Reference<task::XStatusIndicator> xTmpStatusIndicator(
            xValue, UNO_QUERY );
        if( xTmpStatusIndicator.is() )
            mxStatusIndicator = xTmpStatusIndicator;

        uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler(xValue, UNO_QUERY);
        if (xGraphicStorageHandler.is())
            mxGraphicStorageHandler = xGraphicStorageHandler;

        uno::Reference<document::XEmbeddedObjectResolver> xTmpObjectResolver(
            xValue, UNO_QUERY );
        if( xTmpObjectResolver.is() )
            mxEmbeddedResolver = xTmpObjectResolver;

        uno::Reference<beans::XPropertySet> xTmpPropSet( xValue, UNO_QUERY );
        if( xTmpPropSet.is() )
        {
            mxImportInfo = xTmpPropSet;
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sPropName(XML_NUMBERSTYLES);
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= mxNumberStyles;
                }

                sPropName = "PrivateData";
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    Reference < XInterface > xIfc;
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= xIfc;

                    StyleMap *pSMap = comphelper::getFromUnoTunnel<StyleMap>( xIfc );
                    if( pSMap )
                    {
                        mpStyleMap = pSMap;
                    }
                }
                OUString sBaseURI;
                sPropName = "BaseURI";
                if (xPropertySetInfo->hasPropertyByName(sPropName))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sBaseURI;
                    mpImpl->aBaseURL.SetURL( sBaseURI );
                    mpImpl->aDocBase.SetURL( sBaseURI );
                }
                OUString sRelPath;
                sPropName = "StreamRelPath";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sRelPath;
                }
                OUString sName;
                sPropName = "StreamName";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= sName;
                }
                if( !sBaseURI.isEmpty() && !sName.isEmpty() )
                {
                    if( !sRelPath.isEmpty() )
                        mpImpl->aBaseURL.insertName( sRelPath );
                    mpImpl->aBaseURL.insertName( sName );
                }
                mpImpl->mStreamName = sName; // Note: may be empty (XSLT)
                // Retrieve property <ShapePositionInHoriL2R> (#i28749#)
                sPropName = "ShapePositionInHoriL2R";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= mpImpl->mbShapePositionInHoriL2R;
                }
                sPropName = "TextDocInOOoFileFormat";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= mpImpl->mbTextDocInOOoFileFormat;
                }

                sPropName = "SourceStorage";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                    mxImportInfo->getPropertyValue(sPropName) >>= mpImpl->mxSourceStorage;
            }
        }
    }

    uno::Reference<lang::XInitialization> const xInit(mxParser, uno::UNO_QUERY_THROW);
    xInit->initialize( { makeAny(OUString("IgnoreMissingNSDecl")) });
}

// XServiceInfo
OUString SAL_CALL SvXMLImport::getImplementationName()
{
    return mpImpl->implementationName;
}

sal_Bool SAL_CALL SvXMLImport::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SvXMLImport::getSupportedServiceNames(  )
{
    return mpImpl->maSupportedServiceNames;
}

XMLTextImportHelper* SvXMLImport::CreateTextImport()
{
    return new XMLTextImportHelper( mxModel, *this );
}

XMLShapeImportHelper* SvXMLImport::CreateShapeImport()
{
    return new XMLShapeImportHelper( *this, mxModel );
}

SchXMLImportHelper* SvXMLImport::CreateChartImport()
{
// WASM_CHART change
// TODO: Instead of importing the ChartModel an alternative may be
// added to convert not to Chart/OLE SdrObejct, but to GraphicObject
// with the Chart visualization. There should be a preiew available
// in the imported chart data
#ifndef ENABLE_WASM_STRIP_CHART
    return new SchXMLImportHelper();
#else
    return nullptr;
#endif
}

::xmloff::OFormLayerXMLImport* SvXMLImport::CreateFormImport()
{
    return new ::xmloff::OFormLayerXMLImport(*this);
}


// Get or create fill/line/lineend-style-helper


const Reference< container::XNameContainer > & SvXMLImport::GetGradientHelper()
{
    if( !mxGradientHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxGradientHelper.set( xServiceFact->createInstance(
                        "com.sun.star.drawing.GradientTable" ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetHatchHelper()
{
    if( !mxHatchHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxHatchHelper.set( xServiceFact->createInstance(
                        "com.sun.star.drawing.HatchTable" ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxHatchHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetBitmapHelper()
{
    if( !mxBitmapHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxBitmapHelper.set( xServiceFact->createInstance(
                        "com.sun.star.drawing.BitmapTable" ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxBitmapHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetTransGradientHelper()
{
    if( !mxTransGradientHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxTransGradientHelper.set( xServiceFact->createInstance(
                        "com.sun.star.drawing.TransparencyGradientTable" ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxTransGradientHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetMarkerHelper()
{
    if( !mxMarkerHelper.is() )
    {
        if( mxModel.is() )
        {
            Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
            if( xServiceFact.is() )
            {
                try
                {
                    mxMarkerHelper.set( xServiceFact->createInstance( "com.sun.star.drawing.MarkerTable" ), UNO_QUERY);
                }
                catch( lang::ServiceNotRegisteredException& )
                {}
            }
        }
    }

    return mxMarkerHelper;
}

const Reference< container::XNameContainer > & SvXMLImport::GetDashHelper()
{
    if( !mxDashHelper.is() && mxModel.is() )
    {
        Reference< lang::XMultiServiceFactory > xServiceFact( mxModel, UNO_QUERY);
        if( xServiceFact.is() )
        {
            try
            {
                mxDashHelper.set( xServiceFact->createInstance( "com.sun.star.drawing.DashTable" ), UNO_QUERY);
            }
            catch( lang::ServiceNotRegisteredException& )
            {}
        }
    }

    return mxDashHelper;
}

bool SvXMLImport::IsPackageURL( const OUString& rURL ) const
{

    // if, and only if, only parts are imported, then we're in a package
    const SvXMLImportFlags nTest = SvXMLImportFlags::META|SvXMLImportFlags::STYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SETTINGS;
    if( (mnImportFlags & nTest) == nTest )
        return false;

    // TODO: from this point extract to own static function

    // Some quick tests: Some may rely on the package structure!
    sal_Int32 nLen = rURL.getLength();
    if( nLen > 0 && '/' == rURL[0] )
        // RFC2396 net_path or abs_path
        return false;
    else if( nLen > 1 && '.' == rURL[0] )
    {
        if( '.' == rURL[1] )
            // ../: We are never going up one level, so we know
            // it's not an external URI
            return false;
        else if( '/' == rURL[1] )
            // we are remaining on a level, so it's a package URI
            return true;
    }

    // Now check for a RFC2396 schema
    sal_Int32 nPos = 1;
    while( nPos < nLen )
    {
        switch( rURL[nPos] )
        {
        case '/':
            // a relative path segment
            return true;
        case ':':
            // a schema
            return false;
        default:
            break;
            // we don't care about any other characters
        }
        ++nPos;
    }

    return true;
}

uno::Reference<graphic::XGraphic> SvXMLImport::loadGraphicByURL(OUString const & rURL)
{
    uno::Reference<graphic::XGraphic> xGraphic;

    if (mxGraphicStorageHandler.is())
    {
        if (IsPackageURL(rURL))
        {
            xGraphic = mxGraphicStorageHandler->loadGraphic(rURL);
        }
        else
        {
            OUString const & rAbsoluteURL = GetAbsoluteReference(rURL);
            GraphicExternalLink aExternalLink(rAbsoluteURL);
            Graphic aGraphic(aExternalLink);
            xGraphic = aGraphic.GetXGraphic();
        }
    }

    return xGraphic;
}

uno::Reference<graphic::XGraphic> SvXMLImport::loadGraphicFromBase64(uno::Reference<io::XOutputStream> const & rxOutputStream)
{
    uno::Reference<graphic::XGraphic> xGraphic;

    if (mxGraphicStorageHandler.is())
    {
        xGraphic = mxGraphicStorageHandler->loadGraphicFromOutputStream(rxOutputStream);
    }

    return xGraphic;
}

Reference< XOutputStream > SvXMLImport::GetStreamForGraphicObjectURLFromBase64() const
{
    Reference< XOutputStream > xOStm;
    Reference< document::XBinaryStreamResolver > xStmResolver(mxGraphicStorageHandler, UNO_QUERY);

    if( xStmResolver.is() )
        xOStm = xStmResolver->createOutputStream();

    return xOStm;
}

OUString SvXMLImport::ResolveEmbeddedObjectURL(
                                    const OUString& rURL,
                                    std::u16string_view rClassId )
{
    OUString sRet;

    if( IsPackageURL( rURL ) )
    {
        if ( mxEmbeddedResolver.is() )
        {
            OUString sURL( rURL );
            if( !rClassId.empty() )
            {
                sURL += OUString::Concat("!") + rClassId;
            }
            sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( sURL );
        }
    }
    else
        sRet = GetAbsoluteReference( rURL );

    return sRet;
}

Reference< embed::XStorage > const & SvXMLImport::GetSourceStorage() const
{
    return mpImpl->mxSourceStorage;
}

Reference < XOutputStream >
        SvXMLImport::GetStreamForEmbeddedObjectURLFromBase64() const
{
    Reference < XOutputStream > xOLEStream;

    if( mxEmbeddedResolver.is() )
    {
        Reference< XNameAccess > xNA( mxEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            Any aAny = xNA->getByName( "Obj12345678" );
            aAny >>= xOLEStream;
        }
    }

    return xOLEStream;
}

OUString SvXMLImport::ResolveEmbeddedObjectURLFromBase64()
{
    OUString sRet;

    if( mxEmbeddedResolver.is() )
    {
        sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( "Obj12345678" );
    }

    return sRet;
}

void SvXMLImport::AddStyleDisplayName( XmlStyleFamily nFamily,
                                       const OUString& rName,
                                       const OUString& rDisplayName )
{
    if( !mpStyleMap.is() )
    {
        mpStyleMap = new StyleMap;
        if( mxImportInfo.is() )
        {
            OUString sPrivateData( "PrivateData" );
            Reference< beans::XPropertySetInfo > xPropertySetInfo =
                mxImportInfo->getPropertySetInfo();
            if( xPropertySetInfo.is() &&
                xPropertySetInfo->hasPropertyByName(sPrivateData) )
            {
                Reference < XInterface > xIfc(
                        static_cast< XUnoTunnel *>( mpStyleMap.get() ) );
                mxImportInfo->setPropertyValue( sPrivateData, Any(xIfc) );
            }
        }
    }

    StyleMap::key_type aKey( nFamily, rName );
    StyleMap::value_type aValue( aKey, rDisplayName );
    ::std::pair<StyleMap::iterator,bool> aRes( mpStyleMap->insert( aValue ) );
    SAL_WARN_IF( !aRes.second,
                 "xmloff.core",
       "duplicate style name of family " << static_cast<int>(nFamily) << ": \"" << rName << "\"");

}

OUString SvXMLImport::GetStyleDisplayName( XmlStyleFamily nFamily,
                                           const OUString& rName ) const
{
    OUString sName( rName );
    if( mpStyleMap.is() && !rName.isEmpty() )
    {
        StyleMap::key_type aKey( nFamily, rName );
        StyleMap::const_iterator aIter = mpStyleMap->find( aKey );
        if( aIter != mpStyleMap->end() )
            sName = (*aIter).second;
    }
    return sName;
}

void SvXMLImport::SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>&)
{
}

void SvXMLImport::SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>&)
{
}

void SvXMLImport::SetDocumentSpecificSettings(const OUString&, const uno::Sequence<beans::PropertyValue>&)
{
}

ProgressBarHelper*  SvXMLImport::GetProgressBarHelper()
{
    if (!mpProgressBarHelper)
    {
        mpProgressBarHelper = std::make_unique<ProgressBarHelper>(mxStatusIndicator, false);

        if (mxImportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = mxImportInfo->getPropertySetInfo();
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
                    aAny = mxImportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        mpProgressBarHelper->SetRange(nProgressRange);
                    aAny = mxImportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        mpProgressBarHelper->SetReference(nProgressMax);
                    aAny = mxImportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        mpProgressBarHelper->SetValue(nProgressCurrent);
                }
                if (xPropertySetInfo->hasPropertyByName(sRepeat))
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sRepeat);
                    if (aAny.getValueType() == cppu::UnoType<bool>::get())
                        mpProgressBarHelper->SetRepeat(::cppu::any2bool(aAny));
                    else {
                        SAL_WARN( "xmloff.core", "why is it no boolean?" );
                    }
                }
            }
        }
    }
    return mpProgressBarHelper.get();
}

void SvXMLImport::AddNumberStyle(sal_Int32 nKey, const OUString& rName)
{
    if (!mxNumberStyles.is())
        mxNumberStyles.set( comphelper::NameContainer_createInstance( ::cppu::UnoType<sal_Int32>::get()) );
    if (mxNumberStyles.is())
    {
        try
        {
            mxNumberStyles->insertByName(rName, Any(nKey));
        }
        catch ( uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION( "xmloff.core", "Numberformat could not be inserted");
        }
    }
    else {
        SAL_WARN( "xmloff.core", "not possible to create NameContainer");
    }
}

XMLEventImportHelper& SvXMLImport::GetEventImport()
{
    if (!mpEventImportHelper)
    {
        // construct event helper and register StarBasic handler and standard
        // event tables
        mpEventImportHelper = std::make_unique<XMLEventImportHelper>();
        const OUString& sStarBasic(GetXMLToken(XML_STARBASIC));
        mpEventImportHelper->RegisterFactory(sStarBasic,
                                            std::make_unique<XMLStarBasicContextFactory>());
        const OUString& sScript(GetXMLToken(XML_SCRIPT));
        mpEventImportHelper->RegisterFactory(sScript,
                                            std::make_unique<XMLScriptContextFactory>());
        mpEventImportHelper->AddTranslationTable(aStandardEventTable);

        // register StarBasic event handler with capitalized spelling
        mpEventImportHelper->RegisterFactory("StarBasic",
                                            std::make_unique<XMLStarBasicContextFactory>());
    }

    return *mpEventImportHelper;
}

void SvXMLImport::SetFontDecls( XMLFontStylesContext *pFontDecls )
{
    if (mxFontDecls.is())
        mxFontDecls->dispose();
    mxFontDecls = pFontDecls;
}

void SvXMLImport::SetStyles( SvXMLStylesContext *pStyles )
{
    if (mxStyles.is())
        mxStyles->dispose();
    mxStyles = pStyles;
}

void SvXMLImport::SetAutoStyles( SvXMLStylesContext *pAutoStyles )
{
    if (pAutoStyles && mxNumberStyles.is())
    {
        uno::Reference<xml::sax::XFastAttributeList> xAttrList = new sax_fastparser::FastAttributeList(nullptr);
        const uno::Sequence<OUString> aStyleNames = mxNumberStyles->getElementNames();
        for (const auto& name : aStyleNames)
        {
            uno::Any aAny(mxNumberStyles->getByName(name));
            sal_Int32 nKey(0);
            if (aAny >>= nKey)
            {
                SvXMLStyleContext* pContext = new SvXMLNumFormatContext(
                    *this, name, xAttrList, nKey,
                    GetDataStylesImport()->GetLanguageForKey(nKey), *pAutoStyles);
                pAutoStyles->AddStyle(*pContext);
            }
        }
    }
    if (mxAutoStyles.is())
        mxAutoStyles->dispose();
    mxAutoStyles = pAutoStyles;
    GetTextImport()->SetAutoStyles( pAutoStyles );
    GetShapeImport()->SetAutoStylesContext( pAutoStyles );
    GetChartImport()->SetAutoStylesContext( pAutoStyles );
    GetFormImport()->setAutoStyleContext( pAutoStyles );
}

void SvXMLImport::SetMasterStyles( SvXMLStylesContext *pMasterStyles )
{
    if (mxMasterStyles.is())
        mxMasterStyles->dispose();
    mxMasterStyles = pMasterStyles;
}

XMLFontStylesContext *SvXMLImport::GetFontDecls()
{
    return mxFontDecls.get();
}

SvXMLStylesContext *SvXMLImport::GetStyles()
{
    return mxStyles.get();
}

SvXMLStylesContext *SvXMLImport::GetAutoStyles()
{
    return mxAutoStyles.get();
}

const XMLFontStylesContext *SvXMLImport::GetFontDecls() const
{
    return mxFontDecls.get();
}

const SvXMLStylesContext *SvXMLImport::GetStyles() const
{
    return mxStyles.get();
}

const SvXMLStylesContext *SvXMLImport::GetAutoStyles() const
{
    return mxAutoStyles.get();
}

OUString SvXMLImport::GetAbsoluteReference(const OUString& rValue) const
{
    if( rValue.isEmpty() || rValue[0] == '#' )
        return rValue;

    INetURLObject aAbsURL;
    if( mpImpl->aBaseURL.GetNewAbsURL( rValue, &aAbsURL ) )
        return aAbsURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    else
        return rValue;
}

bool SvXMLImport::IsODFVersionConsistent( const OUString& aODFVersion )
{
    // the check returns sal_False only if the storage version could be retrieved
    bool bResult = true;

    if ( !aODFVersion.isEmpty() && aODFVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
    {
        // check the consistency only for the ODF1.2 and later ( according to content.xml )
        // manifest.xml might have no version, it should be checked here and the correct version should be set
        try
        {   // don't use getDocumentStorage(), it's temporary and latest version
            uno::Reference<embed::XStorage> const xStor(GetSourceStorage());
            uno::Reference< beans::XPropertySet > xStorProps( xStor, uno::UNO_QUERY_THROW );

            // the check should be done only for OASIS format
            if (!IsOOoXML())
            {
                bool bRepairPackage = false;
                try
                {
                    xStorProps->getPropertyValue( "RepairPackage" )
                        >>= bRepairPackage;
                } catch ( uno::Exception& )
                {}

                // check only if not in Repair mode
                if ( !bRepairPackage )
                {
                    OUString aStorVersion;
                    xStorProps->getPropertyValue( "Version" )
                        >>= aStorVersion;

                    // if the storage version is set in manifest.xml, it must be the same as in content.xml
                    // if not, set it explicitly to be used further ( it will work even for readonly storage )
                    // This workaround is not nice, but I see no other way to handle it, since there are
                    // ODF1.2 documents without version in manifest.xml
                    if ( !aStorVersion.isEmpty() )
                        bResult = aODFVersion == aStorVersion;
                    else
                        xStorProps->setPropertyValue( "Version",
                                                      uno::makeAny( aODFVersion ) );

                    if ( bResult )
                    {
                        bool bInconsistent = false;
                        xStorProps->getPropertyValue( "IsInconsistent" )
                            >>= bInconsistent;
                        bResult = !bInconsistent;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

void SvXMLImport::CreateNumberFormatsSupplier_()
{
    SAL_WARN_IF( mxNumberFormatsSupplier.is(), "xmloff.core", "number formats supplier already exists!" );
    if(mxModel.is())
        mxNumberFormatsSupplier =
            uno::Reference< util::XNumberFormatsSupplier> (mxModel, uno::UNO_QUERY);
}

void SvXMLImport::CreateDataStylesImport_()
{
    SAL_WARN_IF( bool(mpNumImport), "xmloff.core", "data styles import already exists!" );
    uno::Reference<util::XNumberFormatsSupplier> xNum =
        GetNumberFormatsSupplier();
    if ( xNum.is() )
        mpNumImport = std::make_unique<SvXMLNumFmtHelper>(xNum, GetComponentContext() );
}

sal_Unicode SvXMLImport::ConvStarBatsCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !mpImpl->hBatsFontConv )
    {
        mpImpl->hBatsFontConv = CreateFontToSubsFontConverter( u"StarBats",
                 FontToSubsFontFlags::IMPORT );
        SAL_WARN_IF( !mpImpl->hBatsFontConv, "xmloff.core", "Got no symbol font converter" );
    }
    if( mpImpl->hBatsFontConv )
    {
        cNew = ConvertFontToSubsFontChar( mpImpl->hBatsFontConv, c );
    }

    return cNew;
}

sal_Unicode SvXMLImport::ConvStarMathCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !mpImpl->hMathFontConv )
    {
        mpImpl->hMathFontConv = CreateFontToSubsFontConverter( u"StarMath",
                 FontToSubsFontFlags::IMPORT );
        SAL_WARN_IF( !mpImpl->hMathFontConv, "xmloff.core", "Got no symbol font converter" );
    }
    if( mpImpl->hMathFontConv )
    {
        cNew = ConvertFontToSubsFontChar( mpImpl->hMathFontConv, c );
    }

    return cNew;
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams,
    const OUString& rExceptionMessage,
    const Reference<xml::sax::XLocator>& rLocator )
{
    // create error list on demand
    if ( !mpXMLErrors )
        mpXMLErrors = std::make_unique<XMLErrors>();

    // save error information
    // use document locator (if none supplied)
    mpXMLErrors->AddRecord( nId, rMsgParams, rExceptionMessage,
                           rLocator.is() ? rLocator : mxLocator );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const Sequence<OUString>& rMsgParams)
{
    SetError( nId, rMsgParams, "", nullptr );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const OUString& rMsg1)
{
    Sequence<OUString> aSeq { rMsg1 };
    SetError( nId, aSeq );
}

void SvXMLImport::DisposingModel()
{
    if( mxFontDecls.is() )
        mxFontDecls->dispose();
    if( mxStyles.is() )
        mxStyles->dispose();
    if( mxAutoStyles.is() )
        mxAutoStyles->dispose();
    if( mxMasterStyles.is() )
        mxMasterStyles->dispose();

    mxModel.set(nullptr);
    mxEventListener.set(nullptr);
}

::comphelper::UnoInterfaceToUniqueIdentifierMapper& SvXMLImport::getInterfaceToIdentifierMapper()
{
    return mpImpl->maInterfaceToIdentifierMapper;
}

uno::Reference< uno::XComponentContext > const &
SvXMLImport::GetComponentContext() const
{
    return mpImpl->mxComponentContext;
}

OUString SvXMLImport::GetBaseURL() const
{
    return mpImpl->aBaseURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

OUString SvXMLImport::GetDocumentBase() const
{
    return mpImpl->aDocBase.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

// Convert drawing object positions from OOo file format to OASIS (#i28749#)
bool SvXMLImport::IsShapePositionInHoriL2R() const
{
    return mpImpl->mbShapePositionInHoriL2R;
}

bool SvXMLImport::IsTextDocInOOoFileFormat() const
{
    return mpImpl->mbTextDocInOOoFileFormat;
}

void SvXMLImport::initXForms()
{
    // dummy method; to be implemented by derived classes supporting XForms
}

bool SvXMLImport::getBuildIds( sal_Int32& rUPD, sal_Int32& rBuild ) const
{
    bool bRet = false;
    OUString const aBuildId(getBuildIdsProperty(mxImportInfo));
    if (!aBuildId.isEmpty())
    {
        sal_Int32 nIndex = aBuildId.indexOf('$');
        if (nIndex != -1)
        {
            rUPD = aBuildId.copy( 0, nIndex ).toInt32();
            sal_Int32 nIndexEnd = aBuildId.indexOf(';', nIndex);
            rBuild = (nIndexEnd == -1)
                ? aBuildId.copy(nIndex + 1).toInt32()
                : aBuildId.copy(nIndex + 1, nIndexEnd - nIndex - 1).toInt32();
            bRet = true;
        }
    }
    return bRet;
}

sal_uInt16 SvXMLImport::getGeneratorVersion() const
{
    // --> ORW
    return mpImpl->getGeneratorVersion( *this );
    // <--
}

bool SvXMLImport::isGeneratorVersionOlderThan(
        sal_uInt16 const nOOoVersion, sal_uInt16 const nLOVersion)
{
    assert( (nLOVersion  & LO_flag));
    assert(!(nOOoVersion & LO_flag));
    const sal_uInt16 nGeneratorVersion(getGeneratorVersion());
    return (nGeneratorVersion & LO_flag)
        ?   nGeneratorVersion < nLOVersion
        :   nGeneratorVersion < nOOoVersion;
}


OUString SvXMLImport::GetODFVersion() const
{
    return mpImpl->mxODFVersion ? *mpImpl->mxODFVersion : OUString();
}

bool SvXMLImport::IsOOoXML() const
{
    return mpImpl->mbIsOOoXML;
}

// xml:id for RDF metadata
void SvXMLImport::SetXmlId(uno::Reference<uno::XInterface> const & i_xIfc,
    OUString const & i_rXmlId)
{
    if (i_rXmlId.isEmpty())
        return;

    try {
        const uno::Reference<rdf::XMetadatable> xMeta(i_xIfc,
            uno::UNO_QUERY);
//FIXME: not yet
        if (xMeta.is()) {
            const beans::StringPair mdref( mpImpl->mStreamName, i_rXmlId );
            try {
                xMeta->setMetadataReference(mdref);
            } catch (lang::IllegalArgumentException &) {
                // probably duplicate; ignore
                SAL_INFO("xmloff.core", "SvXMLImport::SetXmlId: cannot set xml:id");
            }
        }
    } catch (uno::Exception &) {
        TOOLS_WARN_EXCEPTION("xmloff.core","SvXMLImport::SetXmlId");
    }
}

::xmloff::RDFaImportHelper &
SvXMLImport::GetRDFaImportHelper()
{
    if (!mpImpl->mpRDFaHelper)
    {
        mpImpl->mpRDFaHelper.reset( new ::xmloff::RDFaImportHelper(*this) );
    }
    return *mpImpl->mpRDFaHelper;
}

void
SvXMLImport::AddRDFa(const uno::Reference<rdf::XMetadatable>& i_xObject,
    OUString const & i_rAbout,
    OUString const & i_rProperty,
    OUString const & i_rContent,
    OUString const & i_rDatatype)
{
    // N.B.: we only get called if i_xObject had xhtml:about attribute
    // (an empty attribute value is valid)
    ::xmloff::RDFaImportHelper & rRDFaHelper( GetRDFaImportHelper() );
    rRDFaHelper.ParseAndAddRDFa(i_xObject,
        i_rAbout, i_rProperty, i_rContent, i_rDatatype);
}

bool SvXMLImport::embeddedFontAlreadyProcessed( const OUString& url )
{
    if( embeddedFontUrlsKnown.count( url ) != 0 )
        return true;
    embeddedFontUrlsKnown.insert( url );
    return false;
}

const OUString & SvXMLImport::getNameFromToken( sal_Int32 nToken )
{
    return xTokenHandler->getIdentifier( nToken & TOKEN_MASK );
}

OUString SvXMLImport::getPrefixAndNameFromToken( sal_Int32 nToken )
{
    OUString rv;
    sal_Int32 nNamespaceToken = ( nToken & NMSP_MASK ) >> NMSP_SHIFT;
    auto aIter( aNamespaceMap.find( nNamespaceToken ) );
    if( aIter != aNamespaceMap.end() )
        rv = (*aIter).second.second + " " + aIter->second.first + ":";
    return rv + xTokenHandler->getIdentifier( nToken & TOKEN_MASK );
}

OUString SvXMLImport::getNamespacePrefixFromToken(sal_Int32 nToken, const SvXMLNamespaceMap* pMap)
{
    sal_Int32 nNamespaceToken = ( nToken & NMSP_MASK ) >> NMSP_SHIFT;
    auto aIter( aNamespaceMap.find( nNamespaceToken ) );
    if( aIter != aNamespaceMap.end() )
    {
        if (pMap)
        {
            OUString sRet = pMap->GetPrefixByKey(pMap->GetKeyByName((*aIter).second.second));
            if (!sRet.isEmpty())
                return sRet;
        }
        return (*aIter).second.first;
    }
    else
        return OUString();
}

OUString SvXMLImport::getNamespaceURIFromToken( sal_Int32 nToken )
{
    sal_Int32 nNamespaceToken = ( nToken & NMSP_MASK ) >> NMSP_SHIFT;
    auto aIter( aNamespaceMap.find( nNamespaceToken ) );
    if( aIter != aNamespaceMap.end() )
        return (*aIter).second.second;
    else
        return OUString();
}

OUString SvXMLImport::getNamespacePrefixFromURI( const OUString& rURI )
{
    auto aIter( aNamespaceURIPrefixMap.find(rURI) );
    if( aIter != aNamespaceURIPrefixMap.end() )
        return (*aIter).second;
    else
        return OUString();
}

sal_Int32 SvXMLImport::getTokenFromName( const OUString& rName )
{
    Sequence< sal_Int8 > aLocalNameSeq( reinterpret_cast<sal_Int8 const *>(
        OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr()), rName.getLength() );
    return xTokenHandler->getTokenFromUTF8( aLocalNameSeq );
}

void SvXMLImport::initializeNamespaceMaps()
{
    auto mapTokenToNamespace = []( sal_Int32 nToken, sal_Int32 nPrefix, sal_Int32 nNamespace )
    {
        if ( nToken >= 0 )
        {
            const OUString& sNamespace = GetXMLToken( static_cast<XMLTokenEnum>( nNamespace ) );
            const OUString& sPrefix = GetXMLToken( static_cast<XMLTokenEnum>( nPrefix ) );
            assert( aNamespaceMap.find(nToken +1) == aNamespaceMap.end() && "cannot map two namespaces to the same token here");
            aNamespaceMap[ nToken + 1 ] = std::make_pair( sPrefix, sNamespace );
            aNamespaceURIPrefixMap.emplace( sNamespace, sPrefix );
        }
    };

    mapTokenToNamespace( XML_NAMESPACE_XML,              XML_XML,              XML_N_XML              ); // implicit "xml" namespace prefix
    mapTokenToNamespace( XML_NAMESPACE_OFFICE,           XML_NP_OFFICE,        XML_N_OFFICE           );
    mapTokenToNamespace( XML_NAMESPACE_OFFICE_SO52,      XML_NP_OFFICE,        XML_N_OFFICE_OLD       );
    mapTokenToNamespace( XML_NAMESPACE_OFFICE_OOO,       XML_NP_OFFICE,        XML_N_OFFICE_OOO       );
    mapTokenToNamespace( XML_NAMESPACE_STYLE,            XML_NP_STYLE,         XML_N_STYLE            );
    mapTokenToNamespace( XML_NAMESPACE_STYLE_SO52,       XML_NP_STYLE,         XML_N_STYLE_OLD        );
    mapTokenToNamespace( XML_NAMESPACE_STYLE_OOO,        XML_NP_STYLE,         XML_N_STYLE_OOO        );
    mapTokenToNamespace( XML_NAMESPACE_TEXT,             XML_NP_TEXT,          XML_N_TEXT             );
    mapTokenToNamespace( XML_NAMESPACE_TEXT_SO52,        XML_NP_TEXT,          XML_N_TEXT_OLD         );
    mapTokenToNamespace( XML_NAMESPACE_TEXT_OOO,         XML_NP_TEXT,          XML_N_TEXT_OOO         );
    mapTokenToNamespace( XML_NAMESPACE_TABLE,            XML_NP_TABLE,         XML_N_TABLE            );
    mapTokenToNamespace( XML_NAMESPACE_TABLE_SO52,       XML_NP_TABLE,         XML_N_TABLE_OLD        );
    mapTokenToNamespace( XML_NAMESPACE_TABLE_OOO,        XML_NP_TABLE,         XML_N_TABLE_OOO        );
    mapTokenToNamespace( XML_NAMESPACE_DRAW,             XML_NP_DRAW,          XML_N_DRAW             );
    mapTokenToNamespace( XML_NAMESPACE_DRAW_SO52,        XML_NP_DRAW,          XML_N_DRAW_OLD         );
    mapTokenToNamespace( XML_NAMESPACE_DRAW_OOO,         XML_NP_DRAW,          XML_N_DRAW_OOO         );
    mapTokenToNamespace( XML_NAMESPACE_FO,               XML_NP_FO,            XML_N_FO               );
    mapTokenToNamespace( XML_NAMESPACE_FO_SO52,          XML_NP_FO,            XML_N_FO_OLD           );
    mapTokenToNamespace( XML_NAMESPACE_FO_COMPAT,        XML_NP_FO,            XML_N_FO_COMPAT        );
    mapTokenToNamespace( XML_NAMESPACE_XLINK,            XML_NP_XLINK,         XML_N_XLINK            );
    mapTokenToNamespace( XML_NAMESPACE_XLINK_SO52,       XML_NP_XLINK,         XML_N_XLINK_OLD        );
    mapTokenToNamespace( XML_NAMESPACE_DC,               XML_NP_DC,            XML_N_DC               );
    mapTokenToNamespace( XML_NAMESPACE_META,             XML_NP_META,          XML_N_META             );
    mapTokenToNamespace( XML_NAMESPACE_META_SO52,        XML_NP_META,          XML_N_META_OLD         );
    mapTokenToNamespace( XML_NAMESPACE_META_OOO,         XML_NP_META,          XML_N_META_OOO         );
    mapTokenToNamespace( XML_NAMESPACE_NUMBER,           XML_NP_NUMBER,        XML_N_NUMBER           );
    mapTokenToNamespace( XML_NAMESPACE_NUMBER_SO52,      XML_NP_NUMBER,        XML_N_NUMBER_OLD       );
    mapTokenToNamespace( XML_NAMESPACE_NUMBER_OOO,       XML_NP_NUMBER,        XML_N_NUMBER_OOO       );
    mapTokenToNamespace( XML_NAMESPACE_PRESENTATION,     XML_NP_PRESENTATION,  XML_N_PRESENTATION     );
    mapTokenToNamespace( XML_NAMESPACE_PRESENTATION_SO52,XML_NP_PRESENTATION,  XML_N_PRESENTATION_OLD );
    mapTokenToNamespace( XML_NAMESPACE_PRESENTATION_OOO, XML_NP_PRESENTATION,  XML_N_PRESENTATION_OOO );
    mapTokenToNamespace( XML_NAMESPACE_PRESENTATION_OASIS, XML_NP_PRESENTATION, XML_N_PRESENTATION_OASIS );
    mapTokenToNamespace( XML_NAMESPACE_SVG,              XML_NP_SVG,           XML_N_SVG              );
    mapTokenToNamespace( XML_NAMESPACE_SVG_COMPAT,       XML_NP_SVG,           XML_N_SVG_COMPAT       );
    mapTokenToNamespace( XML_NAMESPACE_CHART,            XML_NP_CHART,         XML_N_CHART            );
    mapTokenToNamespace( XML_NAMESPACE_CHART_SO52,       XML_NP_CHART,         XML_N_CHART_OLD        );
    mapTokenToNamespace( XML_NAMESPACE_CHART_OOO,        XML_NP_CHART,         XML_N_CHART_OOO        );
    mapTokenToNamespace( XML_NAMESPACE_DR3D,             XML_NP_DR3D,          XML_N_DR3D             );
    mapTokenToNamespace( XML_NAMESPACE_DR3D_OOO,         XML_NP_DR3D,          XML_N_DR3D_OOO         );
    mapTokenToNamespace( XML_NAMESPACE_MATH,             XML_NP_MATH,          XML_N_MATH             );
    mapTokenToNamespace( XML_NAMESPACE_VERSIONS_LIST,    XML_NP_VERSIONS_LIST, XML_N_VERSIONS_LIST    );
    mapTokenToNamespace( XML_NAMESPACE_FORM,             XML_NP_FORM,          XML_N_FORM             );
    mapTokenToNamespace( XML_NAMESPACE_FORM_OOO,         XML_NP_FORM,          XML_N_FORM_OOO         );
    mapTokenToNamespace( XML_NAMESPACE_SCRIPT,           XML_NP_SCRIPT,        XML_N_SCRIPT           );
    mapTokenToNamespace( XML_NAMESPACE_SCRIPT_OOO,       XML_NP_SCRIPT,        XML_N_SCRIPT_OOO       );
    mapTokenToNamespace( XML_NAMESPACE_BLOCKLIST,        XML_NP_BLOCK_LIST,    XML_N_BLOCK_LIST       );
    mapTokenToNamespace( XML_NAMESPACE_CONFIG,           XML_NP_CONFIG,        XML_N_CONFIG           );
    mapTokenToNamespace( XML_NAMESPACE_CONFIG_OOO,       XML_NP_CONFIG,        XML_N_CONFIG_OOO       );
    mapTokenToNamespace( XML_NAMESPACE_OOO,              XML_NP_OOO,           XML_N_OOO              );
    mapTokenToNamespace( XML_NAMESPACE_OOOW,             XML_NP_OOOW,          XML_N_OOOW             );
    mapTokenToNamespace( XML_NAMESPACE_OOOC,             XML_NP_OOOC,          XML_N_OOOC             );
    mapTokenToNamespace( XML_NAMESPACE_DOM,              XML_NP_DOM,           XML_N_DOM              );
    mapTokenToNamespace( XML_NAMESPACE_DB,               XML_NP_DB,            XML_N_DB               );
    mapTokenToNamespace( XML_NAMESPACE_DB_OASIS,         XML_NP_DB,            XML_N_DB_OASIS         );
    mapTokenToNamespace( XML_NAMESPACE_DLG,              XML_NP_DLG,           XML_N_DLG              );
    mapTokenToNamespace( XML_NAMESPACE_XFORMS,           XML_NP_XFORMS_1_0,    XML_N_XFORMS_1_0       );
    mapTokenToNamespace( XML_NAMESPACE_XSD,              XML_NP_XSD,           XML_N_XSD              );
    mapTokenToNamespace( XML_NAMESPACE_XSI,              XML_NP_XSI,           XML_N_XSI              );
    mapTokenToNamespace( XML_NAMESPACE_SMIL,             XML_NP_SMIL,          XML_N_SMIL             );
    mapTokenToNamespace( XML_NAMESPACE_SMIL_SO52,        XML_NP_SMIL,          XML_N_SMIL_OLD         );
    mapTokenToNamespace( XML_NAMESPACE_SMIL_COMPAT,      XML_NP_SMIL,          XML_N_SMIL_COMPAT      );
    mapTokenToNamespace( XML_NAMESPACE_ANIMATION,        XML_NP_ANIMATION,     XML_N_ANIMATION        );
    mapTokenToNamespace( XML_NAMESPACE_ANIMATION_OOO,    XML_NP_ANIMATION,     XML_N_ANIMATION_OOO    );
    mapTokenToNamespace( XML_NAMESPACE_REPORT,           XML_NP_RPT,           XML_N_RPT              );
    mapTokenToNamespace( XML_NAMESPACE_REPORT_OASIS,     XML_NP_RPT,           XML_N_RPT_OASIS        );
    mapTokenToNamespace( XML_NAMESPACE_OF,               XML_NP_OF,            XML_N_OF               );
    mapTokenToNamespace( XML_NAMESPACE_XHTML,            XML_NP_XHTML,         XML_N_XHTML            );
    mapTokenToNamespace( XML_NAMESPACE_GRDDL,            XML_NP_GRDDL,         XML_N_GRDDL            );
    mapTokenToNamespace( XML_NAMESPACE_OFFICE_EXT,       XML_NP_OFFICE_EXT,    XML_N_OFFICE_EXT       );
    mapTokenToNamespace( XML_NAMESPACE_TABLE_EXT,        XML_NP_TABLE_EXT,     XML_N_TABLE_EXT        );
    mapTokenToNamespace( XML_NAMESPACE_CHART_EXT,        XML_NP_CHART_EXT,     XML_N_CHART_EXT        );
    mapTokenToNamespace( XML_NAMESPACE_DRAW_EXT,         XML_NP_DRAW_EXT,      XML_N_DRAW_EXT         );
    mapTokenToNamespace( XML_NAMESPACE_CALC_EXT,         XML_NP_CALC_EXT,      XML_N_CALC_EXT         );
    mapTokenToNamespace( XML_NAMESPACE_LO_EXT,           XML_NP_LO_EXT,        XML_N_LO_EXT           );
    mapTokenToNamespace( XML_NAMESPACE_CSS3TEXT,         XML_NP_CSS3TEXT,      XML_N_CSS3TEXT         );
    mapTokenToNamespace( XML_NAMESPACE_FIELD,            XML_NP_FIELD,         XML_N_FIELD            );
    mapTokenToNamespace( XML_NAMESPACE_FORMX,            XML_NP_FORMX,         XML_N_FORMX            );
}

void SvXMLImport::registerNamespaces()
{
    for( auto const &aNamespaceEntry : aNamespaceMap )
    {
        // aNamespaceMap = { Token : ( NamespacePrefix, NamespaceURI ) }
        registerNamespace( aNamespaceEntry.second.second, aNamespaceEntry.first << NMSP_SHIFT );
    }
}

void SvXMLImport::NotifyMacroEventRead()
{
    if (mbNotifyMacroEventRead)
        return;

    comphelper::DocumentInfo::notifyMacroEventRead(mxModel);

    mbNotifyMacroEventRead = true;
}

SvXMLImportFastNamespaceHandler::SvXMLImportFastNamespaceHandler()
{
}

void SvXMLImportFastNamespaceHandler::addNSDeclAttributes( rtl::Reference < comphelper::AttributeList > const & rAttrList )
{
    for(const auto& aNamespaceDefine : m_aNamespaceDefines)
    {
        const OUString& rPrefix = aNamespaceDefine.m_aPrefix;
        const OUString& rNamespaceURI = aNamespaceDefine.m_aNamespaceURI;
        OUString sDecl;
        if ( rPrefix.isEmpty() )
            sDecl = "xmlns";
        else
            sDecl = "xmlns:" + rPrefix;
        rAttrList->AddAttribute( sDecl, "CDATA", rNamespaceURI );
    }
    m_aNamespaceDefines.clear();
}

void SvXMLImportFastNamespaceHandler::registerNamespace( const OUString& rNamespacePrefix, const OUString& rNamespaceURI )
{
    // Elements with default namespace parsed by FastParser have namespace prefix.
    // A default namespace needs to be registered with the prefix, to maintain the compatibility.
    if ( rNamespacePrefix.isEmpty() )
        m_aNamespaceDefines.push_back( NamespaceDefine(
                                    SvXMLImport::getNamespacePrefixFromURI( rNamespaceURI ), rNamespaceURI) );

    m_aNamespaceDefines.push_back( NamespaceDefine(
                                    rNamespacePrefix, rNamespaceURI) );
}

OUString SvXMLImportFastNamespaceHandler::getNamespaceURI( const OUString&/* rNamespacePrefix */ )
{
    return OUString();
}

SvXMLLegacyToFastDocHandler::SvXMLLegacyToFastDocHandler( const rtl::Reference< SvXMLImport > & rImport )
:   mrImport( rImport ),
    mxFastAttributes( new sax_fastparser::FastAttributeList( SvXMLImport::xTokenHandler.get() ) )
{
}

void SAL_CALL SvXMLLegacyToFastDocHandler::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
{
    mrImport->setTargetDocument( xDoc );
}

void SAL_CALL SvXMLLegacyToFastDocHandler::startDocument()
{
    mrImport->startDocument();
}

void SAL_CALL SvXMLLegacyToFastDocHandler::endDocument()
{
    mrImport->endDocument();
}

void SAL_CALL SvXMLLegacyToFastDocHandler::startElement( const OUString& rName,
                        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_uInt16 nDefaultNamespace = XML_NAMESPACE_UNKNOWN;
    if (!maDefaultNamespaces.empty())
        nDefaultNamespace = maDefaultNamespaces.top();
    SvXMLImport::processNSAttributes(mrImport->mxNamespaceMap, mrImport.get(), xAttrList);
    OUString aLocalName;
    sal_uInt16 nPrefix = mrImport->mxNamespaceMap->GetKeyByAttrName( rName, &aLocalName );
    sal_Int32 mnElement = NAMESPACE_TOKEN( nPrefix ) | SvXMLImport::getTokenFromName( aLocalName );
    mxFastAttributes->clear();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (rAttrName == "xmlns")
        {
            sal_uInt16 nNamespaceKey = mrImport->mxNamespaceMap->GetKeyByName(rAttrValue);
            if (nNamespaceKey != XML_NAMESPACE_UNKNOWN)
            {
                nDefaultNamespace = nNamespaceKey;
                continue;
            }
            assert(false && "unknown namespace");
        }
        else if (rAttrName.indexOf(":") == -1 && nDefaultNamespace != XML_NAMESPACE_UNKNOWN)
        {
            auto const nToken = SvXMLImport::getTokenFromName(rAttrName);
            if (nToken == xmloff::XML_TOKEN_INVALID)
            {
                mxFastAttributes->addUnknown(mrImport->mxNamespaceMap->GetNameByKey(nDefaultNamespace),
                    OUStringToOString(rAttrName, RTL_TEXTENCODING_UTF8),
                    OUStringToOString(rAttrValue, RTL_TEXTENCODING_UTF8));
            }
            else
            {
                sal_Int32 const nAttr = NAMESPACE_TOKEN(nDefaultNamespace) | nToken;
                mxFastAttributes->add(nAttr, OUStringToOString(rAttrValue, RTL_TEXTENCODING_UTF8));
            }
            continue;
        }

        OUString aLocalAttrName;
        OUString aNamespace;
        // don't add unknown namespaces to the map
        sal_uInt16 const nAttrPrefix = mrImport->mxNamespaceMap->GetKeyByQName(
                rAttrName, nullptr, &aLocalAttrName, &aNamespace, SvXMLNamespaceMap::QNameMode::AttrValue);
        if( XML_NAMESPACE_XMLNS == nAttrPrefix )
            continue; // ignore
        auto const nToken = SvXMLImport::getTokenFromName(aLocalAttrName);
        if (XML_NAMESPACE_UNKNOWN == nAttrPrefix || nToken == xmloff::XML_TOKEN_INVALID)
        {
            mxFastAttributes->addUnknown(aNamespace,
                OUStringToOString(rAttrName, RTL_TEXTENCODING_UTF8),
                OUStringToOString(rAttrValue, RTL_TEXTENCODING_UTF8));
        }
        else
        {
            sal_Int32 const nAttr = NAMESPACE_TOKEN(nAttrPrefix) | nToken;
            mxFastAttributes->add(nAttr, OUStringToOString(rAttrValue, RTL_TEXTENCODING_UTF8));
        }
    }
    mrImport->startFastElement( mnElement, mxFastAttributes );
    maDefaultNamespaces.push(nDefaultNamespace);
}

void SAL_CALL SvXMLLegacyToFastDocHandler::endElement( const OUString& rName )
{
    OUString aLocalName;
    sal_uInt16 nPrefix = mrImport->mxNamespaceMap->GetKeyByAttrName( rName, &aLocalName );
    sal_Int32 mnElement = NAMESPACE_TOKEN( nPrefix ) | SvXMLImport::getTokenFromName(aLocalName);
    mrImport->endFastElement( mnElement );
    maDefaultNamespaces.pop();
}

void SAL_CALL SvXMLLegacyToFastDocHandler::characters( const OUString& aChars )
{
    mrImport->characters( aChars );
}

void SAL_CALL SvXMLLegacyToFastDocHandler::ignorableWhitespace( const OUString& )
{
}

void SAL_CALL SvXMLLegacyToFastDocHandler::processingInstruction( const OUString& aTarget,
                                                                  const OUString& aData)
{
    mrImport->processingInstruction( aTarget, aData );
}

void SAL_CALL SvXMLLegacyToFastDocHandler::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& rLocator )
{
    mrImport->setDocumentLocator( rLocator );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

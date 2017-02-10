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

#include <string.h>

#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <tools/urlobj.hxx>
#include <osl/mutex.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumfi.hxx>
#include "XMLEventImportHelper.hxx"
#include "XMLStarBasicContextFactory.hxx"
#include "XMLScriptContextFactory.hxx"
#include "StyleMap.hxx"
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmlerror.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <comphelper/namecontainer.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/fontcvt.hxx>
#include <o3tl/make_unique.hxx>
#include <xmloff/fasttokenhandler.hxx>

#include <com/sun/star/rdf/XMetadatable.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include "RDFaImportHelper.hxx"

using ::com::sun::star::beans::XPropertySetInfo;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;

class SvXMLImportEventListener : public cppu::WeakImplHelper< css::lang::XEventListener >
{
private:
    SvXMLImport*    pImport;

public:
    explicit SvXMLImportEventListener(SvXMLImport* pImport);

                            // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& rEventObject) override;
};

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
            SAL_WARN("xmloff.core", "exception getting BuildId");
            DBG_UNHANDLED_EXCEPTION();
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
                    else
                    {
                        SAL_INFO_IF('5' != loVersion[0], "xmloff.core", "unknown LO version: " << loVersion);
                        mnGeneratorVersion = SvXMLImport::LO_5x;
                    }
                    return; // ignore buildIds
                }
            }
        }
        sal_Int32 nUPD, nBuild;
        if ( rImport.getBuildIds( nUPD, nBuild ) )
        {
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
            else if (nUPD == 400)
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

    OUString aODFVersion;

    bool mbIsOOoXML;

    // Boolean, indicating that position attributes
    // of shapes are given in horizontal left-to-right layout. This is the case
    // for the OpenOffice.org file format. (#i28749#)
    bool mbShapePositionInHoriL2R;
    bool mbTextDocInOOoFileFormat;

    const uno::Reference< uno::XComponentContext > mxComponentContext;
    OUString implementationName;

    uno::Reference< embed::XStorage > mxSourceStorage;

    std::unique_ptr< xmloff::RDFaImportHelper > mpRDFaHelper;

    std::unique_ptr< DocumentInfo > mpDocumentInfo;

    SvXMLImport_Impl( const uno::Reference< uno::XComponentContext >& rxContext,
                      OUString const & theImplementationName)
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
        , mpRDFaHelper() // lazy
        , mpDocumentInfo() // lazy
    {
        SAL_WARN_IF(!mxComponentContext.is(), "xmloff.core", "SvXMLImport: no ComponentContext");
        if (!mxComponentContext.is()) throw uno::RuntimeException();
    }

    sal_uInt16 getGeneratorVersion( const SvXMLImport& rImport )
    {
        if ( !mpDocumentInfo.get() )
        {
            mpDocumentInfo.reset( new DocumentInfo( rImport ) );
        }

        return mpDocumentInfo->getGeneratorVersion();
    }

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper maInterfaceToIdentifierMapper;
};

SvXMLImportContext *SvXMLImport::CreateContext( sal_uInt16 nPrefix,
                                         const OUString& rLocalName,
                                         const uno::Reference< xml::sax::XAttributeList >& )
{
    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

SvXMLImportContext *SvXMLImport::CreateFastContext( sal_Int32 /*Element*/,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( *this );
}

void SvXMLImport::InitCtor_()
{
    if( mnImportFlags != SvXMLImportFlags::NONE )
    {
        // implicit "xml" namespace prefix
        mpNamespaceMap->Add( GetXMLToken(XML_XML), GetXMLToken(XML_N_XML), XML_NAMESPACE_XML );
        mpNamespaceMap->Add( "_office", GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
        mpNamespaceMap->Add( "_office_ooo", GetXMLToken(XML_N_OFFICE_EXT), XML_NAMESPACE_OFFICE_EXT );
        mpNamespaceMap->Add( "_ooo", GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
        mpNamespaceMap->Add( "_style", GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
        mpNamespaceMap->Add( "_text", GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );
        mpNamespaceMap->Add( "_table", GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
        mpNamespaceMap->Add( "_table_ooo", GetXMLToken(XML_N_TABLE_EXT), XML_NAMESPACE_TABLE_EXT );
        mpNamespaceMap->Add( "_draw", GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
        mpNamespaceMap->Add( "_draw_ooo", GetXMLToken(XML_N_DRAW_EXT), XML_NAMESPACE_DRAW_EXT );
        mpNamespaceMap->Add( "_dr3d", GetXMLToken(XML_N_DR3D), XML_NAMESPACE_DR3D );
        mpNamespaceMap->Add( "_fo", GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );
        mpNamespaceMap->Add( "_xlink", GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
        mpNamespaceMap->Add( "_dc", GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
        mpNamespaceMap->Add( "_dom", GetXMLToken(XML_N_DOM), XML_NAMESPACE_DOM );
        mpNamespaceMap->Add( "_meta", GetXMLToken(XML_N_META), XML_NAMESPACE_META );
        mpNamespaceMap->Add( "_number", GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );
        mpNamespaceMap->Add( "_svg", GetXMLToken(XML_N_SVG_COMPAT), XML_NAMESPACE_SVG );
        mpNamespaceMap->Add( "_chart", GetXMLToken(XML_N_CHART), XML_NAMESPACE_CHART );
        mpNamespaceMap->Add( "_math", GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
        mpNamespaceMap->Add( "_form", GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
        mpNamespaceMap->Add( "_script", GetXMLToken(XML_N_SCRIPT), XML_NAMESPACE_SCRIPT );
        mpNamespaceMap->Add( "_config", GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
        mpNamespaceMap->Add( "_xforms", GetXMLToken(XML_N_XFORMS_1_0), XML_NAMESPACE_XFORMS );
        mpNamespaceMap->Add( "_formx", GetXMLToken( XML_N_FORMX ), XML_NAMESPACE_FORMX );
        mpNamespaceMap->Add( "_xsd", GetXMLToken(XML_N_XSD), XML_NAMESPACE_XSD );
        mpNamespaceMap->Add( "_xsi", GetXMLToken(XML_N_XSI), XML_NAMESPACE_XFORMS );
        mpNamespaceMap->Add( "_ooow", GetXMLToken(XML_N_OOOW), XML_NAMESPACE_OOOW );
        mpNamespaceMap->Add( "_oooc", GetXMLToken(XML_N_OOOC), XML_NAMESPACE_OOOC );
        mpNamespaceMap->Add( "_field", GetXMLToken(XML_N_FIELD), XML_NAMESPACE_FIELD );
        mpNamespaceMap->Add( "_of", GetXMLToken(XML_N_OF), XML_NAMESPACE_OF );
        mpNamespaceMap->Add( "_xhtml", GetXMLToken(XML_N_XHTML), XML_NAMESPACE_XHTML );
        mpNamespaceMap->Add( "_css3text", GetXMLToken(XML_N_CSS3TEXT), XML_NAMESPACE_CSS3TEXT );

        mpNamespaceMap->Add( "_calc_libo", GetXMLToken(XML_N_CALC_EXT), XML_NAMESPACE_CALC_EXT);
        mpNamespaceMap->Add( "_office_libo",
                             GetXMLToken(XML_N_LO_EXT), XML_NAMESPACE_LO_EXT);
    }

    msPackageProtocol = "vnd.sun.star.Package:";

    if (mxNumberFormatsSupplier.is())
        mpNumImport = o3tl::make_unique<SvXMLNumFmtHelper>(mxNumberFormatsSupplier, GetComponentContext());

    if (mxModel.is() && !mxEventListener.is())
    {
        mxEventListener.set(new SvXMLImportEventListener(this));
        mxModel->addEventListener(mxEventListener);
    }
}

SvXMLImport::SvXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags )
:   mpImpl( new SvXMLImport_Impl(xContext, implementationName) ),
    mpNamespaceMap( new SvXMLNamespaceMap ),

    mpUnitConv( new SvXMLUnitConverter( xContext,
                util::MeasureUnit::MM_100TH, util::MeasureUnit::MM_100TH) ),

    mnImportFlags( nImportFlags ),
    mnErrorFlags(SvXMLErrorFlags::NO),
    isFastContext( false ),
    maNamespaceHandler( new SvXMLImportFastNamespaceHandler() ),
    mxFastDocumentHandler( nullptr ),
    mxTokenHandler( new FastTokenHandler() ),
    mbIsFormsSupported( true ),
    mbIsTableShapeSupported( false ),
    mbIsGraphicLoadOnDemandSupported( true )
{
    SAL_WARN_IF( !xContext.is(), "xmloff.core", "got no service manager" );
    InitCtor_();
    mxParser = xml::sax::FastParser::create( xContext );
    setNamespaceHandler( maNamespaceHandler.get() );
    setTokenHandler( mxTokenHandler  );
    registerNamespaces();
}

SvXMLImport::~SvXMLImport() throw ()
{
    if (mxEventListener.is() && mxModel.is())
        mxModel->removeEventListener(mxEventListener);
    // clear context stacks first in case of parse error because the context
    // class dtors are full of application logic
    while (!maFastContexts.empty()) { maFastContexts.pop(); }
    while (!maContexts.empty()) { maContexts.pop(); }
}

namespace
{
    class theSvXMLImportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvXMLImportUnoTunnelId> {};
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLImport::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 &&
        0 == memcmp( theSvXMLImportUnoTunnelId::get().getSeq().getConstArray(),
                     rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

// XFastParser
void SAL_CALL SvXMLImport::parseStream( const xml::sax::InputSource& aInputSource )
{
    if ( mxFastDocumentHandler.is() )
        mxParser->setFastDocumentHandler( mxFastDocumentHandler );
    else
        mxParser->setFastDocumentHandler( this );

    mxParser->parseStream( aInputSource );
    mxParser->setFastDocumentHandler( nullptr );
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


void SAL_CALL SvXMLImport::startDocument()
{
    SAL_INFO( "xmloff.core", "{ SvXMLImport::startDocument" );
    if( !mxGraphicResolver.is() || !mxEmbeddedResolver.is() )
    {
        Reference< lang::XMultiServiceFactory > xFactory( mxModel,  UNO_QUERY );
        if( xFactory.is() )
        {
            try
            {
                if( !mxGraphicResolver.is() )
                {
                    // #99870# Import... instead of Export...
                    mxGraphicResolver.set(
                        xFactory->createInstance("com.sun.star.document.ImportGraphicObjectResolver"),
                        UNO_QUERY);
                    mpImpl->mbOwnGraphicResolver = mxGraphicResolver.is();
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
    }
}

void SAL_CALL SvXMLImport::endDocument()
{
    SAL_INFO( "xmloff.core", "} SvXMLImport::endDocument" );
    //  #i9518# All the stuff that accesses the document has to be done here, not in the dtor,
    //  because the SvXMLImport dtor might not be called until after the document has been closed.

    GetTextImport()->MapCrossRefHeadingFieldsHorribly();

    if (mpImpl->mpRDFaHelper.get())
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
        static_cast<SvXMLStylesContext *>(mxFontDecls.get())->Clear();
    if( mxStyles.is() )
        static_cast<SvXMLStylesContext *>(mxStyles.get())->Clear();
    if( mxAutoStyles.is() )
        static_cast<SvXMLStylesContext *>(mxAutoStyles.get())->Clear();
    if( mxMasterStyles.is() )
        static_cast<SvXMLStylesContext *>(mxMasterStyles.get())->Clear();

    // possible form-layer related knittings which can only be done when
    // the whole document exists
    if ( mxFormImport.is() )
        mxFormImport->documentDone();

    //  The shape import helper does the z-order sorting in the dtor,
    //  so it must be deleted here, too.
    mxShapeImport = nullptr;

    if( mpImpl->mbOwnGraphicResolver )
    {
        Reference< lang::XComponent > xComp( mxGraphicResolver, UNO_QUERY );
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

std::unique_ptr<SvXMLNamespaceMap> SvXMLImport::processNSAttributes(
        const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    std::unique_ptr<SvXMLNamespaceMap> pRewindMap;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        if ( rAttrName == "office:version" )
        {
            mpImpl->aODFVersion = xAttrList->getValueByIndex( i );

            // the ODF version in content.xml and manifest.xml must be the same starting from ODF1.2
            if ( mpImpl->mStreamName == "content.xml" && !IsODFVersionConsistent( mpImpl->aODFVersion ) )
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
                pRewindMap = std::move(mpNamespaceMap);
                mpNamespaceMap.reset(new SvXMLNamespaceMap(*pRewindMap));
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            OUString aPrefix( ( rAttrName.getLength() == 5 )
                                 ? OUString()
                                 : rAttrName.copy( 6 ) );
            // Add namespace, but only if it is known.
            sal_uInt16 nKey = mpNamespaceMap->AddIfKnown( aPrefix, rAttrValue );
            // If namespace is unknown, try to match a name with similar
            // TC Id an version
            if( XML_NAMESPACE_UNKNOWN == nKey  )
            {
                OUString aTestName( rAttrValue );
                if( SvXMLNamespaceMap::NormalizeURI( aTestName ) )
                    nKey = mpNamespaceMap->AddIfKnown( aPrefix, aTestName );
            }
            // If that namespace is not known, too, add it as unknown
            if( XML_NAMESPACE_UNKNOWN == nKey  )
                mpNamespaceMap->Add( aPrefix, rAttrValue );

        }
    }
    return pRewindMap;
}

void SAL_CALL SvXMLImport::startElement( const OUString& rName,
                                         const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    //    SAL_INFO("svg", "startElement " << rName);
    // Process namespace attributes. This must happen before creating the
    // context, because namespace decaration apply to the element name itself.
    std::unique_ptr<SvXMLNamespaceMap> pRewindMap(processNSAttributes(xAttrList));

    // Get element's namespace and local name.
    OUString aLocalName;
    sal_uInt16 nPrefix =
        mpNamespaceMap->GetKeyByAttrName( rName, &aLocalName );

    // If there are contexts already, call a CreateChildContext at the topmost
    // context. Otherwise, create a default context.
    SvXMLImportContextRef xContext;
    if(!maContexts.empty())
    {
        xContext.set(maContexts.top()->CreateChildContext(nPrefix, aLocalName, xAttrList));
        SAL_WARN_IF( !xContext.is() || (xContext->GetPrefix() != nPrefix), "xmloff.core",
                "SvXMLImport::startElement: created context has wrong prefix" );
    }
    else
    {
        xContext.set(CreateContext( nPrefix, aLocalName, xAttrList ));
        if( (nPrefix & XML_NAMESPACE_UNKNOWN_FLAG) != 0 &&
            dynamic_cast< const SvXMLImportContext*>(xContext.get()) !=  nullptr )
        {
            OUString aMsg( "Root element unknown" );
            Reference<xml::sax::XLocator> xDummyLocator;
            Sequence < OUString > aParams { rName };

            SetError( XMLERROR_FLAG_SEVERE|XMLERROR_UNKNOWN_ROOT,
                      aParams, aMsg, xDummyLocator );
        }
    }

    SAL_WARN_IF( !xContext.is(), "xmloff.core", "SvXMLImport::startElement: missing context" );
    if( !xContext.is() )
        xContext.set(new SvXMLImportContext( *this, nPrefix, aLocalName ));

    // Remember old namespace map.
    if( pRewindMap )
        xContext->PutRewindMap(std::move(pRewindMap));

    // Call a startElement at the new context.
    xContext->StartElement( xAttrList );

    // Push context on stack.
    maContexts.push(xContext);
}

void SAL_CALL SvXMLImport::endElement( const OUString&
#ifdef DBG_UTIL
rName
#endif
)
{
    if (maContexts.empty())
    {
        SAL_WARN("xmloff.core", "SvXMLImport::endElement: no context left");
        return;
    }

    std::unique_ptr<SvXMLNamespaceMap> pRewindMap;

    {
        // Get topmost context and remove it from the stack.
        SvXMLImportContextRef xContext = maContexts.top();
        maContexts.pop();

#ifdef DBG_UTIL
        // Non product only: check if endElement call matches startELement call.
        OUString aLocalName;
        sal_uInt16 nPrefix =
            mpNamespaceMap->GetKeyByAttrName( rName, &aLocalName );
        SAL_WARN_IF( xContext->GetPrefix() != nPrefix,  "xmloff.core", "SvXMLImport::endElement: popped context has wrong prefix" );
        SAL_WARN_IF( xContext->GetLocalName() != aLocalName, "xmloff.core", "SvXMLImport::endElement: popped context has wrong lname" );
#endif

        // Call a EndElement at the current context.
        xContext->EndElement();
        // Get a namespace map to rewind.
        pRewindMap = xContext->TakeRewindMap();
        // note: delete xContext *before* rewinding namespace map!
    }

    // Rewind a namespace map.
    if (pRewindMap)
    {
        mpNamespaceMap.reset();
        mpNamespaceMap = std::move(pRewindMap);
    }
}

void SAL_CALL SvXMLImport::characters( const OUString& rChars )
{
    if ( !maFastContexts.empty() )
    {
        maFastContexts.top()->characters( rChars );
    }
    else if( !maContexts.empty() )
    {
        maContexts.top()->Characters( rChars );
    }
}

void SvXMLImport::Characters( const OUString& rChars )
{
    if( !maContexts.empty() )
    {
        maContexts.top()->Characters( rChars );
    }
}

void SAL_CALL SvXMLImport::ignorableWhitespace( const OUString& )
{
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
    //Namespace handling is unnecessary. It is done by the fastparser itself.
    uno::Reference<XFastContextHandler> xContext;
    if (!maFastContexts.empty())
    {
        uno::Reference<XFastContextHandler> pHandler = maFastContexts.top();
        xContext = pHandler->createFastChildContext( Element, Attribs );
    }
    else
        xContext.set( CreateFastContext( Element, Attribs ) );

    if ( !xContext.is() )
        xContext.set( new SvXMLImportContext( *this ) );

    isFastContext = true;

    // Call a startElement at the new context.
    xContext->startFastElement( Element, Attribs );

    if ( isFastContext )
    {
        if ( maNamespaceAttrList.is() )
            maNamespaceAttrList->Clear();
        else
            maNamespaceAttrList = new comphelper::AttributeList;

        maNamespaceHandler->addNSDeclAttributes( maNamespaceAttrList );
        std::unique_ptr<SvXMLNamespaceMap> pRewindMap(
                processNSAttributes( maNamespaceAttrList.get() ));
        SvXMLImportContext *pContext = dynamic_cast<SvXMLImportContext*>( xContext.get() );
        if( pContext && pRewindMap )
            pContext->PutRewindMap(std::move(pRewindMap));
        maContexts.push(pContext);
    }

    // Push context on stack.
    maFastContexts.push(xContext);
}

void SAL_CALL SvXMLImport::startUnknownElement (const OUString & rPrefix, const OUString & rLocalName,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    uno::Reference<XFastContextHandler> xContext;
    if (!maFastContexts.empty())
    {
        uno::Reference<XFastContextHandler> pHandler = maFastContexts.top();
        xContext = pHandler->createUnknownChildContext( rPrefix, rLocalName, Attribs );
    }
    else
        xContext.set( CreateFastContext( -1, Attribs ) );

    if ( !xContext.is() )
        xContext.set( new SvXMLImportContext( *this ) );

    xContext->startUnknownElement( rPrefix, rLocalName, Attribs );
    maFastContexts.push(xContext);
}

void SAL_CALL SvXMLImport::endFastElement (sal_Int32 Element)
{
    if (!maFastContexts.empty())
    {
        uno::Reference<XFastContextHandler> xContext = maFastContexts.top();
        maFastContexts.pop();
        isFastContext = true;
        xContext->endFastElement( Element );
        if (isFastContext)
            maContexts.pop();

        xContext = nullptr;
    }
}

void SAL_CALL SvXMLImport::endUnknownElement (const OUString & rPrefix, const OUString & rLocalName)
{
    if (!maFastContexts.empty())
    {
        uno::Reference<XFastContextHandler> xContext = maFastContexts.top();
        maFastContexts.pop();
        xContext->endUnknownElement( rPrefix, rLocalName );
        xContext = nullptr;
    }
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

// XExtendedDocumentHandler
void SAL_CALL SvXMLImport::startCDATA()
{
}

void SAL_CALL SvXMLImport::endCDATA()
{
}

void SAL_CALL SvXMLImport::comment( const OUString& )
{
}

void SAL_CALL SvXMLImport::allowLineBreak()
{
}

void SAL_CALL SvXMLImport::unknown( const OUString& )
{
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
        uno::Reference<document::XStorageBasedDocument> const xSBDoc(mxModel,
                uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> const xStor(
                xSBDoc->getDocumentStorage());
        if (xStor.is())
        {
            mpImpl->mbIsOOoXML =
                ::comphelper::OStorageHelper::GetXStorageFormat(xStor)
                   < SOFFICE_FILEFORMAT_8;
        }
    }
    catch (uno::Exception const&)
    {
        SAL_WARN("xmloff.core", "exception caught");
        DBG_UNHANDLED_EXCEPTION();
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
    const sal_Int32 nAnyCount = aArguments.getLength();
    const uno::Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        Reference<XInterface> xValue;
        *pAny >>= xValue;

        uno::Reference<task::XStatusIndicator> xTmpStatusIndicator(
            xValue, UNO_QUERY );
        if( xTmpStatusIndicator.is() )
            mxStatusIndicator = xTmpStatusIndicator;

        uno::Reference<document::XGraphicObjectResolver> xTmpGraphicResolver(
            xValue, UNO_QUERY );
        if( xTmpGraphicResolver.is() )
            mxGraphicResolver = xTmpGraphicResolver;

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

                    StyleMap *pSMap = StyleMap::getImplementation( xIfc );
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
                    aAny >>= (mpImpl->mbShapePositionInHoriL2R);
                }
                sPropName = "TextDocInOOoFileFormat";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                {
                    uno::Any aAny = mxImportInfo->getPropertyValue(sPropName);
                    aAny >>= (mpImpl->mbTextDocInOOoFileFormat);
                }

                sPropName = "SourceStorage";
                if( xPropertySetInfo->hasPropertyByName(sPropName) )
                    mxImportInfo->getPropertyValue(sPropName) >>= mpImpl->mxSourceStorage;
            }
        }
    }
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
    uno::Sequence<OUString> aSeq(2);
    aSeq[0] = "com.sun.star.document.ImportFilter";
    aSeq[1] = "com.sun.star.xml.XMLImportFilter";
    return aSeq;
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
    return new SchXMLImportHelper();
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
    if( !mxDashHelper.is() )
    {
        if( mxModel.is() )
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
    }

    return mxDashHelper;
}

bool SvXMLImport::IsPackageURL( const OUString& rURL ) const
{

    // if, and only if, only parts are imported, then we're in a package
    const SvXMLImportFlags nTest = SvXMLImportFlags::META|SvXMLImportFlags::STYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SETTINGS;
    if( (mnImportFlags & nTest) == nTest )
        return false;

    // Some quick tests: Some may rely on the package structure!
    sal_Int32 nLen = rURL.getLength();
    if( (nLen > 0 && '/' == rURL[0]) )
        // RFC2396 net_path or abs_path
        return false;
    else if( nLen > 1 && '.' == rURL[0] )
    {
        if( '.' == rURL[1] )
            // ../: We are never going up one level, so we know
            // it's not an external URI
            return false;
        else if( '/' == rURL[1] )
            // we are remaining on a level, so it's an package URI
            return true;
    }

    // Now check for a RFC2396 schema
    sal_Int32 nPos = 1;
    while( nPos < nLen )
    {
        switch( rURL[nPos] )
        {
        case '/':
            // a relative path segement
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

OUString SvXMLImport::ResolveGraphicObjectURL( const OUString& rURL,
                                                      bool bLoadOnDemand )
{
    OUString sRet;

    if( IsPackageURL( rURL ) )
    {
        if( !bLoadOnDemand && mxGraphicResolver.is() )
        {
            OUString     aTmp( msPackageProtocol );
            aTmp += rURL;
            sRet = mxGraphicResolver->resolveGraphicObjectURL( aTmp );
        }

        if( sRet.isEmpty() )
        {
            sRet = msPackageProtocol;
            sRet += rURL;
        }
    }

    if( sRet.isEmpty() )
        sRet = GetAbsoluteReference( rURL );

    return sRet;
}

Reference< XOutputStream > SvXMLImport::GetStreamForGraphicObjectURLFromBase64()
{
    Reference< XOutputStream > xOStm;
    Reference< document::XBinaryStreamResolver > xStmResolver( mxGraphicResolver, UNO_QUERY );

    if( xStmResolver.is() )
        xOStm = xStmResolver->createOutputStream();

    return xOStm;
}

OUString SvXMLImport::ResolveGraphicObjectURLFromBase64(
                                 const Reference < XOutputStream >& rOut )
{
    OUString sURL;
    Reference< document::XBinaryStreamResolver > xStmResolver( mxGraphicResolver, UNO_QUERY );
    if( xStmResolver.is() )
        sURL = xStmResolver->resolveOutputStream( rOut );

    return sURL;
}

OUString SvXMLImport::ResolveEmbeddedObjectURL(
                                    const OUString& rURL,
                                    const OUString& rClassId )
{
    OUString sRet;

    if( IsPackageURL( rURL ) )
    {
        if ( mxEmbeddedResolver.is() )
        {
            OUString sURL( rURL );
            if( !rClassId.isEmpty() )
            {
                sURL += "!" + rClassId;
            }
            sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( sURL );
        }
    }
    else
        sRet = GetAbsoluteReference( rURL );

    return sRet;
}

Reference< embed::XStorage > SvXMLImport::GetSourceStorage()
{
    return mpImpl->mxSourceStorage;
}

Reference < XOutputStream >
        SvXMLImport::GetStreamForEmbeddedObjectURLFromBase64()
{
    Reference < XOutputStream > xOLEStream;

    if( mxEmbeddedResolver.is() )
    {
        Reference< XNameAccess > xNA( mxEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            OUString aURL( "Obj12345678" );
            Any aAny = xNA->getByName( aURL );
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
        OUString aURL( "Obj12345678" );
        sRet = mxEmbeddedResolver->resolveEmbeddedObjectURL( aURL );
    }

    return sRet;
}

void SvXMLImport::AddStyleDisplayName( sal_uInt16 nFamily,
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
       "duplicate style name of family " << nFamily << ": \"" << rName << "\"");

}

OUString SvXMLImport::GetStyleDisplayName( sal_uInt16 nFamily,
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

void SvXMLImport::SetDocumentSpecificSettings(const OUString& _rSettingsGroupName, const uno::Sequence<beans::PropertyValue>& _rSettings)
{
    (void)_rSettingsGroupName;
    (void)_rSettings;
}

ProgressBarHelper*  SvXMLImport::GetProgressBarHelper()
{
    if (!mpProgressBarHelper)
    {
        mpProgressBarHelper = o3tl::make_unique<ProgressBarHelper>(mxStatusIndicator, false);

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
            SAL_WARN( "xmloff.core", "Numberformat could not be inserted");
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else {
        SAL_WARN( "xmloff.core", "not possible to create NameContainer");
        DBG_UNHANDLED_EXCEPTION();
    }
}

XMLEventImportHelper& SvXMLImport::GetEventImport()
{
    if (!mpEventImportHelper)
    {
        // construct event helper and register StarBasic handler and standard
        // event tables
        mpEventImportHelper = o3tl::make_unique<XMLEventImportHelper>();
        const OUString& sStarBasic(GetXMLToken(XML_STARBASIC));
        mpEventImportHelper->RegisterFactory(sStarBasic,
                                            new XMLStarBasicContextFactory());
        const OUString& sScript(GetXMLToken(XML_SCRIPT));
        mpEventImportHelper->RegisterFactory(sScript,
                                            new XMLScriptContextFactory());
        mpEventImportHelper->AddTranslationTable(aStandardEventTable);

        // register StarBasic event handler with capitalized spelling
        OUString sStarBasicCap( "StarBasic" );
        mpEventImportHelper->RegisterFactory(sStarBasicCap,
                                            new XMLStarBasicContextFactory());
    }

    return *mpEventImportHelper;
}

void SvXMLImport::SetFontDecls( XMLFontStylesContext *pFontDecls )
{
    mxFontDecls = pFontDecls;
}

void SvXMLImport::SetStyles( SvXMLStylesContext *pStyles )
{
    mxStyles = pStyles;
}

void SvXMLImport::SetAutoStyles( SvXMLStylesContext *pAutoStyles )
{
    if (pAutoStyles && mxNumberStyles.is() && (mnImportFlags & SvXMLImportFlags::CONTENT) )
    {
        uno::Reference<xml::sax::XAttributeList> xAttrList;
        uno::Sequence< OUString > aNames = mxNumberStyles->getElementNames();
        sal_uInt32 nCount(aNames.getLength());
        if (nCount)
        {
            const OUString* pNames = aNames.getConstArray();
            if ( pNames )
            {
                SvXMLStyleContext* pContext;
                uno::Any aAny;
                sal_Int32 nKey(0);
                for (sal_uInt32 i = 0; i < nCount; i++, pNames++)
                {
                    aAny = mxNumberStyles->getByName(*pNames);
                    if (aAny >>= nKey)
                    {
                        pContext = new SvXMLNumFormatContext( *this, XML_NAMESPACE_NUMBER,
                                    *pNames, xAttrList, nKey, *pAutoStyles );
                        pAutoStyles->AddStyle(*pContext);
                    }
                }
            }
        }
    }
    mxAutoStyles = pAutoStyles;
    GetTextImport()->SetAutoStyles( pAutoStyles );
    GetShapeImport()->SetAutoStylesContext( pAutoStyles );
    GetChartImport()->SetAutoStylesContext( pAutoStyles );
    GetFormImport()->setAutoStyleContext( pAutoStyles );
}

void SvXMLImport::SetMasterStyles( SvXMLStylesContext *pMasterStyles )
{
    mxMasterStyles = pMasterStyles;
}

XMLFontStylesContext *SvXMLImport::GetFontDecls()
{
    return static_cast<XMLFontStylesContext *>(mxFontDecls.get());
}

SvXMLStylesContext *SvXMLImport::GetStyles()
{
    return static_cast<SvXMLStylesContext *>(mxStyles.get());
}

SvXMLStylesContext *SvXMLImport::GetAutoStyles()
{
    return static_cast<SvXMLStylesContext *>(mxAutoStyles.get());
}

const XMLFontStylesContext *SvXMLImport::GetFontDecls() const
{
    return static_cast<const XMLFontStylesContext *>(mxFontDecls.get());
}

const SvXMLStylesContext *SvXMLImport::GetStyles() const
{
    return static_cast<const SvXMLStylesContext *>(mxStyles.get());
}

const SvXMLStylesContext *SvXMLImport::GetAutoStyles() const
{
    return static_cast<const SvXMLStylesContext *>(mxAutoStyles.get());
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
        {
            uno::Reference< document::XStorageBasedDocument > xDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< embed::XStorage > xStor = xDoc->getDocumentStorage();
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
                        bResult = aODFVersion.equals( aStorVersion );
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
        mpNumImport = o3tl::make_unique<SvXMLNumFmtHelper>(xNum, GetComponentContext() );
}

sal_Unicode SvXMLImport::ConvStarBatsCharToStarSymbol( sal_Unicode c )
{
    sal_Unicode cNew = c;
    if( !mpImpl->hBatsFontConv )
    {
        OUString sStarBats( "StarBats" );
        mpImpl->hBatsFontConv = CreateFontToSubsFontConverter( sStarBats,
                 FontToSubsFontFlags::IMPORT|FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
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
        OUString sStarMath( "StarMath" );
        mpImpl->hMathFontConv = CreateFontToSubsFontConverter( sStarMath,
                 FontToSubsFontFlags::IMPORT|FontToSubsFontFlags::ONLYOLDSOSYMBOLFONTS );
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
    // maintain error flags
    if ( ( nId & XMLERROR_FLAG_ERROR ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::ERROR_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_WARNING ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::WARNING_OCCURRED;
    if ( ( nId & XMLERROR_FLAG_SEVERE ) != 0 )
        mnErrorFlags |= SvXMLErrorFlags::DO_NOTHING;

    // create error list on demand
    if ( !mpXMLErrors )
        mpXMLErrors = o3tl::make_unique<XMLErrors>();

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
    sal_Int32 nId)
{
    Sequence<OUString> aSeq(0);
    SetError( nId, aSeq );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const OUString& rMsg1)
{
    Sequence<OUString> aSeq { rMsg1 };
    SetError( nId, aSeq );
}

void SvXMLImport::SetError(
    sal_Int32 nId,
    const OUString& rMsg1,
    const OUString& rMsg2)
{
    Sequence<OUString> aSeq(2);
    OUString* pSeq = aSeq.getArray();
    pSeq[0] = rMsg1;
    pSeq[1] = rMsg2;
    SetError( nId, aSeq );
}

void SvXMLImport::DisposingModel()
{
    if( mxFontDecls.is() )
        static_cast<SvXMLStylesContext *>(mxFontDecls.get())->Clear();
    if( mxStyles.is() )
        static_cast<SvXMLStylesContext *>(mxStyles.get())->Clear();
    if( mxAutoStyles.is() )
        static_cast<SvXMLStylesContext *>(mxAutoStyles.get())->Clear();
    if( mxMasterStyles.is() )
        static_cast<SvXMLStylesContext *>(mxMasterStyles.get())->Clear();

    mxModel.set(nullptr);
    mxEventListener.set(nullptr);
}

::comphelper::UnoInterfaceToUniqueIdentifierMapper& SvXMLImport::getInterfaceToIdentifierMapper()
{
    return mpImpl->maInterfaceToIdentifierMapper;
}

uno::Reference< uno::XComponentContext >
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
    return mpImpl->aODFVersion;
}

bool SvXMLImport::IsOOoXML() const
{
    return mpImpl->mbIsOOoXML;
}

// xml:id for RDF metadata
void SvXMLImport::SetXmlId(uno::Reference<uno::XInterface> const & i_xIfc,
    OUString const & i_rXmlId)
{
    if (!i_rXmlId.isEmpty()) {
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
            SAL_WARN("xmloff.core","SvXMLImport::SetXmlId: exception?");
        }
    }
}

::xmloff::RDFaImportHelper &
SvXMLImport::GetRDFaImportHelper()
{
    if (!mpImpl->mpRDFaHelper.get())
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

const OUString SvXMLImport::getNameFromToken( sal_Int32 nToken )
{
    uno::Sequence< sal_Int8 > aSeq = mxTokenHandler->getUTF8Identifier( nToken & TOKEN_MASK );
    return OUString( reinterpret_cast< const char* >(
                    aSeq.getConstArray() ), aSeq.getLength(), RTL_TEXTENCODING_UTF8 );
}

const OUString SvXMLImport::getNamespacePrefixFromToken( sal_Int32 nToken )
{
    sal_Int32 nNamespaceToken = ( nToken & NMSP_MASK ) >> NMSP_SHIFT;
    auto aIter( maNamespaceMap.find( nNamespaceToken ) );
    if( aIter != maNamespaceMap.end() )
        return (*aIter).second;
    else
        return OUString();
}

void SvXMLImport::registerNamespaces()
{
    registerNSHelper( XML_NAMESPACE_OFFICE + 1, XML_NP_OFFICE, XML_N_OFFICE );
    registerNSHelper( XML_NAMESPACE_OFFICE + 1, XML_NP_OFFICE, XML_N_OFFICE_OLD );
    registerNSHelper( XML_NAMESPACE_STYLE + 1, XML_NP_STYLE, XML_N_STYLE );
    registerNSHelper( XML_NAMESPACE_STYLE + 1, XML_NP_STYLE, XML_N_STYLE_OLD );
    registerNSHelper( XML_NAMESPACE_TEXT + 1, XML_NP_TEXT, XML_N_TEXT );
    registerNSHelper( XML_NAMESPACE_TEXT + 1, XML_NP_TEXT, XML_N_TEXT_OLD );
    registerNSHelper( XML_NAMESPACE_TABLE + 1, XML_NP_TABLE, XML_N_TABLE );
    registerNSHelper( XML_NAMESPACE_TABLE + 1, XML_NP_TABLE, XML_N_TABLE_OLD );
    registerNSHelper( XML_NAMESPACE_DRAW + 1, XML_NP_DRAW, XML_N_DRAW );
    registerNSHelper( XML_NAMESPACE_DRAW + 1, XML_NP_DRAW, XML_N_DRAW_OLD );
    registerNSHelper( XML_NAMESPACE_FO + 1, XML_NP_FO, XML_N_FO );
    registerNSHelper( XML_NAMESPACE_FO + 1, XML_NP_FO, XML_N_FO_OLD );
    registerNSHelper( XML_NAMESPACE_FO + 1, XML_NP_FO, XML_N_FO_COMPAT );
    registerNSHelper( XML_NAMESPACE_XLINK + 1, XML_NP_XLINK, XML_N_XLINK );
    registerNSHelper( XML_NAMESPACE_XLINK + 1, XML_NP_XLINK, XML_N_XLINK_OLD );
    registerNSHelper( XML_NAMESPACE_DC + 1, XML_NP_DC, XML_N_DC );
    registerNSHelper( XML_NAMESPACE_META + 1, XML_NP_META, XML_N_META );
    registerNSHelper( XML_NAMESPACE_META + 1, XML_NP_META, XML_N_META_OLD );
    registerNSHelper( XML_NAMESPACE_NUMBER + 1, XML_NP_NUMBER, XML_N_NUMBER );
    registerNSHelper( XML_NAMESPACE_NUMBER + 1, XML_NP_NUMBER, XML_N_NUMBER_OLD );
    registerNSHelper( XML_NAMESPACE_PRESENTATION + 1, XML_NP_PRESENTATION, XML_N_PRESENTATION );
    registerNSHelper( XML_NAMESPACE_PRESENTATION + 1, XML_NP_PRESENTATION, XML_N_PRESENTATION_OLD );
    registerNSHelper( XML_NAMESPACE_SVG + 1, XML_NP_SVG, XML_N_SVG );
    registerNSHelper( XML_NAMESPACE_SVG + 1, XML_NP_SVG, XML_N_SVG_COMPAT );
    registerNSHelper( XML_NAMESPACE_CHART + 1, XML_NP_CHART, XML_N_CHART );
    registerNSHelper( XML_NAMESPACE_CHART + 1, XML_NP_CHART, XML_N_CHART_OLD );
    registerNSHelper( XML_NAMESPACE_DR3D + 1, XML_NP_DR3D, XML_N_DR3D );
    registerNSHelper( XML_NAMESPACE_MATH + 1, XML_NP_MATH, XML_N_MATH );
    registerNSHelper( XML_NAMESPACE_FORM + 1, XML_NP_FORM, XML_N_FORM );
    registerNSHelper( XML_NAMESPACE_SCRIPT + 1, XML_NP_SCRIPT, XML_N_SCRIPT );
    registerNSHelper( XML_NAMESPACE_BLOCKLIST + 1, XML_NP_BLOCK_LIST, XML_N_BLOCK_LIST );
    registerNSHelper( XML_NAMESPACE_CONFIG + 1, XML_NP_CONFIG, XML_N_CONFIG );
    registerNSHelper( XML_NAMESPACE_OOO + 1, XML_NP_OOO, XML_N_OOO );
    registerNSHelper( XML_NAMESPACE_OOOW + 1, XML_NP_OOOW, XML_N_OOOW );
    registerNSHelper( XML_NAMESPACE_OOOC + 1, XML_NP_OOOC, XML_N_OOOC );
    registerNSHelper( XML_NAMESPACE_DOM + 1, XML_NP_DOM, XML_N_DOM );
    registerNSHelper( XML_NAMESPACE_DB + 1, XML_NP_DB, XML_N_DB );
    registerNSHelper( XML_NAMESPACE_DB + 1, XML_NP_DB, XML_N_DB_OASIS );
    registerNSHelper( XML_NAMESPACE_DLG + 1, XML_NP_DLG, XML_N_DLG );
    registerNSHelper( XML_NAMESPACE_XFORMS + 1, XML_NP_XFORMS_1_0, XML_N_XFORMS_1_0 );
    registerNSHelper( XML_NAMESPACE_XSD + 1, XML_NP_XSD, XML_N_XSD );
    registerNSHelper( XML_NAMESPACE_XSI + 1, XML_NP_XSI, XML_N_XSI );
    registerNSHelper( XML_NAMESPACE_SMIL + 1, XML_NP_SMIL, XML_N_SMIL );
    registerNSHelper( XML_NAMESPACE_SMIL + 1, XML_NP_SMIL, XML_N_SMIL_OLD );
    registerNSHelper( XML_NAMESPACE_SMIL + 1, XML_NP_SMIL, XML_N_SMIL_COMPAT );
    registerNSHelper( XML_NAMESPACE_ANIMATION + 1, XML_NP_ANIMATION, XML_N_ANIMATION );
    registerNSHelper( XML_NAMESPACE_REPORT + 1, XML_NP_RPT, XML_N_RPT );
    registerNSHelper( XML_NAMESPACE_REPORT + 1, XML_NP_RPT, XML_N_RPT_OASIS );
    registerNSHelper( XML_NAMESPACE_OF + 1, XML_NP_OF, XML_N_OF );
    registerNSHelper( XML_NAMESPACE_XHTML + 1, XML_NP_XHTML, XML_N_XHTML );
    registerNSHelper( XML_NAMESPACE_GRDDL + 1, XML_NP_GRDDL, XML_N_GRDDL );
    registerNSHelper( XML_NAMESPACE_OFFICE_EXT + 1, XML_NP_OFFICE_EXT, XML_N_OFFICE_EXT );
    registerNSHelper( XML_NAMESPACE_TABLE_EXT + 1, XML_NP_TABLE_EXT, XML_N_TABLE_EXT );
    registerNSHelper( XML_NAMESPACE_CHART_EXT + 1, XML_NP_CHART_EXT, XML_N_CHART_EXT );
    registerNSHelper( XML_NAMESPACE_DRAW_EXT + 1, XML_NP_DRAW_EXT, XML_N_DRAW_EXT );
    registerNSHelper( XML_NAMESPACE_CALC_EXT + 1, XML_NP_CALC_EXT, XML_N_CALC_EXT );
    registerNSHelper( XML_NAMESPACE_LO_EXT + 1, XML_NP_LO_EXT, XML_N_LO_EXT );
    registerNSHelper( XML_NAMESPACE_CSS3TEXT + 1, XML_NP_CSS3TEXT, XML_N_CSS3TEXT );
    registerNSHelper( XML_NAMESPACE_FIELD + 1, XML_NP_FIELD, XML_N_FIELD );
    registerNSHelper( XML_NAMESPACE_FORMX + 1, XML_NP_FORMX, XML_N_FORMX );
}

void SvXMLImport::registerNSHelper(sal_Int32 nToken, sal_Int32 nPrefix, sal_Int32 nNamespace )
{
    if ( nToken > 0 )
    {
        maNamespaceMap[ nToken ] = GetXMLToken( static_cast<XMLTokenEnum>( nPrefix ) );
        registerNamespace( GetXMLToken( static_cast<XMLTokenEnum>( nNamespace ) ), nToken << NMSP_SHIFT );
    }
}

SvXMLImportFastNamespaceHandler::SvXMLImportFastNamespaceHandler()
{
}

void SvXMLImportFastNamespaceHandler::addNSDeclAttributes( rtl::Reference < comphelper::AttributeList >& rAttrList )
{
    for(const auto& aNamespaceDefine : m_aNamespaceDefines)
    {
        OUString& rPrefix = aNamespaceDefine.get()->m_aPrefix;
        OUString& rNamespaceURI = aNamespaceDefine.get()->m_aNamespaceURI;
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
    m_aNamespaceDefines.push_back( o3tl::make_unique<NamespaceDefine>(
                                    rNamespacePrefix, rNamespaceURI) );
}

OUString SvXMLImportFastNamespaceHandler::getNamespaceURI( const OUString&/* rNamespacePrefix */ )
{
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

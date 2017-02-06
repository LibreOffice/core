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


/*todo: Change characters and tcharacters to accumulate the characters together
into one string, xml parser hands them to us line by line rather than all in
one go*/

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/uno/Any.h>

#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <o3tl/make_unique.hxx>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <osl/diagnose.h>
#include <svtools/sfxecode.hxx>
#include <unotools/saveopt.hxx>
#include <svl/stritem.hxx>
#include <svl/itemprop.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlmetai.hxx>
#include <osl/mutex.hxx>

#include <memory>

#include "mathmlattr.hxx"
#include "mathmlimport.hxx"
#include "register.hxx"
#include <starmath.hrc>
#include <unomodel.hxx>
#include <document.hxx>
#include <utility.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;


#define IMPORT_SVC_NAME "com.sun.star.xml.XMLImportFilter"


sal_uLong SmXMLImportWrapper::Import(SfxMedium &rMedium)
{
    sal_uLong nError = ERRCODE_SFX_DOLOADFAILED;

    uno::Reference<uno::XComponentContext> xContext( comphelper::getProcessComponentContext() );

    //Make a model component from our SmModel
    uno::Reference< lang::XComponent > xModelComp( xModel, uno::UNO_QUERY );
    OSL_ENSURE( xModelComp.is(), "XMLReader::Read: got no model" );

    // try to get an XStatusIndicator from the Medium
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    bool bEmbedded = false;
    uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    SmDocShell *pDocShell = pModel ?
            static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    if (pDocShell)
    {
        OSL_ENSURE( pDocShell->GetMedium() == &rMedium,
                "different SfxMedium found" );

        SfxItemSet* pSet = rMedium.GetItemSet();
        if (pSet)
        {
            const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(
                pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );
            if (pItem)
                pItem->GetValue() >>= xStatusIndicator;
        }

        if ( SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode() )
            bEmbedded = true;
    }

    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { OUString("PrivateData"), 0,
              cppu::UnoType<XInterface>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("BaseURI"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamRelPath"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamName"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    // Set base URI
    OUString const baseURI(rMedium.GetBaseURL());
    // needed for relative URLs; but it's OK to import e.g. MathML from the
    // clipboard without one
    SAL_INFO_IF(baseURI.isEmpty(), "starmath", "SmXMLImportWrapper: no base URL");
    xInfoSet->setPropertyValue("BaseURI", makeAny(baseURI));

    sal_Int32 nSteps=3;
    if ( !(rMedium.IsStorage()))
        nSteps = 1;

    sal_Int32 nProgressRange(nSteps);
    if (xStatusIndicator.is())
    {
        xStatusIndicator->start(SM_RESSTR(STR_STATSTR_READING), nProgressRange);
    }

    nSteps=0;
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    if ( rMedium.IsStorage())
    {
        // TODO/LATER: handle the case of embedded links gracefully
        if ( bEmbedded ) // && !rMedium.GetStorage()->IsRoot() )
        {
            OUString aName( "dummyObjName" );
            if ( rMedium.GetItemSet() )
            {
                const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                    rMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
                if ( pDocHierarchItem )
                    aName = pDocHierarchItem->GetValue();
            }

            if ( !aName.isEmpty() )
            {
                xInfoSet->setPropertyValue("StreamRelPath", makeAny(aName));
            }
        }

        bool bOASIS = ( SotStorage::GetVersion( rMedium.GetStorage() ) > SOFFICE_FILEFORMAT_60 );
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        sal_uLong nWarn = ReadThroughComponent(
            rMedium.GetStorage(), xModelComp, "meta.xml", "Meta.xml",
            xContext, xInfoSet,
                (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaImporter"
                        : "com.sun.star.comp.Math.XMLMetaImporter") );

        if ( nWarn != ERRCODE_IO_BROKENPACKAGE )
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            nWarn = ReadThroughComponent(
                rMedium.GetStorage(), xModelComp, "settings.xml", nullptr,
                xContext, xInfoSet,
                (bOASIS ? "com.sun.star.comp.Math.XMLOasisSettingsImporter"
                        : "com.sun.star.comp.Math.XMLSettingsImporter" ) );

            if ( nWarn != ERRCODE_IO_BROKENPACKAGE )
            {
                if (xStatusIndicator.is())
                    xStatusIndicator->setValue(nSteps++);

                nError = ReadThroughComponent(
                    rMedium.GetStorage(), xModelComp, "content.xml", "Content.xml",
                    xContext, xInfoSet, "com.sun.star.comp.Math.XMLImporter" );
            }
            else
                nError = ERRCODE_IO_BROKENPACKAGE;
        }
        else
            nError = ERRCODE_IO_BROKENPACKAGE;
    }
    else
    {
        Reference<io::XInputStream> xInputStream =
            new utl::OInputStreamWrapper(rMedium.GetInStream());

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        nError = ReadThroughComponent( xInputStream, xModelComp,
            xContext, xInfoSet, "com.sun.star.comp.Math.XMLImporter", false );
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return nError;
}


/// read a component (file + filter version)
sal_uLong SmXMLImportWrapper::ReadThroughComponent(
    const Reference<io::XInputStream>& xInputStream,
    const Reference<XComponent>& xModelComponent,
    Reference<uno::XComponentContext> & rxContext,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pFilterName,
    bool bEncrypted )
{
    sal_uLong nError = ERRCODE_SFX_DOLOADFAILED;
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rxContext.is(), "factory missing");
    OSL_ENSURE(nullptr != pFilterName,"I need a service name for the component!");

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(rxContext);

    Sequence<Any> aArgs( 1 );
    aArgs[0] <<= rPropSet;

    // get filter
    Reference< xml::sax::XDocumentHandler > xFilter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pFilterName), aArgs, rxContext),
        UNO_QUERY );
    OSL_ENSURE( xFilter.is(), "Can't instantiate filter component." );
    if ( !xFilter.is() )
        return nError;

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // connect model and filter
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );

        uno::Reference<lang::XUnoTunnel> xFilterTunnel( xFilter, uno::UNO_QUERY );
        SmXMLImport *pFilter = reinterpret_cast< SmXMLImport * >(
                sal::static_int_cast< sal_uIntPtr >(
                xFilterTunnel->getSomething( SmXMLImport::getUnoTunnelId() )));
        if ( pFilter && pFilter->GetSuccess() )
            nError = 0;
    }
    catch( xml::sax::SAXParseException& r )
    {
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *static_cast<xml::sax::SAXException*>(&r);
        bool bTryChild = true;

        while( bTryChild )
        {
            xml::sax::SAXException aTmp;
            if ( aSaxEx.WrappedException >>= aTmp )
                aSaxEx = aTmp;
            else
                bTryChild = false;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if ( aSaxEx.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if ( bEncrypted )
            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch( const xml::sax::SAXException& r )
    {
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if ( bEncrypted )
            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch( packages::zip::ZipIOException& )
    {
        nError = ERRCODE_IO_BROKENPACKAGE;
    }
    catch( io::IOException& )
    {
    }

    return nError;
}


sal_uLong SmXMLImportWrapper::ReadThroughComponent(
    const uno::Reference< embed::XStorage >& xStorage,
    const Reference<XComponent>& xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    Reference<uno::XComponentContext> & rxContext,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pFilterName )
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
    if ( !xAccess->hasByName(sStreamName) || !xStorage->isStreamElement(sStreamName) )
    {
        // stream name not found! Then try the compatibility name.
        // do we even have an alternative name?
        if ( pCompatibilityStreamName )
            sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
    }

    // get input stream
    try
    {
        uno::Reference < io::XStream > xEventsStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );

        // determine if stream is encrypted or not
        uno::Reference < beans::XPropertySet > xProps( xEventsStream, uno::UNO_QUERY );
        Any aAny = xProps->getPropertyValue( "Encrypted" );
        bool bEncrypted = false;
        if ( aAny.getValueType() == cppu::UnoType<bool>::get() )
            aAny >>= bEncrypted;

        // set Base URL
        if ( rPropSet.is() )
        {
            OUString sPropName( "StreamName");
            rPropSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
        }


        Reference < io::XInputStream > xStream = xEventsStream->getInputStream();
        return ReadThroughComponent( xStream, xModelComponent, rxContext, rPropSet, pFilterName, bEncrypted );
    }
    catch ( packages::WrongPasswordException& )
    {
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch( packages::zip::ZipIOException& )
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch ( uno::Exception& )
    {
    }

    return ERRCODE_SFX_DOLOADFAILED;
}


SmXMLImport::SmXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags)
:   SvXMLImport(rContext, implementationName, nImportFlags),
    bSuccess(false)
{
}

namespace
{
    class theSmXMLImportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSmXMLImportUnoTunnelId> {};
}

const uno::Sequence< sal_Int8 > & SmXMLImport::getUnoTunnelId() throw()
{
    return theSmXMLImportUnoTunnelId::get().getSeq();
}

OUString SAL_CALL SmXMLImport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLImporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLImport_getSupportedServiceNames()
        throw()
{
    return uno::Sequence<OUString>{ IMPORT_SVC_NAME };
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new SmXMLImport(comphelper::getComponentContext(rSMgr), SmXMLImport_getImplementationName(), SvXMLImportFlags::ALL));
}


OUString SAL_CALL SmXMLImportMeta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisMetaImporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLImportMeta_getSupportedServiceNames()
throw()
{
    return uno::Sequence<OUString>{ IMPORT_SVC_NAME };
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImportMeta_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new SmXMLImport( comphelper::getComponentContext(rSMgr), SmXMLImportMeta_getImplementationName(), SvXMLImportFlags::META ));
}


OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisSettingsImporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLImportSettings_getSupportedServiceNames()
        throw()
{
    return uno::Sequence<OUString>{ IMPORT_SVC_NAME };
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImportSettings_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new SmXMLImport( comphelper::getComponentContext(rSMgr), SmXMLImportSettings_getImplementationName(), SvXMLImportFlags::SETTINGS ));
}

sal_Int64 SAL_CALL SmXMLImport::getSomething(
    const uno::Sequence< sal_Int8 >&rId )
{
    if ( rId.getLength() == 16 &&
        0 == memcmp( getUnoTunnelId().getConstArray(),
        rId.getConstArray(), 16 ) )
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_uIntPtr >(this));

    return SvXMLImport::getSomething( rId );
}

void SmXMLImport::endDocument()
{
    //Set the resulted tree into the SmDocShell where it belongs
    SmNode *pTree = popOrZero(aNodeStack);
    if (pTree && pTree->GetType() == NTABLE)
    {
        uno::Reference <frame::XModel> xModel = GetModel();
        uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
        SmModel *pModel = reinterpret_cast<SmModel *>
            (xTunnel->getSomething(SmModel::getUnoTunnelId()));

        if (pModel)
        {
            SmDocShell *pDocShell =
                static_cast<SmDocShell*>(pModel->GetObjectShell());
            pDocShell->SetFormulaTree(static_cast<SmTableNode *>(pTree));
            if (aText.isEmpty())  //If we picked up no annotation text
            {
                // Get text from imported formula
                pTree->CreateTextFromNode(aText);
                aText = comphelper::string::stripEnd(aText, ' ');
            }

            // Convert symbol names
            SmParser &rParser = pDocShell->GetParser();
            bool bVal = rParser.IsImportSymbolNames();
            rParser.SetImportSymbolNames( true );
            SmNode *pTmpTree = rParser.Parse( aText );
            aText = rParser.GetText();
            delete pTmpTree;
            rParser.SetImportSymbolNames( bVal );

            pDocShell->SetText( aText );
        }
        OSL_ENSURE(pModel,"So there *was* a uno problem after all");

        bSuccess = true;
    }

    SvXMLImport::endDocument();
}


class SmXMLImportContext: public SvXMLImportContext
{
public:
    SmXMLImportContext( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SvXMLImportContext(rImport, nPrfx, rLName) {}

    SmXMLImport& GetSmImport()
    {
        return static_cast<SmXMLImport&>(GetImport());
    }

    virtual void TCharacters(const OUString & /*rChars*/);
    virtual void Characters(const OUString &rChars) override;
    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/, const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/) override;
};

void SmXMLImportContext::TCharacters(const OUString & /*rChars*/)
{
}

void SmXMLImportContext::Characters(const OUString &rChars)
{
    /*
    Whitespace occurring within the content of token elements is "trimmed"
    from the ends (i.e. all whitespace at the beginning and end of the
    content is removed), and "collapsed" internally (i.e. each sequence of
    1 or more whitespace characters is replaced with one blank character).
    */
    //collapsing not done yet!
    const OUString &rChars2 = rChars.trim();
    if (!rChars2.isEmpty())
        TCharacters(rChars2/*.collapse()*/);
}

SvXMLImportContext * SmXMLImportContext::CreateChildContext(sal_uInt16 /*nPrefix*/,
    const OUString& /*rLocalName*/,
    const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/)
{
    return nullptr;
}


struct SmXMLContext_Helper
{
    sal_Int8 nIsBold;
    sal_Int8 nIsItalic;
    double nFontSize;
    OUString sFontFamily;
    OUString sColor;

    SmXMLImportContext & rContext;

    explicit SmXMLContext_Helper(SmXMLImportContext &rImport)
        : nIsBold( -1 )
        , nIsItalic( -1 )
        , nFontSize( 0.0 )
        , rContext( rImport )
        {}

    bool IsFontNodeNeeded() const;
    void RetrieveAttrs(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
    void ApplyAttrs();
};

bool SmXMLContext_Helper::IsFontNodeNeeded() const
{
    return nIsBold != -1 ||
        nIsItalic != -1 ||
        nFontSize != 0.0 ||
        !sFontFamily.isEmpty() ||
        !sColor.isEmpty();
}

void SmXMLContext_Helper::RetrieveAttrs(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    bool bMvFound = false;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = rContext.GetSmImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            rContext.GetSmImport().GetPresLayoutAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_FONTWEIGHT:
                nIsBold = sal_Int8(sValue.equals(GetXMLToken(XML_BOLD)));
                break;
            case XML_TOK_FONTSTYLE:
                nIsItalic = sal_Int8(sValue.equals(GetXMLToken(XML_ITALIC)));
                break;
            case XML_TOK_FONTSIZE:
                ::sax::Converter::convertDouble(nFontSize, sValue);
                rContext.GetSmImport().GetMM100UnitConverter().
                    SetXMLMeasureUnit(util::MeasureUnit::POINT);
                if (-1 == sValue.indexOf(GetXMLToken(XML_UNIT_PT)))
                {
                    if (-1 == sValue.indexOf('%'))
                        nFontSize=0.0;
                    else
                    {
                        rContext.GetSmImport().GetMM100UnitConverter().
                            SetXMLMeasureUnit(util::MeasureUnit::PERCENT);
                    }
                }
                break;
            case XML_TOK_FONTFAMILY:
                sFontFamily = sValue;
                break;
            case XML_TOK_COLOR:
                sColor = sValue;
                break;
            case XML_TOK_MATHCOLOR:
                sColor = sValue;
                break;
            case XML_TOK_MATHVARIANT:
                bMvFound = true;
                break;
            default:
                break;
        }
    }

    if (bMvFound)
    {
        // Ignore deprecated attributes fontfamily, fontweight, and fontstyle
        // in favor of mathvariant, as specified in
        // <https://www.w3.org/TR/MathML3/chapter3.html#presm.deprecatt>.
        sFontFamily.clear();
        nIsBold = -1;
        nIsItalic = -1;
    }
}

void SmXMLContext_Helper::ApplyAttrs()
{
    SmNodeStack &rNodeStack = rContext.GetSmImport().GetNodeStack();

    if (IsFontNodeNeeded())
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;

        if (nIsBold != -1)
        {
            if (nIsBold)
                aToken.eType = TBOLD;
            else
                aToken.eType = TNBOLD;
            std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
            pFontNode->SetSubNodes(nullptr,popOrZero(rNodeStack));
            rNodeStack.push_front(std::move(pFontNode));
        }
        if (nIsItalic != -1)
        {
            if (nIsItalic)
                aToken.eType = TITALIC;
            else
                aToken.eType = TNITALIC;
            std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
            pFontNode->SetSubNodes(nullptr,popOrZero(rNodeStack));
            rNodeStack.push_front(std::move(pFontNode));
        }
        if (nFontSize != 0.0)
        {
            aToken.eType = TSIZE;
            std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));

            if (util::MeasureUnit::PERCENT == rContext.GetSmImport()
                    .GetMM100UnitConverter().GetXMLMeasureUnit())
            {
                if (nFontSize < 100.00)
                    pFontNode->SetSizeParameter(Fraction(100.00/nFontSize),
                        FontSizeType::DIVIDE);
                else
                    pFontNode->SetSizeParameter(Fraction(nFontSize/100.00),
                        FontSizeType::MULTIPLY);
            }
            else
                pFontNode->SetSizeParameter(Fraction(nFontSize),FontSizeType::ABSOLUT);

            pFontNode->SetSubNodes(nullptr,popOrZero(rNodeStack));
            rNodeStack.push_front(std::move(pFontNode));
        }
        if (!sFontFamily.isEmpty())
        {
            if (sFontFamily.equalsIgnoreAsciiCase(GetXMLToken(XML_FIXED)))
                aToken.eType = TFIXED;
            else if (sFontFamily.equalsIgnoreAsciiCase("sans"))
                aToken.eType = TSANS;
            else if (sFontFamily.equalsIgnoreAsciiCase("serif"))
                aToken.eType = TSERIF;
            else //Just give up, we need to extend our font mechanism to be
                //more general
                return;

            aToken.aText = sFontFamily;
            std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
            pFontNode->SetSubNodes(nullptr,popOrZero(rNodeStack));
            rNodeStack.push_front(std::move(pFontNode));
        }
        if (!sColor.isEmpty())
        {
            //Again we can only handle a small set of colours in
            //StarMath for now.
            const SvXMLTokenMap& rTokenMap =
                rContext.GetSmImport().GetColorTokenMap();
            sal_uInt16 tok = rTokenMap.Get(XML_NAMESPACE_MATH, sColor);
            if (tok != XML_TOK_UNKNOWN)
            {
                aToken.eType = static_cast<SmTokenType>(tok);
                std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
                pFontNode->SetSubNodes(nullptr,popOrZero(rNodeStack));
                rNodeStack.push_front(std::move(pFontNode));
            }
        }

    }
}


class SmXMLTokenAttrHelper
{
    SmXMLImportContext& mrContext;
    MathMLMathvariantValue meMv;
    bool mbMvFound;

public:
    SmXMLTokenAttrHelper(SmXMLImportContext& rContext)
        : mrContext(rContext)
        , meMv(MathMLMathvariantValue::Normal)
        , mbMvFound(false)
    {}

    void RetrieveAttrs(const uno::Reference<xml::sax::XAttributeList>& xAttrList);
    void ApplyAttrs(MathMLMathvariantValue eDefaultMv);
};

void SmXMLTokenAttrHelper::RetrieveAttrs(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if (!xAttrList.is())
        return;
    sal_Int16 nAttrCount = xAttrList->getLength();
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = mrContext.GetSmImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            mrContext.GetSmImport().GetPresLayoutAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_MATHVARIANT:
                if (!GetMathMLMathvariantValue(sValue, &meMv))
                    SAL_WARN("starmath", "failed to recognize mathvariant: " << sValue);
                mbMvFound = true;
                break;
            default:
                break;
        }
    }
}

void SmXMLTokenAttrHelper::ApplyAttrs(MathMLMathvariantValue eDefaultMv)
{
    assert( eDefaultMv == MathMLMathvariantValue::Normal ||
            eDefaultMv == MathMLMathvariantValue::Italic );

    std::vector<SmTokenType> vVariant;
    MathMLMathvariantValue eMv = mbMvFound ? meMv : eDefaultMv;
    switch(eMv)
    {
        case MathMLMathvariantValue::Normal:
            vVariant.push_back(TNITALIC);
            break;
        case MathMLMathvariantValue::Bold:
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Italic:
            // nothing to do
            break;
        case MathMLMathvariantValue::BoldItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::DoubleStruck:
            // TODO
            break;
        case MathMLMathvariantValue::BoldFraktur:
            // TODO: Fraktur
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Script:
            // TODO
            break;
        case MathMLMathvariantValue::BoldScript:
            // TODO: Script
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Fraktur:
            // TODO
            break;
        case MathMLMathvariantValue::SansSerif:
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::BoldSansSerif:
            vVariant.push_back(TSANS);
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::SansSerifItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::SansSerifBoldItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TBOLD);
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::Monospace:
            vVariant.push_back(TFIXED);
            break;
        case MathMLMathvariantValue::Initial:
        case MathMLMathvariantValue::Tailed:
        case MathMLMathvariantValue::Looped:
        case MathMLMathvariantValue::Stretched:
            // TODO
            break;
    }
    if (vVariant.empty())
        return;
    SmNodeStack &rNodeStack = mrContext.GetSmImport().GetNodeStack();
    for (auto eType : vVariant)
    {
        SmToken aToken;
        aToken.eType = eType;
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
}


class SmXMLDocContext_Impl : public SmXMLImportContext
{
public:
    SmXMLDocContext_Impl( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrfx,rLName) {}

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;

    void EndElement() override;
};


/*avert thy gaze from the proginator*/
class SmXMLRowContext_Impl : public SmXMLDocContext_Impl
{
protected:
    size_t nElementCount;

public:
    SmXMLRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLDocContext_Impl(rImport,nPrefix,rLName)
        { nElementCount = GetSmImport().GetNodeStack().size(); }

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;

    SvXMLImportContext *StrictCreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);

    void EndElement() override;
};


class SmXMLEncloseContext_Impl : public SmXMLRowContext_Impl
{
public:
    // TODO/LATER: convert <menclose notation="horizontalstrike"> into
    // "overstrike{}" and extend the Math syntax to support more notations
    SmXMLEncloseContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};

void SmXMLEncloseContext_Impl::EndElement()
{
    /*
    <menclose> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::EndElement();
}


class SmXMLFracContext_Impl : public SmXMLRowContext_Impl
{
public:
    // TODO/LATER: convert <mfrac bevelled="true"> into "wideslash{}{}"
    SmXMLFracContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};


class SmXMLSqrtContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSqrtContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};


class SmXMLRootContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLRootContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};


class SmXMLStyleContext_Impl : public SmXMLRowContext_Impl
{
protected:
    SmXMLContext_Helper aStyleHelper;

public:
    /*Right now the style tag is completely ignored*/
    SmXMLStyleContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) : SmXMLRowContext_Impl(rImport,nPrefix,rLName),
        aStyleHelper(*this) {}

    void EndElement() override;
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList ) override;
};

void SmXMLStyleContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    aStyleHelper.RetrieveAttrs(xAttrList);
}


void SmXMLStyleContext_Impl::EndElement()
{
    /*
    <mstyle> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    if (rNodeStack.size() - nElementCount != 1)
        SmXMLRowContext_Impl::EndElement();
    aStyleHelper.ApplyAttrs();
}


class SmXMLPaddedContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPaddedContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};

void SmXMLPaddedContext_Impl::EndElement()
{
    /*
    <mpadded> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::EndElement();
}


class SmXMLPhantomContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPhantomContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};

void SmXMLPhantomContext_Impl::EndElement()
{
    /*
    <mphantom> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nLevel = 5;
    aToken.eType = TPHANTOM;

    std::unique_ptr<SmFontNode> pPhantom(new SmFontNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pPhantom->SetSubNodes(nullptr,popOrZero(rNodeStack));
    rNodeStack.push_front(std::move(pPhantom));
}


class SmXMLFencedContext_Impl : public SmXMLRowContext_Impl
{
protected:
    sal_Unicode cBegin;
    sal_Unicode cEnd;

public:
    SmXMLFencedContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName),
        cBegin('('), cEnd(')') {}

    void StartElement(const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override;
    void EndElement() override;
};


void SmXMLFencedContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetFencedAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            //temp, starmath cannot handle multichar brackets (I think)
            case XML_TOK_OPEN:
                cBegin = sValue[0];
                break;
            case XML_TOK_CLOSE:
                cEnd = sValue[0];
                break;
            default:
                /*Go to superclass*/
                break;
        }
    }
}


void SmXMLFencedContext_Impl::EndElement()
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.aText = ",";
    aToken.nLevel = 5;

    aToken.eType = TLPARENT;
    aToken.cMathChar = cBegin;
    std::unique_ptr<SmStructureNode> pSNode(new SmBraceNode(aToken));
    SmNode *pLeft = new SmMathSymbolNode(aToken);

    aToken.cMathChar = cEnd;
    aToken.eType = TRPARENT;
    SmNode *pRight = new SmMathSymbolNode(aToken);

    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    aToken.cMathChar = '\0';
    aToken.aText = ",";
    aToken.eType = TIDENT;

    auto i = rNodeStack.size() - nElementCount;
    if (rNodeStack.size() - nElementCount > 1)
        i += rNodeStack.size() - 1 - nElementCount;
    aRelationArray.resize(i);
    while (rNodeStack.size() > nElementCount)
    {
        auto pNode = std::move(rNodeStack.front());
        rNodeStack.pop_front();
        aRelationArray[--i] = pNode.release();
        if (i > 1 && rNodeStack.size() > 1)
            aRelationArray[--i] = new SmGlyphSpecialNode(aToken);
    }

    SmToken aDummy;
    SmStructureNode *pBody = new SmExpressionNode(aDummy);
    pBody->SetSubNodes(aRelationArray);


    pSNode->SetSubNodes(pLeft,pBody,pRight);
    pSNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().push_front(std::move(pSNode));
}


class SmXMLErrorContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLErrorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override;
};

void SmXMLErrorContext_Impl::EndElement()
{
    /*Right now the error tag is completely ignored, what
     can I do with it in starmath, ?, maybe we need a
     report window ourselves, do a test for validity of
     the xml input, use mirrors, and then generate
     the markup inside the merror with a big red colour
     of something. For now just throw them all away.
     */
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    while (rNodeStack.size() > nElementCount)
    {
        rNodeStack.pop_front();
    }
}


class SmXMLNumberContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLNumberContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TNUMBER;
    }

    virtual void TCharacters(const OUString &rChars) override;

    void EndElement() override;
};

void SmXMLNumberContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}

void SmXMLNumberContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push_front(o3tl::make_unique<SmTextNode>(aToken,FNT_NUMBER));
}


class SmXMLAnnotationContext_Impl : public SmXMLImportContext
{
    bool bIsStarMath;

public:
    SmXMLAnnotationContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName), bIsStarMath(false) {}

    virtual void Characters(const OUString &rChars) override;

    void StartElement(const uno::Reference<xml::sax::XAttributeList > & xAttrList ) override;
};

void SmXMLAnnotationContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);

        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetAnnotationAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_ENCODING:
                bIsStarMath= sValue == "StarMath 5.0";
                break;
            default:
                break;
        }
    }
}

void SmXMLAnnotationContext_Impl::Characters(const OUString &rChars)
{
    if (bIsStarMath)
        GetSmImport().SetText( GetSmImport().GetText() + rChars );
}


class SmXMLTextContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLTextContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString &rChars) override;

    void EndElement() override;
};

void SmXMLTextContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}

void SmXMLTextContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push_front(o3tl::make_unique<SmTextNode>(aToken,FNT_TEXT));
}


class SmXMLStringContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLStringContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString &rChars) override;

    void EndElement() override;
};

void SmXMLStringContext_Impl::TCharacters(const OUString &rChars)
{
    /*
    The content of <ms> elements should be rendered with visible "escaping" of
    certain characters in the content, including at least "double quote"
    itself, and preferably whitespace other than individual blanks. The intent
    is for the viewer to see that the expression is a string literal, and to
    see exactly which characters form its content. For example, <ms>double
    quote is "</ms> might be rendered as "double quote is \"".

    Obviously this isn't fully done here.
    */
    aToken.aText = "\"" + rChars + "\"";
}

void SmXMLStringContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push_front(o3tl::make_unique<SmTextNode>(aToken,FNT_FIXED));
}


class SmXMLIdentifierContext_Impl : public SmXMLImportContext
{
    SmXMLTokenAttrHelper maTokenAttrHelper;
    SmXMLContext_Helper aStyleHelper;
    SmToken aToken;

public:
    SmXMLIdentifierContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName)
        , maTokenAttrHelper(*this)
        , aStyleHelper(*this)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TIDENT;
    }

    void TCharacters(const OUString &rChars) override;
    void StartElement(const uno::Reference< xml::sax::XAttributeList > & xAttrList ) override
    {
        maTokenAttrHelper.RetrieveAttrs(xAttrList);
        aStyleHelper.RetrieveAttrs(xAttrList);
    };
    void EndElement() override;
};

void SmXMLIdentifierContext_Impl::EndElement()
{
    std::unique_ptr<SmTextNode> pNode;
    //we will handle identifier italic/normal here instead of with a standalone
    //font node
    if (((aStyleHelper.nIsItalic == -1) && (aToken.aText.getLength() > 1))
        || ((aStyleHelper.nIsItalic == 0) && (aToken.aText.getLength() == 1)))
    {
        pNode.reset(new SmTextNode(aToken,FNT_FUNCTION));
        pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    else
        pNode.reset(new SmTextNode(aToken,FNT_VARIABLE));
    if (aStyleHelper.nIsItalic != -1)
    {
        if (aStyleHelper.nIsItalic)
            pNode->GetFont().SetItalic(ITALIC_NORMAL);
        else
            pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    GetSmImport().GetNodeStack().push_front(std::move(pNode));
    aStyleHelper.ApplyAttrs();

    maTokenAttrHelper.ApplyAttrs( (aToken.aText.getLength() == 1)
                                  ? MathMLMathvariantValue::Italic
                                  : MathMLMathvariantValue::Normal );
}

void SmXMLIdentifierContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}


class SmXMLOperatorContext_Impl : public SmXMLImportContext
{
    SmXMLTokenAttrHelper maTokenAttrHelper;
    bool bIsStretchy;
    SmToken aToken;

public:
    SmXMLOperatorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName)
        , maTokenAttrHelper(*this)
        , bIsStretchy(false)
    {
        aToken.eType = TSPECIAL;
        aToken.nLevel = 5;
    }

    void TCharacters(const OUString &rChars) override;
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList ) override;
    void EndElement() override;
};

void SmXMLOperatorContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.cMathChar = rChars[0];
}

void SmXMLOperatorContext_Impl::EndElement()
{
    std::unique_ptr<SmMathSymbolNode> pNode(new SmMathSymbolNode(aToken));
    //For stretchy scaling the scaling must be retrieved from this node
    //and applied to the expression itself so as to get the expression
    //to scale the operator to the height of the expression itself
    if (bIsStretchy)
        pNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().push_front(std::move(pNode));

    // TODO: apply to non-alphabetic characters too
    if (rtl::isAsciiAlpha(aToken.cMathChar))
        maTokenAttrHelper.ApplyAttrs(MathMLMathvariantValue::Normal);
}


void SmXMLOperatorContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    maTokenAttrHelper.RetrieveAttrs(xAttrList);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);

        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetOperatorAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_STRETCHY:
                bIsStretchy = sValue.equals(
                    GetXMLToken(XML_TRUE));
                break;
            default:
                break;
        }
    }
}


class SmXMLSpaceContext_Impl : public SmXMLImportContext
{
public:
    SmXMLSpaceContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}

    void StartElement(const uno::Reference< xml::sax::XAttributeList >& xAttrList ) override;
};

namespace {

bool lcl_CountBlanks(const MathMLAttributeLengthValue &rLV,
                     sal_Int32 *pWide, sal_Int32 *pNarrow)
{
    assert(pWide);
    assert(pNarrow);
    if (rLV.aNumber.GetNumerator() == 0)
    {
        *pWide = *pNarrow = 0;
        return true;
    }
    // TODO: honor other units than em
    if (rLV.eUnit != MathMLLengthUnit::Em)
        return false;
    if (rLV.aNumber.GetNumerator() < 0)
        return false;
    const Fraction aTwo(2, 1);
    auto aWide = rLV.aNumber / aTwo;
    auto nWide = static_cast<sal_Int32>(static_cast<long>(aWide));
    if (nWide < 0)
        return false;
    const Fraction aPointFive(1, 2);
    auto aNarrow = (rLV.aNumber - Fraction(nWide, 1) * aTwo) / aPointFive;
    auto nNarrow = static_cast<sal_Int32>(static_cast<long>(aNarrow));
    if (nNarrow < 0)
        return false;
    *pWide = nWide;
    *pNarrow = nNarrow;
    return true;
}

}

void SmXMLSpaceContext_Impl::StartElement(
    const uno::Reference<xml::sax::XAttributeList > & xAttrList )
{
    // There is no syntax in Math to specify blank nodes of arbitrary size yet.
    MathMLAttributeLengthValue aLV;
    sal_Int32 nWide = 0, nNarrow = 0;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap = GetSmImport().GetMspaceAttrTokenMap();
        switch (rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_WIDTH:
                if ( ParseMathMLAttributeLengthValue(sValue.trim(), &aLV) <= 0 ||
                     !lcl_CountBlanks(aLV, &nWide, &nNarrow) )
                    SAL_WARN("starmath", "ignore mspace's width: " << sValue);
                break;
            default:
                break;
        }
    }
    SmToken aToken;
    aToken.eType = TBLANK;
    aToken.cMathChar = '\0';
    aToken.nGroup = TG::Blank;
    aToken.nLevel = 5;
    std::unique_ptr<SmBlankNode> pBlank(new SmBlankNode(aToken));
    for (sal_Int32 i = 0; i < nWide; i++)
        pBlank->IncreaseBy(aToken);
    if (nNarrow > 0)
    {
        aToken.eType = TSBLANK;
        for (sal_Int32 i = 0; i < nNarrow; i++)
            pBlank->IncreaseBy(aToken);
    }
    GetSmImport().GetNodeStack().push_front(std::move(pBlank));
}


class SmXMLSubContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType,SmSubSup aSubSup);

public:
    SmXMLSubContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override
    {
        GenericEndElement(TRSUB,RSUB);
    }
};


void SmXMLSubContext_Impl::GenericEndElement(SmTokenType eType, SmSubSup eSubSup)
{
    /*The <msub> element requires exactly 2 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE( bNodeCheck, "Sub has not two arguments" );
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = eType;
    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (size_t i = 1;  i < aSubNodes.size();  i++)
        aSubNodes[i] = nullptr;

    aSubNodes[eSubSup+1] = popOrZero(rNodeStack);
    aSubNodes[0] = popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.push_front(std::move(pNode));
}


class SmXMLSupContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override
    {
        GenericEndElement(TRSUP,RSUP);
    }
};


class SmXMLSubSupContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType, SmSubSup aSub,SmSubSup aSup);

public:
    SmXMLSubSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override
    {
        GenericEndElement(TRSUB,RSUB,RSUP);
    }
};

void SmXMLSubSupContext_Impl::GenericEndElement(SmTokenType eType,
        SmSubSup aSub,SmSubSup aSup)
{
    /*The <msub> element requires exactly 3 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 3;
    OSL_ENSURE( bNodeCheck, "SubSup has not three arguments" );
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = eType;
    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (size_t i = 1;  i < aSubNodes.size();  i++)
        aSubNodes[i] = nullptr;

    aSubNodes[aSup+1] = popOrZero(rNodeStack);
    aSubNodes[aSub+1] = popOrZero(rNodeStack);
    aSubNodes[0] =  popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.push_front(std::move(pNode));
}


class SmXMLUnderContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLUnderContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
                           const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName)
        , nAttrCount( 0 )
        {}

    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList ) override;
    void EndElement() override;
    void HandleAccent();
};

void SmXMLUnderContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
}

void SmXMLUnderContext_Impl::HandleAccent()
{
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE( bNodeCheck, "Sub has not two arguments" );
    if (!bNodeCheck)
        return;

    /*Just one special case for the underline thing*/
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNode *pTest = popOrZero(rNodeStack);
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TUNDERLINE;

    SmNode *pFirst;
    std::unique_ptr<SmStructureNode> pNode(new SmAttributNode(aToken));
    if ((pTest->GetToken().cMathChar & 0x0FFF) == 0x0332)
    {
        pFirst = new SmRectangleNode(aToken);
        delete pTest;
    }
    else
        pFirst = pTest;

    SmNode *pSecond = popOrZero(rNodeStack);
    pNode->SetSubNodes(pFirst, pSecond);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.push_front(std::move(pNode));
}


void SmXMLUnderContext_Impl::EndElement()
{
    if (!nAttrCount)
        GenericEndElement(TCSUB,CSUB);
    else
        HandleAccent();
}


class SmXMLOverContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName), nAttrCount(0) {}

    void EndElement() override;
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList ) override;
    void HandleAccent();
};


void SmXMLOverContext_Impl::StartElement(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
}


void SmXMLOverContext_Impl::EndElement()
{
    if (!nAttrCount)
        GenericEndElement(TCSUP,CSUP);
    else
        HandleAccent();
}


void SmXMLOverContext_Impl::HandleAccent()
{
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE (bNodeCheck, "Sub has not two arguments");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TACUTE;

    std::unique_ptr<SmAttributNode> pNode(new SmAttributNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    SmNode *pFirst = popOrZero(rNodeStack);
    SmNode *pSecond = popOrZero(rNodeStack);
    pNode->SetSubNodes(pFirst, pSecond);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.push_front(std::move(pNode));

}


class SmXMLUnderOverContext_Impl : public SmXMLSubSupContext_Impl
{
public:
    SmXMLUnderOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubSupContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement() override
    {
        GenericEndElement(TCSUB,CSUB,CSUP);
    }
};


class SmXMLMultiScriptsContext_Impl : public SmXMLSubSupContext_Impl
{
    bool bHasPrescripts;

    void ProcessSubSupPairs(bool bIsPrescript);

public:
    SmXMLMultiScriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLSubSupContext_Impl(rImport,nPrefix,rLName),
        bHasPrescripts(false) {}

    void EndElement() override;
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;
};


class SmXMLNoneContext_Impl : public SmXMLImportContext
{
public:
    SmXMLNoneContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}

    void EndElement() override;
};


void SmXMLNoneContext_Impl::EndElement()
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.aText.clear();
    aToken.nLevel = 5;
    aToken.eType = TIDENT;
    GetSmImport().GetNodeStack().push_front(
        o3tl::make_unique<SmTextNode>(aToken,FNT_VARIABLE));
}


class SmXMLPrescriptsContext_Impl : public SmXMLImportContext
{
public:
    SmXMLPrescriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
};


class SmXMLTableRowContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}

    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;
};


class SmXMLTableContext_Impl : public SmXMLTableRowContext_Impl
{
public:
    SmXMLTableContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLTableRowContext_Impl(rImport,nPrefix,rLName)
        {}

    void EndElement() override;
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;
};


class SmXMLTableCellContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableCellContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
};


class SmXMLAlignGroupContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLAlignGroupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}

    /*Don't do anything with alignment for now*/
    void EndElement() override
    {
    }
};


class SmXMLActionContext_Impl : public SmXMLRowContext_Impl
{
    size_t mnSelection; // 1-based

public:
    SmXMLActionContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
      , mnSelection(1)
        {}

    void StartElement(const uno::Reference<xml::sax::XAttributeList> &xAttrList) override;
    void EndElement() override;
};


// NB: virtually inherit so we can multiply inherit properly
//     in SmXMLFlatDocContext_Impl
class SmXMLOfficeContext_Impl : public virtual SvXMLImportContext
{
public:
    SmXMLOfficeContext_Impl( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SvXMLImportContext(rImport,nPrfx,rLName) {}

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList) override;
};

SvXMLImportContext *SmXMLOfficeContext_Impl::CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList)
{
    SvXMLImportContext *pContext = nullptr;
    if ( XML_NAMESPACE_OFFICE == nPrefix &&
        rLocalName == GetXMLToken(XML_META) )
    {
        SAL_WARN("starmath", "XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
    }
    else if ( XML_NAMESPACE_OFFICE == nPrefix &&
        rLocalName == GetXMLToken(XML_SETTINGS) )
    {
        pContext = new XMLDocumentSettingsContext( GetImport(),
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    xAttrList );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


// context for flat file xml format
class SmXMLFlatDocContext_Impl
    : public SmXMLOfficeContext_Impl, public SvXMLMetaDocumentContext
{
public:
    SmXMLFlatDocContext_Impl( SmXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 i_nPrefix, const OUString& i_rLocalName, const uno::Reference<xml::sax::XAttributeList>& i_xAttrList) override;
};

SmXMLFlatDocContext_Impl::SmXMLFlatDocContext_Impl( SmXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
    SvXMLImportContext(i_rImport, i_nPrefix, i_rLName),
    SmXMLOfficeContext_Impl(i_rImport, i_nPrefix, i_rLName),
    SvXMLMetaDocumentContext(i_rImport, i_nPrefix, i_rLName,
        i_xDocProps)
{
}

SvXMLImportContext *SmXMLFlatDocContext_Impl::CreateChildContext(
    sal_uInt16 i_nPrefix, const OUString& i_rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& i_xAttrList)
{
    // behave like meta base class iff we encounter office:meta
    if ( XML_NAMESPACE_OFFICE == i_nPrefix &&
            i_rLocalName == GetXMLToken(XML_META) )
    {
        return SvXMLMetaDocumentContext::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    }
    else
    {
        return SmXMLOfficeContext_Impl::CreateChildContext(
                    i_nPrefix, i_rLocalName, i_xAttrList );
    }
}


static const SvXMLTokenMapEntry aPresLayoutElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_SEMANTICS, XML_TOK_SEMANTICS },
    { XML_NAMESPACE_MATH,   XML_MATH,      XML_TOK_MATH   },
    { XML_NAMESPACE_MATH,   XML_MSTYLE,    XML_TOK_MSTYLE  },
    { XML_NAMESPACE_MATH,   XML_MERROR,    XML_TOK_MERROR },
    { XML_NAMESPACE_MATH,   XML_MPHANTOM,  XML_TOK_MPHANTOM },
    { XML_NAMESPACE_MATH,   XML_MROW,      XML_TOK_MROW },
    { XML_NAMESPACE_MATH,   XML_MENCLOSE,  XML_TOK_MENCLOSE },
    { XML_NAMESPACE_MATH,   XML_MFRAC,     XML_TOK_MFRAC },
    { XML_NAMESPACE_MATH,   XML_MSQRT,     XML_TOK_MSQRT },
    { XML_NAMESPACE_MATH,   XML_MROOT,     XML_TOK_MROOT },
    { XML_NAMESPACE_MATH,   XML_MSUB,      XML_TOK_MSUB },
    { XML_NAMESPACE_MATH,   XML_MSUP,      XML_TOK_MSUP },
    { XML_NAMESPACE_MATH,   XML_MSUBSUP,   XML_TOK_MSUBSUP },
    { XML_NAMESPACE_MATH,   XML_MUNDER,    XML_TOK_MUNDER },
    { XML_NAMESPACE_MATH,   XML_MOVER,     XML_TOK_MOVER },
    { XML_NAMESPACE_MATH,   XML_MUNDEROVER,    XML_TOK_MUNDEROVER },
    { XML_NAMESPACE_MATH,   XML_MMULTISCRIPTS, XML_TOK_MMULTISCRIPTS },
    { XML_NAMESPACE_MATH,   XML_MTABLE,    XML_TOK_MTABLE },
    { XML_NAMESPACE_MATH,   XML_MACTION,   XML_TOK_MACTION },
    { XML_NAMESPACE_MATH,   XML_MFENCED,   XML_TOK_MFENCED },
    { XML_NAMESPACE_MATH,   XML_MPADDED,   XML_TOK_MPADDED },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aPresLayoutAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_FONTWEIGHT,      XML_TOK_FONTWEIGHT    },
    { XML_NAMESPACE_MATH,   XML_FONTSTYLE,       XML_TOK_FONTSTYLE     },
    { XML_NAMESPACE_MATH,   XML_FONTSIZE,        XML_TOK_FONTSIZE      },
    { XML_NAMESPACE_MATH,   XML_FONTFAMILY,      XML_TOK_FONTFAMILY    },
    { XML_NAMESPACE_MATH,   XML_COLOR,           XML_TOK_COLOR },
    { XML_NAMESPACE_MATH,   XML_MATHCOLOR,       XML_TOK_MATHCOLOR },
    { XML_NAMESPACE_MATH,   XML_MATHVARIANT,     XML_TOK_MATHVARIANT },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aFencedAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_OPEN,       XML_TOK_OPEN },
    { XML_NAMESPACE_MATH,   XML_CLOSE,      XML_TOK_CLOSE },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aOperatorAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_STRETCHY,      XML_TOK_STRETCHY },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aAnnotationAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_ENCODING,      XML_TOK_ENCODING },
    XML_TOKEN_MAP_END
};


static const SvXMLTokenMapEntry aPresElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_ANNOTATION,    XML_TOK_ANNOTATION },
    { XML_NAMESPACE_MATH,   XML_MI,    XML_TOK_MI },
    { XML_NAMESPACE_MATH,   XML_MN,    XML_TOK_MN },
    { XML_NAMESPACE_MATH,   XML_MO,    XML_TOK_MO },
    { XML_NAMESPACE_MATH,   XML_MTEXT, XML_TOK_MTEXT },
    { XML_NAMESPACE_MATH,   XML_MSPACE,XML_TOK_MSPACE },
    { XML_NAMESPACE_MATH,   XML_MS,    XML_TOK_MS },
    { XML_NAMESPACE_MATH,   XML_MALIGNGROUP,   XML_TOK_MALIGNGROUP },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aPresScriptEmptyElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_MPRESCRIPTS,   XML_TOK_MPRESCRIPTS },
    { XML_NAMESPACE_MATH,   XML_NONE,  XML_TOK_NONE },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aPresTableElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_MTR,       XML_TOK_MTR },
    { XML_NAMESPACE_MATH,   XML_MTD,       XML_TOK_MTD },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aColorTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_BLACK,        TBLACK},
    { XML_NAMESPACE_MATH,   XML_WHITE,        TWHITE},
    { XML_NAMESPACE_MATH,   XML_RED,          TRED},
    { XML_NAMESPACE_MATH,   XML_GREEN,        TGREEN},
    { XML_NAMESPACE_MATH,   XML_BLUE,         TBLUE},
    { XML_NAMESPACE_MATH,   XML_AQUA,         TAQUA},
    { XML_NAMESPACE_MATH,   XML_FUCHSIA,      TFUCHSIA},
    { XML_NAMESPACE_MATH,   XML_YELLOW,       TYELLOW},
    { XML_NAMESPACE_MATH,   XML_NAVY,         TNAVY},
    { XML_NAMESPACE_MATH,   XML_TEAL,         TTEAL},
    { XML_NAMESPACE_MATH,   XML_MAROON,       TMAROON},
    { XML_NAMESPACE_MATH,   XML_PURPLE,       TPURPLE},
    { XML_NAMESPACE_MATH,   XML_OLIVE,        TOLIVE},
    { XML_NAMESPACE_MATH,   XML_GRAY,         TGRAY},
    { XML_NAMESPACE_MATH,   XML_SILVER,       TSILVER},
    { XML_NAMESPACE_MATH,   XML_LIME,         TLIME},
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aActionAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_SELECTION,      XML_TOK_SELECTION },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aMspaceAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_WIDTH,      XML_TOK_WIDTH },
    XML_TOKEN_MAP_END
};


const SvXMLTokenMap& SmXMLImport::GetPresLayoutElemTokenMap()
{
    if (!pPresLayoutElemTokenMap)
        pPresLayoutElemTokenMap.reset(new SvXMLTokenMap(aPresLayoutElemTokenMap));
    return *pPresLayoutElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresLayoutAttrTokenMap()
{
    if (!pPresLayoutAttrTokenMap)
        pPresLayoutAttrTokenMap.reset(new SvXMLTokenMap(aPresLayoutAttrTokenMap));
    return *pPresLayoutAttrTokenMap;
}


const SvXMLTokenMap& SmXMLImport::GetFencedAttrTokenMap()
{
    if (!pFencedAttrTokenMap)
        pFencedAttrTokenMap.reset(new SvXMLTokenMap(aFencedAttrTokenMap));
    return *pFencedAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetOperatorAttrTokenMap()
{
    if (!pOperatorAttrTokenMap)
        pOperatorAttrTokenMap.reset(new SvXMLTokenMap(aOperatorAttrTokenMap));
    return *pOperatorAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetAnnotationAttrTokenMap()
{
    if (!pAnnotationAttrTokenMap)
        pAnnotationAttrTokenMap.reset(new SvXMLTokenMap(aAnnotationAttrTokenMap));
    return *pAnnotationAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresElemTokenMap()
{
    if (!pPresElemTokenMap)
        pPresElemTokenMap.reset(new SvXMLTokenMap(aPresElemTokenMap));
    return *pPresElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresScriptEmptyElemTokenMap()
{
    if (!pPresScriptEmptyElemTokenMap)
        pPresScriptEmptyElemTokenMap.reset(new
            SvXMLTokenMap(aPresScriptEmptyElemTokenMap));
    return *pPresScriptEmptyElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresTableElemTokenMap()
{
    if (!pPresTableElemTokenMap)
        pPresTableElemTokenMap.reset(new SvXMLTokenMap(aPresTableElemTokenMap));
    return *pPresTableElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetColorTokenMap()
{
    if (!pColorTokenMap)
        pColorTokenMap.reset(new SvXMLTokenMap(aColorTokenMap));
    return *pColorTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetActionAttrTokenMap()
{
    if (!pActionAttrTokenMap)
        pActionAttrTokenMap.reset(new SvXMLTokenMap(aActionAttrTokenMap));
    return *pActionAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetMspaceAttrTokenMap()
{
    if (!pMspaceAttrTokenMap)
        pMspaceAttrTokenMap.reset(new SvXMLTokenMap(aMspaceAttrTokenMap));
    return *pMspaceAttrTokenMap;
}


SvXMLImportContext *SmXMLDocContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSmImport().GetPresLayoutElemTokenMap();

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        //Consider semantics a dummy except for any starmath annotations
        case XML_TOK_SEMANTICS:
            pContext = GetSmImport().CreateRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        /*General Layout Schemata*/
        case XML_TOK_MROW:
            pContext = GetSmImport().CreateRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MENCLOSE:
            pContext = GetSmImport().CreateEncloseContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MFRAC:
            pContext = GetSmImport().CreateFracContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSQRT:
            pContext = GetSmImport().CreateSqrtContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MROOT:
            pContext = GetSmImport().CreateRootContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSTYLE:
            pContext = GetSmImport().CreateStyleContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MERROR:
            pContext = GetSmImport().CreateErrorContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MPADDED:
            pContext = GetSmImport().CreatePaddedContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MPHANTOM:
            pContext = GetSmImport().CreatePhantomContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MFENCED:
            pContext = GetSmImport().CreateFencedContext(nPrefix,rLocalName,
                xAttrList);
            break;
        /*Script and Limit Schemata*/
        case XML_TOK_MSUB:
            pContext = GetSmImport().CreateSubContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSUP:
            pContext = GetSmImport().CreateSupContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSUBSUP:
            pContext = GetSmImport().CreateSubSupContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MUNDER:
            pContext = GetSmImport().CreateUnderContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MOVER:
            pContext = GetSmImport().CreateOverContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MUNDEROVER:
            pContext = GetSmImport().CreateUnderOverContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MMULTISCRIPTS:
            pContext = GetSmImport().CreateMultiScriptsContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_MTABLE:
            pContext = GetSmImport().CreateTableContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_MACTION:
            pContext = GetSmImport().CreateActionContext(nPrefix,
                rLocalName, xAttrList);
            break;
        default:
            /*Basically theres an implicit mrow around certain bare
             *elements, use a RowContext to see if this is one of
             *those ones*/
            rtl::Reference<SmXMLRowContext_Impl> aTempContext(new SmXMLRowContext_Impl(GetSmImport(),nPrefix,
                GetXMLToken(XML_MROW)));

            pContext = aTempContext->StrictCreateChildContext(nPrefix,
                rLocalName, xAttrList);
            break;
    }
    return pContext;
}

void SmXMLDocContext_Impl::EndElement()
{
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    SmNode *pContextNode = popOrZero(rNodeStack);

    SmToken aDummy;
    std::unique_ptr<SmStructureNode> pSNode(new SmLineNode(aDummy));
    pSNode->SetSubNodes(pContextNode, nullptr);
    rNodeStack.push_front(std::move(pSNode));

    SmNodeArray  LineArray;
    auto n = rNodeStack.size();
    LineArray.resize(n);
    for (size_t j = 0; j < n; j++)
    {
        auto pNode = std::move(rNodeStack.front());
        rNodeStack.pop_front();
        LineArray[n - (j + 1)] = pNode.release();
    }
    std::unique_ptr<SmStructureNode> pSNode2(new SmTableNode(aDummy));
    pSNode2->SetSubNodes(LineArray);
    rNodeStack.push_front(std::move(pSNode2));
}

void SmXMLFracContext_Impl::EndElement()
{
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    const bool bNodeCheck = rNodeStack.size() - nElementCount == 2;
    OSL_ENSURE( bNodeCheck, "Fraction (mfrac) tag is missing component" );
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TOVER;
    std::unique_ptr<SmStructureNode> pSNode(new SmBinVerNode(aToken));
    SmNode *pOper = new SmRectangleNode(aToken);
    SmNode *pSecond = popOrZero(rNodeStack);
    SmNode *pFirst = popOrZero(rNodeStack);
    pSNode->SetSubNodes(pFirst,pOper,pSecond);
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLRootContext_Impl::EndElement()
{
    /*The <mroot> element requires exactly 2 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE( bNodeCheck, "Root tag is missing component");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = MS_SQRT;  //Temporary: alert, based on StarSymbol font
    aToken.eType = TNROOT;
    std::unique_ptr<SmStructureNode> pSNode(new SmRootNode(aToken));
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNode *pIndex = popOrZero(rNodeStack);
    SmNode *pBase = popOrZero(rNodeStack);
    pSNode->SetSubNodes(pIndex,pOper,pBase);
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLSqrtContext_Impl::EndElement()
{
    /*
    <msqrt> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = MS_SQRT;  //Temporary: alert, based on StarSymbol font
    aToken.eType = TSQRT;
    std::unique_ptr<SmStructureNode> pSNode(new SmRootNode(aToken));
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pSNode->SetSubNodes(nullptr,pOper,popOrZero(rNodeStack));
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLRowContext_Impl::EndElement()
{
    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    if (rNodeStack.size() > nElementCount)
    {
        auto nSize = rNodeStack.size() - nElementCount;

        aRelationArray.resize(nSize);
        for (auto j=nSize;j > 0;j--)
        {
            auto pNode = std::move(rNodeStack.front());
            rNodeStack.pop_front();
            aRelationArray[j-1] = pNode.release();
        }

        //If the first or last element is an operator with stretchyness
        //set then we must create a brace node here from those elements,
        //removing the stretchness from the operators and applying it to
        //ourselves, and creating the appropriate dummy StarMath none bracket
        //to balance the arrangement
        if (((aRelationArray[0]->GetScaleMode() == SCALE_HEIGHT)
            && (aRelationArray[0]->GetType() == NMATH))
        || ((aRelationArray[nSize-1]->GetScaleMode() == SCALE_HEIGHT)
            && (aRelationArray[nSize-1]->GetType() == NMATH)))
        {
            SmToken aToken;
            aToken.cMathChar = '\0';
            aToken.nLevel = 5;

            int nLeft=0,nRight=0;
            if ((aRelationArray[0]->GetScaleMode() == SCALE_HEIGHT)
                && (aRelationArray[0]->GetType() == NMATH))
            {
                aToken = aRelationArray[0]->GetToken();
                nLeft=1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TLPARENT;
            SmNode *pLeft = new SmMathSymbolNode(aToken);

            if ((aRelationArray[nSize-1]->GetScaleMode() == SCALE_HEIGHT)
                && (aRelationArray[nSize-1]->GetType() == NMATH))
            {
                aToken = aRelationArray[nSize-1]->GetToken();
                nRight=1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TRPARENT;
            SmNode *pRight = new SmMathSymbolNode(aToken);

            SmNodeArray aRelationArray2;

            //!! nSize-nLeft-nRight may be < 0 !!
            int nRelArrSize = nSize-nLeft-nRight;
            if (nRelArrSize > 0)
            {
                aRelationArray2.resize(nRelArrSize);
                for (int i=0;i < nRelArrSize;i++)
                    aRelationArray2[i] = aRelationArray[i+nLeft];
            }

            SmToken aDummy;
            std::unique_ptr<SmStructureNode> pSNode(new SmBraceNode(aToken));
            SmStructureNode *pBody = new SmExpressionNode(aDummy);
            pBody->SetSubNodes(aRelationArray2);

            pSNode->SetSubNodes(pLeft,pBody,pRight);
            pSNode->SetScaleMode(SCALE_HEIGHT);
            rNodeStack.push_front(std::move(pSNode));
            return;
        }
    }
    else
    {
        // The elements msqrt, mstyle, merror, menclose, mpadded, mphantom, mtd, and math
        // treat their content as a single inferred mrow in case their content is empty.
        // Here an empty group {} is used to catch those cases and transform them without error
        // to StarMath.
        aRelationArray.resize(2);
        SmToken aToken;
        aToken.cMathChar = MS_LBRACE;
        aToken.nLevel = 5;
        aToken.eType = TLGROUP;
        aToken.aText = "{";
        aRelationArray[0] = new SmLineNode(aToken);

        aToken.cMathChar = MS_RBRACE;
        aToken.nLevel = 0;
        aToken.eType = TRGROUP;
        aToken.aText = "}";
        aRelationArray[1] = new SmLineNode(aToken);
    }

    SmToken aDummy;
    std::unique_ptr<SmStructureNode> pSNode(new SmExpressionNode(aDummy));
    pSNode->SetSubNodes(aRelationArray);
    rNodeStack.push_front(std::move(pSNode));
}


SvXMLImportContext *SmXMLRowContext_Impl::StrictCreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSmImport().GetPresElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        /*Note that these should accept malignmark subelements, but do not*/
        case XML_TOK_MN:
            pContext = GetSmImport().CreateNumberContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MI:
            pContext = GetSmImport().CreateIdentifierContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MO:
            pContext = GetSmImport().CreateOperatorContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MTEXT:
            pContext = GetSmImport().CreateTextContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MSPACE:
            pContext = GetSmImport().CreateSpaceContext(nPrefix,rLocalName,
                xAttrList);
            break;
        case XML_TOK_MS:
            pContext = GetSmImport().CreateStringContext(nPrefix,rLocalName,
                xAttrList);
            break;

        /*Note: The maligngroup should only be seen when the row
         * (or descendants) are in a table*/
        case XML_TOK_MALIGNGROUP:
            pContext = GetSmImport().CreateAlignGroupContext(nPrefix,rLocalName,
                xAttrList);
            break;

        case XML_TOK_ANNOTATION:
            pContext = GetSmImport().CreateAnnotationContext(nPrefix,rLocalName,
                xAttrList);
            break;

        default:
            break;
    }
    return pContext;
}


SvXMLImportContext *SmXMLRowContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = StrictCreateChildContext(nPrefix,
    rLocalName, xAttrList);

    if (!pContext)
    {
        //Hmm, unrecognized for this level, check to see if its
        //an element that can have an implicit schema around it
        pContext = SmXMLDocContext_Impl::CreateChildContext(nPrefix,
            rLocalName,xAttrList);
    }
    return pContext;
}


SvXMLImportContext *SmXMLMultiScriptsContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresScriptEmptyElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MPRESCRIPTS:
            bHasPrescripts = true;
            ProcessSubSupPairs(false);
            pContext = GetSmImport().CreatePrescriptsContext(nPrefix,
                rLocalName, xAttrList);
            break;
        case XML_TOK_NONE:
            pContext = GetSmImport().CreateNoneContext(nPrefix,rLocalName,
                xAttrList);
            break;
        default:
            pContext = SmXMLRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

void SmXMLMultiScriptsContext_Impl::ProcessSubSupPairs(bool bIsPrescript)
{
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    if (rNodeStack.size() <= nElementCount)
        return;

    auto nCount = rNodeStack.size() - nElementCount - 1;
    if (nCount == 0)
        return;

    if (nCount % 2 == 0)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.eType = bIsPrescript ? TLSUB : TRSUB;

        SmNodeStack aReverseStack;
        for (size_t i = 0; i < nCount + 1; i++)
        {
            auto pNode = std::move(rNodeStack.front());
            rNodeStack.pop_front();
            aReverseStack.push_front(std::move(pNode));
        }

        SmSubSup eSub = bIsPrescript ? LSUB : RSUB;
        SmSubSup eSup = bIsPrescript ? LSUP : RSUP;

        for (size_t i = 0; i < nCount; i += 2)
        {
            std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));

            // initialize subnodes array
            SmNodeArray aSubNodes(1 + SUBSUP_NUM_ENTRIES);

            /*On each loop the base and its sub sup pair becomes the
             base for the next loop to which the next sub sup pair is
             attached, i.e. wheels within wheels*/
            aSubNodes[0] = popOrZero(aReverseStack);

            SmNode *pScriptNode = popOrZero(aReverseStack);

            if (pScriptNode && ((pScriptNode->GetToken().eType != TIDENT) ||
                (!pScriptNode->GetToken().aText.isEmpty())))
                aSubNodes[eSub+1] = pScriptNode;
            pScriptNode = popOrZero(aReverseStack);
            if (pScriptNode && ((pScriptNode->GetToken().eType != TIDENT) ||
                (!pScriptNode->GetToken().aText.isEmpty())))
                aSubNodes[eSup+1] = pScriptNode;

            pNode->SetSubNodes(aSubNodes);
            aReverseStack.push_front(std::move(pNode));
        }
        assert(!aReverseStack.empty());
        auto pNode = std::move(aReverseStack.front());
        aReverseStack.pop_front();
        rNodeStack.push_front(std::move(pNode));
    }
    else
    {
        // Ignore odd number of elements.
        for (size_t i = 0; i < nCount; i++)
        {
            rNodeStack.pop_front();
        }
    }
}


void SmXMLTableContext_Impl::EndElement()
{
    SmNodeArray aExpressionArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNodeStack aReverseStack;
    aExpressionArray.resize(rNodeStack.size()-nElementCount);

    auto nRows = rNodeStack.size()-nElementCount;
    sal_uInt16 nCols = 0;

    SmStructureNode *pArray;
    for (auto i=nRows;i > 0;i--)
    {
        pArray = static_cast<SmStructureNode *>(rNodeStack.front().release());
        rNodeStack.pop_front();
        if (pArray->GetNumSubNodes() == 0)
        {
            //This is a little tricky, it is possible that there was
            //be elements that were not inside a <mtd> pair, in which
            //case they will not be in a row, i.e. they will not have
            //SubNodes, so we have to wait until here before we can
            //resolve the situation. Implicitsurrounding tags are
            //surprisingly difficult to get right within this
            //architecture

            SmNodeArray aRelationArray;
            aRelationArray.resize(1);
            aRelationArray[0] = pArray;
            SmToken aDummy;
            pArray = new SmExpressionNode(aDummy);
            pArray->SetSubNodes(aRelationArray);
        }

        if (pArray->GetNumSubNodes() > nCols)
            nCols = pArray->GetNumSubNodes();
        aReverseStack.push_front(std::unique_ptr<SmStructureNode>(pArray));
    }
    aExpressionArray.resize(nCols*nRows);
    size_t j=0;
    while ( !aReverseStack.empty() )
    {
        pArray = static_cast<SmStructureNode *>(aReverseStack.front().release());
        aReverseStack.pop_front();
        for (sal_uInt16 i=0;i<pArray->GetNumSubNodes();i++)
            aExpressionArray[j++] = pArray->GetSubNode(i);
    }

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TMATRIX;
    std::unique_ptr<SmMatrixNode> pSNode(new SmMatrixNode(aToken));
    pSNode->SetSubNodes(aExpressionArray);
    pSNode->SetRowCol(static_cast<sal_uInt16>(nRows),nCols);
    rNodeStack.push_front(std::move(pSNode));
}

SvXMLImportContext *SmXMLTableRowContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresTableElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MTD:
            pContext = GetSmImport().CreateTableCellContext(nPrefix,
                rLocalName, xAttrList);
            break;
        default:
            pContext = SmXMLRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

SvXMLImportContext *SmXMLTableContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSmImport().
        GetPresTableElemTokenMap();
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MTR:
            pContext = GetSmImport().CreateTableRowContext(nPrefix,rLocalName,
                xAttrList);
            break;
        default:
            pContext = SmXMLTableRowContext_Impl::CreateChildContext(nPrefix,
                rLocalName,xAttrList);
            break;
    }
    return pContext;
}

void SmXMLMultiScriptsContext_Impl::EndElement()
{
    ProcessSubSupPairs(bHasPrescripts);
}

void SmXMLActionContext_Impl::StartElement(const uno::Reference<xml::sax::XAttributeList> & xAttrList)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0;i<nAttrCount;i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName(sAttrName,&aLocalName);

        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap &rAttrTokenMap =
            GetSmImport().GetActionAttrTokenMap();
        switch(rAttrTokenMap.Get(nPrefix,aLocalName))
        {
            case XML_TOK_SELECTION:
                {
                    sal_uInt32 n = sValue.toUInt32();
                    if (n > 0) mnSelection = static_cast<size_t>(n);
                }
                break;
            default:
                break;
        }
    }
}

void SmXMLActionContext_Impl::EndElement()
{
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    auto nSize = rNodeStack.size();
    if (nSize <= nElementCount) {
        // not compliant to maction's specification, e.g., no subexpressions
        return;
    }
    assert(mnSelection > 0);
    if (nSize < nElementCount + mnSelection) {
        // No selected subexpression exists, which is a MathML error;
        // fallback to selecting the first
        mnSelection = 1;
    }
    assert(nSize >= nElementCount + mnSelection);
    for (auto i=nSize-(nElementCount+mnSelection); i > 0; i--)
    {
        rNodeStack.pop_front();
    }
    auto pSelected = std::move(rNodeStack.front());
    rNodeStack.pop_front();
    for (auto i=rNodeStack.size()-nElementCount; i > 0; i--)
    {
        rNodeStack.pop_front();
    }
    rNodeStack.push_front(std::move(pSelected));
}

SvXMLImportContext *SmXMLImport::CreateContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    if ( XML_NAMESPACE_OFFICE == nPrefix )
    {
        if ( (IsXMLToken(rLocalName, XML_DOCUMENT) ||
              IsXMLToken(rLocalName, XML_DOCUMENT_META)))
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            return IsXMLToken(rLocalName, XML_DOCUMENT_META)
                ? new SvXMLMetaDocumentContext(*this,
                        XML_NAMESPACE_OFFICE, rLocalName,
                        xDPS->getDocumentProperties())
                // flat OpenDocument file format -- this has not been tested...
                : new SmXMLFlatDocContext_Impl( *this, nPrefix, rLocalName,
                            xDPS->getDocumentProperties());
        }
        else
        {
            return new SmXMLOfficeContext_Impl( *this,nPrefix,rLocalName);
        }
    }
    else
        return new SmXMLDocContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateRowContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
        return new SmXMLRowContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateTextContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLTextContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateAnnotationContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLAnnotationContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateStringContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLStringContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateNumberContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLNumberContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateIdentifierContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLIdentifierContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateOperatorContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLOperatorContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSpaceContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLSpaceContext_Impl(*this,nPrefix,rLocalName);
}


SvXMLImportContext *SmXMLImport::CreateEncloseContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLEncloseContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateFracContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLFracContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSqrtContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLSqrtContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateRootContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLRootContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateStyleContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLStyleContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePaddedContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLPaddedContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePhantomContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLPhantomContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateFencedContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLFencedContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateErrorContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLErrorContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSubContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLSubContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSubSupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLSubSupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateSupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLSupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateUnderContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
        return new SmXMLUnderContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateOverContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLOverContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateUnderOverContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLUnderOverContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateMultiScriptsContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLMultiScriptsContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateTableContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLTableContext_Impl(*this,nPrefix,rLocalName);
}
SvXMLImportContext *SmXMLImport::CreateTableRowContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLTableRowContext_Impl(*this,nPrefix,rLocalName);
}
SvXMLImportContext *SmXMLImport::CreateTableCellContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLTableCellContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateNoneContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLNoneContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreatePrescriptsContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLPrescriptsContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateAlignGroupContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLAlignGroupContext_Impl(*this,nPrefix,rLocalName);
}

SvXMLImportContext *SmXMLImport::CreateActionContext(sal_uInt16 nPrefix,
    const OUString &rLocalName,
    const uno::Reference <xml::sax::XAttributeList> & /*xAttrList*/)
{
    return new SmXMLActionContext_Impl(*this,nPrefix,rLocalName);
}

SmXMLImport::~SmXMLImport() throw ()
{
}

void SmXMLImport::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
{
    uno::Reference <frame::XModel> xModel = GetModel();
    if ( !xModel.is() )
        return;

    uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    if ( !pModel )
        return;

    SmDocShell *pDocShell =
        static_cast<SmDocShell*>(pModel->GetObjectShell());
    if ( !pDocShell )
        return;

    Rectangle aRect( pDocShell->GetVisArea() );

    sal_Int32 nCount = aViewProps.getLength();
    const PropertyValue *pValue = aViewProps.getConstArray();

    long nTmp = 0;

    for (sal_Int32 i = 0; i < nCount ; i++)
    {
        if (pValue->Name == "ViewAreaTop" )
        {
            pValue->Value >>= nTmp;
            aRect.setY( nTmp );
        }
        else if (pValue->Name == "ViewAreaLeft" )
        {
            pValue->Value >>= nTmp;
            aRect.setX( nTmp );
        }
        else if (pValue->Name == "ViewAreaWidth" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Width() = nTmp;
            aRect.SetSize( aSize );
        }
        else if (pValue->Name == "ViewAreaHeight" )
        {
            pValue->Value >>= nTmp;
            Size aSize( aRect.GetSize() );
            aSize.Height() = nTmp;
            aRect.SetSize( aSize );
        }
        pValue++;
    }

    pDocShell->SetVisArea ( aRect );
}

void SmXMLImport::SetConfigurationSettings(const Sequence<PropertyValue>& aConfProps)
{
    uno::Reference < XPropertySet > xProps ( GetModel(), UNO_QUERY );
    if ( xProps.is() )
    {
        Reference < XPropertySetInfo > xInfo ( xProps->getPropertySetInfo() );
        if (xInfo.is() )
        {
            sal_Int32 nCount = aConfProps.getLength();
            const PropertyValue* pValues = aConfProps.getConstArray();

            const OUString sFormula ( "Formula" );
            const OUString sBasicLibraries ( "BasicLibraries" );
            const OUString sDialogLibraries ( "DialogLibraries" );
            while ( nCount-- )
            {
                if (pValues->Name != sFormula &&
                    pValues->Name != sBasicLibraries &&
                    pValues->Name != sDialogLibraries)
                {
                    try
                    {
                        if ( xInfo->hasPropertyByName( pValues->Name ) )
                            xProps->setPropertyValue( pValues->Name, pValues->Value );
                    }
                    catch (const beans::PropertyVetoException &)
                    {
                        // dealing with read-only properties here. Nothing to do...
                    }
                    catch (const Exception& rEx)
                    {
                        SAL_WARN("starmath", "SmXMLImport::SetConfigurationSettings: Exception: " << rEx.Message );
                    }
                }

                pValues++;
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

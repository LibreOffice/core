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
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
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

#include "mathmlimport.hxx"
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

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#define IMPORT_SVC_NAME "com.sun.star.xml.XMLImportFilter"

#undef WANTEXCEPT

////////////////////////////////////////////////////////////

namespace {
template < typename T >
T* lcl_popOrZero( ::std::stack<T*> & rStack )
{
    if (rStack.empty())
        return 0;
    T* pTmp = rStack.top();
    rStack.pop();
    return pTmp;
}
}

sal_uLong SmXMLImportWrapper::Import(SfxMedium &rMedium)
{
    sal_uLong nError = ERRCODE_SFX_DOLOADFAILED;

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(
        comphelper::getProcessServiceFactory());
    OSL_ENSURE(xServiceFactory.is(), "XMLReader::Read: got no service manager");
    if ( !xServiceFactory.is() )
        return nError;

    //Make a model component from our SmModel
    uno::Reference< lang::XComponent > xModelComp( xModel, uno::UNO_QUERY );
    OSL_ENSURE( xModelComp.is(), "XMLReader::Read: got no model" );

    // try to get an XStatusIndicator from the Medium
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    sal_Bool bEmbedded = sal_False;
    uno::Reference <lang::XUnoTunnel> xTunnel;
    xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    SmDocShell *pDocShell = pModel ?
            static_cast<SmDocShell*>(pModel->GetObjectShell()) : 0;
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

        if ( SFX_CREATE_MODE_EMBEDDED == pDocShell->GetCreateMode() )
            bEmbedded = sal_True;
    }

    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { "PrivateData", sizeof("PrivateData")-1, 0,
              &::getCppuType( (Reference<XInterface> *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "BaseURI", sizeof("BaseURI")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StreamRelPath", sizeof("StreamRelPath")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StreamName", sizeof("StreamName")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    // Set base URI
    OUString sPropName( "BaseURI" );
    xInfoSet->setPropertyValue( sPropName, makeAny( rMedium.GetBaseURL() ) );

    sal_Int32 nSteps=3;
    if ( !(rMedium.IsStorage()))
        nSteps = 1;

    sal_Int32 nProgressRange(nSteps);
    if (xStatusIndicator.is())
    {
        xStatusIndicator->start(String(SmResId(STR_STATSTR_READING)),
            nProgressRange);
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
                sPropName = "StreamRelPath";
                xInfoSet->setPropertyValue( sPropName, makeAny( aName ) );
            }
        }

        sal_Bool bOASIS = ( SotStorage::GetVersion( rMedium.GetStorage() ) > SOFFICE_FILEFORMAT_60 );
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        sal_uLong nWarn = ReadThroughComponent(
            rMedium.GetStorage(), xModelComp, "meta.xml", "Meta.xml",
            xServiceFactory, xInfoSet,
                (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaImporter"
                        : "com.sun.star.comp.Math.XMLMetaImporter") );

        if ( nWarn != ERRCODE_IO_BROKENPACKAGE )
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            nWarn = ReadThroughComponent(
                rMedium.GetStorage(), xModelComp, "settings.xml", 0,
                xServiceFactory, xInfoSet,
                (bOASIS ? "com.sun.star.comp.Math.XMLOasisSettingsImporter"
                        : "com.sun.star.comp.Math.XMLSettingsImporter" ) );

            if ( nWarn != ERRCODE_IO_BROKENPACKAGE )
            {
                if (xStatusIndicator.is())
                    xStatusIndicator->setValue(nSteps++);

                nError = ReadThroughComponent(
                    rMedium.GetStorage(), xModelComp, "content.xml", "Content.xml",
                    xServiceFactory, xInfoSet, "com.sun.star.comp.Math.XMLImporter" );
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
            xServiceFactory, xInfoSet, "com.sun.star.comp.Math.XMLImporter", false );
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return nError;
}


/// read a component (file + filter version)
sal_uLong SmXMLImportWrapper::ReadThroughComponent(
    Reference<io::XInputStream> xInputStream,
    Reference<XComponent> xModelComponent,
    Reference<lang::XMultiServiceFactory> & rFactory,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pFilterName,
    sal_Bool bEncrypted )
{
    sal_uLong nError = ERRCODE_SFX_DOLOADFAILED;
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rFactory.is(), "factory missing");
    OSL_ENSURE(NULL != pFilterName,"I need a service name for the component!");

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    Reference< xml::sax::XParser > xParser(
        rFactory->createInstance(
            "com.sun.star.xml.sax.Parser"),
        UNO_QUERY );
    OSL_ENSURE( xParser.is(), "Can't create parser" );
    if ( !xParser.is() )
        return nError;

    Sequence<Any> aArgs( 1 );
    aArgs[0] <<= rPropSet;

    // get filter
    Reference< xml::sax::XDocumentHandler > xFilter(
        rFactory->createInstanceWithArguments(
            OUString::createFromAscii(pFilterName), aArgs ),
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

        uno::Reference<lang::XUnoTunnel> xFilterTunnel;
        xFilterTunnel = uno::Reference<lang::XUnoTunnel>
            ( xFilter, uno::UNO_QUERY );
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
        xml::sax::SAXException aSaxEx = *(xml::sax::SAXException*)(&r);
        sal_Bool bTryChild = sal_True;

        while( bTryChild )
        {
            xml::sax::SAXException aTmp;
            if ( aSaxEx.WrappedException >>= aTmp )
                aSaxEx = aTmp;
            else
                bTryChild = sal_False;
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
    Reference<XComponent> xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    Reference<lang::XMultiServiceFactory> & rFactory,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pFilterName )
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(NULL != pStreamName, "Please, please, give me a name!");

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
        sal_Bool bEncrypted = sal_False;
        if ( aAny.getValueType() == ::getBooleanCppuType() )
            aAny >>= bEncrypted;

        // set Base URL
        if ( rPropSet.is() )
        {
            OUString sPropName( "StreamName");
            rPropSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
        }


        Reference < io::XInputStream > xStream = xEventsStream->getInputStream();
        return ReadThroughComponent( xStream, xModelComponent, rFactory, rPropSet, pFilterName, bEncrypted );
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

////////////////////////////////////////////////////////////

SmXMLImport::SmXMLImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    sal_uInt16 nImportFlags)
:   SvXMLImport( xServiceFactory, nImportFlags ),
    pPresLayoutElemTokenMap(0),
    pPresLayoutAttrTokenMap(0),
    pFencedAttrTokenMap(0),
    pOperatorAttrTokenMap(0),
    pAnnotationAttrTokenMap(0),
    pPresElemTokenMap(0),
    pPresScriptEmptyElemTokenMap(0),
    pPresTableElemTokenMap(0),
    pColorTokenMap(0),
    bSuccess(sal_False)
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
    const OUString aServiceName( IMPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
        return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLImport(rSMgr, IMPORT_ALL);
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLImportMeta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisMetaImporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLImportMeta_getSupportedServiceNames()
throw()
{
    const OUString aServiceName( IMPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImportMeta_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLImport( rSMgr, IMPORT_META );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisSettingsImporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLImportSettings_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( IMPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
        return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLImportSettings_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLImport( rSMgr, IMPORT_SETTINGS );
}

////////////////////////////////////////////////////////////

// XServiceInfo
// override empty method from parent class
rtl::OUString SAL_CALL SmXMLImport::getImplementationName()
    throw(uno::RuntimeException)
{
    OUString aTxt;
    switch( getImportFlags() )
    {
        case IMPORT_META:
            aTxt = SmXMLImportMeta_getImplementationName();
            break;
        case IMPORT_SETTINGS:
            aTxt = SmXMLImportSettings_getImplementationName();
            break;
        case IMPORT_ALL:
        default:
            aTxt = SmXMLImport_getImplementationName();
            break;
    }
    return aTxt;
}


sal_Int64 SAL_CALL SmXMLImport::getSomething(
    const uno::Sequence< sal_Int8 >&rId )
throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
        rId.getConstArray(), 16 ) )
    return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_uIntPtr >(this));

    return SvXMLImport::getSomething( rId );
}

void SmXMLImport::endDocument(void)
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    //Set the resulted tree into the SmDocShell where it belongs
    SmNode *pTree;
    if (NULL != (pTree = GetTree()))
    {
        uno::Reference <frame::XModel> xModel = GetModel();
        uno::Reference <lang::XUnoTunnel> xTunnel;
        xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
        SmModel *pModel = reinterpret_cast<SmModel *>
            (xTunnel->getSomething(SmModel::getUnoTunnelId()));

        if (pModel)
        {
            SmDocShell *pDocShell =
                static_cast<SmDocShell*>(pModel->GetObjectShell());
            pDocShell->SetFormulaTree(pTree);
            if (0 == aText.Len())  //If we picked up no annotation text
            {
                //Make up some editable text
                aText = pDocShell->GetText();
                pTree->CreateTextFromNode(aText);
                aText = comphelper::string::stripEnd(aText, ' ');
            }
            pDocShell->SetText( String() );

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

        bSuccess = sal_True;
    }

    SvXMLImport::endDocument();
}

////////////////////////////////////////////////////////////

class SmXMLImportContext: public SvXMLImportContext
{
public:
    SmXMLImportContext( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SvXMLImportContext(rImport, nPrfx, rLName) {}

    const SmXMLImport& GetSmImport() const
    {
        return (const SmXMLImport&)GetImport();
    }

    SmXMLImport& GetSmImport()
    {
        return (SmXMLImport&)GetImport();
    }

    virtual void TCharacters(const OUString & /*rChars*/);
    virtual void Characters(const OUString &rChars);
    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 /*nPrefix*/, const OUString& /*rLocalName*/, const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/);
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
    return 0;
}

////////////////////////////////////////////////////////////

struct SmXMLContext_Helper
{
    sal_Int8 nIsBold;
    sal_Int8 nIsItalic;
    double nFontSize;
    sal_Bool bFontNodeNeeded;
    OUString sFontFamily;
    OUString sColor;

    SmXMLImportContext rContext;

    SmXMLContext_Helper(SmXMLImportContext &rImport) :
        nIsBold(-1), nIsItalic(-1), nFontSize(0.0), rContext(rImport)  {}

    void RetrieveAttrs(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
    void ApplyAttrs();
};

void SmXMLContext_Helper::RetrieveAttrs(const uno::Reference<
    xml::sax::XAttributeList > & xAttrList )
{
    sal_Int8 nOldIsBold=nIsBold;
    sal_Int8 nOldIsItalic=nIsItalic;
    double nOldFontSize=nFontSize;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    OUString sOldFontFamily = sFontFamily;
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
                nIsBold = sValue.equals(GetXMLToken(XML_BOLD));
                break;
            case XML_TOK_FONTSTYLE:
                nIsItalic = sValue.equals(GetXMLToken(XML_ITALIC));
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
            default:
                break;
        }
    }

    if ((nOldIsBold!=nIsBold) || (nOldIsItalic!=nIsItalic) ||
        (nOldFontSize!=nFontSize) || (sOldFontFamily!=sFontFamily)
        || !sColor.isEmpty())
        bFontNodeNeeded=sal_True;
    else
        bFontNodeNeeded=sal_False;
}

void SmXMLContext_Helper::ApplyAttrs()
{
    SmNodeStack &rNodeStack = rContext.GetSmImport().GetNodeStack();

    if (bFontNodeNeeded)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;

        if (nIsBold != -1)
        {
            if (nIsBold)
                aToken.eType = TBOLD;
            else
                aToken.eType = TNBOLD;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,lcl_popOrZero(rNodeStack));
            rNodeStack.push(pFontNode);
        }
        if (nIsItalic != -1)
        {
            if (nIsItalic)
                aToken.eType = TITALIC;
            else
                aToken.eType = TNITALIC;
            SmStructureNode *pFontNode = static_cast<SmStructureNode *>
                (new SmFontNode(aToken));
            pFontNode->SetSubNodes(0,lcl_popOrZero(rNodeStack));
            rNodeStack.push(pFontNode);
        }
        if (nFontSize != 0.0)
        {
            aToken.eType = TSIZE;
            SmFontNode *pFontNode = new SmFontNode(aToken);

            if (util::MeasureUnit::PERCENT == rContext.GetSmImport()
                    .GetMM100UnitConverter().GetXMLMeasureUnit())
            {
                if (nFontSize < 100.00)
                    pFontNode->SetSizeParameter(Fraction(100.00/nFontSize),
                        FNTSIZ_DIVIDE);
                else
                    pFontNode->SetSizeParameter(Fraction(nFontSize/100.00),
                        FNTSIZ_MULTIPLY);
            }
            else
                pFontNode->SetSizeParameter(Fraction(nFontSize),FNTSIZ_ABSOLUT);

            pFontNode->SetSubNodes(0,lcl_popOrZero(rNodeStack));
            rNodeStack.push(pFontNode);
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
            SmFontNode *pFontNode = new SmFontNode(aToken);
            pFontNode->SetSubNodes(0,lcl_popOrZero(rNodeStack));
            rNodeStack.push(pFontNode);
        }
        if (!sColor.isEmpty())
        {
            //Again we can only handle a small set of colours in
            //StarMath for now.
            const SvXMLTokenMap& rTokenMap =
                rContext.GetSmImport().GetColorTokenMap();
            aToken.eType = static_cast<SmTokenType>(rTokenMap.Get(
                XML_NAMESPACE_MATH, sColor));
            if (aToken.eType != -1)
            {
                SmFontNode *pFontNode = new SmFontNode(aToken);
                pFontNode->SetSubNodes(0,lcl_popOrZero(rNodeStack));
                rNodeStack.push(pFontNode);
            }
        }

    }
}

////////////////////////////////////////////////////////////

class SmXMLDocContext_Impl : public SmXMLImportContext
{
public:
    SmXMLDocContext_Impl( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrfx,rLName) {}

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList);

    void EndElement();
};

////////////////////////////////////////////////////////////

/*avert thy gaze from the proginator*/
class SmXMLRowContext_Impl : public SmXMLDocContext_Impl
{
protected:
    sal_uLong nElementCount;

public:
    SmXMLRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLDocContext_Impl(rImport,nPrefix,rLName)
        { nElementCount = GetSmImport().GetNodeStack().size(); }

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList);

    SvXMLImportContext *StrictCreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);

    void EndElement();
};

////////////////////////////////////////////////////////////

class SmXMLFracContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLFracContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

////////////////////////////////////////////////////////////

class SmXMLSqrtContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSqrtContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

////////////////////////////////////////////////////////////

class SmXMLRootContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLRootContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

////////////////////////////////////////////////////////////

class SmXMLStyleContext_Impl : public SmXMLRowContext_Impl
{
protected:
    SmXMLContext_Helper aStyleHelper;

public:
    /*Right now the style tag is completely ignored*/
    SmXMLStyleContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) : SmXMLRowContext_Impl(rImport,nPrefix,rLName),
        aStyleHelper(*this) {}

    void EndElement();
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
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
    if (rNodeStack.size() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();
    aStyleHelper.ApplyAttrs();
}

////////////////////////////////////////////////////////////

class SmXMLPaddedContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPaddedContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

void SmXMLPaddedContext_Impl::EndElement()
{
    /*
    <mpadded> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();
}

////////////////////////////////////////////////////////////

class SmXMLPhantomContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPhantomContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

void SmXMLPhantomContext_Impl::EndElement()
{
    /*
    <mphantom> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 5;
    aToken.eType = TPHANTOM;

    SmStructureNode *pPhantom = static_cast<SmStructureNode *>
        (new SmFontNode(aToken));
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pPhantom->SetSubNodes(0,lcl_popOrZero(rNodeStack));
    rNodeStack.push(pPhantom);
}

////////////////////////////////////////////////////////////

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

    void StartElement(const uno::Reference< xml::sax::XAttributeList > & xAttrList );
    void EndElement();
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
    aToken.nGroup = 0;
    aToken.aText = ',';
    aToken.eType = TLEFT;
    aToken.nLevel = 5;

    aToken.eType = TLPARENT;
    aToken.cMathChar = cBegin;
    SmStructureNode *pSNode = new SmBraceNode(aToken);
    SmNode *pLeft = new SmMathSymbolNode(aToken);

    aToken.cMathChar = cEnd;
    aToken.eType = TRPARENT;
    SmNode *pRight = new SmMathSymbolNode(aToken);

    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    aToken.cMathChar = '\0';
    aToken.aText = ',';
    aToken.eType = TIDENT;

    sal_uLong i = rNodeStack.size() - nElementCount;
    if (rNodeStack.size() - nElementCount > 1)
        i += rNodeStack.size() - 1 - nElementCount;
    aRelationArray.resize(i);
    while (rNodeStack.size() > nElementCount)
    {
        aRelationArray[--i] = rNodeStack.top();
        rNodeStack.pop();
        if (i > 1 && rNodeStack.size() > 1)
            aRelationArray[--i] = new SmGlyphSpecialNode(aToken);
    }

    SmToken aDummy;
    SmStructureNode *pBody = new SmExpressionNode(aDummy);
    pBody->SetSubNodes(aRelationArray);


    pSNode->SetSubNodes(pLeft,pBody,pRight);
    pSNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().push(pSNode);
}


////////////////////////////////////////////////////////////

class SmXMLErrorContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLErrorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement();
};

void SmXMLErrorContext_Impl::EndElement()
{
    /*Right now the error tag is completely ignored, what
     can I do with it in starmath, ?, maybe we need a
     report window ourselves, do a test for validity of
     the xml input, use merrors, and then generate
     the markup inside the merror with a big red colour
     of something. For now just throw them all away.
     */
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    while (rNodeStack.size() > nElementCount)
    {
        delete rNodeStack.top();
        rNodeStack.pop();
    }
}

////////////////////////////////////////////////////////////

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
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TNUMBER;
    }

    virtual void TCharacters(const OUString &rChars);

    void EndElement();
};

void SmXMLNumberContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}

void SmXMLNumberContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push(new SmTextNode(aToken,FNT_NUMBER));
}

////////////////////////////////////////////////////////////

class SmXMLAnnotationContext_Impl : public SmXMLImportContext
{
    sal_Bool bIsStarMath;

public:
    SmXMLAnnotationContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName), bIsStarMath(sal_False) {}

    virtual void Characters(const OUString &rChars);

    void StartElement(const uno::Reference<xml::sax::XAttributeList > & xAttrList );
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
        GetSmImport().GetText().Append(String(rChars));
}

////////////////////////////////////////////////////////////

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
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString &rChars);

    void EndElement();
};

void SmXMLTextContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}

void SmXMLTextContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push(new SmTextNode(aToken,FNT_TEXT));
}

////////////////////////////////////////////////////////////

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
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString &rChars);

    void EndElement();
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
    aToken.aText.Erase();
    aToken.aText += '\"';
    aToken.aText += String(rChars);
    aToken.aText += '\"';
}

void SmXMLStringContext_Impl::EndElement()
{
    GetSmImport().GetNodeStack().push(new SmTextNode(aToken,FNT_FIXED));
}

////////////////////////////////////////////////////////////

class SmXMLIdentifierContext_Impl : public SmXMLImportContext
{
protected:
    SmXMLContext_Helper aStyleHelper;
    SmToken aToken;

public:
    SmXMLIdentifierContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName),aStyleHelper(*this)
    {
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TIDENT;
    }

    void TCharacters(const OUString &rChars);
    void StartElement(const uno::Reference< xml::sax::XAttributeList > & xAttrList )
    {
        aStyleHelper.RetrieveAttrs(xAttrList);
    };
    void EndElement();
};

void SmXMLIdentifierContext_Impl::EndElement()
{
    SmTextNode *pNode = 0;
    //we will handle identifier italic/normal here instead of with a standalone
    //font node
    if (((aStyleHelper.nIsItalic == -1) && (aToken.aText.Len() > 1))
        || ((aStyleHelper.nIsItalic == 0) && (aToken.aText.Len() == 1)))
    {
        pNode = new SmTextNode(aToken,FNT_FUNCTION);
        pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    else
        pNode = new SmTextNode(aToken,FNT_VARIABLE);
    if (aStyleHelper.bFontNodeNeeded && aStyleHelper.nIsItalic != -1)
    {
        if (aStyleHelper.nIsItalic)
            pNode->GetFont().SetItalic(ITALIC_NORMAL);
        else
            pNode->GetFont().SetItalic(ITALIC_NONE);
    }

    if ((-1!=aStyleHelper.nIsBold) || (0.0!=aStyleHelper.nFontSize) ||
        (!aStyleHelper.sFontFamily.isEmpty()) ||
        !aStyleHelper.sColor.isEmpty())
        aStyleHelper.bFontNodeNeeded=sal_True;
    else
        aStyleHelper.bFontNodeNeeded=sal_False;
    if (aStyleHelper.bFontNodeNeeded)
        aStyleHelper.ApplyAttrs();
    GetSmImport().GetNodeStack().push(pNode);
}

void SmXMLIdentifierContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.aText = rChars;
}

////////////////////////////////////////////////////////////

class SmXMLOperatorContext_Impl : public SmXMLImportContext
{
    sal_Bool bIsStretchy;

protected:
    SmToken aToken;

public:
    SmXMLOperatorContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName), bIsStretchy(sal_False)
    {
        aToken.nGroup = 0;
        aToken.eType = TSPECIAL;
        aToken.nLevel = 5;
    }

    void TCharacters(const OUString &rChars);
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
    void EndElement();
};

void SmXMLOperatorContext_Impl::TCharacters(const OUString &rChars)
{
    aToken.cMathChar = rChars[0];
}

void SmXMLOperatorContext_Impl::EndElement()
{
    SmMathSymbolNode *pNode = new SmMathSymbolNode(aToken);
    //For stretchy scaling the scaling must be retrieved from this node
    //and applied to the expression itself so as to get the expression
    //to scale the operator to the height of the expression itself
    if (bIsStretchy)
        pNode->SetScaleMode(SCALE_HEIGHT);
    GetSmImport().GetNodeStack().push(pNode);
}



void SmXMLOperatorContext_Impl::StartElement(const uno::Reference<
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


////////////////////////////////////////////////////////////

class SmXMLSpaceContext_Impl : public SmXMLImportContext
{
public:
    SmXMLSpaceContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}

    void StartElement(const uno::Reference< xml::sax::XAttributeList >& xAttrList );
};

void SmXMLSpaceContext_Impl::StartElement(
    const uno::Reference<xml::sax::XAttributeList > & /*xAttrList*/ )
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.eType = TBLANK;
    aToken.nLevel = 5;
    SmBlankNode *pBlank = new SmBlankNode(aToken);
    pBlank->IncreaseBy(aToken);
    GetSmImport().GetNodeStack().push(pBlank);
}

////////////////////////////////////////////////////////////

class SmXMLSubContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType,SmSubSup aSubSup);

public:
    SmXMLSubContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement()
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
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = eType;
    SmSubSupNode *pNode = new SmSubSupNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (sal_uLong i = 1;  i < aSubNodes.size();  i++)
        aSubNodes[i] = NULL;

    aSubNodes[eSubSup+1] = lcl_popOrZero(rNodeStack);
    aSubNodes[0] = lcl_popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.push(pNode);
}

////////////////////////////////////////////////////////////

class SmXMLSupContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement()
    {
        GenericEndElement(TRSUP,RSUP);
    }
};

////////////////////////////////////////////////////////////

class SmXMLSubSupContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType, SmSubSup aSub,SmSubSup aSup);

public:
    SmXMLSubSupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLRowContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement()
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
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = eType;
    SmSubSupNode *pNode = new SmSubSupNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (sal_uLong i = 1;  i < aSubNodes.size();  i++)
        aSubNodes[i] = NULL;

    aSubNodes[aSup+1] = lcl_popOrZero(rNodeStack);
    aSubNodes[aSub+1] = lcl_popOrZero(rNodeStack);
    aSubNodes[0] =  lcl_popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    rNodeStack.push(pNode);
}

////////////////////////////////////////////////////////////

class SmXMLUnderContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLUnderContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName) {}

    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
    void EndElement();
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
    SmNode *pTest = lcl_popOrZero(rNodeStack);
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TUNDERLINE;


    SmNodeArray aSubNodes;
    aSubNodes.resize(2);

    SmStructureNode *pNode = new SmAttributNode(aToken);
    if ((pTest->GetToken().cMathChar & 0x0FFF) == 0x0332)
    {
        aSubNodes[0] = new SmRectangleNode(aToken);
        delete pTest;
    }
    else
        aSubNodes[0] = pTest;

    aSubNodes[1] = lcl_popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.push(pNode);
}


void SmXMLUnderContext_Impl::EndElement()
{
    if (!nAttrCount)
        GenericEndElement(TCSUB,CSUB);
    else
        HandleAccent();
}

////////////////////////////////////////////////////////////

class SmXMLOverContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubContext_Impl(rImport,nPrefix,rLName), nAttrCount(0) {}

    void EndElement();
    void StartElement(const uno::Reference< xml::sax::XAttributeList > &xAttrList );
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
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TACUTE;

    SmAttributNode *pNode = new SmAttributNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    SmNodeArray aSubNodes;
    aSubNodes.resize(2);
    aSubNodes[0] = lcl_popOrZero(rNodeStack);
    aSubNodes[1] = lcl_popOrZero(rNodeStack);
    pNode->SetSubNodes(aSubNodes);
    pNode->SetScaleMode(SCALE_WIDTH);
    rNodeStack.push(pNode);

}

////////////////////////////////////////////////////////////

class SmXMLUnderOverContext_Impl : public SmXMLSubSupContext_Impl
{
public:
    SmXMLUnderOverContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLSubSupContext_Impl(rImport,nPrefix,rLName) {}

    void EndElement()
    {
        GenericEndElement(TCSUB,CSUB,CSUP);
    }
};

////////////////////////////////////////////////////////////

class SmXMLMultiScriptsContext_Impl : public SmXMLSubSupContext_Impl
{
    bool bHasPrescripts;

    void ProcessSubSupPairs(bool bIsPrescript);

public:
    SmXMLMultiScriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLSubSupContext_Impl(rImport,nPrefix,rLName),
        bHasPrescripts(false) {}

    void EndElement();
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};

////////////////////////////////////////////////////////////

class SmXMLNoneContext_Impl : public SmXMLImportContext
{
public:
    SmXMLNoneContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}

    void EndElement();
};


void SmXMLNoneContext_Impl::EndElement(void)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = 0;
    aToken.aText.Erase();
    aToken.nLevel = 5;
    aToken.eType = TIDENT;
    GetSmImport().GetNodeStack().push(
        new SmTextNode(aToken,FNT_VARIABLE));
}

////////////////////////////////////////////////////////////

class SmXMLPrescriptsContext_Impl : public SmXMLImportContext
{
public:
    SmXMLPrescriptsContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName)
        : SmXMLImportContext(rImport,nPrefix,rLName) {}
};

////////////////////////////////////////////////////////////

class SmXMLTableRowContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableRowContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}

    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};


////////////////////////////////////////////////////////////

class SmXMLTableContext_Impl : public SmXMLTableRowContext_Impl
{
public:
    SmXMLTableContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLTableRowContext_Impl(rImport,nPrefix,rLName)
        {}

    void EndElement();
    SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};


////////////////////////////////////////////////////////////

class SmXMLTableCellContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableCellContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}
};

////////////////////////////////////////////////////////////

class SmXMLAlignGroupContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLAlignGroupContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}

    /*Don't do anything with alignment for now*/
    void EndElement()
    {
    }
};

////////////////////////////////////////////////////////////

class SmXMLActionContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLActionContext_Impl(SmXMLImport &rImport,sal_uInt16 nPrefix,
        const OUString& rLName) :
        SmXMLRowContext_Impl(rImport,nPrefix,rLName)
        {}

    void EndElement();
};

////////////////////////////////////////////////////////////

// NB: virtually inherit so we can multiply inherit properly
//     in SmXMLFlatDocContext_Impl
class SmXMLOfficeContext_Impl : public virtual SvXMLImportContext
{
public:
    SmXMLOfficeContext_Impl( SmXMLImport &rImport, sal_uInt16 nPrfx,
        const OUString& rLName)
        : SvXMLImportContext(rImport,nPrfx,rLName) {}

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList > &xAttrList);
};

SvXMLImportContext *SmXMLOfficeContext_Impl::CreateChildContext(sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > &xAttrList)
{
    SvXMLImportContext *pContext = 0;
    if ( XML_NAMESPACE_OFFICE == nPrefix &&
        rLocalName == GetXMLToken(XML_META) )
    {
        OSL_FAIL("XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
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

////////////////////////////////////////////////////////////

// context for flat file xml format
class SmXMLFlatDocContext_Impl
    : public SmXMLOfficeContext_Impl, public SvXMLMetaDocumentContext
{
public:
    SmXMLFlatDocContext_Impl( SmXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const OUString & i_rLName,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual ~SmXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(sal_uInt16 i_nPrefix, const OUString& i_rLocalName, const uno::Reference<xml::sax::XAttributeList>& i_xAttrList);
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

SmXMLFlatDocContext_Impl::~SmXMLFlatDocContext_Impl()
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

////////////////////////////////////////////////////////////

static SvXMLTokenMapEntry aPresLayoutElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_SEMANTICS, XML_TOK_SEMANTICS },
    { XML_NAMESPACE_MATH,   XML_MATH,      XML_TOK_MATH   },
    { XML_NAMESPACE_MATH,   XML_MSTYLE,    XML_TOK_MSTYLE  },
    { XML_NAMESPACE_MATH,   XML_MERROR,    XML_TOK_MERROR },
    { XML_NAMESPACE_MATH,   XML_MPHANTOM,  XML_TOK_MPHANTOM },
    { XML_NAMESPACE_MATH,   XML_MROW,      XML_TOK_MROW },
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

static SvXMLTokenMapEntry aPresLayoutAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_FONTWEIGHT,      XML_TOK_FONTWEIGHT    },
    { XML_NAMESPACE_MATH,   XML_FONTSTYLE,       XML_TOK_FONTSTYLE     },
    { XML_NAMESPACE_MATH,   XML_FONTSIZE,        XML_TOK_FONTSIZE      },
    { XML_NAMESPACE_MATH,   XML_FONTFAMILY,      XML_TOK_FONTFAMILY    },
    { XML_NAMESPACE_MATH,   XML_COLOR,           XML_TOK_COLOR },
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aFencedAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_OPEN,       XML_TOK_OPEN },
    { XML_NAMESPACE_MATH,   XML_CLOSE,      XML_TOK_CLOSE },
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aOperatorAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_STRETCHY,      XML_TOK_STRETCHY },
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aAnnotationAttrTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_ENCODING,      XML_TOK_ENCODING },
    XML_TOKEN_MAP_END
};


static SvXMLTokenMapEntry aPresElemTokenMap[] =
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

static SvXMLTokenMapEntry aPresScriptEmptyElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_MPRESCRIPTS,   XML_TOK_MPRESCRIPTS },
    { XML_NAMESPACE_MATH,   XML_NONE,  XML_TOK_NONE },
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aPresTableElemTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_MTR,       XML_TOK_MTR },
    { XML_NAMESPACE_MATH,   XML_MTD,       XML_TOK_MTD },
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aColorTokenMap[] =
{
    { XML_NAMESPACE_MATH,   XML_BLACK,        TBLACK},
    { XML_NAMESPACE_MATH,   XML_WHITE,        TWHITE},
    { XML_NAMESPACE_MATH,   XML_RED,          TRED},
    { XML_NAMESPACE_MATH,   XML_GREEN,        TGREEN},
    { XML_NAMESPACE_MATH,   XML_BLUE,         TBLUE},
    { XML_NAMESPACE_MATH,   XML_AQUA,         TCYAN},
    { XML_NAMESPACE_MATH,   XML_FUCHSIA,      TMAGENTA},
    { XML_NAMESPACE_MATH,   XML_YELLOW,       TYELLOW},
    XML_TOKEN_MAP_END
};


////////////////////////////////////////////////////////////

const SvXMLTokenMap& SmXMLImport::GetPresLayoutElemTokenMap()
{
    if (!pPresLayoutElemTokenMap)
        pPresLayoutElemTokenMap = new SvXMLTokenMap(aPresLayoutElemTokenMap);
    return *pPresLayoutElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresLayoutAttrTokenMap()
{
    if (!pPresLayoutAttrTokenMap)
        pPresLayoutAttrTokenMap = new SvXMLTokenMap(aPresLayoutAttrTokenMap);
    return *pPresLayoutAttrTokenMap;
}


const SvXMLTokenMap& SmXMLImport::GetFencedAttrTokenMap()
{
    if (!pFencedAttrTokenMap)
        pFencedAttrTokenMap = new SvXMLTokenMap(aFencedAttrTokenMap);
    return *pFencedAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetOperatorAttrTokenMap()
{
    if (!pOperatorAttrTokenMap)
        pOperatorAttrTokenMap = new SvXMLTokenMap(aOperatorAttrTokenMap);
    return *pOperatorAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetAnnotationAttrTokenMap()
{
    if (!pAnnotationAttrTokenMap)
        pAnnotationAttrTokenMap = new SvXMLTokenMap(aAnnotationAttrTokenMap);
    return *pAnnotationAttrTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresElemTokenMap()
{
    if (!pPresElemTokenMap)
        pPresElemTokenMap = new SvXMLTokenMap(aPresElemTokenMap);
    return *pPresElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresScriptEmptyElemTokenMap()
{
    if (!pPresScriptEmptyElemTokenMap)
        pPresScriptEmptyElemTokenMap = new
            SvXMLTokenMap(aPresScriptEmptyElemTokenMap);
    return *pPresScriptEmptyElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetPresTableElemTokenMap()
{
    if (!pPresTableElemTokenMap)
        pPresTableElemTokenMap = new SvXMLTokenMap(aPresTableElemTokenMap);
    return *pPresTableElemTokenMap;
}

const SvXMLTokenMap& SmXMLImport::GetColorTokenMap()
{
    if (!pColorTokenMap)
        pColorTokenMap = new SvXMLTokenMap(aColorTokenMap);
    return *pColorTokenMap;
}

////////////////////////////////////////////////////////////

SvXMLImportContext *SmXMLDocContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

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
            SmXMLRowContext_Impl aTempContext(GetSmImport(),nPrefix,
                GetXMLToken(XML_MROW));

            pContext = aTempContext.StrictCreateChildContext(nPrefix,
                rLocalName, xAttrList);
            break;
    }
    return pContext;
}

void SmXMLDocContext_Impl::EndElement()
{
    SmNodeArray ContextArray;
    ContextArray.resize(1);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();

    ContextArray[0] = lcl_popOrZero(rNodeStack);

    SmToken aDummy;
    SmStructureNode *pSNode = new SmLineNode(aDummy);
    pSNode->SetSubNodes(ContextArray);
    rNodeStack.push(pSNode);

    SmNodeArray  LineArray;
    sal_uLong n = rNodeStack.size();
    LineArray.resize(n);
    for (sal_uLong j = 0; j < n; j++)
    {
        LineArray[n - (j + 1)] = rNodeStack.top();
        rNodeStack.pop();
    }
    SmStructureNode *pSNode2 = new SmTableNode(aDummy);
    pSNode2->SetSubNodes(LineArray);
    rNodeStack.push(pSNode2);
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
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TOVER;
    SmStructureNode *pSNode = new SmBinVerNode(aToken);
    SmNode *pOper = new SmRectangleNode(aToken);
    SmNode *pSecond = lcl_popOrZero(rNodeStack);
    SmNode *pFirst = lcl_popOrZero(rNodeStack);
    pSNode->SetSubNodes(pFirst,pOper,pSecond);
    rNodeStack.push(pSNode);
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
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TNROOT;
    SmStructureNode *pSNode = new SmRootNode(aToken);
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNode *pIndex = lcl_popOrZero(rNodeStack);
    SmNode *pBase = lcl_popOrZero(rNodeStack);
    pSNode->SetSubNodes(pIndex,pOper,pBase);
    rNodeStack.push(pSNode);
}

void SmXMLSqrtContext_Impl::EndElement()
{
    /*
    <msqrt> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount > 1)
        SmXMLRowContext_Impl::EndElement();

    SmToken aToken;
    aToken.cMathChar = MS_SQRT;  //Temporary: alert, based on StarSymbol font
    aToken.nGroup = 0;
    aToken.nLevel = 0;
    aToken.eType = TSQRT;
    SmStructureNode *pSNode = new SmRootNode(aToken);
    SmNode *pOper = new SmRootSymbolNode(aToken);
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    pSNode->SetSubNodes(0,pOper,lcl_popOrZero(rNodeStack));
    rNodeStack.push(pSNode);
}

void SmXMLRowContext_Impl::EndElement()
{
    SmNodeArray aRelationArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    sal_uLong nSize = rNodeStack.size()-nElementCount;

    if (nSize > 0)
    {
        aRelationArray.resize(nSize);
        for (sal_uLong j=nSize;j > 0;j--)
        {
            aRelationArray[j-1] = rNodeStack.top();
            rNodeStack.pop();
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
            aToken.nGroup = 0;
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
            SmStructureNode *pSNode = new SmBraceNode(aToken);
            SmStructureNode *pBody = new SmExpressionNode(aDummy);
            pBody->SetSubNodes(aRelationArray2);

            pSNode->SetSubNodes(pLeft,pBody,pRight);
            pSNode->SetScaleMode(SCALE_HEIGHT);
            rNodeStack.push(pSNode);
            return;
        }
    }
    else //Multiple newlines result in empty row elements
    {
        aRelationArray.resize(1);
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 5;
        aToken.eType = TNEWLINE;
        aRelationArray[0] = new SmLineNode(aToken);
    }

    SmToken aDummy;
    SmStructureNode *pSNode = new SmExpressionNode(aDummy);
    pSNode->SetSubNodes(aRelationArray);
    rNodeStack.push(pSNode);
}


SvXMLImportContext *SmXMLRowContext_Impl::StrictCreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

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
         * (or decendants) are in a table*/
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
    SvXMLImportContext* pContext = 0L;

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

    sal_uLong nCount = rNodeStack.size() - nElementCount - 1;
    if (nCount == 0)
        return;

    if (nCount % 2 == 0)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.nGroup = 0;
        aToken.nLevel = 0;
        aToken.eType = bIsPrescript ? TLSUB : TRSUB;

        SmNodeStack aReverseStack;
        for (sal_uLong i = 0; i < nCount + 1; i++)
        {
            aReverseStack.push(rNodeStack.top());
            rNodeStack.pop();
        }

        SmSubSup eSub = bIsPrescript ? LSUB : RSUB;
        SmSubSup eSup = bIsPrescript ? LSUP : RSUP;

        for (sal_uLong i = 0; i < nCount; i += 2)
        {
            SmSubSupNode *pNode = new SmSubSupNode(aToken);

            // initialize subnodes array
            SmNodeArray aSubNodes(1 + SUBSUP_NUM_ENTRIES);

            /*On each loop the base and its sub sup pair becomes the
             base for the next loop to which the next sub sup pair is
             attached, i.e. wheels within wheels*/
            aSubNodes[0] = lcl_popOrZero(aReverseStack);

            SmNode *pScriptNode = lcl_popOrZero(aReverseStack);

            if (pScriptNode && ((pScriptNode->GetToken().eType != TIDENT) ||
                (pScriptNode->GetToken().aText.Len())))
                aSubNodes[eSub+1] = pScriptNode;
            pScriptNode = lcl_popOrZero(aReverseStack);
            if (pScriptNode && ((pScriptNode->GetToken().eType != TIDENT) ||
                (pScriptNode->GetToken().aText.Len())))
                aSubNodes[eSup+1] = pScriptNode;

            pNode->SetSubNodes(aSubNodes);
            aReverseStack.push(pNode);
        }
        rNodeStack.push(lcl_popOrZero(aReverseStack));
    }
    else
    {
        // Ignore odd number of elements.
        for (sal_uLong i = 0; i < nCount; i++)
        {
            delete rNodeStack.top();
            rNodeStack.pop();
        }
    }
}


void SmXMLTableContext_Impl::EndElement()
{
    SmNodeArray aExpressionArray;
    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    SmNodeStack aReverseStack;
    aExpressionArray.resize(rNodeStack.size()-nElementCount);

    sal_uLong nRows = rNodeStack.size()-nElementCount;
    sal_uInt16 nCols = 0;

    SmStructureNode *pArray;
    for (sal_uLong i=nRows;i > 0;i--)
    {
        pArray = (SmStructureNode *)rNodeStack.top();
        rNodeStack.pop();
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
        aReverseStack.push(pArray);
    }
    aExpressionArray.resize(nCols*nRows);
    sal_uLong j=0;
    while ( !aReverseStack.empty() )
    {
        pArray = (SmStructureNode *)aReverseStack.top();
        aReverseStack.pop();
        for (sal_uInt16 i=0;i<pArray->GetNumSubNodes();i++)
            aExpressionArray[j++] = pArray->GetSubNode(i);
    }

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nGroup = TRGROUP;
    aToken.nLevel = 0;
    aToken.eType = TMATRIX;
    SmMatrixNode *pSNode = new SmMatrixNode(aToken);
    pSNode->SetSubNodes(aExpressionArray);
    pSNode->SetRowCol(static_cast<sal_uInt16>(nRows),nCols);
    rNodeStack.push(pSNode);
}

SvXMLImportContext *SmXMLTableRowContext_Impl::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0L;

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
    SvXMLImportContext* pContext = 0L;

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

void SmXMLActionContext_Impl::EndElement()
{
    /*For now we will just assume that the
     selected attribute is one, and then just display
     that expression alone, i.e. remove all expect the
     first pushed one*/

    SmNodeStack &rNodeStack = GetSmImport().GetNodeStack();
    for (sal_uLong i=rNodeStack.size()-nElementCount;i > 1;i--)
    {
        delete rNodeStack.top();
        rNodeStack.pop();
    }
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
    delete pPresLayoutElemTokenMap;
    delete pPresElemTokenMap;
    delete pPresScriptEmptyElemTokenMap;
    delete pPresTableElemTokenMap;
    delete pPresLayoutAttrTokenMap;
    delete pFencedAttrTokenMap;
    delete pColorTokenMap;
    delete pOperatorAttrTokenMap;
    delete pAnnotationAttrTokenMap;
}

void SmXMLImport::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
{
    uno::Reference <frame::XModel> xModel = GetModel();
    if ( !xModel.is() )
        return;

    uno::Reference <lang::XUnoTunnel> xTunnel;
    xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
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
                    catch( Exception& )
                    {
                        OSL_FAIL( "SmXMLImport::SetConfigurationSettings: Exception!" );
                    }
                }

                pValues++;
            }
        }
    }
}


////////////////////////////////////////////////////////////


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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


/*
 Warning: The SvXMLElementExport helper class creates the beginning and
 closing tags of xml elements in its constructor and destructor, so theres
 hidden stuff going on, on occasion the ordering of these classes declarations
 may be significant
*/


#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/uno/Any.h>

#include <rtl/math.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <osl/diagnose.h>
#include <svtools/sfxecode.hxx>
#include <unotools/saveopt.hxx>
#include <svl/stritem.hxx>
#include <svl/itemprop.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmlmetai.hxx>
#include <osl/mutex.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/servicehelper.hxx>

#include <memory>

#include "mathmlexport.hxx"
#include <starmath.hrc>
#include <unomodel.hxx>
#include <document.hxx>
#include <utility.hxx>
#include <config.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#define EXPORT_SVC_NAME "com.sun.star.xml.XMLExportFilter"

#undef WANTEXCEPT


////////////////////////////////////////////////////////////

sal_Bool SmXMLExportWrapper::Export(SfxMedium &rMedium)
{
    sal_Bool bRet=sal_True;
    uno::Reference<lang::XMultiServiceFactory>
        xServiceFactory(comphelper::getProcessServiceFactory());
    OSL_ENSURE(xServiceFactory.is(),"got no service manager");

    //Get model
    uno::Reference< lang::XComponent > xModelComp(xModel, uno::UNO_QUERY );

    sal_Bool bEmbedded = sal_False;
    uno::Reference <lang::XUnoTunnel> xTunnel;
    xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    SmDocShell *pDocShell = pModel ?
            static_cast<SmDocShell*>(pModel->GetObjectShell()) : 0;
    if ( pDocShell &&
        SFX_CREATE_MODE_EMBEDDED == pDocShell->GetCreateMode() )
        bEmbedded = sal_True;

    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (!bEmbedded)
    {
        if (pDocShell /*&& pDocShell->GetMedium()*/)
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
        }

        // set progress range and start status indicator
        if (xStatusIndicator.is())
        {
            sal_Int32 nProgressRange = bFlat ? 1 : 3;
            xStatusIndicator->start(SM_RESSTR(STR_STATSTR_WRITING),
                nProgressRange);
        }
    }


    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { "UsePrettyPrinting", sizeof("UsePrettyPrinting")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0},
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

    SvtSaveOptions aSaveOpt;
    OUString sUsePrettyPrinting("UsePrettyPrinting");
    sal_Bool bUsePrettyPrinting( bFlat || aSaveOpt.IsPrettyPrinting() );
    Any aAny;
    aAny.setValue( &bUsePrettyPrinting, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sUsePrettyPrinting, aAny );

    // Set base URI
    OUString sPropName( "BaseURI" );
    xInfoSet->setPropertyValue( sPropName, makeAny( rMedium.GetBaseURL( true ) ) );

    sal_Int32 nSteps=0;
    if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);
    if (!bFlat) //Storage (Package) of Stream
    {
        uno::Reference < embed::XStorage > xStg = rMedium.GetOutputStorage();
        sal_Bool bOASIS = ( SotStorage::GetVersion( xStg ) > SOFFICE_FILEFORMAT_60 );

        // TODO/LATER: handle the case of embedded links gracefully
        if ( bEmbedded ) //&& !pStg->IsRoot() )
        {
            OUString aName;
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

        if ( !bEmbedded )
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(
                    xStg, xModelComp, "meta.xml", xServiceFactory, xInfoSet,
                    (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaExporter"
                            : "com.sun.star.comp.Math.XMLMetaExporter"),
                    sal_False);
        }
        if ( bRet )
        {
           if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(
                    xStg, xModelComp, "content.xml", xServiceFactory, xInfoSet,
                    "com.sun.star.comp.Math.XMLContentExporter");
        }

        if ( bRet )
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(
                    xStg, xModelComp, "settings.xml", xServiceFactory, xInfoSet,
                    (bOASIS ? "com.sun.star.comp.Math.XMLOasisSettingsExporter"
                            : "com.sun.star.comp.Math.XMLSettingsExporter") );
        }
    }
    else
    {
        SvStream *pStream = rMedium.GetOutStream();
        uno::Reference<io::XOutputStream> xOut(
            new utl::OOutputStreamWrapper(*pStream) );

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        bRet = WriteThroughComponent(
            xOut, xModelComp, xServiceFactory, xInfoSet,
            "com.sun.star.comp.Math.XMLContentExporter");
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();

    return bRet;
}


/// export through an XML exporter component (output stream version)
sal_Bool SmXMLExportWrapper::WriteThroughComponent(
    Reference<io::XOutputStream> xOutputStream,
    Reference<XComponent> xComponent,
    Reference<lang::XMultiServiceFactory> & rFactory,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pComponentName )
{
    OSL_ENSURE(xOutputStream.is(), "I really need an output stream!");
    OSL_ENSURE(xComponent.is(), "Need component!");
    OSL_ENSURE(NULL != pComponentName, "Need component name!");

    // get component
    Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(
        comphelper::getComponentContext(rFactory) );
    OSL_ENSURE( xSaxWriter.is(), "can't instantiate XML writer" );
    if (!xSaxWriter.is())
        return sal_False;

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference<xml::sax::XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);

    Sequence<Any> aArgs( 2 );
    aArgs[0] <<= xDocHandler;
    aArgs[1] <<= rPropSet;

    // get filter component
    Reference< document::XExporter > xExporter(
        rFactory->createInstanceWithArguments(
            OUString::createFromAscii(pComponentName), aArgs), UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if ( !xExporter.is() )
        return sal_False;


    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    Reference < XFilter > xFilter( xExporter, UNO_QUERY );
    uno::Sequence< PropertyValue > aProps(0);
    xFilter->filter( aProps );

    uno::Reference<lang::XUnoTunnel> xFilterTunnel;
    xFilterTunnel = uno::Reference<lang::XUnoTunnel>
        ( xFilter, uno::UNO_QUERY );
    SmXMLExport *pFilter = reinterpret_cast< SmXMLExport * >(
                sal::static_int_cast< sal_uIntPtr >(
                xFilterTunnel->getSomething( SmXMLExport::getUnoTunnelId() )));
    return pFilter ? pFilter->GetSuccess() : sal_True;
}


/// export through an XML exporter component (storage version)
sal_Bool SmXMLExportWrapper::WriteThroughComponent(
    const Reference < embed::XStorage >& xStorage,
    Reference<XComponent> xComponent,
    const sal_Char* pStreamName,
    Reference<lang::XMultiServiceFactory> & rFactory,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pComponentName,
    sal_Bool bCompress
    )
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(NULL != pStreamName, "Need stream name!");

    // open stream
    Reference < io::XStream > xStream;
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    try
    {
        xStream = xStorage->openStreamElement( sStreamName,
            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "Can't create output stream in package!" );
        return sal_False;
    }

    OUString aPropName( "MediaType" );
    OUString aMime( "text/xml" );
    uno::Any aAny;
    aAny <<= aMime;

    uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
    xSet->setPropertyValue( aPropName, aAny );

    if ( !bCompress )
    {
        aPropName = "Compressed";
        sal_Bool bFalse = sal_False;
        aAny.setValue( &bFalse, ::getBooleanCppuType() );
        xSet->setPropertyValue( aPropName, aAny );
    }

    // even plain stream must be encrypted in encrypted document
    OUString aTmpPropName( "UseCommonStoragePasswordEncryption" );
    sal_Bool bTrue = sal_True;
    aAny.setValue( &bTrue, ::getBooleanCppuType() );
    xSet->setPropertyValue( aTmpPropName, aAny );

    // set Base URL
    if ( rPropSet.is() )
    {
        OUString sPropName( "StreamName" );
        rPropSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
    }

    // write the stuff
    sal_Bool bRet = WriteThroughComponent( xStream->getOutputStream(), xComponent, rFactory,
        rPropSet, pComponentName );

    return bRet;
}

////////////////////////////////////////////////////////////

SmXMLExport::SmXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    sal_uInt16 nExportFlags)
:   SvXMLExport(util::MeasureUnit::INCH, xServiceFactory, XML_MATH,
                nExportFlags)
,   pTree(0) ,
    bSuccess(sal_False)
{
}

sal_Int64 SAL_CALL SmXMLExport::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
        0 == memcmp( getUnoTunnelId().getConstArray(),
        rId.getConstArray(), 16 ) )
    return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_uIntPtr >(this));

    return SvXMLExport::getSomething( rId );
}

namespace
{
    class theSmXMLExportUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSmXMLExportUnoTunnelId> {};
}

const uno::Sequence< sal_Int8 > & SmXMLExport::getUnoTunnelId() throw()
{
    return theSmXMLExportUnoTunnelId::get().getSeq();
}

OUString SAL_CALL SmXMLExport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExport_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExport_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // EXPORT_OASIS is required here allthough there is no differrence between
    // OOo and OASIS, because without the flag, a transformation to OOo would
    // be chained in.
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_OASIS|EXPORT_ALL );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLExportMetaOOO_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLMetaExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExportMetaOOO_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExportMetaOOO_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_META );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLExportMeta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisMetaExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExportMeta_getSupportedServiceNames()
throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExportMeta_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_OASIS|EXPORT_META );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLExportSettingsOOO_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLSettingsExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExportSettingsOOO_getSupportedServiceNames()
throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExportSettingsOOO_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_SETTINGS );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLExportSettings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLOasisSettingsExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExportSettings_getSupportedServiceNames()
throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExportSettings_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_OASIS|EXPORT_SETTINGS );
}

////////////////////////////////////////////////////////////

OUString SAL_CALL SmXMLExportContent_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.XMLContentExporter" );
}

uno::Sequence< OUString > SAL_CALL SmXMLExportContent_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( EXPORT_SVC_NAME );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmXMLExportContent_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
throw( uno::Exception )
{
    // The EXPORT_OASIS flag is only required to avoid that a transformer is
    // chanied in
    return (cppu::OWeakObject*)new SmXMLExport( rSMgr, EXPORT_OASIS|EXPORT_CONTENT );
}

////////////////////////////////////////////////////////////

// XServiceInfo
// override empty method from parent class
rtl::OUString SAL_CALL SmXMLExport::getImplementationName()
throw(uno::RuntimeException)
{
    OUString aTxt;
    switch( getExportFlags() )
    {
        case EXPORT_META:
            aTxt = SmXMLExportMeta_getImplementationName();
            break;
        case EXPORT_SETTINGS:
            aTxt = SmXMLExportSettings_getImplementationName();
            break;
        case EXPORT_CONTENT:
            aTxt = SmXMLExportContent_getImplementationName();
            break;
        case EXPORT_ALL:
        default:
            aTxt = SmXMLExport_getImplementationName();
            break;
    }
    return aTxt;
}

sal_uInt32 SmXMLExport::exportDoc(enum XMLTokenEnum eClass)
{
    if ( (getExportFlags() & EXPORT_CONTENT) == 0 )
    {
        SvXMLExport::exportDoc( eClass );
    }
    else
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
            pTree = pDocShell->GetFormulaTree();
            aText = pDocShell->GetText();
        }

        GetDocHandler()->startDocument();

        addChaffWhenEncryptedStorage();

        /*Add xmlns line*/
        SvXMLAttributeList &rList = GetAttrList();

        // make use of a default namespace
        ResetNamespaceMap();    // Math doesn't need namespaces from xmloff, since it now uses default namespaces (because that is common with current MathML usage in the web)
        _GetNamespaceMap().Add( OUString(), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );

        rList.AddAttribute(GetNamespaceMap().GetAttrNameByKey(XML_NAMESPACE_MATH_IDX),
                GetNamespaceMap().GetNameByKey( XML_NAMESPACE_MATH_IDX));

        //I think we need something like ImplExportEntities();
        _ExportContent();
        GetDocHandler()->endDocument();
    }

    bSuccess=sal_True;
    return 0;
}

void SmXMLExport::_ExportContent()
{
    SvXMLElementExport aEquation(*this, XML_NAMESPACE_MATH, XML_MATH, sal_True, sal_True);
    SvXMLElementExport *pSemantics=0;

    if (aText.Len())
    {
        pSemantics = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
            XML_SEMANTICS, sal_True, sal_True);
    }

    ExportNodes(pTree, 0);

    if (aText.Len())
    {
        // Convert symbol names
        uno::Reference <frame::XModel> xModel = GetModel();
        uno::Reference <lang::XUnoTunnel> xTunnel;
        xTunnel = uno::Reference <lang::XUnoTunnel> (xModel,uno::UNO_QUERY);
        SmModel *pModel = reinterpret_cast<SmModel *>
            (xTunnel->getSomething(SmModel::getUnoTunnelId()));
        SmDocShell *pDocShell = pModel ?
            static_cast<SmDocShell*>(pModel->GetObjectShell()) : 0;
        OSL_ENSURE( pDocShell, "doc shell missing" );
        if (pDocShell)
        {
            SmParser &rParser = pDocShell->GetParser();
            bool bVal = rParser.IsExportSymbolNames();
            rParser.SetExportSymbolNames( true );
            SmNode *pTmpTree = rParser.Parse( aText );
            aText = rParser.GetText();
            delete pTmpTree;
            rParser.SetExportSymbolNames( bVal );
        }

        AddAttribute(XML_NAMESPACE_MATH, XML_ENCODING,
            OUString("StarMath 5.0"));
        SvXMLElementExport aAnnotation(*this, XML_NAMESPACE_MATH,
            XML_ANNOTATION, sal_True, sal_False);
        GetDocHandler()->characters(OUString( aText ));
    }
    delete pSemantics;
}

void SmXMLExport::GetViewSettings( Sequence < PropertyValue >& aProps)
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

    aProps.realloc( 4 );
    PropertyValue *pValue = aProps.getArray();
    sal_Int32 nIndex = 0;

    Rectangle aRect( pDocShell->GetVisArea() );

    pValue[nIndex].Name = "ViewAreaTop";
    pValue[nIndex++].Value <<= aRect.Top();

    pValue[nIndex].Name = "ViewAreaLeft";
    pValue[nIndex++].Value <<= aRect.Left();

    pValue[nIndex++].Value <<= aRect.GetWidth();

    pValue[nIndex].Name = "ViewAreaHeight";
    pValue[nIndex++].Value <<= aRect.GetHeight();
}

void SmXMLExport::GetConfigurationSettings( Sequence < PropertyValue > & rProps)
{
    Reference < XPropertySet > xProps ( GetModel(), UNO_QUERY );
    if ( xProps.is() )
    {
        Reference< XPropertySetInfo > xPropertySetInfo = xProps->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            Sequence< Property > aProps = xPropertySetInfo->getProperties();
            sal_Int32 nCount(aProps.getLength());
            if (nCount > 0)
            {
                rProps.realloc(nCount);
                PropertyValue* pProps = rProps.getArray();
                if (pProps)
                {
                    SmConfig *pConfig = SM_MOD()->GetConfig();
                    const bool bUsedSymbolsOnly = pConfig ? pConfig->IsSaveOnlyUsedSymbols() : false;

                    const OUString sFormula ( "Formula" );
                    const OUString sBasicLibraries ( "BasicLibraries" );
                    const OUString sDialogLibraries ( "DialogLibraries" );
                    const OUString sRuntimeUID ( "RuntimeUID" );
                    for (sal_Int32 i = 0; i < nCount; i++, pProps++)
                    {
                        const OUString &rPropName = aProps[i].Name;
                        if (rPropName != sFormula &&
                            rPropName != sBasicLibraries &&
                            rPropName != sDialogLibraries &&
                            rPropName != sRuntimeUID)
                        {
                            pProps->Name = rPropName;

                            rtl::OUString aActualName( rPropName );

                            // handle 'save used symbols only'
                            if (bUsedSymbolsOnly && rPropName == "Symbols" )
                                aActualName = "UserDefinedSymbolsInUse";

                            pProps->Value = xProps->getPropertyValue( aActualName );
                        }
                    }
                }
            }
        }
    }
}

void SmXMLExport::ExportLine(const SmNode *pNode, int nLevel)
{
    ExportExpression(pNode, nLevel);
}

void SmXMLExport::ExportBinaryHorizontal(const SmNode *pNode, int nLevel)
{
    ExportExpression(pNode, nLevel);
}

void SmXMLExport::ExportUnaryHorizontal(const SmNode *pNode, int nLevel)
{
    ExportExpression(pNode, nLevel);
}

void SmXMLExport::ExportExpression(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pRow=0;
    sal_uLong  nSize = pNode->GetNumSubNodes();

    // #i115443: nodes of type expression always need to be grouped with mrow statement
    if (nSize > 1 || (pNode && pNode->GetType() == NEXPRESSION))
        pRow = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MROW, sal_True, sal_True);

        for (sal_uInt16 i = 0; i < nSize; i++)
        if (const SmNode *pTemp = pNode->GetSubNode(i))
            ExportNodes(pTemp, nLevel+1);

    delete pRow;
}

void SmXMLExport::ExportBinaryVertical(const SmNode *pNode, int nLevel)
{
    OSL_ENSURE(pNode->GetNumSubNodes()==3,"Bad Fraction");
    SvXMLElementExport aFraction(*this, XML_NAMESPACE_MATH, XML_MFRAC, sal_True, sal_True);
    ExportNodes(pNode->GetSubNode(0), nLevel);
    ExportNodes(pNode->GetSubNode(2), nLevel);
}

void SmXMLExport::ExportTable(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pTable=0;

    sal_uInt16 nSize = pNode->GetNumSubNodes();

    //If the list ends in newline then the last entry has
    //no subnodes, the newline is superfulous so we just drop
    //the last node, inclusion would create a bad MathML
    //table
    if (nSize >= 1 && pNode->GetSubNode(nSize-1)->GetNumSubNodes() == 0)
        --nSize;

    // try to avoid creating a mtable element when the formula consists only
    // of a single output line
    if (nLevel || (nSize >1))
        pTable = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTABLE, sal_True, sal_True);

    for (sal_uInt16 i = 0; i < nSize; i++)
        if (const SmNode *pTemp = pNode->GetSubNode(i))
        {
            SvXMLElementExport *pRow=0;
            SvXMLElementExport *pCell=0;
            if (pTable)
            {
                pRow  = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTR, sal_True, sal_True);
                pCell = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTD, sal_True, sal_True);
            }
            ExportNodes(pTemp, nLevel+1);
            delete pCell;
            delete pRow;
        }

    delete pTable;
}

void SmXMLExport::ExportMath(const SmNode *pNode, int /*nLevel*/)
{
    const SmMathSymbolNode *pTemp = static_cast<const SmMathSymbolNode *>(pNode);
    SvXMLElementExport aMath(*this, XML_NAMESPACE_MATH, XML_MO, sal_True, sal_False);
    sal_Unicode nArse[2];
    nArse[0] = pTemp->GetText()[0];
    sal_Unicode cTmp = ConvertMathToMathML( nArse[0] );
    if (cTmp != 0)
        nArse[0] = cTmp;
    OSL_ENSURE(nArse[0] != 0xffff,"Non existent symbol");
    nArse[1] = 0;
    GetDocHandler()->characters(nArse);
}

void SmXMLExport::ExportText(const SmNode *pNode, int /*nLevel*/)
{
    SvXMLElementExport *pText;
    const SmTextNode *pTemp = static_cast<const SmTextNode *>(pNode);
    switch (pNode->GetToken().eType)
    {
        default:
        case TIDENT:
        {
            //Note that we change the fontstyle to italic for strings that
            //are italic and longer than a single character.
            sal_Bool bIsItalic = IsItalic( pTemp->GetFont() );
            if ((pTemp->GetText().getLength() > 1) && bIsItalic)
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, XML_ITALIC);
            else if ((pTemp->GetText().getLength() == 1) && !bIsItalic)
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, XML_NORMAL);
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MI,sal_True,sal_False);
            break;
        }
        case TNUMBER:
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MN,sal_True,sal_False);
            break;
        case TTEXT:
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTEXT,sal_True,sal_False);
            break;
        }
    GetDocHandler()->characters(pTemp->GetText());
    delete pText;
}

void SmXMLExport::ExportBlank(const SmNode * /*pNode*/, int /*nLevel*/)
{
    //!! exports an empty <mi> tag since for example "~_~" is allowed in
    //!! Math (so it has no sense at all) but must not result in an empty
    //!! <msub> tag in MathML !!

    SvXMLElementExport *pText;

    pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MI, sal_True, sal_False);

    GetDocHandler()->characters( OUString() );
    delete pText;
}

void SmXMLExport::ExportSubSupScript(const SmNode *pNode, int nLevel)
{
    const SmNode *pSub  = 0;
    const SmNode *pSup  = 0;
    const SmNode *pCSub = 0;
    const SmNode *pCSup = 0;
    const SmNode *pLSub = 0;
    const SmNode *pLSup = 0;
    SvXMLElementExport *pThing = 0, *pThing2 = 0;

    //if we have prescripts at all then we must use the tensor notation

    //This is one of those excellent locations where scope is vital to
    //arrange the construction and destruction of the element helper
    //classes correctly
    pLSub = pNode->GetSubNode(LSUB+1);
    pLSup = pNode->GetSubNode(LSUP+1);
    if (pLSub || pLSup)
    {
        SvXMLElementExport aMultiScripts(*this, XML_NAMESPACE_MATH,
            XML_MMULTISCRIPTS, sal_True, sal_True);


        if (NULL != (pCSub = pNode->GetSubNode(CSUB+1))
            && NULL != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDEROVER, sal_True,sal_True);
        }
        else if (NULL != (pCSub = pNode->GetSubNode(CSUB+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDER, sal_True,sal_True);
        }
        else if (NULL != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MOVER, sal_True,sal_True);
        }

        ExportNodes(pNode->GetSubNode(0), nLevel+1);    //Main Term

        if (pCSub)
            ExportNodes(pCSub, nLevel+1);
        if (pCSup)
            ExportNodes(pCSup, nLevel+1);
        delete pThing2;

        pSub = pNode->GetSubNode(RSUB+1);
        pSup = pNode->GetSubNode(RSUP+1);
        if (pSub || pSup)
        {
            if (pSub)
                ExportNodes(pSub, nLevel+1);
            else
            {
                SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,sal_True,sal_True);
            }
            if (pSup)
                ExportNodes(pSup, nLevel+1);
            else
            {
                SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,sal_True,sal_True);
            }
        }

        //Seperator element between suffix and prefix sub/sup pairs
        {
            SvXMLElementExport aPrescripts(*this, XML_NAMESPACE_MATH,
                XML_MPRESCRIPTS, sal_True,sal_True);
        }

        if (pLSub)
            ExportNodes(pLSub, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,
                sal_True,sal_True);

        }
        if (pLSup)
            ExportNodes(pLSup, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,
                sal_True,sal_True);

        }
    }
    else
    {
        if (NULL != (pSub = pNode->GetSubNode(RSUB+1)) &&
            NULL != (pSup = pNode->GetSubNode(RSUP+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MSUBSUP, sal_True,sal_True);
        }
        else if (NULL != (pSub = pNode->GetSubNode(RSUB+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MSUB,
                sal_True,sal_True);
        }
        else if (NULL != (pSup = pNode->GetSubNode(RSUP+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MSUP,
                sal_True,sal_True);
        }

        if (NULL != (pCSub = pNode->GetSubNode(CSUB+1))
            && NULL != (pCSup=pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDEROVER, sal_True,sal_True);
        }
        else if (NULL != (pCSub = pNode->GetSubNode(CSUB+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDER, sal_True,sal_True);
        }
        else if (NULL != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MOVER, sal_True,sal_True);
        }
        ExportNodes(pNode->GetSubNode(0), nLevel+1);    //Main Term

        if (pCSub)
            ExportNodes(pCSub, nLevel+1);
        if (pCSup)
            ExportNodes(pCSup, nLevel+1);
        delete pThing2;

        if (pSub)
            ExportNodes(pSub, nLevel+1);
        if (pSup)
            ExportNodes(pSup, nLevel+1);
        delete pThing;
    }
}

void SmXMLExport::ExportBrace(const SmNode *pNode, int nLevel)
{
    const SmNode *pTemp;
    const SmNode *pLeft=pNode->GetSubNode(0);
    const SmNode *pRight=pNode->GetSubNode(2);
    SvXMLElementExport *pFences=0,*pRow=0;
    if ( ((pLeft) && (pLeft->GetToken().eType != TNONE)) &&
        ((pRight) && (pRight->GetToken().eType != TNONE)) &&
        (pNode->GetScaleMode() == SCALE_HEIGHT))
    {
        sal_Unicode nArse[2];
        nArse[1] = 0;
        nArse[0] = static_cast<
            const SmMathSymbolNode* >(pLeft)->GetText()[0];
        OSL_ENSURE(nArse[0] != 0xffff,"Non existent symbol");
        AddAttribute(XML_NAMESPACE_MATH, XML_OPEN,nArse);
        nArse[0] = static_cast<
            const SmMathSymbolNode* >(pRight)->GetText()[0];
        OSL_ENSURE(nArse[0] != 0xffff,"Non existent symbol");
        AddAttribute(XML_NAMESPACE_MATH, XML_CLOSE,nArse);
        pFences = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MFENCED,
            sal_True,sal_True);
    }
    else if (pLeft && (pLeft->GetToken().eType != TNONE))
    {
        pRow = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MROW,
            sal_True, sal_True);
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
        else
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
        ExportNodes(pLeft, nLevel+1);
    }
    else
        pRow = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MROW,
            sal_True, sal_True);

    if (NULL != (pTemp = pNode->GetSubNode(1)))
        ExportNodes(pTemp, nLevel+1);
    if (pFences)
        delete pFences;
    else if (pRight && (pRight->GetToken().eType != TNONE))
    {
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
        else
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
        ExportNodes(pRight, nLevel+1);
    }
    delete pRow;
}

void SmXMLExport::ExportRoot(const SmNode *pNode, int nLevel)
{
    if (pNode->GetSubNode(0))
    {
        SvXMLElementExport aRoot(*this, XML_NAMESPACE_MATH, XML_MROOT,sal_True,
            sal_True);
        ExportNodes(pNode->GetSubNode(2), nLevel+1);
        ExportNodes(pNode->GetSubNode(0), nLevel+1);
    }
    else
    {
        SvXMLElementExport aSqrt(*this, XML_NAMESPACE_MATH, XML_MSQRT,sal_True,
            sal_True);
        ExportNodes(pNode->GetSubNode(2), nLevel+1);
    }
}

void SmXMLExport::ExportOperator(const SmNode *pNode, int nLevel)
{
    /*we need to either use content or font and size attributes
     *here*/
    SvXMLElementExport aRow(*this, XML_NAMESPACE_MATH, XML_MROW,
        sal_True, sal_True);
    ExportNodes(pNode->GetSubNode(0), nLevel+1);
    ExportNodes(pNode->GetSubNode(1), nLevel+1);
}

void SmXMLExport::ExportAttributes(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pElement=0;

    if (pNode->GetToken().eType == TUNDERLINE)
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENTUNDER,
            XML_TRUE);
        pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MUNDER,
            sal_True,sal_True);
    }
    else if (pNode->GetToken().eType != TOVERSTRIKE)
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENT,
            XML_TRUE);
        pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MOVER,
            sal_True,sal_True);
    }

    ExportNodes(pNode->GetSubNode(1), nLevel+1);
    switch (pNode->GetToken().eType)
    {
        case TOVERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this, XML_NAMESPACE_MATH, XML_MO,
                sal_True,sal_True);
            sal_Unicode nArse[2] = {0xAF,0x00};
            GetDocHandler()->characters(nArse);
            }
            break;
        case TUNDERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this, XML_NAMESPACE_MATH, XML_MO,
                sal_True,sal_True);
            sal_Unicode nArse[2] = {0x0332,0x00};
            GetDocHandler()->characters(nArse);
            }
            break;
        case TOVERSTRIKE:
            break;
        default:
            ExportNodes(pNode->GetSubNode(0), nLevel+1);
            break;
    }
    delete pElement;
}

static bool lcl_HasEffectOnMathvariant( const SmTokenType eType )
{
    return  eType == TBOLD || eType == TNBOLD ||
            eType == TITALIC || eType == TNITALIC ||
            eType == TSANS || eType == TSERIF || eType == TFIXED;
}

void SmXMLExport::ExportFont(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pElement = 0;

    //
    // gather the mathvariant attribut relevant data from all
    // successively following SmFontNodes...
    //
    int nBold   = -1;   // for the following variables: -1 = yet undefined; 0 = false; 1 = true;
    int nItalic = -1;   // for the following variables: -1 = yet undefined; 0 = false; 1 = true;
    int nSansSerifFixed   = -1;
    SmTokenType eNodeType = TUNKNOWN;
    while (lcl_HasEffectOnMathvariant( (eNodeType = pNode->GetToken().eType) ))
    {
        switch (eNodeType)
        {
            case TBOLD      : nBold   = 1; break;
            case TNBOLD     : nBold   = 0; break;
            case TITALIC    : nItalic = 1; break;
            case TNITALIC   : nItalic = 0; break;
            case TSANS      : nSansSerifFixed  = 0; break;
            case TSERIF     : nSansSerifFixed  = 1; break;
            case TFIXED     : nSansSerifFixed  = 2; break;
            default:
                OSL_FAIL( "unexpected case" );
        }
        // According to the parser every node that is to be evaluated heres
        // has a single non-zero subnode at index 1!! Thus we only need to check
        // that single node for follow-up nodes that have an effect on the attribute.
        if (pNode->GetNumSubNodes() > 1 && pNode->GetSubNode(1) &&
            lcl_HasEffectOnMathvariant( pNode->GetSubNode(1)->GetToken().eType))
        {
            pNode = pNode->GetSubNode(1);
        }
        else
            break;
    }

    switch (pNode->GetToken().eType)
    {
        //wrap a phantom element around everything*/
        case TPHANTOM:
            pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MPHANTOM, sal_True,sal_True);
            break;
        case TBLACK:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_BLACK);
            break;
        case TWHITE:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_WHITE);
            break;
        case TRED:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_RED);
            break;
        case TGREEN:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_GREEN);
            break;
        case TBLUE:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_BLUE);
            break;
        case TCYAN:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_AQUA);
            break;
        case TMAGENTA:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_FUCHSIA);
            break;
        case TYELLOW:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_YELLOW);
            break;
        case TSIZE:
            {
                const SmFontNode *pFontNode = static_cast<const SmFontNode *>(pNode);
                const Fraction &aFrac = pFontNode->GetSizeParameter();

                OUStringBuffer sStrBuf;
                switch(pFontNode->GetSizeType())
                {
                    case FNTSIZ_MULTIPLY:
                        ::sax::Converter::convertDouble(sStrBuf,
                            static_cast<double>(aFrac*Fraction(100.00)));
                        sStrBuf.append(static_cast<sal_Unicode>('%'));
                        break;
                    case FNTSIZ_DIVIDE:
                        ::sax::Converter::convertDouble(sStrBuf,
                            static_cast<double>(Fraction(100.00)/aFrac));
                        sStrBuf.append(static_cast<sal_Unicode>('%'));
                        break;
                    case FNTSIZ_ABSOLUT:
                        ::sax::Converter::convertDouble(sStrBuf,
                            static_cast<double>(aFrac));
                        sStrBuf.append(
                            GetXMLToken(XML_UNIT_PT));
                        break;
                    default:
                        {
                            //The problem here is that the wheels fall off because
                            //font size is stored in 100th's of a mm not pts, and
                            //rounding errors take their toll on the original
                            //value specified in points.

                            //Must fix StarMath to retain the original pt values
                            Fraction aTemp = Sm100th_mmToPts(pFontNode->GetFont().
                                GetSize().Height());

                            if (pFontNode->GetSizeType() == FNTSIZ_MINUS)
                                aTemp-=aFrac;
                            else
                                aTemp+=aFrac;

                            double mytest = static_cast<double>(aTemp);

                            mytest = ::rtl::math::round(mytest,1);
                            ::sax::Converter::convertDouble(sStrBuf,mytest);
                            sStrBuf.append(GetXMLToken(XML_UNIT_PT));
                        }
                        break;
                }

                OUString sStr(sStrBuf.makeStringAndClear());
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHSIZE, sStr);
            }
            break;
        case TBOLD:
        case TITALIC:
        case TNBOLD:
        case TNITALIC:
        case TFIXED:
        case TSANS:
        case TSERIF:
            {
                // nBold:   -1 = yet undefined; 0 = false; 1 = true;
                // nItalic: -1 = yet undefined; 0 = false; 1 = true;
                // nSansSerifFixed: -1 = undefined; 0 = sans; 1 = serif; 2 = fixed;
                const sal_Char *pText = "normal";
                if (nSansSerifFixed == -1 || nSansSerifFixed == 1)
                {
                    pText = "normal";
                    if (nBold == 1 && nItalic != 1)
                        pText = "bold";
                    else if (nBold != 1 && nItalic == 1)
                        pText = "italic";
                    else if (nBold == 1 && nItalic == 1)
                        pText = "bold-italic";
                }
                else if (nSansSerifFixed == 0)
                {
                    pText = "sans-serif";
                    if (nBold == 1 && nItalic != 1)
                        pText = "bold-sans-serif";
                    else if (nBold != 1 && nItalic == 1)
                        pText = "sans-serif-italic";
                    else if (nBold == 1 && nItalic == 1)
                        pText = "sans-serif-bold-italic";
                }
                else if (nSansSerifFixed == 2)
                    pText = "monospace";    // no modifiers allowed for monospace ...
                else
                {
                    OSL_FAIL( "unexpected case" );
                }
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, OUString::createFromAscii( pText ));
            }
            break;
        default:
            break;

    }
    //for now we will just always export with a style and not worry about
    //anyone else for the moment.
    {
        //wrap a style around it
        SvXMLElementExport aStyle(*this, XML_NAMESPACE_MATH, XML_MSTYLE, sal_True,sal_True);
        ExportExpression(pNode, nLevel);
    }

    delete pElement;
}


void SmXMLExport::ExportVerticalBrace(const SmNode *pNode, int nLevel)
{
    //Place the overbrace value OVER a vertical brace and then place that
    //expression OVER the overbrace value, If someone can find a
    //dedicated term in MathML to handle this overbrace/underbrace concept
    //let me know. C.
    XMLTokenEnum which;

    switch (pNode->GetToken().eType)
    {
        case TOVERBRACE:
        default:
            which = XML_MOVER;
            break;
        case TUNDERBRACE:
            which = XML_MUNDER;
            break;
    }

    OSL_ENSURE(pNode->GetNumSubNodes()==3,"Bad Vertical Brace");
    SvXMLElementExport aOver1(*this, XML_NAMESPACE_MATH,which, sal_True, sal_True);
    {//Scoping
        // using accents will draw the over-/underbraces too close to the base
        // see http://www.w3.org/TR/MathML2/chapter3.html#id.3.4.5.2
        // also XML_ACCENT is illegal with XML_MUNDER. Thus no XML_ACCENT attribut here!
        SvXMLElementExport aOver2(*this, XML_NAMESPACE_MATH,which, sal_True, sal_True);
        ExportNodes(pNode->GetSubNode(0), nLevel);
        ExportNodes(pNode->GetSubNode(1), nLevel);
    }
    ExportNodes(pNode->GetSubNode(2), nLevel);
}

void SmXMLExport::ExportMatrix(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport aTable(*this, XML_NAMESPACE_MATH, XML_MTABLE, sal_True, sal_True);
    const SmMatrixNode *pMatrix = static_cast<const SmMatrixNode *>(pNode);
    sal_uInt16 i=0;
    for (sal_uLong y = 0; y < pMatrix->GetNumRows(); y++)
    {
        SvXMLElementExport aRow(*this, XML_NAMESPACE_MATH, XML_MTR, sal_True, sal_True);
        for (sal_uLong x = 0; x < pMatrix->GetNumCols(); x++)
            if (const SmNode *pTemp = pNode->GetSubNode(i++))
            {
                SvXMLElementExport aCell(*this, XML_NAMESPACE_MATH, XML_MTD, sal_True, sal_True);
                ExportNodes(pTemp, nLevel+1);
            }
    }
}

void SmXMLExport::ExportNodes(const SmNode *pNode, int nLevel)
{
    if (!pNode)
        return;
    switch(pNode->GetType())
    {
        case NTABLE:
            ExportTable(pNode, nLevel);
            break;
        case NALIGN:
        case NBRACEBODY:
        case NEXPRESSION:
            ExportExpression(pNode, nLevel);
            break;
        case NLINE:
            ExportLine(pNode, nLevel);
            break;
        case NTEXT:
            ExportText(pNode, nLevel);
            break;
        case NSPECIAL: //NSPECIAL requires some sort of Entity preservation in the XML engine.
        case NGLYPH_SPECIAL:
        case NMATH:
            {
                sal_Unicode cTmp = 0;
                const SmTextNode *pTemp = static_cast< const SmTextNode * >(pNode);
                if (!pTemp->GetText().isEmpty())
                    cTmp = ConvertMathToMathML( pTemp->GetText()[0] );
                if (cTmp == 0)
                {
                    // no conversion to MathML implemented -> export it as text
                    // thus at least it will not vanish into nothing
                    ExportText(pNode, nLevel);
                }
                else
                {
                    //To fully handle generic MathML we need to implement the full
                    //operator dictionary, we will generate MathML with explicit
                    //stretchiness for now.
                    sal_Int16 nLength = GetAttrList().getLength();
                    sal_Bool bAddStretch=sal_True;
                    for ( sal_Int16 i = 0; i < nLength; i++ )
                    {
                        OUString sLocalName;
                        sal_uInt16 nPrefix = GetNamespaceMap().GetKeyByAttrName(
                            GetAttrList().getNameByIndex(i), &sLocalName );

                        if ( ( XML_NAMESPACE_MATH == nPrefix ) &&
                            IsXMLToken(sLocalName, XML_STRETCHY) )
                        {
                            bAddStretch = sal_False;
                            break;
                        }
                    }
                    if (bAddStretch)
                    {
                        AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
                    }
                    ExportMath(pNode, nLevel);
                }
            }
            break;
        case NPLACE:
            ExportMath(pNode, nLevel);
            break;
        case NBINHOR:
            ExportBinaryHorizontal(pNode, nLevel);
            break;
        case NUNHOR:
            ExportUnaryHorizontal(pNode, nLevel);
            break;
        case NBRACE:
            ExportBrace(pNode, nLevel);
            break;
        case NBINVER:
            ExportBinaryVertical(pNode, nLevel);
            break;
        case NSUBSUP:
            ExportSubSupScript(pNode, nLevel);
            break;
        case NROOT:
            ExportRoot(pNode, nLevel);
            break;
        case NOPER:
            ExportOperator(pNode, nLevel);
            break;
        case NATTRIBUT:
            ExportAttributes(pNode, nLevel);
            break;
        case NFONT:
            ExportFont(pNode, nLevel);
            break;
        case NVERTICAL_BRACE:
            ExportVerticalBrace(pNode, nLevel);
            break;
        case NMATRIX:
            ExportMatrix(pNode, nLevel);
            break;
        case NBLANK:
            ExportBlank(pNode, nLevel);
            break;
       default:
            OSL_FAIL( "Warning: failed to export a node?" );
            break;

    }
}

////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

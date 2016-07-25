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
 closing tags of xml elements in its constructor and destructor, so there's
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
#include <stack>

#include "mathmlexport.hxx"
#include "register.hxx"
#include <starmath.hrc>
#include <unomodel.hxx>
#include <document.hxx>
#include <utility.hxx>
#include "cfgitem.hxx"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

inline bool IsInPrivateUseArea( sal_Unicode cChar ) { return 0xE000 <= cChar  &&  cChar <= 0xF8FF; }

sal_Unicode ConvertMathToMathML( sal_Unicode cChar )
{
    sal_Unicode cRes = cChar;
    if (IsInPrivateUseArea( cChar ))
    {
        SAL_WARN("starmath", "Error: private use area characters should no longer be in use!" );
        cRes = sal_Unicode('@'); // just some character that should easily be notice as odd in the context
    }
    return cRes;
}

}

bool SmXMLExportWrapper::Export(SfxMedium &rMedium)
{
    bool bRet=true;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    //Get model
    uno::Reference< lang::XComponent > xModelComp(xModel, uno::UNO_QUERY );

    bool bEmbedded = false;
    uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));

    SmDocShell *pDocShell = pModel ?
            static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    if ( pDocShell &&
        SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode() )
        bEmbedded = true;

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
        { OUString("UsePrettyPrinting"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
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

    SvtSaveOptions aSaveOpt;
    OUString sUsePrettyPrinting("UsePrettyPrinting");
    bool bUsePrettyPrinting( bFlat || aSaveOpt.IsPrettyPrinting() );
    xInfoSet->setPropertyValue( sUsePrettyPrinting, Any(bUsePrettyPrinting) );

    // Set base URI
    OUString sPropName( "BaseURI" );
    xInfoSet->setPropertyValue( sPropName, makeAny( rMedium.GetBaseURL( true ) ) );

    sal_Int32 nSteps=0;
    if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);
    if (!bFlat) //Storage (Package) of Stream
    {
        uno::Reference < embed::XStorage > xStg = rMedium.GetOutputStorage();
        bool bOASIS = ( SotStorage::GetVersion( xStg ) > SOFFICE_FILEFORMAT_60 );

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
                    xStg, xModelComp, "meta.xml", xContext, xInfoSet,
                    (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaExporter"
                            : "com.sun.star.comp.Math.XMLMetaExporter"));
        }
        if ( bRet )
        {
           if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(
                    xStg, xModelComp, "content.xml", xContext, xInfoSet,
                    "com.sun.star.comp.Math.XMLContentExporter");
        }

        if ( bRet )
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(
                    xStg, xModelComp, "settings.xml", xContext, xInfoSet,
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
            xOut, xModelComp, xContext, xInfoSet,
            "com.sun.star.comp.Math.XMLContentExporter");
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();

    return bRet;
}


/// export through an XML exporter component (output stream version)
bool SmXMLExportWrapper::WriteThroughComponent(
    const Reference<io::XOutputStream>& xOutputStream,
    const Reference<XComponent>& xComponent,
    Reference<uno::XComponentContext> & rxContext,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pComponentName )
{
    OSL_ENSURE(xOutputStream.is(), "I really need an output stream!");
    OSL_ENSURE(xComponent.is(), "Need component!");
    OSL_ENSURE(nullptr != pComponentName, "Need component name!");

    // get component
    Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(rxContext );

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference<xml::sax::XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);

    Sequence<Any> aArgs( 2 );
    aArgs[0] <<= xDocHandler;
    aArgs[1] <<= rPropSet;

    // get filter component
    Reference< document::XExporter > xExporter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(OUString::createFromAscii(pComponentName), aArgs, rxContext),
        UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if ( !xExporter.is() )
        return false;


    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    Reference < XFilter > xFilter( xExporter, UNO_QUERY );
    uno::Sequence< PropertyValue > aProps(0);
    xFilter->filter( aProps );

    uno::Reference<lang::XUnoTunnel> xFilterTunnel( xFilter, uno::UNO_QUERY );
    SmXMLExport *pFilter = reinterpret_cast< SmXMLExport * >(
                sal::static_int_cast< sal_uIntPtr >(
                xFilterTunnel->getSomething( SmXMLExport::getUnoTunnelId() )));
    return pFilter == nullptr || pFilter->GetSuccess();
}


/// export through an XML exporter component (storage version)
bool SmXMLExportWrapper::WriteThroughComponent(
    const Reference < embed::XStorage >& xStorage,
    const Reference<XComponent>& xComponent,
    const sal_Char* pStreamName,
    Reference<uno::XComponentContext> & rxContext,
    Reference<beans::XPropertySet> & rPropSet,
    const sal_Char* pComponentName
    )
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Need stream name!");

    // open stream
    Reference < io::XStream > xStream;
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    try
    {
        xStream = xStorage->openStreamElement( sStreamName,
            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
    }
    catch ( const uno::Exception& rEx )
    {
        SAL_WARN("starmath", "Can't create output stream in package: " << rEx.Message );
        return false;
    }

    OUString aPropName( "MediaType" );
    OUString aMime( "text/xml" );

    uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
    xSet->setPropertyValue( aPropName, Any(aMime) );

    // all streams must be encrypted in encrypted document
    OUString aTmpPropName( "UseCommonStoragePasswordEncryption" );
    xSet->setPropertyValue( aTmpPropName, Any(true) );

    // set Base URL
    if ( rPropSet.is() )
    {
        OUString sPropName( "StreamName" );
        rPropSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
    }

    // write the stuff
    bool bRet = WriteThroughComponent( xStream->getOutputStream(), xComponent, rxContext,
        rPropSet, pComponentName );

    return bRet;
}

SmXMLExport::SmXMLExport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(util::MeasureUnit::INCH, rContext, implementationName, XML_MATH,
                  nExportFlags)
    , pTree(nullptr)
    , bSuccess(false)
{
}

sal_Int64 SAL_CALL SmXMLExport::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
throw(uno::RuntimeException, std::exception)
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLExporter", SvXMLExportFlags::OASIS|SvXMLExportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLMetaExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLMetaExporter", SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLOasisMetaExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLOasisMetaExporter", SvXMLExportFlags::OASIS|SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLSettingsExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLSettingsExporter", SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLOasisSettingsExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLOasisSettingsExporter", SvXMLExportFlags::OASIS|SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Math_XMLContentExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLContentExporter", SvXMLExportFlags::OASIS|SvXMLExportFlags::CONTENT));
}

sal_uInt32 SmXMLExport::exportDoc(enum XMLTokenEnum eClass)
{
    if ( !(getExportFlags() & SvXMLExportFlags::CONTENT) )
    {
        SvXMLExport::exportDoc( eClass );
    }
    else
    {
        uno::Reference <frame::XModel> xModel = GetModel();
        uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
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
        GetNamespaceMap_().Add( OUString(), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );

        rList.AddAttribute(GetNamespaceMap().GetAttrNameByKey(XML_NAMESPACE_MATH_IDX),
                GetNamespaceMap().GetNameByKey( XML_NAMESPACE_MATH_IDX));

        //I think we need something like ImplExportEntities();
        ExportContent_();
        GetDocHandler()->endDocument();
    }

    bSuccess=true;
    return 0;
}

void SmXMLExport::ExportContent_()
{
    uno::Reference <frame::XModel> xModel = GetModel();
    uno::Reference <lang::XUnoTunnel> xTunnel(xModel,uno::UNO_QUERY);
    SmModel *pModel = reinterpret_cast<SmModel *>
        (xTunnel->getSomething(SmModel::getUnoTunnelId()));
    SmDocShell *pDocShell = pModel ?
        static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    OSL_ENSURE( pDocShell, "doc shell missing" );

    if (pDocShell && !pDocShell->GetFormat().IsTextmode())
    {
        // If the Math equation is not in text mode, we attach a display="block"
        // attribute on the <math> root. We don't do anything if it is in
        // text mode, the default display="inline" value will be used.
        AddAttribute(XML_NAMESPACE_MATH, XML_DISPLAY, XML_BLOCK);
    }
    SvXMLElementExport aEquation(*this, XML_NAMESPACE_MATH, XML_MATH, true, true);
    SvXMLElementExport *pSemantics=nullptr;

    if (!aText.isEmpty())
    {
        pSemantics = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
            XML_SEMANTICS, true, true);
    }

    ExportNodes(pTree, 0);

    if (!aText.isEmpty())
    {
        // Convert symbol names
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
            XML_ANNOTATION, true, false);
        GetDocHandler()->characters( aText );
    }
    delete pSemantics;
}

void SmXMLExport::GetViewSettings( Sequence < PropertyValue >& aProps)
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

    aProps.realloc( 4 );
    PropertyValue *pValue = aProps.getArray();
    sal_Int32 nIndex = 0;

    Rectangle aRect( pDocShell->GetVisArea() );

    pValue[nIndex].Name = "ViewAreaTop";
    pValue[nIndex++].Value <<= aRect.Top();

    pValue[nIndex].Name = "ViewAreaLeft";
    pValue[nIndex++].Value <<= aRect.Left();

    pValue[nIndex].Name = "ViewAreaWidth";
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
                    SmMathConfig *pConfig = SM_MOD()->GetConfig();
                    const bool bUsedSymbolsOnly = pConfig && pConfig->IsSaveOnlyUsedSymbols();

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

                            OUString aActualName( rPropName );

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
    TG nGroup = pNode->GetToken().nGroup;

    SvXMLElementExport* pRow = new SvXMLElementExport(*this,
        XML_NAMESPACE_MATH, XML_MROW, true, true);

    // Unfold the binary tree structure as long as the nodes are SmBinHorNode
    // with the same nGroup. This will reduce the number of nested <mrow>
    // elements e.g. we only need three <mrow> levels to export

    // "a*b*c*d+e*f*g*h+i*j*k*l = a*b*c*d+e*f*g*h+i*j*k*l =
    //  a*b*c*d+e*f*g*h+i*j*k*l = a*b*c*d+e*f*g*h+i*j*k*l"

    // See https://www.libreoffice.org/bugzilla/show_bug.cgi?id=66081
    ::std::stack< const SmNode* > s;
    s.push(pNode);
    while (!s.empty())
    {
        const SmNode *node = s.top();
        s.pop();
        if (node->GetType() != NBINHOR || node->GetToken().nGroup != nGroup)
        {
            ExportNodes(node, nLevel+1);
            continue;
        }
        const SmBinHorNode* binNode = static_cast<const SmBinHorNode*>(node);
        s.push(binNode->RightOperand());
        s.push(binNode->Symbol());
        s.push(binNode->LeftOperand());
    }

    delete pRow;
}

void SmXMLExport::ExportUnaryHorizontal(const SmNode *pNode, int nLevel)
{
    ExportExpression(pNode, nLevel);
}

void SmXMLExport::ExportExpression(const SmNode *pNode, int nLevel,
                                   bool bNoMrowContainer /*=false*/)
{
    SvXMLElementExport *pRow=nullptr;
    auto nSize = pNode->GetNumSubNodes();

    // #i115443: nodes of type expression always need to be grouped with mrow statement
    if (!bNoMrowContainer &&
        (nSize > 1 || pNode->GetType() == NEXPRESSION))
        pRow = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MROW, true, true);

    for (sal_uInt16 i = 0; i < nSize; i++)
        if (const SmNode *pTemp = pNode->GetSubNode(i))
            ExportNodes(pTemp, nLevel+1);

    delete pRow;
}

void SmXMLExport::ExportBinaryVertical(const SmNode *pNode, int nLevel)
{
    OSL_ENSURE(pNode->GetNumSubNodes()==3,"Bad Fraction");
    const SmNode *pNum = pNode->GetSubNode(0);
    const SmNode *pDenom = pNode->GetSubNode(2);
    if (pNum->GetType() == NALIGN && pNum->GetToken().eType != TALIGNC)
    {
        // A left or right alignment is specified on the numerator:
        // attach the corresponding numalign attribute.
        AddAttribute(XML_NAMESPACE_MATH, XML_NUMALIGN,
            pNum->GetToken().eType == TALIGNL ? XML_LEFT : XML_RIGHT);
    }
    if (pDenom->GetType() == NALIGN && pDenom->GetToken().eType != TALIGNC)
    {
        // A left or right alignment is specified on the denominator:
        // attach the corresponding denomalign attribute.
        AddAttribute(XML_NAMESPACE_MATH, XML_DENOMALIGN,
            pDenom->GetToken().eType == TALIGNL ? XML_LEFT : XML_RIGHT);
    }
    SvXMLElementExport aFraction(*this, XML_NAMESPACE_MATH, XML_MFRAC, true, true);
    ExportNodes(pNum, nLevel);
    ExportNodes(pDenom, nLevel);
}

void SmXMLExport::ExportBinaryDiagonal(const SmNode *pNode, int nLevel)
{
    OSL_ENSURE(pNode->GetNumSubNodes()==3, "Bad Slash");

    if (pNode->GetToken().eType == TWIDESLASH)
    {
        // wideslash
        // export the node as <mfrac bevelled="true">
        AddAttribute(XML_NAMESPACE_MATH, XML_BEVELLED, XML_TRUE);
        SvXMLElementExport aFraction(*this, XML_NAMESPACE_MATH, XML_MFRAC,
            true, true);
        ExportNodes(pNode->GetSubNode(0), nLevel);
        ExportNodes(pNode->GetSubNode(1), nLevel);
    }
    else
    {
        // widebslash
        // We can not use <mfrac> to a backslash, so just use <mo>\</mo>
        SvXMLElementExport *pRow = new SvXMLElementExport(*this,
            XML_NAMESPACE_MATH, XML_MROW, true, true);

        ExportNodes(pNode->GetSubNode(0), nLevel);

        { // Scoping for <mo> creation
        SvXMLElementExport aMo(*this, XML_NAMESPACE_MATH, XML_MO,
            true, true);
        sal_Unicode nArse[2] = {MS_BACKSLASH,0x00};
        GetDocHandler()->characters(nArse);
        }

        ExportNodes(pNode->GetSubNode(1), nLevel);

        delete pRow;
    }
}

void SmXMLExport::ExportTable(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pTable=nullptr;

    sal_uInt16 nSize = pNode->GetNumSubNodes();

    //If the list ends in newline then the last entry has
    //no subnodes, the newline is superfluous so we just drop
    //the last node, inclusion would create a bad MathML
    //table
    if (nSize >= 1)
    {
        const SmNode *pLine = pNode->GetSubNode(nSize-1);
        if (pLine->GetType() == NLINE && pLine->GetNumSubNodes() == 1 &&
            pLine->GetSubNode(0) != nullptr &&
            pLine->GetSubNode(0)->GetToken().eType == TNEWLINE)
            --nSize;
    }

    // try to avoid creating a mtable element when the formula consists only
    // of a single output line
    if (nLevel || (nSize >1))
        pTable = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTABLE, true, true);

    for (sal_uInt16 i = 0; i < nSize; i++)
        if (const SmNode *pTemp = pNode->GetSubNode(i))
        {
            SvXMLElementExport *pRow=nullptr;
            SvXMLElementExport *pCell=nullptr;
            if (pTable)
            {
                pRow  = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTR, true, true);
                SmTokenType eAlign = TALIGNC;
                if (pTemp->GetType() == NALIGN)
                {
                    // For Binom() and Stack() constructions, the NALIGN nodes
                    // are direct children.
                    // binom{alignl ...}{alignr ...} and
                    // stack{alignl ... ## alignr ... ## ...}
                    eAlign = pTemp->GetToken().eType;
                }
                else if (pTemp->GetType() == NLINE &&
                         pTemp->GetNumSubNodes() == 1 &&
                         pTemp->GetSubNode(0) &&
                         pTemp->GetSubNode(0)->GetType() == NALIGN)
                {
                    // For the Table() construction, the NALIGN node is a child
                    // of an NLINE node.
                    // alignl ... newline alignr ... newline ...
                    eAlign = pTemp->GetSubNode(0)->GetToken().eType;
                }
                if (eAlign != TALIGNC)
                {
                    // If a left or right alignment is specified on this line,
                    // attach the corresponding columnalign attribute.
                    AddAttribute(XML_NAMESPACE_MATH, XML_COLUMNALIGN,
                        eAlign == TALIGNL ? XML_LEFT : XML_RIGHT);
                }
                pCell = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTD, true, true);
            }
            ExportNodes(pTemp, nLevel+1);
            delete pCell;
            delete pRow;
        }

    delete pTable;
}

void SmXMLExport::ExportMath(const SmNode *pNode, int /*nLevel*/)
{
    const SmTextNode *pTemp = static_cast<const SmTextNode *>(pNode);
    SvXMLElementExport *pMath = nullptr;

    if (pNode->GetType() == NMATH || pNode->GetType() == NGLYPH_SPECIAL)
    {
        // Export NMATH and NGLYPH_SPECIAL symbols as <mo> elements
        pMath = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MO, true, false);
    }
    else
    {
        // Export NMATHIDENT and NPLACE symbols as <mi> elements:
        // - These math symbols should not be drawn slanted. Hence we should
        // attach a mathvariant="normal" attribute to single-char <mi> elements
        // that are not mathematical alphanumeric symbol. For simplicity and to
        // work around browser limitations, we always attach such an attribute.
        // - The MathML specification suggests to use empty <mi> elements as
        // placeholders but they won't be visible in most MathML rendering
        // engines so let's use an empty square for NPLACE instead.
        AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, XML_NORMAL);
        pMath = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MI, true, false);
    }
    sal_Unicode nArse[2];
    nArse[0] = pTemp->GetText()[0];
    sal_Unicode cTmp = ConvertMathToMathML( nArse[0] );
    if (cTmp != 0)
        nArse[0] = cTmp;
    OSL_ENSURE(nArse[0] != 0xffff,"Non existent symbol");
    nArse[1] = 0;
    GetDocHandler()->characters(nArse);

    delete pMath;
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
            bool bIsItalic = IsItalic( pTemp->GetFont() );
            if ((pTemp->GetText().getLength() > 1) && bIsItalic)
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, XML_ITALIC);
            else if ((pTemp->GetText().getLength() == 1) && !bIsItalic)
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, XML_NORMAL);
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MI, true, false);
            break;
        }
        case TNUMBER:
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MN, true, false);
            break;
        case TTEXT:
            pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MTEXT, true, false);
            break;
        }
    GetDocHandler()->characters(pTemp->GetText());
    delete pText;
}

void SmXMLExport::ExportBlank(const SmNode *pNode, int /*nLevel*/)
{
    const SmBlankNode *pTemp = static_cast<const SmBlankNode *>(pNode);
    //!! exports an <mspace> element. Note that for example "~_~" is allowed in
    //!! Math (so it has no sense at all) but must not result in an empty
    //!! <msub> tag in MathML !!

    if (pTemp->GetBlankNum() != 0)
    {
        // Attach a width attribute. We choose the (somewhat arbitrary) values
        // ".5em" for a small gap '`' and "2em" for a large gap '~'.
        // (see SmBlankNode::IncreaseBy for how pTemp->nNum is set).
        OUStringBuffer sStrBuf;
        ::sax::Converter::convertDouble(sStrBuf, pTemp->GetBlankNum() * .5);
        sStrBuf.append("em");
        AddAttribute(XML_NAMESPACE_MATH, XML_WIDTH, sStrBuf.getStr());
    }

    SvXMLElementExport *pText;

    pText = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MSPACE,
        true, false);

    GetDocHandler()->characters( OUString() );
    delete pText;
}

void SmXMLExport::ExportSubSupScript(const SmNode *pNode, int nLevel)
{
    const SmNode *pSub  = nullptr;
    const SmNode *pSup  = nullptr;
    const SmNode *pCSub = nullptr;
    const SmNode *pCSup = nullptr;
    const SmNode *pLSub = nullptr;
    const SmNode *pLSup = nullptr;
    SvXMLElementExport *pThing2 = nullptr;

    //if we have prescripts at all then we must use the tensor notation

    //This is one of those excellent locations where scope is vital to
    //arrange the construction and destruction of the element helper
    //classes correctly
    pLSub = pNode->GetSubNode(LSUB+1);
    pLSup = pNode->GetSubNode(LSUP+1);
    if (pLSub || pLSup)
    {
        SvXMLElementExport aMultiScripts(*this, XML_NAMESPACE_MATH,
            XML_MMULTISCRIPTS, true, true);


        if (nullptr != (pCSub = pNode->GetSubNode(CSUB+1))
            && nullptr != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDEROVER, true, true);
        }
        else if (nullptr != (pCSub = pNode->GetSubNode(CSUB+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDER, true, true);
        }
        else if (nullptr != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MOVER, true, true);
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
                SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE, true, true);
            }
            if (pSup)
                ExportNodes(pSup, nLevel+1);
            else
            {
                SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE, true, true);
            }
        }

        //Separator element between suffix and prefix sub/sup pairs
        {
            SvXMLElementExport aPrescripts(*this, XML_NAMESPACE_MATH,
                XML_MPRESCRIPTS, true, true);
        }

        if (pLSub)
            ExportNodes(pLSub, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,
                true, true);

        }
        if (pLSup)
            ExportNodes(pLSup, nLevel+1);
        else
        {
            SvXMLElementExport aNone(*this, XML_NAMESPACE_MATH, XML_NONE,
                true, true);

        }
    }
    else
    {
        SvXMLElementExport *pThing = nullptr;
        if (nullptr != (pSub = pNode->GetSubNode(RSUB+1)) &&
            nullptr != (pSup = pNode->GetSubNode(RSUP+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MSUBSUP, true, true);
        }
        else if (nullptr != (pSub = pNode->GetSubNode(RSUB+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MSUB,
                true, true);
        }
        else if (nullptr != (pSup = pNode->GetSubNode(RSUP+1)))
        {
            pThing = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MSUP,
                true, true);
        }

        if (nullptr != (pCSub = pNode->GetSubNode(CSUB+1))
            && nullptr != (pCSup=pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDEROVER, true, true);
        }
        else if (nullptr != (pCSub = pNode->GetSubNode(CSUB+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MUNDER, true, true);
        }
        else if (nullptr != (pCSup = pNode->GetSubNode(CSUP+1)))
        {
            pThing2 = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                XML_MOVER, true, true);
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
    SvXMLElementExport *pRow=nullptr;

    // This used to generate <mfenced> or <mrow>+<mo> elements according to
    // the stretchiness of fences. The MathML recommendation defines an
    // <mrow>+<mo> construction that is equivalent to the <mfenced> element:
    // http://www.w3.org/TR/MathML3/chapter3.html#presm.mfenced
    // To simplify our code and avoid issues with mfenced implementations in
    // MathML rendering engines, we now always generate <mrow>+<mo> elements.
    // See #fdo 66282.

    // <mrow>
    pRow = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MROW,
        true, true);

    //   <mo fence="true"> opening-fence </mo>
    if (pLeft && (pLeft->GetToken().eType != TNONE))
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_FENCE, XML_TRUE);
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
        else
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
        ExportNodes(pLeft, nLevel+1);
    }

    if (nullptr != (pTemp = pNode->GetSubNode(1)))
    {
        // <mrow>
        SvXMLElementExport aRow(*this, XML_NAMESPACE_MATH, XML_MROW,
            true, true);
        ExportNodes(pTemp, nLevel+1);
        // </mrow>
    }

    //   <mo fence="true"> closing-fence </mo>
    if (pRight && (pRight->GetToken().eType != TNONE))
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_FENCE, XML_TRUE);
        if (pNode->GetScaleMode() == SCALE_HEIGHT)
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
        else
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
        ExportNodes(pRight, nLevel+1);
    }

    delete pRow;
    // </mrow>
}

void SmXMLExport::ExportRoot(const SmNode *pNode, int nLevel)
{
    if (pNode->GetSubNode(0))
    {
        SvXMLElementExport aRoot(*this, XML_NAMESPACE_MATH, XML_MROOT, true,
            true);
        ExportNodes(pNode->GetSubNode(2), nLevel+1);
        ExportNodes(pNode->GetSubNode(0), nLevel+1);
    }
    else
    {
        SvXMLElementExport aSqrt(*this, XML_NAMESPACE_MATH, XML_MSQRT, true,
            true);
        ExportNodes(pNode->GetSubNode(2), nLevel+1);
    }
}

void SmXMLExport::ExportOperator(const SmNode *pNode, int nLevel)
{
    /*we need to either use content or font and size attributes
     *here*/
    SvXMLElementExport aRow(*this, XML_NAMESPACE_MATH, XML_MROW,
        true, true);
    ExportNodes(pNode->GetSubNode(0), nLevel+1);
    ExportNodes(pNode->GetSubNode(1), nLevel+1);
}

void SmXMLExport::ExportAttributes(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport *pElement=nullptr;

    if (pNode->GetToken().eType == TUNDERLINE)
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENTUNDER,
            XML_TRUE);
        pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MUNDER,
            true, true);
    }
    else if (pNode->GetToken().eType == TOVERSTRIKE)
    {
        // export as <menclose notation="horizontalstrike">
        AddAttribute(XML_NAMESPACE_MATH, XML_NOTATION, XML_HORIZONTALSTRIKE);
        pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
            XML_MENCLOSE, true, true);
    }
    else
    {
        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENT,
            XML_TRUE);
        pElement = new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_MOVER,
            true, true);
    }

    ExportNodes(pNode->GetSubNode(1), nLevel+1);
    switch (pNode->GetToken().eType)
    {
        case TOVERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this, XML_NAMESPACE_MATH, XML_MO,
                true, true);
            sal_Unicode nArse[2] = {0xAF,0x00};
            GetDocHandler()->characters(nArse);
            }
            break;
        case TUNDERLINE:
            {
            //proper entity support required
            SvXMLElementExport aMath(*this, XML_NAMESPACE_MATH, XML_MO,
                true, true);
            sal_Unicode nArse[2] = {0x0332,0x00};
            GetDocHandler()->characters(nArse);
            }
            break;
        case TOVERSTRIKE:
            break;
        case TWIDETILDE:
        case TWIDEHAT:
        case TWIDEVEC:
            {
            // make these wide accents stretchy
            AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
            ExportNodes(pNode->GetSubNode(0), nLevel+1);
            }
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

    // gather the mathvariant attribute relevant data from all
    // successively following SmFontNodes...

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
                SAL_WARN("starmath", "unexpected case");
        }
        // According to the parser every node that is to be evaluated here
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
        case TPHANTOM:
            // No attribute needed. An <mphantom> element will be used below.
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
        case TSILVER:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_SILVER);
            break;
        case TGRAY:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_GRAY);
            break;
        case TMAROON:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_MAROON);
            break;
        case TOLIVE:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_OLIVE);
            break;
        case TLIME:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_LIME);
            break;
        case TAQUA:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_AQUA);
            break;
        case TTEAL:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_TEAL);
            break;
        case TNAVY:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_NAVY);
            break;
        case TFUCHSIA:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_FUCHSIA);
            break;
        case TPURPLE:
            AddAttribute(XML_NAMESPACE_MATH, XML_COLOR, XML_PURPLE);
            break;
        case TSIZE:
            {
                const SmFontNode *pFontNode = static_cast<const SmFontNode *>(pNode);
                const Fraction &aFrac = pFontNode->GetSizeParameter();

                OUStringBuffer sStrBuf;
                switch(pFontNode->GetSizeType())
                {
                    case FontSizeType::MULTIPLY:
                        ::sax::Converter::convertDouble(sStrBuf,
                            static_cast<double>(aFrac*Fraction(100.00)));
                        sStrBuf.append('%');
                        break;
                    case FontSizeType::DIVIDE:
                        ::sax::Converter::convertDouble(sStrBuf,
                            static_cast<double>(Fraction(100.00)/aFrac));
                        sStrBuf.append('%');
                        break;
                    case FontSizeType::ABSOLUT:
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
                            Fraction aTemp = Sm100th_mmToPts(pFontNode->GetFont().GetFontSize().Height());

                            if (pFontNode->GetSizeType() == FontSizeType::MINUS)
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
                    SAL_WARN("starmath", "unexpected case");
                }
                AddAttribute(XML_NAMESPACE_MATH, XML_MATHVARIANT, OUString::createFromAscii( pText ));
            }
            break;
        default:
            break;

    }
    {
        // Wrap everything in an <mphantom> or <mstyle> element. These elements
        // are mrow-like, so ExportExpression doesn't need to add an explicit
        // <mrow> element. See #fdo 66283.
        SvXMLElementExport aElement(*this, XML_NAMESPACE_MATH,
            pNode->GetToken().eType == TPHANTOM ? XML_MPHANTOM : XML_MSTYLE,
            true, true);
        ExportExpression(pNode, nLevel, true);
    }
}


void SmXMLExport::ExportVerticalBrace(const SmNode *pNode, int nLevel)
{
    // "[body] overbrace [script]"

    // Position body, overbrace and script vertically. First place the overbrace
    // OVER the body and then the script OVER this expression.

    //      [script]
    //   --[overbrace]--
    // XXXXXX[body]XXXXXXX

    // Similarly for the underbrace construction.

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
    SvXMLElementExport aOver1(*this, XML_NAMESPACE_MATH,which, true, true);
    {//Scoping
        // using accents will draw the over-/underbraces too close to the base
        // see http://www.w3.org/TR/MathML2/chapter3.html#id.3.4.5.2
        // also XML_ACCENT is illegal with XML_MUNDER. Thus no XML_ACCENT attribute here!
        SvXMLElementExport aOver2(*this, XML_NAMESPACE_MATH,which, true, true);
        ExportNodes(pNode->GetSubNode(0), nLevel);
        AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
        ExportNodes(pNode->GetSubNode(1), nLevel);
    }
    ExportNodes(pNode->GetSubNode(2), nLevel);
}

void SmXMLExport::ExportMatrix(const SmNode *pNode, int nLevel)
{
    SvXMLElementExport aTable(*this, XML_NAMESPACE_MATH, XML_MTABLE, true, true);
    const SmMatrixNode *pMatrix = static_cast<const SmMatrixNode *>(pNode);
    sal_uInt16 i=0;
    for (sal_uInt16 y = 0; y < pMatrix->GetNumRows(); y++)
    {
        SvXMLElementExport aRow(*this, XML_NAMESPACE_MATH, XML_MTR, true, true);
        for (sal_uInt16 x = 0; x < pMatrix->GetNumCols(); x++)
            if (const SmNode *pTemp = pNode->GetSubNode(i++))
            {
                if (pTemp->GetType() == NALIGN &&
                    pTemp->GetToken().eType != TALIGNC)
                {
                    // A left or right alignment is specified on this cell,
                    // attach the corresponding columnalign attribute.
                    AddAttribute(XML_NAMESPACE_MATH, XML_COLUMNALIGN,
                        pTemp->GetToken().eType == TALIGNL ?
                        XML_LEFT : XML_RIGHT);
                }
                SvXMLElementExport aCell(*this, XML_NAMESPACE_MATH, XML_MTD, true, true);
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
                    bool bAddStretch=true;
                    for ( sal_Int16 i = 0; i < nLength; i++ )
                    {
                        OUString sLocalName;
                        sal_uInt16 nPrefix = GetNamespaceMap().GetKeyByAttrName(
                            GetAttrList().getNameByIndex(i), &sLocalName );

                        if ( ( XML_NAMESPACE_MATH == nPrefix ) &&
                            IsXMLToken(sLocalName, XML_STRETCHY) )
                        {
                            bAddStretch = false;
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
        case NSPECIAL: //NSPECIAL requires some sort of Entity preservation in the XML engine.
        case NMATHIDENT :
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
        case NBINDIAGONAL:
            ExportBinaryDiagonal(pNode, nLevel);
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
            SAL_WARN("starmath", "Warning: failed to export a node?");
            break;

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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


#include <rsc/rscsfx.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <sfx2/docfile.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/stritem.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/frame.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <swerror.h>
#include <fltini.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <unotextrange.hxx>
#include <swmodule.hxx>
#include <SwXMLSectionList.hxx>

#include <statstr.hrc>

// #i44177#
#include <SwStyleNameMapper.hxx>
#include <poolfmt.hxx>
#include <numrule.hxx>
#include <paratr.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <sfx2/docfilt.hxx> // #i70748#
#include <istyleaccess.hxx>

#include <sfx2/DocumentMetadataAccess.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;


static void lcl_EnsureValidPam( SwPaM& rPam )
{
    if( rPam.GetCntntNode() != NULL )
    {
        // set proper point content
        if( rPam.GetCntntNode() != rPam.GetPoint()->nContent.GetIdxReg() )
        {
            rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
        }
        // else: point was already valid

        // if mark is invalid, we delete it
        if( ( rPam.GetCntntNode( sal_False ) == NULL ) ||
            ( rPam.GetCntntNode( sal_False ) != rPam.GetMark()->nContent.GetIdxReg() ) )
        {
            rPam.DeleteMark();
        }
    }
    else
    {
        // point is not valid, so move it into the first content
        rPam.DeleteMark();
        rPam.GetPoint()->nNode =
            *rPam.GetDoc()->GetNodes().GetEndOfContent().StartOfSectionNode();
        ++ rPam.GetPoint()->nNode;
        rPam.Move( fnMoveForward, fnGoCntnt ); // go into content
    }
}

XMLReader::XMLReader()
{
}

int XMLReader::GetReaderType()
{
    return SW_STORAGE_READER;
}

/// read a component (file + filter version)
sal_Int32 ReadThroughComponent(
    uno::Reference<io::XInputStream> xInputStream,
    uno::Reference<XComponent> xModelComponent,
    const String& rStreamName,
    uno::Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessfull,
    bool bEncrypted )
{
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rxContext.is(), "factory missing");
    OSL_ENSURE(NULL != pFilterName,"I need a service name for the component!");

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;
    aParserInput.aInputStream = xInputStream;

    // get parser
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(rxContext);
    SAL_INFO( "sw.filter", "parser created" );
    // get filter
    OUString aFilterName(OUString::createFromAscii(pFilterName));
    uno::Reference< xml::sax::XDocumentHandler > xFilter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(aFilterName, rFilterArguments, rxContext),
        UNO_QUERY);
    SAL_WARN_IF(!xFilter.is(), "sw", "Can't instantiate filter component: " << aFilterName);
    if( !xFilter.is() )
        return ERR_SWG_READ_ERROR;
    SAL_INFO( "sw.filter", "" << pFilterName << " created" );
    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // connect model and filter
    uno::Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );


#ifdef TIMELOG
    // if we do profiling, we want to know the stream
    OString aString(OUStringToOString(rStreamName,
        RTL_TEXTENCODING_ASCII_US));
    SAL_INFO( "sw.filter", "ReadThroughComponent : parsing \"" << aString.getStr() << "\"" );
#endif

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& r )
    {
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *(xml::sax::SAXException*)(&r);
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

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError(RTL_CONSTASCII_STRINGPARAM(
            "SAX parse exception caught while importing:\n"));
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif

        String sErr( OUString::number( r.LineNumber ));
        sErr += ',';
        sErr += OUString::number( r.ColumnNumber );

        if( rStreamName.Len() )
        {
            return *new TwoStringErrorInfo(
                            (bMustBeSuccessfull ? ERR_FORMAT_FILE_ROWCOL
                                                    : WARN_FORMAT_FILE_ROWCOL),
                            rStreamName, sErr,
                            ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
        else
        {
            OSL_ENSURE( bMustBeSuccessfull, "Warnings are not supported" );
            return *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                             ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
    }
    catch(const xml::sax::SAXException& r)
    {
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError(RTL_CONSTASCII_STRINGPARAM(
            "SAX exception caught while importing:\n"));
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif

        return ERR_SWG_READ_ERROR;
    }
    catch(const packages::zip::ZipIOException& r)
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError(RTL_CONSTASCII_STRINGPARAM(
            "Zip exception caught while importing:\n"));
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch(const io::IOException& r)
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError(RTL_CONSTASCII_STRINGPARAM(
            "IO exception caught while importing:\n"));
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif
        return ERR_SWG_READ_ERROR;
    }
    catch(const uno::Exception& r)
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError(RTL_CONSTASCII_STRINGPARAM(
            "uno exception caught while importing:\n"));
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif
        return ERR_SWG_READ_ERROR;
    }

    // success!
    return 0;
}

/// read a component (storage version)
sal_Int32 ReadThroughComponent(
    uno::Reference<embed::XStorage> xStorage,
    uno::Reference<XComponent> xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    uno::Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessfull)
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(NULL != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    sal_Bool bContainsStream = sal_False;
    try
    {
        bContainsStream = xStorage->isStreamElement(sStreamName);
    }
    catch( container::NoSuchElementException& )
    {
    }

    if (!bContainsStream )
    {
        // stream name not found! Then try the compatibility name.
        // if no stream can be opened, return immediately with OK signal

        // do we even have an alternative name?
        if ( NULL == pCompatibilityStreamName )
            return 0;

        // if so, does the stream exist?
        sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
        try
        {
            bContainsStream = xStorage->isStreamElement(sStreamName);
        }
        catch( container::NoSuchElementException& )
        {
        }

        if (! bContainsStream )
            return 0;
    }

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( rFilterArguments.getLength() > 0 )
        rFilterArguments.getConstArray()[0] >>= xInfoSet;
    OSL_ENSURE( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        xInfoSet->setPropertyValue( "StreamName", makeAny( sStreamName ) );
    }

    try
    {
        // get input stream
        uno::Reference <io::XStream> xStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
        uno::Reference <beans::XPropertySet > xProps( xStream, uno::UNO_QUERY );

        Any aAny = xProps->getPropertyValue("Encrypted");

        bool bEncrypted = aAny.getValueType() == ::getBooleanCppuType() &&
                *(sal_Bool *)aAny.getValue();

        uno::Reference <io::XInputStream> xInputStream = xStream->getInputStream();

        // read from the stream
        return ReadThroughComponent(
            xInputStream, xModelComponent, sStreamName, rxContext,
            pFilterName, rFilterArguments,
            rName, bMustBeSuccessfull, bEncrypted );
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
        OSL_FAIL( "Error on import!\n" );
        // TODO/LATER: error handling
    }

    return ERR_SWG_READ_ERROR;
}

// #i44177#
static void lcl_AdjustOutlineStylesForOOo( SwDoc& _rDoc )
{
    // array containing the names of the default outline styles ('Heading 1',
    // 'Heading 2', ..., 'Heading 10')
    String aDefOutlStyleNames[ MAXLEVEL ];
    {
        String sStyleName;
        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            sStyleName =
                SwStyleNameMapper::GetProgName( static_cast< sal_uInt16 >(RES_POOLCOLL_HEADLINE1 + i),
                                                sStyleName );
            aDefOutlStyleNames[i] = sStyleName;
        }
    }

    // array indicating, which outline level already has a style assigned.
    bool aOutlineLevelAssigned[ MAXLEVEL ];
    // array of the default outline styles, which are created for the document.
    SwTxtFmtColl* aCreatedDefaultOutlineStyles[ MAXLEVEL ];

    {
        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            aOutlineLevelAssigned[ i ] = false;
            aCreatedDefaultOutlineStyles[ i ] = 0L;
        }
    }

    // determine, which outline level has already a style assigned and
    // which of the default outline styles is created.
    const SwTxtFmtColls& rColls = *(_rDoc.GetTxtFmtColls());
    for ( sal_uInt16 n = 1; n < rColls.size(); ++n )
    {
        SwTxtFmtColl* pColl = rColls[ n ];
        if ( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            aOutlineLevelAssigned[ pColl->GetAssignedOutlineStyleLevel() ] = true;//<-end,zhaojianwei
        }

        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            if ( aCreatedDefaultOutlineStyles[ i ] == 0L &&
                 pColl->GetName() == aDefOutlStyleNames[i] )
            {
                aCreatedDefaultOutlineStyles[ i ] = pColl;
                break;
            }
        }
    }

    // assign already created default outline style to outline level, which
    // doesn't have a style assigned to it.
    const SwNumRule* pOutlineRule = _rDoc.GetOutlineNumRule();
    for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
    {
        // #i73361#
        // Do not change assignment of already created default outline style
        // to a certain outline level.
        if ( !aOutlineLevelAssigned[ i ] &&
             aCreatedDefaultOutlineStyles[ i ] != 0 &&
             ! aCreatedDefaultOutlineStyles[ i ]->IsAssignedToListLevelOfOutlineStyle() )
        {
            // apply outline level at created default outline style
            aCreatedDefaultOutlineStyles[ i ]->AssignToListLevelOfOutlineStyle(i);//#outline level added by zhaojianwei

            // apply outline numbering rule, if none is set.
            const SfxPoolItem& rItem =
                aCreatedDefaultOutlineStyles[ i ]->GetFmtAttr( RES_PARATR_NUMRULE, sal_False );
            if ( static_cast<const SwNumRuleItem&>(rItem).GetValue().isEmpty() )
            {
                SwNumRuleItem aItem( pOutlineRule->GetName() );
                aCreatedDefaultOutlineStyles[ i ]->SetFmtAttr( aItem );
            }
        }
    }

}

static void lcl_ConvertSdrOle2ObjsToSdrGrafObjs( SwDoc& _rDoc )
{
    if ( _rDoc.GetDrawModel() &&
         _rDoc.GetDrawModel()->GetPage( 0 ) )
    {
        const SdrPage& rSdrPage( *(_rDoc.GetDrawModel()->GetPage( 0 )) );

        // iterate recursive with group objects over all shapes on the draw page
        SdrObjListIter aIter( rSdrPage );
        while( aIter.IsMore() )
        {
            SdrOle2Obj* pOle2Obj = dynamic_cast< SdrOle2Obj* >( aIter.Next() );
            if( pOle2Obj )
            {
                // found an ole2 shape
                SdrObjList* pObjList = pOle2Obj->GetObjList();

                // get its graphic
                Graphic aGraphic;
                pOle2Obj->Connect();
                const Graphic* pGraphic = pOle2Obj->GetGraphic();
                if( pGraphic )
                    aGraphic = *pGraphic;
                pOle2Obj->Disconnect();

                // create new graphic shape with the ole graphic and shape size
                SdrGrafObj* pGraphicObj = new SdrGrafObj( aGraphic, pOle2Obj->GetCurrentBoundRect() );
                // apply layer of ole2 shape at graphic shape
                pGraphicObj->SetLayer( pOle2Obj->GetLayer() );

                // replace ole2 shape with the new graphic object and delete the ol2 shape
                SdrObject* pReplaced = pObjList->ReplaceObject( pGraphicObj, pOle2Obj->GetOrdNum() );
                SdrObject::Free( pReplaced );
            }
        }
    }
}


sal_uLong XMLReader::Read( SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPaM, const OUString & rName )
{
    // Get service factory
    uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

    uno::Reference< io::XActiveDataSource > xSource;
    uno::Reference< XInterface > xPipe;
    uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    // get the input stream (storage or stream)
    uno::Reference<io::XInputStream> xInputStream;
    uno::Reference<embed::XStorage> xStorage;
    if( pMedium )
        xStorage = pMedium->GetStorage();
    else
        xStorage = xStg;

    if( !xStorage.is() )
        return ERR_SWG_READ_ERROR;

    pGraphicHelper = SvXMLGraphicHelper::Create( xStorage,
                                                 GRAPHICHELPER_MODE_READ,
                                                 sal_False );
    xGraphicResolver = pGraphicHelper;
    SfxObjectShell *pPersist = rDoc.GetPersist();
    if( pPersist )
    {
        pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                        xStorage, *pPersist,
                                        EMBEDDEDOBJECTHELPER_MODE_READ,
                                        sal_False );
        xObjectResolver = pObjectHelper;
    }

    // Get the docshell, the model, and finally the model's component
    SwDocShell *pDocSh = rDoc.GetDocShell();
    OSL_ENSURE( pDocSh, "XMLReader::Read: got no doc shell" );
    if( !pDocSh )
        return ERR_SWG_READ_ERROR;
    uno::Reference< lang::XComponent > xModelComp( pDocSh->GetModel(), UNO_QUERY );
    OSL_ENSURE( xModelComp.is(),
            "XMLReader::Read: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_READ_ERROR;


    // create and prepare the XPropertySet that gets passed through
    // the components, and the XStatusIndicator that shows progress to
    // the user.

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { "ProgressRange", sizeof("ProgressRange")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressMax", sizeof("ProgressMax")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressCurrent", sizeof("ProgressCurrent")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "NumberStyles", sizeof("NumberStyles")-1, 0,
              &::getCppuType( (uno::Reference<container::XNameContainer> *) 0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "RecordChanges", sizeof("RecordChanges")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "ShowChanges", sizeof("ShowChanges")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "RedlineProtectionKey", sizeof("RedlineProtectionKey")-1, 0,
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
              new uno::Type(::getCppuType((Sequence<sal_Int8>*)0)),
#else
              &::getCppuType((Sequence<sal_Int8>*)0),
#endif
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "PrivateData", sizeof("PrivateData")-1, 0,
              &::getCppuType( (uno::Reference<XInterface> *)0 ),
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
        // properties for insert modes
        { "StyleInsertModeFamilies", sizeof("StyleInsertModeFamilies")-1, 0,
              &::getCppuType((Sequence<OUString>*)0),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StyleInsertModeOverwrite", sizeof("StyleInsertModeOverwrite")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "TextInsertModeRange", sizeof("TextInsertModeRange")-1, 0,
              &::getCppuType( (uno::Reference<text::XTextRange> *) 0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "AutoTextMode", sizeof("AutoTextMode")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "OrganizerMode", sizeof("OrganizerMode")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        // #i28749# - Add property, which indicates, if the
        // shape position attributes are given in horizontal left-to-right layout.
        // This is the case for the OpenOffice.org file format.
        { "ShapePositionInHoriL2R", sizeof("ShapePositionInHoriL2R")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        { "BuildId", sizeof("BuildId")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        // Add property, which indicates, if a text document in OpenOffice.org
        // file format is read.
        // Note: Text documents read via the binary filter are also finally
        //       read using the OpenOffice.org file format. Thus, e.g. for text
        //       documents in StarOffice 5.2 binary file format this property
        //       will be sal_True.
        { "TextDocInOOoFileFormat", sizeof("TextDocInOOoFileFormat")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "SourceStorage", sizeof("SourceStorage")-1, 0, &embed::XStorage::static_type(),
          ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    // ---- get BuildId from parent container if available

    uno::Reference< container::XChild > xChild( xModelComp, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< beans::XPropertySet > xParentSet( xChild->getParent(), uno::UNO_QUERY );
        if( xParentSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xParentSet->getPropertySetInfo() );
            const OUString sPropName("BuildId" );
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(sPropName) )
            {
                xInfoSet->setPropertyValue( sPropName, xParentSet->getPropertyValue(sPropName) );
            }
        }
    }

    // try to get an XStatusIndicator from the Medium
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    if (pDocSh->GetMedium())
    {
        SfxItemSet* pSet = pDocSh->GetMedium()->GetItemSet();
        if (pSet)
        {
            const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(
                pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );
            if (pItem)
            {
                pItem->GetValue() >>= xStatusIndicator;
            }
        }
    }


    // set progress range and start status indicator
    sal_Int32 nProgressRange(1000000);
    if (xStatusIndicator.is())
    {
        xStatusIndicator->start(SW_RESSTR(STR_STATSTR_SWGREAD), nProgressRange);
    }
    uno::Any aProgRange;
    aProgRange <<= nProgressRange;
    xInfoSet->setPropertyValue("ProgressRange", aProgRange);

    Reference< container::XNameAccess > xLateInitSettings( document::NamedPropertyValues::create(xContext), UNO_QUERY_THROW );
    beans::NamedValue aLateInitSettings(
        OUString( "LateInitSettings" ),
        makeAny( xLateInitSettings )
    );

    xInfoSet->setPropertyValue( "SourceStorage", Any( xStorage ) );

    // prepare filter arguments, WARNING: the order is important!
    Sequence<Any> aFilterArgs( 5 );
    Any *pArgs = aFilterArgs.getArray();
    *pArgs++ <<= xInfoSet;
    *pArgs++ <<= xStatusIndicator;
    *pArgs++ <<= xGraphicResolver;
    *pArgs++ <<= xObjectResolver;
    *pArgs++ <<= aLateInitSettings;

    Sequence<Any> aEmptyArgs( 3 );
    pArgs = aEmptyArgs.getArray();
    *pArgs++ <<= xInfoSet;
    *pArgs++ <<= xStatusIndicator;

    // prepare for special modes
    if( aOpt.IsFmtsOnly() )
    {
        sal_Int32 nCount =
            (aOpt.IsFrmFmts() ? 1 : 0) +
            (aOpt.IsPageDescs() ? 1 : 0) +
            (aOpt.IsTxtFmts() ? 2 : 0) +
            (aOpt.IsNumRules() ? 1 : 0);

        Sequence< OUString> aFamiliesSeq( nCount );
        OUString *pSeq = aFamiliesSeq.getArray();
        if( aOpt.IsFrmFmts() )
            // SFX_STYLE_FAMILY_FRAME;
            *pSeq++ = "FrameStyles";
        if( aOpt.IsPageDescs() )
            // SFX_STYLE_FAMILY_PAGE;
            *pSeq++ = "PageStyles";
        if( aOpt.IsTxtFmts() )
        {
            // (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
            *pSeq++ = "CharacterStyles";
            *pSeq++ = "ParagraphStyles";
        }
        if( aOpt.IsNumRules() )
            // SFX_STYLE_FAMILY_PSEUDO;
            *pSeq++ = "NumberingStyles";

        xInfoSet->setPropertyValue( "StyleInsertModeFamilies",
                                    makeAny(aFamiliesSeq) );

        sal_Bool bTmp = !aOpt.IsMerge();
        Any aAny;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xInfoSet->setPropertyValue( "StyleInsertModeOverwrite", aAny );
    }
    else if( bInsertMode )
    {
        const uno::Reference<text::XTextRange> xInsertTextRange =
            SwXTextRange::CreateXTextRange(rDoc, *rPaM.GetPoint(), 0);
        xInfoSet->setPropertyValue( "TextInsertModeRange",
                                    makeAny(xInsertTextRange) );
    }
    else
    {
        rPaM.GetBound(true).nContent.Assign(0, 0);
        rPaM.GetBound(false).nContent.Assign(0, 0);
    }

    if( IsBlockMode() )
    {
        sal_Bool bTmp = sal_True;
        Any aAny;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xInfoSet->setPropertyValue( "AutoTextMode", aAny );
    }
    if( IsOrganizerMode() )
    {
        sal_Bool bTmp = sal_True;
        Any aAny;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xInfoSet->setPropertyValue( "OrganizerMode", aAny );
    }

    // Set base URI
    // there is ambiguity which medium should be used here
    // for now the own medium has a preference
    SfxMedium* pMedDescrMedium = pMedium ? pMedium : pDocSh->GetMedium();
    OSL_ENSURE( pMedDescrMedium, "There is no medium to get MediaDescriptor from!\n" );

    xInfoSet->setPropertyValue( "BaseURI", makeAny( rBaseURL ) );

    // TODO/LATER: separate links from usual embedded objects
    OUString StreamPath;
    if( SFX_CREATE_MODE_EMBEDDED == rDoc.GetDocShell()->GetCreateMode() )
    {
        if ( pMedDescrMedium && pMedDescrMedium->GetItemSet() )
        {
            const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                pMedDescrMedium->GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
            if ( pDocHierarchItem )
                StreamPath = pDocHierarchItem->GetValue();
        }
        else
        {
            StreamPath = "dummyObjectName";
        }

        if( !StreamPath.isEmpty() )
        {
            xInfoSet->setPropertyValue( "StreamRelPath", makeAny( StreamPath ) );
        }
    }

    rDoc.acquire(); // prevent deletion
    sal_uInt32 nRet = 0;

    // save redline mode into import info property set
    Any aAny;
    sal_Bool bTmp;
    const OUString sShowChanges("ShowChanges");
    const OUString sRecordChanges("RecordChanges");
    const OUString sRedlineProtectionKey("RedlineProtectionKey");
    bTmp = IDocumentRedlineAccess::IsShowChanges( rDoc.GetRedlineMode() );
    aAny.setValue( &bTmp, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sShowChanges, aAny );
    bTmp = IDocumentRedlineAccess::IsRedlineOn(rDoc.GetRedlineMode());
    aAny.setValue( &bTmp, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sRecordChanges, aAny );
    aAny <<= rDoc.GetRedlinePassword();
    xInfoSet->setPropertyValue( sRedlineProtectionKey, aAny );


    // force redline mode to "none"
    rDoc.SetRedlineMode_intern( nsRedlineMode_t::REDLINE_NONE );

    const sal_Bool bOASIS = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
    // #i28749# - set property <ShapePositionInHoriL2R>
    {
        const sal_Bool bShapePositionInHoriL2R = !bOASIS;
        xInfoSet->setPropertyValue(
                "ShapePositionInHoriL2R",
                makeAny( bShapePositionInHoriL2R ) );
    }
    {
        const sal_Bool bTextDocInOOoFileFormat = !bOASIS;
        xInfoSet->setPropertyValue(
                "TextDocInOOoFileFormat",
                makeAny( bTextDocInOOoFileFormat ) );
    }

    sal_uInt32 nWarnRDF = 0;
    if ( !(IsOrganizerMode() || IsBlockMode() || aOpt.IsFmtsOnly() ||
           bInsertMode) )
    {
        // RDF metadata - must be read before styles/content
        // N.B.: embedded documents have their own manifest.rdf!
        try
        {
            const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(xModelComp,
                uno::UNO_QUERY_THROW);
            const uno::Reference<rdf::XURI> xBaseURI( ::sfx2::createBaseURI(
                xContext, xStorage, rBaseURL, StreamPath) );
            const uno::Reference<task::XInteractionHandler> xHandler(
                pDocSh->GetMedium()->GetInteractionHandler() );
            xDMA->loadMetadataFromStorage(xStorage, xBaseURI, xHandler);
        }
        catch (const lang::WrappedTargetException & e)
        {
            ucb::InteractiveAugmentedIOException iaioe;
            if (e.TargetException >>= iaioe)
            {
                // import error that was not ignored by InteractionHandler!
                nWarnRDF = ERR_SWG_READ_ERROR;
            }
            else
            {
                nWarnRDF = WARN_SWG_FEATURES_LOST; // uhh... something wrong?
            }
        }
        catch (uno::Exception &)
        {
            nWarnRDF = WARN_SWG_FEATURES_LOST; // uhh... something went wrong?
        }
    }

    // read storage streams

    // #i103539#: always read meta.xml for generator
    sal_uInt32 const nWarn = ReadThroughComponent(
        xStorage, xModelComp, "meta.xml", "Meta.xml", xContext,
        (bOASIS ? "com.sun.star.comp.Writer.XMLOasisMetaImporter"
                : "com.sun.star.comp.Writer.XMLMetaImporter"),
        aEmptyArgs, rName, false );

    sal_uInt32 nWarn2 = 0;
    if( !(IsOrganizerMode() || IsBlockMode() || aOpt.IsFmtsOnly() ||
          bInsertMode) )
    {
        nWarn2 = ReadThroughComponent(
            xStorage, xModelComp, "settings.xml", NULL, xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisSettingsImporter"
                    : "com.sun.star.comp.Writer.XMLSettingsImporter"),
            aFilterArgs, rName, false );
    }

    nRet = ReadThroughComponent(
        xStorage, xModelComp, "styles.xml", NULL, xContext,
        (bOASIS ? "com.sun.star.comp.Writer.XMLOasisStylesImporter"
                : "com.sun.star.comp.Writer.XMLStylesImporter"),
        aFilterArgs, rName, true );

    if( !nRet && !(IsOrganizerMode() || aOpt.IsFmtsOnly()) )
        nRet = ReadThroughComponent(
           xStorage, xModelComp, "content.xml", "Content.xml", xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisContentImporter"
                    : "com.sun.star.comp.Writer.XMLContentImporter"),
           aFilterArgs, rName, true );

    if( !(IsOrganizerMode() || IsBlockMode() || bInsertMode ||
          aOpt.IsFmtsOnly() ) )
    {
        OUString sStreamName("layout-cache");
        try
        {
            uno::Reference < io::XStream > xStm = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
            SvStream* pStrm2 = utl::UcbStreamHelper::CreateStream( xStm );
            if( !pStrm2->GetError() )
                rDoc.ReadLayoutCache( *pStrm2 );
            delete pStrm2;
        }
        catch ( uno::Exception& )
        {
        }
    }

    // Notify math objects
    if( bInsertMode )
        rDoc.PrtOLENotify( sal_False );
    else if ( rDoc.IsOLEPrtNotifyPending() )
        rDoc.PrtOLENotify( sal_True );

    nRet = nRet ? nRet : (nWarn ? nWarn : (nWarn2 ? nWarn2 : nWarnRDF ) );

    aOpt.ResetAllFmtsOnly();

    // redline password
    aAny = xInfoSet->getPropertyValue( sRedlineProtectionKey );
    Sequence<sal_Int8> aKey;
    aAny >>= aKey;
    rDoc.SetRedlinePassword( aKey );

    // restore redline mode from import info property set
    sal_Int16 nRedlineMode = nsRedlineMode_t::REDLINE_SHOW_INSERT;
    aAny = xInfoSet->getPropertyValue( sShowChanges );
    if ( *(sal_Bool*)aAny.getValue() )
        nRedlineMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;
    aAny = xInfoSet->getPropertyValue( sRecordChanges );
    if ( *(sal_Bool*)aAny.getValue() || (aKey.getLength() > 0) )
        nRedlineMode |= nsRedlineMode_t::REDLINE_ON;
    else
        nRedlineMode |= nsRedlineMode_t::REDLINE_NONE;

    // ... restore redline mode
    // (First set bogus mode to make sure the mode in SetRedlineMode()
    //  is different from it's previous mode.)
    rDoc.SetRedlineMode_intern((RedlineMode_t)( ~nRedlineMode ));
    rDoc.SetRedlineMode( (RedlineMode_t)( nRedlineMode ));

    lcl_EnsureValidPam( rPaM ); // move Pam into valid content

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;
    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;
    rDoc.release();

    if ( !bOASIS )
    {
        // #i44177# - assure that for documents in OpenOffice.org
        // file format the relation between outline numbering rule and styles is
        // filled-up accordingly.
        // Note: The OpenOffice.org file format, which has no content that applys
        //       a certain style, which is related to the outline numbering rule,
        //       has lost the information, that this certain style is related to
        //       the outline numbering rule.
        // #i70748# - only for templates
        if ( pMedium && pMedium->GetFilter() &&
             pMedium->GetFilter()->IsOwnTemplateFormat() )
        {
            lcl_AdjustOutlineStylesForOOo( rDoc );
        }
        // Fix #i58251#: Unfortunately is the static default different to SO7 behaviour,
        // so we have to set a dynamic default after importing SO7
        rDoc.SetDefault( SfxBoolItem( RES_ROW_SPLIT, sal_False ) );
    }

    rDoc.PropagateOutlineRule();

    // #i62875#
    if ( rDoc.get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) && !docfunc::ExistsDrawObjs( rDoc ) )
    {
        rDoc.set(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
    }

    // Convert all instances of <SdrOle2Obj> into <SdrGrafObj>, because the
    // Writer doesn't support such objects.
    lcl_ConvertSdrOle2ObjsToSdrGrafObjs( rDoc );

    // set BuildId on XModel for later OLE object loading
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySet > xModelSet( xModelComp, uno::UNO_QUERY );
        if( xModelSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xModelSetInfo( xModelSet->getPropertySetInfo() );
            const OUString sName("BuildId" );
            if( xModelSetInfo.is() && xModelSetInfo->hasPropertyByName(sName) )
            {
                xModelSet->setPropertyValue( sName, xInfoSet->getPropertyValue(sName) );
            }
        }
    }

    if (xStatusIndicator.is())
    {
        xStatusIndicator->end();
    }

    rDoc.GetIStyleAccess().clearCaches(); // Clear Automatic-Style-Caches(shared_pointer!)
    return nRet;
}

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
size_t XMLReader::GetSectionList( SfxMedium& rMedium,
                                  std::vector<OUString*>& rStrings ) const
{
    uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();
    uno::Reference < embed::XStorage > xStg2;
    if( ( xStg2 = rMedium.GetStorage() ).is() )
    {
        try
        {

            xml::sax::InputSource aParserInput;
            OUString sDocName( "content.xml" );
            aParserInput.sSystemId = sDocName;

            uno::Reference < io::XStream > xStm = xStg2->openStreamElement( sDocName, embed::ElementModes::READ );
            aParserInput.aInputStream = xStm->getInputStream();

            // get filter
            uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLSectionList( xContext, rStrings );

            // connect parser and filter
            uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);
            xParser->setDocumentHandler( xFilter );

            // parse
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }
        catch( packages::WrongPasswordException& )
        {
            // re throw ?
        }
    }
    return rStrings.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <o3tl/any.hxx>
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
#include <drawdoc.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <unotextrange.hxx>
#include <swmodule.hxx>
#include <SwXMLSectionList.hxx>
#include <statstr.hrc>

#include <SwStyleNameMapper.hxx>
#include <poolfmt.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <fmtrowsplt.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <sfx2/docfilt.hxx>
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
    if( rPam.GetContentNode() != nullptr )
    {
        // set proper point content
        if( rPam.GetContentNode() != rPam.GetPoint()->nContent.GetIdxReg() )
        {
            rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), 0 );
        }
        // else: point was already valid

        // if mark is invalid, we delete it
        if( ( rPam.GetContentNode( false ) == nullptr ) ||
            ( rPam.GetContentNode( false ) != rPam.GetMark()->nContent.GetIdxReg() ) )
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
        rPam.Move( fnMoveForward, GoInContent ); // go into content
    }
}

XMLReader::XMLReader()
{
}

int XMLReader::GetReaderType()
{
    return SW_STORAGE_READER;
}

namespace
{

/// read a component (file + filter version)
sal_Int32 ReadThroughComponent(
    uno::Reference<io::XInputStream> const & xInputStream,
    uno::Reference<XComponent> const & xModelComponent,
    const OUString& rStreamName,
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
    OSL_ENSURE(nullptr != pFilterName,"I need a service name for the component!");

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;
    aParserInput.aInputStream = xInputStream;

    // get parser
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(rxContext);
    SAL_INFO( "sw.filter", "parser created" );
    // get filter
    const OUString aFilterName(OUString::createFromAscii(pFilterName));
    uno::Reference< xml::sax::XDocumentHandler > xFilter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(aFilterName, rFilterArguments, rxContext),
        UNO_QUERY);
    SAL_WARN_IF(!xFilter.is(), "sw.filter", "Can't instantiate filter component: " << aFilterName);
    if( !xFilter.is() )
        return ERR_SWG_READ_ERROR;
    SAL_INFO( "sw.filter", "" << pFilterName << " created" );
    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // connect model and filter
    uno::Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );
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

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aError("SAX parse exception caught while importing:\n");
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif

        const OUString sErr( OUString::number( r.LineNumber )
            + ","
            + OUString::number( r.ColumnNumber ) );

        if( !rStreamName.isEmpty() )
        {
            return (new TwoStringErrorInfo(
                            (bMustBeSuccessfull ? ERR_FORMAT_FILE_ROWCOL
                                                    : WARN_FORMAT_FILE_ROWCOL),
                            rStreamName, sErr,
                            ErrorHandlerFlags::ButtonsOk | ErrorHandlerFlags::MessageError ))->GetErrorCode();
        }
        else
        {
            OSL_ENSURE( bMustBeSuccessfull, "Warnings are not supported" );
            return (new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                             ErrorHandlerFlags::ButtonsOk | ErrorHandlerFlags::MessageError ))->GetErrorCode();
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
        OStringBuffer aError("SAX exception caught while importing:\n");
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
        OStringBuffer aError("Zip exception caught while importing:\n");
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
        OStringBuffer aError("IO exception caught while importing:\n");
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
        OStringBuffer aError("uno exception caught while importing:\n");
        aError.append(OUStringToOString(r.Message,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aError.getStr());
#endif
        return ERR_SWG_READ_ERROR;
    }

    // success!
    return 0;
}

// read a component (storage version)
sal_Int32 ReadThroughComponent(
    uno::Reference<embed::XStorage> const & xStorage,
    uno::Reference<XComponent> const & xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    uno::Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessfull)
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    bool bContainsStream = false;
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
        if ( nullptr == pCompatibilityStreamName )
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

        auto b = o3tl::tryAccess<bool>(aAny);
        bool bEncrypted = b && *b;

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
        OSL_FAIL( "Error on import" );
        // TODO/LATER: error handling
    }

    return ERR_SWG_READ_ERROR;
}

}

// #i44177#
static void lcl_AdjustOutlineStylesForOOo(SwDoc& _rDoc)
{
    // array containing the names of the default outline styles ('Heading 1',
    // 'Heading 2', ..., 'Heading 10')
    OUString aDefOutlStyleNames[ MAXLEVEL ];
    {
        OUString sStyleName;
        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            sStyleName =
                SwStyleNameMapper::GetProgName( RES_POOLCOLL_HEADLINE1 + i,
                                                sStyleName );
            aDefOutlStyleNames[i] = sStyleName;
        }
    }

    // array indicating, which outline level already has a style assigned.
    bool aOutlineLevelAssigned[ MAXLEVEL ];
    // array of the default outline styles, which are created for the document.
    SwTextFormatColl* aCreatedDefaultOutlineStyles[ MAXLEVEL ];

    {
        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            aOutlineLevelAssigned[ i ] = false;
            aCreatedDefaultOutlineStyles[ i ] = nullptr;
        }
    }

    // determine, which outline level has already a style assigned and
    // which of the default outline styles is created.
    const SwTextFormatColls& rColls = *(_rDoc.GetTextFormatColls());
    for ( size_t n = 1; n < rColls.size(); ++n )
    {
        SwTextFormatColl* pColl = rColls[ n ];
        if ( pColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            aOutlineLevelAssigned[ pColl->GetAssignedOutlineStyleLevel() ] = true;
        }

        for ( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
        {
            if ( aCreatedDefaultOutlineStyles[ i ] == nullptr &&
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
             aCreatedDefaultOutlineStyles[ i ] != nullptr &&
             ! aCreatedDefaultOutlineStyles[ i ]->IsAssignedToListLevelOfOutlineStyle() )
        {
            // apply outline level at created default outline style
            aCreatedDefaultOutlineStyles[ i ]->AssignToListLevelOfOutlineStyle(i);

            // apply outline numbering rule, if none is set.
            const SfxPoolItem& rItem =
                aCreatedDefaultOutlineStyles[ i ]->GetFormatAttr( RES_PARATR_NUMRULE, false );
            if ( static_cast<const SwNumRuleItem&>(rItem).GetValue().isEmpty() )
            {
                SwNumRuleItem aItem( pOutlineRule->GetName() );
                aCreatedDefaultOutlineStyles[ i ]->SetFormatAttr( aItem );
            }
        }
    }
}

static void lcl_ConvertSdrOle2ObjsToSdrGrafObjs(SwDoc& _rDoc)
{
    if ( _rDoc.getIDocumentDrawModelAccess().GetDrawModel() &&
         _rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 ) )
    {
        const SdrPage& rSdrPage( *(_rDoc.getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 )) );

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
    // needed for relative URLs, but in clipboard copy/paste there may be none
    // and also there is the SwXMLTextBlocks special case
    SAL_INFO_IF(rBaseURL.isEmpty(), "sw.filter", "sw::XMLReader: no base URL");

    // Get service factory
    uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

    uno::Reference< io::XActiveDataSource > xSource;
    uno::Reference< XInterface > xPipe;
    uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = nullptr;
    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = nullptr;

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
                                                 SvXMLGraphicHelperMode::Read,
                                                 false );
    xGraphicResolver = pGraphicHelper;
    SfxObjectShell *pPersist = rDoc.GetPersist();
    if( pPersist )
    {
        pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                        xStorage, *pPersist,
                                        SvXMLEmbeddedObjectHelperMode::Read,
                                        false );
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
    comphelper::PropertyMapEntry const aInfoMap[] =
    {
        { OUString("ProgressRange"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressMax"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressCurrent"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("NumberStyles"), 0,
              cppu::UnoType<container::XNameContainer>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("RecordChanges"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("ShowChanges"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("RedlineProtectionKey"), 0,
              cppu::UnoType<Sequence<sal_Int8>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
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
        // properties for insert modes
        { OUString("StyleInsertModeFamilies"), 0,
              cppu::UnoType<Sequence<OUString>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StyleInsertModeOverwrite"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("TextInsertModeRange"), 0,
              cppu::UnoType<text::XTextRange>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("AutoTextMode"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("OrganizerMode"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        // #i28749# - Add property, which indicates, if the
        // shape position attributes are given in horizontal left-to-right layout.
        // This is the case for the OpenOffice.org file format.
        { OUString("ShapePositionInHoriL2R"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        { OUString("BuildId"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },

        // Add property, which indicates, if a text document in OpenOffice.org
        // file format is read.
        // Note: Text documents read via the binary filter are also finally
        //       read using the OpenOffice.org file format. Thus, e.g. for text
        //       documents in StarOffice 5.2 binary file format this property
        //       will be true.
        { OUString("TextDocInOOoFileFormat"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("SourceStorage"), 0, cppu::UnoType<embed::XStorage>::get(),
          css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    // get BuildId from parent container if available
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
    beans::NamedValue aLateInitSettings( "LateInitSettings", makeAny( xLateInitSettings ) );

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
    // cppcheck-suppress redundantAssignment
    pArgs = aEmptyArgs.getArray();
    *pArgs++ <<= xInfoSet;
    *pArgs++ <<= xStatusIndicator;

    // prepare for special modes
    if( aOpt.IsFormatsOnly() )
    {
        sal_Int32 nCount =
            (aOpt.IsFrameFormats() ? 1 : 0) +
            (aOpt.IsPageDescs() ? 1 : 0) +
            (aOpt.IsTextFormats() ? 2 : 0) +
            (aOpt.IsNumRules() ? 1 : 0);

        Sequence< OUString> aFamiliesSeq( nCount );
        OUString *pSeq = aFamiliesSeq.getArray();
        if( aOpt.IsFrameFormats() )
            // SfxStyleFamily::Frame;
            *pSeq++ = "FrameStyles";
        if( aOpt.IsPageDescs() )
            // SfxStyleFamily::Page;
            *pSeq++ = "PageStyles";
        if( aOpt.IsTextFormats() )
        {
            // (SfxStyleFamily::Char|SfxStyleFamily::Para);
            *pSeq++ = "CharacterStyles";
            *pSeq++ = "ParagraphStyles";
        }
        if( aOpt.IsNumRules() )
            // SfxStyleFamily::Pseudo;
            *pSeq++ = "NumberingStyles";

        xInfoSet->setPropertyValue( "StyleInsertModeFamilies",
                                    makeAny(aFamiliesSeq) );

        xInfoSet->setPropertyValue( "StyleInsertModeOverwrite", makeAny(!aOpt.IsMerge()) );
    }
    else if( bInsertMode )
    {
        const uno::Reference<text::XTextRange> xInsertTextRange =
            SwXTextRange::CreateXTextRange(rDoc, *rPaM.GetPoint(), nullptr);
        xInfoSet->setPropertyValue( "TextInsertModeRange",
                                    makeAny(xInsertTextRange) );
    }
    else
    {
        rPaM.GetBound().nContent.Assign(nullptr, 0);
        rPaM.GetBound(false).nContent.Assign(nullptr, 0);
    }

    if( IsBlockMode() )
    {
        xInfoSet->setPropertyValue( "AutoTextMode", makeAny(true) );
    }
    if( IsOrganizerMode() )
    {
        xInfoSet->setPropertyValue( "OrganizerMode", makeAny(true) );
    }

    // Set base URI
    // there is ambiguity which medium should be used here
    // for now the own medium has a preference
    SfxMedium* pMedDescrMedium = pMedium ? pMedium : pDocSh->GetMedium();
    OSL_ENSURE( pMedDescrMedium, "There is no medium to get MediaDescriptor from!\n" );

    xInfoSet->setPropertyValue( "BaseURI", makeAny( rBaseURL ) );

    // TODO/LATER: separate links from usual embedded objects
    OUString StreamPath;
    if( SfxObjectCreateMode::EMBEDDED == rDoc.GetDocShell()->GetCreateMode() )
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

    (void)rDoc.acquire(); // prevent deletion
    sal_uInt32 nRet = 0;

    // save redline mode into import info property set
    const OUString sShowChanges("ShowChanges");
    const OUString sRecordChanges("RecordChanges");
    const OUString sRedlineProtectionKey("RedlineProtectionKey");
    xInfoSet->setPropertyValue( sShowChanges,
        makeAny(IDocumentRedlineAccess::IsShowChanges(rDoc.getIDocumentRedlineAccess().GetRedlineFlags())) );
    xInfoSet->setPropertyValue( sRecordChanges,
        makeAny(IDocumentRedlineAccess::IsRedlineOn(rDoc.getIDocumentRedlineAccess().GetRedlineFlags())) );
    xInfoSet->setPropertyValue( sRedlineProtectionKey,
        makeAny(rDoc.getIDocumentRedlineAccess().GetRedlinePassword()) );

    // force redline mode to "none"
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::NONE );

    const bool bOASIS = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
    // #i28749# - set property <ShapePositionInHoriL2R>
    {
        const bool bShapePositionInHoriL2R = !bOASIS;
        xInfoSet->setPropertyValue(
                "ShapePositionInHoriL2R",
                makeAny( bShapePositionInHoriL2R ) );
    }
    {
        const bool bTextDocInOOoFileFormat = !bOASIS;
        xInfoSet->setPropertyValue(
                "TextDocInOOoFileFormat",
                makeAny( bTextDocInOOoFileFormat ) );
    }

    sal_uInt32 nWarnRDF = 0;
    if ( !(IsOrganizerMode() || IsBlockMode() || aOpt.IsFormatsOnly() ||
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
    if( !(IsOrganizerMode() || IsBlockMode() || aOpt.IsFormatsOnly() ||
          bInsertMode) )
    {
        nWarn2 = ReadThroughComponent(
            xStorage, xModelComp, "settings.xml", nullptr, xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisSettingsImporter"
                    : "com.sun.star.comp.Writer.XMLSettingsImporter"),
            aFilterArgs, rName, false );
    }

    nRet = ReadThroughComponent(
        xStorage, xModelComp, "styles.xml", nullptr, xContext,
        (bOASIS ? "com.sun.star.comp.Writer.XMLOasisStylesImporter"
                : "com.sun.star.comp.Writer.XMLStylesImporter"),
        aFilterArgs, rName, true );

    if( !nRet && !(IsOrganizerMode() || aOpt.IsFormatsOnly()) )
        nRet = ReadThroughComponent(
           xStorage, xModelComp, "content.xml", "Content.xml", xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisContentImporter"
                    : "com.sun.star.comp.Writer.XMLContentImporter"),
           aFilterArgs, rName, true );

    if( !(IsOrganizerMode() || IsBlockMode() || bInsertMode ||
          aOpt.IsFormatsOnly() ) )
    {
        try
        {
            uno::Reference < io::XStream > xStm = xStorage->openStreamElement( "layout-cache", embed::ElementModes::READ );
            SvStream* pStrm2 = utl::UcbStreamHelper::CreateStream( xStm );
            if( !pStrm2->GetError() )
                rDoc.ReadLayoutCache( *pStrm2 );
            delete pStrm2;
        }
        catch (const uno::Exception&)
        {
        }
    }

    // Notify math objects
    if( bInsertMode )
        rDoc.PrtOLENotify( false );
    else if ( rDoc.IsOLEPrtNotifyPending() )
        rDoc.PrtOLENotify( true );

    nRet = nRet ? nRet : (nWarn ? nWarn : (nWarn2 ? nWarn2 : nWarnRDF ) );

    aOpt.ResetAllFormatsOnly();

    // redline password
    Any aAny = xInfoSet->getPropertyValue( sRedlineProtectionKey );
    Sequence<sal_Int8> aKey;
    aAny >>= aKey;
    rDoc.getIDocumentRedlineAccess().SetRedlinePassword( aKey );

    // restore redline mode from import info property set
    RedlineFlags nRedlineFlags = RedlineFlags::ShowInsert;
    aAny = xInfoSet->getPropertyValue( sShowChanges );
    if ( *o3tl::doAccess<bool>(aAny) )
        nRedlineFlags |= RedlineFlags::ShowDelete;
    aAny = xInfoSet->getPropertyValue( sRecordChanges );
    if ( *o3tl::doAccess<bool>(aAny) || (aKey.getLength() > 0) )
        nRedlineFlags |= RedlineFlags::On;

    // ... restore redline mode
    // (First set bogus mode to make sure the mode in getIDocumentRedlineAccess().SetRedlineFlags()
    //  is different from its previous mode.)
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ~nRedlineFlags );
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags(  nRedlineFlags );

    lcl_EnsureValidPam( rPaM ); // move Pam into valid content

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = nullptr;
    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = nullptr;
    (void)rDoc.release();

    if ( !bOASIS )
    {
        // #i44177# - assure that for documents in OpenOffice.org
        // file format the relation between outline numbering rule and styles is
        // filled-up accordingly.
        // Note: The OpenOffice.org file format, which has no content that applies
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
        rDoc.SetDefault(SwFormatRowSplit(false));
    }

    rDoc.PropagateOutlineRule();

    // #i62875#
    if ( rDoc.getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) && !docfunc::ExistsDrawObjs( rDoc ) )
    {
        rDoc.getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE, false);
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
            const OUString sDocName( "content.xml" );
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

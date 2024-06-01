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

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/embed/EmbeddedObjectCreator.hpp>
#include <com/sun/star/embed/OOoEmbeddedObjectFactory.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <xmloff/prstylei.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmlimppr.hxx>
#include <TextCursorHelper.hxx>
#include <unoframe.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include "xmlimp.hxx"
#include "xmltbli.hxx"
#include "xmltexti.hxx"
#include "XMLRedlineImportHelper.hxx"
#include <xmloff/XMLFilterServiceNames.h>
#include <SwAppletImpl.hxx>
#include <ndole.hxx>
#include <docsh.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/embedhlp.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/frmdescr.hxx>
#include <tools/globname.hxx>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace xml::sax;

const std::pair<OUString, SvGUID> aServiceMap[] = {
    { XML_IMPORT_FILTER_WRITER, { SO3_SW_CLASSID } },
    { XML_IMPORT_FILTER_CALC, { SO3_SC_CLASSID } },
    { XML_IMPORT_FILTER_DRAW, { SO3_SDRAW_CLASSID } },
    { XML_IMPORT_FILTER_IMPRESS, { SO3_SIMPRESS_CLASSID } },
    { XML_IMPORT_FILTER_CHART, { SO3_SCH_CLASSID } },
    { XML_IMPORT_FILTER_MATH, { SO3_SM_CLASSID } },
};
static void lcl_putHeightAndWidth ( SfxItemSet &rItemSet,
        sal_Int32 nHeight, sal_Int32 nWidth,
        Size *pTwipSize = nullptr )
{
    if( nWidth > 0 && nHeight > 0 )
    {
        nWidth = o3tl::toTwips(nWidth, o3tl::Length::mm100);
        if( nWidth < MINFLY )
            nWidth = MINFLY;
        nHeight = o3tl::toTwips(nHeight, o3tl::Length::mm100);
        if( nHeight < MINFLY )
            nHeight = MINFLY;
        rItemSet.Put( SwFormatFrameSize( SwFrameSize::Fixed, nWidth, nHeight ) );
    }

    SwFormatAnchor aAnchor( RndStdIds::FLY_AT_CHAR );
    rItemSet.Put( aAnchor );

    if( pTwipSize )
    {
        pTwipSize->setWidth( nWidth );
        pTwipSize->setHeight( nHeight);
    }
}

static void lcl_setObjectVisualArea( const uno::Reference< embed::XEmbeddedObject >& xObj,
                                    sal_Int64 nAspect,
                                    const Size& aVisSize,
                                    const MapUnit& aUnit )
{
    if( !(xObj.is() && nAspect != embed::Aspects::MSOLE_ICON) )
        return;

    // convert the visual area to the objects units
    MapUnit aObjUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
    Size aObjVisSize = OutputDevice::LogicToLogic(aVisSize, MapMode(aUnit), MapMode(aObjUnit));
    awt::Size aSz;
    aSz.Width = aObjVisSize.Width();
    aSz.Height = aObjVisSize.Height();

    try
    {
        xObj->setVisualAreaSize( nAspect, aSz );
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "Couldn't set visual area of the object!" );
    }
}

SwXMLTextImportHelper::SwXMLTextImportHelper(
        const uno::Reference < XModel>& rModel,
        SwXMLImport& rImport,
        const uno::Reference<XPropertySet> & rInfoSet,
        bool bInsertM, bool bStylesOnlyM,
        bool bBlockM, bool bOrganizerM ) :
    XMLTextImportHelper( rModel, rImport, bInsertM, bStylesOnlyM, true/*bProgress*/,
                         bBlockM, bOrganizerM ),
    m_pRedlineHelper( nullptr )
{
    uno::Reference<XPropertySet> xDocPropSet( rModel, UNO_QUERY );
    m_pRedlineHelper = new XMLRedlineImportHelper(rImport,
        bInsertM || bBlockM, xDocPropSet, rInfoSet );
}

SwXMLTextImportHelper::~SwXMLTextImportHelper()
{
    // the redline helper destructor sets properties on the document
    // and may throw an exception while doing so... catch this
    try
    {
        delete m_pRedlineHelper;
    }
    catch ( const RuntimeException& )
    {
        // ignore
    }
}

SvXMLImportContext *SwXMLTextImportHelper::CreateTableChildContext(
                SvXMLImport& rImport,
                sal_Int32 /*nElement*/,
                const uno::Reference< XFastAttributeList > & xAttrList )
{
    return new SwXMLTableContext( static_cast<SwXMLImport&>(rImport), xAttrList );
}

bool SwXMLTextImportHelper::IsInHeaderFooter() const
{
    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(const_cast<SwXMLTextImportHelper *>(this)->GetCursor().get());
    SAL_WARN_IF(!pTextCursor, "sw.uno", "SwXTextCursor missing");
    SwDoc *pDoc = pTextCursor ? pTextCursor->GetDoc() : nullptr;

    return pDoc && pDoc->IsInHeaderFooter( pTextCursor->GetPaM()->GetPoint()->GetNode() );
}

static SwOLENode *lcl_GetOLENode( const SwFrameFormat *pFrameFormat )
{
    SwOLENode *pOLENd = nullptr;
    if( pFrameFormat )
    {
        const SwFormatContent& rContent = pFrameFormat->GetContent();
        const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
        pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetOLENode();
    }
    OSL_ENSURE( pOLENd, "Where is the OLE node" );
    return pOLENd;
}

uno::Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertOLEObject(
        SvXMLImport& rImport,
        const OUString& rHRef,
        const OUString& rStyleName,
        const OUString& rTableName,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    uno::Reference < XPropertySet > xPropSet;

    sal_Int32 nPos = rHRef.indexOf( ':' );
    if( -1 == nPos )
        return xPropSet;

    OUString aObjName( rHRef.copy( nPos+1) );

    if( aObjName.isEmpty() )
        return xPropSet;

    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(GetCursor().get());
    SAL_WARN_IF(!pTextCursor, "sw.uno", "SwXTextCursor missing");
    SwDoc *pDoc = static_cast<SwXMLImport&>(rImport).getDoc();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aItemSet( pDoc->GetAttrPool() );
    Size aTwipSize( 0, 0 );
    tools::Rectangle aVisArea( 0, 0, nWidth, nHeight );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth,
                           &aTwipSize );

    SwFrameFormat *pFrameFormat = nullptr;
    SwOLENode *pOLENd = nullptr;
    if( rHRef.startsWith("vnd.sun.star.ServiceName:") )
    {
        bool bInsert = false;
        SvGlobalName aClassName;
        for (const auto& [sFilterService, rCLASSID] : aServiceMap)
        {
            if (aObjName == sFilterService)
            {
                aClassName = SvGlobalName(rCLASSID);
                bInsert = true;
                break;
            }
        }

        if( bInsert )
        {
            uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
            try
            {
                // create object with desired ClassId
                uno::Sequence < sal_Int8 > aClass( aClassName.GetByteSequence() );
                uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );
                uno::Sequence<beans::PropertyValue> aObjArgs( comphelper::InitPropertySequence({
                        { "DefaultParentBaseURL", Any(GetXMLImport().GetBaseURL()) }
                    }));
                uno::Reference < embed::XEmbeddedObject > xObj( xFactory->createInstanceInitNew(
                    aClass, OUString(), xStorage, u"DummyName"_ustr, aObjArgs), uno::UNO_QUERY );
                if ( xObj.is() )
                {
                    //TODO/LATER: is it enough to only set the VisAreaSize?
                    lcl_setObjectVisualArea( xObj, embed::Aspects::MSOLE_CONTENT, aTwipSize, MapUnit::MapTwip );
                }

                if( pTextCursor )
                {
                    pFrameFormat = pDoc->getIDocumentContentOperations().InsertEmbObject(
                        *pTextCursor->GetPaM(),
                        ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                        &aItemSet);
                    pOLENd = lcl_GetOLENode( pFrameFormat );
                }

                if( pOLENd )
                    aObjName = pOLENd->GetOLEObj().GetCurrentPersistName();
            }
            catch ( uno::Exception& )
            {
            }
        }
    }
    else
    {
        // check whether an object with this name already exists in the document
        OUString aName;
        SwIterator<SwContentNode,SwFormatColl> aIter( *pDoc->GetDfltGrfFormatColl() );
        for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
        {
            SwOLENode* pExistingOLENd = pNd->GetOLENode();
            if( pExistingOLENd )
            {
                OUString aExistingName = pExistingOLENd->GetOLEObj().GetCurrentPersistName();
                if ( aExistingName == aObjName )
                {
                    OSL_FAIL( "The document contains duplicate object references, means it is partially broken, please let developers know how this document was generated!" );

                    OUString aTmpName = pDoc->GetPersist()->GetEmbeddedObjectContainer().CreateUniqueObjectName();
                    try
                    {
                        pDoc->GetPersist()->GetStorage()->copyElementTo( aObjName,
                                                                         pDoc->GetPersist()->GetStorage(),
                                                                         aTmpName );
                        aName = aTmpName;
                    }
                    catch ( uno::Exception& )
                    {
                        OSL_FAIL( "Couldn't create a copy of the object!" );
                    }

                    break;
                }
            }
        }

        if ( aName.isEmpty() )
            aName = aObjName;

        // the correct aspect will be set later
        // TODO/LATER: Actually it should be set here
        if( pTextCursor )
        {
            pFrameFormat = pDoc->getIDocumentContentOperations().InsertOLE( *pTextCursor->GetPaM(), aName, embed::Aspects::MSOLE_CONTENT, &aItemSet, nullptr );
            pOLENd = lcl_GetOLENode( pFrameFormat );
        }
        aObjName = aName;
    }

    if( !pFrameFormat )
        return xPropSet;

    if( IsInsertMode() )
    {
        if( !pOLENd )
            pOLENd = lcl_GetOLENode( pFrameFormat );
        if( pOLENd )
            pOLENd->SetOLESizeInvalid( true );
    }

    xPropSet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                *pDoc, pFrameFormat);
    if( pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
    {
        // req for z-order
        SwXFrame::GetOrCreateSdrObject(*static_cast<SwFlyFrameFormat*>(pFrameFormat));
    }
    if( !rTableName.isEmpty() )
    {
        const SwFormatContent& rContent = pFrameFormat->GetContent();
        const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
        SwOLENode *pOLENode = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetOLENode();
        OSL_ENSURE( pOLENode, "Where is the OLE node" );

        OUStringBuffer aBuffer( rTableName.getLength() );
        bool bQuoted = false;
        bool bEscape = false;
        bool bError = false;
        for( sal_Int32 i=0; i < rTableName.getLength(); i++ )
        {
            bool bEndOfNameFound = false;
            sal_Unicode c = rTableName[i];
            switch( c )
            {
            case '\'':
                if( bEscape )
                {
                    aBuffer.append( c );
                    bEscape = false;
                }
                else if( bQuoted )
                {
                    bEndOfNameFound = true;
                }
                else if( 0 == i )
                {
                    bQuoted = true;
                }
                else
                {
                    bError = true;
                }
                break;
            case '\\':
                if( bEscape )
                {
                    aBuffer.append( c );
                    bEscape = false;
                }
                else
                {
                    bEscape = true;
                }
                break;
            case ' ':
            case '.':
                if( !bQuoted )
                {
                    bEndOfNameFound = true;
                }
                else
                {
                    aBuffer.append( c );
                    bEscape = false;
                }
                break;
            default:
                {
                    aBuffer.append( c );
                    bEscape = false;
                }
                break;
            }
            if( bError || bEndOfNameFound )
                break;
        }
        if( !bError )
        {
            OUString sTableName( aBuffer.makeStringAndClear() );
            pOLENode->SetChartTableName( GetRenameMap().Get( XML_TEXT_RENAME_TYPE_TABLE, sTableName ) );
        }
    }

    sal_Int64 nDrawAspect = 0;
    const XMLPropStyleContext *pStyle = nullptr;
    bool bHasSizeProps = false;
    if( !rStyleName.isEmpty() )
    {
        pStyle = FindAutoFrameStyle( rStyleName );
        if( pStyle )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                pStyle->GetStyles()
                      ->GetImportPropertyMapper(pStyle->GetFamily());
            OSL_ENSURE( xImpPrMap.is(), "Where is the import prop mapper?" );
            if( xImpPrMap.is() )
            {
                rtl::Reference<XMLPropertySetMapper> rPropMapper =
                xImpPrMap->getPropertySetMapper();

                sal_Int32 nCount = pStyle->GetProperties().size();
                for( sal_Int32 i=0; i < nCount; i++ )
                {
                    const XMLPropertyState& rProp = pStyle->GetProperties()[i];
                    sal_Int32 nIdx = rProp.mnIndex;
                    if( -1 == nIdx )
                        continue;

                    switch( rPropMapper->GetEntryContextId(nIdx) )
                    {
                    case CTF_OLE_VIS_AREA_LEFT:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.SetPosX( nVal );
                        }
                        break;
                    case CTF_OLE_VIS_AREA_TOP:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.SetPosY( nVal );
                        }
                        break;
                    case CTF_OLE_VIS_AREA_WIDTH:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.setWidth( nVal );
                            bHasSizeProps = true;
                        }
                        break;
                    case CTF_OLE_VIS_AREA_HEIGHT:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.setHeight( nVal );
                            bHasSizeProps = true;
                        }
                        break;
                    case CTF_OLE_DRAW_ASPECT:
                        {
                            rProp.maValue >>= nDrawAspect;

                            if ( !nDrawAspect )
                                nDrawAspect = embed::Aspects::MSOLE_CONTENT;

                            if ( pOLENd )
                                pOLENd->GetOLEObj().GetObject().SetViewAspect( nDrawAspect );
                        }
                        break;
                    }
                }
            }
        }
    }

    if ( bHasSizeProps )
    {
        uno::Reference < embed::XEmbeddedObject > xObj =
                    pDoc->GetPersist()->GetEmbeddedObjectContainer().GetEmbeddedObject( aObjName );
        if( xObj.is() )
            lcl_setObjectVisualArea( xObj, ( nDrawAspect ? nDrawAspect : embed::Aspects::MSOLE_CONTENT ),
                                     aVisArea.GetSize(), MapUnit::Map100thMM );
    }

    return xPropSet;
}

uno::Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertOOoLink(
        SvXMLImport& rImport,
        const OUString& rHRef,
        const OUString& /*rStyleName*/,
        const OUString& /*rTableName*/,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    uno::Reference < XPropertySet > xPropSet;

    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(GetCursor().get());
    assert( pTextCursor && "SwXTextCursor missing" );
    SwDoc *pDoc = static_cast<SwXMLImport&>(rImport).getDoc();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aItemSet( pDoc->GetAttrPool() );
    Size aTwipSize( 0, 0 );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth,
                           &aTwipSize );

    // We'll need a (valid) URL. If we don't have do not insert the link and return early.
    // Copy URL into URL object on the way.
    INetURLObject aURLObj;
    bool bValidURL = !rHRef.isEmpty() &&
                     aURLObj.SetURL( URIHelper::SmartRel2Abs(
                                INetURLObject( GetXMLImport().GetBaseURL() ), rHRef ) );
    if( !bValidURL )
        return xPropSet;

    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    try
    {
        // create object with desired ClassId
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory =
                embed::OOoEmbeddedObjectFactory::create(::comphelper::getProcessComponentContext());

        uno::Sequence< beans::PropertyValue > aMediaDescriptor{ comphelper::makePropertyValue(
            u"URL"_ustr, aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE )) };

        if (SfxMedium* pMedium = pDoc->GetDocShell() ? pDoc->GetDocShell()->GetMedium() : nullptr)
        {
            uno::Reference< task::XInteractionHandler > xInteraction = pMedium->GetInteractionHandler();
            if ( xInteraction.is() )
            {
                aMediaDescriptor.realloc( 2 );
                auto pMediaDescriptor = aMediaDescriptor.getArray();
                pMediaDescriptor[1].Name = "InteractionHandler";
                pMediaDescriptor[1].Value <<= xInteraction;
            }

            const auto nLen = aMediaDescriptor.getLength() + 1;
            aMediaDescriptor.realloc(nLen);
            auto pMediaDescriptor = aMediaDescriptor.getArray();
            pMediaDescriptor[nLen - 1].Name = "Referer";
            pMediaDescriptor[nLen - 1].Value <<= pMedium->GetName();
        }

        uno::Reference < embed::XEmbeddedObject > xObj(
            xFactory->createInstanceLink(
                xStorage, u"DummyName"_ustr, aMediaDescriptor, uno::Sequence< beans::PropertyValue >() ),
            uno::UNO_QUERY_THROW );

        {
            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet );

            // TODO/LATER: in future may need a way to set replacement image url to the link ( may be even to the object ), needs oasis cws???

            xPropSet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat);
            if( pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
            {
                SwXFrame::GetOrCreateSdrObject(*
                        static_cast<SwFlyFrameFormat*>(pFrameFormat)); // req for z-order
            }
        }
    }
    catch ( uno::Exception& )
    {
    }

    // TODO/LATER: should the rStyleName and rTableName be handled as for usual embedded object?

    return xPropSet;
}

uno::Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertApplet(
        const OUString &rName,
        const OUString &rCode,
        bool bMayScript,
        const OUString& rHRef,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    uno::Reference < XPropertySet > xPropSet;
    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(GetCursor().get());
    assert( pTextCursor && "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aItemSet( pDoc->GetAttrPool() );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    SwApplet_Impl aAppletImpl ( std::move(aItemSet) );

    OUString sCodeBase;
    if( !rHRef.isEmpty() )
        sCodeBase = GetXMLImport().GetAbsoluteReference( rHRef );

    aAppletImpl.CreateApplet ( rCode, rName, bMayScript, sCodeBase, GetXMLImport().GetDocumentBase() );

    // set the size of the applet
    lcl_setObjectVisualArea( aAppletImpl.GetApplet(),
                            embed::Aspects::MSOLE_CONTENT,
                            Size( nWidth, nHeight ),
                            MapUnit::Map100thMM );

    SwFrameFormat *const pFrameFormat =
        pDoc->getIDocumentContentOperations().InsertEmbObject( *pTextCursor->GetPaM(),
        ::svt::EmbeddedObjectRef(aAppletImpl.GetApplet(), embed::Aspects::MSOLE_CONTENT),
        &aAppletImpl.GetItemSet());
    xPropSet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                *pDoc, pFrameFormat);
    if( pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
    {
        // req for z-order
        SwXFrame::GetOrCreateSdrObject(*static_cast<SwFlyFrameFormat*>(pFrameFormat));
    }

    return xPropSet;
}

uno::Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertPlugin(
        const OUString &rMimeType,
        const OUString& rHRef,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    uno::Reference < XPropertySet > xPropSet;
    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(GetCursor().get());
    assert( pTextCursor && "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aItemSet( pDoc->GetAttrPool() );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    // We'll need a (valid) URL, or we need a MIME type. If we don't have
    // either, do not insert plugin and return early. Copy URL into URL object
    // on the way.
    INetURLObject aURLObj;

    bool bValidURL = !rHRef.isEmpty() &&
                     aURLObj.SetURL( URIHelper::SmartRel2Abs( INetURLObject( GetXMLImport().GetBaseURL() ), rHRef ) );
    bool bValidMimeType = !rMimeType.isEmpty();
    if( !bValidURL && !bValidMimeType )
        return xPropSet;

    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    try
    {
        // create object with desired ClassId
        uno::Sequence < sal_Int8 > aClass( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence() );
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory =  embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );
        uno::Reference < embed::XEmbeddedObject > xObj( xFactory->createInstanceInitNew(
            aClass, OUString(), xStorage, u"DummyName"_ustr,
            uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );

        // set size to the object
        lcl_setObjectVisualArea( xObj,
                                embed::Aspects::MSOLE_CONTENT,
                                Size( nWidth, nHeight ),
                                MapUnit::Map100thMM );

        if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                if( bValidURL )
                    xSet->setPropertyValue(u"PluginURL"_ustr,
                        Any( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );
                if( bValidMimeType )
                    xSet->setPropertyValue(u"PluginMimeType"_ustr,
                        Any( rMimeType ) );
            }

            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet);
            xPropSet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat);
            if( pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
            {
                SwXFrame::GetOrCreateSdrObject(*
                        static_cast<SwFlyFrameFormat*>(pFrameFormat)); // req for z-order
            }
        }
    }
    catch ( uno::Exception& )
    {
    }

    return xPropSet;
}
uno::Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertFloatingFrame(
        const OUString& rName,
        const OUString& rHRef,
        const OUString& rStyleName,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    uno::Reference < XPropertySet > xPropSet;
    OTextCursorHelper* pTextCursor = dynamic_cast<OTextCursorHelper*>(GetCursor().get());
    assert( pTextCursor && "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aItemSet( pDoc->GetAttrPool() );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    ScrollingMode eScrollMode = ScrollingMode::Auto;
    bool bHasBorder = false;
    bool bIsBorderSet = false;
    Size aMargin( SIZE_NOT_SET, SIZE_NOT_SET );
    const XMLPropStyleContext *pStyle = nullptr;
    if( !rStyleName.isEmpty() )
    {
        pStyle = FindAutoFrameStyle( rStyleName );
        if( pStyle )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                pStyle->GetStyles()
                      ->GetImportPropertyMapper(pStyle->GetFamily());
            OSL_ENSURE( xImpPrMap.is(), "Where is the import prop mapper?" );
            if( xImpPrMap.is() )
            {
                rtl::Reference<XMLPropertySetMapper> rPropMapper =
                xImpPrMap->getPropertySetMapper();

                sal_Int32 nCount = pStyle->GetProperties().size();
                for( sal_Int32 i=0; i < nCount; i++ )
                {
                    const XMLPropertyState& rProp = pStyle->GetProperties()[i];
                    sal_Int32 nIdx = rProp.mnIndex;
                    if( -1 == nIdx )
                        continue;

                    switch( rPropMapper->GetEntryContextId(nIdx) )
                    {
                    case CTF_FRAME_DISPLAY_SCROLLBAR:
                        {
                            bool bYes = *o3tl::doAccess<bool>(rProp.maValue);
                            eScrollMode = bYes ? ScrollingMode::Yes : ScrollingMode::No;
                        }
                        break;
                    case CTF_FRAME_DISPLAY_BORDER:
                        {
                            bHasBorder = *o3tl::doAccess<bool>(rProp.maValue);
                            bIsBorderSet = true;
                        }
                        break;
                    case CTF_FRAME_MARGIN_HORI:
                        {
                            sal_Int32 nVal = SIZE_NOT_SET;
                            rProp.maValue >>= nVal;
                            aMargin.setWidth( nVal );
                        }
                        break;
                    case CTF_FRAME_MARGIN_VERT:
                        {
                            sal_Int32 nVal = SIZE_NOT_SET;
                            rProp.maValue >>= nVal;
                            aMargin.setHeight( nVal );
                        }
                        break;
                    }
                }
            }
        }
    }

    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    try
    {
        // create object with desired ClassId
        uno::Sequence < sal_Int8 > aClass( SvGlobalName( SO3_IFRAME_CLASSID ).GetByteSequence() );
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );
        uno::Reference < embed::XEmbeddedObject > xObj( xFactory->createInstanceInitNew(
            aClass, OUString(), xStorage, u"DummyName"_ustr,
            uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );

        // set size to the object
        lcl_setObjectVisualArea( xObj,
                                embed::Aspects::MSOLE_CONTENT,
                                Size( nWidth, nHeight ),
                                MapUnit::Map100thMM );

        if ( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                OUString sHRef = URIHelper::SmartRel2Abs(
                            INetURLObject( GetXMLImport().GetBaseURL() ), rHRef );

                if (INetURLObject(sHRef).IsExoticProtocol())
                    GetXMLImport().NotifyMacroEventRead();

                xSet->setPropertyValue(u"FrameURL"_ustr,
                    Any( sHRef ) );

                xSet->setPropertyValue(u"FrameName"_ustr,
                    Any( rName ) );

                if ( eScrollMode == ScrollingMode::Auto )
                    xSet->setPropertyValue(u"FrameIsAutoScroll"_ustr,
                        Any( true ) );
                else
                    xSet->setPropertyValue(u"FrameIsScrollingMode"_ustr,
                        Any( eScrollMode == ScrollingMode::Yes ) );

                if ( bIsBorderSet )
                    xSet->setPropertyValue(u"FrameIsBorder"_ustr,
                        Any( bHasBorder ) );
                else
                    xSet->setPropertyValue(u"FrameIsAutoBorder"_ustr,
                        Any( true ) );

                xSet->setPropertyValue(u"FrameMarginWidth"_ustr,
                    Any( sal_Int32( aMargin.Width() ) ) );

                xSet->setPropertyValue(u"FrameMarginHeight"_ustr,
                    Any( sal_Int32( aMargin.Height() ) ) );
            }

            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet);
            xPropSet = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat);
            if( pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
            {
                // req for z-order
                SwXFrame::GetOrCreateSdrObject(*
                        static_cast<SwFlyFrameFormat*>(pFrameFormat));
            }
        }
    }
    catch ( uno::Exception& )
    {
    }

    return xPropSet;
}

void SwXMLTextImportHelper::endAppletOrPlugin(
        const uno::Reference < XPropertySet > &rPropSet,
        std::map < const OUString, OUString > &rParamMap)
{
    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    SwXFrame* pFrame = dynamic_cast<SwXFrame*>(rPropSet.get());
    assert(pFrame && "SwXFrame missing");
    SwFrameFormat *pFrameFormat = pFrame->GetFrameFormat();
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
    SwOLENode *pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTextNode()->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();

    uno::Reference < embed::XEmbeddedObject > xEmbObj( rOLEObj.GetOleRef() );
    if ( !svt::EmbeddedObjectRef::TryRunningState( xEmbObj ) )
        return;

    uno::Reference < beans::XPropertySet > xSet( xEmbObj->getComponent(), uno::UNO_QUERY );
    if ( !xSet.is() )
        return;

    const sal_Int32 nCount = rParamMap.size();
    uno::Sequence< beans::PropertyValue > aCommandSequence( nCount );

    std::transform(rParamMap.begin(), rParamMap.end(), aCommandSequence.getArray(),
                   [](const auto& rParam)
                   {
                       return beans::PropertyValue(/* Name   */ rParam.first,
                                                   /* Handle */ -1,
                                                   /* Value  */ uno::Any(rParam.second),
                                                   /* State  */ beans::PropertyState_DIRECT_VALUE);
                   });

    // unfortunately the names of the properties are depending on the object
    OUString aParaName(u"AppletCommands"_ustr);
    try
    {
        xSet->setPropertyValue( aParaName, Any( aCommandSequence ) );
    }
    catch ( uno::Exception& )
    {
        aParaName = "PluginCommands";
        try
        {
            xSet->setPropertyValue( aParaName, Any( aCommandSequence ) );
        }
        catch ( uno::Exception& )
        {
        }
    }
}

// redlining helper methods
// (override to provide the real implementation)
void SwXMLTextImportHelper::RedlineAdd(
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const util::DateTime& rDateTime,
    const OUString& rMovedID,
    bool bMergeLastPara)
{
    // create redline helper on demand
    OSL_ENSURE(nullptr != m_pRedlineHelper, "helper should have been created in constructor");
    if (nullptr != m_pRedlineHelper)
        m_pRedlineHelper->Add(rType, rId, rAuthor, rComment, rDateTime, rMovedID,
                            bMergeLastPara);
}

uno::Reference<XTextCursor> SwXMLTextImportHelper::RedlineCreateText(
    uno::Reference<XTextCursor> & rOldCursor,
    const OUString& rId)
{
    uno::Reference<XTextCursor> xRet;

    if (nullptr != m_pRedlineHelper)
    {
        xRet = m_pRedlineHelper->CreateRedlineTextSection(rOldCursor, rId);
    }

    return xRet;
}

void SwXMLTextImportHelper::RedlineSetCursor(
    const OUString& rId,
    bool bStart,
    bool bIsOutsideOfParagraph)
{
    if (nullptr != m_pRedlineHelper) {
        uno::Reference<XTextRange> xTextRange( GetCursor()->getStart() );
        m_pRedlineHelper->SetCursor(rId, bStart, xTextRange,
                                  bIsOutsideOfParagraph);
    }
    // else: ignore redline (wasn't added before, else we'd have a helper)
}

void SwXMLTextImportHelper::RedlineAdjustStartNodeCursor()
{
    OUString rId = GetOpenRedlineId();
    if ((nullptr != m_pRedlineHelper) && !rId.isEmpty())
    {
        m_pRedlineHelper->AdjustStartNodeCursor(rId);
        ResetOpenRedlineId();
    }
    // else: ignore redline (wasn't added before, or no open redline ID
}

void SwXMLTextImportHelper::SetShowChanges( bool bShowChanges )
{
    if ( nullptr != m_pRedlineHelper )
        m_pRedlineHelper->SetShowChanges( bShowChanges );
}

void SwXMLTextImportHelper::SetRecordChanges( bool bRecordChanges )
{
    if ( nullptr != m_pRedlineHelper )
        m_pRedlineHelper->SetRecordChanges( bRecordChanges );
}

void SwXMLTextImportHelper::SetChangesProtectionKey(
    const Sequence<sal_Int8> & rKey )
{
    if ( nullptr != m_pRedlineHelper )
        m_pRedlineHelper->SetProtectionKey( rKey );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <com/sun/star/embed/EmbeddedObjectCreator.hpp>
#include <com/sun/star/embed/OOoEmbeddedObjectFactory.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <o3tl/any.hxx>
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
#include <unocrsr.hxx>
#include <TextCursorHelper.hxx>
#include <unoframe.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <unocoll.hxx>
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
#include <calbck.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/embedhlp.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/frmdescr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace xml::sax;

struct XMLServiceMapEntry_Impl
{
    const sal_Char *sFilterService;
    sal_Int32 const nFilterServiceLen;

    sal_uInt32 const n1;
    sal_uInt16  n2, n3;
    sal_uInt8   n4, n5, n6, n7, n8, n9, n10, n11;
};

#define SERVICE_MAP_ENTRY( app, s ) \
    { XML_IMPORT_FILTER_##app, sizeof(XML_IMPORT_FILTER_##app)-1, \
      SO3_##s##_CLASSID }

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( WRITER, SW ),
    SERVICE_MAP_ENTRY( CALC, SC ),
    SERVICE_MAP_ENTRY( DRAW, SDRAW ),
    SERVICE_MAP_ENTRY( IMPRESS, SIMPRESS ),
    SERVICE_MAP_ENTRY( CHART, SCH ),
    SERVICE_MAP_ENTRY( MATH, SM ),
    { nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static void lcl_putHeightAndWidth ( SfxItemSet &rItemSet,
        sal_Int32 nHeight, sal_Int32 nWidth,
        Size *pTwipSize = nullptr )
{
    if( nWidth > 0 && nHeight > 0 )
    {
        nWidth = convertMm100ToTwip( nWidth );
        if( nWidth < MINFLY )
            nWidth = MINFLY;
        nHeight = convertMm100ToTwip( nHeight );
        if( nHeight < MINFLY )
            nHeight = MINFLY;
        rItemSet.Put( SwFormatFrameSize( ATT_FIX_SIZE, nWidth, nHeight ) );
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
    if( xObj.is() && nAspect != embed::Aspects::MSOLE_ICON )
    {
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
}

SwXMLTextImportHelper::SwXMLTextImportHelper(
        const uno::Reference < XModel>& rModel,
        SvXMLImport& rImport,
        const uno::Reference<XPropertySet> & rInfoSet,
        bool bInsertM, bool bStylesOnlyM,
        bool bBlockM, bool bOrganizerM ) :
    XMLTextImportHelper( rModel, rImport, bInsertM, bStylesOnlyM, true/*bProgress*/,
                         bBlockM, bOrganizerM ),
    pRedlineHelper( nullptr )
{
    uno::Reference<XPropertySet> xDocPropSet( rModel, UNO_QUERY );
    pRedlineHelper = new XMLRedlineImportHelper(rImport,
        bInsertM || bBlockM, xDocPropSet, rInfoSet );
}

SwXMLTextImportHelper::~SwXMLTextImportHelper()
{
    // the redline helper destructor sets properties on the document
    // and may throw an exception while doing so... catch this
    try
    {
        delete pRedlineHelper;
    }
    catch ( const RuntimeException& )
    {
        // ignore
    }
}

SvXMLImportContext *SwXMLTextImportHelper::CreateTableChildContext(
                SvXMLImport& rImport,
                sal_uInt16 nPrefix, const OUString& rLocalName,
                const uno::Reference< XAttributeList > & xAttrList )
{
    return new SwXMLTableContext(
                static_cast<SwXMLImport&>(rImport), nPrefix, rLocalName, xAttrList );
}

bool SwXMLTextImportHelper::IsInHeaderFooter() const
{
    uno::Reference<XUnoTunnel> xCursorTunnel(
            const_cast<SwXMLTextImportHelper *>(this)->GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    SAL_WARN_IF(!pTextCursor, "sw.uno", "SwXTextCursor missing");
    SwDoc *pDoc = pTextCursor ? pTextCursor->GetDoc() : nullptr;

    return pDoc && pDoc->IsInHeaderFooter( pTextCursor->GetPaM()->GetPoint()->nNode );
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

    uno::Reference<XUnoTunnel> xCursorTunnel( GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    SAL_WARN_IF(!pTextCursor, "sw.uno", "SwXTextCursor missing");
    SwDoc *pDoc = SwImport::GetDocFromXMLImport( rImport );

    SfxItemSet aItemSet( pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
                         RES_FRMATR_END>{} );
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
        const XMLServiceMapEntry_Impl *pEntry = aServiceMap;
        while( pEntry->sFilterService )
        {
            if( aObjName.equalsAsciiL( pEntry->sFilterService,
                                     pEntry->nFilterServiceLen ) )
            {
                aClassName = SvGlobalName( pEntry->n1, pEntry->n2,
                                           pEntry->n3, pEntry->n4,
                                           pEntry->n5, pEntry->n6,
                                           pEntry->n7, pEntry->n8,
                                           pEntry->n9, pEntry->n10,
                                           pEntry->n11  );
                bInsert = true;
                break;
            }
            pEntry++;
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
                uno::Reference < embed::XEmbeddedObject > xObj =
                    uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitNew(
                    aClass, OUString(), xStorage, "DummyName", aObjArgs), uno::UNO_QUERY );
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

    xPropSet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                *pDoc, pFrameFormat), uno::UNO_QUERY);
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
                            aVisArea.setX( nVal );
                        }
                        break;
                    case CTF_OLE_VIS_AREA_TOP:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.setY( nVal );
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

    uno::Reference<XUnoTunnel> xCursorTunnel( GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    SwDoc *pDoc = SwImport::GetDocFromXMLImport( rImport );

    SfxItemSet aItemSet( pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
                         RES_FRMATR_END>{} );
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

        uno::Sequence< beans::PropertyValue > aMediaDescriptor( 1 );
        aMediaDescriptor[0].Name = "URL";
        aMediaDescriptor[0].Value <<= aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        if (SfxMedium* pMedium = pDoc->GetDocShell() ? pDoc->GetDocShell()->GetMedium() : nullptr)
        {
            uno::Reference< task::XInteractionHandler > xInteraction = pMedium->GetInteractionHandler();
            if ( xInteraction.is() )
            {
                aMediaDescriptor.realloc( 2 );
                aMediaDescriptor[1].Name = "InteractionHandler";
                aMediaDescriptor[1].Value <<= xInteraction;
            }

            const auto nLen = aMediaDescriptor.getLength() + 1;
            aMediaDescriptor.realloc(nLen);
            aMediaDescriptor[nLen - 1].Name = "Referer";
            aMediaDescriptor[nLen - 1].Value <<= pMedium->GetName();
        }

        uno::Reference < embed::XEmbeddedObject > xObj(
            xFactory->createInstanceLink(
                xStorage, "DummyName", aMediaDescriptor, uno::Sequence< beans::PropertyValue >() ),
            uno::UNO_QUERY_THROW );

        {
            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet );

            // TODO/LATER: in future may need a way to set replacement image url to the link ( may be even to the object ), needs oasis cws???

            xPropSet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat), uno::UNO_QUERY);
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
    uno::Reference<XUnoTunnel> xCursorTunnel( GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
                         RES_FRMATR_END>{} );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    SwApplet_Impl aAppletImpl ( aItemSet );

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
    xPropSet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                *pDoc, pFrameFormat), uno::UNO_QUERY);
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
    uno::Reference<XUnoTunnel> xCursorTunnel( GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
            sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
                         RES_FRMATR_END>{} );
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
        uno::Reference < embed::XEmbeddedObject > xObj =
            uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitNew(
            aClass, OUString(), xStorage, "DummyName",
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
                    xSet->setPropertyValue("PluginURL",
                        makeAny( aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) );
                if( bValidMimeType )
                    xSet->setPropertyValue("PluginMimeType",
                        makeAny( rMimeType ) );
            }

            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet);
            xPropSet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat), uno::UNO_QUERY);
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
    uno::Reference<XUnoTunnel> xCursorTunnel( GetCursor(), UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for Cursor");
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );
    SwDoc *pDoc = pTextCursor->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
                         RES_FRMATR_END>{} );
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
        uno::Reference < embed::XEmbeddedObject > xObj =
            uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitNew(
            aClass, OUString(), xStorage, "DummyName",
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
                xSet->setPropertyValue("FrameURL",
                    makeAny( URIHelper::SmartRel2Abs(
                            INetURLObject( GetXMLImport().GetBaseURL() ), rHRef ) ) );

                xSet->setPropertyValue("FrameName",
                    makeAny( rName ) );

                if ( eScrollMode == ScrollingMode::Auto )
                    xSet->setPropertyValue("FrameIsAutoScroll",
                        makeAny( true ) );
                else
                    xSet->setPropertyValue("FrameIsScrollingMode",
                        makeAny( eScrollMode == ScrollingMode::Yes ) );

                if ( bIsBorderSet )
                    xSet->setPropertyValue("FrameIsBorder",
                        makeAny( bHasBorder ) );
                else
                    xSet->setPropertyValue("FrameIsAutoBorder",
                        makeAny( true ) );

                xSet->setPropertyValue("FrameMarginWidth",
                    makeAny( sal_Int32( aMargin.Width() ) ) );

                xSet->setPropertyValue("FrameMarginHeight",
                    makeAny( sal_Int32( aMargin.Height() ) ) );
            }

            SwFrameFormat *const pFrameFormat =
                pDoc->getIDocumentContentOperations().InsertEmbObject(
                    *pTextCursor->GetPaM(),
                    ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                    &aItemSet);
            xPropSet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pDoc, pFrameFormat), uno::UNO_QUERY);
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

    uno::Reference<XUnoTunnel> xCursorTunnel( rPropSet, UNO_QUERY );
    assert(xCursorTunnel.is() && "missing XUnoTunnel for embedded");
    SwXFrame *pFrame = reinterpret_cast< SwXFrame * >(
                sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( SwXFrame::getUnoTunnelId() )));
    OSL_ENSURE( pFrame, "SwXFrame missing" );
    SwFrameFormat *pFrameFormat = pFrame->GetFrameFormat();
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    const SwNodeIndex *pNdIdx = rContent.GetContentIdx();
    SwOLENode *pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTextNode()->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();

    uno::Reference < embed::XEmbeddedObject > xEmbObj( rOLEObj.GetOleRef() );
    if ( svt::EmbeddedObjectRef::TryRunningState( xEmbObj ) )
    {
        uno::Reference < beans::XPropertySet > xSet( xEmbObj->getComponent(), uno::UNO_QUERY );
        if ( xSet.is() )
        {
            const sal_Int32 nCount = rParamMap.size();
            uno::Sequence< beans::PropertyValue > aCommandSequence( nCount );

            sal_Int32 nIndex=0;
            for (const auto& rParam : rParamMap )
            {
                aCommandSequence[nIndex].Name = rParam.first;
                aCommandSequence[nIndex].Handle = -1;
                aCommandSequence[nIndex].Value <<= rParam.second;
                aCommandSequence[nIndex].State = beans::PropertyState_DIRECT_VALUE;
                ++nIndex;
            }

            // unfortunately the names of the properties are depending on the object
            OUString aParaName("AppletCommands");
            try
            {
                xSet->setPropertyValue( aParaName, makeAny( aCommandSequence ) );
            }
            catch ( uno::Exception& )
            {
                aParaName = "PluginCommands";
                try
                {
                    xSet->setPropertyValue( aParaName, makeAny( aCommandSequence ) );
                }
                catch ( uno::Exception& )
                {
                }
            }
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
    bool bMergeLastPara)
{
    // create redline helper on demand
    OSL_ENSURE(nullptr != pRedlineHelper, "helper should have been created in constructor");
    if (nullptr != pRedlineHelper)
        pRedlineHelper->Add(rType, rId, rAuthor, rComment, rDateTime,
                            bMergeLastPara);
}

uno::Reference<XTextCursor> SwXMLTextImportHelper::RedlineCreateText(
    uno::Reference<XTextCursor> & rOldCursor,
    const OUString& rId)
{
    uno::Reference<XTextCursor> xRet;

    if (nullptr != pRedlineHelper)
    {
        xRet = pRedlineHelper->CreateRedlineTextSection(rOldCursor, rId);
    }

    return xRet;
}

void SwXMLTextImportHelper::RedlineSetCursor(
    const OUString& rId,
    bool bStart,
    bool bIsOutsideOfParagraph)
{
    if (nullptr != pRedlineHelper) {
        uno::Reference<XTextRange> xTextRange( GetCursor()->getStart() );
        pRedlineHelper->SetCursor(rId, bStart, xTextRange,
                                  bIsOutsideOfParagraph);
    }
    // else: ignore redline (wasn't added before, else we'd have a helper)
}

void SwXMLTextImportHelper::RedlineAdjustStartNodeCursor(
    bool /*bStart*/)
{
    OUString rId = GetOpenRedlineId();
    if ((nullptr != pRedlineHelper) && !rId.isEmpty())
    {
        pRedlineHelper->AdjustStartNodeCursor(rId);
        ResetOpenRedlineId();
    }
    // else: ignore redline (wasn't added before, or no open redline ID
}

void SwXMLTextImportHelper::SetShowChanges( bool bShowChanges )
{
    if ( nullptr != pRedlineHelper )
        pRedlineHelper->SetShowChanges( bShowChanges );
}

void SwXMLTextImportHelper::SetRecordChanges( bool bRecordChanges )
{
    if ( nullptr != pRedlineHelper )
        pRedlineHelper->SetRecordChanges( bRecordChanges );
}

void SwXMLTextImportHelper::SetChangesProtectionKey(
    const Sequence<sal_Int8> & rKey )
{
    if ( nullptr != pRedlineHelper )
        pRedlineHelper->SetProtectionKey( rKey );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

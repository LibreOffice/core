/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <comphelper/classids.hxx>
#endif
#ifndef _EMBOBJ_HXX 
#include <bf_so3/embobj.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_ 
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_ 
#include <bf_xmloff/prstylei.hxx>
#endif
#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX 
#include <bf_xmloff/maptype.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX 
#include <bf_xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <bf_xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <bf_xmloff/i18nmap.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOCRSR_HXX
#include "unocrsr.hxx"
#endif
#ifndef _UNOOBJ_HXX
#include "unoobj.hxx"
#endif
#ifndef _UNOFRAME_HXX
#include "unoframe.hxx"
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX 
#include "doc.hxx"
#endif
#ifndef _UNOCOLL_HXX
#include "unocoll.hxx"
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif
#ifndef _XMLTEXTI_HXX
#include "xmltexti.hxx"
#endif
#ifndef _XMLREDLINEIMPORTHELPER_HXX
#include "XMLRedlineImportHelper.hxx"
#endif
#ifndef _XMLOFF_XMLFILTERSERVICENAMES_H
#include <bf_xmloff/XMLFilterServiceNames.h>
#endif

#ifndef _SW_APPLET_IMPL_HXX
#include <SwAppletImpl.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif

#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif

#include <bf_sfx2/frmdescr.hxx>

// for locking SolarMutex: svapp + mutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {
 


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;


struct XMLServiceMapEntry_Impl
{
    const sal_Char *sFilterService;
    sal_Int32	   nFilterServiceLen;

    sal_uInt32	n1;
    sal_uInt16	n2, n3;
    sal_uInt8	n4, n5, n6, n7, n8, n9, n10, n11;
};

#define SERVICE_MAP_ENTRY( app, s ) \
    { XML_IMPORT_FILTER_##app, sizeof(XML_IMPORT_FILTER_##app)-1, \
      BF_SO3_##s##_CLASSID	}

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( WRITER, SW ),
    SERVICE_MAP_ENTRY( CALC, SC ),
    SERVICE_MAP_ENTRY( DRAW, SDRAW ),
    SERVICE_MAP_ENTRY( IMPRESS, SIMPRESS ),
    SERVICE_MAP_ENTRY( CHART, SCH ),
    SERVICE_MAP_ENTRY( MATH, SM ),
    { 0, 0, 0, 0 }
};
static void lcl_putHeightAndWidth ( SfxItemSet &rItemSet, 
        sal_Int32 nHeight, sal_Int32 nWidth,
        long *pTwipHeight=0, long *pTwipWidth=0 )
{
    if( nWidth > 0 && nHeight > 0 )
    {
        nWidth = MM100_TO_TWIP( nWidth );
        if( nWidth < MINFLY )
            nWidth = MINFLY;
        nHeight = MM100_TO_TWIP( nHeight );
        if( nHeight < MINFLY )
            nHeight = MINFLY;
        rItemSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth, nHeight ) );
    }

    SwFmtAnchor aAnchor( FLY_AUTO_CNTNT );
    rItemSet.Put( aAnchor );

    if( pTwipWidth )
        *pTwipWidth = nWidth;
    if( pTwipHeight )
        *pTwipHeight = nHeight;
}

SwXMLTextImportHelper::SwXMLTextImportHelper(
        const Reference < XModel>& rModel,
        SvXMLImport& rImport,
        const Reference<XPropertySet> & rInfoSet,
        sal_Bool bInsertM, sal_Bool bStylesOnlyM, sal_Bool bProgress,
        sal_Bool bBlockM, sal_Bool bOrganizerM, 
        sal_Bool bPreserveRedlineMode ) :
    XMLTextImportHelper( rModel, rImport, bInsertM, bStylesOnlyM, bProgress, 
                         bBlockM, bOrganizerM ),
    pRedlineHelper( NULL )
{
    Reference<XPropertySet> xDocPropSet( rModel, UNO_QUERY );
    pRedlineHelper = new XMLRedlineImportHelper(
        bInsertM || bBlockM, xDocPropSet, rInfoSet );
}

SwXMLTextImportHelper::~SwXMLTextImportHelper()
{
    // #90463# the redline helper destructor sets properties on the document
    //         and may through an exception while doing so... catch this
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
                const Reference< XAttributeList > & xAttrList )
{
    return new SwXMLTableContext( 
                (SwXMLImport&)rImport, nPrefix, rLocalName, xAttrList );
}

sal_Bool SwXMLTextImportHelper::IsInHeaderFooter() const
{
    Reference<XUnoTunnel> xCrsrTunnel(
            ((SwXMLTextImportHelper *)this)->GetCursor(), UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = 
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    return pDoc->IsInHeaderFooter( pTxtCrsr->GetPaM()->GetPoint()->nNode );
}

SwOLENode *lcl_GetOLENode( const SwFrmFmt *pFrmFmt )
{
    SwOLENode *pOLENd = 0;
    if( pFrmFmt )
    {
        const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
        const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
        pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetOLENode();
    }
    ASSERT( pOLENd, "Where is the OLE node" );
    return pOLENd;
}

Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertOLEObject(
           SvXMLImport& rImport,
        const OUString& rHRef,
        const OUString& rStyleName,
        const OUString& rTblName,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference < XPropertySet > xPropSet;

    sal_Int32 nPos = rHRef.indexOf( ':' );
    if( -1 == nPos )
        return xPropSet;

    OUString aObjName( rHRef.copy( nPos+1) );

    if( !aObjName.getLength() )
        return xPropSet;

    Reference<XUnoTunnel> xCrsrTunnel( GetCursor(), UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = 
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                         RES_FRMATR_END );
    Size aTwipSize( 0, 0 );
    Rectangle aVisArea( 0, 0, nWidth, nHeight );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth, 
                           &aTwipSize.Height(), &aTwipSize.Width() );

    SwFrmFmt *pFrmFmt = 0;
    SwOLENode *pOLENd = 0;
    if( rHRef.copy( 0, nPos ).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.ServiceName") ) )
    {
        sal_Bool bInsert = sal_False;
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
                bInsert = sal_True;
                break;
            }
            pEntry++;
        }

        if( bInsert )
        {
            SvStorageRef aStor = new SvStorage( aEmptyStr );
            SvInPlaceObjectRef xIPObj =
                &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                        aClassName, aStor );
            if( xIPObj.Is() )
            {
                aVisArea.SetSize( aTwipSize );
                aVisArea = OutputDevice::LogicToLogic(
                            aVisArea, MAP_TWIP, xIPObj->GetMapUnit() );
                xIPObj->SetVisArea( aVisArea );
            }
            pFrmFmt = pDoc->Insert( *pTxtCrsr->GetPaM(), xIPObj, &aItemSet );

            pOLENd = lcl_GetOLENode( pFrmFmt );
            if( pOLENd )
                aObjName = pOLENd->GetOLEObj().GetName();
        }
    }
    else
    {
        String aName( aObjName );
        pFrmFmt = pDoc->InsertOLE( *pTxtCrsr->GetPaM(), aName, &aItemSet );
        aObjName = aName;
    }

    if( !pFrmFmt )
        return xPropSet;

    if( IsInsertMode() )
    {
        if( !pOLENd )
            pOLENd = lcl_GetOLENode( pFrmFmt );
        if( pOLENd )
            pOLENd->SetOLESizeInvalid( sal_True );
    }

    SwXFrame *pXFrame = SwXFrames::GetObject( *pFrmFmt, FLYCNTTYPE_OLE );
    xPropSet = pXFrame;
    if( pDoc->GetDrawModel() )
        SwXFrame::GetOrCreateSdrObject( 
                static_cast<SwFlyFrmFmt*>( pXFrame->GetFrmFmt() ) ); // req for z-order
    if( rTblName.getLength() )
    {
        const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
        const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
        SwOLENode *pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetOLENode();
        ASSERT( pOLENd, "Where is the OLE node" );

        OUStringBuffer aBuffer( rTblName.getLength() );
        sal_Bool bQuoted = sal_False;
        sal_Bool bEscape = sal_False;
        sal_Bool bError = sal_False;
        for( sal_Int32 i=0; i < rTblName.getLength(); i++ )
        {
            sal_Bool bEndOfNameFound = sal_False;
            sal_Unicode c = rTblName[i];
            switch( c )
            {
            case '\'':
                if( bEscape )
                {
                    aBuffer.append( c );
                    bEscape = sal_False;
                }
                else if( bQuoted )
                {
                    bEndOfNameFound = sal_True;
                }
                else if( 0 == i )
                {
                    bQuoted = sal_True;
                }
                else
                {
                    bError = sal_True;
                }
                break;
            case '\\':
                if( bEscape )
                {
                    aBuffer.append( c );
                    bEscape = sal_False;
                }
                else
                {
                    bEscape = sal_True;
                }
                break;
            case ' ':
            case '.':
                if( !bQuoted )
                {
                    bEndOfNameFound = sal_True;
                }
                else
                {
                    aBuffer.append( c );
                    bEscape = sal_False;
                }
                break;
            default:
                {
                    aBuffer.append( c );
                    bEscape = sal_False;
                }
                break;
            }
            if( bError || bEndOfNameFound )
                break;
        }
        if( !bError )
        {
            OUString sTblName( aBuffer.makeStringAndClear() );
            pOLENd->SetChartTblName( GetRenameMap().Get( XML_TEXT_RENAME_TYPE_TABLE, sTblName ) );
        }
    }

    sal_Int32 nDrawAspect = 0;
    const XMLPropStyleContext *pStyle = 0;
    if( rStyleName.getLength() )
    {
        pStyle = FindAutoFrameStyle( rStyleName );
        if( pStyle )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                pStyle->GetStyles()
                      ->GetImportPropertyMapper(pStyle->GetFamily());
            ASSERT( xImpPrMap.is(), "Where is the import prop mapper?" );
            if( xImpPrMap.is() )
            {
                UniReference<XMLPropertySetMapper> rPropMapper = 
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
                        }
                        break;
                    case CTF_OLE_VIS_AREA_HEIGHT:
                        {
                            sal_Int32 nVal = 0;
                            rProp.maValue >>= nVal;
                            aVisArea.setHeight( nVal );
                        }
                        break;
                    case CTF_OLE_DRAW_ASPECT:
                        {
                            rProp.maValue >>= nDrawAspect;
                        }
                        break;
                    }
                }
            }
        }
    }

    SvInfoObject *pInfo = pDoc->GetPersist()->Find( aObjName );
    if( pInfo )
    {
        SvEmbeddedInfoObject * pEmbed = PTR_CAST(SvEmbeddedInfoObject, pInfo );
        pEmbed->SetInfoVisArea( aVisArea );
        if( nDrawAspect )
            pEmbed->SetInfoViewAspect( (UINT32)nDrawAspect );
    }
    return xPropSet;
}

Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertApplet(
        const OUString &rName,
        const OUString &rCode,
        sal_Bool bMayScript,
        const OUString& rHRef,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference < XPropertySet > xPropSet;
    Reference<XUnoTunnel> xCrsrTunnel( GetCursor(), UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = 
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                         RES_FRMATR_END );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    SwApplet_Impl aAppletImpl ( aItemSet );
    aAppletImpl.CreateApplet ( rCode, rName, bMayScript, rHRef );

    SwFrmFmt *pFrmFmt = pDoc->Insert( *pTxtCrsr->GetPaM(), 
                                       aAppletImpl.GetApplet(),
                                       &aAppletImpl.GetItemSet());
    SwXFrame *pXFrame = SwXFrames::GetObject( *pFrmFmt, FLYCNTTYPE_OLE );
    xPropSet = pXFrame;
    if( pDoc->GetDrawModel() )
        SwXFrame::GetOrCreateSdrObject( 
                static_cast<SwFlyFrmFmt*>( pXFrame->GetFrmFmt() ) ); // req for z-order

    return xPropSet;
}
Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertPlugin(
        const OUString &rMimeType,
        const OUString& rHRef,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    Reference < XPropertySet > xPropSet;
    Reference<XUnoTunnel> xCrsrTunnel( GetCursor(), UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = 
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                         RES_FRMATR_END );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    // We'll need a (valid) URL, or we need a MIME type. If we don't have
    // either, do not insert plugin and return early. Copy URL into URL oject
    // on the way.
       INetURLObject aURLObj;
    bool bValidURL = rHRef.getLength() != 0 && 
                     aURLObj.SetURL( ::binfilter::StaticBaseUrl::RelToAbs(rHRef) );
    bool bValidMimeType = rMimeType.getLength() != 0;
    if( !bValidURL && !bValidMimeType )
        return xPropSet;

    SvStorageRef pStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE);
    SvFactory *pPlugInFactory = (SvFactory*)SvPlugInObject::ClassFactory(); // SvFactory::GetDefaultPlugInFactory()
    SvPlugInObjectRef xPlugin = &pPlugInFactory->CreateAndInit( *pPlugInFactory, pStor );

    xPlugin->EnableSetModified( FALSE );
    xPlugin->SetPlugInMode( (USHORT)PLUGIN_EMBEDED );
    if( bValidURL )
        xPlugin->SetURL( aURLObj );
    if( bValidMimeType )
        xPlugin->SetMimeType( rMimeType );

    SwFrmFmt *pFrmFmt = pDoc->Insert( *pTxtCrsr->GetPaM(), 
                                       xPlugin,
                                       &aItemSet);
    SwXFrame *pXFrame = SwXFrames::GetObject( *pFrmFmt, FLYCNTTYPE_OLE );
    xPropSet = pXFrame;
    if( pDoc->GetDrawModel() )
        SwXFrame::GetOrCreateSdrObject( 
                static_cast<SwFlyFrmFmt*>( pXFrame->GetFrmFmt() ) ); // req for z-order

    return xPropSet;
}
Reference< XPropertySet > SwXMLTextImportHelper::createAndInsertFloatingFrame(
        const OUString& rName,
        const OUString& rHRef,
        const OUString& rStyleName,
        sal_Int32 nWidth, sal_Int32 nHeight )
{
    // this method will modify the document directly -> lock SolarMutex
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference < XPropertySet > xPropSet;
    Reference<XUnoTunnel> xCrsrTunnel( GetCursor(), UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = 
                (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                            OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();

    SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                         RES_FRMATR_END );
    lcl_putHeightAndWidth( aItemSet, nHeight, nWidth);

    SfxFrameDescriptor *pFrameDesc = new SfxFrameDescriptor( 0 );

    pFrameDesc->SetURL( ::binfilter::StaticBaseUrl::RelToAbs( rHRef ) );
    pFrameDesc->SetName( rName );

    ScrollingMode eScrollMode = ScrollingAuto;
    sal_Bool bHasBorder = sal_False;
    sal_Bool bIsBorderSet = sal_False;
    Size aMargin( SIZE_NOT_SET, SIZE_NOT_SET );
    const XMLPropStyleContext *pStyle = 0;
    if( rStyleName.getLength() )
    {
        pStyle = FindAutoFrameStyle( rStyleName );
        if( pStyle )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                pStyle->GetStyles()
                      ->GetImportPropertyMapper(pStyle->GetFamily());
            ASSERT( xImpPrMap.is(), "Where is the import prop mapper?" );
            if( xImpPrMap.is() )
            {
                UniReference<XMLPropertySetMapper> rPropMapper = 
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
                            sal_Bool bYes = *(sal_Bool *)rProp.maValue.getValue();
                            eScrollMode = bYes ? ScrollingYes : ScrollingNo;
                        }
                        break;
                    case CTF_FRAME_DISPLAY_BORDER:
                        {
                            bHasBorder = *(sal_Bool *)rProp.maValue.getValue();
                            bIsBorderSet = sal_True;
                        }
                        break;
                    case CTF_FRAME_MARGIN_HORI:
                        {
                            sal_Int32 nVal = SIZE_NOT_SET;
                            rProp.maValue >>= nVal;
                            aMargin.Width() = nVal;
                        }
                        break;
                    case CTF_FRAME_MARGIN_VERT:
                        {
                            sal_Int32 nVal = SIZE_NOT_SET;
                            rProp.maValue >>= nVal;
                            aMargin.Height() = nVal;
                        }
                        break;
                    }
                }
            }
        }
    }
    pFrameDesc->SetScrollingMode( eScrollMode );
    if( bIsBorderSet )
        pFrameDesc->SetFrameBorder( bHasBorder );
    else
        pFrameDesc->ResetBorder();
    pFrameDesc->SetMargin( aMargin );

    SvStorageRef pStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
    SfxFrameObjectRef pFrame = new SfxFrameObject();
    pFrame->DoInitNew( pStor );

    pFrame->EnableSetModified( FALSE );
    pFrame->SetFrameDescriptor( pFrameDesc );
    pFrame->EnableSetModified( TRUE );

    SwFrmFmt *pFrmFmt = pDoc->Insert( *pTxtCrsr->GetPaM(), 
                                       pFrame,
                                       &aItemSet);
    SwXFrame *pXFrame = SwXFrames::GetObject( *pFrmFmt, FLYCNTTYPE_OLE );
    xPropSet = pXFrame;
    if( pDoc->GetDrawModel() )
        SwXFrame::GetOrCreateSdrObject( 
                static_cast<SwFlyFrmFmt*>( pXFrame->GetFrmFmt() ) ); // req for z-order

    return xPropSet;
}

void SwXMLTextImportHelper::endAppletOrPlugin(
        Reference < XPropertySet > &rPropSet,
        ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess > &rParamMap)
{
    // this method will modify the document directly -> lock SolarMutex
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference<XUnoTunnel> xCrsrTunnel( rPropSet, UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for embedded" );
    SwXFrame *pFrame = 
                (SwXFrame *)xCrsrTunnel->getSomething(
                                    SwXFrame::getUnoTunnelId() );
    ASSERT( pFrame, "SwXFrame missing" );
    SwFrmFmt *pFrmFmt = pFrame->GetFrmFmt();
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
    SwOLENode *pOLENd = pNdIdx->GetNodes()[pNdIdx->GetIndex() + 1]->GetNoTxtNode()->GetOLENode();
    SwOLEObj& rOLEObj = pOLENd->GetOLEObj();
    
       SvPlugInObjectRef xPlugin ( rOLEObj.GetOleRef() );
    SvAppletObjectRef xApplet ( rOLEObj.GetOleRef() );
    SvCommandList aCommandList;

    ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess > ::iterator aIter = rParamMap.begin();
    ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess > ::iterator aEnd = rParamMap.end();

    while (aIter != aEnd )
    {
        aCommandList.Append( (*aIter).first, (*aIter).second);
        aIter++;
    }

    if (xApplet.Is())
    {
        xApplet->SetCommandList( aCommandList );
        xApplet->EnableSetModified ( TRUE );
    }
    else if (xPlugin.Is())
    {
        xPlugin->SetCommandList( aCommandList );
        xPlugin->EnableSetModified ( TRUE );
    }
}

XMLTextImportHelper* SwXMLImport::CreateTextImport()
{
    return new SwXMLTextImportHelper( GetModel(), *this, getImportInfo(),
                                      IsInsertMode(),
                                      IsStylesOnlyMode(), bShowProgress, 
                                      IsBlockMode(), IsOrganizerMode(), 
                                      bPreserveRedlineMode );
}


// redlining helper methods
// (override to provide the real implementation)

void SwXMLTextImportHelper::RedlineAdd( 
    const OUString& rType,
    const OUString& rId,
    const OUString& rAuthor,
    const OUString& rComment,
    const util::DateTime& rDateTime,
    sal_Bool bMergeLastPara)
{
    // create redline helper on demand
    DBG_ASSERT(NULL != pRedlineHelper, "helper should have been created in constructor");
    if (NULL != pRedlineHelper)
        pRedlineHelper->Add(rType, rId, rAuthor, rComment, rDateTime, 
                            bMergeLastPara);
}

Reference<XTextCursor> SwXMLTextImportHelper::RedlineCreateText(
    Reference<XTextCursor> & rOldCursor,
    const OUString& rId)
{
    Reference<XTextCursor> xRet;

    if (NULL != pRedlineHelper)
    {
        xRet = pRedlineHelper->CreateRedlineTextSection(rOldCursor, rId);
    }

    return xRet;
}

void SwXMLTextImportHelper::RedlineSetCursor(
    const OUString& rId,
    sal_Bool bStart,
    sal_Bool bIsOutsideOfParagraph)
{
    if (NULL != pRedlineHelper) {
        Reference<XTextRange> xTextRange( GetCursor()->getStart() ); 
        pRedlineHelper->SetCursor(rId, bStart, xTextRange,
                                  bIsOutsideOfParagraph);
    }
    // else: ignore redline (wasn't added before, else we'd have a helper)
}

void SwXMLTextImportHelper::RedlineAdjustStartNodeCursor(
    sal_Bool bStart)
{
    OUString rId = GetOpenRedlineId();
    if ((NULL != pRedlineHelper) && (rId.getLength() > 0))
    {
        Reference<XTextRange> xTextRange( GetCursor()->getStart() ); 
        pRedlineHelper->AdjustStartNodeCursor(rId, bStart, xTextRange );
        ResetOpenRedlineId();
    }
    // else: ignore redline (wasn't added before, or no open redline ID
}

void SwXMLTextImportHelper::SetShowChanges( sal_Bool bShowChanges )
{
    if ( NULL != pRedlineHelper )
        pRedlineHelper->SetShowChanges( bShowChanges );
}

void SwXMLTextImportHelper::SetRecordChanges( sal_Bool bRecordChanges )
{
    if ( NULL != pRedlineHelper )
        pRedlineHelper->SetRecordChanges( bRecordChanges );
}

void SwXMLTextImportHelper::SetChangesProtectionKey( 
    const Sequence<sal_Int8> & rKey )
{
    if ( NULL != pRedlineHelper )
        pRedlineHelper->SetProtectionKey( rKey );
}



}

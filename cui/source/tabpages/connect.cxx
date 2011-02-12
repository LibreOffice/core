/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// include ---------------------------------------------------------------
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <tools/shl.hxx>
#include "svx/ofaitem.hxx"
#include <sfx2/request.hxx>
#include <svx/dialogs.hrc>

#define _SVX_CONNECT_CXX

#include <svx/svdoedge.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdview.hxx>

#include "svx/connctrl.hxx"
#include "connect.hxx"
#include "connect.hrc"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"

#include <cuires.hrc>

static USHORT pRanges[] =
{
    SDRATTR_EDGE_FIRST,
    SDRATTR_EDGE_LAST,
    0
};

/*************************************************************************
|*
|* Dialog zum Aendern von Konnektoren (Connectors)
|*
\************************************************************************/

SvxConnectionDialog::SvxConnectionDialog( Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView ) :
        SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_CONNECTION )
{
    SvxConnectionPage* _pPage = new SvxConnectionPage( this, rInAttrs );

    _pPage->SetView( pSdrView );
    _pPage->Construct();

    SetTabPage( _pPage );
    SetText( _pPage->GetText() );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxConnectionDialog::~SvxConnectionDialog()
{
}

/*************************************************************************
|*
|* Seite zum Aendern von Konnektoren (Connectors)
|*
\************************************************************************/

SvxConnectionPage::SvxConnectionPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SfxTabPage      ( pWindow, CUI_RES( RID_SVXPAGE_CONNECTION ),
                                  rInAttrs ),

        aFtType                 ( this, CUI_RES( FT_TYPE ) ),
        aLbType                 ( this, CUI_RES( LB_TYPE ) ),

        aFlDelta                ( this, CUI_RES( FL_DELTA ) ),
        aFtLine1                ( this, CUI_RES( FT_LINE_1 ) ),
        aMtrFldLine1            ( this, CUI_RES( MTR_FLD_LINE_1 ) ),
        aFtLine2                ( this, CUI_RES( FT_LINE_2 ) ),
        aMtrFldLine2            ( this, CUI_RES( MTR_FLD_LINE_2 ) ),
        aFtLine3                ( this, CUI_RES( FT_LINE_3 ) ),
        aMtrFldLine3            ( this, CUI_RES( MTR_FLD_LINE_3 ) ),

        aFlDistance             ( this, CUI_RES( FL_DISTANCE ) ),
        aFtHorz1                ( this, CUI_RES( FT_HORZ_1 ) ),
        aMtrFldHorz1            ( this, CUI_RES( MTR_FLD_HORZ_1 ) ),
        aFtVert1                ( this, CUI_RES( FT_VERT_1 ) ),
        aMtrFldVert1            ( this, CUI_RES( MTR_FLD_VERT_1 ) ),
        aFtHorz2                ( this, CUI_RES( FT_HORZ_2 ) ),
        aMtrFldHorz2            ( this, CUI_RES( MTR_FLD_HORZ_2 ) ),
        aFtVert2                ( this, CUI_RES( FT_VERT_2 ) ),
        aMtrFldVert2            ( this, CUI_RES( MTR_FLD_VERT_2 ) ),

        aCtlPreview             ( this, CUI_RES( CTL_PREVIEW ), rInAttrs ),
        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() )
{
    FreeResource();

    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_EDGENODE1HORZDIST );

    FillTypeLB();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( aMtrFldHorz1, eFUnit );
    SetFieldUnit( aMtrFldHorz2, eFUnit );
    SetFieldUnit( aMtrFldVert1, eFUnit );
    SetFieldUnit( aMtrFldVert2, eFUnit );
    SetFieldUnit( aMtrFldLine1, eFUnit );
    SetFieldUnit( aMtrFldLine2, eFUnit );
    SetFieldUnit( aMtrFldLine3, eFUnit );
    if( eFUnit == FUNIT_MM )
    {
        aMtrFldHorz1.SetSpinSize( 50 );
        aMtrFldHorz2.SetSpinSize( 50 );
        aMtrFldVert1.SetSpinSize( 50 );
        aMtrFldVert2.SetSpinSize( 50 );
        aMtrFldLine1.SetSpinSize( 50 );
        aMtrFldLine2.SetSpinSize( 50 );
        aMtrFldLine3.SetSpinSize( 50 );
    }

    // disable 3D border
    aCtlPreview.SetBorderStyle(WINDOW_BORDER_MONO);

    Link aLink( LINK( this, SvxConnectionPage, ChangeAttrHdl_Impl ) );
    aMtrFldHorz1.SetModifyHdl( aLink );
    aMtrFldVert1.SetModifyHdl( aLink );
    aMtrFldHorz2.SetModifyHdl( aLink );
    aMtrFldVert2.SetModifyHdl( aLink );
    aMtrFldLine1.SetModifyHdl( aLink );
    aMtrFldLine2.SetModifyHdl( aLink );
    aMtrFldLine3.SetModifyHdl( aLink );
    aLbType.SetSelectHdl( aLink );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxConnectionPage::~SvxConnectionPage()
{
}

/*************************************************************************
|*
|* Liest uebergebenen Item-Set
|*
\************************************************************************/

void SvxConnectionPage::Reset( const SfxItemSet& rAttrs )
{
    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_EDGENODE1HORZDIST );
    const SfxItemPool* pPool = rAttrs.GetPool();

    // SdrEdgeNode1HorzDistItem
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE1HORZDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode1HorzDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHorz1, nValue, eUnit );
    }
    else
        aMtrFldHorz1.SetEmptyFieldValue();

    aMtrFldHorz1.SaveValue();

    // SdrEdgeNode2HorzDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE2HORZDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE2HORZDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode2HorzDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldHorz2, nValue, eUnit );
    }
    else
        aMtrFldHorz2.SetEmptyFieldValue();

    aMtrFldHorz2.SaveValue();

    // SdrEdgeNode1VertDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE1VERTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE1VERTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode1VertDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldVert1, nValue, eUnit );
    }
    else
        aMtrFldVert1.SetEmptyFieldValue();

    aMtrFldVert1.SaveValue();

    // SdrEdgeNode2VertDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE2VERTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE2VERTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode2VertDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldVert2, nValue, eUnit );
    }
    else
        aMtrFldVert2.SetEmptyFieldValue();

    aMtrFldVert2.SaveValue();

    // SdrEdgeLine1DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE1DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE1DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine1DeltaItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldLine1, nValue, eUnit );
    }
    else
        aMtrFldLine1.SetEmptyFieldValue();

    aMtrFldLine1.SaveValue();

    // SdrEdgeLine2DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE2DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE2DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine2DeltaItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldLine2, nValue, eUnit );
    }
    else
        aMtrFldLine2.SetEmptyFieldValue();

    aMtrFldLine2.SaveValue();

    // SdrEdgeLine3DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE3DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE3DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine3DeltaItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldLine3, nValue, eUnit );
    }
    else
        aMtrFldLine3.SetEmptyFieldValue();

    aMtrFldLine3.SaveValue();

    // SdrEdgeLineDeltaAnzItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINEDELTAANZ );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINEDELTAANZ );
    if( pItem )
    {
        UINT16 nValue = ( ( const SdrEdgeLineDeltaAnzItem* )pItem )->GetValue();
        if( nValue <= 2 )
        {
            aFtLine3.Enable( FALSE );
            aMtrFldLine3.Enable( FALSE );
            aMtrFldLine3.SetEmptyFieldValue();
        }
        if( nValue <= 1 )
        {
            aFtLine2.Enable( FALSE );
            aMtrFldLine2.Enable( FALSE );
            aMtrFldLine2.SetEmptyFieldValue();
        }
        if( nValue == 0 )
        {
            aFtLine1.Enable( FALSE );
            aMtrFldLine1.Enable( FALSE );
            aMtrFldLine1.SetEmptyFieldValue();
        }
    }

    // SdrEdgeKindItem
    pItem = GetItem( rAttrs, SDRATTR_EDGEKIND );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGEKIND );
    if( pItem )
    {
        SdrEdgeKind nValue = ( ( const SdrEdgeKindItem* )pItem )->GetValue();
        aLbType.SelectEntryPos( sal::static_int_cast< USHORT >(nValue) );
    }
    else
        aLbType.SetNoSelection();
    aLbType.SaveValue();
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\************************************************************************/

BOOL SvxConnectionPage::FillItemSet( SfxItemSet& rAttrs)
{
    BOOL     bModified = FALSE;
    INT32    nValue;

    if( aMtrFldHorz1.GetText() != aMtrFldHorz1.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHorz1, eUnit );
        rAttrs.Put( SdrEdgeNode1HorzDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldHorz2.GetText() != aMtrFldHorz2.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldHorz2, eUnit );
        rAttrs.Put( SdrEdgeNode2HorzDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldVert1.GetText() != aMtrFldVert1.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldVert1, eUnit );
        rAttrs.Put( SdrEdgeNode1VertDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldVert2.GetText() != aMtrFldVert2.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldVert2, eUnit );
        rAttrs.Put( SdrEdgeNode2VertDistItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldLine1.GetText() != aMtrFldLine1.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLine1, eUnit );
        rAttrs.Put( SdrEdgeLine1DeltaItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldLine2.GetText() != aMtrFldLine2.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLine2, eUnit );
        rAttrs.Put( SdrEdgeLine2DeltaItem( nValue ) );
        bModified = TRUE;
    }

    if( aMtrFldLine3.GetText() != aMtrFldLine3.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLine3, eUnit );
        rAttrs.Put( SdrEdgeLine3DeltaItem( nValue ) );
        bModified = TRUE;
    }


    USHORT nPos = aLbType.GetSelectEntryPos();
    if( nPos != aLbType.GetSavedValue() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rAttrs.Put( SdrEdgeKindItem( (SdrEdgeKind) nPos ) );
            bModified = TRUE;
        }
    }

    return( bModified );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxConnectionPage::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    aCtlPreview.SetView( pView );
    aCtlPreview.Construct();
}

/*************************************************************************
|*
|* Erzeugt die Seite
|*
\************************************************************************/

SfxTabPage* SvxConnectionPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxConnectionPage( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT* SvxConnectionPage::GetRanges()
{
    return( pRanges );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxConnectionPage, ChangeAttrHdl_Impl, void *, p )
{
    if( p == &aMtrFldHorz1 )
    {
        INT32 nValue = GetCoreValue( aMtrFldHorz1, eUnit );
        aAttrSet.Put( SdrEdgeNode1HorzDistItem( nValue ) );
    }

    if( p == &aMtrFldHorz2 )
    {
        INT32 nValue = GetCoreValue( aMtrFldHorz2, eUnit );
        aAttrSet.Put( SdrEdgeNode2HorzDistItem( nValue ) );
    }

    if( p == &aMtrFldVert1 )
    {
        INT32 nValue = GetCoreValue( aMtrFldVert1, eUnit );
        aAttrSet.Put( SdrEdgeNode1VertDistItem( nValue ) );
    }

    if( p == &aMtrFldVert2 )
    {
        INT32 nValue = GetCoreValue( aMtrFldVert2, eUnit );
        aAttrSet.Put( SdrEdgeNode2VertDistItem( nValue ) );
    }

    if( p == &aMtrFldLine1 )
    {
        INT32 nValue = GetCoreValue( aMtrFldLine1, eUnit );
        aAttrSet.Put( SdrEdgeLine1DeltaItem( nValue ) );
    }

    if( p == &aMtrFldLine2 )
    {
        INT32 nValue = GetCoreValue( aMtrFldLine2, eUnit );
        aAttrSet.Put( SdrEdgeLine2DeltaItem( nValue ) );
    }

    if( p == &aMtrFldLine3 )
    {
        INT32 nValue = GetCoreValue( aMtrFldLine3, eUnit );
        aAttrSet.Put( SdrEdgeLine3DeltaItem( nValue ) );
    }


    if( p == &aLbType )
    {
        USHORT nPos = aLbType.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aAttrSet.Put( SdrEdgeKindItem( (SdrEdgeKind) nPos ) );
        }
    }

    aCtlPreview.SetAttributes( aAttrSet );

    if( p == &aLbType )
    {
        // Anzahl der Linienversaetze ermitteln
        USHORT nCount = aCtlPreview.GetLineDeltaAnz();

        aFtLine3.Enable( nCount > 2 );
        aMtrFldLine3.Enable( nCount > 2 );
        if( nCount > 2 )
            aMtrFldLine3.SetValue( aMtrFldLine3.GetValue() );
        else
            aMtrFldLine3.SetEmptyFieldValue();

        aFtLine2.Enable( nCount > 1 );
        aMtrFldLine2.Enable( nCount > 1 );
        if( nCount > 1 )
            aMtrFldLine2.SetValue( aMtrFldLine2.GetValue() );
        else
            aMtrFldLine2.SetEmptyFieldValue();

        aFtLine1.Enable( nCount > 0 );
        aMtrFldLine1.Enable( nCount > 0 );
        if( nCount > 0 )
            aMtrFldLine1.SetValue( aMtrFldLine1.GetValue() );
        else
            aMtrFldLine1.SetEmptyFieldValue();

    }

    return( 0L );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxConnectionPage::FillTypeLB()
{
    // ListBox mit Verbindernamen fuellen
    const SfxPoolItem* pItem = GetItem( rOutAttrs, SDRATTR_EDGEKIND );
    const SfxItemPool* pPool = rOutAttrs.GetPool();

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGEKIND );
    if( pItem )
    {
        const SdrEdgeKindItem* pEdgeKindItem = (const SdrEdgeKindItem*) pItem;
        USHORT nCount = pEdgeKindItem->GetValueCount();
        String aStr;

        for( USHORT i = 0; i < nCount; i++ )
        {
            aStr = pEdgeKindItem->GetValueTextByPos( i );
            aLbType.InsertEntry( aStr );
        }
    }
}
void SvxConnectionPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG(&aSet,pOfaPtrItem,OfaPtrItem,SID_OBJECT_LIST,sal_False);
    if (pOfaPtrItem)
        SetView( static_cast<SdrView *>(pOfaPtrItem->GetValue()) );

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

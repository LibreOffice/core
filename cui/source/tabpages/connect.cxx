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

#include <cuires.hrc>
#include <dialmgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/request.hxx>
#include <tools/shl.hxx>

#include <svx/connctrl.hxx>
#include <svx/dialogs.hrc>
#include <svx/dlgutil.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdview.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>

#include "connect.hxx"
#include "paragrph.hrc"

static sal_uInt16 pRanges[] =
{
    SDRATTR_EDGE_FIRST,
    SDRATTR_EDGE_LAST,
    0
};

/*************************************************************************
|*
|* dialog for changing connectors
|*
\************************************************************************/

SvxConnectionDialog::SvxConnectionDialog( Window* pParent, const SfxItemSet& rInAttrs,
                                const SdrView* pSdrView )
    : SfxSingleTabDialog(pParent, rInAttrs)
{
    SvxConnectionPage* _pPage = new SvxConnectionPage( get_content_area(), rInAttrs );

    _pPage->SetView( pSdrView );
    _pPage->Construct();

    setTabPage( _pPage );
    SetText(CUI_RESSTR( RID_SVXSTR_CONNECTOR ));
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
|* page for changing connectors
|*
\************************************************************************/

SvxConnectionPage::SvxConnectionPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SfxTabPage      ( pWindow
                                 ,"ConnectorTabPage"
                                 ,"cui/ui/connectortabpage.ui"
                                 ,rInAttrs ),
        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() )
{
    get(m_pLbType,"LB_TYPE");

    get(m_pFtLine1,"FT_LINE_1");
    get(m_pMtrFldLine1,"MTR_FLD_LINE_1");
    get(m_pFtLine2,"FT_LINE_2");
    get(m_pMtrFldLine2,"MTR_FLD_LINE_2");
    get(m_pFtLine3,"FT_LINE_3");
    get(m_pMtrFldLine3,"MTR_FLD_LINE_3");

    get(m_pMtrFldHorz1,"MTR_FLD_HORZ_1");
    get(m_pMtrFldVert1,"MTR_FLD_VERT_1");
    get(m_pMtrFldHorz2,"MTR_FLD_HORZ_2");
    get(m_pMtrFldVert2,"MTR_FLD_VERT_2");

    get(m_pCtlPreview,"CTL_PREVIEW");

    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_EDGENODE1HORZDIST );

    FillTypeLB();

    const FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_pMtrFldHorz1, eFUnit );
    SetFieldUnit( *m_pMtrFldHorz2, eFUnit );
    SetFieldUnit( *m_pMtrFldVert1, eFUnit );
    SetFieldUnit( *m_pMtrFldVert2, eFUnit );
    SetFieldUnit( *m_pMtrFldLine1, eFUnit );
    SetFieldUnit( *m_pMtrFldLine2, eFUnit );
    SetFieldUnit( *m_pMtrFldLine3, eFUnit );
    if( eFUnit == FUNIT_MM )
    {
        m_pMtrFldHorz1->SetSpinSize( 50 );
        m_pMtrFldHorz2->SetSpinSize( 50 );
        m_pMtrFldVert1->SetSpinSize( 50 );
        m_pMtrFldVert2->SetSpinSize( 50 );
        m_pMtrFldLine1->SetSpinSize( 50 );
        m_pMtrFldLine2->SetSpinSize( 50 );
        m_pMtrFldLine3->SetSpinSize( 50 );
    }

    // disable 3D border
    m_pCtlPreview->SetBorderStyle(WINDOW_BORDER_MONO);

    Link aLink( LINK( this, SvxConnectionPage, ChangeAttrHdl_Impl ) );
    m_pMtrFldHorz1->SetModifyHdl( aLink );
    m_pMtrFldVert1->SetModifyHdl( aLink );
    m_pMtrFldHorz2->SetModifyHdl( aLink );
    m_pMtrFldVert2->SetModifyHdl( aLink );
    m_pMtrFldLine1->SetModifyHdl( aLink );
    m_pMtrFldLine2->SetModifyHdl( aLink );
    m_pMtrFldLine3->SetModifyHdl( aLink );
    m_pLbType->SetSelectHdl( aLink );
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
|* reads passed Item-Set
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
        SetMetricValue( *m_pMtrFldHorz1, nValue, eUnit );
    }
    else
        m_pMtrFldHorz1->SetEmptyFieldValue();

    m_pMtrFldHorz1->SaveValue();

    // SdrEdgeNode2HorzDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE2HORZDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE2HORZDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode2HorzDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldHorz2, nValue, eUnit );
    }
    else
        m_pMtrFldHorz2->SetEmptyFieldValue();

    m_pMtrFldHorz2->SaveValue();

    // SdrEdgeNode1VertDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE1VERTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE1VERTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode1VertDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldVert1, nValue, eUnit );
    }
    else
        m_pMtrFldVert1->SetEmptyFieldValue();

    m_pMtrFldVert1->SaveValue();

    // SdrEdgeNode2VertDistItem
    pItem = GetItem( rAttrs, SDRATTR_EDGENODE2VERTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGENODE2VERTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeNode2VertDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldVert2, nValue, eUnit );
    }
    else
        m_pMtrFldVert2->SetEmptyFieldValue();

    m_pMtrFldVert2->SaveValue();

    // SdrEdgeLine1DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE1DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE1DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine1DeltaItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLine1, nValue, eUnit );
    }
    else
        m_pMtrFldLine1->SetEmptyFieldValue();

    m_pMtrFldLine1->SaveValue();

    // SdrEdgeLine2DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE2DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE2DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine2DeltaItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLine2, nValue, eUnit );
    }
    else
        m_pMtrFldLine2->SetEmptyFieldValue();

    m_pMtrFldLine2->SaveValue();

    // SdrEdgeLine3DeltaItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINE3DELTA );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINE3DELTA );
    if( pItem )
    {
        long nValue = ( ( const SdrEdgeLine3DeltaItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLine3, nValue, eUnit );
    }
    else
        m_pMtrFldLine3->SetEmptyFieldValue();

    m_pMtrFldLine3->SaveValue();

    // SdrEdgeLineDeltaAnzItem
    pItem = GetItem( rAttrs, SDRATTR_EDGELINEDELTAANZ );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGELINEDELTAANZ );
    if( pItem )
    {
        sal_uInt16 nValue = ( ( const SdrEdgeLineDeltaAnzItem* )pItem )->GetValue();
        if( nValue <= 2 )
        {
            m_pFtLine3->Enable( sal_False );
            m_pMtrFldLine3->Enable( sal_False );
            m_pMtrFldLine3->SetEmptyFieldValue();
        }
        if( nValue <= 1 )
        {
            m_pFtLine2->Enable( sal_False );
            m_pMtrFldLine2->Enable( sal_False );
            m_pMtrFldLine2->SetEmptyFieldValue();
        }
        if( nValue == 0 )
        {
            m_pFtLine1->Enable( sal_False );
            m_pMtrFldLine1->Enable( sal_False );
            m_pMtrFldLine1->SetEmptyFieldValue();
        }
    }

    // SdrEdgeKindItem
    pItem = GetItem( rAttrs, SDRATTR_EDGEKIND );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGEKIND );
    if( pItem )
    {
        SdrEdgeKind nValue = ( ( const SdrEdgeKindItem* )pItem )->GetValue();
        m_pLbType->SelectEntryPos( sal::static_int_cast< sal_uInt16 >(nValue) );
    }
    else
        m_pLbType->SetNoSelection();
    m_pLbType->SaveValue();
}

/*************************************************************************
|*
|* fills the passed Item-Set width Dialogbox attributes
|*
\************************************************************************/

sal_Bool SvxConnectionPage::FillItemSet( SfxItemSet& rAttrs)
{
    sal_Bool     bModified = sal_False;
    sal_Int32    nValue;

    if( m_pMtrFldHorz1->GetText() != m_pMtrFldHorz1->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHorz1, eUnit );
        rAttrs.Put( SdrEdgeNode1HorzDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldHorz2->GetText() != m_pMtrFldHorz2->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldHorz2, eUnit );
        rAttrs.Put( SdrEdgeNode2HorzDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldVert1->GetText() != m_pMtrFldVert1->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldVert1, eUnit );
        rAttrs.Put( SdrEdgeNode1VertDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldVert2->GetText() != m_pMtrFldVert2->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldVert2, eUnit );
        rAttrs.Put( SdrEdgeNode2VertDistItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldLine1->GetText() != m_pMtrFldLine1->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldLine1, eUnit );
        rAttrs.Put( SdrEdgeLine1DeltaItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldLine2->GetText() != m_pMtrFldLine2->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldLine2, eUnit );
        rAttrs.Put( SdrEdgeLine2DeltaItem( nValue ) );
        bModified = sal_True;
    }

    if( m_pMtrFldLine3->GetText() != m_pMtrFldLine3->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldLine3, eUnit );
        rAttrs.Put( SdrEdgeLine3DeltaItem( nValue ) );
        bModified = sal_True;
    }


    sal_uInt16 nPos = m_pLbType->GetSelectEntryPos();
    if( nPos != m_pLbType->GetSavedValue() )
    {
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rAttrs.Put( SdrEdgeKindItem( (SdrEdgeKind) nPos ) );
            bModified = sal_True;
        }
    }

    return( bModified );
}

void SvxConnectionPage::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    m_pCtlPreview->SetView( pView );
    m_pCtlPreview->Construct();
}

/*************************************************************************
|*
|* creates the page
|*
\************************************************************************/

SfxTabPage* SvxConnectionPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxConnectionPage( pWindow, rAttrs ) );
}

sal_uInt16* SvxConnectionPage::GetRanges()
{
    return( pRanges );
}

IMPL_LINK( SvxConnectionPage, ChangeAttrHdl_Impl, void *, p )
{
    if( p == m_pMtrFldHorz1 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHorz1, eUnit );
        aAttrSet.Put( SdrEdgeNode1HorzDistItem( nValue ) );
    }

    if( p == m_pMtrFldHorz2 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldHorz2, eUnit );
        aAttrSet.Put( SdrEdgeNode2HorzDistItem( nValue ) );
    }

    if( p == m_pMtrFldVert1 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldVert1, eUnit );
        aAttrSet.Put( SdrEdgeNode1VertDistItem( nValue ) );
    }

    if( p == m_pMtrFldVert2 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldVert2, eUnit );
        aAttrSet.Put( SdrEdgeNode2VertDistItem( nValue ) );
    }

    if( p == m_pMtrFldLine1 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldLine1, eUnit );
        aAttrSet.Put( SdrEdgeLine1DeltaItem( nValue ) );
    }

    if( p == m_pMtrFldLine2 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldLine2, eUnit );
        aAttrSet.Put( SdrEdgeLine2DeltaItem( nValue ) );
    }

    if( p == m_pMtrFldLine3 )
    {
        sal_Int32 nValue = GetCoreValue( *m_pMtrFldLine3, eUnit );
        aAttrSet.Put( SdrEdgeLine3DeltaItem( nValue ) );
    }


    if( p == m_pLbType )
    {
        sal_uInt16 nPos = m_pLbType->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aAttrSet.Put( SdrEdgeKindItem( (SdrEdgeKind) nPos ) );
        }
    }

    m_pCtlPreview->SetAttributes( aAttrSet );

    if( p == m_pLbType )
    {
        // get the number of line displacements
        sal_uInt16 nCount = m_pCtlPreview->GetLineDeltaAnz();

        m_pFtLine3->Enable( nCount > 2 );
        m_pMtrFldLine3->Enable( nCount > 2 );
        if( nCount > 2 )
            m_pMtrFldLine3->SetValue( m_pMtrFldLine3->GetValue() );
        else
            m_pMtrFldLine3->SetEmptyFieldValue();

        m_pFtLine2->Enable( nCount > 1 );
        m_pMtrFldLine2->Enable( nCount > 1 );
        if( nCount > 1 )
            m_pMtrFldLine2->SetValue( m_pMtrFldLine2->GetValue() );
        else
            m_pMtrFldLine2->SetEmptyFieldValue();

        m_pFtLine1->Enable( nCount > 0 );
        m_pMtrFldLine1->Enable( nCount > 0 );
        if( nCount > 0 )
            m_pMtrFldLine1->SetValue( m_pMtrFldLine1->GetValue() );
        else
            m_pMtrFldLine1->SetEmptyFieldValue();

    }

    return( 0L );
}

void SvxConnectionPage::FillTypeLB()
{
    // fill ListBox with connector names
    const SfxPoolItem* pItem = GetItem( rOutAttrs, SDRATTR_EDGEKIND );
    const SfxItemPool* pPool = rOutAttrs.GetPool();

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGEKIND );
    if( pItem )
    {
        const SdrEdgeKindItem* pEdgeKindItem = (const SdrEdgeKindItem*) pItem;
        sal_uInt16 nCount = pEdgeKindItem->GetValueCount();
        String aStr;

        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            aStr = pEdgeKindItem->GetValueTextByPos( i );
            m_pLbType->InsertEntry( aStr );
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

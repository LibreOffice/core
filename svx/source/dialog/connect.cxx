/*************************************************************************
 *
 *  $RCSfile: connect.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#pragma hdrstop


#define _SVX_CONNECT_CXX

#include "svdoedge.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdview.hxx"

#include "connctrl.hxx"
#include "connect.hxx"
#include "connect.hrc"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#include "dialogs.hrc"

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
        SfxSingleTabDialog( pParent, rInAttrs, RID_SVXPAGE_CONNECTION, FALSE )
{
    SvxConnectionPage* pPage = new SvxConnectionPage( this, rInAttrs );

    pPage->SetView( pSdrView );
    pPage->Construct();

    SetTabPage( pPage );
    SetText( pPage->GetText() );
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
                SfxTabPage      ( pWindow, ResId( RID_SVXPAGE_CONNECTION, DIALOG_MGR() ),
                                  rInAttrs ),

        rOutAttrs               ( rInAttrs ),
        aAttrSet                ( *rInAttrs.GetPool() ),

        aFtType                 ( this, ResId( FT_TYPE ) ),
        aLbType                 ( this, ResId( LB_TYPE ) ),

        aFtHorz1                ( this, ResId( FT_HORZ_1 ) ),
        aMtrFldHorz1            ( this, ResId( MTR_FLD_HORZ_1 ) ),
        aFtHorz2                ( this, ResId( FT_HORZ_2 ) ),
        aMtrFldHorz2            ( this, ResId( MTR_FLD_HORZ_2 ) ),
        aFtVert1                ( this, ResId( FT_VERT_1 ) ),
        aMtrFldVert1            ( this, ResId( MTR_FLD_VERT_1 ) ),
        aFtVert2                ( this, ResId( FT_VERT_2 ) ),
        aMtrFldVert2            ( this, ResId( MTR_FLD_VERT_2 ) ),
        aGrpDistance            ( this, ResId( GRP_DISTANCE ) ),

        aFtLine1                ( this, ResId( FT_LINE_1 ) ),
        aMtrFldLine1            ( this, ResId( MTR_FLD_LINE_1 ) ),
        aFtLine2                ( this, ResId( FT_LINE_2 ) ),
        aMtrFldLine2            ( this, ResId( MTR_FLD_LINE_2 ) ),
        aFtLine3                ( this, ResId( FT_LINE_3 ) ),
        aMtrFldLine3            ( this, ResId( MTR_FLD_LINE_3 ) ),
        aGrpDelta               ( this, ResId( GRP_DELTA ) ),

        aCtlPreview             ( this, ResId( CTL_PREVIEW ), rInAttrs ),
        aGrpPreview             ( this, ResId( GRP_PREVIEW ) )
{
    FreeResource();

    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_EDGENODE1HORZDIST );

    FillTypeLB();

    FieldUnit eFUnit;
    GET_MODULE_FIELDUNIT( eFUnit );
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

void __EXPORT SvxConnectionPage::Reset( const SfxItemSet& rAttrs )
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
#ifndef VCL
        aMtrFldHorz1.SetEmptyValue();
#else
        aMtrFldHorz1.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldHorz2.SetEmptyValue();
#else
        aMtrFldHorz2.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldVert1.SetEmptyValue();
#else
        aMtrFldVert1.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldVert2.SetEmptyValue();
#else
        aMtrFldVert2.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldLine1.SetEmptyValue();
#else
        aMtrFldLine1.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldLine2.SetEmptyValue();
#else
        aMtrFldLine2.SetEmptyFieldValue();
#endif

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
#ifndef VCL
        aMtrFldLine3.SetEmptyValue();
#else
        aMtrFldLine3.SetEmptyFieldValue();
#endif

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
#ifndef VCL
            aMtrFldLine3.SetEmptyValue();
#else
            aMtrFldLine3.SetEmptyFieldValue();
#endif
        }
        if( nValue <= 1 )
        {
            aFtLine2.Enable( FALSE );
            aMtrFldLine2.Enable( FALSE );
#ifndef VCL
            aMtrFldLine2.SetEmptyValue();
#else
            aMtrFldLine2.SetEmptyFieldValue();
#endif
        }
        if( nValue == 0 )
        {
            aFtLine1.Enable( FALSE );
            aMtrFldLine1.Enable( FALSE );
#ifndef VCL
            aMtrFldLine1.SetEmptyValue();
#else
            aMtrFldLine1.SetEmptyFieldValue();
#endif
        }
    }

    // SdrEdgeKindItem
    pItem = GetItem( rAttrs, SDRATTR_EDGEKIND );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_EDGEKIND );
    if( pItem )
    {
        USHORT nValue = ( ( const SdrEdgeKindItem* )pItem )->GetValue();
        aLbType.SelectEntryPos( nValue );
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
#ifndef VCL
            aMtrFldLine3.SetEmptyValue();
#else
            aMtrFldLine3.SetEmptyFieldValue();
#endif

        aFtLine2.Enable( nCount > 1 );
        aMtrFldLine2.Enable( nCount > 1 );
        if( nCount > 1 )
            aMtrFldLine2.SetValue( aMtrFldLine2.GetValue() );
        else
#ifndef VCL
            aMtrFldLine2.SetEmptyValue();
#else
            aMtrFldLine2.SetEmptyFieldValue();
#endif

        aFtLine1.Enable( nCount > 0 );
        aMtrFldLine1.Enable( nCount > 0 );
        if( nCount > 0 )
            aMtrFldLine1.SetValue( aMtrFldLine1.GetValue() );
        else
#ifndef VCL
            aMtrFldLine1.SetEmptyValue();
#else
            aMtrFldLine1.SetEmptyFieldValue();
#endif

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



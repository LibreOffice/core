/*************************************************************************
 *
 *  $RCSfile: _bmpmask.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:06 $
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

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#pragma hdrstop

#define BMPMASK_PRIVATE

#include <dialmgr.hxx>
#include <bmpmask.hxx>
#include <dialogs.hrc>
#include <bmpmask.hrc>
#include <svxids.hrc>

//-------------------------------------------------------------------------

#define BMP_RESID(nId)  ResId(nId, DIALOG_MGR())
#define TRANSP_STRING   "Transparent"
#define TRANSP_COL      (Color( 252, 252, 252 ))
#define OWN_CALLMODE    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD

//-------------------------------------------------------------------------

#define TEST_COLS()                                                 \
{                                                                   \
    nR = aCol.GetRed(); nG = aCol.GetGreen(); nB = aCol.GetBlue();  \
    for( i = 0; i < nCount; i++ )                                   \
    {                                                               \
        if ( ( pMinR[i] <= nR ) && ( pMaxR[i] >= nR ) &&            \
             ( pMinG[i] <= nG ) && ( pMaxG[i] >= nG ) &&            \
             ( pMinB[i] <= nB ) && ( pMaxB[i] >= nB ) )             \
        {                                                           \
            aCol = pDstCols[i]; bReplace = TRUE; break;             \
        }                                                           \
    }                                                               \
}

// -------------------------------------------------------------------------

SFX_IMPL_DOCKINGWINDOW( SvxBmpMaskChildWindow, SID_BMPMASK )

// -------------------------------------------------------------------------

class ColorWindow : public Control
{
    Color       aColor;


public:

                ColorWindow( Window* pParent, WinBits nWinStyle ) :
                    Control( pParent, nWinStyle ),
                    aColor( COL_WHITE ) {};

                ColorWindow( Window* pParent, const ResId& rId ) :
                    Control( pParent, rId ),
                    aColor( COL_WHITE ) {};

    void        SetColor( const Color& rColor )
                {
                    aColor = rColor;
                    Invalidate();
                }

    virtual void Paint( const Rectangle& rRect );
};

//-------------------------------------------------------------------------

class MaskSet : public ValueSet
{
    SvxBmpMask*     pSvxBmpMask;


public:

                    MaskSet( SvxBmpMask* pParent, WinBits nWinStyle );
                    MaskSet( SvxBmpMask* pParent, const ResId& rId );

    virtual void    Select();
};

//-------------------------------------------------------------------------

MaskSet::MaskSet( SvxBmpMask* pParent, WinBits nWinStyle ) :
            ValueSet        ( pParent, nWinStyle ),
            pSvxBmpMask     ( pParent )
{
}

//-------------------------------------------------------------------------

MaskSet::MaskSet( SvxBmpMask* pParent, const ResId& rId ) :
            ValueSet        ( pParent, rId ),
            pSvxBmpMask     ( pParent )
{
}

//-------------------------------------------------------------------------

void MaskSet::Select()
{
    ValueSet::Select();

    switch ( GetSelectItemId() )
    {
        case ( 1 ) :
            pSvxBmpMask->aLbColor1.GrabFocus();
        break;

        case ( 2 ) :
            pSvxBmpMask->aLbColor2.GrabFocus();
        break;

        case ( 3 ) :
            pSvxBmpMask->aLbColor3.GrabFocus();
        break;

        case ( 4 ) :
            pSvxBmpMask->aLbColor4.GrabFocus();
        break;

        default :
        break;
    }
}

//-------------------------------------------------------------------------

class MaskData
{
    SvxBmpMask*     pMask;
    BOOL            bIsReady;
    BOOL            bExecState;
    SfxBindings&    rBindings;

public:
                MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind );

    BOOL        IsCbxReady() const { return bIsReady; }
    void        SetExecState( BOOL bState ) { bExecState = bState; }
    BOOL        IsExecReady() const { return bExecState; }

                DECL_LINK( PipetteHdl, ToolBox* pTbx );
                DECL_LINK( CbxHdl, CheckBox* pCbx );
                DECL_LINK( CbxTransHdl, CheckBox* pCbx );
                DECL_LINK( FocusLbHdl, ColorLB* pLb );
                DECL_LINK( ExecHdl, PushButton* pBtn );
};

//-------------------------------------------------------------------------

MaskData::MaskData( SvxBmpMask* pBmpMask, SfxBindings& rBind ) :

    pMask       ( pBmpMask ),
    bIsReady    ( FALSE ),
    bExecState  ( FALSE ),
    rBindings   ( rBind )

{
}

//-------------------------------------------------------------------------

IMPL_LINK( MaskData, PipetteHdl, ToolBox*, pTbx )
{
    SfxBoolItem aBItem( SID_BMPMASK_PIPETTE,
                        pTbx->IsItemChecked( TBI_PIPETTE ) );

    rBindings.GetDispatcher()->Execute( SID_BMPMASK_PIPETTE, OWN_CALLMODE, &aBItem, 0L );

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( MaskData, CbxHdl, CheckBox*, pCbx )
{
    bIsReady =  pMask->aCbx1.IsChecked() || pMask->aCbx2.IsChecked() ||
                pMask->aCbx3.IsChecked() || pMask->aCbx4.IsChecked();

    if ( bIsReady && IsExecReady() )
        pMask->aBtnExec.Enable();
    else
        pMask->aBtnExec.Disable();

    // Wenn eine Checkbox gecheckt wurde, wird die Pipette enabled
    if ( pCbx->IsChecked() )
    {
        if ( pCbx == &( pMask->aCbx1 ) )
            pMask->pQSet->SelectItem( 1 );
        else if ( pCbx == &( pMask->aCbx2 ) )
            pMask->pQSet->SelectItem( 2 );
        else if ( pCbx == &( pMask->aCbx3 ) )
            pMask->pQSet->SelectItem( 3 );
        else if ( pCbx == &( pMask->aCbx4 ) )
            pMask->pQSet->SelectItem( 4 );

        pMask->pQSet->Select();

        pMask->aTbxPipette.CheckItem( TBI_PIPETTE, TRUE );
        PipetteHdl( &( pMask->aTbxPipette ) );
    }

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( MaskData, CbxTransHdl, CheckBox*, pCbx )
{
    if ( bIsReady = pCbx->IsChecked() )
    {
        pMask->pQSet->Disable();
        pMask->pCtlPipette->Disable();
        pMask->aCbx1.Disable();
        pMask->aSp1.Disable();
        pMask->aCbx2.Disable();
        pMask->aSp2.Disable();
        pMask->aCbx3.Disable();
        pMask->aSp3.Disable();
        pMask->aCbx4.Disable();
        pMask->aSp4.Disable();
        pMask->aTbxPipette.Disable();

        pMask->aLbColor1.Disable();
        pMask->aLbColor2.Disable();
        pMask->aLbColor3.Disable();
        pMask->aLbColor4.Disable();
        pMask->aLbColorTrans.Enable();
    }
    else
    {
        pMask->pQSet->Enable();
        pMask->pCtlPipette->Enable();
        pMask->aCbx1.Enable();
        pMask->aSp1.Enable();
        pMask->aCbx2.Enable();
        pMask->aSp2.Enable();
        pMask->aCbx3.Enable();
        pMask->aSp3.Enable();
        pMask->aCbx4.Enable();
        pMask->aSp4.Enable();
        pMask->aTbxPipette.Enable();

        pMask->aLbColor1.Enable();
        pMask->aLbColor2.Enable();
        pMask->aLbColor3.Enable();
        pMask->aLbColor4.Enable();
        pMask->aLbColorTrans.Disable();

        bIsReady = pMask->aCbx1.IsChecked() || pMask->aCbx2.IsChecked() ||
                   pMask->aCbx3.IsChecked() || pMask->aCbx4.IsChecked();
    }

    if ( bIsReady && IsExecReady() )
        pMask->aBtnExec.Enable();
    else
        pMask->aBtnExec.Disable();

    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( MaskData, FocusLbHdl, ColorLB*, pLb )
{
    if ( pLb == &( pMask->aLbColor1 ) )
        pMask->pQSet->SelectItem( 1 );
    else if ( pLb == &( pMask->aLbColor2 ) )
        pMask->pQSet->SelectItem( 2 );
    else if ( pLb == &( pMask->aLbColor3 ) )
        pMask->pQSet->SelectItem( 3 );
    else if ( pLb == &( pMask->aLbColor4 ) )
        pMask->pQSet->SelectItem( 4 );

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( MaskData, ExecHdl, PushButton*, pBtn )
{
    SfxBoolItem aBItem( SID_BMPMASK_EXEC, TRUE );
    rBindings.GetDispatcher()->Execute( SID_BMPMASK_EXEC, OWN_CALLMODE, &aBItem, 0L );

    return 0L;
}

//-------------------------------------------------------------------------

void ColorWindow::Paint( const Rectangle &Rect )
{
    const Color& rOldLineColor = GetLineColor();
    const Color& rOldFillColor = GetFillColor();

    SetLineColor( aColor );
    SetFillColor( aColor );

    DrawRect( Rectangle( Point(), GetSizePixel() ) );

    SetLineColor( rOldLineColor );
    SetFillColor( rOldFillColor );
}

//-------------------------------------------------------------------------

SvxBmpMaskSelectItem::SvxBmpMaskSelectItem( USHORT nId, SvxBmpMask& rMask,
                                            SfxBindings& rBindings ) :
            SfxControllerItem   ( nId, rBindings ),
            rBmpMask            ( rMask)
{
}

//-------------------------------------------------------------------------

void SvxBmpMaskSelectItem::StateChanged( USHORT nSID, SfxItemState eState,
                                         const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_BMPMASK_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pItem );

        DBG_ASSERT( pStateItem || pItem == 0, "SfxBoolItem erwartet");

        rBmpMask.SetExecState( pStateItem->GetValue() );
    }
}

//-------------------------------------------------------------------------

SvxBmpMaskChildWindow::SvxBmpMaskChildWindow( Window* pParent, USHORT nId,
                                              SfxBindings* pBindings,
                                              SfxChildWinInfo* pInfo ) :
        SfxChildWindow( pParent, nId )
{
    pWindow = new SvxBmpMask( pBindings, this, pParent,
                              BMP_RESID( RID_SVXDLG_BMPMASK ) );
    SvxBmpMask* pDlg = (SvxBmpMask*) pWindow;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}

//-------------------------------------------------------------------------

SvxBmpMask::SvxBmpMask( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent,
                        const ResId& rResId ) :
        SfxDockingWindow    ( pBindinx, pCW, pParent, rResId ),
        aTbxPipette         ( this, ResId( TBX_PIPETTE ) ),
        pCtlPipette         ( new ColorWindow( this, ResId( WND_PIPETTE ) ) ),
        aBtnExec            ( this, ResId( BTN_EXEC ) ),
        pQSet               ( new MaskSet( this, ResId( QCOL_1 ) ) ),
        aCbx1               ( this, ResId( CBX_1 ) ),
        aSp1                ( this, ResId( SP_1 ) ),
        aCbx2               ( this, ResId( CBX_2 ) ),
        aSp2                ( this, ResId( SP_2 ) ),
        aCbx3               ( this, ResId( CBX_3 ) ),
        aSp3                ( this, ResId( SP_3 ) ),
        aCbx4               ( this, ResId( CBX_4 ) ),
        aSp4                ( this, ResId( SP_4 ) ),
        aCbxTrans           ( this, ResId( CBX_TRANS ) ),
        aGrpQ               ( this, ResId( GRP_Q ) ),
        pColTab             ( NULL ),
        pData               ( new MaskData( this, *pBindinx ) ),
        aPipetteColor       ( COL_WHITE ),
        aSelItem            ( SID_BMPMASK_EXEC, *this, *pBindinx ),
        aLbColor1           ( this, ResId ( LB_1 ) ),
        aLbColor2           ( this, ResId ( LB_2 ) ),
        aLbColor3           ( this, ResId ( LB_3 ) ),
        aLbColor4           ( this, ResId ( LB_4 ) ),
        aLbColorTrans       ( this, ResId ( LB_TRANS ) ),
        aFt1                ( this, ResId ( FT_1 ) ),
        aFt2                ( this, ResId ( FT_2 ) ),
        aFt3                ( this, ResId ( FT_3 ) )
{
    FreeResource();

    aTbxPipette.SetSizePixel( aTbxPipette.CalcWindowSizePixel() );
    aTbxPipette.SetSelectHdl( LINK( pData, MaskData, PipetteHdl ) );
    aBtnExec.SetClickHdl( LINK( pData, MaskData, ExecHdl ) );

    aCbx1.SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    aCbx2.SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    aCbx3.SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    aCbx4.SetClickHdl( LINK( pData, MaskData, CbxHdl ) );
    aCbxTrans.SetClickHdl( LINK( pData, MaskData, CbxTransHdl ) );

    aLbColor1.SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    aLbColor2.SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    aLbColor3.SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );
    aLbColor4.SetGetFocusHdl( LINK( pData, MaskData, FocusLbHdl ) );

    aSp1.SetValue( 10 );
    aSp2.SetValue( 10 );
    aSp3.SetValue( 10 );
    aSp4.SetValue( 10 );

    pQSet->SetStyle( pQSet->GetStyle() | WB_DOUBLEBORDER | WB_ITEMBORDER );
    pQSet->SetColCount( 1 );
    pQSet->SetLineCount( 4 );
    pQSet->SetExtraSpacing( 1 );
    pQSet->InsertItem( 1, aPipetteColor );
    pQSet->InsertItem( 2, aPipetteColor );
    pQSet->InsertItem( 3, aPipetteColor );
    pQSet->InsertItem( 4, aPipetteColor );
    pQSet->SelectItem( 1 );

    pQSet->Show();
}

//-------------------------------------------------------------------------

SvxBmpMask::~SvxBmpMask()
{
    delete pQSet;
    delete pCtlPipette;
    delete pData;
}

//-------------------------------------------------------------------------

BOOL SvxBmpMask::Close()
{
    SfxBoolItem aItem2( SID_BMPMASK_PIPETTE, FALSE );
    GetBindings().GetDispatcher()->Execute( SID_BMPMASK_PIPETTE, OWN_CALLMODE, &aItem2, 0L );

    return SfxDockingWindow::Close();
}

//-------------------------------------------------------------------------

BOOL SvxBmpMask::NeedsColorTable() const
{
    return ( aLbColor1.GetEntryCount() == 0 );
}

//-------------------------------------------------------------------------

void SvxBmpMask::SetColorTable( const XColorTable* pTable )
{
    if ( pTable && ( pTable != pColTab ) )
    {
        const String aTransp( BMP_RESID( RID_SVXDLG_BMPMASK_STR_TRANSP ) );

        pColTab = pTable;

        aLbColorTrans.Fill( pColTab );
        aLbColorTrans.SelectEntryPos( 0 );

        aLbColor1.Fill( pColTab );
        aLbColor1.InsertEntry( TRANSP_COL, aTransp, 0 );
        aLbColor1.SelectEntryPos( 0 );

        aLbColor2.Fill( pColTab );
        aLbColor2.InsertEntry( TRANSP_COL, aTransp, 0 );
        aLbColor2.SelectEntryPos( 0 );

        aLbColor3.Fill( pColTab );
        aLbColor3.InsertEntry( TRANSP_COL, aTransp, 0 );
        aLbColor3.SelectEntryPos( 0 );

        aLbColor4.Fill( pColTab );
        aLbColor4.InsertEntry( TRANSP_COL, aTransp, 0 );
        aLbColor4.SelectEntryPos( 0 );
    }
}

//-------------------------------------------------------------------------

void SvxBmpMask::SetColor( const Color& rColor )
{
    aPipetteColor = rColor;
    pCtlPipette->SetColor( aPipetteColor );
}

//-------------------------------------------------------------------------

void SvxBmpMask::PipetteClicked()
{
    USHORT nId = pQSet->GetSelectItemId();

    pQSet->SetItemColor( nId, aPipetteColor );

    switch( nId )
    {
        case ( 1 ) :
        {
            aCbx1.Check( TRUE );
            pData->CbxHdl( &aCbx1 );
        }
        break;

        case ( 2 ) :
        {
            aCbx2.Check( TRUE );
            pData->CbxHdl( &aCbx2 );
        }
        break;

        case ( 3 ) :
        {
            aCbx3.Check( TRUE );
            pData->CbxHdl( &aCbx3 );
        }
        break;

        case ( 4 ) :
        {
            aCbx4.Check( TRUE );
            pData->CbxHdl( &aCbx4 );
        }
        break;

        default:
        break;
    }

    aTbxPipette.CheckItem( TBI_PIPETTE, FALSE );
    pData->PipetteHdl( &aTbxPipette );
}

//-------------------------------------------------------------------------

void SvxBmpMask::SetExecState( BOOL bEnable )
{
    pData->SetExecState( bEnable );

    if ( pData->IsExecReady() && pData->IsCbxReady() )
        aBtnExec.Enable();
    else
        aBtnExec.Disable();
}

//-------------------------------------------------------------------------

USHORT SvxBmpMask::InitColorArrays( Color* pSrcCols, Color* pDstCols, ULONG* pTols )
{
    USHORT  nCount = 0;

    if ( aCbx1.IsChecked() )
    {
        pSrcCols[nCount] = pQSet->GetItemColor( 1 );
        pDstCols[nCount] = aLbColor1.GetSelectEntryColor();
        pTols[nCount++] = aSp1.GetValue();
    }

    if ( aCbx2.IsChecked() )
    {
        pSrcCols[nCount] = pQSet->GetItemColor( 2 );
        pDstCols[nCount] = aLbColor2.GetSelectEntryColor();
        pTols[nCount++] = aSp2.GetValue();
    }

    if ( aCbx3.IsChecked() )
    {
        pSrcCols[nCount] = pQSet->GetItemColor( 3 );
        pDstCols[nCount] = aLbColor3.GetSelectEntryColor();
        pTols[nCount++] = aSp3.GetValue();
    }

    if ( aCbx4.IsChecked() )
    {
        pSrcCols[nCount] = pQSet->GetItemColor( 4 );
        pDstCols[nCount] = aLbColor4.GetSelectEntryColor();
        pTols[nCount++] = aSp4.GetValue();
    }

    return nCount;
}

//-------------------------------------------------------------------------

Bitmap SvxBmpMask::ImpMask( const Bitmap& rBitmap )
{
    Bitmap          aBitmap( rBitmap );
    Color           pSrcCols[4];
    Color           pDstCols[4];
    ULONG           pTols[4];
    const USHORT    nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

    EnterWait();
    aBitmap.Replace( pSrcCols, pDstCols, nCount, pTols );
    LeaveWait();

    return aBitmap;
}

//-------------------------------------------------------------------------

BitmapEx SvxBmpMask::ImpMask( const BitmapEx& rBitmapEx )
{
    return BitmapEx( ImpMask( rBitmapEx.GetBitmap() ), rBitmapEx.GetMask() );
}

//-------------------------------------------------------------------------

BitmapEx SvxBmpMask::ImpMaskTransparent( const BitmapEx& rBitmapEx, const Color& rColor, const long nTol )
{
    EnterWait();

    BitmapEx    aBmpEx;
    Bitmap      aMask( rBitmapEx.GetBitmap().CreateMask( rColor, nTol ) );

    if( rBitmapEx.IsTransparent() )
        aMask.CombineSimple( rBitmapEx.GetMask(), BMP_COMBINE_OR );

    aBmpEx = BitmapEx( rBitmapEx.GetBitmap(), aMask );
    LeaveWait();

    return aBmpEx;
}

//-------------------------------------------------------------------------

Animation SvxBmpMask::ImpMask( const Animation& rAnimation )
{
    Animation   aAnimation( rAnimation );
    Color       pSrcCols[4];
    Color       pDstCols[4];
    ULONG       pTols[4];
    USHORT      nCount = InitColorArrays( pSrcCols, pDstCols, pTols );
    USHORT      nAnimationCount = aAnimation.Count();

    for( USHORT i = 0; i < nAnimationCount; i++ )
    {
        AnimationBitmap aAnimBmp( aAnimation.Get( i ) );
        aAnimBmp.aBmpEx = Mask( aAnimBmp.aBmpEx ).GetBitmapEx();
        aAnimation.Replace( aAnimBmp, i );
    }

    return aAnimation;
}

//-------------------------------------------------------------------------

GDIMetaFile SvxBmpMask::ImpMask( const GDIMetaFile& rMtf )
{
    GDIMetaFile aMtf;
    Color       pSrcCols[4];
    Color       pDstCols[4];
    ULONG       pTols[4];
    USHORT      nCount = InitColorArrays( pSrcCols, pDstCols, pTols );
    BOOL        pTrans[4];

    // Falls keine Farben ausgewaehlt, kopieren wir nur das Mtf
    if( !nCount )
        aMtf = rMtf;
    else
    {
        Color       aCol;
        long        nVal;
        long        nTol;
        long        nR;
        long        nG;
        long        nB;
        long*       pMinR = new long[nCount];
        long*       pMaxR = new long[nCount];
        long*       pMinG = new long[nCount];
        long*       pMaxG = new long[nCount];
        long*       pMinB = new long[nCount];
        long*       pMaxB = new long[nCount];
        USHORT      i;
        BOOL        bReplace;

        aMtf.SetPrefSize( rMtf.GetPrefSize() );
        aMtf.SetPrefMapMode( rMtf.GetPrefMapMode() );

        // Farbvergleichsarrays vorbereiten
        for( i = 0; i < nCount; i++ )
        {
            nTol = ( pTols[i] * 255L ) / 100L;

            nVal = ( (long) pSrcCols[i].GetRed() );
            pMinR[i] = Max( nVal - nTol, 0L );
            pMaxR[i] = Min( nVal + nTol, 255L );

            nVal = ( (long) pSrcCols[i].GetGreen() );
            pMinG[i] = Max( nVal - nTol, 0L );
            pMaxG[i] = Min( nVal + nTol, 255L );

            nVal = ( (long) pSrcCols[i].GetBlue() );
            pMinB[i] = Max( nVal - nTol, 0L );
            pMaxB[i] = Min( nVal + nTol, 255L );

            pTrans[ i ] = ( pDstCols[ i ] == TRANSP_COL );
        }

        // Actions untersuchen und Farben ggf. ersetzen
        for( ULONG nAct = 0UL, nActCount = rMtf.GetActionCount(); nAct < nActCount; nAct++ )
        {
            MetaAction* pAction = rMtf.GetAction( nAct );

            bReplace = FALSE;

            switch( pAction->GetType() )
            {
                case( META_PIXEL_ACTION ):
                {
                    MetaPixelAction* pAct = (MetaPixelAction*) pAction;

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaPixelAction( pAct->GetPoint(), aCol );
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_LINECOLOR_ACTION ):
                {
                    MetaLineColorAction* pAct = (MetaLineColorAction*) pAction;

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaLineColorAction( aCol, !pTrans[ i ] );
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_FILLCOLOR_ACTION ):
                {
                    MetaFillColorAction* pAct = (MetaFillColorAction*) pAction;

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaFillColorAction( aCol, !pTrans[ i ] );
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_TEXTCOLOR_ACTION ):
                {
                    MetaTextColorAction* pAct = (MetaTextColorAction*) pAction;

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaTextColorAction( aCol );
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_TEXTFILLCOLOR_ACTION ):
                {
                    MetaTextFillColorAction* pAct = (MetaTextFillColorAction*) pAction;

                    aCol = pAct->GetColor();
                    TEST_COLS();

                    if( bReplace )
                        pAct = new MetaTextFillColorAction( aCol, !pTrans[ i ] );
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    MetaFontAction* pAct = (MetaFontAction*) pAction;
                    Font            aFont( pAct->GetFont() );

                    aCol = aFont.GetColor();
                    TEST_COLS();

                    if( bReplace )
                    {
                        aFont.SetColor( aCol );
                        pAct = new MetaFontAction( aFont );
                    }
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_WALLPAPER_ACTION ):
                {
                    MetaWallpaperAction*    pAct = (MetaWallpaperAction*) pAction;
                    Wallpaper               aWall( pAct->GetWallpaper() );

                    aCol = aWall.GetColor();
                    TEST_COLS();

                    if( bReplace )
                    {
                        aWall.SetColor( aCol );
                        pAct = new MetaWallpaperAction( pAct->GetRect(), aWall );
                    }
                    else
                        pAct->Duplicate();

                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMP_ACTION ):
                {
                    MetaBmpAction*  pAct = (MetaBmpAction*) pAction;
                    const Bitmap    aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

                    pAct = new MetaBmpAction( pAct->GetPoint(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                    const Bitmap        aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

                    pAct = new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMPSCALEPART_ACTION ):
                {
                    MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                    const Bitmap            aBmp( Mask( pAct->GetBitmap() ).GetBitmap() );

                    pAct = new MetaBmpScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                       pAct->GetSrcPoint(), pAct->GetSrcSize(), aBmp );
                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMPEX_ACTION ):
                {
                    MetaBmpExAction*    pAct = (MetaBmpExAction*) pAction;
                    const BitmapEx      aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExAction( pAct->GetPoint(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    MetaBmpExScaleAction*   pAct = (MetaBmpExScaleAction*) pAction;
                    const BitmapEx          aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExScaleAction( pAct->GetPoint(), pAct->GetSize(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                {
                    MetaBmpExScalePartAction*   pAct = (MetaBmpExScalePartAction*) pAction;
                    const BitmapEx              aBmpEx( Mask( pAct->GetBitmapEx() ).GetBitmapEx() );

                    pAct = new MetaBmpExScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                         pAct->GetSrcPoint(), pAct->GetSrcSize(), aBmpEx );
                    aMtf.AddAction( pAct );
                }
                break;

                default:
                {
                    pAction->Duplicate();
                    aMtf.AddAction( pAction );
                }
                break;
            }
        }

        delete[] pMinR;
        delete[] pMaxR;
        delete[] pMinG;
        delete[] pMaxG;
        delete[] pMinB;
        delete[] pMaxB;
    }

    LeaveWait();

    return aMtf;
}

//-------------------------------------------------------------------------

BitmapEx SvxBmpMask::ImpReplaceTransparency( const BitmapEx& rBmpEx, const Color& rColor )
{
    if( rBmpEx.IsTransparent() )
    {
        Bitmap aBmp( rBmpEx.GetBitmap() );
        aBmp.Replace( rBmpEx.GetMask(), rColor );
        return aBmp;
    }
    else
        return rBmpEx;
}

//-------------------------------------------------------------------------

Animation SvxBmpMask::ImpReplaceTransparency( const Animation& rAnim, const Color& rColor )
{
    Animation   aAnimation( rAnim );
    USHORT      nAnimationCount = aAnimation.Count();

    for( USHORT i = 0; i < nAnimationCount; i++ )
    {
        AnimationBitmap aAnimBmp( aAnimation.Get( i ) );
        aAnimBmp.aBmpEx = ImpReplaceTransparency( aAnimBmp.aBmpEx, rColor );
        aAnimation.Replace( aAnimBmp, i );
    }

    return aAnimation;
}

//-------------------------------------------------------------------------

GDIMetaFile SvxBmpMask::ImpReplaceTransparency( const GDIMetaFile& rMtf, const Color& rColor )
{
    VirtualDevice   aVDev;
    GDIMetaFile     aMtf;
    const MapMode&  rPrefMap = rMtf.GetPrefMapMode();
    const Size&     rPrefSize = rMtf.GetPrefSize();
    const ULONG     nActionCount = rMtf.GetActionCount();

    aVDev.EnableOutput( FALSE );
    aMtf.Record( &aVDev );
    aMtf.SetPrefSize( rPrefSize );
    aMtf.SetPrefMapMode( rPrefMap );
    aVDev.SetLineColor( rColor );
    aVDev.SetFillColor( rColor );

    // Actions nacheinander abspielen; zuerst
    // den gesamten Bereich auf die Ersatzfarbe setzen
    aVDev.DrawRect( Rectangle( rPrefMap.GetOrigin(), rPrefSize ) );
    for ( ULONG i = 0; i < nActionCount; i++ )
    {
        MetaAction* pAct = rMtf.GetAction( i );

        pAct->Duplicate();
        aMtf.AddAction( pAct );
    }

    aMtf.Stop();
    aMtf.WindStart();

    return aMtf;
}

//-------------------------------------------------------------------------

Graphic SvxBmpMask::Mask( const Graphic& rGraphic )
{
    Graphic     aGraphic( rGraphic );
    const Color aReplColor( aLbColorTrans.GetSelectEntryColor() );

    switch( rGraphic.GetType() )
    {
        case( GRAPHIC_BITMAP ):
        {
            if( rGraphic.IsAnimated() )
            {
                // Transparenz ersetzen?
                if ( aCbxTrans.IsChecked() )
                    aGraphic = ImpReplaceTransparency( rGraphic.GetAnimation(), aReplColor );
                else
                    aGraphic = ImpMask( rGraphic.GetAnimation() );
            }
            else
            {
                // Transparenz ersetzen?
                if( aCbxTrans.IsChecked() )
                {
                    if( aGraphic.IsTransparent() )
                    {
                        BitmapEx    aBmpEx( ImpReplaceTransparency( aGraphic.GetBitmapEx(), aReplColor ) );
                        const Size  aSize( aBmpEx.GetSizePixel() );

                        if( aSize.Width() && aSize.Height() )
                            aGraphic = aBmpEx;
                    }
                }
                else
                {
                    Color   pSrcCols[4];
                    Color   pDstCols[4];
                    ULONG   pTols[4];
                    USHORT  nCount = InitColorArrays( pSrcCols, pDstCols, pTols );

                    if( nCount )
                    {
                        // erstmal alle Transparent-Farben setzen
                        for( USHORT i = 0; i < nCount; i++ )
                        {
                            // Haben wir eine Transparenzfarbe?
                            if( pDstCols[i] == TRANSP_COL )
                            {
                                BitmapEx    aBmpEx( ImpMaskTransparent( aGraphic.GetBitmapEx(),
                                                                        pSrcCols[ i ], pTols[ i ] ) );
                                const Size  aSize( aBmpEx.GetSizePixel() );

                                if( aSize.Width() && aSize.Height() )
                                    aGraphic = aBmpEx;
                            }
                        }

                        // jetzt noch einmal mit den normalen Farben ersetzen
                        Bitmap  aBitmap( ImpMask( aGraphic.GetBitmap() ) );
                        Size    aSize( aBitmap.GetSizePixel() );

                        if ( aSize.Width() && aSize.Height() )
                        {
                            if ( aGraphic.IsTransparent() )
                                aGraphic = Graphic( BitmapEx( aBitmap, aGraphic.GetBitmapEx().GetMask() ) );
                            else
                                aGraphic = aBitmap;
                        }
                    }
                }
            }
        }
        break;

        case( GRAPHIC_GDIMETAFILE ):
        {
            GDIMetaFile aMtf( aGraphic.GetGDIMetaFile() );

            // Transparenz ersetzen?
            if( aCbxTrans.IsChecked() )
                aMtf = ImpReplaceTransparency( aMtf, aReplColor );
            else
                aMtf = ImpMask( aMtf );

            Size aSize( aMtf.GetPrefSize() );
            if ( aSize.Width() && aSize.Height() )
                aGraphic = Graphic( aMtf );
            else
                aGraphic = rGraphic;
        }
        break;

        default:
            aGraphic = rGraphic;
        break;
    }

    if( aGraphic != rGraphic )
    {
        aGraphic.SetPrefSize( rGraphic.GetPrefSize() );
        aGraphic.SetPrefMapMode( rGraphic.GetPrefMapMode() );
    }

    return aGraphic;
}

//-------------------------------------------------------------------------

BOOL SvxBmpMask::IsEyedropping() const
{
    return aTbxPipette.IsItemChecked( TBI_PIPETTE );
}



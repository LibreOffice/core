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

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#define _SVX_TPLNEDEF_CXX

#include <cuires.hrc>
#include "tabline.hrc"
#include "helpid.hrc"

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>

#include "svx/drawitem.hxx"
#include "cuitabline.hxx"
#include "defdlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#define XOUT_WIDTH    150

SvxLineDefTabPage::SvxLineDefTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_LINE_DEF ), rInAttrs ),

    aFlDefinition   ( this, CUI_RES( FL_DEFINITION ) ),
    aFTLinestyle    ( this, CUI_RES( FT_LINESTYLE ) ),
    aLbLineStyles   ( this, CUI_RES( LB_LINESTYLES ) ),
    aFtType         ( this, CUI_RES( FT_TYPE ) ),
    aLbType1        ( this, CUI_RES( LB_TYPE_1 ) ),
    aLbType2        ( this, CUI_RES( LB_TYPE_2 ) ),
    aFtNumber       ( this, CUI_RES( FT_NUMBER ) ),
    aNumFldNumber1  ( this, CUI_RES( NUM_FLD_1 ) ),
    aNumFldNumber2  ( this, CUI_RES( NUM_FLD_2 ) ),
    aFtLength       ( this, CUI_RES( FT_LENGTH ) ),
    aMtrLength1     ( this, CUI_RES( MTR_FLD_LENGTH_1 ) ),
    aMtrLength2     ( this, CUI_RES( MTR_FLD_LENGTH_2 ) ),
    aFtDistance     ( this, CUI_RES( FT_DISTANCE ) ),
    aMtrDistance    ( this, CUI_RES( MTR_FLD_DISTANCE ) ),
    aCbxSynchronize ( this, CUI_RES( CBX_SYNCHRONIZE ) ),
    aBtnAdd         ( this, CUI_RES( BTN_ADD ) ),
    aBtnModify      ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnDelete      ( this, CUI_RES( BTN_DELETE ) ),
    aBtnLoad        ( this, CUI_RES( BTN_LOAD ) ),
    aBtnSave        ( this, CUI_RES( BTN_SAVE ) ),
    aCtlPreview     ( this, CUI_RES( CTL_PREVIEW ) ),

    rOutAttrs       ( rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( XOUT_WIDTH ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( String(), COL_BLACK ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() )
{
    aLbType1.SetAccessibleName(String(CUI_RES( STR_START_TYPE ) ) );
    aLbType2.SetAccessibleName(String(CUI_RES( STR_END_TYPE ) ) );
    aNumFldNumber1.SetAccessibleName(String(CUI_RES( STR_START_NUM ) ) );
    aNumFldNumber2.SetAccessibleName(String(CUI_RES( STR_END_NUM ) ) );
    aMtrLength1.SetAccessibleName(String(CUI_RES( STR_START_LENGTH ) ) );
    aMtrLength2.SetAccessibleName(String(CUI_RES( STR_END_LENGTH ) ) );

    FreeResource();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ; //prevent warning
    }
    SetFieldUnit( aMtrDistance, eFUnit );
    SetFieldUnit( aMtrLength1, eFUnit );
    SetFieldUnit( aMtrLength2, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXDash );
    rXLSet.Put( aXColor );

    // #i34740#
    aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

    aBtnAdd.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxLineDefTabPage, ClickSaveHdl_Impl ) );

    aNumFldNumber1.SetModifyHdl(
        LINK( this, SvxLineDefTabPage, ChangeNumber1Hdl_Impl ) );
    aNumFldNumber2.SetModifyHdl(
        LINK( this, SvxLineDefTabPage, ChangeNumber2Hdl_Impl ) );
    aLbLineStyles.SetSelectHdl(
        LINK( this, SvxLineDefTabPage, SelectLinestyleHdl_Impl ) );

    // absolute (in mm) or relative (in %)
    aCbxSynchronize.SetClickHdl(
        LINK( this, SvxLineDefTabPage, ChangeMetricHdl_Impl ) );

    // preview must be updated when there's something changed
    Link aLink = LINK( this, SvxLineDefTabPage, SelectTypeHdl_Impl );
    aLbType1.SetSelectHdl( aLink );
    aLbType2.SetSelectHdl( aLink );
    aLink = LINK( this, SvxLineDefTabPage, ChangePreviewHdl_Impl );
    aMtrLength1.SetModifyHdl( aLink );
    aMtrLength2.SetModifyHdl( aLink );
    aMtrDistance.SetModifyHdl( aLink );

    pDashList = NULL;

    aBtnAdd.SetAccessibleRelationMemberOf( &aFlDefinition );
    aBtnModify.SetAccessibleRelationMemberOf( &aFlDefinition );
    aBtnDelete.SetAccessibleRelationMemberOf( &aFlDefinition );
    aBtnLoad.SetAccessibleRelationMemberOf( &aFlDefinition );
    aBtnSave.SetAccessibleRelationMemberOf( &aFlDefinition );
}


// -----------------------------------------------------------------------

void SvxLineDefTabPage::Construct()
{
    aLbLineStyles.Fill( pDashList );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( *pDlgType == 0 ) // area dialog
    {
        // ActivatePage() is called before the dialog receives PageCreated() !!!
        if( pDashList.is() )
        {
            if( *pPageType == 1 &&
                *pPosDashLb != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbLineStyles.SelectEntryPos( *pPosDashLb );
            }
            // so that a possibly existing line style is discarded
            SelectLinestyleHdl_Impl( this );

            // determining (and possibly cutting) the name
            // and displaying it in the GroupBox
            String          aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pDashList->GetPath() );

            aURL.Append( pDashList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            *pPageType = 0; // 2
            *pPosDashLb = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxLineDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    CheckChanges_Impl();

    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::CheckChanges_Impl()
{
    // is here used to NOT lose changes
    //XDashStyle eXDS;

    if( aNumFldNumber1.GetText()     != aNumFldNumber1.GetSavedValue() ||
        aMtrLength1.GetText()        != aMtrLength1.GetSavedValue() ||
        aLbType1.GetSelectEntryPos() != aLbType1.GetSavedValue() ||
        aNumFldNumber2.GetText()     != aNumFldNumber2.GetSavedValue() ||
        aMtrLength2.GetText()        != aMtrLength2.GetSavedValue() ||
        aLbType2.GetSelectEntryPos() != aLbType2.GetSavedValue() ||
        aMtrDistance.GetText()       != aMtrDistance.GetSavedValue() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( GetParentDialog(), RID_SVXDLG_MESSBOX,
                                                    SVX_RESSTR( RID_SVXSTR_LINESTYLE ),
                                                    String( ResId( RID_SVXSTR_ASK_CHANGE_LINESTYLE, rMgr ) ),
                                                    &aWarningBoxImage );
        DBG_ASSERT(aMessDlg, "Dialogdiet fail!");
        aMessDlg->SetButtonText( MESS_BTN_1,
                                String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
        aMessDlg->SetButtonText( MESS_BTN_2,
                                String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

        short nRet = aMessDlg->Execute();

        switch( nRet )
        {
            case RET_BTN_1:
            {
                ClickModifyHdl_Impl( this );
            }
            break;

            case RET_BTN_2:
            {
                ClickAddHdl_Impl( this );
            }
            break;

            case RET_CANCEL:
            break;
        }
        delete aMessDlg;
    }



    sal_uInt16 nPos = aLbLineStyles.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPosDashLb = nPos;
    }
}

// -----------------------------------------------------------------------

sal_Bool SvxLineDefTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    if( *pDlgType == 0 ) // line dialog
    {
        if( *pPageType == 2 )
        {
            FillDash_Impl();

            String aString( aLbLineStyles.GetSelectEntry() );
            rAttrs.Put( XLineStyleItem( XLINE_DASH ) );
            rAttrs.Put( XLineDashItem( aString, aDash ) );
        }
    }
    return( sal_True );
}

// -----------------------------------------------------------------------

void SvxLineDefTabPage::Reset( const SfxItemSet& rAttrs )
{
    if( rAttrs.GetItemState( GetWhich( XATTR_LINESTYLE ) ) != SFX_ITEM_DONTCARE )
    {
        XLineStyle eXLS = (XLineStyle) ( ( const XLineStyleItem& ) rAttrs.Get( GetWhich( XATTR_LINESTYLE ) ) ).GetValue();

        switch( eXLS )
        {
            case XLINE_NONE:
            case XLINE_SOLID:
                aLbLineStyles.SelectEntryPos( 0 );
                break;

            case XLINE_DASH:
            {
                const XLineDashItem& rDashItem = ( const XLineDashItem& ) rAttrs.Get( XATTR_LINEDASH );
                aDash = rDashItem.GetDashValue();

                aLbLineStyles.SetNoSelection();
                aLbLineStyles.SelectEntry( rDashItem.GetName() );
            }
                break;

            default:
                break;
        }
    }
    SelectLinestyleHdl_Impl( NULL );

    // determine button state
    if( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineDefTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxLineDefTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectLinestyleHdl_Impl, void *, p )
{
    if( pDashList->Count() > 0 )
    {
        int nTmp = aLbLineStyles.GetSelectEntryPos();
        if( nTmp == LISTBOX_ENTRY_NOTFOUND )
        {
        }
        else
            aDash = pDashList->GetDash( nTmp )->GetDash();

        FillDialog_Impl();

        rXLSet.Put( XLineDashItem( String(), aDash ) );

        // #i34740#
        aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

        aCtlPreview.Invalidate();

        // Is not set before, in order to take the new style
        // only if there was an entry selected in the ListBox.
        // If it was called via Reset(), then p is == NULL
        if( p )
            *pPageType = 2;
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxLineDefTabPage, ChangePreviewHdl_Impl)
{
    FillDash_Impl();
    aCtlPreview.Invalidate();

    return( 0L );
}
IMPL_LINK_NOARG_INLINE_END(SvxLineDefTabPage, ChangePreviewHdl_Impl)

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber1Hdl_Impl)
{
    if( aNumFldNumber1.GetValue() == 0L )
    {
        aNumFldNumber2.SetMin( 1L );
        aNumFldNumber2.SetFirst( 1L );
    }
    else
    {
        aNumFldNumber2.SetMin( 0L );
        aNumFldNumber2.SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber2Hdl_Impl)
{
    if( aNumFldNumber2.GetValue() == 0L )
    {
        aNumFldNumber1.SetMin( 1L );
        aNumFldNumber1.SetFirst( 1L );
    }
    else
    {
        aNumFldNumber1.SetMin( 0L );
        aNumFldNumber1.SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}


//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ChangeMetricHdl_Impl, void *, p )
{
    if( !aCbxSynchronize.IsChecked() && aMtrLength1.GetUnit() != eFUnit )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit ) * XOUT_WIDTH / 100;
        }
        else
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit );
        }
        aMtrLength1.SetDecimalDigits( 2 );
        aMtrLength2.SetDecimalDigits( 2 );
        aMtrDistance.SetDecimalDigits( 2 );

        // adjust metric
        aMtrLength1.SetUnit( eFUnit );
        aMtrLength2.SetUnit( eFUnit );
        aMtrDistance.SetUnit( eFUnit );

        SetMetricValue( aMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( aMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( aMtrDistance, nTmp3, ePoolUnit );
    }
    else if( aCbxSynchronize.IsChecked() && aMtrLength1.GetUnit() != FUNIT_CUSTOM )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit ) * 100 / XOUT_WIDTH;
        }
        else
        {
            nTmp1 = GetCoreValue( aMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( aMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( aMtrDistance, ePoolUnit );
        }

        aMtrLength1.SetDecimalDigits( 0 );
        aMtrLength2.SetDecimalDigits( 0 );
        aMtrDistance.SetDecimalDigits( 0 );

        aMtrLength1.SetUnit( FUNIT_CUSTOM );
        aMtrLength2.SetUnit( FUNIT_CUSTOM );
        aMtrDistance.SetUnit( FUNIT_CUSTOM );


        SetMetricValue( aMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( aMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( aMtrDistance, nTmp3, ePoolUnit );
    }
    SelectTypeHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectTypeHdl_Impl, void *, p )
{
    if ( p == &aLbType1 || !p )
    {
        if ( aLbType1.GetSelectEntryPos() == 0 )
        {
            aMtrLength1.Disable();
            aMtrLength1.SetText( String() );
        }
        else if ( !aMtrLength1.IsEnabled() )
        {
            aMtrLength1.Enable();
            aMtrLength1.Reformat();
        }
    }

    if ( p == &aLbType2 || !p )
    {
        if ( aLbType2.GetSelectEntryPos() == 0 )
        {
            aMtrLength2.Disable();
            aMtrLength2.SetText( String() );
        }
        else if ( !aMtrLength2.IsEnabled() )
        {
            aMtrLength2.Enable();
            aMtrLength2.Reformat();
        }
    }
    ChangePreviewHdl_Impl( p );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickAddHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    String aNewName( SVX_RES( RID_SVXSTR_LINESTYLE ) );
    String aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, rMgr ) );
    String aName;
    XDashEntry* pEntry;

    long nCount = pDashList->Count();
    long j = 1;
    sal_Bool bDifferent = sal_False;

    while ( !bDifferent )
    {
        aName = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = sal_True;

        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = sal_False;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    sal_Bool bLoop = sal_True;

    while ( bLoop && pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = sal_False;
        }

        if( bDifferent )
        {
            bLoop = sal_False;
            FillDash_Impl();

            pEntry = new XDashEntry( aDash, aName );

            long nDashCount = pDashList->Count();
            pDashList->Insert( pEntry, nDashCount );
            Bitmap* pBitmap = pDashList->GetBitmap( nDashCount );
            aLbLineStyles.Append( pEntry, pBitmap );

            aLbLineStyles.SelectEntryPos( aLbLineStyles.GetEntryCount() - 1 );

            *pnDashListState |= CT_MODIFIED;

            *pPageType = 2;

            // save values for changes recognition (-> method)
            aNumFldNumber1.SaveValue();
            aMtrLength1.SaveValue();
            aLbType1.SaveValue();
            aNumFldNumber2.SaveValue();
            aMtrLength2.SaveValue();
            aLbType2.SaveValue();
            aMtrDistance.SaveValue();
        }
        else
        {
            WarningBox aBox( GetParentDialog(), WinBits( WB_OK ),String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
            aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
            aBox.Execute();
        }
    }
    delete( pDlg );

    // determine button state
    if ( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = aLbLineStyles.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aNewName( SVX_RES( RID_SVXSTR_LINESTYLE ) );
        String aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, rMgr ) );
        String aName( pDashList->GetDash( nPos )->GetName() );
        String aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");

        long nCount = pDashList->Count();
        sal_Bool bDifferent = sal_False;
        sal_Bool bLoop = sal_True;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pDashList->GetDash( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = sal_False;
            }

            if ( bDifferent )
            {
                bLoop = sal_False;
                FillDash_Impl();

                XDashEntry* pEntry = new XDashEntry( aDash, aName );

                delete pDashList->Replace( pEntry, nPos );
                Bitmap* pBitmap = pDashList->GetBitmap( nPos );
                aLbLineStyles.Modify( pEntry, nPos, pBitmap );

                aLbLineStyles.SelectEntryPos( nPos );

                *pnDashListState |= CT_MODIFIED;

                *pPageType = 2;

                // save values for changes recognition (-> method)
                aNumFldNumber1.SaveValue();
                aMtrLength1.SaveValue();
                aLbType1.SaveValue();
                aNumFldNumber2.SaveValue();
                aMtrLength2.SaveValue();
                aLbType2.SaveValue();
                aMtrDistance.SaveValue();
            }
            else
            {
                WarningBox aBox( GetParentDialog(), WinBits( WB_OK ), String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
                aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
                aBox.Execute();
            }
        }
        delete( pDlg );
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = aLbLineStyles.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_LINESTYLE ) ) );

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pDashList->Remove( nPos );
            aLbLineStyles.RemoveEntry( nPos );
            aLbLineStyles.SelectEntryPos( 0 );

            SelectLinestyleHdl_Impl( this );
            *pPageType = 0; // style should not be taken

            *pnDashListState |= CT_MODIFIED;

            ChangePreviewHdl_Impl( this );
        }
    }

    // determine button state
    if ( !pDashList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickLoadHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *pnDashListState & CT_MODIFIED )
    {
        nReturn = WarningBox( GetParentDialog(), WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pDashList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sod" ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XDashListRef pDshLst = XPropertyList::CreatePropertyList(
                XGRADIENT_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), pXPool )->AsDashList();
            pDshLst->SetName( aURL.getName() );

            if( pDshLst->Load() )
            {
                pDashList = pDshLst;
                ( (SvxLineTabDialog*) GetParentDialog() )->SetNewDashList( pDashList );

                aLbLineStyles.Clear();
                aLbLineStyles.Fill( pDashList );
                Reset( rOutAttrs );

                pDashList->SetName( aURL.getName() );

                *pnDashListState |= CT_CHANGED;
                *pnDashListState &= ~CT_MODIFIED;
            }
            else
                //aIStream.Close();
                ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
        }
    }

    // determine button state
    if ( pDashList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sod" ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pDashList->GetName().Len() )
    {
        aFile.Append( pDashList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension(rtl::OUString("sod"));
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pDashList->SetName( aURL.getName() );
        pDashList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pDashList->Save() )
        {
            *pnDashListState |= CT_SAVED;
            *pnDashListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDash_Impl()
{
    XDashStyle eXDS;

    if( aCbxSynchronize.IsChecked() )
        eXDS = XDASH_RECTRELATIVE;
    else
        eXDS = XDASH_RECT;

    aDash.SetDashStyle( eXDS );
    aDash.SetDots( (sal_uInt8) aNumFldNumber1.GetValue() );
    aDash.SetDotLen( aLbType1.GetSelectEntryPos() == 0 ? 0 :
                                GetCoreValue( aMtrLength1, ePoolUnit ) );
    aDash.SetDashes( (sal_uInt8) aNumFldNumber2.GetValue() );
    aDash.SetDashLen( aLbType2.GetSelectEntryPos() == 0 ? 0 :
                                GetCoreValue( aMtrLength2, ePoolUnit ) );
    aDash.SetDistance( GetCoreValue( aMtrDistance, ePoolUnit ) );

    rXLSet.Put( XLineDashItem( String(), aDash ) );

    // #i34740#
    aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDialog_Impl()
{
    XDashStyle eXDS = aDash.GetDashStyle(); // XDASH_RECT, XDASH_ROUND
    if( eXDS == XDASH_RECTRELATIVE )
        aCbxSynchronize.Check();
    else
        aCbxSynchronize.Check( sal_False );

    aNumFldNumber1.SetValue( aDash.GetDots() );
    SetMetricValue( aMtrLength1, aDash.GetDotLen(), ePoolUnit );
    aLbType1.SelectEntryPos( aDash.GetDotLen() == 0 ? 0 : 1 );
    aNumFldNumber2.SetValue( aDash.GetDashes() );
    SetMetricValue( aMtrLength2, aDash.GetDashLen(), ePoolUnit );
    aLbType2.SelectEntryPos( aDash.GetDashLen() == 0 ? 0 : 1 );
    SetMetricValue( aMtrDistance, aDash.GetDistance(), ePoolUnit );

    ChangeMetricHdl_Impl( NULL );

    // save values for changes recognition (-> method)
    aNumFldNumber1.SaveValue();
    aMtrLength1.SaveValue();
    aLbType1.SaveValue();
    aNumFldNumber2.SaveValue();
    aMtrLength2.SaveValue();
    aLbType2.SaveValue();
    aMtrDistance.SaveValue();
}


void SvxLineDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        sal_uInt16 nOldSelect = aLbLineStyles.GetSelectEntryPos();
        aLbLineStyles.Clear();
        aLbLineStyles.Fill( pDashList );
        aLbLineStyles.SelectEntryPos( nOldSelect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

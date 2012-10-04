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

#define _SVX_TPHATCH_CXX

#include <cuires.hrc>
#include "tabarea.hrc"
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include "paragrph.hrc"
#include <svx/dialogs.hrc>

SvxHatchTabPage::SvxHatchTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SvxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_HATCH ), rInAttrs ),

    aFtDistance         ( this, CUI_RES( FT_LINE_DISTANCE ) ),
    aMtrDistance        ( this, CUI_RES( MTR_FLD_DISTANCE ) ),
    aFtAngle            ( this, CUI_RES( FT_LINE_ANGLE ) ),
    aMtrAngle           ( this, CUI_RES( MTR_FLD_ANGLE ) ),
    aCtlAngle           ( this, CUI_RES( CTL_ANGLE ),
                                    RP_RB, 200, 80, CS_ANGLE ),
    aFlProp             ( this, CUI_RES( FL_PROP ) ),
    aFtLineType         ( this, CUI_RES( FT_LINE_TYPE ) ),
    aLbLineType         ( this, CUI_RES( LB_LINE_TYPE ) ),
    aFtLineColor        ( this, CUI_RES( FT_LINE_COLOR ) ),
    aLbLineColor        ( this, CUI_RES( LB_LINE_COLOR ) ),
    aLbHatchings        ( this, CUI_RES( LB_HATCHINGS ) ),
    aCtlPreview         ( this, CUI_RES( CTL_PREVIEW ) ),
    aBtnAdd             ( this, CUI_RES( BTN_ADD ) ),
    aBtnModify          ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnDelete          ( this, CUI_RES( BTN_DELETE ) ),
    aBtnLoad            ( this, CUI_RES( BTN_LOAD ) ),
    aBtnSave            ( this, CUI_RES( BTN_SAVE ) ),

    rOutAttrs           ( rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_HATCH ),
    aXHatchItem         ( String(), XHatch() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )

{
    FreeResource();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( aMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXHatchItem );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );

    aLbHatchings.SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl_Impl ) );

    Link aLink = LINK( this, SvxHatchTabPage, ModifiedHdl_Impl );
    aMtrDistance.SetModifyHdl( aLink );
    aMtrAngle.SetModifyHdl( aLink );
    aLbLineType.SetSelectHdl( aLink );
    aLbLineColor.SetSelectHdl( aLink );

    aBtnAdd.SetClickHdl( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxHatchTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxHatchTabPage, ClickSaveHdl_Impl ) );

    aCtlPreview.SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    aCtlPreview.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));
    aLbHatchings.SetAccessibleName( String(CUI_RES(STR_LB_HATCHINGSTYLE)) );
    aCtlAngle.SetAccessibleRelationMemberOf( &aFlProp );
    aLbHatchings.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnAdd.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnModify.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnDelete.SetAccessibleRelationMemberOf( &aFlProp );
    aLbHatchings.SetAccessibleRelationLabeledBy(&aLbHatchings);
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::Construct()
{
    aLbLineColor.Fill( pColorList );
    aLbHatchings.Fill( pHatchingList );
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16 nPos;
    sal_uInt16 nCount;

    if( *pDlgType == 0 ) // area dialog
    {
        *pbAreaTP = sal_False;

        if( pColorList.is() )
        {
            // ColorList
            if( *pnColorListState & CT_CHANGED ||
                *pnColorListState & CT_MODIFIED )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxAreaTabDialog*) GetParentDialog() )->GetNewColorList();

                // LbLineColor
                nPos = aLbLineColor.GetSelectEntryPos();
                aLbLineColor.Clear();
                aLbLineColor.Fill( pColorList );
                nCount = aLbLineColor.GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    aLbLineColor.SelectEntryPos( 0 );
                else
                    aLbLineColor.SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            String          aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pHatchingList->GetPath() );

            aURL.Append( pHatchingList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            if( *pPageType == PT_HATCH && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbHatchings.SelectEntryPos( *pPos );
            }
            // colors could have been deleted
            ChangeHatchHdl_Impl( this );

            *pPageType = PT_HATCH;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();
}

// -----------------------------------------------------------------------

int SvxHatchTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

long SvxHatchTabPage::CheckChanges_Impl()
{
    if( aMtrDistance.GetText()           != aMtrDistance.GetSavedValue() ||
        aMtrAngle.GetText()              != aMtrAngle.GetSavedValue() ||
        aLbLineType.GetSelectEntryPos()  != aLbLineType.GetSavedValue()  ||
        aLbLineColor.GetSelectEntryPos() != aLbLineColor.GetSavedValue() ||
        aLbHatchings.GetSelectEntryPos() != aLbHatchings.GetSavedValue() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( GetParentDialog(), RID_SVXDLG_MESSBOX,
                                                        SVX_RESSTR( RID_SVXSTR_HATCH ),
                                                        CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_HATCH ),
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

    sal_uInt16 nPos = aLbHatchings.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}

// -----------------------------------------------------------------------

sal_Bool SvxHatchTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( *pDlgType == 0 && *pbAreaTP == sal_False ) // area dialog
    {
        if( *pPageType == PT_HATCH )
        {
            // CheckChanges(); <-- duplicate inquiry ?

            XHatch* pXHatch = NULL;
            String  aString;
            sal_uInt16  nPos = aLbHatchings.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                pXHatch = new XHatch( pHatchingList->GetHatch( nPos )->GetHatch() );
                aString = aLbHatchings.GetSelectEntry();
            }
            // gradient has been (unidentifiedly) passed
            else
            {
                pXHatch = new XHatch( aLbLineColor.GetSelectEntryColor(),
                                 (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                                 GetCoreValue( aMtrDistance, ePoolUnit ),
                                 static_cast<long>(aMtrAngle.GetValue() * 10) );
            }
            DBG_ASSERT( pXHatch, "XHatch konnte nicht erzeugt werden" );
            rSet.Put( XFillStyleItem( XFILL_HATCH ) );
            rSet.Put( XFillHatchItem( aString, *pXHatch ) );

            delete pXHatch;
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::Reset( const SfxItemSet& rSet )
{
    ChangeHatchHdl_Impl( this );

    // determine button state
    if( pHatchingList->Count() )
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

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxHatchTabPage::Create( Window* pWindow,
                const SfxItemSet& rSet )
{
    return new SvxHatchTabPage( pWindow, rSet );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ModifiedHdl_Impl, void *, p )
{
    if( p == &aMtrAngle )
    {
        switch( aMtrAngle.GetValue() )
        {
            case 135: aCtlAngle.SetActualRP( RP_LT ); break;
            case  90: aCtlAngle.SetActualRP( RP_MT ); break;
            case  45: aCtlAngle.SetActualRP( RP_RT ); break;
            case 180: aCtlAngle.SetActualRP( RP_LM ); break;
            case   0: aCtlAngle.SetActualRP( RP_RM ); break;
            case 225: aCtlAngle.SetActualRP( RP_LB ); break;
            case 270: aCtlAngle.SetActualRP( RP_MB ); break;
            case 315: aCtlAngle.SetActualRP( RP_RB ); break;
            default:  aCtlAngle.SetActualRP( RP_MM ); break;
        }
    }

    XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                    (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                    GetCoreValue( aMtrDistance, ePoolUnit ),
                    static_cast<long>(aMtrAngle.GetValue() * 10) );

    rXFSet.Put( XFillHatchItem( String(), aXHatch ) );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );

    aCtlPreview.Invalidate();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ChangeHatchHdl_Impl)
{
    XHatch* pHatch = NULL;
    int nPos = aLbHatchings.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pHatch = new XHatch( ( (XHatchEntry*) pHatchingList->GetHatch( nPos ) )->GetHatch() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), sal_True, &pPoolItem ) )
        {
            if( ( XFILL_HATCH == (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue() ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), sal_True, &pPoolItem ) ) )
            {
                pHatch = new XHatch( ( ( const XFillHatchItem* ) pPoolItem )->GetHatchValue() );
            }
        }
        if( !pHatch )
        {
            aLbHatchings.SelectEntryPos( 0 );
            nPos = aLbHatchings.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pHatch = new XHatch( ( (XHatchEntry*) pHatchingList->GetHatch( nPos ) )->GetHatch() );
        }
    }
    if( pHatch )
    {
        aLbLineType.SelectEntryPos(
            sal::static_int_cast< sal_uInt16 >( pHatch->GetHatchStyle() ) );
        // if the entry is not in the listbox
        // the color is added temporarily
        aLbLineColor.SetNoSelection();
        aLbLineColor.SelectEntry( pHatch->GetColor() );
        if( aLbLineColor.GetSelectEntryCount() == 0 )
        {
            aLbLineColor.InsertEntry( pHatch->GetColor(), String() );
            aLbLineColor.SelectEntry( pHatch->GetColor() );
        }
        SetMetricValue( aMtrDistance, pHatch->GetDistance(), ePoolUnit );
        aMtrAngle.SetValue( pHatch->GetAngle() / 10 );

        switch( aMtrAngle.GetValue() )
        {
            case 135: aCtlAngle.SetActualRP( RP_LT ); break;
            case  90: aCtlAngle.SetActualRP( RP_MT ); break;
            case  45: aCtlAngle.SetActualRP( RP_RT ); break;
            case 180: aCtlAngle.SetActualRP( RP_LM ); break;
            case   0: aCtlAngle.SetActualRP( RP_RM ); break;
            case 225: aCtlAngle.SetActualRP( RP_LB ); break;
            case 270: aCtlAngle.SetActualRP( RP_MB ); break;
            case 315: aCtlAngle.SetActualRP( RP_RB ); break;
            default:  aCtlAngle.SetActualRP( RP_MM ); break;
        }

        // fill ItemSet and pass it on to aCtlPreview
        rXFSet.Put( XFillHatchItem( String(), *pHatch ) );
        aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreview.Invalidate();
        delete pHatch;
    }
    aMtrDistance.SaveValue();
    aMtrAngle.SaveValue();
    aLbLineType.SaveValue();
    aLbLineColor.SaveValue();
    aLbHatchings.SaveValue();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickAddHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    String aNewName( SVX_RES( RID_SVXSTR_HATCH ) );
    String aDesc( CUI_RES( RID_SVXSTR_DESC_HATCH ) );
    String aName;

    long nCount = pHatchingList->Count();
    long j = 1;
    sal_Bool bDifferent = sal_False;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pHatchingList->GetHatch( i )->GetName() )
                bDifferent = sal_False;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    WarningBox*    pWarnBox = NULL;
    sal_uInt16         nError   = RID_SVXSTR_WARN_NAME_DUPLICATE;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pHatchingList->GetHatch( i )->GetName() )
                bDifferent = sal_False;

        if( bDifferent ) {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox = new WarningBox( GetParentDialog(),
                                       WinBits( WB_OK_CANCEL ),
                                       String( ResId( nError, rMgr ) ) );
            pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                        (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                        GetCoreValue( aMtrDistance, ePoolUnit ),
                        static_cast<long>(aMtrAngle.GetValue() * 10) );
        XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

        pHatchingList->Insert( pEntry, nCount );

        aLbHatchings.Append( pEntry );

        aLbHatchings.SelectEntryPos( aLbHatchings.GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( aLbHatchings.GetPosPixel(), aLbHatchings.GetSizePixel() );
        if( sal_True ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
        }
#endif

        *pnHatchingListState |= CT_MODIFIED;

        ChangeHatchHdl_Impl( this );
    }

    // determine button state
    if( pHatchingList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = aLbHatchings.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aNewName( SVX_RES( RID_SVXSTR_HATCH ) );
        String aDesc( CUI_RES( RID_SVXSTR_DESC_HATCH ) );
        String aName( pHatchingList->GetHatch( nPos )->GetName() );
        String aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");

        long nCount = pHatchingList->Count();
        sal_Bool bDifferent = sal_False;
        sal_Bool bLoop = sal_True;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pHatchingList->GetHatch( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = sal_False;
            }

            if( bDifferent )
            {
                bLoop = sal_False;
                XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                                (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                                 GetCoreValue( aMtrDistance, ePoolUnit ),
                                static_cast<long>(aMtrAngle.GetValue() * 10) );

                XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

                delete pHatchingList->Replace( pEntry, nPos );

                aLbHatchings.Modify( pEntry, nPos );

                aLbHatchings.SelectEntryPos( nPos );

                // save values for changes recognition (-> method)
                aMtrDistance.SaveValue();
                aMtrAngle.SaveValue();
                aLbLineType.SaveValue();
                aLbLineColor.SaveValue();
                aLbHatchings.SaveValue();

                *pnHatchingListState |= CT_MODIFIED;
            }
            else
            {
                WarningBox aBox( GetParentDialog(), WinBits( WB_OK ),String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
                aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
                aBox.Execute();
            }
        }
        delete( pDlg );
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = aLbHatchings.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_HATCH ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pHatchingList->Remove( nPos );
            aLbHatchings.RemoveEntry( nPos );
            aLbHatchings.SelectEntryPos( 0 );

            aCtlPreview.Invalidate();

            ChangeHatchHdl_Impl( this );

            *pnHatchingListState |= CT_MODIFIED;
        }
    }
    // determine button state
    if( !pHatchingList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickLoadHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *pnHatchingListState & CT_MODIFIED )
    {
        nReturn = WarningBox( GetParentDialog(), WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pHatchingList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soh" ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XHatchListRef pHatchList = XPropertyList::CreatePropertyList(
                XHATCH_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), pXPool )->AsHatchList();
            pHatchList->SetName( aURL.getName() );
            if( pHatchList->Load() )
            {
                pHatchingList = pHatchList;
                ( (SvxAreaTabDialog*) GetParentDialog() )->SetNewHatchingList( pHatchingList );

                aLbHatchings.Clear();
                aLbHatchings.Fill( pHatchingList );
                Reset( rOutAttrs );

                pHatchingList->SetName( aURL.getName() );

                // determining (and possibly cutting) the name
                // and displaying it in the GroupBox
                String aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += String(aURL.getBase()).Copy( 0, 15 );
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
                }
                else
                    aString += String(aURL.getBase());

                *pnHatchingListState |= CT_CHANGED;
                *pnHatchingListState &= ~CT_MODIFIED;
            }
            else
                ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
        }
    }

    // determine button state
    if ( pHatchingList->Count() )
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
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soh" ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pHatchingList->GetName().Len() )
    {
        aFile.Append( pHatchingList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension(rtl::OUString("soh"));
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pHatchingList->SetName( aURL.getName() );
        pHatchingList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pHatchingList->Save() )
        {
            // determining (and possibly cutting) the name
            // and displaying it in the GroupBox
            String aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            *pnHatchingListState |= CT_SAVED;
            *pnHatchingListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }

    return 0L;
}

//------------------------------------------------------------------------

void SvxHatchTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    if( pWindow == &aCtlAngle )
    {
        switch( eRcPt )
        {
            case RP_LT: aMtrAngle.SetValue( 135 ); break;
            case RP_MT: aMtrAngle.SetValue( 90 );  break;
            case RP_RT: aMtrAngle.SetValue( 45 );  break;
            case RP_LM: aMtrAngle.SetValue( 180 ); break;
            case RP_RM: aMtrAngle.SetValue( 0 );   break;
            case RP_LB: aMtrAngle.SetValue( 225 ); break;
            case RP_MB: aMtrAngle.SetValue( 270 ); break;
            case RP_RB: aMtrAngle.SetValue( 315 ); break;
            case RP_MM: break;
        }
        ModifiedHdl_Impl( this );
    }
}


void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        aCtlPreview.SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SvxTabPage::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

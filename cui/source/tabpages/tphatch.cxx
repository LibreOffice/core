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
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/module.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
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

    SvxTabPage          ( pParent, "HatchPage", "cui/ui/hatchpage.ui", rInAttrs ),

    rOutAttrs           ( rInAttrs ),

    pnHatchingListState ( 0 ),
    pnColorListState    ( 0 ),
    pPageType           ( 0 ),
    pDlgType            ( 0 ),
    pPos                ( 0 ),
    pbAreaTP            ( 0 ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_HATCH ),
    aXHatchItem         ( String(), XHatch() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )

{
    get(m_pMtrDistance, "distancemtr");
    get(m_pMtrAngle, "anglemtr");
    get(m_pCtlAngle, "anglectl");
    m_pCtlAngle->SetCS(CS_ANGLE);
    get(m_pLbLineType, "linetypelb");
    get(m_pLbLineColor, "linecolorlb");
    get(m_pLbHatchings, "hatchingslb");
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbHatchings->set_width_request(aSize.Width());
    m_pLbHatchings->set_height_request(aSize.Height());
    get(m_pCtlPreview, "previewctl");
    aSize = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd, "add");
    get(m_pBtnModify, "modify");
    get(m_pBtnDelete, "delete");
    get(m_pBtnLoad, "load");
    get(m_pBtnSave, "save");

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
    SetFieldUnit( *m_pMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXHatchItem );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pLbHatchings->SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl_Impl ) );

    Link aLink = LINK( this, SvxHatchTabPage, ModifiedHdl_Impl );
    m_pMtrDistance->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pLbLineType->SetSelectHdl( aLink );
    m_pLbLineColor->SetSelectHdl( aLink );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxHatchTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxHatchTabPage, ClickSaveHdl_Impl ) );

    m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    setPreviewsToSamePlace(pParent, this);
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::Construct()
{
    m_pLbLineColor->Fill( pColorList );
    m_pLbHatchings->Fill( pHatchingList );
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
                nPos = m_pLbLineColor->GetSelectEntryPos();
                m_pLbLineColor->Clear();
                m_pLbLineColor->Fill( pColorList );
                nCount = m_pLbLineColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbLineColor->SelectEntryPos( 0 );
                else
                    m_pLbLineColor->SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( pHatchingList->GetPath() );

            aURL.Append( pHatchingList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            if( *pPageType == PT_HATCH && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbHatchings->SelectEntryPos( *pPos );
            }
            // colors could have been deleted
            ChangeHatchHdl_Impl( this );

            *pPageType = PT_HATCH;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
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
    if( m_pMtrDistance->GetText()           != m_pMtrDistance->GetSavedValue() ||
        m_pMtrAngle->GetText()              != m_pMtrAngle->GetSavedValue() ||
        m_pLbLineType->GetSelectEntryPos()  != m_pLbLineType->GetSavedValue()  ||
        m_pLbLineColor->GetSelectEntryPos() != m_pLbLineColor->GetSavedValue() ||
        m_pLbHatchings->GetSelectEntryPos() != m_pLbHatchings->GetSavedValue() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        SvxMessDialog* aMessDlg = new SvxMessDialog(GetParentDialog(),
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

    sal_uInt16 nPos = m_pLbHatchings->GetSelectEntryPos();
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
            sal_uInt16  nPos = m_pLbHatchings->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                pXHatch = new XHatch( pHatchingList->GetHatch( nPos )->GetHatch() );
                aString = m_pLbHatchings->GetSelectEntry();
            }
            // gradient has been (unidentifiedly) passed
            else
            {
                pXHatch = new XHatch( m_pLbLineColor->GetSelectEntryColor(),
                                 (XHatchStyle) m_pLbLineType->GetSelectEntryPos(),
                                 GetCoreValue( *m_pMtrDistance, ePoolUnit ),
                                 static_cast<long>(m_pMtrAngle->GetValue() * 10) );
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
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
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
    if( p == m_pMtrAngle )
    {
        switch( m_pMtrAngle->GetValue() )
        {
            case 135: m_pCtlAngle->SetActualRP( RP_LT ); break;
            case  90: m_pCtlAngle->SetActualRP( RP_MT ); break;
            case  45: m_pCtlAngle->SetActualRP( RP_RT ); break;
            case 180: m_pCtlAngle->SetActualRP( RP_LM ); break;
            case   0: m_pCtlAngle->SetActualRP( RP_RM ); break;
            case 225: m_pCtlAngle->SetActualRP( RP_LB ); break;
            case 270: m_pCtlAngle->SetActualRP( RP_MB ); break;
            case 315: m_pCtlAngle->SetActualRP( RP_RB ); break;
            default:  m_pCtlAngle->SetActualRP( RP_MM ); break;
        }
    }

    XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                    (XHatchStyle) m_pLbLineType->GetSelectEntryPos(),
                    GetCoreValue( *m_pMtrDistance, ePoolUnit ),
                    static_cast<long>(m_pMtrAngle->GetValue() * 10) );

    rXFSet.Put( XFillHatchItem( String(), aXHatch ) );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ChangeHatchHdl_Impl)
{
    XHatch* pHatch = NULL;
    int nPos = m_pLbHatchings->GetSelectEntryPos();

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
            m_pLbHatchings->SelectEntryPos( 0 );
            nPos = m_pLbHatchings->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pHatch = new XHatch( ( (XHatchEntry*) pHatchingList->GetHatch( nPos ) )->GetHatch() );
        }
    }
    if( pHatch )
    {
        m_pLbLineType->SelectEntryPos(
            sal::static_int_cast< sal_uInt16 >( pHatch->GetHatchStyle() ) );
        // if the entry is not in the listbox
        // the color is added temporarily
        m_pLbLineColor->SetNoSelection();
        m_pLbLineColor->SelectEntry( pHatch->GetColor() );
        if( m_pLbLineColor->GetSelectEntryCount() == 0 )
        {
            m_pLbLineColor->InsertEntry( pHatch->GetColor(), String() );
            m_pLbLineColor->SelectEntry( pHatch->GetColor() );
        }
        SetMetricValue( *m_pMtrDistance, pHatch->GetDistance(), ePoolUnit );
        m_pMtrAngle->SetValue( pHatch->GetAngle() / 10 );

        switch( m_pMtrAngle->GetValue() )
        {
            case 135: m_pCtlAngle->SetActualRP( RP_LT ); break;
            case  90: m_pCtlAngle->SetActualRP( RP_MT ); break;
            case  45: m_pCtlAngle->SetActualRP( RP_RT ); break;
            case 180: m_pCtlAngle->SetActualRP( RP_LM ); break;
            case   0: m_pCtlAngle->SetActualRP( RP_RM ); break;
            case 225: m_pCtlAngle->SetActualRP( RP_LB ); break;
            case 270: m_pCtlAngle->SetActualRP( RP_MB ); break;
            case 315: m_pCtlAngle->SetActualRP( RP_RB ); break;
            default:  m_pCtlAngle->SetActualRP( RP_MM ); break;
        }

        // fill ItemSet and pass it on to m_pCtlPreview
        rXFSet.Put( XFillHatchItem( String(), *pHatch ) );
        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
        delete pHatch;
    }
    m_pMtrDistance->SaveValue();
    m_pMtrAngle->SaveValue();
    m_pLbLineType->SaveValue();
    m_pLbLineColor->SaveValue();
    m_pLbHatchings->SaveValue();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickAddHdl_Impl)
{
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
        aName += OUString::number( j++ );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pHatchingList->GetHatch( i )->GetName() )
                bDifferent = sal_False;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    MessageDialog*    pWarnBox = NULL;
    sal_uInt16         nError   = 1;

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
            pWarnBox = new MessageDialog( GetParentDialog()
                                         ,"DuplicateNameDialog"
                                         ,"cui/ui/queryduplicatedialog.ui");
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                        (XHatchStyle) m_pLbLineType->GetSelectEntryPos(),
                        GetCoreValue( *m_pMtrDistance, ePoolUnit ),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10) );
        XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

        pHatchingList->Insert( pEntry, nCount );

        m_pLbHatchings->Append( *pEntry, pHatchingList->GetUiBitmap( nCount ) );

        m_pLbHatchings->SelectEntryPos( m_pLbHatchings->GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( m_pLbHatchings->GetPosPixel(), m_pLbHatchings->GetSizePixel() );
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
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbHatchings->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
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
                XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                                (XHatchStyle) m_pLbLineType->GetSelectEntryPos(),
                                 GetCoreValue( *m_pMtrDistance, ePoolUnit ),
                                static_cast<long>(m_pMtrAngle->GetValue() * 10) );

                XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

                delete pHatchingList->Replace( pEntry, nPos );

                m_pLbHatchings->Modify( *pEntry, nPos, pHatchingList->GetUiBitmap( nPos ) );

                m_pLbHatchings->SelectEntryPos( nPos );

                // save values for changes recognition (-> method)
                m_pMtrDistance->SaveValue();
                m_pMtrAngle->SaveValue();
                m_pLbLineType->SaveValue();
                m_pLbLineColor->SaveValue();
                m_pLbHatchings->SaveValue();

                *pnHatchingListState |= CT_MODIFIED;
            }
            else
            {
                MessageDialog aBox( GetParentDialog()
                                    ,"DuplicateNameDialog"
                                    ,"cui/ui/queryduplicatedialog.ui");
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
    sal_uInt16 nPos = m_pLbHatchings->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_HATCH ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pHatchingList->Remove( nPos );
            m_pLbHatchings->RemoveEntry( nPos );
            m_pLbHatchings->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();

            ChangeHatchHdl_Impl( this );

            *pnHatchingListState |= CT_MODIFIED;
        }
    }
    // determine button state
    if( !pHatchingList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
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
        nReturn = MessageDialog( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            pHatchingList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,  0 );
        String aStrFilterType( "*.soh" );
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
                XHATCH_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) )->AsHatchList();
            pHatchList->SetName( aURL.getName() );
            if( pHatchList->Load() )
            {
                pHatchingList = pHatchList;
                ( (SvxAreaTabDialog*) GetParentDialog() )->SetNewHatchingList( pHatchingList );

                m_pLbHatchings->Clear();
                m_pLbHatchings->Fill( pHatchingList );
                Reset( rOutAttrs );

                pHatchingList->SetName( aURL.getName() );

                // determining (and possibly cutting) the name
                // and displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 );
                    aString += "...";
                }
                else
                    aString += aURL.getBase();

                *pnHatchingListState |= CT_CHANGED;
                *pnHatchingListState &= ~CT_MODIFIED;
            }
            else
                MessageDialog( GetParentDialog()
                               ,"NoLoadedFileDialog"
                               ,"cui/ui/querynoloadedfiledialog.ui").Execute();
        }
    }

    // determine button state
    if ( pHatchingList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxHatchTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( "*.soh" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pHatchingList->GetName().Len() )
    {
        aFile.Append( pHatchingList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( OUString("soh") );
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
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            *pnHatchingListState |= CT_SAVED;
            *pnHatchingListState &= ~CT_MODIFIED;
        }
        else
        {
            MessageDialog( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui").Execute();
        }
    }

    return 0L;
}

//------------------------------------------------------------------------

void SvxHatchTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    if( pWindow == m_pCtlAngle )
    {
        switch( eRcPt )
        {
            case RP_LT: m_pMtrAngle->SetValue( 135 ); break;
            case RP_MT: m_pMtrAngle->SetValue( 90 );  break;
            case RP_RT: m_pMtrAngle->SetValue( 45 );  break;
            case RP_LM: m_pMtrAngle->SetValue( 180 ); break;
            case RP_RM: m_pMtrAngle->SetValue( 0 );   break;
            case RP_LB: m_pMtrAngle->SetValue( 225 ); break;
            case RP_MB: m_pMtrAngle->SetValue( 270 ); break;
            case RP_RB: m_pMtrAngle->SetValue( 315 ); break;
            case RP_MM: break;
        }
        ModifiedHdl_Impl( this );
    }
}


void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SvxTabPage::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

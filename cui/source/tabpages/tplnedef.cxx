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

#include <cuires.hrc>
#include "helpid.hrc"

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>

#include "svx/drawitem.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
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

    SfxTabPage( pParent
              , "LineStylePage"
              , "cui/ui/linestyletabpage.ui"
              , rInAttrs ),
    rOutAttrs       ( rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( XOUT_WIDTH ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( String(), COL_BLACK ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() )
{

   get(m_pLbLineStyles   ,"LB_LINESTYLES");
   get(m_pLbType1        ,"LB_TYPE_1");
   get(m_pLbType2        ,"LB_TYPE_2");
   get(m_pNumFldNumber1  ,"NUM_FLD_1");
   get(m_pNumFldNumber2  ,"NUM_FLD_2");
   get(m_pMtrLength1     ,"MTR_FLD_LENGTH_1");
   m_pMtrLength1->SetCustomUnitText("%");
   get(m_pMtrLength2     ,"MTR_FLD_LENGTH_2");
   m_pMtrLength2->SetCustomUnitText("%");
   get(m_pMtrDistance    ,"MTR_FLD_DISTANCE");
   m_pMtrDistance->SetCustomUnitText("%");
   get(m_pCbxSynchronize ,"CBX_SYNCHRONIZE");
   get(m_pBtnAdd         ,"BTN_ADD");
   get(m_pBtnModify      ,"BTN_MODIFY");
   get(m_pBtnDelete      ,"BTN_DELETE");
   get(m_pBtnLoad        ,"BTN_LOAD");
   get(m_pBtnSave        ,"BTN_SAVE");
   get(m_pCtlPreview     ,"CTL_PREVIEW");

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
    SetFieldUnit( *m_pMtrDistance, eFUnit );
    SetFieldUnit( *m_pMtrLength1, eFUnit );
    SetFieldUnit( *m_pMtrLength2, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXDash );
    rXLSet.Put( aXColor );

    // #i34740#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

    m_pBtnAdd->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickSaveHdl_Impl ) );

    m_pNumFldNumber1->SetModifyHdl( LINK( this, SvxLineDefTabPage, ChangeNumber1Hdl_Impl ) );
    m_pNumFldNumber2->SetModifyHdl( LINK( this, SvxLineDefTabPage, ChangeNumber2Hdl_Impl ) );
    m_pLbLineStyles->SetSelectHdl( LINK( this, SvxLineDefTabPage, SelectLinestyleHdl_Impl ) );

    // #i122042# switch off default adding of 'none' and 'solid' entries
    // for this ListBox; we want to select only editable/dashed styles
    m_pLbLineStyles->setAddStandardFields(false);

    // absolute (in mm) or relative (in %)
    m_pCbxSynchronize->SetClickHdl(  LINK( this, SvxLineDefTabPage, ChangeMetricHdl_Impl ) );

    // preview must be updated when there's something changed
    Link aLink = LINK( this, SvxLineDefTabPage, SelectTypeHdl_Impl );
    m_pLbType1->SetSelectHdl( aLink );
    m_pLbType2->SetSelectHdl( aLink );
    aLink = LINK( this, SvxLineDefTabPage, ChangePreviewHdl_Impl );
    m_pMtrLength1->SetModifyHdl( aLink );
    m_pMtrLength2->SetModifyHdl( aLink );
    m_pMtrDistance->SetModifyHdl( aLink );

    pDashList = NULL;
}


// -----------------------------------------------------------------------

void SvxLineDefTabPage::Construct()
{
    // Line style fill; do *not* add default fields here
    m_pLbLineStyles->Fill( pDashList );
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
                m_pLbLineStyles->SelectEntryPos( *pPosDashLb );
            }
            // so that a possibly existing line style is discarded
            SelectLinestyleHdl_Impl( this );

            // determining (and possibly cutting) the name
            // and displaying it in the GroupBox
//             OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
//             aString         += ": ";
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

    if( m_pNumFldNumber1->GetText()     != m_pNumFldNumber1->GetSavedValue() ||
        m_pMtrLength1->GetText()        != m_pMtrLength1->GetSavedValue() ||
        m_pLbType1->GetSelectEntryPos() != m_pLbType1->GetSavedValue() ||
        m_pNumFldNumber2->GetText()     != m_pNumFldNumber2->GetSavedValue() ||
        m_pMtrLength2->GetText()        != m_pMtrLength2->GetSavedValue() ||
        m_pLbType2->GetSelectEntryPos() != m_pLbType2->GetSavedValue() ||
        m_pMtrDistance->GetText()       != m_pMtrDistance->GetSavedValue() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        SvxMessDialog* aMessDlg = new SvxMessDialog(GetParentDialog(),
                                                    SVX_RESSTR( RID_SVXSTR_LINESTYLE ),
                                                    String( ResId( RID_SVXSTR_ASK_CHANGE_LINESTYLE, rMgr ) ),
                                                    &aWarningBoxImage );
        DBG_ASSERT(aMessDlg, "Dialogdiet fail!");
        aMessDlg->SetButtonText( MESS_BTN_1, String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
        aMessDlg->SetButtonText( MESS_BTN_2, String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

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



    sal_uInt16 nPos = m_pLbLineStyles->GetSelectEntryPos();
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

            String aString( m_pLbLineStyles->GetSelectEntry() );
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
                m_pLbLineStyles->SelectEntryPos( 0 );
                break;

            case XLINE_DASH:
            {
                const XLineDashItem& rDashItem = ( const XLineDashItem& ) rAttrs.Get( XATTR_LINEDASH );
                aDash = rDashItem.GetDashValue();

                m_pLbLineStyles->SetNoSelection();
                m_pLbLineStyles->SelectEntry( rDashItem.GetName() );
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
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineDefTabPage::Create( Window* pWindow, const SfxItemSet& rOutAttrs )
{
    return( new SvxLineDefTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectLinestyleHdl_Impl, void *, p )
{
    if(pDashList->Count())
    {
        int nTmp = m_pLbLineStyles->GetSelectEntryPos();

        if(LISTBOX_ENTRY_NOTFOUND == nTmp)
        {
            OSL_ENSURE(false, "OOps, non-existent LineDash selected (!)");
            nTmp = 1;
        }

        aDash = pDashList->GetDash( nTmp )->GetDash();

        FillDialog_Impl();

        rXLSet.Put( XLineDashItem( String(), aDash ) );

        // #i34740#
        m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

        m_pCtlPreview->Invalidate();

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
    m_pCtlPreview->Invalidate();

    return( 0L );
}
IMPL_LINK_NOARG_INLINE_END(SvxLineDefTabPage, ChangePreviewHdl_Impl)

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber1Hdl_Impl)
{
    if( m_pNumFldNumber1->GetValue() == 0L )
    {
        m_pNumFldNumber2->SetMin( 1L );
        m_pNumFldNumber2->SetFirst( 1L );
    }
    else
    {
        m_pNumFldNumber2->SetMin( 0L );
        m_pNumFldNumber2->SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ChangeNumber2Hdl_Impl)
{
    if( m_pNumFldNumber2->GetValue() == 0L )
    {
        m_pNumFldNumber1->SetMin( 1L );
        m_pNumFldNumber1->SetFirst( 1L );
    }
    else
    {
        m_pNumFldNumber1->SetMin( 0L );
        m_pNumFldNumber1->SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}


//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, ChangeMetricHdl_Impl, void *, p )
{
    if( !m_pCbxSynchronize->IsChecked() && m_pMtrLength1->GetUnit() != eFUnit )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit ) * XOUT_WIDTH / 100;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit );
        }
        m_pMtrLength1->SetDecimalDigits( 2 );
        m_pMtrLength2->SetDecimalDigits( 2 );
        m_pMtrDistance->SetDecimalDigits( 2 );

        // adjust metric
        m_pMtrLength1->SetUnit( eFUnit );
        m_pMtrLength2->SetUnit( eFUnit );
        m_pMtrDistance->SetUnit( eFUnit );

        SetMetricValue( *m_pMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( *m_pMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( *m_pMtrDistance, nTmp3, ePoolUnit );
    }
    else if( m_pCbxSynchronize->IsChecked() && m_pMtrLength1->GetUnit() != FUNIT_CUSTOM )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit ) * 100 / XOUT_WIDTH;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit );
        }

        m_pMtrLength1->SetDecimalDigits( 0 );
        m_pMtrLength2->SetDecimalDigits( 0 );
        m_pMtrDistance->SetDecimalDigits( 0 );

        m_pMtrLength1->SetUnit( FUNIT_CUSTOM );
        m_pMtrLength2->SetUnit( FUNIT_CUSTOM );
        m_pMtrDistance->SetUnit( FUNIT_CUSTOM );


        SetMetricValue( *m_pMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( *m_pMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( *m_pMtrDistance, nTmp3, ePoolUnit );

    }
    SelectTypeHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineDefTabPage, SelectTypeHdl_Impl, void *, p )
{
    if ( p == m_pLbType1 || !p )
    {
        if ( m_pLbType1->GetSelectEntryPos() == 0 )
        {
            m_pMtrLength1->Disable();
            m_pMtrLength1->SetText( "" );
        }
        else if ( !m_pMtrLength1->IsEnabled() )
        {
            m_pMtrLength1->Enable();
            m_pMtrLength1->Reformat();
        }
    }

    if ( p == m_pLbType2 || !p )
    {
        if ( m_pLbType2->GetSelectEntryPos() == 0 )
        {
            m_pMtrLength2->Disable();
            m_pMtrLength2->SetText( "" );
        }
        else if ( !m_pMtrLength2->IsEnabled() )
        {
            m_pMtrLength2->Enable();
            m_pMtrLength2->Reformat();
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
        aName += OUString::number( j++ );
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
            const Bitmap aBitmap = pDashList->GetUiBitmap( nDashCount );
            m_pLbLineStyles->Append( *pEntry, pDashList->GetUiBitmap( nDashCount ) );

            m_pLbLineStyles->SelectEntryPos( m_pLbLineStyles->GetEntryCount() - 1 );

            *pnDashListState |= CT_MODIFIED;

            *pPageType = 2;

            // save values for changes recognition (-> method)
            m_pNumFldNumber1->SaveValue();
            m_pMtrLength1->SaveValue();
            m_pLbType1->SaveValue();
            m_pNumFldNumber2->SaveValue();
            m_pMtrLength2->SaveValue();
            m_pLbType2->SaveValue();
            m_pMtrDistance->SaveValue();
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

    // determine button state
    if ( pDashList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbLineStyles->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
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
                m_pLbLineStyles->Modify( *pEntry, nPos, pDashList->GetUiBitmap( nPos ) );

                m_pLbLineStyles->SelectEntryPos( nPos );

                *pnDashListState |= CT_MODIFIED;

                *pPageType = 2;

                // save values for changes recognition (-> method)
                m_pNumFldNumber1->SaveValue();
                m_pMtrLength1->SaveValue();
                m_pLbType1->SaveValue();
                m_pNumFldNumber2->SaveValue();
                m_pMtrLength2->SaveValue();
                m_pLbType2->SaveValue();
                m_pMtrDistance->SaveValue();
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
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = m_pLbLineStyles->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        MessageDialog aQueryBox( GetParentDialog()
                                ,"AskDelLineStyleDialog"
                                ,"cui/ui/querydeletelinestyledialog.ui");

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pDashList->Remove( nPos );
            m_pLbLineStyles->RemoveEntry( nPos );
            m_pLbLineStyles->SelectEntryPos( 0 );

            SelectLinestyleHdl_Impl( this );
            *pPageType = 0; // style should not be taken

            *pnDashListState |= CT_MODIFIED;

            ChangePreviewHdl_Impl( this );
        }
    }

    // determine button state
    if ( !pDashList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
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
        nReturn = MessageDialog( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            pDashList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        String aStrFilterType( "*.sod" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XDashListRef pDshLst = XPropertyList::CreatePropertyList( XDASH_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) )->AsDashList();
            pDshLst->SetName( aURL.getName() );

            if( pDshLst->Load() )
            {
                pDashList = pDshLst;
                ( (SvxLineTabDialog*) GetParentDialog() )->SetNewDashList( pDashList );

                m_pLbLineStyles->Clear();
                m_pLbLineStyles->Fill( pDashList );
                Reset( rOutAttrs );

                pDashList->SetName( aURL.getName() );

                *pnDashListState |= CT_CHANGED;
                *pnDashListState &= ~CT_MODIFIED;
            }
            else
                //aIStream.Close();
                MessageDialog( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui").Execute();
        }
    }

    // determine button state
    if ( pDashList->Count() )
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
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineDefTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( "*.sod" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pDashList->GetName().Len() )
    {
        aFile.Append( pDashList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( OUString("sod") );
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
            ErrorBox( GetParentDialog(), WinBits( WB_OK ), OUString( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDash_Impl()
{
    XDashStyle eXDS;

    if( m_pCbxSynchronize->IsChecked() )
        eXDS = XDASH_RECTRELATIVE;
    else
        eXDS = XDASH_RECT;

    aDash.SetDashStyle( eXDS );
    aDash.SetDots( (sal_uInt8) m_pNumFldNumber1->GetValue() );
    aDash.SetDotLen( m_pLbType1->GetSelectEntryPos() == 0 ? 0 : GetCoreValue( *m_pMtrLength1, ePoolUnit ) );
    aDash.SetDashes( (sal_uInt8) m_pNumFldNumber2->GetValue() );
    aDash.SetDashLen( m_pLbType2->GetSelectEntryPos() == 0 ? 0 : GetCoreValue( *m_pMtrLength2, ePoolUnit ) );
    aDash.SetDistance( GetCoreValue( *m_pMtrDistance, ePoolUnit ) );

    rXLSet.Put( XLineDashItem( String(), aDash ) );

    // #i34740#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());
}

//------------------------------------------------------------------------

void SvxLineDefTabPage::FillDialog_Impl()
{
    XDashStyle eXDS = aDash.GetDashStyle(); // XDASH_RECT, XDASH_ROUND
    if( eXDS == XDASH_RECTRELATIVE )
        m_pCbxSynchronize->Check();
    else
        m_pCbxSynchronize->Check( sal_False );

    m_pNumFldNumber1->SetValue( aDash.GetDots() );
    SetMetricValue( *m_pMtrLength1, aDash.GetDotLen(), ePoolUnit );
    m_pLbType1->SelectEntryPos( aDash.GetDotLen() == 0 ? 0 : 1 );
    m_pNumFldNumber2->SetValue( aDash.GetDashes() );
    SetMetricValue( *m_pMtrLength2, aDash.GetDashLen(), ePoolUnit );
    m_pLbType2->SelectEntryPos( aDash.GetDashLen() == 0 ? 0 : 1 );
    SetMetricValue( *m_pMtrDistance, aDash.GetDistance(), ePoolUnit );

    ChangeMetricHdl_Impl( NULL );

    // save values for changes recognition (-> method)
    m_pNumFldNumber1->SaveValue();
    m_pMtrLength1->SaveValue();
    m_pLbType1->SaveValue();
    m_pNumFldNumber2->SaveValue();
    m_pMtrLength2->SaveValue();
    m_pLbType2->SaveValue();
    m_pMtrDistance->SaveValue();
}


void SvxLineDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        sal_uInt16 nOldSelect = m_pLbLineStyles->GetSelectEntryPos();
        m_pLbLineStyles->Clear();
        m_pLbLineStyles->Fill( pDashList );
        m_pLbLineStyles->SelectEntryPos( nOldSelect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

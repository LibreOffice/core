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

#include <vcl/wrkwin.hxx>
#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "paragrph.hrc"

SvxGradientTabPage::SvxGradientTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SfxTabPage          ( pParent, "GradientPage", "cui/ui/gradientpage.ui", rInAttrs ),

    rOutAttrs           ( rInAttrs ),

    pnGradientListState ( 0 ),
    pnColorListState    ( 0 ),
    pPageType           ( 0 ),
    pDlgType            ( 0 ),
    pPos                ( 0 ),
    pbAreaTP            ( 0 ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_GRADIENT ),
    aXGradientItem      ( String(), XGradient( COL_BLACK, COL_WHITE ) ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    get(m_pLbGradientType, "gradienttypelb");
    get(m_pFtCenterX,      "centerxft");
    get(m_pMtrCenterX,     "centerxmtr");
    get(m_pFtCenterY,      "centeryft");
    get(m_pMtrCenterY,     "centerymtr");
    get(m_pFtAngle,        "angleft");
    get(m_pMtrAngle,       "anglemtr");
    get(m_pMtrBorder,      "bordermtr");
    get(m_pLbColorFrom,    "colorfromlb");
    get(m_pMtrColorFrom,   "colorfrommtr");
    get(m_pLbColorTo,      "colortolb");
    get(m_pMtrColorTo,     "colortomtr");
    get(m_pLbGradients,    "gradientslb");
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbGradients->set_width_request(aSize.Width());
    m_pLbGradients->set_height_request(aSize.Height());
    get(m_pCtlPreview,     "previewctl");
    aSize = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd,         "add");
    get(m_pBtnModify,      "modify");
    get(m_pBtnDelete,      "delete");
    get(m_pBtnLoad,        "load");
    get(m_pBtnSave,        "save");

    m_pLbGradients->SetAccessibleName(GetText());


    // this page needs ExchangeSupport
    SetExchangeSupport();

    // as long as NOT supported by the item

    m_pMtrColorTo->SetValue( 100 );
    m_pMtrColorFrom->SetValue( 100 );

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXGradientItem );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    // overload the handler
    m_pLbGradients->SetSelectHdl(
        LINK( this, SvxGradientTabPage, ChangeGradientHdl_Impl ) );
    m_pBtnAdd->SetClickHdl( LINK( this, SvxGradientTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickDeleteHdl_Impl ) );

    Link aLink = LINK( this, SvxGradientTabPage, ModifiedHdl_Impl );
    m_pLbGradientType->SetSelectHdl( aLink );
    m_pMtrCenterX->SetModifyHdl( aLink );
    m_pMtrCenterY->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pMtrBorder->SetModifyHdl( aLink );
    m_pMtrColorFrom->SetModifyHdl( aLink );
    m_pLbColorFrom->SetSelectHdl( aLink );
    m_pMtrColorTo->SetModifyHdl( aLink );
    m_pLbColorTo->SetSelectHdl( aLink );

    m_pBtnLoad->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickSaveHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlPreview->EnableRTL( sal_False );

    setPreviewsToSamePlace(pParent, this);
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::Construct()
{
    m_pLbColorFrom->Fill( pColorList );
    m_pLbColorTo->CopyEntries( *m_pLbColorFrom );

    m_pLbGradients->Fill( pGradientList );
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::ActivatePage( const SfxItemSet&  )
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

                // LbColorFrom
                nPos = m_pLbColorFrom->GetSelectEntryPos();
                m_pLbColorFrom->Clear();
                m_pLbColorFrom->Fill( pColorList );
                nCount = m_pLbColorFrom->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbColorFrom->SelectEntryPos( 0 );
                else
                    m_pLbColorFrom->SelectEntryPos( nPos );

                // LbColorTo
                nPos = m_pLbColorTo->GetSelectEntryPos();
                m_pLbColorTo->Clear();
                m_pLbColorTo->CopyEntries( *m_pLbColorFrom );
                nCount = m_pLbColorTo->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbColorTo->SelectEntryPos( 0 );
                else
                    m_pLbColorTo->SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // determining (and possibly cutting) the name and
            // displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( pGradientList->GetPath() );

            aURL.Append( pGradientList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            if ( *pPageType == PT_GRADIENT && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbGradients->SelectEntryPos( *pPos );
            }
            // colors could have been deleted
            ChangeGradientHdl_Impl( this );

            *pPageType = PT_GRADIENT;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxGradientTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

long SvxGradientTabPage::CheckChanges_Impl()
{
    // is used here in order to NOT lose changes
    XGradient aTmpGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          (XGradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue() );

    sal_uInt16 nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XGradient aGradient = pGradientList->GetGradient( nPos )->GetGradient();

        if( !( aTmpGradient == aGradient ) )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxMessDialog* aMessDlg = new SvxMessDialog(GetParentDialog(),
                                                        SVX_RESSTR( RID_SVXSTR_GRADIENT ),
                                                        CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_GRADIENT ),
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
                    aGradient = pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( this );
                    nPos = m_pLbGradients->GetSelectEntryPos();
                    aGradient = pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_CANCEL:
                break;
            }
            delete aMessDlg;
        }
    }
    nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPos = nPos;
    }
    return 0L;
}

// -----------------------------------------------------------------------

sal_Bool SvxGradientTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( *pDlgType == 0 && *pPageType == PT_GRADIENT && *pbAreaTP == sal_False )
    {
        // CheckChanges(); <-- duplicate inquiry ?

        XGradient*  pXGradient = NULL;
        String      aString;
        sal_uInt16      nPos = m_pLbGradients->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            pXGradient = new XGradient( pGradientList->GetGradient( nPos )->GetGradient() );
            aString = m_pLbGradients->GetSelectEntry();

        }
        else
        // gradient was passed (unidentified)
        {
            pXGradient = new XGradient( m_pLbColorFrom->GetSelectEntryColor(),
                        m_pLbColorTo->GetSelectEntryColor(),
                        (XGradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                        (sal_uInt16) m_pMtrCenterX->GetValue(),
                        (sal_uInt16) m_pMtrCenterY->GetValue(),
                        (sal_uInt16) m_pMtrBorder->GetValue(),
                        (sal_uInt16) m_pMtrColorFrom->GetValue(),
                        (sal_uInt16) m_pMtrColorTo->GetValue() );
        }
        DBG_ASSERT( pXGradient, "XGradient konnte nicht erzeugt werden" );
        rSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rSet.Put( XFillGradientItem( aString, *pXGradient ) );

        delete pXGradient;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SvxGradientTabPage::Reset( const SfxItemSet& )
{
    // m_pLbGradients->SelectEntryPos( 0 );
    ChangeGradientHdl_Impl( this );

    // determine state of the buttons
    if( pGradientList->Count() )
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

SfxTabPage* SvxGradientTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return new SvxGradientTabPage( pWindow, rOutAttrs );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGradientTabPage, ModifiedHdl_Impl, void *, pControl )
{
    XGradientStyle eXGS = (XGradientStyle) m_pLbGradientType->GetSelectEntryPos();

    XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          eXGS,
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue() );

    // enable/disable controls
    if( pControl == m_pLbGradientType || pControl == this )
        SetControlState_Impl( eXGS );

    // displaying in XOutDev
    rXFSet.Put( XFillGradientItem( String(), aXGradient ) );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGradientTabPage, ClickAddHdl_Impl)
{
    String aNewName( SVX_RES( RID_SVXSTR_GRADIENT ) );
    String aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
    OUString aName;

    long nCount = pGradientList->Count();
    long j = 1;
    sal_Bool bDifferent = sal_False;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += " ";
        aName += OUString::number( j++ );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pGradientList->GetGradient( i )->GetName() )
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
            if( aName == pGradientList->GetGradient( i )->GetName() )
                bDifferent = sal_False;

        if( bDifferent )
        {
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
        XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                              m_pLbColorTo->GetSelectEntryColor(),
                              (XGradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                              static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                              (sal_uInt16) m_pMtrCenterX->GetValue(),
                              (sal_uInt16) m_pMtrCenterY->GetValue(),
                              (sal_uInt16) m_pMtrBorder->GetValue(),
                              (sal_uInt16) m_pMtrColorFrom->GetValue(),
                              (sal_uInt16) m_pMtrColorTo->GetValue() );
        XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

        pGradientList->Insert( pEntry, nCount );

        m_pLbGradients->Append( *pEntry, pGradientList->GetUiBitmap( nCount ) );

        m_pLbGradients->SelectEntryPos( m_pLbGradients->GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( m_pLbGradients->GetPosPixel(), m_pLbGradients->GetSizePixel() );
        if( sal_True ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
        }
#endif

        *pnGradientListState |= CT_MODIFIED;

        ChangeGradientHdl_Impl( this );
    }

    // determine button state
    if( pGradientList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGradientTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbGradients->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
        OUString aName( pGradientList->GetGradient( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");

        long nCount = pGradientList->Count();
        sal_Bool bDifferent = sal_False;
        sal_Bool bLoop = sal_True;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pGradientList->GetGradient( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = sal_False;
            }

            if( bDifferent )
            {
                bLoop = sal_False;
                XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                                      m_pLbColorTo->GetSelectEntryColor(),
                                      (XGradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                                      static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                                      (sal_uInt16) m_pMtrCenterX->GetValue(),
                                      (sal_uInt16) m_pMtrCenterY->GetValue(),
                                      (sal_uInt16) m_pMtrBorder->GetValue(),
                                      (sal_uInt16) m_pMtrColorFrom->GetValue(),
                                      (sal_uInt16) m_pMtrColorTo->GetValue() );

                XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

                delete pGradientList->Replace( pEntry, nPos );

                m_pLbGradients->Modify( *pEntry, nPos, pGradientList->GetUiBitmap( nPos ) );

                m_pLbGradients->SelectEntryPos( nPos );

                *pnGradientListState |= CT_MODIFIED;
            }
            else
            {
                MessageDialog aBox( GetParentDialog()
                                    ,"DuplicateNameDialog"
                                    ,"cui/ui/queryduplicatedialog.ui");
                aBox.Execute();
            }

        }
        delete pDlg;
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGradientTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_GRADIENT ) ) );

        if ( aQueryBox.Execute() == RET_YES )
        {
            delete pGradientList->Remove( nPos );
            m_pLbGradients->RemoveEntry( nPos );
            m_pLbGradients->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();

            ChangeGradientHdl_Impl( this );

            *pnGradientListState |= CT_MODIFIED;
        }
    }
    // determine button state
    if( !pGradientList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGradientTabPage, ClickLoadHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *pnGradientListState & CT_MODIFIED )
    {
        nReturn = MessageDialog( GetParentDialog()
                                 ,"AskSaveList"
                                 ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            pGradientList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        String aStrFilterType( "*.sog" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            EnterWait();

            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // save list
            XGradientListRef pGrdList = XPropertyList::CreatePropertyList(
                XGRADIENT_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) )->AsGradientList();
            pGrdList->SetName( aURL.getName() );

            if ( pGrdList->Load() )
            {
                pGradientList = pGrdList;
                ( (SvxAreaTabDialog*) GetParentDialog() )->
                    SetNewGradientList( pGradientList );

                m_pLbGradients->Clear();
                m_pLbGradients->Fill( pGradientList );
                Reset( rOutAttrs );

                pGradientList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
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

                *pnGradientListState |= CT_CHANGED;
                *pnGradientListState &= ~CT_MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                MessageDialog( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui").Execute();
            }
        }
    }

    // determine button state
    if( pGradientList->Count() )
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

IMPL_LINK_NOARG(SvxGradientTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( "*.sog" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pGradientList->GetName().Len() )
    {
        aFile.Append( pGradientList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( OUString("sog") );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pGradientList->SetName( aURL.getName() );
        pGradientList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pGradientList->Save() )
        {
            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString  += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            *pnGradientListState |= CT_SAVED;
            *pnGradientListState &= ~CT_MODIFIED;
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

IMPL_LINK_NOARG(SvxGradientTabPage, ChangeGradientHdl_Impl)
{
    XGradient* pGradient = NULL;
    int nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pGradient = new XGradient( ( (XGradientEntry*) pGradientList->GetGradient( nPos ) )->GetGradient() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), sal_True, &pPoolItem ) )
        {
            if( ( XFILL_GRADIENT == (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue() ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), sal_True, &pPoolItem ) ) )
            {
                pGradient = new XGradient( ( ( const XFillGradientItem* ) pPoolItem )->GetGradientValue() );
            }
        }
        if( !pGradient )
        {
            m_pLbGradients->SelectEntryPos( 0 );
            nPos = m_pLbGradients->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pGradient = new XGradient( ( (XGradientEntry*) pGradientList->GetGradient( nPos ) )->GetGradient() );
        }
    }

    if( pGradient )
    {
        XGradientStyle eXGS = pGradient->GetGradientStyle();

        m_pLbGradientType->SelectEntryPos(
            sal::static_int_cast< sal_uInt16 >( eXGS ) );
        // if the entry is not in the listbox ist,
        // colors are added temporarily
        m_pLbColorFrom->SetNoSelection();
        m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );

        if ( m_pLbColorFrom->GetSelectEntryCount() == 0 )
        {
            m_pLbColorFrom->InsertEntry( pGradient->GetStartColor(),
                                      String() );
            m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );
        }
        m_pLbColorTo->SetNoSelection();
        m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );

        if ( m_pLbColorTo->GetSelectEntryCount() == 0 )
        {
            m_pLbColorTo->InsertEntry( pGradient->GetEndColor(), String() );
            m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );
        }

        m_pMtrAngle->SetValue( pGradient->GetAngle() / 10 ); // should be changed in resource
        m_pMtrBorder->SetValue( pGradient->GetBorder() );
        m_pMtrCenterX->SetValue( pGradient->GetXOffset() );
        m_pMtrCenterY->SetValue( pGradient->GetYOffset() );
        m_pMtrColorFrom->SetValue( pGradient->GetStartIntens() );
        m_pMtrColorTo->SetValue( pGradient->GetEndIntens() );

        // disable/enable controls
        SetControlState_Impl( eXGS );

        // fill ItemSet and pass it on to aCtlPreview
        rXFSet.Put( XFillGradientItem( String(), *pGradient ) );
        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
        delete pGradient;
    }
    return 0L;
}

//------------------------------------------------------------------------

void SvxGradientTabPage::SetControlState_Impl( XGradientStyle eXGS )
{
    switch( eXGS )
    {
        case XGRAD_LINEAR:
        case XGRAD_AXIAL:
            m_pFtCenterX->Disable();
            m_pMtrCenterX->Disable();
            m_pFtCenterY->Disable();
            m_pMtrCenterY->Disable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case XGRAD_RADIAL:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Disable();
            m_pMtrAngle->Disable();
            break;

        case XGRAD_ELLIPTICAL:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case XGRAD_SQUARE:
        case XGRAD_RECT:
            m_pFtCenterX->Enable();
            m_pMtrCenterX->Enable();
            m_pFtCenterY->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

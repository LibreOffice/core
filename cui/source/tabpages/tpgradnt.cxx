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

#define DEFAULT_GRADIENTSTEP 64

using namespace com::sun::star;

SvxGradientTabPage::SvxGradientTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SfxTabPage          ( pParent, "GradientPage", "cui/ui/gradientpage.ui", &rInAttrs ),

    m_rOutAttrs           ( rInAttrs ),

    m_pnGradientListState ( nullptr ),
    m_pnColorListState    ( nullptr ),
    m_pPageType           ( nullptr ),
    m_nDlgType            ( 0 ),
    m_pPos                ( nullptr ),
    m_pbAreaTP            ( nullptr ),

    m_aXFStyleItem        ( drawing::FillStyle_GRADIENT ),
    m_aXGradientItem      ( OUString(), XGradient( COL_BLACK, COL_WHITE ) ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pMtrIncrement,   "incrementmtr");
    get(m_pSliderIncrement,"incrementslider");
    get(m_pLbGradientType, "gradienttypelb");
    get(m_pFtCenter,       "centerft");
    get(m_pMtrCenterX,     "centerxmtr");
    get(m_pMtrCenterY,     "centerymtr");
    get(m_pFtAngle,        "angleft");
    get(m_pMtrAngle,       "anglemtr");
    get(m_pMtrBorder,      "bordermtr");
    get(m_pSliderBorder,   "borderslider");
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

    m_pSliderIncrement->SetRange(Range(3,256));
    m_pMtrColorTo->SetValue( 100 );
    m_pMtrColorFrom->SetValue( 100 );
    m_pSliderBorder->SetRange(Range(0,100));

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXGradientItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    // set handler
    m_pLbGradients->SetSelectHdl(
        LINK( this, SvxGradientTabPage, ChangeGradientHdl_Impl ) );
    m_pBtnAdd->SetClickHdl( LINK( this, SvxGradientTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickDeleteHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxGradientTabPage, ModifiedEditHdl_Impl );
    Link<ListBox&,void> aLink2 = LINK( this, SvxGradientTabPage, ModifiedListBoxHdl_Impl );
    m_pLbGradientType->SetSelectHdl( aLink2 );
    m_pMtrIncrement->SetModifyHdl( aLink );
    m_pSliderIncrement->SetSlideHdl( LINK( this, SvxGradientTabPage, ModifiedSliderHdl_Impl )  );
    m_pMtrCenterX->SetModifyHdl( aLink );
    m_pMtrCenterY->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pMtrBorder->SetModifyHdl( aLink );
    m_pSliderBorder->SetSlideHdl( LINK( this, SvxGradientTabPage, ModifiedSliderHdl_Impl ) );
    m_pMtrColorFrom->SetModifyHdl( aLink );
    m_pLbColorFrom->SetSelectHdl( aLink2 );
    m_pMtrColorTo->SetModifyHdl( aLink );
    m_pLbColorTo->SetSelectHdl( aLink2 );

    m_pBtnLoad->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickSaveHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlPreview->EnableRTL( false );

    setPreviewsToSamePlace(pParent, this);
}

SvxGradientTabPage::~SvxGradientTabPage()
{
    disposeOnce();
}

void SvxGradientTabPage::dispose()
{
    m_pMtrIncrement.clear();
    m_pSliderIncrement.clear();
    m_pLbGradientType.clear();
    m_pFtCenter.clear();
    m_pMtrCenterX.clear();
    m_pMtrCenterY.clear();
    m_pFtAngle.clear();
    m_pMtrAngle.clear();
    m_pMtrBorder.clear();
    m_pSliderBorder.clear();
    m_pLbColorFrom.clear();
    m_pMtrColorFrom.clear();
    m_pLbColorTo.clear();
    m_pMtrColorTo.clear();
    m_pLbGradients.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    SfxTabPage::dispose();
}


void SvxGradientTabPage::Construct()
{
    m_pLbColorFrom->Fill( m_pColorList );
    m_pLbColorTo->CopyEntries( *m_pLbColorFrom );

    m_pLbGradients->Fill( m_pGradientList );
}


void SvxGradientTabPage::ActivatePage( const SfxItemSet&  )
{
    sal_Int32 nPos;
    sal_Int32 nCount;

    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = false;

        if( m_pColorList.is() )
        {
            // ColorList
            if( *m_pnColorListState & ChangeType::CHANGED ||
                *m_pnColorListState & ChangeType::MODIFIED )
            {
                if( *m_pnColorListState & ChangeType::CHANGED )
                    m_pColorList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewColorList();

                // LbColorFrom
                nPos = m_pLbColorFrom->GetSelectEntryPos();
                m_pLbColorFrom->Clear();
                m_pLbColorFrom->Fill( m_pColorList );
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
            INetURLObject   aURL( m_pGradientList->GetPath() );

            aURL.Append( m_pGradientList->GetName() );
            SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 ) + "...";
            }
            else
                aString += aURL.getBase();

            if ( *m_pPageType == PT_GRADIENT && *m_pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbGradients->SelectEntryPos( *m_pPos );
            }
            // colors could have been deleted
            ChangeGradientHdl_Impl( *m_pLbGradients );

            *m_pPageType = PT_GRADIENT;
            *m_pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}


DeactivateRC SvxGradientTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( CheckChanges_Impl() == -1L )
        return DeactivateRC::KeepPage;

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


long SvxGradientTabPage::CheckChanges_Impl()
{
    // is used here in order to NOT lose changes
    XGradient aTmpGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue() );

    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XGradient aGradient = m_pGradientList->GetGradient( nPos )->GetGradient();

        if( !( aTmpGradient == aGradient ) )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                          SVX_RESSTR( RID_SVXSTR_GRADIENT ),
                                                          CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_GRADIENT ),
                                                          &aWarningBoxImage );
            assert(aMessDlg && "Dialog creation failed!");
            aMessDlg->SetButtonText( SvxMessDialogButton::N1,
                                    OUString( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( SvxMessDialogButton::N2,
                                    OUString( ResId( RID_SVXSTR_ADD, rMgr ) ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( nullptr );
                    aGradient = m_pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( nullptr );
                    nPos = m_pLbGradients->GetSelectEntryPos();
                    aGradient = m_pGradientList->GetGradient( nPos )->GetGradient();
                }
                break;

                case RET_CANCEL:
                break;
            }
        }
    }
    nPos = m_pLbGradients->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *m_pPos = nPos;
    }
    return 0L;
}


bool SvxGradientTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( m_nDlgType == 0 && *m_pPageType == PT_GRADIENT && !*m_pbAreaTP )
    {
        // CheckChanges(); <-- duplicate inquiry ?

        std::unique_ptr<XGradient> pXGradient;
        OUString      aString;
        sal_Int32      nPos = m_pLbGradients->GetSelectEntryPos();
        sal_uInt16     nValue = m_pMtrIncrement->GetValue();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            pXGradient.reset(new XGradient( m_pGradientList->GetGradient( nPos )->GetGradient() ));
            aString = m_pLbGradients->GetSelectEntry();

        }
        else
        // gradient was passed (unidentified)
        {
            pXGradient.reset(new XGradient( m_pLbColorFrom->GetSelectEntryColor(),
                        m_pLbColorTo->GetSelectEntryColor(),
                        (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                        (sal_uInt16) m_pMtrCenterX->GetValue(),
                        (sal_uInt16) m_pMtrCenterY->GetValue(),
                        (sal_uInt16) m_pMtrBorder->GetValue(),
                        (sal_uInt16) m_pMtrColorFrom->GetValue(),
                        (sal_uInt16) m_pMtrColorTo->GetValue() ));
        }
        assert( pXGradient && "XGradient could not be created" );
        rSet->Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
        rSet->Put( XFillGradientItem( aString, *pXGradient ) );
        rSet->Put( XGradientStepCountItem( nValue ) );
    }
    return true;
}


void SvxGradientTabPage::Reset( const SfxItemSet* )
{
    // m_pLbGradients->SelectEntryPos( 0 );
    ChangeGradientHdl_Impl( *m_pLbGradients );

    // determine state of the buttons
    if( m_pGradientList->Count() )
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


VclPtr<SfxTabPage> SvxGradientTabPage::Create( vcl::Window* pWindow,
                                               const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxGradientTabPage>::Create( pWindow, *rOutAttrs );
}


IMPL_LINK_TYPED( SvxGradientTabPage, ModifiedListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
}
IMPL_LINK_TYPED( SvxGradientTabPage, ModifiedEditHdl_Impl, Edit&, rBox, void )
{
    ModifiedHdl_Impl(&rBox);
}
IMPL_LINK_TYPED( SvxGradientTabPage, ModifiedSliderHdl_Impl, Slider*, rSlider, void )
{
    ModifiedHdl_Impl(rSlider);
}
void SvxGradientTabPage::ModifiedHdl_Impl( void* pControl )
{
    if( pControl == m_pMtrBorder )
        m_pSliderBorder->SetThumbPos( m_pMtrBorder->GetValue() );
    if( pControl == m_pSliderBorder )
        m_pMtrBorder->SetValue( m_pSliderBorder->GetThumbPos() );
    if( pControl == m_pMtrIncrement )
        m_pSliderIncrement->SetThumbPos( m_pMtrIncrement->GetValue() );
    if(pControl == m_pSliderIncrement)
        m_pMtrIncrement->SetValue( m_pSliderIncrement->GetThumbPos() );

    css::awt::GradientStyle eXGS = (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos();

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

    sal_uInt16 nValue = (sal_uInt16)m_pMtrIncrement->GetValue();
    m_rXFSet.Put( XGradientStepCountItem( nValue ) );

    // displaying in XOutDev
    m_rXFSet.Put( XFillGradientItem( OUString(), aXGradient ) );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SVX_RES( RID_SVXSTR_GRADIENT ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
    OUString aName;

    long nCount = m_pGradientList->Count();
    long j = 1;
    bool bValidGradientName = false;

    while( !bValidGradientName )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bValidGradientName = (SearchGradientList(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16 nError   = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bValidGradientName = (SearchGradientList(aName) == LISTBOX_ENTRY_NOTFOUND);

        if (bValidGradientName)
        {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox.disposeAndReset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui"));
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    pDlg.reset();
    pWarnBox.disposeAndClear();

    if( !nError )
    {
        XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                              m_pLbColorTo->GetSelectEntryColor(),
                              (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                              static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                              (sal_uInt16) m_pMtrCenterX->GetValue(),
                              (sal_uInt16) m_pMtrCenterY->GetValue(),
                              (sal_uInt16) m_pMtrBorder->GetValue(),
                              (sal_uInt16) m_pMtrColorFrom->GetValue(),
                              (sal_uInt16) m_pMtrColorTo->GetValue() );
        XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

        m_pGradientList->Insert( pEntry, nCount );

        m_pLbGradients->Append( *pEntry, m_pGradientList->GetUiBitmap( nCount ) );

        m_pLbGradients->SelectEntryPos( m_pLbGradients->GetEntryCount() - 1 );

#ifdef _WIN32
        // hack: #31355# W.P.
        Rectangle aRect( m_pLbGradients->GetPosPixel(), m_pLbGradients->GetSizePixel() );
        if( sal_True ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
        }
#endif

        *m_pnGradientListState |= ChangeType::MODIFIED;

        ChangeGradientHdl_Impl( *m_pLbGradients );
    }

    // determine button state
    if( m_pGradientList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString aDesc( CUI_RES( RID_SVXSTR_DESC_GRADIENT ) );
        OUString aName( m_pGradientList->GetGradient( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        bool bLoop = true;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nGradientPos = SearchGradientList(aName);
            bool bValidGradientName = (nGradientPos == nPos) || (nGradientPos == LISTBOX_ENTRY_NOTFOUND);

            if( bValidGradientName )
            {
                bLoop = false;
                XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                                      m_pLbColorTo->GetSelectEntryColor(),
                                      (css::awt::GradientStyle) m_pLbGradientType->GetSelectEntryPos(),
                                      static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                                      (sal_uInt16) m_pMtrCenterX->GetValue(),
                                      (sal_uInt16) m_pMtrCenterY->GetValue(),
                                      (sal_uInt16) m_pMtrBorder->GetValue(),
                                      (sal_uInt16) m_pMtrColorFrom->GetValue(),
                                      (sal_uInt16) m_pMtrColorTo->GetValue() );

                XGradientEntry* pEntry = new XGradientEntry( aXGradient, aName );

                delete m_pGradientList->Replace( pEntry, nPos );

                m_pLbGradients->Modify( *pEntry, nPos, m_pGradientList->GetUiBitmap( nPos ) );

                m_pLbGradients->SelectEntryPos( nPos );

                *m_pnGradientListState |= ChangeType::MODIFIED;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox( GetParentDialog()
                                                          ,"DuplicateNameDialog"
                                                          ,"cui/ui/queryduplicatedialog.ui" );
                aBox->Execute();
            }

        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelGradientDialog","cui/ui/querydeletegradientdialog.ui");

        if ( aQueryBox->Execute() == RET_YES )
        {
            delete m_pGradientList->Remove( nPos );
            m_pLbGradients->RemoveEntry( nPos );
            m_pLbGradients->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();

            ChangeGradientHdl_Impl( *m_pLbGradients );

            *m_pnGradientListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pGradientList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ClickLoadHdl_Impl, Button*, void)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *m_pnGradientListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                 ,"AskSaveList"
                                 ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            m_pGradientList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        OUString aStrFilterType( "*.sog" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        OUString aPalettePath(SvtPathOptions().GetPalettePath());
        OUString aLastDir;
        sal_Int32 nIndex = 0;
        do
        {
            aLastDir = aPalettePath.getToken(0, ';', nIndex);
        }
        while (nIndex >= 0);

        INetURLObject aFile(aLastDir);
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            EnterWait();

            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // save list
            XGradientListRef pGrdList = XPropertyList::AsGradientList(
                XPropertyList::CreatePropertyList(
                    XGRADIENT_LIST,
                    aPathURL.GetMainURL(INetURLObject::NO_DECODE), ""));
            pGrdList->SetName( aURL.getName() );

            if ( pGrdList->Load() )
            {
                m_pGradientList = pGrdList;
                static_cast<SvxAreaTabDialog*>( GetParentDialog() )->
                    SetNewGradientList( m_pGradientList );

                m_pLbGradients->Clear();
                m_pLbGradients->Fill( m_pGradientList );
                Reset( &m_rOutAttrs );

                m_pGradientList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
                // and displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 ) + "...";
                }
                else
                    aString += aURL.getBase();

                *m_pnGradientListState |= ChangeType::CHANGED;
                *m_pnGradientListState &= ~ChangeType::MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
            }
        }
    }

    // determine button state
    if( m_pGradientList->Count() )
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


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ClickSaveHdl_Impl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );
    OUString aStrFilterType( "*.sog" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aFile(aLastDir);
    SAL_WARN_IF( aFile.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

    if( !m_pGradientList->GetName().isEmpty() )
    {
        aFile.Append( m_pGradientList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( "sog" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        m_pGradientList->SetName( aURL.getName() );
        m_pGradientList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( m_pGradientList->Save() )
        {
            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString  += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 ) + "...";
            }
            else
                aString += aURL.getBase();

            *m_pnGradientListState |= ChangeType::SAVED;
            *m_pnGradientListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxGradientTabPage, ChangeGradientHdl_Impl, ListBox&, void)
{
    std::unique_ptr<XGradient> pGradient;
    int nPos = m_pLbGradients->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pGradient.reset(new XGradient( m_pGradientList->GetGradient( nPos )->GetGradient() ));
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;
        if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), true, &pPoolItem ) )
        {
            if( ( drawing::FillStyle_GRADIENT == (drawing::FillStyle) static_cast<const XFillStyleItem*>( pPoolItem )->GetValue() ) &&
                ( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), true, &pPoolItem ) ) )
            {
                pGradient.reset(new XGradient( static_cast<const XFillGradientItem*>( pPoolItem )->GetGradientValue() ));
            }
        }
        if( !pGradient )
        {
            m_pLbGradients->SelectEntryPos( 0 );
            nPos = m_pLbGradients->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pGradient.reset(new XGradient( m_pGradientList->GetGradient( nPos )->GetGradient() ));
        }
    }

    if( pGradient )
    {
        css::awt::GradientStyle eXGS = pGradient->GetGradientStyle();
        sal_uInt16 nValue = static_cast<const XGradientStepCountItem&>( m_rOutAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
        if(nValue == 0)
            nValue = DEFAULT_GRADIENTSTEP;

        m_pMtrIncrement->SetValue( nValue );
        m_pSliderIncrement->SetThumbPos( nValue );
        m_pLbGradientType->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( eXGS ) );
        // if the entry is not in the listbox,
        // colors are added temporarily
        m_pLbColorFrom->SetNoSelection();
        m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );

        if ( m_pLbColorFrom->GetSelectEntryCount() == 0 )
        {
            m_pLbColorFrom->InsertEntry( pGradient->GetStartColor(),
                                      OUString() );
            m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );
        }
        m_pLbColorTo->SetNoSelection();
        m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );

        if ( m_pLbColorTo->GetSelectEntryCount() == 0 )
        {
            m_pLbColorTo->InsertEntry( pGradient->GetEndColor(), OUString() );
            m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );
        }

        m_pMtrAngle->SetValue( pGradient->GetAngle() / 10 ); // should be changed in resource
        m_pMtrBorder->SetValue( pGradient->GetBorder() );
        m_pSliderBorder->SetThumbPos( pGradient->GetBorder() );
        m_pMtrCenterX->SetValue( pGradient->GetXOffset() );
        m_pMtrCenterY->SetValue( pGradient->GetYOffset() );
        m_pMtrColorFrom->SetValue( pGradient->GetStartIntens() );
        m_pMtrColorTo->SetValue( pGradient->GetEndIntens() );

        // disable/enable controls
        SetControlState_Impl( eXGS );

        // fill ItemSet and pass it on to aCtlPreview
        m_rXFSet.Put( XFillGradientItem( OUString(), *pGradient ) );
        m_rXFSet.Put( XGradientStepCountItem( nValue ) );
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
    }
}


void SvxGradientTabPage::SetControlState_Impl( css::awt::GradientStyle eXGS )
{
    switch( eXGS )
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_pFtCenter->Disable();
            m_pMtrCenterX->Disable();
            m_pMtrCenterY->Disable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_RADIAL:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Disable();
            m_pMtrAngle->Disable();
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;
        default:
            break;
    }
}

sal_Int32 SvxGradientTabPage::SearchGradientList(const OUString& rGradientName)
{
    long nCount = m_pGradientList->Count();
    bool bValidGradientName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidGradientName;i++)
    {
        if(rGradientName == m_pGradientList->GetGradient( i )->GetName())
        {
            nPos = i;
            bValidGradientName = false;
        }
    }
    return nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <svx/dialogs.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/langitem.hxx>
#include <svx/langbox.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editeng.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdresid.hxx"

#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>

#include "helpids.h"
#include "Outliner.hxx"
#include "headerfooterdlg.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"

// preview control for presentation layout
#include <vcl/ctrl.hxx>
#include <tools/color.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <vcl/decoview.hxx>
#include <vcl/builderfactory.hxx>

#include "undoheaderfooter.hxx"
#include "sdundogr.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace sd
{

class PresLayoutPreview : public Control
{
private:
    SdPage* mpMaster;
    HeaderFooterSettings maSettings;
    Size maPageSize;
    Rectangle maOutRect;

private:
    void Paint(vcl::RenderContext& rRenderContext, SdrTextObj* pObj, bool bVisible, bool bDotted = false);

public:
    explicit PresLayoutPreview(vcl::Window* pParent);
    virtual ~PresLayoutPreview();

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual Size GetOptimalSize() const override;

    void init(SdPage* pMaster);
    void update(HeaderFooterSettings& rSettings);
};

}

// tab page for slide & header'n'notes

namespace sd
{

const int nDateTimeFormatsCount = 12;
int nDateTimeFormats[nDateTimeFormatsCount] =
{
    SVXDATEFORMAT_A,
    SVXDATEFORMAT_B,
    SVXDATEFORMAT_C,
    SVXDATEFORMAT_D,
    SVXDATEFORMAT_E,
    SVXDATEFORMAT_F,

    SVXDATEFORMAT_A | (SVXTIMEFORMAT_24_HM << 4),
    SVXDATEFORMAT_A | (SVXTIMEFORMAT_12_HM << 4),

    (SVXTIMEFORMAT_24_HM << 4),
    (SVXTIMEFORMAT_24_HMS <<4),

    (SVXTIMEFORMAT_12_HM << 4 ),
    (SVXTIMEFORMAT_12_HMS << 4 )
};

class HeaderFooterTabPage : public TabPage
{
private:

    VclPtr<FixedText>   mpFTIncludeOn;

    VclPtr<CheckBox>    mpCBHeader;
    VclPtr<VclContainer> mpHeaderBox;
    VclPtr<Edit>        mpTBHeader;

    VclPtr<CheckBox>    mpCBDateTime;
    VclPtr<RadioButton> mpRBDateTimeFixed;
    VclPtr<RadioButton> mpRBDateTimeAutomatic;
    VclPtr<Edit>        mpTBDateTimeFixed;
    VclPtr<ListBox>     mpCBDateTimeFormat;
    VclPtr<FixedText>   mpFTDateTimeLanguage;
    VclPtr<SvxLanguageBox>  mpCBDateTimeLanguage;

    VclPtr<CheckBox>    mpCBFooter;
    VclPtr<VclContainer> mpFooterBox;
    VclPtr<Edit>        mpTBFooter;

    VclPtr<CheckBox>   mpCBSlideNumber;

    VclPtr<CheckBox>    mpCBNotOnTitle;

    VclPtr<PresLayoutPreview>   mpCTPreview;

    SdDrawDocument*    mpDoc;
    LanguageType        meOldLanguage;

    bool            mbHandoutMode;

    DECL_LINK_TYPED( UpdateOnClickHdl, Button*, void );
    DECL_LINK_TYPED( LanguageChangeHdl, ListBox&, void );

    void FillFormatList(int eFormat);
    void GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet );
    void GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet, SdPage* pPage );

public:
    HeaderFooterTabPage( vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode );
    virtual ~HeaderFooterTabPage();
    virtual void dispose() override;

    void    init( const HeaderFooterSettings& rSettings, bool bNotOnTitle );
    void    getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle );
    void    update();
};

}

using namespace ::sd;

HeaderFooterDialog::HeaderFooterDialog( ViewShell* pViewShell, vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage ) :
        TabDialog ( pParent, "HeaderFooterDialog", "modules/simpress/ui/headerfooterdialog.ui" ),
        mpDoc( pDoc ),
        mpCurrentPage( pCurrentPage ),
        mpViewShell( pViewShell )
{
    get(mpTabCtrl, "tabs" );

    SdPage* pSlide;
    SdPage* pNotes;
    if( pCurrentPage->GetPageKind() == PK_STANDARD )
    {
        pSlide = pCurrentPage;
        pNotes = static_cast<SdPage*>(pDoc->GetPage( pCurrentPage->GetPageNum() + 1 ));
    }
    else if( pCurrentPage->GetPageKind() == PK_NOTES )
    {
        pNotes = pCurrentPage;
        pSlide = static_cast<SdPage*>(pDoc->GetPage( pCurrentPage->GetPageNum() -1 ));
        mpCurrentPage = pSlide;
    }
    else
    {
        // handout
        pSlide = pDoc->GetSdPage( 0, PK_STANDARD );
        pNotes = pDoc->GetSdPage( 0, PK_NOTES );
        mpCurrentPage = nullptr;
    }

    pDoc->StopWorkStartupDelay();
    mpTabCtrl->Show();

    mnSlidesId = mpTabCtrl->GetPageId("slides");
    mpSlideTabPage = VclPtr<HeaderFooterTabPage>::Create( mpTabCtrl, pDoc, pSlide, false );
    mpTabCtrl->SetTabPage( mnSlidesId, mpSlideTabPage );

    Size aSiz = mpSlideTabPage->GetSizePixel();
    Size aCtrlSiz = mpTabCtrl->GetOutputSizePixel();
    // set size on TabControl only if smaller than TabPage
    if ( aCtrlSiz.Width() < aSiz.Width() || aCtrlSiz.Height() < aSiz.Height() )
    {
        mpTabCtrl->SetOutputSizePixel( aSiz );
        aCtrlSiz = aSiz;
    }

    mnNotesId = mpTabCtrl->GetPageId("notes");
    mpNotesHandoutsTabPage = VclPtr<HeaderFooterTabPage>::Create( mpTabCtrl, pDoc, pNotes, true );
    mpTabCtrl->SetTabPage( mnNotesId, mpNotesHandoutsTabPage );

    get(maPBApplyToAll, "apply_all" );
    get(maPBApply, "apply" );
    get(maPBCancel, "cancel" );

    ActivatePageHdl( mpTabCtrl );

    mpTabCtrl->SetActivatePageHdl( LINK( this, HeaderFooterDialog, ActivatePageHdl ) );

    maPBApplyToAll->SetClickHdl( LINK( this, HeaderFooterDialog, ClickApplyToAllHdl ) );
    maPBApply->SetClickHdl( LINK( this, HeaderFooterDialog, ClickApplyHdl ) );
    maPBCancel->SetClickHdl( LINK( this, HeaderFooterDialog, ClickCancelHdl ) );

    maSlideSettings = pSlide->getHeaderFooterSettings();

    const HeaderFooterSettings& rTitleSettings = mpDoc->GetSdPage(0, PK_STANDARD)->getHeaderFooterSettings();
    bool bNotOnTitle = !rTitleSettings.mbFooterVisible && !rTitleSettings.mbSlideNumberVisible && !rTitleSettings.mbDateTimeVisible;

    mpSlideTabPage->init( maSlideSettings, bNotOnTitle );

    maNotesHandoutSettings = pNotes->getHeaderFooterSettings();
    mpNotesHandoutsTabPage->init( maNotesHandoutSettings, false );
}

HeaderFooterDialog::~HeaderFooterDialog()
{
    disposeOnce();
}

void HeaderFooterDialog::dispose()
{
    mpSlideTabPage.disposeAndClear();
    mpNotesHandoutsTabPage.disposeAndClear();
    mpTabCtrl.clear();
    maPBApplyToAll.clear();
    maPBApply.clear();
    maPBCancel.clear();
    TabDialog::dispose();
}

IMPL_LINK_TYPED( HeaderFooterDialog, ActivatePageHdl, TabControl *, pTabCtrl, void )
{
    const sal_uInt16 nId = pTabCtrl->GetCurPageId();
    TabPage* pTabPage = pTabCtrl->GetTabPage( nId );
    pTabPage->Show();
    maPBApply->Show( nId == mnSlidesId );
    maPBApply->Enable( mpCurrentPage != nullptr );
}

IMPL_LINK_NOARG_TYPED(HeaderFooterDialog, ClickApplyToAllHdl, Button*, void)
{
    ApplyToAll();
}

IMPL_LINK_NOARG_TYPED(HeaderFooterDialog, ClickApplyHdl, Button*, void)
{
    Apply();
}

IMPL_LINK_NOARG_TYPED(HeaderFooterDialog, ClickCancelHdl, Button*, void)
{
    Cancel();
}

short HeaderFooterDialog::Execute()
{
    sal_uInt16 nRet = TabDialog::Execute();
    if( nRet )
        mpViewShell->GetDocSh()->SetModified();
    return nRet;
}

void HeaderFooterDialog::ApplyToAll()
{
    sal_uInt16 tabId = mpTabCtrl->GetCurPageId();
    apply( true, tabId == mnSlidesId );
    EndDialog(1);
}

void HeaderFooterDialog::Apply()
{
    sal_uInt16 tabId = mpTabCtrl->GetCurPageId();
    apply( false, tabId == mnSlidesId );
    EndDialog(1);
}

void HeaderFooterDialog::Cancel()
{
    EndDialog();
}

void HeaderFooterDialog::apply( bool bToAll, bool bForceSlides )
{
    SdUndoGroup* pUndoGroup = new SdUndoGroup(mpDoc);
    OUString aComment( GetText() );
    pUndoGroup->SetComment( aComment );

    HeaderFooterSettings aNewSettings;
    bool bNewNotOnTitle;

    // change slide settings first ...

    mpSlideTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the slide tab page or if the slide settings
    // have been changed
    if( bForceSlides || !(aNewSettings == maSlideSettings) )
    {
        // apply to all slides
        if( bToAll )
        {
            int nPageCount = mpDoc->GetSdPageCount( PK_STANDARD );
            int nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                SdPage* pPage = mpDoc->GetSdPage( (sal_uInt16)nPage, PK_STANDARD );
                change( pUndoGroup, pPage, aNewSettings );
            }
        }
        else
        {
            // apply only to the current slide
            DBG_ASSERT( mpCurrentPage && mpCurrentPage->GetPageKind() == PK_STANDARD, "no current page to apply to!" );
            if( mpCurrentPage && (mpCurrentPage->GetPageKind() == PK_STANDARD) )
            {
                change( pUndoGroup, mpCurrentPage, aNewSettings );
            }
        }
    }

    // if we don't want to have header&footer on the first slide
    if( bNewNotOnTitle )
    {
        // just hide them, plain simple UI feature
        HeaderFooterSettings aTempSettings = mpDoc->GetSdPage( 0, PK_STANDARD )->getHeaderFooterSettings();

        aTempSettings.mbFooterVisible = false;
        aTempSettings.mbSlideNumberVisible = false;
        aTempSettings.mbDateTimeVisible = false;

        change( pUndoGroup, mpDoc->GetSdPage( 0, PK_STANDARD ), aTempSettings );
    }

    // now notes settings

    mpNotesHandoutsTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the notes tab page or if the notes settings
    // have been changed
    if( !bForceSlides || !(aNewSettings == maNotesHandoutSettings) )
    {
        // first set to all notes pages
        int nPageCount = mpDoc->GetSdPageCount( PK_NOTES );
        int nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            SdPage* pPage = mpDoc->GetSdPage( (sal_uInt16)nPage, PK_NOTES );

            change( pUndoGroup, pPage, aNewSettings );
        }

        // and last but not least to the handout page
        change( pUndoGroup, mpDoc->GetMasterSdPage( 0, PK_HANDOUT ), aNewSettings );
    }

    // give the undo group to the undo manager
    mpViewShell->GetViewFrame()->GetObjectShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
}

void HeaderFooterDialog::change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings )
{
    pUndoGroup->AddAction(new SdHeaderFooterUndoAction(mpDoc, pPage, rNewSettings ));
    pPage->setHeaderFooterSettings( rNewSettings );
}

HeaderFooterTabPage::HeaderFooterTabPage( vcl::Window* pWindow, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode ) :
        TabPage( pWindow, "HeaderFooterTab", "modules/simpress/ui/headerfootertab.ui" ),
        mpDoc(pDoc),
        mbHandoutMode( bHandoutMode )
{
    get(mpFTIncludeOn, "include_label");
    get(mpCBHeader, "header_cb" );
    get(mpHeaderBox, "header_box" );
    get(mpTBHeader, "header_text" );
    get(mpCBDateTime, "datetime_cb" );
    get(mpRBDateTimeFixed, "rb_fixed" );
    get(mpRBDateTimeAutomatic, "rb_auto" );
    get(mpTBDateTimeFixed, "datetime_value" );
    get(mpCBDateTimeFormat, "datetime_format_list" );
    get(mpFTDateTimeLanguage, "language_label" );
    get(mpCBDateTimeLanguage, "language_list" );

    get(mpCBFooter, "footer_cb" );
    get(mpFooterBox, "footer_box" );
    get(mpTBFooter, "footer_text" );

    get(mpCBSlideNumber, "slide_number" );

    get(mpCBNotOnTitle, "not_on_title" );

    get(mpCTPreview, "preview");
    mpCTPreview->init( pActualPage ?
            (pActualPage->IsMasterPage() ? pActualPage : static_cast<SdPage*>(&(pActualPage->TRG_GetMasterPage()))) :
            (pDoc->GetMasterSdPage( 0, bHandoutMode ? PK_NOTES : PK_STANDARD )) );

    if( mbHandoutMode )
    {
        OUString sPageNo = get<FixedText>("replacement_a")->GetText();
        mpCBSlideNumber->SetText( sPageNo );

        OUString sFrameTitle = get<FixedText>("replacement_b")->GetText();
        mpFTIncludeOn->SetText( sFrameTitle );
    }

    mpCBHeader->Show( mbHandoutMode );
    mpHeaderBox->Show( mbHandoutMode );
    mpCBNotOnTitle->Show( !mbHandoutMode );

    mpCBDateTime->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    mpRBDateTimeFixed->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    mpRBDateTimeAutomatic->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    mpCBFooter->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    mpCBHeader->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );
    mpCBSlideNumber->SetClickHdl( LINK( this, HeaderFooterTabPage, UpdateOnClickHdl ) );

    mpCBDateTimeLanguage->SetLanguageList( SvxLanguageListFlags::ALL|SvxLanguageListFlags::ONLY_KNOWN, false );
    mpCBDateTimeLanguage->SetSelectHdl( LINK( this, HeaderFooterTabPage, LanguageChangeHdl ) );

    GetOrSetDateTimeLanguage( meOldLanguage, false );
    meOldLanguage = MsLangId::getRealLanguage( meOldLanguage );
    mpCBDateTimeLanguage->SelectLanguage( meOldLanguage );

    FillFormatList(SVXDATEFORMAT_A);
}

HeaderFooterTabPage::~HeaderFooterTabPage()
{
    disposeOnce();
}

void HeaderFooterTabPage::dispose()
{
    mpFTIncludeOn.clear();
    mpCBHeader.clear();
    mpHeaderBox.clear();
    mpTBHeader.clear();
    mpCBDateTime.clear();
    mpRBDateTimeFixed.clear();
    mpRBDateTimeAutomatic.clear();
    mpTBDateTimeFixed.clear();
    mpCBDateTimeFormat.clear();
    mpFTDateTimeLanguage.clear();
    mpCBDateTimeLanguage.clear();
    mpCBFooter.clear();
    mpFooterBox.clear();
    mpTBFooter.clear();
    mpCBSlideNumber.clear();
    mpCBNotOnTitle.clear();
    mpCTPreview.clear();
    TabPage::dispose();
}

IMPL_LINK_NOARG_TYPED(HeaderFooterTabPage, LanguageChangeHdl, ListBox&, void)
{
    FillFormatList( (int)reinterpret_cast<sal_IntPtr>(mpCBDateTimeFormat->GetSelectEntryData()) );
}

void HeaderFooterTabPage::FillFormatList( int eFormat )
{
    LanguageType eLanguage = mpCBDateTimeLanguage->GetSelectLanguage();

    mpCBDateTimeFormat->Clear();

    Date aDate( Date::SYSTEM );
    tools::Time aTime( tools::Time::SYSTEM );

    int nFormat;
    for( nFormat = 0; nFormat < nDateTimeFormatsCount; nFormat++ )
    {
        OUString aStr( SvxDateTimeField::GetFormatted(
                aDate, aTime, nDateTimeFormats[nFormat],
                *(SD_MOD()->GetNumberFormatter()), eLanguage ) );
        const sal_Int32 nEntry = mpCBDateTimeFormat->InsertEntry( aStr );
        mpCBDateTimeFormat->SetEntryData( nEntry, reinterpret_cast<void*>((sal_IntPtr)nDateTimeFormats[nFormat] ));
        if( nDateTimeFormats[nFormat] == eFormat )
        {
            mpCBDateTimeFormat->SelectEntryPos( nEntry );
            mpCBDateTimeFormat->SetText( aStr );
        }
    }
}

void HeaderFooterTabPage::init( const HeaderFooterSettings& rSettings, bool bNotOnTitle )
{
    mpCBDateTime->Check( rSettings.mbDateTimeVisible );
    mpRBDateTimeFixed->Check( rSettings.mbDateTimeIsFixed );
    mpRBDateTimeAutomatic->Check( !rSettings.mbDateTimeIsFixed );
    mpTBDateTimeFixed->SetText( rSettings.maDateTimeText );

    mpCBHeader->Check( rSettings.mbHeaderVisible );
    mpTBHeader->SetText( rSettings.maHeaderText );

    mpCBFooter->Check( rSettings.mbFooterVisible );
    mpTBFooter->SetText( rSettings.maFooterText );

    mpCBSlideNumber->Check( rSettings.mbSlideNumberVisible );

    mpCBNotOnTitle->Check( bNotOnTitle );

    mpCBDateTimeLanguage->SelectLanguage( meOldLanguage );

    sal_uInt16 nPos;
    for( nPos = 0; nPos < mpCBDateTimeFormat->GetEntryCount(); nPos++ )
    {
        int nFormat = (int)reinterpret_cast<sal_IntPtr>(mpCBDateTimeFormat->GetEntryData( nPos ));
        if( nFormat == rSettings.meDateTimeFormat )
        {
            mpCBDateTimeFormat->SelectEntryPos( nPos );
            mpCBDateTimeFormat->SetText( mpCBDateTimeFormat->GetEntry(nPos) );
            break;
        }
    }

    update();
}

void HeaderFooterTabPage::getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle )
{
    rSettings.mbDateTimeVisible = mpCBDateTime->IsChecked();
    rSettings.mbDateTimeIsFixed = mpRBDateTimeFixed->IsChecked();
    rSettings.maDateTimeText = mpTBDateTimeFixed->GetText();
    rSettings.mbFooterVisible = mpCBFooter->IsChecked();
    rSettings.maFooterText = mpTBFooter->GetText();
    rSettings.mbSlideNumberVisible = mpCBSlideNumber->IsChecked();
    rSettings.mbHeaderVisible = mpCBHeader->IsChecked();
    rSettings.maHeaderText = mpTBHeader->GetText();

    if( mpCBDateTimeFormat->GetSelectEntryCount() == 1 )
        rSettings.meDateTimeFormat = (int)reinterpret_cast<sal_IntPtr>(mpCBDateTimeFormat->GetSelectEntryData());

    LanguageType eLanguage = mpCBDateTimeLanguage->GetSelectLanguage();
    if( eLanguage != meOldLanguage )
        GetOrSetDateTimeLanguage( eLanguage, true );

    rNotOnTitle = mpCBNotOnTitle->IsChecked();
}

void HeaderFooterTabPage::update()
{
    mpRBDateTimeFixed->Enable( mpCBDateTime->IsChecked() );
    mpTBDateTimeFixed->Enable( mpRBDateTimeFixed->IsChecked() && mpCBDateTime->IsChecked() );
    mpRBDateTimeAutomatic->Enable( mpCBDateTime->IsChecked() );
    mpCBDateTimeFormat->Enable( mpCBDateTime->IsChecked() && mpRBDateTimeAutomatic->IsChecked() );
    mpFTDateTimeLanguage->Enable( mpCBDateTime->IsChecked() && mpRBDateTimeAutomatic->IsChecked() );
    mpCBDateTimeLanguage->Enable( mpCBDateTime->IsChecked() && mpRBDateTimeAutomatic->IsChecked() );

    mpFooterBox->Enable( mpCBFooter->IsChecked() );

    mpHeaderBox->Enable( mpCBHeader->IsChecked() );

    HeaderFooterSettings aSettings;
    bool bNotOnTitle;
    getData( aSettings, bNotOnTitle );
    mpCTPreview->update( aSettings );
}

IMPL_LINK_NOARG_TYPED(HeaderFooterTabPage, UpdateOnClickHdl, Button*, void)
{
    update();
}

void HeaderFooterTabPage::GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet )
{
    if( mbHandoutMode )
    {
        // if set, set it on all notes master pages
        if( bSet )
        {
            sal_uInt16 nPageCount = mpDoc->GetMasterSdPageCount( PK_NOTES );
            sal_uInt16 nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( nPage, PK_NOTES ) );
            }
        }

        // #i119985# and set it, or just get it from the notes master page
        GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( 0, PK_NOTES ) );
    }
    else
    {
        // get the language from the first master page
        // or set it to all master pages
        sal_uInt16 nPageCount = bSet ? mpDoc->GetMasterSdPageCount( PK_NOTES ) : 1;
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( nPage, PK_STANDARD ) );
        }
    }
}

void HeaderFooterTabPage::GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet, SdPage* pPage )
{
    if( pPage )
    {
        SdrTextObj* pObj = static_cast<SdrTextObj*>(pPage->GetPresObj( PRESOBJ_DATETIME ));
        if( pObj )
        {
            Outliner* pOutl = mpDoc->GetInternalOutliner();
            pOutl->Init( OutlinerMode::TextObject );
            OutlinerMode nOutlMode = pOutl->GetMode();

            EditEngine* pEdit = const_cast< EditEngine* >(&pOutl->GetEditEngine());

            OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
            if( pOPO )
                pOutl->SetText( *pOPO );

            EFieldInfo aFieldInfo;
            aFieldInfo.pFieldItem = nullptr;

            sal_Int32 nParaCount = pEdit->GetParagraphCount();
            sal_Int32 nPara;
            for( nPara = 0; (nPara < nParaCount) && (aFieldInfo.pFieldItem == nullptr); nPara++ )
            {
                sal_uInt16 nFieldCount = pEdit->GetFieldCount( nPara );
                sal_uInt16 nField;
                for( nField = 0; (nField < nFieldCount) && (aFieldInfo.pFieldItem == nullptr); nField++ )
                {
                    aFieldInfo = pEdit->GetFieldInfo( nPara, nField );
                    if( aFieldInfo.pFieldItem )
                    {
                        const SvxFieldData* pFieldData = aFieldInfo.pFieldItem->GetField();
                        if( pFieldData && ( dynamic_cast< const SvxDateTimeField *>( pFieldData ) != nullptr || dynamic_cast< const SvxDateField *>( pFieldData ) != nullptr))
                        {
                            break;
                        }
                    }
                    aFieldInfo.pFieldItem = nullptr;
                }
            }

            if( aFieldInfo.pFieldItem != nullptr )
            {
                if( bSet )
                {
                    SfxItemSet aSet( pEdit->GetAttribs( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex, aFieldInfo.aPosition.nIndex+1, GetAttribsFlags::CHARATTRIBS ) );

                    SvxLanguageItem aItem( rLanguage, EE_CHAR_LANGUAGE );
                    aSet.Put( aItem );

                    SvxLanguageItem aItemCJK( rLanguage, EE_CHAR_LANGUAGE_CJK );
                    aSet.Put( aItemCJK );

                    SvxLanguageItem aItemCTL( rLanguage, EE_CHAR_LANGUAGE_CTL );
                    aSet.Put( aItemCTL );

                    ESelection aSel( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex, aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex+1 );
                    pEdit->QuickSetAttribs( aSet, aSel );

                    pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
                    pOutl->UpdateFields();
                }
                else
                {
                    rLanguage =  pOutl->GetLanguage( aFieldInfo.aPosition.nPara, aFieldInfo.aPosition.nIndex );
                }
            }

            pOutl->Clear();
            pOutl->Init( nOutlMode );
        }
    }
}

PresLayoutPreview::PresLayoutPreview( vcl::Window* pParent )
    : Control(pParent)
    , mpMaster(nullptr)
{
}

VCL_BUILDER_FACTORY(PresLayoutPreview)

PresLayoutPreview::~PresLayoutPreview()
{
}

Size PresLayoutPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 80), MAP_APPFONT);
}

void PresLayoutPreview::init( SdPage *pMaster )
{
    mpMaster = pMaster;
    maPageSize = pMaster->GetSize();
}

void PresLayoutPreview::update( HeaderFooterSettings& rSettings )
{
    maSettings = rSettings;
    Invalidate();
}

void PresLayoutPreview::Paint(vcl::RenderContext& rRenderContext, SdrTextObj* pObj, bool bVisible, bool bDotted /* = false*/ )
{
    // get object transformation
    basegfx::B2DHomMatrix aObjectTransform;
    basegfx::B2DPolyPolygon aObjectPolyPolygon;
    pObj->TRGetBaseGeometry(aObjectTransform, aObjectPolyPolygon);

    // build complete transformation by adding view transformation from
    // logic page coordinates to local pixel coordinates
    const double fScaleX((double)maOutRect.getWidth() / (double)maPageSize.Width());
    const double fScaleY((double)maOutRect.getHeight() / (double)maPageSize.Height());
    aObjectTransform.scale(fScaleX, fScaleY);
    aObjectTransform.translate(maOutRect.TopLeft().X(), maOutRect.TopLeft().Y());

    // create geometry using unit range and object transform
    basegfx::B2DPolyPolygon aGeometry(basegfx::tools::createUnitPolygon());
    aGeometry.transform(aObjectTransform);

    // apply line pattern if wanted
    if (bDotted)
    {
        std::vector<double> aPattern;
        static double fFactor(1.0);
        aPattern.push_back(3.0 * fFactor);
        aPattern.push_back(1.0 * fFactor);

        basegfx::B2DPolyPolygon aDashed;
        basegfx::tools::applyLineDashing(aGeometry, aPattern, &aDashed);
        aGeometry = aDashed;
    }

    // determine line color
    svtools::ColorConfig aColorConfig;
    svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( bVisible ? svtools::FONTCOLOR : svtools::OBJECTBOUNDARIES ) );

    // paint at OutDev
    rRenderContext.SetLineColor(Color(aColor.nColor));
    rRenderContext.SetFillColor();

    for (sal_uInt32 a(0); a < aGeometry.count(); a++)
    {
        rRenderContext.DrawPolyLine(aGeometry.getB2DPolygon(a));
    }
}

void PresLayoutPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.Push();

    maOutRect = Rectangle(Point(0,0), rRenderContext.GetOutputSize());

    // calculate page size with correct aspect ratio
    int nWidth, nHeight;
    if( maPageSize.Width() > maPageSize.Height() )
    {
        nWidth = maOutRect.GetWidth();
        nHeight = long( (double)(nWidth * maPageSize.Height()) / (double)maPageSize.Width() );
    }
    else
    {
        nHeight = maOutRect.GetHeight();
        nWidth = long( (double)(nHeight * maPageSize.Width()) / (double)maPageSize.Height() );
    }

    maOutRect.Left() += (maOutRect.GetWidth() - nWidth) >> 1;
    maOutRect.Right() = maOutRect.Left() + nWidth - 1;
    maOutRect.Top() += (maOutRect.GetHeight() - nHeight) >> 1;
    maOutRect.Bottom() = maOutRect.Top() + nHeight - 1;

    // draw decoration frame
    DecorationView aDecoView(&rRenderContext);
    maOutRect = aDecoView.DrawFrame(maOutRect, DrawFrameStyle::In);

    // draw page background
    rRenderContext.SetFillColor(Color(COL_WHITE));
    rRenderContext.DrawRect(maOutRect);

    // paint presentation objects from masterpage
    if (nullptr != mpMaster)
    {
        SdrTextObj* pMasterTitle = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PRESOBJ_TITLE));
        SdrTextObj* pMasterOutline = static_cast<SdrTextObj*>(mpMaster->GetPresObj(mpMaster->GetPageKind() == PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE));
        SdrTextObj* pHeader = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PRESOBJ_HEADER));
        SdrTextObj* pFooter = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PRESOBJ_FOOTER));
        SdrTextObj* pDate = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PRESOBJ_DATETIME));
        SdrTextObj* pNumber = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PRESOBJ_SLIDENUMBER));

        if (pMasterTitle)
            Paint(rRenderContext, pMasterTitle, true, true);
        if (pMasterOutline)
            Paint(rRenderContext, pMasterOutline, true, true);
        if (pHeader)
            Paint(rRenderContext, pHeader, maSettings.mbHeaderVisible);
        if (pFooter)
            Paint(rRenderContext, pFooter, maSettings.mbFooterVisible);
        if (pDate)
            Paint(rRenderContext, pDate, maSettings.mbDateTimeVisible);
        if (pNumber)
            Paint(rRenderContext, pNumber, maSettings.mbSlideNumberVisible);
    }

    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/langitem.hxx>
#include <svx/langbox.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/debug.hxx>

#include <Outliner.hxx>
#include <headerfooterdlg.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <sdmod.hxx>

// preview control for presentation layout
#include <tools/color.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/customweld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>

#include <undoheaderfooter.hxx>
#include <sdundogr.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace sd
{

namespace {

class PresLayoutPreview : public weld::CustomWidgetController
{
private:
    SdPage* mpMaster;
    HeaderFooterSettings maSettings;
    Size maPageSize;
    ::tools::Rectangle maOutRect;

private:
    void Paint(vcl::RenderContext& rRenderContext, SdrTextObj const * pObj, bool bVisible, bool bDotted = false);

public:
    explicit PresLayoutPreview();

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

    void init(SdPage* pMaster);
    void update(HeaderFooterSettings const & rSettings);
};

}

}

// tab page for slide & header'n'notes

namespace sd
{

const int nDateTimeFormatsCount = 12;

namespace {

struct DateAndTimeFormat {
    SvxDateFormat meDateFormat;
    SvxTimeFormat meTimeFormat;
};

}

DateAndTimeFormat const nDateTimeFormats[nDateTimeFormatsCount] =
{
    { SvxDateFormat::A, SvxTimeFormat::AppDefault },
    { SvxDateFormat::B, SvxTimeFormat::AppDefault },
    { SvxDateFormat::C, SvxTimeFormat::AppDefault },
    { SvxDateFormat::D, SvxTimeFormat::AppDefault },
    { SvxDateFormat::E, SvxTimeFormat::AppDefault },
    { SvxDateFormat::F, SvxTimeFormat::AppDefault },

    { SvxDateFormat::A, SvxTimeFormat::HH24_MM },
    { SvxDateFormat::A, SvxTimeFormat::HH12_MM },

    { SvxDateFormat::AppDefault, SvxTimeFormat::HH24_MM },
    { SvxDateFormat::AppDefault, SvxTimeFormat::HH24_MM_SS },

    { SvxDateFormat::AppDefault, SvxTimeFormat::HH12_MM },
    { SvxDateFormat::AppDefault, SvxTimeFormat::HH12_MM_SS },
};

class HeaderFooterTabPage
{
private:
    SdDrawDocument*     mpDoc;
    LanguageType        meOldLanguage;
    bool                mbHandoutMode;

    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Label> mxFTIncludeOn;
    std::unique_ptr<weld::CheckButton> mxCBHeader;
    std::unique_ptr<weld::Widget> mxHeaderBox;
    std::unique_ptr<weld::Entry> mxTBHeader;
    std::unique_ptr<weld::CheckButton> mxCBDateTime;
    std::unique_ptr<weld::RadioButton> mxRBDateTimeFixed;
    std::unique_ptr<weld::RadioButton> mxRBDateTimeAutomatic;
    std::unique_ptr<weld::Entry> mxTBDateTimeFixed;
    std::unique_ptr<weld::ComboBox> mxCBDateTimeFormat;
    std::unique_ptr<weld::Label> mxFTDateTimeLanguage;
    std::unique_ptr<SvxLanguageBox> mxCBDateTimeLanguage;
    std::unique_ptr<weld::CheckButton> mxCBFooter;
    std::unique_ptr<weld::Widget> mxFooterBox;
    std::unique_ptr<weld::Entry> mxTBFooter;
    std::unique_ptr<weld::CheckButton> mxCBSlideNumber;
    std::unique_ptr<weld::CheckButton> mxCBNotOnTitle;
    std::unique_ptr<weld::Label> mxReplacementA;
    std::unique_ptr<weld::Label> mxReplacementB;
    std::unique_ptr<PresLayoutPreview> mxCTPreview;
    std::unique_ptr<weld::CustomWeld> mxCTPreviewWin;


    DECL_LINK( UpdateOnToggleHdl, weld::Toggleable&, void );
    DECL_LINK( LanguageChangeHdl, weld::ComboBox&, void );

    void FillFormatList(sal_Int32 nSelectedPos);
    void GetOrSetDateTimeLanguage( LanguageType &rLanguage, bool bSet );
    // returns true if the page has a date/time field item
    bool GetOrSetDateTimeLanguage(LanguageType& rLanguage, bool bSet, SdPage* pPage);

public:
    HeaderFooterTabPage(weld::Container* pParent, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode );

    void    init( const HeaderFooterSettings& rSettings, bool bNotOnTitle );
    void    getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle );
    void    update();
};

}

using namespace ::sd;

HeaderFooterDialog::HeaderFooterDialog(ViewShell* pViewShell, weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage)
    : GenericDialogController(pParent, u"modules/simpress/ui/headerfooterdialog.ui"_ustr, u"HeaderFooterDialog"_ustr)
    , mpDoc( pDoc )
    , mpCurrentPage( pCurrentPage )
    , mpViewShell( pViewShell )
    , mxTabCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , mxPBApplyToAll(m_xBuilder->weld_button(u"apply_all"_ustr))
    , mxPBApply(m_xBuilder->weld_button(u"apply"_ustr))
    , mxPBCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    SdPage* pSlide;
    SdPage* pNotes;
    if( pCurrentPage->GetPageKind() == PageKind::Standard )
    {
        pSlide = pCurrentPage;
        pNotes = static_cast<SdPage*>(pDoc->GetPage( pCurrentPage->GetPageNum() + 1 ));
    }
    else if( pCurrentPage->GetPageKind() == PageKind::Notes )
    {
        pNotes = pCurrentPage;
        pSlide = static_cast<SdPage*>(pDoc->GetPage( pCurrentPage->GetPageNum() -1 ));
        mpCurrentPage = pSlide;
    }
    else
    {
        // handout
        pSlide = pDoc->GetSdPage( 0, PageKind::Standard );
        pNotes = pDoc->GetSdPage( 0, PageKind::Notes );
        mpCurrentPage = nullptr;
    }

    mxSlideTabPage.reset(new HeaderFooterTabPage(mxTabCtrl->get_page(u"slides"_ustr), pDoc, pSlide, false));
    mxNotesHandoutsTabPage.reset(new HeaderFooterTabPage(mxTabCtrl->get_page(u"notes"_ustr), pDoc, pNotes, true));

    pDoc->StopWorkStartupDelay();
    mxTabCtrl->show();

    ActivatePageHdl(mxTabCtrl->get_current_page_ident());

    mxTabCtrl->connect_enter_page( LINK( this, HeaderFooterDialog, ActivatePageHdl ) );

    mxPBApplyToAll->connect_clicked( LINK( this, HeaderFooterDialog, ClickApplyToAllHdl ) );
    mxPBApply->connect_clicked( LINK( this, HeaderFooterDialog, ClickApplyHdl ) );
    mxPBCancel->connect_clicked( LINK( this, HeaderFooterDialog, ClickCancelHdl ) );

    maSlideSettings = pSlide->getHeaderFooterSettings();

    const HeaderFooterSettings& rTitleSettings = mpDoc->GetSdPage(0, PageKind::Standard)->getHeaderFooterSettings();
    bool bNotOnTitle = !rTitleSettings.mbFooterVisible && !rTitleSettings.mbSlideNumberVisible && !rTitleSettings.mbDateTimeVisible;

    mxSlideTabPage->init( maSlideSettings, bNotOnTitle );

    maNotesHandoutSettings = pNotes->getHeaderFooterSettings();
    mxNotesHandoutsTabPage->init( maNotesHandoutSettings, false );
}

HeaderFooterDialog::~HeaderFooterDialog()
{
}

IMPL_LINK(HeaderFooterDialog, ActivatePageHdl, const OUString&, rIdent, void)
{
    mxPBApply->set_visible(rIdent == "slides");
    mxPBApply->set_sensitive(mpCurrentPage != nullptr);
}

IMPL_LINK_NOARG(HeaderFooterDialog, ClickApplyToAllHdl, weld::Button&, void)
{
    ApplyToAll();
}

IMPL_LINK_NOARG(HeaderFooterDialog, ClickApplyHdl, weld::Button&, void)
{
    Apply();
}

IMPL_LINK_NOARG(HeaderFooterDialog, ClickCancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

short HeaderFooterDialog::run()
{
    short nRet = GenericDialogController::run();
    if (nRet)
        mpViewShell->GetDocSh()->SetModified();
    return nRet;
}

void HeaderFooterDialog::ApplyToAll()
{
    OUString tabId = mxTabCtrl->get_current_page_ident();
    apply(true, tabId == "slides");
    m_xDialog->response(RET_OK);
}

void HeaderFooterDialog::Apply()
{
    OUString tabId = mxTabCtrl->get_current_page_ident();
    apply(false, tabId == "slides");
    m_xDialog->response(RET_OK);
}

void HeaderFooterDialog::apply( bool bToAll, bool bForceSlides )
{
    std::unique_ptr<SdUndoGroup> pUndoGroup(new SdUndoGroup(mpDoc));
    OUString aComment( m_xDialog->get_title() );
    pUndoGroup->SetComment( aComment );

    HeaderFooterSettings aNewSettings;
    bool bNewNotOnTitle;

    // change slide settings first ...

    mxSlideTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the slide tab page or if the slide settings
    // have been changed
    if( bForceSlides || !(aNewSettings == maSlideSettings) )
    {
        // apply to all slides
        if( bToAll )
        {
            int nPageCount = mpDoc->GetSdPageCount( PageKind::Standard );
            int nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                SdPage* pPage = mpDoc->GetSdPage( static_cast<sal_uInt16>(nPage), PageKind::Standard );
                change( pUndoGroup.get(), pPage, aNewSettings );
            }
        }
        else
        {
            // apply only to the current slide
            DBG_ASSERT( mpCurrentPage && mpCurrentPage->GetPageKind() == PageKind::Standard, "no current page to apply to!" );
            if( mpCurrentPage && (mpCurrentPage->GetPageKind() == PageKind::Standard) )
            {
                change( pUndoGroup.get(), mpCurrentPage, aNewSettings );
            }
        }
    }

    // if we don't want to have header&footer on the first slide
    if( bNewNotOnTitle )
    {
        // just hide them, plain simple UI feature
        HeaderFooterSettings aTempSettings = mpDoc->GetSdPage( 0, PageKind::Standard )->getHeaderFooterSettings();

        aTempSettings.mbFooterVisible = false;
        aTempSettings.mbSlideNumberVisible = false;
        aTempSettings.mbDateTimeVisible = false;

        change( pUndoGroup.get(), mpDoc->GetSdPage( 0, PageKind::Standard ), aTempSettings );
    }

    // now notes settings

    mxNotesHandoutsTabPage->getData( aNewSettings, bNewNotOnTitle );

    // only if we pressed apply or apply all on the notes tab page or if the notes settings
    // have been changed
    if( !bForceSlides || !(aNewSettings == maNotesHandoutSettings) )
    {
        // first set to all notes pages
        int nPageCount = mpDoc->GetSdPageCount( PageKind::Notes );
        int nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            SdPage* pPage = mpDoc->GetSdPage( static_cast<sal_uInt16>(nPage), PageKind::Notes );

            change( pUndoGroup.get(), pPage, aNewSettings );
        }

        // and last but not least to the handout page
        change( pUndoGroup.get(), mpDoc->GetMasterSdPage( 0, PageKind::Handout ), aNewSettings );
    }

    // give the undo group to the undo manager
    mpViewShell->GetViewFrame()->GetObjectShell()->GetUndoManager()->AddUndoAction(std::move(pUndoGroup));
}

void HeaderFooterDialog::change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings )
{
    pUndoGroup->AddAction(new SdHeaderFooterUndoAction(mpDoc, pPage, rNewSettings ));
    pPage->setHeaderFooterSettings( rNewSettings );
}

HeaderFooterTabPage::HeaderFooterTabPage(weld::Container* pParent, SdDrawDocument* pDoc, SdPage* pActualPage, bool bHandoutMode)
    : mpDoc(pDoc)
    , mbHandoutMode(bHandoutMode)
    , mxBuilder(Application::CreateBuilder(pParent, u"modules/simpress/ui/headerfootertab.ui"_ustr))
    , mxContainer(mxBuilder->weld_container(u"HeaderFooterTab"_ustr))
    , mxFTIncludeOn(mxBuilder->weld_label(u"include_label"_ustr))
    , mxCBHeader(mxBuilder->weld_check_button(u"header_cb"_ustr ))
    , mxHeaderBox(mxBuilder->weld_widget(u"header_box"_ustr))
    , mxTBHeader(mxBuilder->weld_entry(u"header_text"_ustr))
    , mxCBDateTime(mxBuilder->weld_check_button(u"datetime_cb"_ustr))
    , mxRBDateTimeFixed(mxBuilder->weld_radio_button(u"rb_fixed"_ustr))
    , mxRBDateTimeAutomatic(mxBuilder->weld_radio_button(u"rb_auto"_ustr))
    , mxTBDateTimeFixed(mxBuilder->weld_entry(u"datetime_value"_ustr))
    , mxCBDateTimeFormat(mxBuilder->weld_combo_box(u"datetime_format_list"_ustr))
    , mxFTDateTimeLanguage(mxBuilder->weld_label(u"language_label"_ustr))
    , mxCBDateTimeLanguage(new SvxLanguageBox(mxBuilder->weld_combo_box(u"language_list"_ustr)))
    , mxCBFooter(mxBuilder->weld_check_button(u"footer_cb"_ustr))
    , mxFooterBox(mxBuilder->weld_widget(u"footer_box"_ustr ))
    , mxTBFooter(mxBuilder->weld_entry(u"footer_text"_ustr))
    , mxCBSlideNumber(mxBuilder->weld_check_button(u"slide_number"_ustr))
    , mxCBNotOnTitle(mxBuilder->weld_check_button(u"not_on_title"_ustr))
    , mxReplacementA(mxBuilder->weld_label(u"replacement_a"_ustr))
    , mxReplacementB(mxBuilder->weld_label(u"replacement_b"_ustr))
    , mxCTPreview(new PresLayoutPreview)
    , mxCTPreviewWin(new weld::CustomWeld(*mxBuilder, u"preview"_ustr, *mxCTPreview))
{
    mxCTPreview->init( pActualPage ?
            (pActualPage->IsMasterPage() ? pActualPage : static_cast<SdPage*>(&(pActualPage->TRG_GetMasterPage()))) :
            (pDoc->GetMasterSdPage( 0, bHandoutMode ? PageKind::Notes : PageKind::Standard )) );

    if( mbHandoutMode )
    {
        OUString sPageNo = mxReplacementA->get_label();
        mxCBSlideNumber->set_label( sPageNo );

        OUString sFrameTitle = mxReplacementB->get_label();
        mxFTIncludeOn->set_label( sFrameTitle );
    }

    mxCBHeader->set_visible( mbHandoutMode );
    mxHeaderBox->set_visible( mbHandoutMode );
    mxCBNotOnTitle->set_visible( !mbHandoutMode );

    mxCBDateTime->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );
    mxRBDateTimeFixed->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );
    mxRBDateTimeAutomatic->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );
    mxCBFooter->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );
    mxCBHeader->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );
    mxCBSlideNumber->connect_toggled( LINK( this, HeaderFooterTabPage, UpdateOnToggleHdl ) );

    mxCBDateTimeLanguage->SetLanguageList( SvxLanguageListFlags::ALL|SvxLanguageListFlags::ONLY_KNOWN, false, false );
    mxCBDateTimeLanguage->connect_changed( LINK( this, HeaderFooterTabPage, LanguageChangeHdl ) );

    GetOrSetDateTimeLanguage( meOldLanguage, false );
    meOldLanguage = MsLangId::getRealLanguage( meOldLanguage );
    mxCBDateTimeLanguage->set_active_id( meOldLanguage );

    FillFormatList(0);
}

IMPL_LINK_NOARG(HeaderFooterTabPage, LanguageChangeHdl, weld::ComboBox&, void)
{
    FillFormatList( mxCBDateTimeFormat->get_active() );
}

void HeaderFooterTabPage::FillFormatList( sal_Int32 nSelectedPos )
{
    LanguageType eLanguage = mxCBDateTimeLanguage->get_active_id();

    mxCBDateTimeFormat->clear();

    DateTime aDateTime( DateTime::SYSTEM );

    for (int nFormat = 0; nFormat < nDateTimeFormatsCount; ++nFormat)
    {
        OUString aStr( SvxDateTimeField::GetFormatted(
                aDateTime, aDateTime,
                nDateTimeFormats[nFormat].meDateFormat, nDateTimeFormats[nFormat].meTimeFormat,
                *(SD_MOD()->GetNumberFormatter()), eLanguage ) );
        mxCBDateTimeFormat->append_text(aStr);
        if (nFormat == nSelectedPos)
            mxCBDateTimeFormat->set_active(nFormat);
    }
}

void HeaderFooterTabPage::init( const HeaderFooterSettings& rSettings, bool bNotOnTitle )
{
    mxCBDateTime->set_active( rSettings.mbDateTimeVisible );
    mxRBDateTimeFixed->set_active( rSettings.mbDateTimeIsFixed );
    mxRBDateTimeAutomatic->set_active( !rSettings.mbDateTimeIsFixed );
    mxTBDateTimeFixed->set_text( rSettings.maDateTimeText );

    mxCBHeader->set_active( rSettings.mbHeaderVisible );
    mxTBHeader->set_text( rSettings.maHeaderText );

    mxCBFooter->set_active( rSettings.mbFooterVisible );
    mxTBFooter->set_text( rSettings.maFooterText );

    mxCBSlideNumber->set_active( rSettings.mbSlideNumberVisible );

    mxCBNotOnTitle->set_active( bNotOnTitle );

    mxCBDateTimeLanguage->set_active_id( meOldLanguage );

    for (sal_Int32 nPos = 0, nEntryCount = mxCBDateTimeFormat->get_count(); nPos < nEntryCount; ++nPos)
    {
        if( nDateTimeFormats[nPos].meDateFormat == rSettings.meDateFormat && nDateTimeFormats[nPos].meTimeFormat == rSettings.meTimeFormat )
        {
            mxCBDateTimeFormat->set_active(nPos);
            break;
        }
    }

    update();
}

void HeaderFooterTabPage::getData( HeaderFooterSettings& rSettings, bool& rNotOnTitle )
{
    rSettings.mbDateTimeVisible = mxCBDateTime->get_active();
    rSettings.mbDateTimeIsFixed = mxRBDateTimeFixed->get_active();
    rSettings.maDateTimeText = mxTBDateTimeFixed->get_text();
    rSettings.mbFooterVisible = mxCBFooter->get_active();
    rSettings.maFooterText = mxTBFooter->get_text();
    rSettings.mbSlideNumberVisible = mxCBSlideNumber->get_active();
    rSettings.mbHeaderVisible = mxCBHeader->get_active();
    rSettings.maHeaderText = mxTBHeader->get_text();

    int nPos = mxCBDateTimeFormat->get_active();
    if (nPos != -1)
    {
        rSettings.meDateFormat = nDateTimeFormats[nPos].meDateFormat;
        rSettings.meTimeFormat = nDateTimeFormats[nPos].meTimeFormat;
    }

    LanguageType eLanguage = mxCBDateTimeLanguage->get_active_id();
    if( eLanguage != meOldLanguage )
        GetOrSetDateTimeLanguage( eLanguage, true );

    rNotOnTitle = mxCBNotOnTitle->get_active();
}

void HeaderFooterTabPage::update()
{
    mxRBDateTimeFixed->set_sensitive( mxCBDateTime->get_active() );
    mxTBDateTimeFixed->set_sensitive( mxRBDateTimeFixed->get_active() && mxCBDateTime->get_active() );
    mxRBDateTimeAutomatic->set_sensitive( mxCBDateTime->get_active() );
    mxCBDateTimeFormat->set_sensitive( mxCBDateTime->get_active() && mxRBDateTimeAutomatic->get_active() );
    mxFTDateTimeLanguage->set_sensitive( mxCBDateTime->get_active() && mxRBDateTimeAutomatic->get_active() );
    mxCBDateTimeLanguage->set_sensitive( mxCBDateTime->get_active() && mxRBDateTimeAutomatic->get_active() );
    mxFooterBox->set_sensitive( mxCBFooter->get_active() );
    mxHeaderBox->set_sensitive( mxCBHeader->get_active() );

    HeaderFooterSettings aSettings;
    bool bNotOnTitle;
    getData( aSettings, bNotOnTitle );
    mxCTPreview->update( aSettings );
}

IMPL_LINK_NOARG(HeaderFooterTabPage, UpdateOnToggleHdl, weld::Toggleable&, void)
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
            sal_uInt16 nPageCount = mpDoc->GetMasterSdPageCount( PageKind::Notes );
            sal_uInt16 nPage;
            for( nPage = 0; nPage < nPageCount; nPage++ )
            {
                GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( nPage, PageKind::Notes ) );
            }
        }

        // #i119985# and set it, or just get it from the notes master page
        GetOrSetDateTimeLanguage( rLanguage, bSet, mpDoc->GetMasterSdPage( 0, PageKind::Notes ) );
    }
    else
    {
        const sal_uInt16 nPageCount = mpDoc->GetMasterSdPageCount(PageKind::Standard);
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nPageCount; nPage++ )
        {
            SdPage* pMasterSlide = mpDoc->GetMasterSdPage(nPage, PageKind::Standard);
            bool bHasDateFieldItem = GetOrSetDateTimeLanguage(rLanguage, bSet, pMasterSlide);

            // All pages must use the same language. If getting the language, only need to find one.
            if (!bSet && bHasDateFieldItem)
                break;
        }
    }
}

bool HeaderFooterTabPage::GetOrSetDateTimeLanguage(LanguageType& rLanguage, bool bSet,
                                                   SdPage* pPage)
{
    if( !pPage )
        return false;

    SdrTextObj* pObj = static_cast<SdrTextObj*>(pPage->GetPresObj( PresObjKind::DateTime ));
    if( !pObj )
        return false;

    Outliner* pOutl = mpDoc->GetInternalOutliner();
    pOutl->Init( OutlinerMode::TextObject );
    OutlinerMode nOutlMode = pOutl->GetOutlinerMode();

    EditEngine* pEdit = const_cast< EditEngine* >(&pOutl->GetEditEngine());

    OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
    if( pOPO )
        pOutl->SetText( *pOPO );

    EPosition aDateFieldPosition;
    bool bHasDateFieldItem = false;

    sal_Int32 nParaCount = pEdit->GetParagraphCount();
    for (sal_Int32 nPara = 0; (nPara < nParaCount) && !bHasDateFieldItem; ++nPara)
    {
        sal_uInt16 nFieldCount = pEdit->GetFieldCount(nPara);
        for (sal_uInt16 nField = 0; (nField < nFieldCount); ++nField)
        {
            EFieldInfo aFieldInfo = pEdit->GetFieldInfo(nPara, nField);
            if (aFieldInfo.pFieldItem)
            {
                const SvxFieldData* pFieldData = aFieldInfo.pFieldItem->GetField();
                if (dynamic_cast<const SvxDateTimeField*>(pFieldData) != nullptr ||
                    dynamic_cast<const SvxDateField*>(pFieldData) != nullptr)
                {
                    bHasDateFieldItem = true;
                    aDateFieldPosition = aFieldInfo.aPosition;
                    break;
                }
            }
        }
    }

    if (bHasDateFieldItem)
    {
        if( bSet )
        {
            SfxItemSet aSet(pEdit->GetAttribs(aDateFieldPosition.nPara,
                                              aDateFieldPosition.nIndex,
                                              aDateFieldPosition.nIndex+1,
                                              GetAttribsFlags::CHARATTRIBS));

            SvxLanguageItem aItem( rLanguage, EE_CHAR_LANGUAGE );
            aSet.Put( aItem );

            SvxLanguageItem aItemCJK( rLanguage, EE_CHAR_LANGUAGE_CJK );
            aSet.Put( aItemCJK );

            SvxLanguageItem aItemCTL( rLanguage, EE_CHAR_LANGUAGE_CTL );
            aSet.Put( aItemCTL );

            ESelection aSel(aDateFieldPosition.nPara, aDateFieldPosition.nIndex,
                            aDateFieldPosition.nPara, aDateFieldPosition.nIndex+1 );
            pEdit->QuickSetAttribs( aSet, aSel );

            pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
            pOutl->UpdateFields();
        }
        else
        {
            rLanguage =  pOutl->GetLanguage(aDateFieldPosition.nPara,
                                            aDateFieldPosition.nIndex );
        }
    }

    pOutl->Clear();
    pOutl->Init( nOutlMode );
    return bHasDateFieldItem;
}

PresLayoutPreview::PresLayoutPreview()
    : mpMaster(nullptr)
{
}

void PresLayoutPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(80, 80), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aSize);
}

void PresLayoutPreview::init( SdPage *pMaster )
{
    mpMaster = pMaster;
    maPageSize = pMaster->GetSize();
}

void PresLayoutPreview::update( HeaderFooterSettings const & rSettings )
{
    maSettings = rSettings;
    Invalidate();
}

void PresLayoutPreview::Paint(vcl::RenderContext& rRenderContext, SdrTextObj const * pObj, bool bVisible, bool bDotted /* = false*/ )
{
    // get object transformation
    basegfx::B2DHomMatrix aObjectTransform;
    basegfx::B2DPolyPolygon aObjectPolyPolygon;
    pObj->TRGetBaseGeometry(aObjectTransform, aObjectPolyPolygon);

    // build complete transformation by adding view transformation from
    // logic page coordinates to local pixel coordinates
    const double fScaleX(static_cast<double>(maOutRect.getOpenWidth()) / static_cast<double>(maPageSize.Width()));
    const double fScaleY(static_cast<double>(maOutRect.getOpenHeight()) / static_cast<double>(maPageSize.Height()));
    aObjectTransform.scale(fScaleX, fScaleY);
    aObjectTransform.translate(maOutRect.Left(), maOutRect.Top());

    // create geometry using unit range and object transform
    basegfx::B2DPolyPolygon aGeometry(basegfx::utils::createUnitPolygon());
    aGeometry.transform(aObjectTransform);

    // apply line pattern if wanted
    if (bDotted)
    {
        static const double fFactor(1.0);
        std::vector<double> aPattern
        {
            3.0 * fFactor,
            1.0 * fFactor
        };

        basegfx::B2DPolyPolygon aDashed;
        basegfx::utils::applyLineDashing(aGeometry, aPattern, &aDashed);
        aGeometry = aDashed;
    }

    // determine line color
    svtools::ColorConfig aColorConfig;
    svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( bVisible ? svtools::FONTCOLOR : svtools::OBJECTBOUNDARIES ) );

    // paint at OutDev
    rRenderContext.SetLineColor(aColor.nColor);
    rRenderContext.SetFillColor();

    for (sal_uInt32 a(0); a < aGeometry.count(); a++)
    {
        rRenderContext.DrawPolyLine(aGeometry.getB2DPolygon(a));
    }
}

void PresLayoutPreview::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.Push();

    maOutRect = ::tools::Rectangle(Point(0,0), rRenderContext.GetOutputSize());

    // calculate page size with correct aspect ratio
    int nWidth, nHeight;
    if( maPageSize.Width() > maPageSize.Height() )
    {
        nWidth = maOutRect.GetWidth();
        nHeight = maPageSize.Width() == 0 ? 0 : tools::Long( static_cast<double>(nWidth * maPageSize.Height()) / static_cast<double>(maPageSize.Width()) );
    }
    else
    {
        nHeight = maOutRect.GetHeight();
        nWidth = maPageSize.Height() == 0 ? 0 : tools::Long( static_cast<double>(nHeight * maPageSize.Width()) / static_cast<double>(maPageSize.Height()) );
    }

    maOutRect.AdjustLeft((maOutRect.GetWidth() - nWidth) >> 1 );
    maOutRect.SetRight( maOutRect.Left() + nWidth - 1 );
    maOutRect.AdjustTop((maOutRect.GetHeight() - nHeight) >> 1 );
    maOutRect.SetBottom( maOutRect.Top() + nHeight - 1 );

    // draw decoration frame
    DecorationView aDecoView(&rRenderContext);
    maOutRect = aDecoView.DrawFrame(maOutRect, DrawFrameStyle::In);

    // draw page background
    rRenderContext.SetFillColor(COL_WHITE);
    rRenderContext.DrawRect(maOutRect);

    // paint presentation objects from masterpage
    if (nullptr != mpMaster)
    {
        SdrTextObj* pMasterTitle = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PresObjKind::Title));
        SdrTextObj* pMasterOutline = static_cast<SdrTextObj*>(mpMaster->GetPresObj(mpMaster->GetPageKind() == PageKind::Notes ? PresObjKind::Notes : PresObjKind::Outline));
        SdrTextObj* pHeader = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PresObjKind::Header));
        SdrTextObj* pFooter = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PresObjKind::Footer));
        SdrTextObj* pDate = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PresObjKind::DateTime));
        SdrTextObj* pNumber = static_cast<SdrTextObj*>(mpMaster->GetPresObj(PresObjKind::SlideNumber));

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

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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>
#include <utility>
#include <vcl/event.hxx>
#include <vcl/mnemonic.hxx>
#include "general.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
#include <strings.hrc>
#include "bibmod.hxx"
#include <helpids.h>
#include <algorithm>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;

namespace
{
/// Tries to split rText into rURL and nPageNumber.
bool SplitUrlAndPage(const OUString& rText, OUString& rUrl, int& nPageNumber)
{
    uno::Reference<uri::XUriReferenceFactory> xUriReferenceFactory
        = uri::UriReferenceFactory::create(comphelper::getProcessComponentContext());
    uno::Reference<uri::XUriReference> xUriRef;
    try
    {
        xUriRef = xUriReferenceFactory->parse(rText);
    }
    catch (const uno::Exception& rException)
    {
        SAL_WARN("extensions.biblio",
                 "SplitUrlAndPage: failed to parse url: " << rException.Message);
        return false;
    }

    OUString aPagePrefix(u"page="_ustr);
    if (!xUriRef->getFragment().startsWith(aPagePrefix))
    {
        return false;
    }

    nPageNumber = o3tl::toInt32(xUriRef->getFragment().subView(aPagePrefix.getLength()));
    xUriRef->clearFragment();
    rUrl = xUriRef->getUriReference();
    return true;
}

/// Merges rUrl and rPageSB to a URL string.
OUString MergeUrlAndPage(const OUString& rUrl, const weld::SpinButton& rPageSB)
{
    if (!rPageSB.get_sensitive())
    {
        return rUrl;
    }

    uno::Reference<uri::XUriReferenceFactory> xUriReferenceFactory
        = uri::UriReferenceFactory::create(comphelper::getProcessComponentContext());
    uno::Reference<uri::XUriReference> xUriRef;
    try
    {
        xUriRef = xUriReferenceFactory->parse(rUrl);
    }
    catch (const uno::Exception& rException)
    {
        SAL_WARN("extensions.biblio",
                 "MergeUrlAndPage: failed to parse url: " << rException.Message);
        return rUrl;
    }

    OUString aFragment("page=" + OUString::number(rPageSB.get_value()));
    xUriRef->setFragment(aFragment);
    return xUriRef->getUriReference();
}
}

static OUString lcl_GetColumnName( const Mapping* pMapping, sal_uInt16 nIndexPos )
{
    BibConfig* pBibConfig = BibModul::GetConfig();
    OUString sRet = pBibConfig->GetDefColumnName(nIndexPos);
    if(pMapping)
        for(const auto & aColumnPair : pMapping->aColumnPairs)
        {
            if(aColumnPair.sLogicalColumnName == sRet)
            {
                sRet = aColumnPair.sRealColumnName;
                break;
            }
        }
    return sRet;
}

BibGeneralPage::BibGeneralPage(vcl::Window* pParent, BibDataManager* pMan)
    : InterimItemWindow(pParent, u"modules/sbibliography/ui/generalpage.ui"_ustr, u"GeneralPage"_ustr)
    , BibShortCutHandler(this)
    , xScrolledWindow(m_xBuilder->weld_scrolled_window(u"scrolledwindow"_ustr))
    , xGrid(m_xBuilder->weld_widget(u"grid"_ustr))
    , xIdentifierFT(m_xBuilder->weld_label(u"shortname"_ustr))
    , xIdentifierED(m_xBuilder->weld_entry(u"shortnamecontrol"_ustr))
    , xAuthTypeFT(m_xBuilder->weld_label(u"authtype"_ustr))
    , xAuthTypeLB(m_xBuilder->weld_combo_box(u"authtypecontrol"_ustr))
    , xYearFT(m_xBuilder->weld_label(u"year"_ustr))
    , xYearED(m_xBuilder->weld_entry(u"yearcontrol"_ustr))
    , xAuthorFT(m_xBuilder->weld_label(u"authors"_ustr))
    , xAuthorED(m_xBuilder->weld_entry(u"authorscontrol"_ustr))
    , xTitleFT(m_xBuilder->weld_label(u"title"_ustr))
    , xTitleED(m_xBuilder->weld_entry(u"titlecontrol"_ustr))
    , xPublisherFT(m_xBuilder->weld_label(u"publisher"_ustr))
    , xPublisherED(m_xBuilder->weld_entry(u"publishercontrol"_ustr))
    , xAddressFT(m_xBuilder->weld_label(u"address"_ustr))
    , xAddressED(m_xBuilder->weld_entry(u"addresscontrol"_ustr))
    , xISBNFT(m_xBuilder->weld_label(u"isbn"_ustr))
    , xISBNED(m_xBuilder->weld_entry(u"isbncontrol"_ustr))
    , xChapterFT(m_xBuilder->weld_label(u"chapter"_ustr))
    , xChapterED(m_xBuilder->weld_entry(u"chaptercontrol"_ustr))
    , xPagesFT(m_xBuilder->weld_label(u"pages"_ustr))
    , xPagesED(m_xBuilder->weld_entry(u"pagescontrol"_ustr))
    , xEditorFT(m_xBuilder->weld_label(u"editor"_ustr))
    , xEditorED(m_xBuilder->weld_entry(u"editorcontrol"_ustr))
    , xEditionFT(m_xBuilder->weld_label(u"edition"_ustr))
    , xEditionED(m_xBuilder->weld_entry(u"editioncontrol"_ustr))
    , xBooktitleFT(m_xBuilder->weld_label(u"booktitle"_ustr))
    , xBooktitleED(m_xBuilder->weld_entry(u"booktitlecontrol"_ustr))
    , xVolumeFT(m_xBuilder->weld_label(u"volume"_ustr))
    , xVolumeED(m_xBuilder->weld_entry(u"volumecontrol"_ustr))
    , xHowpublishedFT(m_xBuilder->weld_label(u"publicationtype"_ustr))
    , xHowpublishedED(m_xBuilder->weld_entry(u"publicationtypecontrol"_ustr))
    , xOrganizationsFT(m_xBuilder->weld_label(u"organization"_ustr))
    , xOrganizationsED(m_xBuilder->weld_entry(u"organizationcontrol"_ustr))
    , xInstitutionFT(m_xBuilder->weld_label(u"institution"_ustr))
    , xInstitutionED(m_xBuilder->weld_entry(u"institutioncontrol"_ustr))
    , xSchoolFT(m_xBuilder->weld_label(u"university"_ustr))
    , xSchoolED(m_xBuilder->weld_entry(u"universitycontrol"_ustr))
    , xReportTypeFT(m_xBuilder->weld_label(u"reporttype"_ustr))
    , xReportTypeED(m_xBuilder->weld_entry(u"reporttypecontrol"_ustr))
    , xMonthFT(m_xBuilder->weld_label(u"month"_ustr))
    , xMonthED(m_xBuilder->weld_entry(u"monthcontrol"_ustr))
    , xJournalFT(m_xBuilder->weld_label(u"journal"_ustr))
    , xJournalED(m_xBuilder->weld_entry(u"journalcontrol"_ustr))
    , xNumberFT(m_xBuilder->weld_label(u"number"_ustr))
    , xNumberED(m_xBuilder->weld_entry(u"numbercontrol"_ustr))
    , xSeriesFT(m_xBuilder->weld_label(u"series"_ustr))
    , xSeriesED(m_xBuilder->weld_entry(u"seriescontrol"_ustr))
    , xAnnoteFT(m_xBuilder->weld_label(u"annotation"_ustr))
    , xAnnoteED(m_xBuilder->weld_entry(u"annotationcontrol"_ustr))
    , xNoteFT(m_xBuilder->weld_label(u"note"_ustr))
    , xNoteED(m_xBuilder->weld_entry(u"notecontrol"_ustr))
    , xURLFT(m_xBuilder->weld_label(u"url"_ustr))
    , xURLED(m_xBuilder->weld_entry(u"urlcontrol"_ustr))
    , xCustom1FT(m_xBuilder->weld_label(u"custom1"_ustr))
    , xCustom1ED(m_xBuilder->weld_entry(u"custom1control"_ustr))
    , xCustom2FT(m_xBuilder->weld_label(u"custom2"_ustr))
    , xCustom2ED(m_xBuilder->weld_entry(u"custom2control"_ustr))
    , xCustom3FT(m_xBuilder->weld_label(u"custom3"_ustr))
    , xCustom3ED(m_xBuilder->weld_entry(u"custom3control"_ustr))
    , xCustom4FT(m_xBuilder->weld_label(u"custom4"_ustr))
    , xCustom4ED(m_xBuilder->weld_entry(u"custom4control"_ustr))
    , xCustom5FT(m_xBuilder->weld_label(u"custom5"_ustr))
    , xCustom5ED(m_xBuilder->weld_entry(u"custom5control"_ustr))
    , m_xLocalURLFT(m_xBuilder->weld_label(u"localurl"_ustr))
    , m_xLocalURLED(m_xBuilder->weld_entry(u"localurlcontrol"_ustr))
    , m_xLocalBrowseButton(m_xBuilder->weld_button(u"localbrowse"_ustr))
    , m_xLocalPageCB(m_xBuilder->weld_check_button(u"localpagecb"_ustr))
    , m_xLocalPageSB(m_xBuilder->weld_spin_button(u"localpagesb"_ustr))
    , pDatMan(pMan)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    xIdentifierED->connect_key_press(LINK(this, BibGeneralPage, FirstElementKeyInputHdl));

    AddControlWithError(lcl_GetColumnName(pMapping, IDENTIFIER_POS),
        xIdentifierFT->get_label(), *xIdentifierED,
        sTableErrorString, HID_BIB_IDENTIFIER_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS),
        xAuthTypeFT->get_label(), *xAuthTypeLB,
        sTableErrorString, HID_BIB_AUTHORITYTYPE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, YEAR_POS),
        xYearFT->get_label(), *xYearED,
        sTableErrorString, HID_BIB_YEAR_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, AUTHOR_POS),
        xAuthorFT->get_label(), *xAuthorED,
        sTableErrorString, HID_BIB_AUTHOR_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, TITLE_POS),
        xTitleFT->get_label(), *xTitleED,
        sTableErrorString, HID_BIB_TITLE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, PUBLISHER_POS),
        xPublisherFT->get_label(), *xPublisherED,
        sTableErrorString, HID_BIB_PUBLISHER_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ADDRESS_POS),
        xAddressFT->get_label(), *xAddressED,
        sTableErrorString, HID_BIB_ADDRESS_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ISBN_POS),
        xISBNFT->get_label(), *xISBNED,
        sTableErrorString, HID_BIB_ISBN_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CHAPTER_POS),
        xChapterFT->get_label(), *xChapterED,
        sTableErrorString, HID_BIB_CHAPTER_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, PAGES_POS),
        xPagesFT->get_label(), *xPagesED,
        sTableErrorString, HID_BIB_PAGES_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, EDITOR_POS),
        xEditorFT->get_label(), *xEditorED,
        sTableErrorString, HID_BIB_EDITOR_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, EDITION_POS),
        xEditionFT->get_label(), *xEditionED,
        sTableErrorString, HID_BIB_EDITION_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, BOOKTITLE_POS),
        xBooktitleFT->get_label(), *xBooktitleED,
        sTableErrorString, HID_BIB_BOOKTITLE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, VOLUME_POS),
        xVolumeFT->get_label(), *xVolumeED,
        sTableErrorString, HID_BIB_VOLUME_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, HOWPUBLISHED_POS),
        xHowpublishedFT->get_label(), *xHowpublishedED,
        sTableErrorString, HID_BIB_HOWPUBLISHED_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ORGANIZATIONS_POS),
        xOrganizationsFT->get_label(), *xOrganizationsED,
        sTableErrorString, HID_BIB_ORGANIZATIONS_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, INSTITUTION_POS),
        xInstitutionFT->get_label(), *xInstitutionED,
        sTableErrorString, HID_BIB_INSTITUTION_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, SCHOOL_POS),
        xSchoolFT->get_label(), *xSchoolED,
        sTableErrorString, HID_BIB_SCHOOL_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, REPORTTYPE_POS),
        xReportTypeFT->get_label(), *xReportTypeED,
        sTableErrorString, HID_BIB_REPORTTYPE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, MONTH_POS),
        xMonthFT->get_label(), *xMonthED,
        sTableErrorString, HID_BIB_MONTH_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, JOURNAL_POS),
        xJournalFT->get_label(), *xJournalED,
        sTableErrorString, HID_BIB_JOURNAL_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, NUMBER_POS),
        xNumberFT->get_label(), *xNumberED,
        sTableErrorString, HID_BIB_NUMBER_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, SERIES_POS),
        xSeriesFT->get_label(), *xSeriesED,
        sTableErrorString, HID_BIB_SERIES_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ANNOTE_POS),
        xAnnoteFT->get_label(), *xAnnoteED,
        sTableErrorString, HID_BIB_ANNOTE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, NOTE_POS),
        xNoteFT->get_label(), *xNoteED,
        sTableErrorString, HID_BIB_NOTE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, URL_POS),
        xURLFT->get_label(), *xURLED,
        sTableErrorString, HID_BIB_URL_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM1_POS),
        xCustom1FT->get_label(), *xCustom1ED,
        sTableErrorString, HID_BIB_CUSTOM1_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM2_POS),
        xCustom2FT->get_label(), *xCustom2ED,
        sTableErrorString, HID_BIB_CUSTOM2_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM3_POS),
        xCustom3FT->get_label(), *xCustom3ED,
        sTableErrorString, HID_BIB_CUSTOM3_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM4_POS),
        xCustom4FT->get_label(), *xCustom4ED,
        sTableErrorString, HID_BIB_CUSTOM4_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM5_POS),
        xCustom5FT->get_label(), *xCustom5ED,
        sTableErrorString, HID_BIB_CUSTOM5_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, LOCAL_URL_POS),
        m_xLocalURLFT->get_label(), *m_xLocalURLED,
        sTableErrorString, HID_BIB_LOCAL_URL_POS);

    m_xLocalBrowseButton->connect_clicked(LINK(this, BibGeneralPage, BrowseHdl));
    m_xLocalPageCB->connect_toggled(LINK(this, BibGeneralPage, PageNumHdl));

    m_xLocalURLED->connect_key_press(LINK(this, BibGeneralPage, LastElementKeyInputHdl));

    if(!sTableErrorString.isEmpty())
        sTableErrorString = BibResId(ST_ERROR_PREFIX) + sTableErrorString;

    SetText(BibResId(ST_TYPE_TITLE));

    Size aSize(LogicToPixel(Size(0, 209), MapMode(MapUnit::MapAppFont)));
    set_height_request(aSize.Height());
}

IMPL_LINK_NOARG(BibGeneralPage, BrowseHdl, weld::Button&, void)
{
    sfx2::FileDialogHelper aFileDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                    FileDialogFlags::NONE, GetFrameWeld());
    OUString aPath = m_xLocalURLED->get_text();
    if (!aPath.isEmpty())
    {
        aFileDlg.SetDisplayDirectory(aPath);
    }
    else
    {
        OUString aBaseURL;
        if (SfxObjectShell* pShell = SfxObjectShell::Current())
        {
            aBaseURL = pShell->getDocumentBaseURL();
        }
        if (!aBaseURL.isEmpty())
        {
            aFileDlg.SetDisplayDirectory(aBaseURL);
        }
    }

    if (aFileDlg.Execute() != ERRCODE_NONE)
    {
        return;
    }

    weld::Entry& rEntry = *m_xLocalURLED;
    rEntry.set_text(aFileDlg.GetPath());
};

IMPL_LINK(BibGeneralPage, PageNumHdl, weld::Toggleable&, rPageCB, void)
{
    weld::SpinButton& rPageSB = *m_xLocalPageSB;
    if (rPageCB.get_active())
    {
        rPageSB.set_sensitive(true);
        rPageSB.set_value(1);
    }
    else
    {
        rPageSB.set_sensitive(false);
    }
}

IMPL_LINK(BibGeneralPage, FirstElementKeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    bool bShift = rKeyEvent.GetKeyCode().IsShift();
    bool bCtrl = rKeyEvent.GetKeyCode().IsMod1();
    bool bAlt = rKeyEvent.GetKeyCode().IsMod2();
    if (KEY_TAB == nCode && bShift && !bCtrl && !bAlt)
    {
        SaveChanges();
        uno::Reference<sdbc::XRowSet> xRowSet(pDatMan->getForm(), UNO_QUERY);
        if (xRowSet.is() && !xRowSet->isFirst())
            xRowSet->previous();
        m_xLocalURLED->grab_focus();
        m_xLocalURLED->select_region(0, -1);
        GainFocusHdl(*m_xLocalURLED);
        return true;
    }
    return false;
}

void BibGeneralPage::SaveChanges()
{
    Reference< XForm > xForm = pDatMan->getForm();
    Reference< beans::XPropertySet > xProps( xForm, UNO_QUERY );
    Reference< sdbc::XResultSetUpdate > xResUpd( xProps, UNO_QUERY );
    if (!xResUpd.is() )
        return;

    Any aModified = xProps->getPropertyValue( u"IsModified"_ustr );
    bool bFlag = false;
    if ( !( aModified >>= bFlag ) || !bFlag )
        return;

    try
    {
        Any aNew = xProps->getPropertyValue( u"IsNew"_ustr );
        aNew >>= bFlag;
        if ( bFlag )
            xResUpd->insertRow();
        else
            xResUpd->updateRow();
    }
    catch( const uno::Exception&) {}
}

IMPL_LINK(BibGeneralPage, LastElementKeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    bool bShift = rKeyEvent.GetKeyCode().IsShift();
    bool bCtrl = rKeyEvent.GetKeyCode().IsMod1();
    bool bAlt = rKeyEvent.GetKeyCode().IsMod2();
    if (KEY_TAB != nCode || bShift || bCtrl || bAlt)
        return false;
    SaveChanges();
    uno::Reference<sdbc::XRowSet> xRowSet(pDatMan->getForm(), UNO_QUERY);
    if (xRowSet.is())
    {
        if (xRowSet->isLast())
        {
            uno::Reference<sdbc::XResultSetUpdate> xUpdateCursor(xRowSet, UNO_QUERY);
            if (xUpdateCursor.is())
                xUpdateCursor->moveToInsertRow();
        }
        else
            (void)xRowSet->next();
    }
    xIdentifierED->grab_focus();
    xIdentifierED->select_region(0, -1);
    GainFocusHdl(*xIdentifierED);
    return true;
}

BibGeneralPage::~BibGeneralPage()
{
    disposeOnce();
}

class ChangeListener : public cppu::WeakImplHelper<css::beans::XPropertyChangeListener>
{
public:
    explicit ChangeListener(css::uno::Reference<css::beans::XPropertySet> xPropSet)
        : m_xPropSet(std::move(xPropSet))
        , m_bSelfChanging(false)
    {
    }

    virtual void SAL_CALL disposing(lang::EventObject const &) override
    {
    }

    virtual void start() = 0;
    virtual void stop()
    {
        WriteBack();
    }

    virtual void WriteBack() = 0;

protected:
    css::uno::Reference<css::beans::XPropertySet> m_xPropSet;
    bool m_bSelfChanging;
};

namespace
{
    class EntryChangeListener : public ChangeListener
    {
    public:
        explicit EntryChangeListener(weld::Entry& rEntry, const css::uno::Reference<css::beans::XPropertySet>& rPropSet,
                                     BibGeneralPage& rPage)
            : ChangeListener(rPropSet)
            , m_rEntry(rEntry)
            , m_rPage(rPage)
        {
            rEntry.connect_focus_out(LINK(this, EntryChangeListener, LoseFocusHdl));
            setValue(rPropSet->getPropertyValue(u"Text"_ustr));
        }

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override
        {
            if (m_bSelfChanging)
                return;
            setValue(evt.NewValue);
        }

        virtual void start() override
        {
            m_xPropSet->addPropertyChangeListener(u"Text"_ustr, this);
        }

        virtual void stop() override
        {
            m_xPropSet->removePropertyChangeListener(u"Text"_ustr, this);
            ChangeListener::stop();
        }

    private:
        weld::Entry& m_rEntry;
        BibGeneralPage& m_rPage;

        DECL_LINK(LoseFocusHdl, weld::Widget&, void);

        /// Updates the UI widget(s) based on rValue.
        void setValue(const css::uno::Any& rValue)
        {
            OUString sNewName;
            rValue >>= sNewName;
            if (&m_rEntry == &m_rPage.GetLocalURLED())
            {
                OUString aUrl;
                int nPageNumber;
                if (SplitUrlAndPage(sNewName, aUrl, nPageNumber))
                {
                    m_rEntry.set_text(aUrl);
                    m_rPage.GetLocalPageCB().set_active(true);
                    m_rPage.GetLocalPageSB().set_sensitive(true);
                    m_rPage.GetLocalPageSB().set_value(nPageNumber);
                }
                else
                {
                    m_rEntry.set_text(sNewName);
                    m_rPage.GetLocalPageCB().set_active(false);
                    m_rPage.GetLocalPageSB().set_sensitive(false);
                    m_rPage.GetLocalPageSB().set_value(0);
                }
            }
            else
            {
                m_rEntry.set_text(sNewName);
            }

            m_rEntry.save_value();
            if (&m_rEntry == &m_rPage.GetLocalURLED())
            {
                m_rPage.GetLocalPageSB().save_value();
            }
        }

        /// Updates m_xPropSet based on the UI widget(s).
        virtual void WriteBack() override
        {
            bool bLocalURL = &m_rEntry == &m_rPage.GetLocalURLED()
                        && m_rPage.GetLocalPageSB().get_value_changed_from_saved();
            if (!m_rEntry.get_value_changed_from_saved() && !bLocalURL)
                return;

            m_bSelfChanging = true;

            OUString aText;
            if (&m_rEntry == &m_rPage.GetLocalURLED())
            {
                aText = MergeUrlAndPage(m_rEntry.get_text(), m_rPage.GetLocalPageSB());
            }
            else
            {
                aText = m_rEntry.get_text();
            }
            m_xPropSet->setPropertyValue(u"Text"_ustr, Any(aText));

            css::uno::Reference<css::form::XBoundComponent> xBound(m_xPropSet, css::uno::UNO_QUERY);
            if (xBound.is())
                xBound->commit();

            m_bSelfChanging = false;
            m_rEntry.save_value();
            if (&m_rEntry == &m_rPage.GetLocalURLED())
            {
                m_rPage.GetLocalPageSB().save_value();
            }
        }

    };

    IMPL_LINK_NOARG(EntryChangeListener, LoseFocusHdl, weld::Widget&, void)
    {
        WriteBack();
    }

    class ComboBoxChangeListener : public ChangeListener
    {
    public:
        explicit ComboBoxChangeListener(weld::ComboBox& rComboBox, const css::uno::Reference<css::beans::XPropertySet>& rPropSet)
            : ChangeListener(rPropSet)
            , m_rComboBox(rComboBox)
        {
            rComboBox.connect_changed(LINK(this, ComboBoxChangeListener, ChangeHdl));
            setValue(rPropSet->getPropertyValue(u"SelectedItems"_ustr));
        }

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override
        {
            if (m_bSelfChanging)
                return;
            setValue(evt.NewValue);
        }

        virtual void start() override
        {
            m_xPropSet->addPropertyChangeListener(u"SelectedItems"_ustr, this);
        }

        virtual void stop() override
        {
            m_xPropSet->removePropertyChangeListener(u"SelectedItems"_ustr, this);
            ChangeListener::stop();
        }

    private:
        weld::ComboBox& m_rComboBox;

        DECL_LINK(ChangeHdl, weld::ComboBox&, void);

        void setValue(const css::uno::Any& rValue)
        {
            sal_Int16 nSelection = -1;
            Sequence<sal_Int16> aSelection;
            rValue >>= aSelection;
            if (aSelection.hasElements())
                nSelection = aSelection[0];

            m_rComboBox.set_active(nSelection);
            m_rComboBox.save_value();
        }

        virtual void WriteBack() override
        {
            if (!m_rComboBox.get_value_changed_from_saved())
                return;
            m_bSelfChanging = true;

            Sequence<sal_Int16> aSelection{ o3tl::narrowing<sal_Int16>(m_rComboBox.get_active()) };
            m_xPropSet->setPropertyValue(u"SelectedItems"_ustr, Any(aSelection));

            css::uno::Reference<css::form::XBoundComponent> xBound(m_xPropSet, css::uno::UNO_QUERY);
            if (xBound.is())
                xBound->commit();

            m_bSelfChanging = false;
            m_rComboBox.save_value();
        }
    };

    IMPL_LINK_NOARG(ComboBoxChangeListener, ChangeHdl, weld::ComboBox&, void)
    {
        WriteBack();
    }
}

void BibGeneralPage::dispose()
{
    for (auto& listener : maChangeListeners)
        listener->stop();
    maChangeListeners.clear();

    SaveChanges();

    xScrolledWindow.reset();
    xGrid.reset();
    xIdentifierFT.reset();
    xIdentifierED.reset();
    xAuthTypeFT.reset();
    xAuthTypeLB.reset();
    xYearFT.reset();
    xYearED.reset();
    xAuthorFT.reset();
    xAuthorED.reset();
    xTitleFT.reset();
    xTitleED.reset();
    xPublisherFT.reset();
    xPublisherED.reset();
    xAddressFT.reset();
    xAddressED.reset();
    xISBNFT.reset();
    xISBNED.reset();
    xChapterFT.reset();
    xChapterED.reset();
    xPagesFT.reset();
    xPagesED.reset();
    xEditorFT.reset();
    xEditorED.reset();
    xEditionFT.reset();
    xEditionED.reset();
    xBooktitleFT.reset();
    xBooktitleED.reset();
    xVolumeFT.reset();
    xVolumeED.reset();
    xHowpublishedFT.reset();
    xHowpublishedED.reset();
    xOrganizationsFT.reset();
    xOrganizationsED.reset();
    xInstitutionFT.reset();
    xInstitutionED.reset();
    xSchoolFT.reset();
    xSchoolED.reset();
    xReportTypeFT.reset();
    xReportTypeED.reset();
    xMonthFT.reset();
    xMonthED.reset();
    xJournalFT.reset();
    xJournalED.reset();
    xNumberFT.reset();
    xNumberED.reset();
    xSeriesFT.reset();
    xSeriesED.reset();
    xAnnoteFT.reset();
    xAnnoteED.reset();
    xNoteFT.reset();
    xNoteED.reset();
    xURLFT.reset();
    xURLED.reset();
    xCustom1FT.reset();
    xCustom1ED.reset();
    xCustom2FT.reset();
    xCustom2ED.reset();
    xCustom3FT.reset();
    xCustom3ED.reset();
    xCustom4FT.reset();
    xCustom4ED.reset();
    xCustom5FT.reset();
    xCustom5ED.reset();
    m_xLocalURLFT.reset();
    m_xLocalURLED.reset();
    m_xLocalBrowseButton.reset();
    m_xLocalPageCB.reset();
    m_xLocalPageSB.reset();
    InterimItemWindow::dispose();
}

weld::Entry& BibGeneralPage::GetLocalURLED() { return *m_xLocalURLED; }

weld::CheckButton& BibGeneralPage::GetLocalPageCB() { return *m_xLocalPageCB; }

weld::SpinButton& BibGeneralPage::GetLocalPageSB() { return *m_xLocalPageSB; }

bool BibGeneralPage::AddXControl(const OUString& rName, weld::Entry& rEntry)
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        xCtrModel = pDatMan->loadControlModel(rName, false);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                maChangeListeners.emplace_back(new EntryChangeListener(rEntry, xPropSet, *this));
                maChangeListeners.back()->start();
                if (&rEntry == m_xLocalURLED.get())
                {
                    m_aURLListener = maChangeListeners.back();
                    m_xLocalPageSB->connect_focus_out(LINK(this, BibGeneralPage, LosePageFocusHdl));
                }
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel.is();
}

IMPL_LINK_NOARG(BibGeneralPage, LosePageFocusHdl, weld::Widget&, void)
{
    m_aURLListener->WriteBack();
}

IMPL_LINK(BibGeneralPage, GainFocusHdl, weld::Widget&, rWidget, void)
{
    int x, y, width, height;
    if (!rWidget.get_extents_relative_to(*xGrid, x, y, width, height))
        return;

    int bottom = y + height;
    int nVScrollPos = xScrolledWindow->vadjustment_get_value();
    if (y < nVScrollPos || bottom > nVScrollPos + xScrolledWindow->vadjustment_get_page_size())
        xScrolledWindow->vadjustment_set_value(y);

    int right = x + width;
    int nHScrollPos = xScrolledWindow->hadjustment_get_value();
    if (x < nHScrollPos || right > nHScrollPos + xScrolledWindow->hadjustment_get_page_size())
        xScrolledWindow->hadjustment_set_value(x);
}

template<class Target> void BibGeneralPage::AddControlWithError(const OUString& rColumnName, const OUString& rColumnUIName,
    Target& rWidget, OUString& rErrorString, const OUString& rHelpId)
{
    rWidget.set_help_id(rHelpId);
    rWidget.connect_focus_in(LINK(this, BibGeneralPage, GainFocusHdl));
    bool bSuccess = AddXControl(rColumnName, rWidget);
    if (!bSuccess)
    {
        if( !rErrorString.isEmpty() )
            rErrorString += "\n";

        rErrorString += MnemonicGenerator::EraseAllMnemonicChars(rColumnUIName);
    }
}

bool BibGeneralPage::AddXControl(const OUString& rName, weld::ComboBox& rList)
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        xCtrModel = pDatMan->loadControlModel(rName, true);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                css::uno::Sequence<OUString> aEntries;
                xPropSet->getPropertyValue(u"StringItemList"_ustr) >>= aEntries;
                for (const OUString& rString : aEntries)
                     rList.append_text(rString);

                sal_Int16 nSelection = -1;
                Sequence<sal_Int16> aSelection;
                xPropSet->getPropertyValue(u"SelectedItems"_ustr) >>= aSelection;
                if (aSelection.hasElements())
                    nSelection = aSelection[0];

                rList.set_active(nSelection);
                rList.save_value();

                maChangeListeners.emplace_back(new ComboBoxChangeListener(rList, xPropSet));
                maChangeListeners.back()->start();
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel.is();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

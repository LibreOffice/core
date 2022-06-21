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

    OUString aPagePrefix("page=");
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
    : InterimItemWindow(pParent, "modules/sbibliography/ui/generalpage.ui", "GeneralPage")
    , BibShortCutHandler(this)
    , xScrolledWindow(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , xGrid(m_xBuilder->weld_widget("grid"))
    , xIdentifierFT(m_xBuilder->weld_label("shortname"))
    , xIdentifierED(m_xBuilder->weld_entry("shortnamecontrol"))
    , xAuthTypeFT(m_xBuilder->weld_label("authtype"))
    , xAuthTypeLB(m_xBuilder->weld_combo_box("authtypecontrol"))
    , xYearFT(m_xBuilder->weld_label("year"))
    , xYearED(m_xBuilder->weld_entry("yearcontrol"))
    , xAuthorFT(m_xBuilder->weld_label("authors"))
    , xAuthorED(m_xBuilder->weld_entry("authorscontrol"))
    , xTitleFT(m_xBuilder->weld_label("title"))
    , xTitleED(m_xBuilder->weld_entry("titlecontrol"))
    , xPublisherFT(m_xBuilder->weld_label("publisher"))
    , xPublisherED(m_xBuilder->weld_entry("publishercontrol"))
    , xAddressFT(m_xBuilder->weld_label("address"))
    , xAddressED(m_xBuilder->weld_entry("addresscontrol"))
    , xISBNFT(m_xBuilder->weld_label("isbn"))
    , xISBNED(m_xBuilder->weld_entry("isbncontrol"))
    , xChapterFT(m_xBuilder->weld_label("chapter"))
    , xChapterED(m_xBuilder->weld_entry("chaptercontrol"))
    , xPagesFT(m_xBuilder->weld_label("pages"))
    , xPagesED(m_xBuilder->weld_entry("pagescontrol"))
    , xEditorFT(m_xBuilder->weld_label("editor"))
    , xEditorED(m_xBuilder->weld_entry("editorcontrol"))
    , xEditionFT(m_xBuilder->weld_label("edition"))
    , xEditionED(m_xBuilder->weld_entry("editioncontrol"))
    , xBooktitleFT(m_xBuilder->weld_label("booktitle"))
    , xBooktitleED(m_xBuilder->weld_entry("booktitlecontrol"))
    , xVolumeFT(m_xBuilder->weld_label("volume"))
    , xVolumeED(m_xBuilder->weld_entry("volumecontrol"))
    , xHowpublishedFT(m_xBuilder->weld_label("publicationtype"))
    , xHowpublishedED(m_xBuilder->weld_entry("publicationtypecontrol"))
    , xOrganizationsFT(m_xBuilder->weld_label("organization"))
    , xOrganizationsED(m_xBuilder->weld_entry("organizationcontrol"))
    , xInstitutionFT(m_xBuilder->weld_label("institution"))
    , xInstitutionED(m_xBuilder->weld_entry("institutioncontrol"))
    , xSchoolFT(m_xBuilder->weld_label("university"))
    , xSchoolED(m_xBuilder->weld_entry("universitycontrol"))
    , xReportTypeFT(m_xBuilder->weld_label("reporttype"))
    , xReportTypeED(m_xBuilder->weld_entry("reporttypecontrol"))
    , xMonthFT(m_xBuilder->weld_label("month"))
    , xMonthED(m_xBuilder->weld_entry("monthcontrol"))
    , xJournalFT(m_xBuilder->weld_label("journal"))
    , xJournalED(m_xBuilder->weld_entry("journalcontrol"))
    , xNumberFT(m_xBuilder->weld_label("number"))
    , xNumberED(m_xBuilder->weld_entry("numbercontrol"))
    , xSeriesFT(m_xBuilder->weld_label("series"))
    , xSeriesED(m_xBuilder->weld_entry("seriescontrol"))
    , xAnnoteFT(m_xBuilder->weld_label("annotation"))
    , xAnnoteED(m_xBuilder->weld_entry("annotationcontrol"))
    , xNoteFT(m_xBuilder->weld_label("note"))
    , xNoteED(m_xBuilder->weld_entry("notecontrol"))
    , xURLFT(m_xBuilder->weld_label("url"))
    , xURLED(m_xBuilder->weld_entry("urlcontrol"))
    , xCustom1FT(m_xBuilder->weld_label("custom1"))
    , xCustom1ED(m_xBuilder->weld_entry("custom1control"))
    , xCustom2FT(m_xBuilder->weld_label("custom2"))
    , xCustom2ED(m_xBuilder->weld_entry("custom2control"))
    , xCustom3FT(m_xBuilder->weld_label("custom3"))
    , xCustom3ED(m_xBuilder->weld_entry("custom3control"))
    , xCustom4FT(m_xBuilder->weld_label("custom4"))
    , xCustom4ED(m_xBuilder->weld_entry("custom4control"))
    , xCustom5FT(m_xBuilder->weld_label("custom5"))
    , xCustom5ED(m_xBuilder->weld_entry("custom5control"))
    , m_xLocalURLFT(m_xBuilder->weld_label("localurl"))
    , m_xLocalURLED(m_xBuilder->weld_entry("localurlcontrol"))
    , m_xLocalBrowseButton(m_xBuilder->weld_button("localbrowse"))
    , m_xLocalPageCB(m_xBuilder->weld_check_button("localpagecb"))
    , m_xLocalPageSB(m_xBuilder->weld_spin_button("localpagesb"))
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
        SfxObjectShell* pShell = SfxObjectShell::Current();
        OUString aBaseURL;
        if (pShell)
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

    Any aModified = xProps->getPropertyValue( "IsModified" );
    bool bFlag = false;
    if ( !( aModified >>= bFlag ) || !bFlag )
        return;

    try
    {
        Any aNew = xProps->getPropertyValue( "IsNew" );
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
            setValue(rPropSet->getPropertyValue("Text"));
        }

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override
        {
            if (m_bSelfChanging)
                return;
            setValue(evt.NewValue);
        }

        virtual void start() override
        {
            m_xPropSet->addPropertyChangeListener("Text", this);
        }

        virtual void stop() override
        {
            m_xPropSet->removePropertyChangeListener("Text", this);
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
            m_xPropSet->setPropertyValue("Text", Any(aText));

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
            setValue(rPropSet->getPropertyValue("SelectedItems"));
        }

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override
        {
            if (m_bSelfChanging)
                return;
            setValue(evt.NewValue);
        }

        virtual void start() override
        {
            m_xPropSet->addPropertyChangeListener("SelectedItems", this);
        }

        virtual void stop() override
        {
            m_xPropSet->removePropertyChangeListener("SelectedItems", this);
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
            m_xPropSet->setPropertyValue("SelectedItems", Any(aSelection));

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
    Target& rWidget, OUString& rErrorString, const OString& rHelpId)
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
                xPropSet->getPropertyValue("StringItemList") >>= aEntries;
                for (const OUString& rString : std::as_const(aEntries))
                     rList.append_text(rString);

                sal_Int16 nSelection = -1;
                Sequence<sal_Int16> aSelection;
                xPropSet->getPropertyValue("SelectedItems") >>= aSelection;
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

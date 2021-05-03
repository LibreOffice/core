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

#include "groupboxwiz.hxx"
#include "commonpagesdbp.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "optiongrouplayouter.hxx"
#include <helpids.h>
#include <o3tl/safeint.hxx>

#define GBW_STATE_OPTIONLIST        0
#define GBW_STATE_DEFAULTOPTION     1
#define GBW_STATE_OPTIONVALUES      2
#define GBW_STATE_DBFIELD           3
#define GBW_STATE_FINALIZE          4

namespace dbp
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;

    OGroupBoxWizard::OGroupBoxWizard(weld::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        : OControlWizard(_pParent, _rxObjectModel, _rxContext)
        , m_bVisitedDefault(false)
        , m_bVisitedDB(false)
    {
        initControlSettings(&m_aSettings);

        m_xPrevPage->set_help_id(HID_GROUPWIZARD_PREVIOUS);
        m_xNextPage->set_help_id(HID_GROUPWIZARD_NEXT);
        m_xCancel->set_help_id(HID_GROUPWIZARD_CANCEL);
        m_xFinish->set_help_id(HID_GROUPWIZARD_FINISH);
        setTitleBase(compmodule::ModuleRes(RID_STR_GROUPWIZARD_TITLE));
    }

    bool OGroupBoxWizard::approveControl(sal_Int16 _nClassId)
    {
        return FormComponentType::GROUPBOX == _nClassId;
    }

    std::unique_ptr<BuilderPage> OGroupBoxWizard::createPage(::vcl::WizardTypes::WizardState _nState)
    {
        OString sIdent(OString::number(_nState));
        weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

        switch (_nState)
        {
            case GBW_STATE_OPTIONLIST:
                return std::make_unique<ORadioSelectionPage>(pPageContainer, this);

            case GBW_STATE_DEFAULTOPTION:
                return std::make_unique<ODefaultFieldSelectionPage>(pPageContainer, this);

            case GBW_STATE_OPTIONVALUES:
                return std::make_unique<OOptionValuesPage>(pPageContainer, this);

            case GBW_STATE_DBFIELD:
                return std::make_unique<OOptionDBFieldPage>(pPageContainer, this);

            case GBW_STATE_FINALIZE:
                return std::make_unique<OFinalizeGBWPage>(pPageContainer, this);
        }

        return nullptr;
    }

    vcl::WizardTypes::WizardState OGroupBoxWizard::determineNextState( ::vcl::WizardTypes::WizardState _nCurrentState ) const
    {
        switch (_nCurrentState)
        {
            case GBW_STATE_OPTIONLIST:
                return GBW_STATE_DEFAULTOPTION;

            case GBW_STATE_DEFAULTOPTION:
                return GBW_STATE_OPTIONVALUES;

            case GBW_STATE_OPTIONVALUES:
                if (getContext().aFieldNames.hasElements())
                    return GBW_STATE_DBFIELD;
                else
                    return GBW_STATE_FINALIZE;

            case GBW_STATE_DBFIELD:
                return GBW_STATE_FINALIZE;
        }

        return WZS_INVALID_STATE;
    }

    void OGroupBoxWizard::enterState(::vcl::WizardTypes::WizardState _nState)
    {
        // some stuff to do before calling the base class (modifying our settings)
        switch (_nState)
        {
            case GBW_STATE_DEFAULTOPTION:
                if (!m_bVisitedDefault)
                {   // assume that the first of the radio buttons should be selected
                    DBG_ASSERT(m_aSettings.aLabels.size(), "OGroupBoxWizard::enterState: should never have reached this state!");
                    m_aSettings.sDefaultField = m_aSettings.aLabels[0];
                }
                m_bVisitedDefault = true;
                break;

            case GBW_STATE_DBFIELD:
                if (!m_bVisitedDB)
                {   // try to generate a default for the DB field
                    // (simply use the first field in the DB names collection)
                    if (getContext().aFieldNames.hasElements())
                        m_aSettings.sDBField = getContext().aFieldNames[0];
                }
                m_bVisitedDB = true;
                break;
        }

        // setting the def button... to be done before the base class is called, too, 'cause the base class
        // calls the pages, which are allowed to override our def button behaviour
        defaultButton(GBW_STATE_FINALIZE == _nState ? WizardButtonFlags::FINISH : WizardButtonFlags::NEXT);

        // allow "finish" on the last page only
        enableButtons(WizardButtonFlags::FINISH, GBW_STATE_FINALIZE == _nState);
        // allow previous on all pages but the first one
        enableButtons(WizardButtonFlags::PREVIOUS, GBW_STATE_OPTIONLIST != _nState);
        // allow next on all pages but the last one
        enableButtons(WizardButtonFlags::NEXT, GBW_STATE_FINALIZE != _nState);

        OControlWizard::enterState(_nState);
    }

    bool OGroupBoxWizard::onFinish()
    {
        // commit the basic control settings
        commitControlSettings(&m_aSettings);

        // create the radio buttons
        try
        {
            OOptionGroupLayouter aLayouter( getComponentContext() );
            aLayouter.doLayout(getContext(), getSettings());
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.dbpilots",
                                 "caught an exception while creating the radio shapes!");
        }

        return OControlWizard::onFinish();
    }

    ORadioSelectionPage::ORadioSelectionPage(weld::Container* pPage, OControlWizard* pWizard)
        : OGBWPage(pPage, pWizard, "modules/sabpilot/ui/groupradioselectionpage.ui", "GroupRadioSelectionPage")
        , m_xRadioName(m_xBuilder->weld_entry("radiolabels"))
        , m_xMoveRight(m_xBuilder->weld_button("toright"))
        , m_xMoveLeft(m_xBuilder->weld_button("toleft"))
        , m_xExistingRadios(m_xBuilder->weld_tree_view("radiobuttons"))
    {
        if (getContext().aFieldNames.hasElements())
        {
            enableFormDatasourceDisplay();
        }

        m_xMoveLeft->connect_clicked(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_xMoveRight->connect_clicked(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_xRadioName->connect_changed(LINK(this, ORadioSelectionPage, OnNameModified));
        m_xExistingRadios->connect_changed(LINK(this, ORadioSelectionPage, OnEntrySelected));

        implCheckMoveButtons();
        m_xExistingRadios->set_selection_mode(SelectionMode::Multiple);

        getDialog()->defaultButton(m_xMoveRight.get());
    }

    ORadioSelectionPage::~ORadioSelectionPage()
    {
    }

    void ORadioSelectionPage::Activate()
    {
        OGBWPage::Activate();
        m_xRadioName->grab_focus();
    }

    void ORadioSelectionPage::initializePage()
    {
        OGBWPage::initializePage();

        m_xRadioName->set_text("");

        // no need to initialize the list of radios here
        // (we're the only one affecting this special setting, so it will be in the same state as last time this
        // page was committed)

        implCheckMoveButtons();
    }

    bool ORadioSelectionPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return false;

        // copy the names of the radio buttons to be inserted
        // and initialize the values
        OOptionGroupSettings& rSettings = getSettings();
        rSettings.aLabels.clear();
        rSettings.aValues.clear();
        rSettings.aLabels.reserve(m_xExistingRadios->n_children());
        rSettings.aValues.reserve(m_xExistingRadios->n_children());
        for (sal_Int32 i=0; i<m_xExistingRadios->n_children(); ++i)
        {
            rSettings.aLabels.push_back(m_xExistingRadios->get_text(i));
            rSettings.aValues.push_back(OUString::number(i + 1));
        }

        return true;
    }

    IMPL_LINK( ORadioSelectionPage, OnMoveEntry, weld::Button&, rButton, void )
    {
        bool bMoveLeft = (m_xMoveLeft.get() == &rButton);
        if (bMoveLeft)
        {
            while (m_xExistingRadios->count_selected_rows())
                m_xExistingRadios->remove(m_xExistingRadios->get_selected_index());
        }
        else
        {
            m_xExistingRadios->append_text(m_xRadioName->get_text());
            m_xRadioName->set_text("");
        }

        implCheckMoveButtons();

        //adjust the focus
        if (bMoveLeft)
            m_xExistingRadios->grab_focus();
        else
            m_xRadioName->grab_focus();
    }

    IMPL_LINK_NOARG( ORadioSelectionPage, OnEntrySelected, weld::TreeView&, void )
    {
        implCheckMoveButtons();
    }

    IMPL_LINK_NOARG( ORadioSelectionPage, OnNameModified, weld::Entry&, void )
    {
        implCheckMoveButtons();
    }

    bool ORadioSelectionPage::canAdvance() const
    {
        return 0 != m_xExistingRadios->n_children();
    }

    void ORadioSelectionPage::implCheckMoveButtons()
    {
        bool bHaveSome = (0 != m_xExistingRadios->n_children());
        bool bSelectedSome = (0 != m_xExistingRadios->count_selected_rows());
        bool bUnfinishedInput = !m_xRadioName->get_text().isEmpty();

        m_xMoveLeft->set_sensitive(bSelectedSome);
        m_xMoveRight->set_sensitive(bUnfinishedInput);

        getDialog()->enableButtons(WizardButtonFlags::NEXT, bHaveSome);

        if (bUnfinishedInput)
        {
            if (!m_xMoveRight->get_has_default())
                getDialog()->defaultButton(m_xMoveRight.get());
        }
        else
        {
            if (m_xMoveRight->get_has_default())
                getDialog()->defaultButton(WizardButtonFlags::NEXT);
        }
    }

    ODefaultFieldSelectionPage::ODefaultFieldSelectionPage(weld::Container* pPage, OControlWizard* pWizard)
        : OMaybeListSelectionPage(pPage, pWizard, "modules/sabpilot/ui/defaultfieldselectionpage.ui", "DefaultFieldSelectionPage")
        , m_xDefSelYes(m_xBuilder->weld_radio_button("defaultselectionyes"))
        , m_xDefSelNo(m_xBuilder->weld_radio_button("defaultselectionno"))
        , m_xDefSelection(m_xBuilder->weld_combo_box("defselectionfield"))
    {
        announceControls(*m_xDefSelYes, *m_xDefSelNo, *m_xDefSelection);
    }

    ODefaultFieldSelectionPage::~ODefaultFieldSelectionPage()
    {
    }

    void ODefaultFieldSelectionPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();

        // fill the listbox
        m_xDefSelection->clear();
        for (auto const& label : rSettings.aLabels)
            m_xDefSelection->append_text(label);

        implInitialize(rSettings.sDefaultField);
    }

    bool ODefaultFieldSelectionPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return false;

        OOptionGroupSettings& rSettings = getSettings();
        implCommit(rSettings.sDefaultField);

        return true;
    }

    OOptionValuesPage::OOptionValuesPage(weld::Container* pPage, OControlWizard* pWizard)
        : OGBWPage(pPage, pWizard, "modules/sabpilot/ui/optionvaluespage.ui", "OptionValuesPage")
        , m_xValue(m_xBuilder->weld_entry("optionvalue"))
        , m_xOptions(m_xBuilder->weld_tree_view("radiobuttons"))
        , m_nLastSelection(::vcl::WizardTypes::WizardState(-1))
    {
       m_xOptions->connect_changed(LINK(this, OOptionValuesPage, OnOptionSelected));
    }

    OOptionValuesPage::~OOptionValuesPage()
    {
    }

    IMPL_LINK_NOARG( OOptionValuesPage, OnOptionSelected, weld::TreeView&, void )
    {
        implTraveledOptions();
    }

    void OOptionValuesPage::Activate()
    {
        OGBWPage::Activate();
        m_xValue->grab_focus();
    }

    void OOptionValuesPage::implTraveledOptions()
    {
        if (::vcl::WizardTypes::WizardState(-1) != m_nLastSelection)
        {
            // save the value for the last option
            DBG_ASSERT(o3tl::make_unsigned(m_nLastSelection) < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid previous selection index!");
            m_aUncommittedValues[m_nLastSelection] = m_xValue->get_text();
        }

        m_nLastSelection = m_xOptions->get_selected_index();
        DBG_ASSERT(o3tl::make_unsigned(m_nLastSelection) < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid new selection index!");
        m_xValue->set_text(m_aUncommittedValues[m_nLastSelection]);
    }

    void OOptionValuesPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        DBG_ASSERT(rSettings.aLabels.size(), "OOptionValuesPage::initializePage: no options!!");
        DBG_ASSERT(rSettings.aLabels.size() == rSettings.aValues.size(), "OOptionValuesPage::initializePage: inconsistent data!");

        // fill the list with all available options
        m_xOptions->clear();
        m_nLastSelection = -1;
        for (auto const& label : rSettings.aLabels)
            m_xOptions->append_text(label);

        // remember the values ... can't set them directly in the settings without the explicit commit call
        // so we need have a copy of the values
        m_aUncommittedValues = rSettings.aValues;

        // select the first entry
        m_xOptions->select(0);
        implTraveledOptions();
    }

    bool OOptionValuesPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return false;

        OOptionGroupSettings& rSettings = getSettings();

        // commit the current value
        implTraveledOptions();
        // copy the uncommitted values
        rSettings.aValues = m_aUncommittedValues;

        return true;
    }

    OOptionDBFieldPage::OOptionDBFieldPage(weld::Container* pPage, OControlWizard* pWizard)
        : ODBFieldPage(pPage, pWizard)
    {
        setDescriptionText(compmodule::ModuleRes(RID_STR_GROUPWIZ_DBFIELD));
    }

    OUString& OOptionDBFieldPage::getDBFieldSetting()
    {
        return static_cast<OGroupBoxWizard*>(getDialog())->getSettings().sDBField;
    }

    OFinalizeGBWPage::OFinalizeGBWPage(weld::Container* pPage, OControlWizard* pWizard)
        : OGBWPage(pPage, pWizard, "modules/sabpilot/ui/optionsfinalpage.ui", "OptionsFinalPage")
        , m_xName(m_xBuilder->weld_entry("nameit"))
    {
    }

    OFinalizeGBWPage::~OFinalizeGBWPage()
    {
    }

    void OFinalizeGBWPage::Activate()
    {
        OGBWPage::Activate();
        m_xName->grab_focus();
    }

    bool OFinalizeGBWPage::canAdvance() const
    {
        return false;
    }

    void OFinalizeGBWPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        m_xName->set_text(rSettings.sControlLabel);
    }

    bool OFinalizeGBWPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return false;

        getSettings().sControlLabel = m_xName->get_text();

        return true;
    }

}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

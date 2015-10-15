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
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include "optiongrouplayouter.hxx"
#include "dbpilots.hrc"
#include <comphelper/processfactory.hxx>

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
    using namespace ::svt;

    OGroupBoxWizard::OGroupBoxWizard( vcl::Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XComponentContext >& _rxContext )
        :OControlWizard(_pParent, _rxObjectModel, _rxContext)
        ,m_bVisitedDefault(false)
        ,m_bVisitedDB(false)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_GROUPWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_GROUPWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_GROUPWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_GROUPWIZARD_FINISH);
        setTitleBase(ModuleRes(RID_STR_GROUPWIZARD_TITLE).toString());
    }


    bool OGroupBoxWizard::approveControl(sal_Int16 _nClassId)
    {
        return FormComponentType::GROUPBOX == _nClassId;
    }


    VclPtr<TabPage> OGroupBoxWizard::createPage(::svt::WizardTypes::WizardState _nState)
    {
        switch (_nState)
        {
            case GBW_STATE_OPTIONLIST:
                return VclPtr<ORadioSelectionPage>::Create(this);

            case GBW_STATE_DEFAULTOPTION:
                return VclPtr<ODefaultFieldSelectionPage>::Create(this);

            case GBW_STATE_OPTIONVALUES:
                return VclPtr<OOptionValuesPage>::Create(this);

            case GBW_STATE_DBFIELD:
                return VclPtr<OOptionDBFieldPage>::Create(this);

            case GBW_STATE_FINALIZE:
                return VclPtr<OFinalizeGBWPage>::Create(this);
        }

        return NULL;
    }


    WizardTypes::WizardState OGroupBoxWizard::determineNextState( ::svt::WizardTypes::WizardState _nCurrentState ) const
    {
        switch (_nCurrentState)
        {
            case GBW_STATE_OPTIONLIST:
                return GBW_STATE_DEFAULTOPTION;

            case GBW_STATE_DEFAULTOPTION:
                return GBW_STATE_OPTIONVALUES;

            case GBW_STATE_OPTIONVALUES:
                if (getContext().aFieldNames.getLength())
                    return GBW_STATE_DBFIELD;
                else
                    return GBW_STATE_FINALIZE;

            case GBW_STATE_DBFIELD:
                return GBW_STATE_FINALIZE;
        }

        return WZS_INVALID_STATE;
    }


    void OGroupBoxWizard::enterState(::svt::WizardTypes::WizardState _nState)
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
                    if (getContext().aFieldNames.getLength())
                        m_aSettings.sDBField = getContext().aFieldNames[0];
                }
                m_bVisitedDB = true;
                break;
        }

        // setting the def button .... to be done before the base class is called, too, 'cause the base class
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


    void OGroupBoxWizard::createRadios()
    {
        try
        {
            OOptionGroupLayouter aLayouter( getComponentContext() );
            aLayouter.doLayout(getContext(), getSettings());
        }
        catch(const Exception&)
        {
            OSL_FAIL("OGroupBoxWizard::createRadios: caught an exception while creating the radio shapes!");
        }
    }


    bool OGroupBoxWizard::onFinish()
    {
        // commit the basic control setttings
        commitControlSettings(&m_aSettings);

        // create the radio buttons
        createRadios();

        return OControlWizard::onFinish();
    }

    ORadioSelectionPage::ORadioSelectionPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, "GroupRadioSelectionPage", "modules/sabpilot/ui/groupradioselectionpage.ui")
    {
        get(m_pRadioName, "radiolabels");
        get(m_pMoveRight, "toright");
        get(m_pMoveLeft, "toleft");
        get(m_pExistingRadios, "radiobuttons");

        if (getContext().aFieldNames.getLength())
        {
            enableFormDatasourceDisplay();
        }

        m_pMoveLeft->SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_pMoveRight->SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_pRadioName->SetModifyHdl(LINK(this, ORadioSelectionPage, OnNameModified));
        m_pExistingRadios->SetSelectHdl(LINK(this, ORadioSelectionPage, OnEntrySelected));

        implCheckMoveButtons();
        m_pExistingRadios->EnableMultiSelection(true);

        getDialog()->defaultButton(m_pMoveRight.get());

        m_pExistingRadios->SetAccessibleRelationMemberOf(m_pExistingRadios);
    }

    ORadioSelectionPage::~ORadioSelectionPage()
    {
        disposeOnce();
    }

    void ORadioSelectionPage::dispose()
    {
        m_pRadioName.clear();
        m_pMoveRight.clear();
        m_pMoveLeft.clear();
        m_pExistingRadios.clear();
        OGBWPage::dispose();
    }

    void ORadioSelectionPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_pRadioName->GrabFocus();
    }


    void ORadioSelectionPage::initializePage()
    {
        OGBWPage::initializePage();

        m_pRadioName->SetText("");

        // no need to initialize the list of radios here
        // (we're the only one affecting this special setting, so it will be in the same state as last time this
        // page was committed)

        implCheckMoveButtons();
    }


    bool ORadioSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return false;

        // copy the names of the radio buttons to be inserted
        // and initialize the values
        OOptionGroupSettings& rSettings = getSettings();
        rSettings.aLabels.clear();
        rSettings.aValues.clear();
        rSettings.aLabels.reserve(m_pExistingRadios->GetEntryCount());
        rSettings.aValues.reserve(m_pExistingRadios->GetEntryCount());
        for (sal_Int32 i=0; i<m_pExistingRadios->GetEntryCount(); ++i)
        {
            rSettings.aLabels.push_back(m_pExistingRadios->GetEntry(i));
            rSettings.aValues.push_back(OUString::number((i + 1)));
        }

        return true;
    }


    IMPL_LINK_TYPED( ORadioSelectionPage, OnMoveEntry, Button*, _pButton, void )
    {
        bool bMoveLeft = (m_pMoveLeft == _pButton);
        if (bMoveLeft)
        {
            while (m_pExistingRadios->GetSelectEntryCount())
                m_pExistingRadios->RemoveEntry(m_pExistingRadios->GetSelectEntryPos());
        }
        else
        {
            m_pExistingRadios->InsertEntry(m_pRadioName->GetText());
            m_pRadioName->SetText("");
        }

        implCheckMoveButtons();

        //adjust the focus
        if (bMoveLeft)
            m_pExistingRadios->GrabFocus();
        else
            m_pRadioName->GrabFocus();
    }


    IMPL_LINK_NOARG_TYPED( ORadioSelectionPage, OnEntrySelected, ListBox&, void )
    {
        implCheckMoveButtons();
    }


    IMPL_LINK_NOARG_TYPED( ORadioSelectionPage, OnNameModified, Edit&, void )
    {
        implCheckMoveButtons();
    }


    bool ORadioSelectionPage::canAdvance() const
    {
        return 0 != m_pExistingRadios->GetEntryCount();
    }


    void ORadioSelectionPage::implCheckMoveButtons()
    {
        bool bHaveSome = (0 != m_pExistingRadios->GetEntryCount());
        bool bSelectedSome = (0 != m_pExistingRadios->GetSelectEntryCount());
        bool bUnfinishedInput = (!m_pRadioName->GetText().isEmpty());

        m_pMoveLeft->Enable(bSelectedSome);
        m_pMoveRight->Enable(bUnfinishedInput);

        getDialog()->enableButtons(WizardButtonFlags::NEXT, bHaveSome);

        if (bUnfinishedInput)
        {
            if (0 == (m_pMoveRight->GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(m_pMoveRight.get());
        }
        else
        {
            if (WB_DEFBUTTON == (m_pMoveRight->GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(WizardButtonFlags::NEXT);
        }
    }

    ODefaultFieldSelectionPage::ODefaultFieldSelectionPage( OControlWizard* _pParent )
        :OMaybeListSelectionPage(_pParent, "DefaultFieldSelectionPage", "modules/sabpilot/ui/defaultfieldselectionpage.ui")
    {
        get(m_pDefSelYes, "defaultselectionyes");
        get(m_pDefSelNo, "defaultselectionno");
        get(m_pDefSelection, "defselectionfield");

        announceControls(*m_pDefSelYes, *m_pDefSelNo, *m_pDefSelection);
        m_pDefSelection->SetDropDownLineCount(10);
        m_pDefSelection->SetAccessibleRelationLabeledBy( m_pDefSelYes );
        m_pDefSelection->SetStyle(WB_DROPDOWN);
    }

    ODefaultFieldSelectionPage::~ODefaultFieldSelectionPage()
    {
        disposeOnce();
    }

    void ODefaultFieldSelectionPage::dispose()
    {
        m_pDefSelYes.clear();
        m_pDefSelNo.clear();
        m_pDefSelection.clear();
        OMaybeListSelectionPage::dispose();
    }

    void ODefaultFieldSelectionPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();

        // fill the listbox
        m_pDefSelection->Clear();
        for (   StringArray::const_iterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_pDefSelection->InsertEntry(*aLoop);


        implInitialize(rSettings.sDefaultField);
    }


    bool ODefaultFieldSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return false;

        OOptionGroupSettings& rSettings = getSettings();
        implCommit(rSettings.sDefaultField);

        return true;
    }

    OOptionValuesPage::OOptionValuesPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, "OptionValuesPage", "modules/sabpilot/ui/optionvaluespage.ui")
        ,m_nLastSelection((::svt::WizardTypes::WizardState)-1)
    {
        get(m_pValue, "optionvalue");
        get(m_pOptions, "radiobuttons");

        m_pOptions->SetSelectHdl(LINK(this, OOptionValuesPage, OnOptionSelected));

        m_pOptions->SetAccessibleRelationMemberOf(m_pOptions);
    }

    OOptionValuesPage::~OOptionValuesPage()
    {
        disposeOnce();
    }

    void OOptionValuesPage::dispose()
    {
        m_pValue.clear();
        m_pOptions.clear();
        OGBWPage::dispose();
    }

    IMPL_LINK_NOARG_TYPED( OOptionValuesPage, OnOptionSelected, ListBox&, void )
    {
        implTraveledOptions();
    }


    void OOptionValuesPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_pValue->GrabFocus();
    }


    void OOptionValuesPage::implTraveledOptions()
    {
        if ((::svt::WizardTypes::WizardState)-1 != m_nLastSelection)
        {
            // save the value for the last option
            DBG_ASSERT((size_t)m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid previous selection index!");
            m_aUncommittedValues[m_nLastSelection] = m_pValue->GetText();
        }

        m_nLastSelection = m_pOptions->GetSelectEntryPos();
        DBG_ASSERT((size_t)m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid new selection index!");
        m_pValue->SetText(m_aUncommittedValues[m_nLastSelection]);
    }


    void OOptionValuesPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        DBG_ASSERT(rSettings.aLabels.size(), "OOptionValuesPage::initializePage: no options!!");
        DBG_ASSERT(rSettings.aLabels.size() == rSettings.aValues.size(), "OOptionValuesPage::initializePage: inconsistent data!");

        // fill the list with all available options
        m_pOptions->Clear();
        m_nLastSelection = -1;
        for (   StringArray::const_iterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_pOptions->InsertEntry(*aLoop);

        // remember the values ... can't set them directly in the settings without the explicit commit call
        // so we need have a copy of the values
        m_aUncommittedValues = rSettings.aValues;

        // select the first entry
        m_pOptions->SelectEntryPos(0);
        implTraveledOptions();
    }


    bool OOptionValuesPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
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

    OOptionDBFieldPage::OOptionDBFieldPage( OControlWizard* _pParent )
        :ODBFieldPage(_pParent)
    {
        setDescriptionText(ModuleRes(RID_STR_GROUPWIZ_DBFIELD).toString());
    }


    OUString& OOptionDBFieldPage::getDBFieldSetting()
    {
        return getSettings().sDBField;
    }

    OFinalizeGBWPage::OFinalizeGBWPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, "OptionsFinalPage", "modules/sabpilot/ui/optionsfinalpage.ui")
    {
        get(m_pName, "nameit");
    }

    OFinalizeGBWPage::~OFinalizeGBWPage()
    {
        disposeOnce();
    }

    void OFinalizeGBWPage::dispose()
    {
        m_pName.clear();
        OGBWPage::dispose();
    }

    void OFinalizeGBWPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_pName->GrabFocus();
    }


    bool OFinalizeGBWPage::canAdvance() const
    {
        return false;
    }


    void OFinalizeGBWPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        m_pName->SetText(rSettings.sControlLabel);
    }


    bool OFinalizeGBWPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return false;

        getSettings().sControlLabel = m_pName->GetText();

        return true;
    }


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

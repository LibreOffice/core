/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::svt;

    //=====================================================================
    //= OGroupBoxWizard
    //=====================================================================
    //---------------------------------------------------------------------
    OGroupBoxWizard::OGroupBoxWizard( Window* _pParent,
            const Reference< XPropertySet >& _rxObjectModel, const Reference< XMultiServiceFactory >& _rxORB )
        :OControlWizard(_pParent, ModuleRes(RID_DLG_GROUPBOXWIZARD), _rxObjectModel, _rxORB)
        ,m_bVisitedDefault(sal_False)
        ,m_bVisitedDB(sal_False)
    {
        initControlSettings(&m_aSettings);

        m_pPrevPage->SetHelpId(HID_GROUPWIZARD_PREVIOUS);
        m_pNextPage->SetHelpId(HID_GROUPWIZARD_NEXT);
        m_pCancel->SetHelpId(HID_GROUPWIZARD_CANCEL);
        m_pFinish->SetHelpId(HID_GROUPWIZARD_FINISH);
    }

    //---------------------------------------------------------------------
    sal_Bool OGroupBoxWizard::approveControl(sal_Int16 _nClassId)
    {
        return FormComponentType::GROUPBOX == _nClassId;
    }

    //---------------------------------------------------------------------
    OWizardPage* OGroupBoxWizard::createPage(::svt::WizardTypes::WizardState _nState)
    {
        switch (_nState)
        {
            case GBW_STATE_OPTIONLIST:
                return new ORadioSelectionPage(this);

            case GBW_STATE_DEFAULTOPTION:
                return new ODefaultFieldSelectionPage(this);

            case GBW_STATE_OPTIONVALUES:
                return new OOptionValuesPage(this);

            case GBW_STATE_DBFIELD:
                return new OOptionDBFieldPage(this);

            case GBW_STATE_FINALIZE:
                return new OFinalizeGBWPage(this);
        }

        return NULL;
    }

    //---------------------------------------------------------------------
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

    //---------------------------------------------------------------------
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
                m_bVisitedDefault = sal_True;
                break;

            case GBW_STATE_DBFIELD:
                if (!m_bVisitedDB)
                {   // try to generate a default for the DB field
                    // (simply use the first field in the DB names collection)
                    if (getContext().aFieldNames.getLength())
                        m_aSettings.sDBField = getContext().aFieldNames[0];
                }
                m_bVisitedDB = sal_True;
                break;
        }

        // setting the def button .... to be done before the base class is called, too, 'cause the base class
        // calls the pages, which are allowed to override our def button behaviour
        defaultButton(GBW_STATE_FINALIZE == _nState ? WZB_FINISH : WZB_NEXT);

        // allow "finish" on the last page only
        enableButtons(WZB_FINISH, GBW_STATE_FINALIZE == _nState);
        // allow previous on all pages but the first one
        enableButtons(WZB_PREVIOUS, GBW_STATE_OPTIONLIST != _nState);
        // allow next on all pages but the last one
        enableButtons(WZB_NEXT, GBW_STATE_FINALIZE != _nState);

        OControlWizard::enterState(_nState);
    }

    //---------------------------------------------------------------------
    void OGroupBoxWizard::createRadios()
    {
        try
        {
            OOptionGroupLayouter aLayouter(comphelper::getComponentContext(getServiceFactory()));
            aLayouter.doLayout(getContext(), getSettings());
        }
        catch(const Exception&)
        {
            OSL_FAIL("OGroupBoxWizard::createRadios: caught an exception while creating the radio shapes!");
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OGroupBoxWizard::onFinish()
    {
        // commit the basic control setttings
        commitControlSettings(&m_aSettings);

        // create the radio buttons
        createRadios();

        return OControlWizard::onFinish();
    }

    //=====================================================================
    //= ORadioSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    ORadioSelectionPage::ORadioSelectionPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_GROUPRADIOSELECTION))
        ,m_aFrame               (this, ModuleRes(FL_DATA))
        ,m_aRadioNameLabel      (this, ModuleRes(FT_RADIOLABELS))
        ,m_aRadioName           (this, ModuleRes(ET_RADIOLABELS))
        ,m_aMoveRight           (this, ModuleRes(PB_MOVETORIGHT))
        ,m_aMoveLeft            (this, ModuleRes(PB_MOVETOLEFT))
        ,m_aExistingRadiosLabel (this, ModuleRes(FT_RADIOBUTTONS))
        ,m_aExistingRadios      (this, ModuleRes(LB_RADIOBUTTONS))
    {
        FreeResource();

        if (getContext().aFieldNames.getLength())
        {
            enableFormDatasourceDisplay();
        }
        else
        {
            adjustControlForNoDSDisplay(&m_aFrame);
            adjustControlForNoDSDisplay(&m_aRadioNameLabel);
            adjustControlForNoDSDisplay(&m_aRadioName);
            adjustControlForNoDSDisplay(&m_aMoveRight);
            adjustControlForNoDSDisplay(&m_aMoveLeft);
            adjustControlForNoDSDisplay(&m_aExistingRadiosLabel);
            adjustControlForNoDSDisplay(&m_aExistingRadios, sal_True);
        }

        m_aMoveLeft.SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_aMoveRight.SetClickHdl(LINK(this, ORadioSelectionPage, OnMoveEntry));
        m_aRadioName.SetModifyHdl(LINK(this, ORadioSelectionPage, OnNameModified));
        m_aExistingRadios.SetSelectHdl(LINK(this, ORadioSelectionPage, OnEntrySelected));

        implCheckMoveButtons();
        m_aExistingRadios.EnableMultiSelection(sal_True);

        getDialog()->defaultButton(&m_aMoveRight);

        m_aExistingRadios.SetAccessibleRelationMemberOf(&m_aExistingRadios);
        m_aExistingRadios.SetAccessibleRelationLabeledBy(&m_aExistingRadiosLabel);
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aRadioName.GrabFocus();
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::initializePage()
    {
        OGBWPage::initializePage();

        m_aRadioName.SetText(String());

        // no need to initialize the list of radios here
        // (we're the only one affecting this special setting, so it will be in the same state as last time this
        // page was commited)

        implCheckMoveButtons();
    }

    //---------------------------------------------------------------------
    sal_Bool ORadioSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        // copy the names of the radio buttons to be inserted
        // and initialize the values
        OOptionGroupSettings& rSettings = getSettings();
        rSettings.aLabels.clear();
        rSettings.aValues.clear();
        rSettings.aLabels.reserve(m_aExistingRadios.GetEntryCount());
        rSettings.aValues.reserve(m_aExistingRadios.GetEntryCount());
        for (::svt::WizardTypes::WizardState i=0; i<m_aExistingRadios.GetEntryCount(); ++i)
        {
            rSettings.aLabels.push_back(m_aExistingRadios.GetEntry(i));
            rSettings.aValues.push_back(rtl::OUString::valueOf((sal_Int32)(i + 1)));
        }

        return sal_True;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnMoveEntry, PushButton*, _pButton )
    {
        sal_Bool bMoveLeft = (&m_aMoveLeft == _pButton);
        if (bMoveLeft)
        {
            while (m_aExistingRadios.GetSelectEntryCount())
                m_aExistingRadios.RemoveEntry(m_aExistingRadios.GetSelectEntryPos(0));
        }
        else
        {
            m_aExistingRadios.InsertEntry(m_aRadioName.GetText());
            m_aRadioName.SetText(String());
        }

        implCheckMoveButtons();

        //adjust the focus
        if (bMoveLeft)
            m_aExistingRadios.GrabFocus();
        else
            m_aRadioName.GrabFocus();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnEntrySelected, ListBox*, /*_pList*/ )
    {
        implCheckMoveButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    IMPL_LINK( ORadioSelectionPage, OnNameModified, Edit*, /*_pList*/ )
    {
        implCheckMoveButtons();
        return 0L;
    }

    //---------------------------------------------------------------------
    bool ORadioSelectionPage::canAdvance() const
    {
        return 0 != m_aExistingRadios.GetEntryCount();
    }

    //---------------------------------------------------------------------
    void ORadioSelectionPage::implCheckMoveButtons()
    {
        sal_Bool bHaveSome = (0 != m_aExistingRadios.GetEntryCount());
        sal_Bool bSelectedSome = (0 != m_aExistingRadios.GetSelectEntryCount());
        sal_Bool bUnfinishedInput = (0 != m_aRadioName.GetText().Len());

        m_aMoveLeft.Enable(bSelectedSome);
        m_aMoveRight.Enable(bUnfinishedInput);

        getDialog()->enableButtons(WZB_NEXT, bHaveSome);

        if (bUnfinishedInput)
        {
            if (0 == (m_aMoveRight.GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(&m_aMoveRight);
        }
        else
        {
            if (WB_DEFBUTTON == (m_aMoveRight.GetStyle() & WB_DEFBUTTON))
                getDialog()->defaultButton(WZB_NEXT);
        }
    }

    //=====================================================================
    //= ODefaultFieldSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    ODefaultFieldSelectionPage::ODefaultFieldSelectionPage( OControlWizard* _pParent )
        :OMaybeListSelectionPage(_pParent, ModuleRes(RID_PAGE_DEFAULTFIELDSELECTION))
        ,m_aFrame                   (this, ModuleRes(FL_DEFAULTSELECTION))
        ,m_aDefaultSelectionLabel   (this, ModuleRes(FT_DEFAULTSELECTION))
        ,m_aDefSelYes               (this, ModuleRes(RB_DEFSELECTION_YES))
        ,m_aDefSelNo                (this, ModuleRes(RB_DEFSELECTION_NO))
        ,m_aDefSelection            (this, ModuleRes(LB_DEFSELECTIONFIELD))
    {
        FreeResource();

        announceControls(m_aDefSelYes, m_aDefSelNo, m_aDefSelection);
        m_aDefSelection.SetDropDownLineCount(10);
        m_aDefSelection.SetAccessibleRelationLabeledBy( &m_aDefSelYes );
        m_aDefSelection.SetAccessibleRelationMemberOf(&m_aDefaultSelectionLabel);
    }

    //---------------------------------------------------------------------
    void ODefaultFieldSelectionPage::initializePage()
    {
        OMaybeListSelectionPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();

        // fill the listbox
        m_aDefSelection.Clear();
        for (   ConstStringArrayIterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_aDefSelection.InsertEntry(*aLoop);


        implInitialize(rSettings.sDefaultField);
    }

    //---------------------------------------------------------------------
    sal_Bool ODefaultFieldSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OMaybeListSelectionPage::commitPage(_eReason))
            return sal_False;

        OOptionGroupSettings& rSettings = getSettings();
        implCommit(rSettings.sDefaultField);

        return sal_True;
    }

    //=====================================================================
    //= OOptionValuesPage
    //=====================================================================
    //---------------------------------------------------------------------
    OOptionValuesPage::OOptionValuesPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_OPTIONVALUES))
        ,m_aFrame               (this, ModuleRes(FL_OPTIONVALUES))
        ,m_aDescription         (this, ModuleRes(FT_OPTIONVALUES_EXPL))
        ,m_aValueLabel          (this, ModuleRes(FT_OPTIONVALUES))
        ,m_aValue               (this, ModuleRes(ET_OPTIONVALUE))
        ,m_aOptionsLabel        (this, ModuleRes(FT_RADIOBUTTONS))
        ,m_aOptions             (this, ModuleRes(LB_RADIOBUTTONS))
        ,m_nLastSelection((::svt::WizardTypes::WizardState)-1)
    {
        FreeResource();

        m_aOptions.SetSelectHdl(LINK(this, OOptionValuesPage, OnOptionSelected));

        m_aOptions.SetAccessibleRelationMemberOf(&m_aOptions);
        m_aOptions.SetAccessibleRelationLabeledBy(&m_aOptionsLabel);
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OOptionValuesPage, OnOptionSelected, ListBox*, /*NOTINTERESTEDIN*/ )
    {
        implTraveledOptions();
        return 0L;
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aValue.GrabFocus();
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::implTraveledOptions()
    {
        if ((::svt::WizardTypes::WizardState)-1 != m_nLastSelection)
        {
            // save the value for the last option
            DBG_ASSERT((size_t)m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid previous selection index!");
            m_aUncommittedValues[m_nLastSelection] = m_aValue.GetText();
        }

        m_nLastSelection = m_aOptions.GetSelectEntryPos();
        DBG_ASSERT((size_t)m_nLastSelection < m_aUncommittedValues.size(), "OOptionValuesPage::implTraveledOptions: invalid new selection index!");
        m_aValue.SetText(m_aUncommittedValues[m_nLastSelection]);
    }

    //---------------------------------------------------------------------
    void OOptionValuesPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        DBG_ASSERT(rSettings.aLabels.size(), "OOptionValuesPage::initializePage: no options!!");
        DBG_ASSERT(rSettings.aLabels.size() == rSettings.aValues.size(), "OOptionValuesPage::initializePage: inconsistent data!");

        // fill the list with all available options
        m_aOptions.Clear();
        m_nLastSelection = -1;
        for (   ConstStringArrayIterator aLoop = rSettings.aLabels.begin();
                aLoop != rSettings.aLabels.end();
                ++aLoop
            )
            m_aOptions.InsertEntry(*aLoop);

        // remember the values ... can't set them directly in the settings without the explicit commit call
        // so we need have a copy of the values
        m_aUncommittedValues = rSettings.aValues;

        // select the first entry
        m_aOptions.SelectEntryPos(0);
        implTraveledOptions();
    }

    //---------------------------------------------------------------------
    sal_Bool OOptionValuesPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        OOptionGroupSettings& rSettings = getSettings();

        // commit the current value
        implTraveledOptions();
        // copy the uncommitted values
        rSettings.aValues = m_aUncommittedValues;

        return sal_True;
    }

    //=====================================================================
    //= OOptionDBFieldPage
    //=====================================================================
    //---------------------------------------------------------------------
    OOptionDBFieldPage::OOptionDBFieldPage( OControlWizard* _pParent )
        :ODBFieldPage(_pParent)
    {
        setDescriptionText(String(ModuleRes(RID_STR_GROUPWIZ_DBFIELD)));
    }

    //---------------------------------------------------------------------
    String& OOptionDBFieldPage::getDBFieldSetting()
    {
        return getSettings().sDBField;
    }

    //=====================================================================
    //= OFinalizeGBWPage
    //=====================================================================
    //---------------------------------------------------------------------
    OFinalizeGBWPage::OFinalizeGBWPage( OControlWizard* _pParent )
        :OGBWPage(_pParent, ModuleRes(RID_PAGE_OPTIONS_FINAL))
        ,m_aFrame           (this, ModuleRes(FL_NAMEIT))
        ,m_aNameLabel       (this, ModuleRes(FT_NAMEIT))
        ,m_aName            (this, ModuleRes(ET_NAMEIT))
        ,m_aThatsAll        (this, ModuleRes(FT_THATSALL))
    {
        FreeResource();
    }

    //---------------------------------------------------------------------
    void OFinalizeGBWPage::ActivatePage()
    {
        OGBWPage::ActivatePage();
        m_aName.GrabFocus();
    }

    //---------------------------------------------------------------------
    bool OFinalizeGBWPage::canAdvance() const
    {
        return false;
    }

    //---------------------------------------------------------------------
    void OFinalizeGBWPage::initializePage()
    {
        OGBWPage::initializePage();

        const OOptionGroupSettings& rSettings = getSettings();
        m_aName.SetText(rSettings.sControlLabel);
    }

    //---------------------------------------------------------------------
    sal_Bool OFinalizeGBWPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!OGBWPage::commitPage(_eReason))
            return sal_False;

        getSettings().sControlLabel = m_aName.GetText();

        return sal_True;
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

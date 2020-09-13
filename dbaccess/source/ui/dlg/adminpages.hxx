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

#pragma once

#include <sfx2/tabdlg.hxx>
#include <vcl/wizardmachine.hxx>
#include <curledit.hxx>

namespace dbaui
{
    /// helper class to wrap the savevalue and disable call
    class SAL_NO_VTABLE ISaveValueWrapper
    {
    public:
        virtual ~ISaveValueWrapper() = 0;
        virtual void SaveValue() = 0;
        virtual void Disable() = 0;
    };

    template < class T > class OSaveValueWidgetWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        explicit OSaveValueWidgetWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual void SaveValue() override { m_pSaveValue->save_value(); }
        virtual void Disable() override { m_pSaveValue->set_sensitive(false); }
    };

    template <> class OSaveValueWidgetWrapper<weld::ToggleButton> : public ISaveValueWrapper
    {
        weld::ToggleButton*  m_pSaveValue;
    public:
        explicit OSaveValueWidgetWrapper(weld::ToggleButton* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual void SaveValue() override { m_pSaveValue->save_state(); }
        virtual void Disable() override { m_pSaveValue->set_sensitive(false); }
    };

    template <> class OSaveValueWidgetWrapper<dbaui::OConnectionURLEdit> : public ISaveValueWrapper
    {
        dbaui::OConnectionURLEdit*  m_pSaveValue;
    public:
        explicit OSaveValueWidgetWrapper(dbaui::OConnectionURLEdit* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual void SaveValue() override { m_pSaveValue->save_value(); }
        virtual void Disable() override { m_pSaveValue->set_sensitive(false); }
    };

    template <class T> class ODisableWidgetWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        explicit ODisableWidgetWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual void SaveValue() override {}
        virtual void Disable() override { m_pSaveValue->set_sensitive(false); }
    };

    // OGenericAdministrationPage
    class IDatabaseSettingsDialog;
    class IItemSetHelper;
    class OGenericAdministrationPage    :public SfxTabPage
                                        ,public ::vcl::IWizardPageController
    {
    private:
        Link<OGenericAdministrationPage const *, void> m_aModifiedHandler; /// to be called if something on the page has been modified
        bool        m_abEnableRoadmap;
    protected:
        IDatabaseSettingsDialog*   m_pAdminDialog;
        IItemSetHelper*            m_pItemSetHelper;

        css::uno::Reference< css::uno::XComponentContext >
                                   m_xORB;
    public:
        OGenericAdministrationPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OString& rId, const SfxItemSet& rAttrSet);
        /// set a handler which gets called every time something on the page has been modified
        void SetModifiedHandler(const Link<OGenericAdministrationPage const *, void>& _rHandler) { m_aModifiedHandler = _rHandler; }

        /** Sets the ParentDialog
            @param  _pAdminDialog
                the ParentDialog
            @param  _pItemSetHelper
                the itemset helper
        */
        void SetAdminDialog(IDatabaseSettingsDialog* _pDialog,IItemSetHelper* _pItemSetHelper)
        {
            OSL_ENSURE(_pDialog && _pItemSetHelper,"Values are NULL!");
            m_pAdminDialog = _pDialog;
            m_pItemSetHelper = _pItemSetHelper;
        }

        /** Sets the ServiceFactory
            @param  _rxORB
                The service factory.
        */
        void SetServiceFactory(const css::uno::Reference< css::uno::XComponentContext >& rxORB)
        {
            m_xORB = rxORB;
        }

        /** opens a dialog filled with all data sources available for this type and
            returns the selected on.
            @param  _eType
                The type for which the data source dialog should be opened.
            @param  _sReturn
                <OUT/> contains the selected name.
            @return
                <FALSE/> if an error occurred, otherwise <TRUE/>
        */
        bool getSelectedDataSource(OUString& _sReturn, OUString const & _sCurr);

        // svt::IWizardPageController
        virtual void initializePage() override;
        virtual bool commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool canAdvance() const override;

        void                SetRoadmapStateValue( bool _bDoEnable ) { m_abEnableRoadmap = _bDoEnable; }
        bool                GetRoadmapStateValue() const { return m_abEnableRoadmap; }

    protected:
        /// default implementation: call FillItemSet, call prepareLeave,
        virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_False
        virtual void Reset(const SfxItemSet* _rCoreAttrs) override;
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_True
        virtual void ActivatePage(const SfxItemSet& _rSet) override;

        // BuilderPage overridables
        virtual void    Activate() override;

    protected:
        virtual void callModifiedHdl(weld::Widget* /*pControl*/ = nullptr) { m_aModifiedHandler.Call(this); }

        /// called from within DeactivatePage. The page is allowed to be deactivated if this method returns sal_True
        virtual bool prepareLeave() { return true; }

        /** called from within Reset and ActivatePage, use to initialize the controls with the items from the given set
            @param      _bSaveValue     if set to sal_True, the implementation should call SaveValue on all relevant controls
        */
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue);

        /// analyze the invalid and the readonly flag which may be present in the set
        static void getFlags(const SfxItemSet& _rSet, bool& _rValid, bool& _rReadonly);

        /** will be called inside <method>implInitControls</method> to save the value if necessary
            @param  _rControlList
                The list must be filled with the controls.
                It is not allowed to clear the list before pushing data into it.
        */
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) = 0;

        /** will be called inside <method>implInitControls</method> to disable if necessary
            @param  _rControlList
                The list must be filled with the controls.
                It is not allowed to clear the list before pushing data into it.
        */
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) = 0;

    public:
        /** fills the Boolean value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pCheckBox
                The check box which is checked.
            @param  _nID
                The id in the itemset to set with the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
            @param _bRevertValue
                set to <TRUE/> if the display value should be reverted before putting it into the set
        */
        static void fillBool(SfxItemSet& _rSet, const weld::CheckButton* pCheckBox, sal_uInt16 _nID, bool bOptionalBool, bool& _bChangedSomething, bool _bRevertValue = false);

        /** fills the int value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pEdit
                The check box which is checked.
            @param  _nID
                The id in the itemset to set with the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
        */
        static void fillInt32(SfxItemSet& _rSet,const weld::SpinButton* pEdit,sal_uInt16 _nID, bool& _bChangedSomething);

        /** fills the String value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pEdit
                The check box which is checked.
            @param  _nID
                The id in the itemset to set with the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
        */
        static void fillString(SfxItemSet& _rSet,const weld::Entry* pEdit,sal_uInt16 _nID, bool& _bChangedSomething);
        static void fillString(SfxItemSet& _rSet,const dbaui::OConnectionURLEdit* pEdit,sal_uInt16 _nID, bool& _bChangedSomething);

    protected:
        /** This link be used for controls where the tabpage does not need to take any special action when the control
            is modified. The implementation just calls callModifiedHdl.
        */
        DECL_LINK(OnControlModified, weld::Widget*, void);
        DECL_LINK(OnControlEntryModifyHdl, weld::Entry&, void);
        DECL_LINK(OnControlSpinButtonModifyHdl, weld::SpinButton&, void);
        DECL_LINK(OnControlModifiedButtonClick, weld::ToggleButton&, void);
        DECL_LINK(OnTestConnectionButtonClickHdl, weld::Button&, void);
    };
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

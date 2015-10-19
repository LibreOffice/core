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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINPAGES_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINPAGES_HXX

#include <sfx2/tabdlg.hxx>
#include "dsntypes.hxx"
#include "commontypes.hxx"
#include <svtools/wizardmachine.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

class NumericField;
class Edit;
namespace dbaui
{
    /// helper class to wrap the savevalue and disable call
    class SAL_NO_VTABLE ISaveValueWrapper
    {
    public:
        virtual ~ISaveValueWrapper() = 0;
        virtual bool SaveValue() = 0;
        virtual bool Disable() = 0;
    };

    template < class T > class OSaveValueWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        explicit OSaveValueWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual bool SaveValue() override { m_pSaveValue->SaveValue(); return true;} // bool return value only for stl
        virtual bool Disable() override { m_pSaveValue->Disable(); return true;} // bool return value only for stl
    };

    template < class T > class ODisableWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        explicit ODisableWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual bool SaveValue() override { return true;} // bool return value only for stl
        virtual bool Disable() override { m_pSaveValue->Disable(); return true;} // bool return value only for stl
    };

    // OGenericAdministrationPage
    class IDatabaseSettingsDialog;
    class IItemSetHelper;
    class OGenericAdministrationPage    :public SfxTabPage
                                        ,public ::svt::IWizardPageController
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
        OGenericAdministrationPage(vcl::Window* _pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rAttrSet);
        /// set a handler which gets called every time something on the page has been modified
        void SetModifiedHandler(const Link<OGenericAdministrationPage const *, void>& _rHandler) { m_aModifiedHandler = _rHandler; }

        /** Sets the ParentDialog
            @param  _pAdminDialog
                the ParentDialog
            @param  _pItemSetHelper
                the itemset helper
        */
        inline void SetAdminDialog(IDatabaseSettingsDialog* _pDialog,IItemSetHelper* _pItemSetHelper)
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
        bool getSelectedDataSource(OUString& _sReturn, OUString& _sCurr);

        // svt::IWizardPageController
        virtual void initializePage() override;
        virtual bool commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool canAdvance() const override;

        void                SetRoadmapStateValue( bool _bDoEnable ) { m_abEnableRoadmap = _bDoEnable; }
        bool                GetRoadmapStateValue() const { return m_abEnableRoadmap; }

    protected:
        /// default implementation: call FillItemSet, call prepareLeave,
        virtual sfxpg DeactivatePage(SfxItemSet* pSet) override;
        using SfxTabPage::DeactivatePage;
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_False
        virtual void Reset(const SfxItemSet* _rCoreAttrs) override;
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_True
        virtual void ActivatePage(const SfxItemSet& _rSet) override;

        // TabPage overridables
        virtual void    ActivatePage() override;

    protected:
        virtual void callModifiedHdl(void* /*pControl*/ = 0) { m_aModifiedHandler.Call(this); }

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
                It is not allowed to clear the list before pusching data into it.
        */
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) = 0;

        /** will be called inside <method>implInitControls</method> to disable if necessary
            @param  _rControlList
                The list must be filled with the controls.
                It is not allowed to clear the list before pusching data into it.
        */
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) = 0;

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
        static void fillBool( SfxItemSet& _rSet, CheckBox* _pCheckBox, sal_uInt16 _nID, bool& _bChangedSomething, bool _bRevertValue = false);

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
        static void fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,sal_uInt16 _nID, bool& _bChangedSomething);

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
        static void fillString(SfxItemSet& _rSet,Edit* _pEdit,sal_uInt16 _nID, bool& _bChangedSomething);

    protected:
        /** This link be used for controls where the tabpage does not need to take any special action when the control
            is modified. The implementation just calls callModifiedHdl.
        */
        DECL_LINK_TYPED(OnControlModified, void*, void);
        DECL_LINK_TYPED(OnControlEditModifyHdl, Edit&, void);
        DECL_LINK_TYPED(OnControlModifiedClick, Button*, void);
        DECL_LINK_TYPED(ControlModifiedCheckBoxHdl, CheckBox&, void);

        DECL_LINK_TYPED(OnTestConnectionClickHdl, Button*, void);
    };

    // ControlRelation
    enum ControlRelation
    {
        RelatedControls, UnrelatedControls
    };

    // LayoutHelper
    class LayoutHelper
    {
    public:
        static void     positionBelow(
                            const Control& _rReference,
                            Control& _rControl,
                            const ControlRelation _eRelation,
                            const long _nIndentAppFont
                        );
        /** fits the button size to be large enough to contain the buttons text
        */
        static void fitSizeRightAligned( PushButton& io_button );
            // why is CalcMinimumSize not a virtual method of vcl::Window?
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADMINPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

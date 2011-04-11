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

#ifndef _DBAUI_ADMINPAGES_HXX_
#define _DBAUI_ADMINPAGES_HXX_

#include <sfx2/tabdlg.hxx>
#include "dsntypes.hxx"
#include "commontypes.hxx"
#include <svtools/wizardmachine.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>


class NumericField;
class Edit;
//.........................................................................
namespace dbaui
{
//.........................................................................
    /// helper class to wrap the savevalue and disable call
    class SAL_NO_VTABLE ISaveValueWrapper
    {
    public:
        virtual bool SaveValue() = 0;
        virtual bool Disable() = 0;
    };

    template < class T > class OSaveValueWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        OSaveValueWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual bool SaveValue() { m_pSaveValue->SaveValue(); return true;} // bool return value only for stl
        virtual bool Disable() { m_pSaveValue->Disable(); return true;} // bool return value only for stl
    };

    template < class T > class ODisableWrapper : public ISaveValueWrapper
    {
        T*  m_pSaveValue;
    public:
        ODisableWrapper(T* _pSaveValue) : m_pSaveValue(_pSaveValue)
        { OSL_ENSURE(m_pSaveValue,"Illegal argument!"); }

        virtual bool SaveValue() { return true;} // bool return value only for stl
        virtual bool Disable() { m_pSaveValue->Disable(); return true;} // bool return value only for stl
    };

    struct TSaveValueWrapperFunctor : public ::std::unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            return lhs->SaveValue();
        }
    };
    struct TDisableWrapperFunctor : public ::std::unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            return lhs->Disable();
        }
    };

    struct TDeleteWrapperFunctor : public ::std::unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            delete lhs;
            return true;
        }
    };

    //=========================================================================
    //= OGenericAdministrationPage
    //=========================================================================
    class IDatabaseSettingsDialog;
    class IItemSetHelper;
    class OGenericAdministrationPage    :public SfxTabPage
                                        ,public ::svt::IWizardPageController
    {
    private:
        Link            m_aModifiedHandler;     /// to be called if something on the page has been modified
        sal_Bool        m_abEnableRoadmap;
    protected:
        IDatabaseSettingsDialog*   m_pAdminDialog;
        IItemSetHelper* m_pItemSetHelper;
        FixedText*      m_pFT_HeaderText;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                            m_xORB;
    public:
        OGenericAdministrationPage(Window* _pParent, const ResId& _rId, const SfxItemSet& _rAttrSet);
        ~OGenericAdministrationPage();

        /// set a handler which gets called every time something on the page has been modified
        void SetModifiedHandler(const Link& _rHandler) { m_aModifiedHandler = _rHandler; }

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
        virtual void SetServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            m_xORB = _rxORB;
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
        sal_Bool getSelectedDataSource(::rtl::OUString& _sReturn,::rtl::OUString& _sCurr);

        // svt::IWizardPageController
        virtual void initializePage();
        virtual sal_Bool commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool canAdvance() const;

        void                SetRoadmapStateValue( sal_Bool _bDoEnable ) { m_abEnableRoadmap = _bDoEnable; }
        bool                GetRoadmapStateValue() const { return m_abEnableRoadmap; }

    protected:
        /// default implementation: call FillItemSet, call prepareLeave,
        virtual int DeactivatePage(SfxItemSet* pSet);
        using SfxTabPage::DeactivatePage;
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_False
        virtual void Reset(const SfxItemSet& _rCoreAttrs);
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_True
        virtual void ActivatePage(const SfxItemSet& _rSet);

        // TabPage overridables
        virtual void    ActivatePage();

    protected:
        void callModifiedHdl() const { if (m_aModifiedHandler.IsSet()) m_aModifiedHandler.Call((void*)this); }

        /// called from within DeactivatePage. The page is allowed to be deactivated if this method returns sal_True
        virtual sal_Bool prepareLeave() { return sal_True; }

        /** called from within Reset and ActivatePage, use to initialize the controls with the items from the given set
            @param      _bSaveValue     if set to sal_True, the implementation should call SaveValue on all relevant controls
        */
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        /// analyze the invalid and the readonly flag which may be present in the set
        void getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly);

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
                The id in the itemset to set whith the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
            @param _bRevertValue
                set to <TRUE/> if the display value should be reverted before putting it into the set
        */
        static void fillBool( SfxItemSet& _rSet, CheckBox* _pCheckBox, sal_uInt16 _nID, sal_Bool& _bChangedSomething, bool _bRevertValue = false);

        /** fills the int value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pEdit
                The check box which is checked.
            @param  _nID
                The id in the itemset to set whith the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
        */
        static void fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,sal_uInt16 _nID,sal_Bool& _bChangedSomething);

        /** fills the String value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pEdit
                The check box which is checked.
            @param  _nID
                The id in the itemset to set whith the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
        */
        static void fillString(SfxItemSet& _rSet,Edit* _pEdit,sal_uInt16 _nID,sal_Bool& _bChangedSomething);

    protected:
        // used to set the right Pane header of a wizard to bold
        void SetControlFontWeight(Window* _pWindow, FontWeight _eWeight = WEIGHT_BOLD);
        void SetHeaderText( sal_uInt16 _nFTResId, sal_uInt16 _StringResId);

        /** This link be used for controls where the tabpage does not need to take any special action when the control
            is modified. The implementation just calls callModifiedHdl.
        */
        DECL_LINK(OnControlModified, Control*);
        DECL_LINK(OnTestConnectionClickHdl,PushButton*);

        /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
        virtual Link getControlModifiedLink() { return LINK(this, OGenericAdministrationPage, OnControlModified); }
    };

    //=========================================================================
    //= ControlRelation
    //=========================================================================
    enum ControlRelation
    {
        RelatedControls, UnrelatedControls
    };

    //=========================================================================
    //= LayoutHelper
    //=========================================================================
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
            // why is CalcMinimumSize not a virtual method of ::Window?
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_ADMINPAGES_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

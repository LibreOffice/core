/*************************************************************************
 *
 *  $RCSfile: adminpages.hxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:43:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_ADMINPAGES_HXX_
#define _DBAUI_ADMINPAGES_HXX_

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _DBAUI_CHARSETS_HXX_
#include "charsets.hxx"
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif

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

    struct TSaveValueWrapperFunctor : public unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            return lhs->SaveValue();
        }
    };
    struct TDisableWrapperFunctor : public unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            return lhs->Disable();
        }
    };

    struct TDeleteWrapperFunctor : public unary_function< ISaveValueWrapper, bool>
    {
        bool operator() (ISaveValueWrapper* lhs)
        {
            delete lhs;
            return true;
        }
    };

    //=========================================================================
    //= OPageSettings
    //=========================================================================
    struct OPageSettings
    {
        virtual ~OPageSettings();
    };


    //=========================================================================
    //= OGenericAdministrationPage
    //=========================================================================
    class IAdminHelper;
    class IItemSetHelper;
    class OGenericAdministrationPage : public SfxTabPage, public svt::IWizardPage
    {
    private:
        Link            m_aModifiedHandler;     /// to be called if something on the page has been modified

    protected:
        IAdminHelper* m_pAdminDialog;
        IItemSetHelper* m_pItemSetHelper;
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
        inline void SetAdminDialog(IAdminHelper* _pDialog,IItemSetHelper* _pItemSetHelper)
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

        /** create an instance of view settings for the page
            <p>The caller is responsible for destroying the object later on.</p>
            <p>The page may return <NULL/> if it does not support view settings.</p>
        */
        virtual OPageSettings*  createViewSettings();

        /** get the pages current view settings, if any
        */
        virtual void            fillViewSettings(OPageSettings* _pSettings);

        /** called by the dialog after changes have been applied asnychronously
            <p>The page can use this method to restore it's (non-persistent, e.g. view-) settings to the
            state before the changes have been applied</p>
            <p>This method is necessary because during applying, the page may die and be re-created.</p>

            @param _pPageState
                the page state as given in <method>IAdminHelper::applyChangesAsync</method>
            @see IAdminHelper::applyChangesAsync
        */
        virtual void            restoreViewSettings(const OPageSettings* _pSettings);

        /** opens a dialog filled with all data sources available for this type and
            returns the selected on.
            @param  _eType
                The type for which the data source dialog should be opened.
            @param  _sReturn
                <OUT/> contains the selected name.
            @return
                <FALSE/> if an error occured, otherwise <TRUE/>
        */
        sal_Bool getSelectedDataSource(DATASOURCE_TYPE _eType,::rtl::OUString& _sReturn);

        // svt::IWizardPage
        virtual void enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight, GrantAccess );
        virtual void initializePage();
        virtual sal_Bool commitPage(COMMIT_REASON _eReason);

    protected:
        /// default implementation: call FillItemSet, call checkItems,
        virtual int DeactivatePage(SfxItemSet* pSet);
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_False
        virtual void Reset(const SfxItemSet& _rCoreAttrs);
        /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_True
        virtual void ActivatePage(const SfxItemSet& _rSet);

        // TabPage overridables
        virtual void    ActivatePage();

    protected:
        void callModifiedHdl() const { if (m_aModifiedHandler.IsSet()) m_aModifiedHandler.Call((void*)this); }

        /// called from within DeactivatePage. The page is allowed to be deactivated if this method returns sal_True
        virtual sal_Bool checkItems() { return sal_True; }

        /** called from within Reset and ActivatePage, use to initialize the controls with the items from the given set
            @param      _bSaveValue     if set to sal_True, the implementation should call SaveValue on all relevant controls
        */
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) { postInitControls(_rSet, _bSaveValue); }

        /// analyze the invalid and the readonly flag which may be present in the set
        void getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly);

        /** will be called inside <method>postInitControl</method> to save the value if necessary
            @param  _rControlList
                The list must be filled with the controls.
                It is not allowed to clear the list before pusching data into it.
        */
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) = 0;

        /** will be called inside <method>postInitControl</method> to disable if necessary
            @param  _rControlList
                The list must be filled with the controls.
                It is not allowed to clear the list before pusching data into it.
        */
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) = 0;

        /** fills the Boolean value into the item set when the value changed.
            @param  _rSet
                The item set where to put the new value into.
            @param  _pCheckBox
                The check box which is checked.
            @param  _nID
                The id in the itemset to set whith the new value.
            @param  _bChangedSomething
                <TRUE/> if something changed otherwise <FALSE/>
        */
        void fillBool(SfxItemSet& _rSet,CheckBox* _pCheckBox,USHORT _nID,sal_Bool& _bChangedSomething);

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
        void fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,USHORT _nID,sal_Bool& _bChangedSomething);

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
        void fillString(SfxItemSet& _rSet,Edit* _pEdit,USHORT _nID,sal_Bool& _bChangedSomething);

    protected:
        /** This link be used for controls where the tabpage does not need to take any special action when the control
            is modified. The implementation just calls callModifiedHdl.
        */
        DECL_LINK(OnControlModified, Control*);

        /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
        Link getControlModifiedLink() { return LINK(this, OGenericAdministrationPage, OnControlModified); }

    private:
        void postInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_ADMINPAGES_HXX_



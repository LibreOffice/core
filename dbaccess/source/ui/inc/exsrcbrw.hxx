/*************************************************************************
 *
 *  $RCSfile: exsrcbrw.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-01 15:16:27 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_EXTCTRLR_HXX
#define _SBA_EXTCTRLR_HXX

#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
//==============================================================================
//= SbaExternalSourceBrowser
//==============================================================================

namespace dbaui
{
    class SbaXFormAdapter;
    class SbaExternalSourceBrowser
                :public SbaXDataBrowserController
                ,public ::com::sun::star::util::XModifyBroadcaster
                ,public ::com::sun::star::awt::XFocusListener
    {
        ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
            // for multiplexing the modify events
        SbaXFormAdapter*                    m_pDataSourceImpl;
        sal_Bool                            m_bInQueryDispatch;
            // our queryDispatch will ask our frame, which first will ask our queryDispatch, so we need to protect against
            // recursion

        //==============================================================================
        // a helper class implementing a ::com::sun::star::form::XFormController, will be aggregated by SbaExternalSourceBrowser
        // (we can't derive from ::com::sun::star::form::XFormController as it's base class is ::com::sun::star::awt::XTabController and the ::com::sun::star::awt::XTabController::getModel collides
        // with the ::com::sun::star::frame::XController::getModel implemented in our base class SbaXDataBrowserController)
        class FormControllerImpl
            : public ::cppu::WeakAggImplHelper2< ::com::sun::star::form::XFormController,
                                                 ::com::sun::star::frame::XFrameActionListener>
        {
            friend class SbaExternalSourceBrowser;
            ::cppu::OInterfaceContainerHelper   m_aActivateListeners;
            SbaExternalSourceBrowser*           m_pOwner;
            sal_Bool                            m_bActive;

        public:
            FormControllerImpl(SbaExternalSourceBrowser* pOwner);

            // ::com::sun::star::form::XFormController
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getCurrentControl(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL addActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL removeActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );

            // ::com::sun::star::awt::XTabController
            virtual void SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > & Model) throw( ::com::sun::star::uno::RuntimeException );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL setContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & _Container) throw( ::com::sun::star::uno::RuntimeException );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  SAL_CALL getContainer(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  > SAL_CALL getControls(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL autoTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL activateTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL activateFirst(void) throw( ::com::sun::star::uno::RuntimeException );
            virtual void SAL_CALL activateLast(void) throw( ::com::sun::star::uno::RuntimeException );

            // ::com::sun::star::frame::XFrameActionListener
            virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

            // ::com::sun::star::lang::XEventListener
            virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        protected:
            ~FormControllerImpl();
        };
        friend class SbaExternalSourceBrowser::FormControllerImpl;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >         m_xFormControllerImpl;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >         m_xTabControllerModelFake;
        FormControllerImpl* m_pFormControllerImpl;

    public:
        SbaExternalSourceBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaExternalSourceBrowser, OGenericUnoController);
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);
        //  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > >  getIdlClasses(void);

        //  static ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getStaticIdlClass();

        // ::com::sun::star::frame::XDispatch
        virtual void SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::util::XModifyListener
        virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::util::XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL disposing();

        // ::com::sun::star::form::XLoadListener
        virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::awt::XFocusListener
        virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );

        // we want to be a focus listener for the control
        virtual void addControlListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & _xGridControl);
        virtual void removeControlListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & _xGridControl);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    protected:
        ~SbaExternalSourceBrowser();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  CreateForm();
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm);
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);

        virtual sal_Bool LoadForm();

        virtual sal_uInt16 SaveData(sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);

        void Attach(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xMaster);

        void ClearView();

        void startListening();
        void stopListening();
    };
}

#endif // _SBA_EXTCTRLR_HXX

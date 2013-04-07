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

#ifndef SVT_UNO_WIZARD_HXX
#define SVT_UNO_WIZARD_HXX

#include "svtools/genericunodialog.hxx"

#include <com/sun/star/ui/dialogs/XWizard.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XWizardController.hpp>

#include <cppuhelper/implbase1.hxx>
#include <comphelper/componentcontext.hxx>

//......................................................................................................................
namespace svt { namespace uno
{
//......................................................................................................................

    //==================================================================================================================
    //= Wizard - declaration
    //==================================================================================================================
    typedef ::cppu::ImplInheritanceHelper1  <   ::svt::OGenericUnoDialog
                                            ,   ::com::sun::star::ui::dialogs::XWizard
                                            >   Wizard_Base;
    class Wizard;
    typedef ::comphelper::OPropertyArrayUsageHelper< Wizard >  Wizard_PBase;
    class Wizard    : public Wizard_Base
                    , public Wizard_PBase
    {
    public:
        Wizard( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext );

        // ::com::sun::star::lang::XServiceInfo - static version
        static OUString SAL_CALL getImplementationName_static() throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static() throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext );

    protected:
        // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // ::com::sun::star::ui::dialogs::XWizard
        virtual OUString SAL_CALL getHelpURL() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHelpURL( const OUString& _helpurl ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getDialogWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardPage > SAL_CALL getCurrentPage(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enableButton( ::sal_Int16 WizardButton, ::sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDefaultButton( ::sal_Int16 WizardButton ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL travelNext(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL travelPrevious(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enablePage( ::sal_Int16 PageID, ::sal_Bool Enable ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTravelUI(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL advanceTo( ::sal_Int16 PageId ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL goBackTo( ::sal_Int16 PageId ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL activatePath( ::sal_Int16 PathIndex, ::sal_Bool Final ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::ui::dialogs::XExecutableDialog
        virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

   protected:
        ~Wizard();

    protected:
        virtual Dialog* createDialog( Window* _pParent );
        virtual void destroyDialog();

    private:
        ::comphelper::ComponentContext                                                          m_aContext;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int16 > >         m_aWizardSteps;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardController >    m_xController;
        OUString                                                                         m_sHelpURL;
    };

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................

#endif // SVT_UNO_WIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

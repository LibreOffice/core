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


#include "wizardshell.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/ui/dialogs/XWizard.hpp>
#include <com/sun/star/ui/dialogs/XWizardController.hpp>
#include <com/sun/star/ui/dialogs/WizardButton.hpp>

#include <cppuhelper/implbase.hxx>
#include <svtools/genericunodialog.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::svt::uno;

namespace {

    using css::uno::Reference;
    using css::uno::XInterface;
    using css::uno::UNO_QUERY;
    using css::uno::UNO_QUERY_THROW;
    using css::uno::UNO_SET_THROW;
    using css::uno::Exception;
    using css::uno::RuntimeException;
    using css::uno::Any;
    using css::uno::makeAny;
    using css::uno::Sequence;
    using css::uno::Type;
    using css::lang::XServiceInfo;
    using css::ui::dialogs::XWizard;
    using css::lang::XInitialization;
    using css::beans::XPropertySetInfo;
    using css::uno::XComponentContext;
    using css::beans::Property;
    using css::lang::IllegalArgumentException;
    using css::ucb::AlreadyInitializedException;
    using css::ui::dialogs::XWizardController;
    using css::ui::dialogs::XWizardPage;
    using css::container::NoSuchElementException;
    using css::util::InvalidStateException;
    using css::awt::XWindow;

    namespace WizardButton = css::ui::dialogs::WizardButton;


    namespace
    {
        WizardButtonFlags lcl_convertWizardButtonToWZB( const sal_Int16 i_nWizardButton )
        {
            switch ( i_nWizardButton )
            {
            case WizardButton::NONE:        return WizardButtonFlags::NONE;
            case WizardButton::NEXT:        return WizardButtonFlags::NEXT;
            case WizardButton::PREVIOUS:    return WizardButtonFlags::PREVIOUS;
            case WizardButton::FINISH:      return WizardButtonFlags::FINISH;
            case WizardButton::CANCEL:      return WizardButtonFlags::CANCEL;
            case WizardButton::HELP:        return WizardButtonFlags::HELP;
            }
            OSL_FAIL( "lcl_convertWizardButtonToWZB: invalid WizardButton constant!" );
            return WizardButtonFlags::NONE;
        }
    }

    typedef ::cppu::ImplInheritanceHelper  <   ::svt::OGenericUnoDialog
                                            ,   ui::dialogs::XWizard
                                            >   Wizard_Base;
    class Wizard;
    typedef ::comphelper::OPropertyArrayUsageHelper< Wizard >  Wizard_PBase;
    class Wizard    : public Wizard_Base
                    , public Wizard_PBase
    {
    public:
        explicit Wizard( const css::uno::Reference< css::uno::XComponentContext >& i_rContext );

        // lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // beans::XPropertySet
        virtual css::uno::Reference< beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // ui::dialogs::XWizard
        virtual OUString SAL_CALL getHelpURL() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpURL( const OUString& _helpurl ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< awt::XWindow > SAL_CALL getDialogWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< ui::dialogs::XWizardPage > SAL_CALL getCurrentPage(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL enableButton( ::sal_Int16 WizardButton, sal_Bool Enable ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDefaultButton( ::sal_Int16 WizardButton ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL travelNext(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL travelPrevious(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL enablePage( ::sal_Int16 PageID, sal_Bool Enable ) throw (container::NoSuchElementException, util::InvalidStateException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTravelUI(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL advanceTo( ::sal_Int16 PageId ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL goBackTo( ::sal_Int16 PageId ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL activatePath( ::sal_Int16 PathIndex, sal_Bool Final ) throw (container::NoSuchElementException, util::InvalidStateException, css::uno::RuntimeException, std::exception) override;

        // ui::dialogs::XExecutableDialog
        virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL execute(  ) throw (css::uno::RuntimeException, std::exception) override;

        // lang::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

   protected:
        virtual ~Wizard();

    protected:
        virtual VclPtr<Dialog> createDialog( vcl::Window* _pParent ) override;
        virtual void destroyDialog() override;

    private:
        css::uno::Sequence< css::uno::Sequence< sal_Int16 > >         m_aWizardSteps;
        css::uno::Reference< ui::dialogs::XWizardController >    m_xController;
        OUString                                            m_sHelpURL;
    };

    Wizard::Wizard( const Reference< XComponentContext >& _rxContext )
        :Wizard_Base( _rxContext )
    {
    }


    Wizard::~Wizard()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore: we're within an dtor,
        // so this virtual-method-call the base class does not work, we're already dead then...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }


    namespace
    {
        static void lcl_checkPaths( const Sequence< Sequence< sal_Int16 > >& i_rPaths, const Reference< XInterface >& i_rContext )
        {
            // need at least one path
            if ( i_rPaths.getLength() == 0 )
                throw IllegalArgumentException( OUString(), i_rContext, 2 );

            // each path must be of length 1, at least
            for ( sal_Int32 i = 0; i < i_rPaths.getLength(); ++i )
            {
                if ( i_rPaths[i].getLength() == 0 )
                    throw IllegalArgumentException( OUString(), i_rContext, 2 );

                // page IDs must be in ascending order
                sal_Int16 nPreviousPageID = i_rPaths[i][0];
                for ( sal_Int32 j=1; j<i_rPaths[i].getLength(); ++j )
                {
                    if ( i_rPaths[i][j] <= nPreviousPageID )
                    {
                        OStringBuffer message;
                        message.append( "Path " );
                        message.append( i );
                        message.append( ": invalid page ID sequence - each page ID must be greater than the previous one." );
                        throw IllegalArgumentException(
                            OStringToOUString( message.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ),
                            i_rContext, 2 );
                    }
                    nPreviousPageID = i_rPaths[i][j];
                }
            }

            // if we have one path, that's okay
            if ( i_rPaths.getLength() == 1 )
                return;

            // if we have multiple paths, they must start with the same page id
            const sal_Int16 nFirstPageId = i_rPaths[0][0];
            for ( sal_Int32 i = 0; i < i_rPaths.getLength(); ++i )
            {
                if ( i_rPaths[i][0] != nFirstPageId )
                    throw IllegalArgumentException(
                        "All paths must start with the same page id.",
                        i_rContext, 2 );
            }
        }
    }


    void SAL_CALL Wizard::initialize( const Sequence< Any >& i_Arguments ) throw (Exception, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInitialized )
            throw AlreadyInitializedException( OUString(), *this );

        if ( i_Arguments.getLength() != 2 )
            throw IllegalArgumentException( OUString(), *this, -1 );

        // the second argument must be a XWizardController, for each constructor
        m_xController.set( i_Arguments[1], UNO_QUERY );
        if ( !m_xController.is() )
            throw IllegalArgumentException( OUString(), *this, 2 );

        // the first arg is either a single path (short[]), or multiple paths (short[][])
        Sequence< sal_Int16 > aSinglePath;
        i_Arguments[0] >>= aSinglePath;
        Sequence< Sequence< sal_Int16 > > aMultiplePaths;
        i_Arguments[0] >>= aMultiplePaths;

        if ( !aMultiplePaths.getLength() )
        {
            aMultiplePaths.realloc(1);
            aMultiplePaths[0] = aSinglePath;
        }
        lcl_checkPaths( aMultiplePaths, *this );
        // if we survived this, the paths are valid, and we're done here ...
        m_aWizardSteps = aMultiplePaths;

        m_bInitialized = true;
    }

    static OString lcl_getHelpId( const OUString& _rHelpURL )
    {
        INetURLObject aHID( _rHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
            return OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 );
        else
            return OUStringToOString( _rHelpURL, RTL_TEXTENCODING_UTF8 );
    }


    static OUString lcl_getHelpURL( const OString& sHelpId )
    {
        OUStringBuffer aBuffer;
        OUString aTmp(
            OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ) );
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INetProtocol::NotValid )
            aBuffer.append( INET_HID_SCHEME );
        aBuffer.append( aTmp.getStr() );
        return aBuffer.makeStringAndClear();
    }


    VclPtr<Dialog> Wizard::createDialog( vcl::Window* i_pParent )
    {
        VclPtrInstance<WizardShell> pDialog( i_pParent, m_xController, m_aWizardSteps );
        pDialog->SetHelpId(  lcl_getHelpId( m_sHelpURL ) );
        pDialog->setTitleBase( m_sTitle );
        return pDialog.get();
    }


    void Wizard::destroyDialog()
    {
        if ( m_pDialog )
            m_sHelpURL = lcl_getHelpURL( m_pDialog->GetHelpId() );

        Wizard_Base::destroyDialog();
    }


    OUString SAL_CALL Wizard::getImplementationName() throw(RuntimeException, std::exception)
    {
        return OUString("com.sun.star.comp.svtools.uno.Wizard");
    }


    Sequence< OUString > SAL_CALL Wizard::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        Sequence< OUString > aServices { "com.sun.star.ui.dialogs.Wizard" };
        return aServices;
    }


    Reference< XPropertySetInfo > SAL_CALL Wizard::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        return createPropertySetInfo( getInfoHelper() );
    }


    ::cppu::IPropertyArrayHelper& SAL_CALL Wizard::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* Wizard::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    OUString SAL_CALL Wizard::getHelpURL() throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pDialog )
            return m_sHelpURL;

        return lcl_getHelpURL( m_pDialog->GetHelpId() );
    }


    void SAL_CALL Wizard::setHelpURL( const OUString& i_HelpURL ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pDialog )
            m_sHelpURL = i_HelpURL;
        else
            m_pDialog->SetHelpId( lcl_getHelpId( i_HelpURL ) );
    }


    Reference< XWindow > SAL_CALL Wizard::getDialogWindow() throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        ENSURE_OR_RETURN( m_pDialog, "Wizard::getDialogWindow: illegal call (execution did not start, yet)!", nullptr );
        return Reference< XWindow >( m_pDialog->GetComponentInterface(), UNO_QUERY );
    }


    void SAL_CALL Wizard::enableButton( ::sal_Int16 i_WizardButton, sal_Bool i_Enable ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enableButtons: invalid dialog implementation!" );

        pWizardImpl->enableButtons( lcl_convertWizardButtonToWZB( i_WizardButton ), i_Enable );
    }


    void SAL_CALL Wizard::setDefaultButton( ::sal_Int16 i_WizardButton ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::setDefaultButton: invalid dialog implementation!" );

        pWizardImpl->defaultButton( lcl_convertWizardButtonToWZB( i_WizardButton ) );
    }


    sal_Bool SAL_CALL Wizard::travelNext(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelNext: invalid dialog implementation!" );

        return pWizardImpl->travelNext();
    }


    sal_Bool SAL_CALL Wizard::travelPrevious(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelPrevious: invalid dialog implementation!" );

        return pWizardImpl->travelPrevious();
    }


    void SAL_CALL Wizard::enablePage( ::sal_Int16 i_PageID, sal_Bool i_Enable ) throw (NoSuchElementException, InvalidStateException, RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enablePage: invalid dialog implementation!" );

        if ( !pWizardImpl->knowsPage( i_PageID ) )
            throw NoSuchElementException( OUString(), *this );

        if ( i_PageID == pWizardImpl->getCurrentPage() )
            throw InvalidStateException( OUString(), *this );

        pWizardImpl->enablePage( i_PageID, i_Enable );
    }


    void SAL_CALL Wizard::updateTravelUI(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::updateTravelUI: invalid dialog implementation!" );

        pWizardImpl->updateTravelUI();
    }


    sal_Bool SAL_CALL Wizard::advanceTo( ::sal_Int16 i_PageId ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::advanceTo: invalid dialog implementation!" );

        return pWizardImpl->advanceTo( i_PageId );
    }


    sal_Bool SAL_CALL Wizard::goBackTo( ::sal_Int16 i_PageId ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::goBackTo: invalid dialog implementation!" );

        return pWizardImpl->goBackTo( i_PageId );
    }


    Reference< XWizardPage > SAL_CALL Wizard::getCurrentPage(  ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN( pWizardImpl, "Wizard::getCurrentPage: invalid dialog implementation!", Reference< XWizardPage >() );

        return pWizardImpl->getCurrentWizardPage();
    }


    void SAL_CALL Wizard::activatePath( ::sal_Int16 i_PathIndex, sal_Bool i_Final ) throw (NoSuchElementException, InvalidStateException, RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( ( i_PathIndex < 0 ) || ( i_PathIndex >= m_aWizardSteps.getLength() ) )
            throw NoSuchElementException( OUString(), *this );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog.get() );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::activatePath: invalid dialog implementation!" );

        pWizardImpl->activatePath( i_PathIndex, i_Final );
    }


    void SAL_CALL Wizard::setTitle( const OUString& i_Title ) throw (RuntimeException, std::exception)
    {
        // simply disambiguate
        Wizard_Base::OGenericUnoDialog::setTitle( i_Title );
    }


    ::sal_Int16 SAL_CALL Wizard::execute(  ) throw (RuntimeException, std::exception)
    {
        return Wizard_Base::OGenericUnoDialog::execute();
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svtools_uno_Wizard_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new Wizard(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

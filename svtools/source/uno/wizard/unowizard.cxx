/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "precompiled_svtools.hxx"

#include "../unowizard.hxx"
#include "wizardshell.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/ui/dialogs/XWizardController.hpp>
#include <com/sun/star/ui/dialogs/WizardButton.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

//......................................................................................................................
namespace svt { namespace uno
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::ui::dialogs::XWizard;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::ui::dialogs::XWizardController;
    using ::com::sun::star::ui::dialogs::XWizardPage;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::util::InvalidStateException;
    using ::com::sun::star::awt::XWindow;
    /** === end UNO using === **/
    namespace WizardButton = ::com::sun::star::ui::dialogs::WizardButton;

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        sal_uInt32 lcl_convertWizardButtonToWZB( const sal_Int16 i_nWizardButton )
        {
            switch ( i_nWizardButton )
            {
            case WizardButton::NONE:        return WZB_NONE;
            case WizardButton::NEXT:        return WZB_NEXT;
            case WizardButton::PREVIOUS:    return WZB_PREVIOUS;
            case WizardButton::FINISH:      return WZB_FINISH;
            case WizardButton::CANCEL:      return WZB_CANCEL;
            case WizardButton::HELP:        return WZB_HELP;
            }
            OSL_FAIL( "lcl_convertWizardButtonToWZB: invalid WizardButton constant!" );
            return WZB_NONE;
        }
    }

    //==================================================================================================================
    //= Wizard - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    Wizard::Wizard( const Reference< XComponentContext >& _rxContext )
        :Wizard_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    Wizard::~Wizard()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
        // so this virtual-method-call the base class does does not work, we're already dead then ...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL Wizard::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new Wizard( _rxContext ) );
    }

    //--------------------------------------------------------------------
    namespace
    {
        static void lcl_checkPaths( const Sequence< Sequence< sal_Int16 > >& i_rPaths, const Reference< XInterface >& i_rContext )
        {
            // need at least one path
            if ( i_rPaths.getLength() == 0 )
                throw IllegalArgumentException( ::rtl::OUString(), i_rContext, 2 );

            // each path must be of length 1, at least
            for ( sal_Int32 i = 0; i < i_rPaths.getLength(); ++i )
            {
                if ( i_rPaths[i].getLength() == 0 )
                    throw IllegalArgumentException( ::rtl::OUString(), i_rContext, 2 );

                // page IDs must be in ascending order
                sal_Int16 nPreviousPageID = i_rPaths[i][0];
                for ( sal_Int32 j=1; j<i_rPaths[i].getLength(); ++j )
                {
                    if ( i_rPaths[i][j] <= nPreviousPageID )
                    {
                        ::rtl::OStringBuffer message;
                        message.append( "Path " );
                        message.append( i );
                        message.append( ": invalid page ID sequence - each page ID must be greater than the previous one." );
                        throw IllegalArgumentException(
                            ::rtl::OStringToOUString( message.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ),
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
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "All paths must start with the same page id." ) ),
                        i_rContext, 2 );
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL Wizard::initialize( const Sequence< Any >& i_Arguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInitialized )
            throw AlreadyInitializedException( ::rtl::OUString(), *this );

        if ( i_Arguments.getLength() != 2 )
            throw IllegalArgumentException( ::rtl::OUString(), *this, -1 );

        // the second argument must be a XWizardController, for each constructor
        m_xController.set( i_Arguments[1], UNO_QUERY );
        if ( !m_xController.is() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );

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

    //--------------------------------------------------------------------
    Dialog* Wizard::createDialog( Window* i_pParent )
    {
        WizardShell* pDialog( new WizardShell( i_pParent, this, m_xController, m_aWizardSteps ) );
        pDialog->SetSmartHelpId( SmartId( m_sHelpURL ) );
        pDialog->setTitleBase( m_sTitle );
        return pDialog;
    }

    //--------------------------------------------------------------------
    void Wizard::destroyDialog()
    {
        if ( m_pDialog )
            m_sHelpURL = m_pDialog->GetSmartHelpId().GetStr();

        Wizard_Base::destroyDialog();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Wizard::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.uno.Wizard" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL Wizard::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(1);
        aServices[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.Wizard" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Wizard::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL Wizard::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL Wizard::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL Wizard::getInfoHelper()
    {
        return *const_cast< Wizard* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* Wizard::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Wizard::getHelpURL() throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pDialog )
            return m_sHelpURL;

        const SmartId aSmartId( m_pDialog->GetSmartHelpId() );
        return aSmartId.GetStr();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::setHelpURL( const ::rtl::OUString& i_HelpURL ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pDialog )
            m_sHelpURL = i_HelpURL;
        else
            m_pDialog->SetSmartHelpId( SmartId( i_HelpURL ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWindow > SAL_CALL Wizard::getDialogWindow() throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        ENSURE_OR_RETURN( m_pDialog, "Wizard::getDialogWindow: illegal call (execution did not start, yet)!", NULL );
        return Reference< XWindow >( m_pDialog->GetComponentInterface(), UNO_QUERY );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::enableButton( ::sal_Int16 i_WizardButton, ::sal_Bool i_Enable ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enableButtons: invalid dialog implementation!" );

        pWizardImpl->enableButtons( lcl_convertWizardButtonToWZB( i_WizardButton ), i_Enable );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::setDefaultButton( ::sal_Int16 i_WizardButton ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::setDefaultButton: invalid dialog implementation!" );

        pWizardImpl->defaultButton( lcl_convertWizardButtonToWZB( i_WizardButton ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL Wizard::travelNext(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelNext: invalid dialog implementation!" );

        return pWizardImpl->travelNext();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL Wizard::travelPrevious(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelPrevious: invalid dialog implementation!" );

        return pWizardImpl->travelPrevious();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::enablePage( ::sal_Int16 i_PageID, ::sal_Bool i_Enable ) throw (NoSuchElementException, InvalidStateException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enablePage: invalid dialog implementation!" );

        if ( !pWizardImpl->knowsPage( i_PageID ) )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        if ( i_PageID == pWizardImpl->getCurrentPage() )
            throw InvalidStateException( ::rtl::OUString(), *this );

        pWizardImpl->enablePage( i_PageID, i_Enable );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::updateTravelUI(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::updateTravelUI: invalid dialog implementation!" );

        pWizardImpl->updateTravelUI();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL Wizard::advanceTo( ::sal_Int16 i_PageId ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::advanceTo: invalid dialog implementation!" );

        return pWizardImpl->advanceTo( i_PageId );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL Wizard::goBackTo( ::sal_Int16 i_PageId ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::goBackTo: invalid dialog implementation!" );

        return pWizardImpl->goBackTo( i_PageId );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWizardPage > SAL_CALL Wizard::getCurrentPage(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN( pWizardImpl, "Wizard::getCurrentPage: invalid dialog implementation!", Reference< XWizardPage >() );

        return pWizardImpl->getCurrentWizardPage();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::activatePath( ::sal_Int16 i_PathIndex, ::sal_Bool i_Final ) throw (NoSuchElementException, InvalidStateException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( ( i_PathIndex < 0 ) || ( i_PathIndex >= m_aWizardSteps.getLength() ) )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        WizardShell* pWizardImpl = dynamic_cast< WizardShell* >( m_pDialog );
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::activatePath: invalid dialog implementation!" );

        pWizardImpl->activatePath( i_PathIndex, i_Final );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Wizard::setTitle( const ::rtl::OUString& i_Title ) throw (RuntimeException)
    {
        // simply disambiguate
        Wizard_Base::OGenericUnoDialog::setTitle( i_Title );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int16 SAL_CALL Wizard::execute(  ) throw (RuntimeException)
    {
        return Wizard_Base::OGenericUnoDialog::execute();
    }

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

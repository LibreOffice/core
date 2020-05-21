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

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/ui/dialogs/XWizard.hpp>
#include <com/sun/star/ui/dialogs/XWizardController.hpp>
#include <com/sun/star/ui/dialogs/WizardButton.hpp>
#include <com/sun/star/util/InvalidStateException.hpp>

#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <svtools/genericunodialog.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::svt::uno;

namespace {

    using css::uno::Reference;
    using css::uno::XInterface;
    using css::uno::UNO_QUERY;
    using css::uno::Any;
    using css::uno::Sequence;
    using css::ui::dialogs::XWizard;
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
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // beans::XPropertySet
        virtual css::uno::Reference< beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // ui::dialogs::XWizard
        virtual OUString SAL_CALL getHelpURL() override;
        virtual void SAL_CALL setHelpURL( const OUString& _helpurl ) override;
        virtual css::uno::Reference< awt::XWindow > SAL_CALL getDialogWindow() override;
        virtual css::uno::Reference< ui::dialogs::XWizardPage > SAL_CALL getCurrentPage(  ) override;
        virtual void SAL_CALL enableButton( ::sal_Int16 WizardButton, sal_Bool Enable ) override;
        virtual void SAL_CALL setDefaultButton( ::sal_Int16 WizardButton ) override;
        virtual sal_Bool SAL_CALL travelNext(  ) override;
        virtual sal_Bool SAL_CALL travelPrevious(  ) override;
        virtual void SAL_CALL enablePage( ::sal_Int16 PageID, sal_Bool Enable ) override;
        virtual void SAL_CALL updateTravelUI(  ) override;
        virtual sal_Bool SAL_CALL advanceTo( ::sal_Int16 PageId ) override;
        virtual sal_Bool SAL_CALL goBackTo( ::sal_Int16 PageId ) override;
        virtual void SAL_CALL activatePath( ::sal_Int16 PathIndex, sal_Bool Final ) override;

        // ui::dialogs::XExecutableDialog
        virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
        virtual ::sal_Int16 SAL_CALL execute(  ) override;

        // lang::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

   protected:
        virtual ~Wizard() override;

    protected:
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    private:
        css::uno::Sequence< css::uno::Sequence< sal_Int16 > >         m_aWizardSteps;
        css::uno::Reference< ui::dialogs::XWizardController >    m_xController;
        OUString                                            m_sHelpURL;
    };

    Wizard::Wizard( const Reference< XComponentContext >& _rxContext )
        :Wizard_Base( _rxContext )
    {
    }

    OUString lcl_getHelpURL( const OString& sHelpId )
    {
        OUStringBuffer aBuffer;
        OUString aTmp(
            OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ) );
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INetProtocol::NotValid )
            aBuffer.append( INET_HID_SCHEME );
        aBuffer.append( aTmp );
        return aBuffer.makeStringAndClear();
    }

    Wizard::~Wizard()
    {
        if (m_xDialog)
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (m_xDialog)
            {
                m_sHelpURL = lcl_getHelpURL(m_xDialog->get_help_id());
                destroyDialog();
            }
        }
    }

    void lcl_checkPaths( const Sequence< Sequence< sal_Int16 > >& i_rPaths, const Reference< XInterface >& i_rContext )
    {
        // need at least one path
        if ( !i_rPaths.hasElements() )
            throw IllegalArgumentException( OUString(), i_rContext, 2 );

        // each path must be of length 1, at least
        sal_Int32 i = 0;
        for ( const Sequence< sal_Int16 >& rPath : i_rPaths )
        {
            if ( !rPath.hasElements() )
                throw IllegalArgumentException( OUString(), i_rContext, 2 );

            // page IDs must be in ascending order
            auto pPageId = std::adjacent_find(rPath.begin(), rPath.end(), std::greater_equal<sal_Int16>());
            if (pPageId != rPath.end())
            {
                throw IllegalArgumentException(
                    "Path " + OUString::number(i)
                    + ": invalid page ID sequence - each page ID must be greater than the previous one.",
                    i_rContext, 2 );
            }
            ++i;
        }

        // if we have one path, that's okay
        if ( i_rPaths.getLength() == 1 )
            return;

        // if we have multiple paths, they must start with the same page id
        const sal_Int16 nFirstPageId = i_rPaths[0][0];
        if (std::any_of(i_rPaths.begin(), i_rPaths.end(),
                [nFirstPageId](const Sequence< sal_Int16 >& rPath) { return rPath[0] != nFirstPageId; }))
            throw IllegalArgumentException(
                "All paths must start with the same page id.",
                i_rContext, 2 );
    }

    void SAL_CALL Wizard::initialize( const Sequence< Any >& i_Arguments )
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

        if ( !aMultiplePaths.hasElements() )
        {
            aMultiplePaths.realloc(1);
            aMultiplePaths[0] = aSinglePath;
        }
        lcl_checkPaths( aMultiplePaths, *this );
        // if we survived this, the paths are valid, and we're done here ...
        m_aWizardSteps = aMultiplePaths;

        m_bInitialized = true;
    }

    OString lcl_getHelpId( const OUString& _rHelpURL )
    {
        INetURLObject aHID( _rHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
            return OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 );
        else
            return OUStringToOString( _rHelpURL, RTL_TEXTENCODING_UTF8 );
    }

    std::unique_ptr<weld::DialogController> Wizard::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        auto xDialog = std::make_unique<WizardShell>(Application::GetFrameWeld(rParent), m_xController, m_aWizardSteps);
        xDialog->set_help_id(lcl_getHelpId(m_sHelpURL));
        xDialog->setTitleBase( m_sTitle );
        return xDialog;
    }

    OUString SAL_CALL Wizard::getImplementationName()
    {
        return "com.sun.star.comp.svtools.uno.Wizard";
    }

    Sequence< OUString > SAL_CALL Wizard::getSupportedServiceNames()
    {
        return { "com.sun.star.ui.dialogs.Wizard" };
    }

    Reference< XPropertySetInfo > SAL_CALL Wizard::getPropertySetInfo()
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

    OUString SAL_CALL Wizard::getHelpURL()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if (!m_xDialog)
            return m_sHelpURL;

        return lcl_getHelpURL(m_xDialog->get_help_id());
    }

    void SAL_CALL Wizard::setHelpURL( const OUString& i_HelpURL )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if (!m_xDialog)
            m_sHelpURL = i_HelpURL;
        else
            m_xDialog->set_help_id(lcl_getHelpId(i_HelpURL));
    }

    Reference< XWindow > SAL_CALL Wizard::getDialogWindow()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        ENSURE_OR_RETURN( m_xDialog, "Wizard::getDialogWindow: illegal call (execution did not start, yet)!", nullptr );
        return m_xDialog->getDialog()->GetXWindow();
    }

    void SAL_CALL Wizard::enableButton( ::sal_Int16 i_WizardButton, sal_Bool i_Enable )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enableButtons: invalid dialog implementation!" );

        pWizardImpl->enableButtons( lcl_convertWizardButtonToWZB( i_WizardButton ), i_Enable );
    }

    void SAL_CALL Wizard::setDefaultButton( ::sal_Int16 i_WizardButton )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::setDefaultButton: invalid dialog implementation!" );

        pWizardImpl->defaultButton( lcl_convertWizardButtonToWZB( i_WizardButton ) );
    }

    sal_Bool SAL_CALL Wizard::travelNext(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelNext: invalid dialog implementation!" );

        return pWizardImpl->travelNext();
    }

    sal_Bool SAL_CALL Wizard::travelPrevious(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::travelPrevious: invalid dialog implementation!" );

        return pWizardImpl->travelPrevious();
    }

    void SAL_CALL Wizard::enablePage( ::sal_Int16 i_PageID, sal_Bool i_Enable )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::enablePage: invalid dialog implementation!" );

        if ( !pWizardImpl->knowsPage( i_PageID ) )
            throw NoSuchElementException( OUString(), *this );

        if ( i_PageID == pWizardImpl->getCurrentPage() )
            throw InvalidStateException( OUString(), *this );

        pWizardImpl->enablePage( i_PageID, i_Enable );
    }

    void SAL_CALL Wizard::updateTravelUI(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::updateTravelUI: invalid dialog implementation!" );

        pWizardImpl->updateTravelUI();
    }

    sal_Bool SAL_CALL Wizard::advanceTo( ::sal_Int16 i_PageId )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::advanceTo: invalid dialog implementation!" );

        return pWizardImpl->advanceTo( i_PageId );
    }

    sal_Bool SAL_CALL Wizard::goBackTo( ::sal_Int16 i_PageId )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_FALSE( pWizardImpl, "Wizard::goBackTo: invalid dialog implementation!" );

        return pWizardImpl->goBackTo( i_PageId );
    }

    Reference< XWizardPage > SAL_CALL Wizard::getCurrentPage(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN( pWizardImpl, "Wizard::getCurrentPage: invalid dialog implementation!", Reference< XWizardPage >() );

        return pWizardImpl->getCurrentWizardPage();
    }

    void SAL_CALL Wizard::activatePath( ::sal_Int16 i_PathIndex, sal_Bool i_Final )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( ( i_PathIndex < 0 ) || ( i_PathIndex >= m_aWizardSteps.getLength() ) )
            throw NoSuchElementException( OUString(), *this );

        WizardShell* pWizardImpl = dynamic_cast<WizardShell*>(m_xDialog.get());
        ENSURE_OR_RETURN_VOID( pWizardImpl, "Wizard::activatePath: invalid dialog implementation!" );

        pWizardImpl->activatePath( i_PathIndex, i_Final );
    }

    void SAL_CALL Wizard::setTitle( const OUString& i_Title )
    {
        // simply disambiguate
        Wizard_Base::OGenericUnoDialog::setTitle( i_Title );
    }

    ::sal_Int16 SAL_CALL Wizard::execute(  )
    {
        return Wizard_Base::OGenericUnoDialog::execute();
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svtools_uno_Wizard_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new Wizard(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

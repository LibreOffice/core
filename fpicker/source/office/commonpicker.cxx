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


#include "commonpicker.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/weakeventlistener.hxx>
#include <comphelper/types.hxx>
#include <vcl/msgbox.hxx>
#include "iodlg.hxx"


namespace svt
{


#define PROPERTY_ID_HELPURL     1
#define PROPERTY_ID_WINDOW      2

    // using --------------------------------------------------------------

    using namespace     ::com::sun::star::lang;
    using namespace     ::com::sun::star::ui::dialogs;
    using namespace     ::com::sun::star::uno;
    using namespace     ::com::sun::star::beans;
    using namespace     ::comphelper;


    OCommonPicker::OCommonPicker()
        :OCommonPicker_Base( m_aMutex )
        ,OPropertyContainer( GetBroadcastHelper() )
        ,m_pDlg( nullptr )
        ,m_nCancelEvent( nullptr )
        ,m_bExecuting( false )
    {
        // the two properties we have
        registerProperty(
            "HelpURL", PROPERTY_ID_HELPURL,
            PropertyAttribute::TRANSIENT,
            &m_sHelpURL, cppu::UnoType<decltype(m_sHelpURL)>::get()
        );

        registerProperty(
            "Window", PROPERTY_ID_WINDOW,
            PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY,
            &m_xWindow, cppu::UnoType<decltype(m_xWindow)>::get()
        );
    }


    OCommonPicker::~OCommonPicker()
    {
        if ( !GetBroadcastHelper().bDisposed )
        {
            acquire();
            dispose();
        }
    }


    // disambiguate XInterface

    IMPLEMENT_FORWARD_XINTERFACE2( OCommonPicker, OCommonPicker_Base, OPropertyContainer )


    // disambiguate XTypeProvider

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCommonPicker, OCommonPicker_Base, OPropertyContainer )


    // XComponent related methods

    void OCommonPicker::checkAlive() const
    {
        if ( GetBroadcastHelper().bInDispose || GetBroadcastHelper().bDisposed )
            throw DisposedException();
    }

    void OCommonPicker::prepareDialog()
    {
        if(createPicker())
        {
            // set the title
            if ( !m_aTitle.isEmpty() )
                m_pDlg->SetText( m_aTitle );
        }
    }


    void SAL_CALL OCommonPicker::disposing()
    {
        SolarMutexGuard aGuard;

        stopWindowListening();

        if ( m_nCancelEvent )
            Application::RemoveUserEvent( m_nCancelEvent );

        {
            ::osl::MutexGuard aOwnGuard( m_aMutex );
            if ( m_bExecuting && m_pDlg )
                m_pDlg->EndDialog();
        }

        m_pDlg.disposeAndClear();
        m_xWindow = nullptr;
        m_xDialogParent = nullptr;
    }


    void OCommonPicker::stopWindowListening()
    {
        disposeComponent( m_xWindowListenerAdapter );
        disposeComponent( m_xParentListenerAdapter );
    }


    // XEventListener

    void SAL_CALL OCommonPicker::disposing( const EventObject& _rSource ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;
        bool bDialogDying = _rSource.Source == m_xWindow;
        bool bParentDying = _rSource.Source == m_xDialogParent;

        if ( bDialogDying || bParentDying )
        {
            stopWindowListening();

            if ( !bDialogDying )    // it's the parent which is dying -> delete the dialog
                m_pDlg.disposeAndClear();
            else
                m_pDlg.clear();

            m_xWindow = nullptr;
            m_xDialogParent = nullptr;
        }
        else
        {
            OSL_FAIL( "OCommonPicker::disposing: where did this come from?" );
        }
    }


    // property set related methods

    ::cppu::IPropertyArrayHelper* OCommonPicker::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new cppu::OPropertyArrayHelper( aProps );
    }


    ::cppu::IPropertyArrayHelper& SAL_CALL OCommonPicker::getInfoHelper()
    {
        return *getArrayHelper();
    }


    Reference< XPropertySetInfo > SAL_CALL OCommonPicker::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
    }


    void SAL_CALL OCommonPicker::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception, std::exception)
    {
        OPropertyContainer::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

        // if the HelpURL changed, forward this to the dialog
        if ( PROPERTY_ID_HELPURL == _nHandle )
            if ( m_pDlg )
                OControlAccess::setHelpURL( m_pDlg, m_sHelpURL, false );
    }



    bool OCommonPicker::createPicker()
    {
        if ( !m_pDlg )
        {
            m_pDlg.reset( implCreateDialog( VCLUnoHelper::GetWindow( m_xDialogParent ) ) );
            DBG_ASSERT( m_pDlg, "OCommonPicker::createPicker: invalid dialog returned!" );

            if ( m_pDlg )
            {
                // synchronize the help id of the dialog with out help URL property
                if ( !m_sHelpURL.isEmpty() )
                {   // somebody already set the help URL while we had no dialog yet
                    OControlAccess::setHelpURL( m_pDlg, m_sHelpURL, false );
                }
                else
                {
                    m_sHelpURL = OControlAccess::getHelpURL( m_pDlg, false );
                }

                m_xWindow = VCLUnoHelper::GetInterface( m_pDlg );

                // add as event listener to the window
                Reference< XComponent > xWindowComp( m_xWindow, UNO_QUERY );
                OSL_ENSURE( xWindowComp.is(), "OCommonPicker::createFileDialog: invalid window component!" );
                if ( xWindowComp.is() )
                {
                    m_xWindowListenerAdapter = new OWeakEventListenerAdapter( this, xWindowComp );
                        // the adapter will add itself as listener, and forward notifications
                }

                // _and_ add as event listener to the parent - in case the parent is destroyed
                // before we are disposed, our disposal would access dead VCL windows then ....
                m_xDialogParent = VCLUnoHelper::GetInterface( m_pDlg->GetParent() );
                xWindowComp.set(m_xDialogParent, css::uno::UNO_QUERY);
                OSL_ENSURE( xWindowComp.is() || !m_pDlg->GetParent(), "OCommonPicker::createFileDialog: invalid window component (the parent this time)!" );
                if ( xWindowComp.is() )
                {
                    m_xParentListenerAdapter = new OWeakEventListenerAdapter( this, xWindowComp );
                        // the adapter will add itself as listener, and forward notifications
                }
            }
        }

        return nullptr != m_pDlg;
    }


    // XControlAccess functions

    void SAL_CALL OCommonPicker::setControlProperty( const OUString& aControlName, const OUString& aControlProperty, const Any& aValue ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
            aAccess.setControlProperty( aControlName, aControlProperty, aValue );
        }
    }


    Any SAL_CALL OCommonPicker::getControlProperty( const OUString& aControlName, const OUString& aControlProperty ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
            return aAccess.getControlProperty( aControlName, aControlProperty );
        }

        return Any();
    }


    // XControlInformation functions

    Sequence< OUString > SAL_CALL OCommonPicker::getSupportedControls(  ) throw (RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
            return aAccess.getSupportedControls( );
        }

        return Sequence< OUString >();
    }


    sal_Bool SAL_CALL OCommonPicker::isControlSupported( const OUString& aControlName ) throw (RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            return svt::OControlAccess::isControlSupported( aControlName );
        }

        return sal_False;
    }


    Sequence< OUString > SAL_CALL OCommonPicker::getSupportedControlProperties( const OUString& aControlName ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
            return aAccess.getSupportedControlProperties( aControlName );
        }

        return Sequence< OUString >();
    }


    sal_Bool SAL_CALL OCommonPicker::isControlPropertySupported( const OUString& aControlName, const OUString& aControlProperty ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
            return aAccess.isControlPropertySupported( aControlName, aControlProperty );
        }

        return sal_False;
    }


    // XExecutableDialog functions

    void SAL_CALL OCommonPicker::setTitle( const OUString& _rTitle ) throw( RuntimeException, std::exception )
    {
        SolarMutexGuard aGuard;
        m_aTitle = _rTitle;
    }


    sal_Int16 OCommonPicker::execute() throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        prepareDialog();

        {
            ::osl::MutexGuard aOwnGuard( m_aMutex );
            m_bExecuting = true;
        }
        sal_Int16 nResult = implExecutePicker();
        {
            ::osl::MutexGuard aOwnGuard( m_aMutex );
            m_bExecuting = false;
        }

        return nResult;
    }


    // XCancellable functions

    void SAL_CALL OCommonPicker::cancel(  ) throw (RuntimeException, std::exception)
    {
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_nCancelEvent )
                // nothing to do - the event for cancelling the dialog is already on the way
                return;
        }

        // The thread which executes our dialog has locked the solar mutex for
        // sure. Cancelling the dialog should be done with a locked solar mutex, too.
        // Thus we post ourself a message for cancelling the dialog. This way, the message
        // is either handled in the thread which opened the dialog (which may even be
        // this thread here), or, if no dialog is open, in the thread doing scheduling
        // currently. Both is okay for us ....

        // Note that we could do check if we are really executing the dialog currently.
        // but the information would be potentially obsolete at the moment our event
        // arrives, so we need to check it there, anyway ...
        m_nCancelEvent = Application::PostUserEvent( LINK( this, OCommonPicker, OnCancelPicker ) );
    }


    IMPL_LINK_NOARG_TYPED(OCommonPicker, OnCancelPicker, void*, void)
    {
        // By definition, the solar mutex is locked when we arrive here. Note that this
        // is important, as for instance the consistency of m_pDlg depends on this mutex.
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nCancelEvent = nullptr;

        if ( !m_bExecuting )
            // nothing to do. This may be because the dialog was canceled after our cancel method
            // posted this async event, or because somebody called cancel without the dialog
            // being executed at this time.
            return;

        OSL_ENSURE( getDialog(), "OCommonPicker::OnCancelPicker: executing, but no dialog!" );
        if ( getDialog() )
            getDialog()->EndDialog();
    }


    // XInitialization functions

    void SAL_CALL OCommonPicker::initialize( const Sequence< Any >& _rArguments )
        throw ( Exception, RuntimeException, std::exception )
    {
        checkAlive();

        OUString sSettingName;
        Any             aSettingValue;

        PropertyValue   aPropArg;
        NamedValue      aPairArg;


        const Any* pArguments       = _rArguments.getConstArray();
        const Any* pArgumentsEnd    = _rArguments.getConstArray() + _rArguments.getLength();
        for (   const Any* pArgument = pArguments;
                pArgument != pArgumentsEnd;
                ++pArgument
            )
        {
            if ( *pArgument >>= aPropArg )
            {
                if ( aPropArg.Name.isEmpty())
                    continue;

                sSettingName = aPropArg.Name;
                aSettingValue = aPropArg.Value;
            }
            else if ( *pArgument >>= aPairArg )
            {
                if ( aPairArg.Name.isEmpty())
                    continue;

                sSettingName = aPairArg.Name;
                aSettingValue = aPairArg.Value;


            }
            else
            {
                OSL_FAIL(
                    OString(
                        "OCommonPicker::initialize: unknown argument type at position "
                        + OString::number(pArguments - _rArguments.getConstArray())).getStr());
                continue;
            }

            bool bKnownSetting =
                implHandleInitializationArgument( sSettingName, aSettingValue );
            DBG_ASSERT( bKnownSetting,
                OString(
                    "OCommonPicker::initialize: unknown argument \""
                    + OString(sSettingName.getStr(), sSettingName.getLength(), osl_getThreadTextEncoding())
                    + "\"!").getStr() );
        }
    }


    bool OCommonPicker::implHandleInitializationArgument( const OUString& _rName, const Any& _rValue )
    {
        bool bKnown = true;
        if ( _rName == "ParentWindow" )
        {
            m_xDialogParent.clear();
            OSL_VERIFY( _rValue >>= m_xDialogParent );
            OSL_ENSURE( VCLUnoHelper::GetWindow( m_xDialogParent ), "OCommonPicker::implHandleInitializationArgument: invalid parent window given!" );
        }
        else
            bKnown = false;
        return bKnown;
    }


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

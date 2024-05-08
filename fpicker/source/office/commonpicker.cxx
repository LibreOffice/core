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
#include "fpdialogbase.hxx"
#include "OfficeControlAccess.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/weakeventlistener.hxx>
#include <comphelper/types.hxx>


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
        ,m_nCancelEvent( nullptr )
        ,m_bExecuting( false )
    {
        // the two properties we have
        registerProperty(
            u"HelpURL"_ustr, PROPERTY_ID_HELPURL,
            PropertyAttribute::TRANSIENT,
            &m_sHelpURL, cppu::UnoType<decltype(m_sHelpURL)>::get()
        );

        registerProperty(
            u"Window"_ustr, PROPERTY_ID_WINDOW,
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
                m_xDlg->set_title(m_aTitle);
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
            if ( m_bExecuting && m_xDlg )
                m_xDlg->response(RET_CANCEL);
        }

        m_xDlg.reset();
        m_xWindow = nullptr;
        m_xDialogParent = nullptr;
    }


    void OCommonPicker::stopWindowListening()
    {
        disposeComponent( m_xWindowListenerAdapter );
        disposeComponent( m_xParentListenerAdapter );
    }

    // XEventListener
    void SAL_CALL OCommonPicker::disposing( const EventObject& _rSource )
    {
        SolarMutexGuard aGuard;
        bool bDialogDying = _rSource.Source == m_xWindow;
        bool bParentDying = _rSource.Source == m_xDialogParent;

        if ( bDialogDying || bParentDying )
        {
            stopWindowListening();

            SAL_WARN_IF(bDialogDying && m_bExecuting, "fpicker.office", "unexpected disposing before response" );

            // it's the parent which is dying -> delete the dialog
            {
                ::osl::MutexGuard aOwnGuard(m_aMutex);
                if (m_bExecuting && m_xDlg)
                    m_xDlg->response(RET_CANCEL);
            }

            m_xDlg.reset();
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

    Reference< XPropertySetInfo > SAL_CALL OCommonPicker::getPropertySetInfo(  )
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
    }

    void SAL_CALL OCommonPicker::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
    {
        OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, rValue);

        // if the HelpURL changed, forward this to the dialog
        if (PROPERTY_ID_HELPURL == nHandle && m_xDlg)
        {
            ::svt::OControlAccess aAccess(m_xDlg.get(), m_xDlg->GetView());
            aAccess.setHelpURL(m_xDlg->getDialog(), m_sHelpURL);
        }
    }

    bool OCommonPicker::createPicker()
    {
        if ( !m_xDlg )
        {
            m_xDlg = implCreateDialog(Application::GetFrameWeld(m_xDialogParent));
            SAL_WARN_IF( !m_xDlg, "fpicker.office", "OCommonPicker::createPicker: invalid dialog returned!" );

            if ( m_xDlg )
            {
                weld::Dialog* pDlg = m_xDlg->getDialog();

                ::svt::OControlAccess aAccess(m_xDlg.get(), m_xDlg->GetView());
                // synchronize the help id of the dialog without help URL property
                if ( !m_sHelpURL.isEmpty() )
                {   // somebody already set the help URL while we had no dialog yet
                    aAccess.setHelpURL(pDlg, m_sHelpURL);
                }
                else
                {
                    m_sHelpURL = aAccess.getHelpURL(pDlg);
                }

                m_xWindow = pDlg->GetXWindow();

                // add as event listener to the window
                OSL_ENSURE( m_xWindow.is(), "OCommonPicker::createFileDialog: invalid window component!" );
                if ( m_xWindow.is() )
                {
                    m_xWindowListenerAdapter = new OWeakEventListenerAdapter( this, m_xWindow );
                        // the adapter will add itself as listener, and forward notifications
                }

                VclPtr<vcl::Window> xVclDialog(VCLUnoHelper::GetWindow(m_xWindow));
                if (xVclDialog) // this block is quite possibly unnecessary by now
                {
                    // _and_ add as event listener to the parent - in case the parent is destroyed
                    // before we are disposed, our disposal would access dead VCL windows then...
                    m_xDialogParent = VCLUnoHelper::GetInterface(xVclDialog->GetParent());
                    OSL_ENSURE(m_xDialogParent.is() || !xVclDialog->GetParent(), "OCommonPicker::createFileDialog: invalid window component (the parent this time)!");
                }
                if ( m_xDialogParent.is() )
                {
                    m_xParentListenerAdapter = new OWeakEventListenerAdapter( this, m_xDialogParent );
                        // the adapter will add itself as listener, and forward notifications
                }
            }
        }

        return nullptr != m_xDlg;
    }

    // XControlAccess functions
    void SAL_CALL OCommonPicker::setControlProperty( const OUString& aControlName, const OUString& aControlProperty, const Any& aValue )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_xDlg.get(), m_xDlg->GetView() );
            aAccess.setControlProperty( aControlName, aControlProperty, aValue );
        }
    }

    Any SAL_CALL OCommonPicker::getControlProperty( const OUString& aControlName, const OUString& aControlProperty )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_xDlg.get(), m_xDlg->GetView() );
            return aAccess.getControlProperty( aControlName, aControlProperty );
        }

        return Any();
    }

    // XControlInformation functions
    Sequence< OUString > SAL_CALL OCommonPicker::getSupportedControls(  )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_xDlg.get(), m_xDlg->GetView() );
            return aAccess.getSupportedControls( );
        }

        return Sequence< OUString >();
    }

    sal_Bool SAL_CALL OCommonPicker::isControlSupported( const OUString& aControlName )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            return svt::OControlAccess::isControlSupported( aControlName );
        }

        return false;
    }

    Sequence< OUString > SAL_CALL OCommonPicker::getSupportedControlProperties( const OUString& aControlName )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_xDlg.get(), m_xDlg->GetView() );
            return aAccess.getSupportedControlProperties( aControlName );
        }

        return Sequence< OUString >();
    }

    sal_Bool SAL_CALL OCommonPicker::isControlPropertySupported( const OUString& aControlName, const OUString& aControlProperty )
    {
        checkAlive();

        SolarMutexGuard aGuard;
        if ( createPicker() )
        {
            ::svt::OControlAccess aAccess( m_xDlg.get(), m_xDlg->GetView() );
            return aAccess.isControlPropertySupported( aControlName, aControlProperty );
        }

        return false;
    }


    // XExecutableDialog functions

    void OCommonPicker::setTitle( const OUString& _rTitle )
    {
        SolarMutexGuard aGuard;
        m_aTitle = _rTitle;
    }


    sal_Int16 OCommonPicker::execute()
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

    void SAL_CALL OCommonPicker::cancel(  )
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
        // currently. Both is okay for us...

        // Note that we could do check if we are really executing the dialog currently.
        // but the information would be potentially obsolete at the moment our event
        // arrives, so we need to check it there, anyway...
        m_nCancelEvent = Application::PostUserEvent( LINK( this, OCommonPicker, OnCancelPicker ) );
    }

    IMPL_LINK_NOARG(OCommonPicker, OnCancelPicker, void*, void)
    {
        // By definition, the solar mutex is locked when we arrive here. Note that this
        // is important, as for instance the consistency of m_xDlg depends on this mutex.
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nCancelEvent = nullptr;

        if ( !m_bExecuting )
            // nothing to do. This may be because the dialog was canceled after our cancel method
            // posted this async event, or because somebody called cancel without the dialog
            // being executed at this time.
            return;

        OSL_ENSURE( m_xDlg, "OCommonPicker::OnCancelPicker: executing, but no dialog!" );
        if (m_xDlg)
            m_xDlg->response(RET_CANCEL);
    }

    // XInitialization functions
    void SAL_CALL OCommonPicker::initialize( const Sequence< Any >& _rArguments )
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
                SAL_WARN( "fpicker", "OCommonPicker::initialize: unknown argument type at position "
                    << (pArguments - _rArguments.getConstArray()));
                continue;
            }

            bool bKnownSetting =
                implHandleInitializationArgument( sSettingName, aSettingValue );
            DBG_ASSERT( bKnownSetting,
                OString(
                    "OCommonPicker::initialize: unknown argument \""
                    + OUStringToOString(sSettingName, osl_getThreadTextEncoding())
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
            OSL_ENSURE( m_xDialogParent.is(), "OCommonPicker::implHandleInitializationArgument: invalid parent window given!" );
        }
        else
            bKnown = false;
        return bKnown;
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

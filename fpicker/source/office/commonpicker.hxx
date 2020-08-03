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

#pragma once

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <tools/link.hxx>

class SvtFileDialog_Base;
namespace weld { class Window; }
struct ImplSVEvent;

namespace svt
{


    typedef ::cppu::WeakComponentImplHelper    <   css::ui::dialogs::XControlAccess
                                                ,   css::ui::dialogs::XControlInformation
                                                ,   css::lang::XEventListener
                                                ,   css::util::XCancellable
                                                ,   css::lang::XInitialization
                                                >   OCommonPicker_Base;
    /** implements common functionality for the 2 UNO picker components
    */
    class OCommonPicker
                    :public ::cppu::BaseMutex
                    ,public OCommonPicker_Base
                    ,public ::comphelper::OPropertyContainer
                    ,public ::comphelper::OPropertyArrayUsageHelper< OCommonPicker >
    {
    private:
        // <properties>
        OUString                                                  m_sHelpURL;
        css::uno::Reference< css::awt::XWindow >                  m_xWindow;
        // </properties>

        ImplSVEvent *                   m_nCancelEvent;
        bool                            m_bExecuting;

        css::uno::Reference< css::awt::XWindow >      m_xDialogParent;

        css::uno::Reference< css::lang::XComponent >  m_xWindowListenerAdapter;
        css::uno::Reference< css::lang::XComponent >  m_xParentListenerAdapter;

    protected:
        OUString     m_aTitle;
        OUString     m_aDisplayDirectory;

    protected:
        std::shared_ptr<SvtFileDialog_Base> m_xDlg;

        const    ::cppu::OBroadcastHelper&   GetBroadcastHelper() const  { return OCommonPicker_Base::rBHelper; }
        ::cppu::OBroadcastHelper&   GetBroadcastHelper()        { return OCommonPicker_Base::rBHelper; }

    public:
        OCommonPicker();

    protected:
        virtual ~OCommonPicker() override;

        // overridables

        // will be called with locked SolarMutex
        virtual std::shared_ptr<SvtFileDialog_Base> implCreateDialog( weld::Window* pParent ) = 0;
        virtual sal_Int16       implExecutePicker( ) = 0;
            // do NOT override XExecutableDialog::execute! We need to do some stuff there ourself ...

    protected:

        // disambiguate XInterface

        DECLARE_XINTERFACE( )


        // disambiguate XTypeProvider

        DECLARE_XTYPEPROVIDER( )


        // ComponentHelper/XComponent

        virtual void SAL_CALL disposing() override;


        // XEventListener

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;


        // property set related methods


        // XPropertySet pure methods
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        // OPropertySetHelper pure methods
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        // OPropertyArrayUsageHelper pure methods
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // OPropertySetHelper overridden methods
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const css::uno::Any& _rValue ) override;


        // XExecutableDialog functions

        /// @throws css::uno::RuntimeException
        virtual void SAL_CALL           setTitle( const OUString& _rTitle );
        /// @throws css::uno::RuntimeException
        virtual sal_Int16 SAL_CALL      execute();


        // XControlAccess functions

        virtual void SAL_CALL setControlProperty( const OUString& aControlName, const OUString& aControlProperty, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getControlProperty( const OUString& aControlName, const OUString& aControlProperty ) override;


        // XControlInformation functions

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedControls(  ) override;
        virtual sal_Bool SAL_CALL isControlSupported( const OUString& aControlName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedControlProperties( const OUString& aControlName ) override;
        virtual sal_Bool SAL_CALL isControlPropertySupported( const OUString& aControlName, const OUString& aControlProperty ) override;


        // XCancellable functions

        virtual void SAL_CALL cancel(  ) override;


        // XInitialization functions


        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;


        // misc

        void checkAlive() const;

        void prepareDialog();

    protected:
                bool    createPicker();

        /** handle a single argument from the XInitialization::initialize method

            @return <TRUE/> if the argument could be handled
        */
        virtual bool    implHandleInitializationArgument(
                                const OUString& _rName,
                                const css::uno::Any& _rValue
                            );

    private:
        void        stopWindowListening();

        DECL_LINK( OnCancelPicker, void*, void );
    };

}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

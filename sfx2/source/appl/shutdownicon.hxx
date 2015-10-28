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

#ifndef __SHUTDOWNICON_HXX__
#define __SHUTDOWNICON_HXX__

#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <sfx2/sfxuno.hxx>
#include <cppuhelper/compbase.hxx>
#include <sfx2/dllapi.h>
#include <tools/link.hxx>

class ResMgr;
namespace sfx2
{
    class FileDialogHelper;
}

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XInitialization,
    css::frame::XTerminateListener,
    css::lang::XServiceInfo,
    css::beans::XFastPropertySet > ShutdownIconServiceBase;

#if defined(USE_APP_SHORTCUTS)
#define WRITER_URL      "private:factory/swriter"
#define CALC_URL        "private:factory/scalc"
#define IMPRESS_URL     "private:factory/simpress"
#define IMPRESS_WIZARD_URL     "private:factory/simpress?slot=6686"
#define DRAW_URL        "private:factory/sdraw"
#define MATH_URL        "private:factory/smath"
#define BASE_URL        "private:factory/sdatabase?Interactive"
#define STARTMODULE_URL ".uno:ShowStartModule"
#endif

class SFX2_DLLPUBLIC ShutdownIcon : public ShutdownIconServiceBase
{
        ::osl::Mutex            m_aMutex;
        bool                    m_bVeto;
        bool                    m_bListenForTermination;
        bool                    m_bSystemDialogs;
        ResMgr*                 m_pResMgr;
        sfx2::FileDialogHelper* m_pFileDlg;
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        static ShutdownIcon *pShutdownIcon; // one instance

        bool m_bInitialized;
        void initSystray();
        void deInitSystray();

        static void EnterModalMode();
        static void LeaveModalMode();
        static OUString getShortcutName();

        friend class SfxNotificationListener_Impl;

    public:
        explicit ShutdownIcon( const css::uno::Reference< css::uno::XComponentContext > & rxContext );

        virtual ~ShutdownIcon();

        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
            throw (css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;

        static ShutdownIcon* getInstance();
        static ShutdownIcon* createInstance();

        static void terminateDesktop();
        static void addTerminateListener();

        static void FileOpen();
        static void OpenURL( const OUString& aURL, const OUString& rTarget, const css::uno::Sequence< css::beans::PropertyValue >& =
            css::uno::Sequence< css::beans::PropertyValue >( 0 ) );
        static void FromTemplate();

        static void SetAutostart( bool bActivate );
        static bool GetAutostart();
        static bool bModalMode;

        void init() throw( css::uno::Exception, std::exception );

        OUString GetResString( int id );
        static OUString GetUrlDescription( const OUString& aUrl );

        void SetVeto( bool bVeto )  { m_bVeto = bVeto;}

        void                    StartFileDialog();
        DECL_LINK_TYPED(DialogClosedHdl_Impl, sfx2::FileDialogHelper*, void);

        static bool IsQuickstarterInstalled();

        // Component Helper - force override
        virtual void SAL_CALL disposing() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
            throw(css::uno::RuntimeException, std::exception) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent )
            throw(css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent )
            throw(css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw( css::uno::Exception, std::exception ) override;

        // XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue(       ::sal_Int32                  nHandle,
                                                    const css::uno::Any& aValue )
            throw (css::beans::UnknownPropertyException,
                    css::beans::PropertyVetoException,
                    css::lang::IllegalArgumentException,
                    css::lang::WrappedTargetException,
                    css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle )
            throw (css::beans::UnknownPropertyException,
                    css::lang::WrappedTargetException,
                    css::uno::RuntimeException, std::exception) override;

        css::uno::Reference< css::frame::XDesktop2 > m_xDesktop;

#ifdef WNT
        static void EnableAutostartW32( const OUString &aShortcutName );
        static OUString GetAutostartFolderNameW32();
#endif
};

extern "C" {
#  ifdef WNT
    // builtin win32 systray
    void win32_init_sys_tray();
    void win32_shutdown_sys_tray();
#  elif defined MACOSX
    void aqua_init_systray();
    void aqua_shutdown_systray();
#  endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

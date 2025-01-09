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

#ifndef INCLUDED_SFX2_SOURCE_APPL_SHUTDOWNICON_HXX
#define INCLUDED_SFX2_SOURCE_APPL_SHUTDOWNICON_HXX

#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <comphelper/compbase.hxx>
#include <tools/link.hxx>
#include <memory>

#ifdef MACOSX
#include <vcl/menu.hxx>
#endif

extern "C" {

void SAL_DLLPUBLIC_EXPORT plugin_init_sys_tray();
void SAL_DLLPUBLIC_EXPORT plugin_shutdown_sys_tray();

}

namespace sfx2
{
    class FileDialogHelper;
}

typedef comphelper::WeakComponentImplHelper<
    css::lang::XInitialization,
    css::frame::XTerminateListener,
    css::lang::XServiceInfo,
    css::beans::XFastPropertySet > ShutdownIconServiceBase;

inline constexpr OUString WRITER_URL          = u"private:factory/swriter"_ustr;
inline constexpr OUString CALC_URL            = u"private:factory/scalc"_ustr;
inline constexpr OUString IMPRESS_URL         = u"private:factory/simpress"_ustr;
inline constexpr OUString IMPRESS_WIZARD_URL  = u"private:factory/simpress?slot=6686"_ustr;
inline constexpr OUString DRAW_URL            = u"private:factory/sdraw"_ustr;
inline constexpr OUString MATH_URL            = u"private:factory/smath"_ustr;
inline constexpr OUString BASE_URL            = u"private:factory/sdatabase?Interactive"_ustr;
inline constexpr OUString STARTMODULE_URL     = u".uno:ShowStartModule"_ustr;

class ShutdownIcon : public ShutdownIconServiceBase
{
        bool                    m_bVeto;
        bool                    m_bListenForTermination;
        bool                    m_bSystemDialogs;
        std::unique_ptr<sfx2::FileDialogHelper> m_pFileDlg;
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        css::uno::Reference< css::frame::XDesktop2 > m_xDesktop;

        static rtl::Reference<ShutdownIcon> pShutdownIcon; // one instance

        bool m_bInitialized;
        void initSystray();
        void deInitSystray();

        static void EnterModalMode();
        static void LeaveModalMode();
        static OUString getShortcutName();

        friend class SfxNotificationListener_Impl;

    public:
        explicit ShutdownIcon( css::uno::Reference< css::uno::XComponentContext > xContext );

        virtual ~ShutdownIcon() override;

        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        static ShutdownIcon* getInstance();
        static ShutdownIcon* createInstance();

        static void terminateDesktop();
        static void addTerminateListener();

        static void FileOpen();
        static void OpenURL( const OUString& aURL, const OUString& rTarget, const css::uno::Sequence< css::beans::PropertyValue >& =
            css::uno::Sequence< css::beans::PropertyValue >( 0 ) );
        static void FromTemplate();
        static void FromCommand( const OUString& rCommand );

        static void SetAutostart( bool bActivate );
        static bool GetAutostart();
        static bool bModalMode;

        /// @throws css::uno::Exception
        void init();

        static OUString GetUrlDescription( std::u16string_view aUrl );

        void SetVeto( bool bVeto )  { m_bVeto = bVeto;}

        void                    StartFileDialog();
        DECL_LINK(DialogClosedHdl_Impl, sfx2::FileDialogHelper*, void);

        static bool IsQuickstarterInstalled();

        // Component Helper - force override
        virtual void disposing(std::unique_lock<std::mutex>&) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue(       ::sal_Int32                  nHandle,
                                                    const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) override;

#ifdef _WIN32
        static void EnableAutostartW32( const OUString &aShortcutName );
        static OUString GetAutostartFolderNameW32();
#elif defined MACOSX
        static void SetDefaultMenuBar( MenuBar *pMenuBar );
#endif
};

extern "C" {
#  ifdef _WIN32
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

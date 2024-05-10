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

#ifndef INCLUDED_VCL_INC_SALINST_HXX
#define INCLUDED_VCL_INC_SALINST_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <vcl/vclenum.hxx>

#include "displayconnectiondispatch.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <memory>

namespace com::sun::star::awt {
    class XWindow;
}
namespace comphelper { class SolarMutex; }
namespace vcl { class Window; }
namespace weld {
    class Builder;
    class MessageDialog;
    class Widget;
    class Window;
}
class SystemChildWindow;
struct SystemParentData;
struct SalPrinterQueueInfo;
class ImplJobSetup;
class OpenGLContext;
class SalGraphics;
class SalFrame;
class SalObject;
class SalMenu;
class SalMenuItem;
class SalVirtualDevice;
class SalInfoPrinter;
class SalPrinter;
class SalTimer;
class ImplPrnQueueList;
class SalSystem;
class SalBitmap;
struct SalItemParams;
class SalSession;
struct SystemEnvData;
struct SystemGraphicsData;
struct SystemWindowData;
class Menu;
enum class VclInputFlags;
enum class SalFrameStyleFlags;

typedef struct _cairo_font_options cairo_font_options_t;

class VCL_DLLPUBLIC SalInstance
{
private:
    rtl::Reference< vcl::DisplayConnectionDispatch > m_pEventInst;
    const std::unique_ptr<comphelper::SolarMutex> m_pYieldMutex;
    css::uno::Reference<css::uno::XInterface> m_clipboard;

protected:
    bool m_bSupportsBitmap32 = false;
    bool m_bSupportsOpenGL = false;

public:
    SalInstance(std::unique_ptr<comphelper::SolarMutex> pMutex);
    virtual ~SalInstance();

    bool supportsBitmap32() const { return m_bSupportsBitmap32; }
    bool supportsOpenGL() const { return m_bSupportsOpenGL; }

    //called directly after Application::Init
    virtual void            AfterAppInit() {}
    virtual bool            SVMainHook(int*) { return false; }

    // Frame
    // DisplayName for Unix ???
    virtual SalFrame*       CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) = 0;
    virtual SalFrame*       CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) = 0;
    virtual void            DestroyFrame( SalFrame* pFrame ) = 0;

    // Object (System Child Window)
    virtual SalObject*      CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow ) = 0;
    virtual void            DestroyObject( SalObject* pObject ) = 0;

    // VirtualDevice
    // nDX and nDY in pixels
    // nBitCount: 0 == default(=as window) / 1 == mono
    // pData allows for using a system dependent graphics or device context,
    // if a system context is passed in nDX and nDY are updated to reflect
    // its size; otherwise these remain unchanged.
    virtual std::unique_ptr<SalVirtualDevice>
                            CreateVirtualDevice( SalGraphics& rGraphics,
                                                 tools::Long &rDX, tools::Long &rDY,
                                                 DeviceFormat eFormat, const SystemGraphicsData *pData = nullptr ) = 0;

    // Printer
    // pSetupData->mpDriverData can be 0
    // pSetupData must be updated with the current
    // JobSetup
    virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData ) = 0;
    virtual void            DestroyInfoPrinter( SalInfoPrinter* pPrinter ) = 0;
    virtual std::unique_ptr<SalPrinter> CreatePrinter( SalInfoPrinter* pInfoPrinter ) = 0;

    virtual void            GetPrinterQueueInfo( ImplPrnQueueList* pList ) = 0;
    virtual void            GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) = 0;
    virtual OUString        GetDefaultPrinter() = 0;

    // SalTimer
    virtual SalTimer*       CreateSalTimer() = 0;
    // SalSystem
    virtual SalSystem*      CreateSalSystem() = 0;
    // SalBitmap
    virtual std::shared_ptr<SalBitmap> CreateSalBitmap() = 0;

    // YieldMutex
    comphelper::SolarMutex* GetYieldMutex();
    sal_uInt32              ReleaseYieldMutexAll();
    void                    AcquireYieldMutex(sal_uInt32 nCount = 1);

    // return true, if the current thread is the main thread
    virtual bool            IsMainThread() const = 0;

    /**
     * Wait for the next event (if bWait) and dispatch it,
     * includes posted events, and timers.
     * If bHandleAllCurrentEvents - dispatch multiple posted
     * user events. Returns true if events were processed.
     */
    virtual bool           DoYield(bool bWait, bool bHandleAllCurrentEvents) = 0;
    virtual bool           AnyInput( VclInputFlags nType ) = 0;

    // menus
    virtual std::unique_ptr<SalMenu>     CreateMenu( bool bMenuBar, Menu* pMenu );
    virtual std::unique_ptr<SalMenuItem> CreateMenuItem( const SalItemParams& pItemData );

    // may return NULL to disable session management, only used by X11 backend
    virtual std::unique_ptr<SalSession> CreateSalSession();

    // also needs to set m_bSupportsOpenGL = true in your SalInstance implementation!
    virtual OpenGLContext*  CreateOpenGLContext();

    virtual std::unique_ptr<weld::Builder> CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    virtual std::unique_ptr<weld::Builder> CreateInterimBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                                                bool bAllowCycleFocusOut, sal_uInt64 nLOKWindowId = 0);
    virtual weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType,
                                                     VclButtonsType eButtonType, const OUString& rPrimaryMessage);
    virtual weld::Window* GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow);

    // methods for XDisplayConnection

    void                    SetEventCallback( rtl::Reference< vcl::DisplayConnectionDispatch > const & pInstance )
        { m_pEventInst = pInstance; }

    bool                    CallEventCallback( void const * pEvent, int nBytes );

    virtual OUString        GetConnectionIdentifier() = 0;

    // dtrans implementation
    virtual css::uno::Reference< css::uno::XInterface > CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments );
    virtual css::uno::Reference<css::uno::XInterface> ImplCreateDragSource(const SystemEnvData*);
    virtual css::uno::Reference<css::uno::XInterface> ImplCreateDropTarget(const SystemEnvData*);
    css::uno::Reference<css::uno::XInterface> CreateDragSource(const SystemEnvData* = nullptr);
    css::uno::Reference<css::uno::XInterface> CreateDropTarget(const SystemEnvData* = nullptr);
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) = 0;

    virtual bool            hasNativeFileSelection() const { return false; }
    // if you override this, make sure to override hasNativeFileSelection too.
    virtual css::uno::Reference< css::ui::dialogs::XFilePicker2 > createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& )
        { return css::uno::Reference< css::ui::dialogs::XFilePicker2 >(); }
    virtual css::uno::Reference< css::ui::dialogs::XFolderPicker2 > createFolderPicker( const css::uno::Reference< css::uno::XComponentContext >& )
        { return css::uno::Reference< css::ui::dialogs::XFolderPicker2 >(); }

    // callbacks for printer updates
    virtual void            updatePrinterUpdate() {}
    virtual void            jobEndedPrinterUpdate() {}

    /// Set the app's (somewhat) magic/main-thread to this one.
    virtual void            updateMainThread() {}
    /// Disconnect that - good for detaching from the JavaVM on Android.
    virtual void            releaseMainThread() {}

    /// get information about underlying versions
    virtual OUString        getOSVersion() { return u"-"_ustr; }

    virtual const cairo_font_options_t* GetCairoFontOptions() { return nullptr; }

    virtual void* CreateGStreamerSink(const SystemChildWindow*) { return nullptr; }

    virtual void BeforeAbort(const OUString& /* rErrorText */, bool /* bDumpCore */) {}

    // Note: we cannot make this a global variable, because it might be initialised BEFORE the putenv() call in cppunittester.
    static bool IsRunningUnitTest() { return getenv("LO_TESTNAME") != nullptr; }

    // both must be implemented, if the VCL plugin needs to run via system event loop
    virtual bool DoExecute(int &nExitCode);
    virtual void DoQuit();
};

// called from SVMain
SalInstance* CreateSalInstance();
void DestroySalInstance( SalInstance* pInst );

void SalAbort( const OUString& rErrorText, bool bDumpCore );

const OUString& SalGetDesktopEnvironment();

#endif // INCLUDED_VCL_INC_SALINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

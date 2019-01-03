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
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <osl/thread.hxx>
#include <vcl/vclenum.hxx>

#include "displayconnectiondispatch.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <memory>

namespace com {
namespace sun {
namespace star {
namespace awt {
    class XWindow;
}
} } }
namespace comphelper { class SolarMutex; }
namespace vcl { class Window; }
namespace weld {
    class Builder;
    class MessageDialog;
    class Widget;
    class Window;
}
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
class SalI18NImeStatus;
class SalSystem;
class SalBitmap;
struct SalItemParams;
class SalSession;
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

public:
    SalInstance(std::unique_ptr<comphelper::SolarMutex> pMutex);
    virtual ~SalInstance();

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
                            CreateVirtualDevice( SalGraphics* pGraphics,
                                                 long &rDX, long &rDY,
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
    // interface to ime status window, only used by the X11 backend
    virtual std::unique_ptr<SalI18NImeStatus>
                            CreateI18NImeStatus();
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

    virtual OpenGLContext*  CreateOpenGLContext() = 0;

    virtual weld::Builder* CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    static  weld::Builder* CreateInterimBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile);
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
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource();
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget();
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) = 0;

    virtual bool            hasNativeFileSelection() const { return false; }

    virtual css::uno::Reference< css::ui::dialogs::XFilePicker2 > createFilePicker( const css::uno::Reference< css::uno::XComponentContext >& )
        { return css::uno::Reference< css::ui::dialogs::XFilePicker2 >(); }

    virtual css::uno::Reference< css::ui::dialogs::XFolderPicker2 > createFolderPicker( const css::uno::Reference< css::uno::XComponentContext >& )
        { return css::uno::Reference< css::ui::dialogs::XFolderPicker2 >(); }

    // callbacks for printer updates
    virtual void            updatePrinterUpdate() {}
    virtual void            jobStartedPrinterUpdate() {}
    virtual void            jobEndedPrinterUpdate() {}

    virtual void            updateMainThread() {}

    /// get information about underlying versions
    virtual OUString        getOSVersion() { return OUString("-"); }

    virtual const cairo_font_options_t* GetCairoFontOptions() { return nullptr; }
};

// called from SVMain
SalInstance* CreateSalInstance();
void DestroySalInstance( SalInstance* pInst );

void SalAbort( const OUString& rErrorText, bool bDumpCore );

VCL_DLLPUBLIC const OUString& SalGetDesktopEnvironment();

#endif // INCLUDED_VCL_INC_SALINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

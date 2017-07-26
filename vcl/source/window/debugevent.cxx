/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/random.hxx>
#include <rtl/math.hxx>
#include <rtl/string.hxx>
#include <tools/time.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/svapp.hxx>
#include <vcl/debugevent.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/menu.hxx>
#include "window.h"
#include "salwtype.hxx"

#if OSL_DEBUG_LEVEL > 0

DebugEventInjector::DebugEventInjector( sal_uInt32 nMaxEvents) :
      Timer("debug event injector")
    , mnEventsLeft( nMaxEvents )
{
    SetTimeout( 1000 /* ms */ );
    Start();
}

static double getRandom()
{
    return comphelper::rng::uniform_real_distribution();
}

vcl::Window *DebugEventInjector::ChooseWindow()
{
    vcl::Window *pParent;

    if (getRandom() < 0.80)
        if (vcl::Window * pWindow = Application::GetFocusWindow())
            return pWindow;

    if (getRandom() > 0.50 ||
        !(pParent = Application::GetActiveTopWindow()))
    {
        // select a top window at random
        long nIdx = Application::GetTopWindowCount() * getRandom();
        pParent = Application::GetTopWindow( nIdx );
        if (!pParent)
            pParent = static_cast<vcl::Window *>(Application::GetAppWindow());
    }
    assert (pParent != nullptr);

    std::vector< vcl::Window *> aChildren;
    pParent->CollectChildren( aChildren );

    return aChildren[ aChildren.size() * getRandom() ];
}


static void CollectMenuItemIds( Menu *pMenu, std::vector< SalMenuEvent > &rIds )
{
    sal_uInt16 nItems = pMenu->GetItemCount();
    for (sal_uInt16 i = 0; i < nItems; i++)
    {
        if (pMenu->GetItemType( i ) != MenuItemType::SEPARATOR || getRandom() < 0.01)
            rIds.emplace_back( pMenu->GetItemId( i ), pMenu );
        PopupMenu *pPopup = pMenu->GetPopupMenu( i );
        if (pPopup)
            CollectMenuItemIds( pPopup, rIds );
    }
}

void DebugEventInjector::InjectMenuEvent()
{
    vcl::Window *pFocus = Application::GetFocusWindow();
    if (!pFocus)
        return;

    SystemWindow *pSysWin = pFocus->GetSystemWindow();
    if (!pSysWin)
        return;

    MenuBar *pMenuBar = pSysWin->GetMenuBar();
    if (!pMenuBar)
        return;

    SalEvent nEvents[] = {
        SalEvent::MenuCommand,
        SalEvent::MenuCommand,
        SalEvent::MenuActivate,
        SalEvent::MenuDeactivate,
        SalEvent::MenuHighlight,
        SalEvent::MenuCommand,
        SalEvent::MenuCommand,
        SalEvent::MenuCommand,
        SalEvent::MenuButtonCommand,
        SalEvent::MenuButtonCommand,
    };

    std::vector< SalMenuEvent > aIds;
    CollectMenuItemIds( pMenuBar, aIds );

    SalEvent nEvent = nEvents[ (int)(getRandom() * SAL_N_ELEMENTS( nEvents )) ];
    SalMenuEvent aEvent = aIds[ getRandom() * aIds.size() ];
    bool bHandled = ImplWindowFrameProc( pSysWin, nEvent, &aEvent);

    SAL_INFO( "vcl.debugevent",
              "Injected menu event " << aEvent.mpMenu
              << " (" << aEvent.mnId << ") '"
              << static_cast<Menu *>(aEvent.mpMenu)->GetItemText( aEvent.mnId ) << "' -> "
              << bHandled );
}

static void InitKeyEvent( SalKeyEvent &rKeyEvent )
{
    if (getRandom() < 0.01)
        rKeyEvent.mnRepeat = getRandom() * 20;
    else
        rKeyEvent.mnRepeat = 0;
}

void DebugEventInjector::InjectTextEvent()
{
    SalKeyEvent aKeyEvent;
    vcl::Window *pWindow = ChooseWindow();

    InitKeyEvent( aKeyEvent );

    if (getRandom() < 0.10) // Occasionally a truly random event
    {
        aKeyEvent.mnCode = getRandom() * KEY_CODE_MASK;
        aKeyEvent.mnCharCode = getRandom() * 0xffff;
    }
    else
    {
        struct {
            sal_uInt16 nCodeStart, nCodeEnd;
            char       aCharStart;
        } nTextCodes[] = {
            { KEY_0, KEY_9, '0' },
            { KEY_A, KEY_Z, 'a' }
        };

        size_t i = getRandom() * SAL_N_ELEMENTS( nTextCodes );
        int offset = int( getRandom() * ( nTextCodes[i].nCodeEnd - nTextCodes[i].nCodeStart ) );
        aKeyEvent.mnCode = nTextCodes[i].nCodeStart + offset;
        aKeyEvent.mnCharCode = nTextCodes[i].aCharStart + offset;
//        fprintf( stderr, "Char '%c' offset %d into record %d base '%c'\n",
//                 aKeyEvent.mnCharCode, offset, (int)i, nTextCodes[i].aCharStart );
    }

    if( getRandom() < 0.05 ) // modifier
        aKeyEvent.mnCode |= (sal_uInt16)( getRandom() * KEY_MODIFIERS_MASK ) & KEY_MODIFIERS_MASK;

    bool bHandled = ImplWindowFrameProc( pWindow, SalEvent::KeyInput, &aKeyEvent);

    SAL_INFO( "vcl.debugevent",
              "Injected key 0x" << std::hex << (int) aKeyEvent.mnCode << std::dec
              << " -> " << bHandled
              << " win " << pWindow );

    ImplWindowFrameProc( pWindow, SalEvent::KeyUp, &aKeyEvent );
}

/*
 *   The more heuristics we have to inform this the better,
 * key-bindings, menu entries, allowable entry types etc.
 */
void DebugEventInjector::InjectEvent()
{
//    fprintf( stderr, "%6d - ", (int)mnEventsLeft );

    double nRand = getRandom();
    if (nRand < 0.30)
    {
        int nEvents = getRandom() * 10;
        for (int i = 0; i < nEvents; i++)
            InjectTextEvent();
    }
    else if (nRand < 0.60)
        InjectKeyNavEdit();
    else if (nRand < 0.95)
        InjectMenuEvent();
}

void DebugEventInjector::InjectKeyNavEdit()
{
    vcl::Window *pWindow = ChooseWindow();

    struct {
        double     mnProb;
        sal_uInt16 mnKey;
    } const nWeights[] = {
        // edit / escape etc. - 50%
        { 0.20, KEY_SPACE },
        { 0.10, KEY_TAB },
        { 0.07, KEY_RETURN },
        { 0.05, KEY_DELETE },
        { 0.05, KEY_BACKSPACE },

        // navigate - 45%
        { 0.15, KEY_LEFT },
        { 0.10, KEY_RIGHT },
        { 0.05, KEY_UP },
        { 0.05, KEY_DOWN },
        { 0.05, KEY_PAGEUP },
        { 0.05, KEY_PAGEDOWN },

        // other
        { 0.01, KEY_INSERT },
        { 0.02, KEY_HOME },
        { 0.02, KEY_END },
    };

    double d = 0.0, nRand = getRandom();
    sal_uInt16 nKey = KEY_SPACE;
    for (auto & rWeight : nWeights)
    {
        d += rWeight.mnProb;
        assert (d < 1.01);
        if ( nRand < d )
        {
            nKey = rWeight.mnKey;
            break;
        }
    }

    SalKeyEvent aKeyEvent;
    InitKeyEvent( aKeyEvent );
    aKeyEvent.mnCode = nKey;

    if (getRandom() < 0.15) // modifier
        aKeyEvent.mnCode |= (sal_uInt16)(getRandom() * KEY_MODIFIERS_MASK) & KEY_MODIFIERS_MASK;

    aKeyEvent.mnCharCode = 0x0; // hopefully unused.

    bool bHandled = ImplWindowFrameProc( pWindow, SalEvent::KeyInput, &aKeyEvent );

    SAL_INFO( "vcl.debugevent",
              "Injected edit / move key 0x" << std::hex << (int) aKeyEvent.mnCode << std::dec
              << " -> " << bHandled
              << " win " <<  pWindow );
    ImplWindowFrameProc( pWindow, SalEvent::KeyUp, &aKeyEvent );
}

void DebugEventInjector::Invoke()
{
    InjectEvent();
    mnEventsLeft--;
    if (mnEventsLeft > 0)
    {
        SetTimeout( 1 );
        Start();
    }
    else
        Application::Quit();
}

DebugEventInjector *DebugEventInjector::getCreate()
{
    sal_uInt32 nEvents;
    const char *pEvents = getenv("VCL_EVENT_INJECTION");
    if (!pEvents)
        return nullptr;
    nEvents = OString( pEvents ).toUInt32();
    if (nEvents > 0)
        return new DebugEventInjector( nEvents );
    else
        return nullptr;
}

#endif // OSL_DEBUG_LEVEL > 0

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
    mnEventsLeft( nMaxEvents )
{
    SetTimeout( 1000 /* ms */ );
    Start();
}

static double getRandom()
{
    return (double)rand() / RAND_MAX;
}

vcl::Window *DebugEventInjector::ChooseWindow()
{
    vcl::Window *pWindow, *pParent;

    if (getRandom() < 0.80 &&
        (pWindow = Application::GetFocusWindow()))
        return pWindow;

    if (getRandom() > 0.50 ||
        !(pParent = Application::GetActiveTopWindow()))
    {
        // select a top window at random
        long nIdx = Application::GetTopWindowCount() * getRandom();
        if (!(pParent = Application::GetTopWindow( nIdx )))
            pParent = static_cast<vcl::Window *>(Application::GetAppWindow());
    }
    assert (pParent != NULL);

    std::vector< vcl::Window *> aChildren;
    pParent->CollectChildren( aChildren );

    return aChildren[ aChildren.size() * getRandom() ];
}

void DebugEventInjector::InjectMouseEvent()
{
}

typedef std::vector< SalMenuEvent > MenuItemIds;

static void CollectMenuItemIds( Menu *pMenu, MenuItemIds &rIds )
{
    sal_uInt16 nItems = pMenu->GetItemCount();
    for (sal_uInt16 i = 0; i < nItems; i++)
    {
        if (pMenu->GetItemType( i ) != MENUITEM_SEPARATOR || getRandom() < 0.01)
            rIds.push_back( SalMenuEvent( pMenu->GetItemId( i ), pMenu ) );
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

    sal_uInt16 nEvents[] = {
        SALEVENT_MENUCOMMAND,
        SALEVENT_MENUCOMMAND,
        SALEVENT_MENUACTIVATE,
        SALEVENT_MENUDEACTIVATE,
        SALEVENT_MENUHIGHLIGHT,
        SALEVENT_MENUCOMMAND,
        SALEVENT_MENUCOMMAND,
        SALEVENT_MENUCOMMAND,
        SALEVENT_MENUBUTTONCOMMAND,
        SALEVENT_MENUBUTTONCOMMAND,
    };

    MenuItemIds aIds;
    CollectMenuItemIds( pMenuBar, aIds );

    sal_uInt16 nEvent = nEvents[ (int)(getRandom() * SAL_N_ELEMENTS( nEvents )) ];
    SalMenuEvent aEvent = aIds[ getRandom() * aIds.size() ];
    bool bHandled = ImplWindowFrameProc( pSysWin, NULL, nEvent, &aEvent);

    SAL_INFO( "vcl.debugevent",
              "Injected menu event " << aEvent.mpMenu
              << " (" << aEvent.mnId << ") '"
              << ((Menu *)aEvent.mpMenu)->GetItemText( aEvent.mnId ) << "' -> "
              << bHandled );
}

static void InitKeyEvent( SalKeyEvent &rKeyEvent )
{
    double nRand = getRandom();
    if (nRand < 0.001)
        rKeyEvent.mnTime = getRandom() * ULONG_MAX;
    else
        rKeyEvent.mnTime = tools::Time::GetSystemTicks();

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
        aKeyEvent.mnCode = getRandom() * KEY_CODE;
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
        aKeyEvent.mnCode |= (sal_uInt16)( getRandom() * KEY_MODTYPE ) & KEY_MODTYPE;

    bool bHandled = ImplWindowFrameProc( pWindow, NULL, SALEVENT_KEYINPUT, &aKeyEvent);

    SAL_INFO( "vcl.debugevent",
              "Injected key 0x" << std::hex << (int) aKeyEvent.mnCode << std::dec
              << " -> " << bHandled
              << " win " << pWindow );

    ImplWindowFrameProc( pWindow, NULL, SALEVENT_KEYUP, &aKeyEvent );
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
    else
        InjectMouseEvent();
}

void DebugEventInjector::InjectKeyNavEdit()
{
    vcl::Window *pWindow = ChooseWindow();

    struct {
        double     mnProb;
        sal_uInt16 mnKey;
    } nWeights[] = {
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
    for ( size_t i = 0; i < SAL_N_ELEMENTS( nWeights ); ++i )
    {
        d += nWeights[i].mnProb;
        assert (d < 1.01);
        if ( nRand < d )
        {
            nKey = nWeights[i].mnKey;
            break;
        }
    }

    SalKeyEvent aKeyEvent;
    InitKeyEvent( aKeyEvent );
    aKeyEvent.mnCode = nKey;

    if (getRandom() < 0.15) // modifier
        aKeyEvent.mnCode |= (sal_uInt16)(getRandom() * KEY_MODTYPE) & KEY_MODTYPE;

    aKeyEvent.mnCharCode = 0x0; // hopefully unused.

    bool bHandled = ImplWindowFrameProc( pWindow, NULL, SALEVENT_KEYINPUT, &aKeyEvent );

    SAL_INFO( "vcl.debugevent",
              "Injected edit / move key 0x" << std::hex << (int) aKeyEvent.mnCode << std::dec
              << " -> " << bHandled
              << " win " <<  pWindow );
    ImplWindowFrameProc( pWindow, NULL, SALEVENT_KEYUP, &aKeyEvent );
}

void DebugEventInjector::Timeout()
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
        return NULL;
    nEvents = OString( pEvents ).toUInt32();
    if (nEvents > 0)
        return new DebugEventInjector( nEvents );
    else
        return NULL;
}

#endif // OSL_DEBUG_LEVEL > 0

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

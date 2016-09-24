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

#include <tools/debug.hxx>

#include <svdata.hxx>

#include <dlgctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>
#include <vcl/unohelp.hxx>

#include <com/sun/star/i18n/XCharacterClassification.hpp>

using namespace ::com::sun::star;


static vcl::Window* ImplGetLabelFor( vcl::Window* pFrameWindow, WindowType nMyType, vcl::Window* pLabel, sal_Unicode nAccel )
{
    vcl::Window* pWindow = nullptr;

    if( nMyType == WINDOW_FIXEDTEXT     ||
        nMyType == WINDOW_FIXEDLINE     ||
        nMyType == WINDOW_GROUPBOX )
    {
        // #i100833# MT 2010/02: Group box and fixed lines can also label a fixed text.
        // See tools/options/print for example.
        bool bThisIsAGroupControl = (nMyType == WINDOW_GROUPBOX) || (nMyType == WINDOW_FIXEDLINE);
        // get index, form start and form end
        sal_uInt16 nIndex=0, nFormStart=0, nFormEnd=0;
        ::ImplFindDlgCtrlWindow( pFrameWindow,
                                           pLabel,
                                           nIndex,
                                           nFormStart,
                                           nFormEnd );
        if( nAccel )
        {
            // find the accelerated window
            pWindow = ::ImplFindAccelWindow( pFrameWindow,
                                             nIndex,
                                             nAccel,
                                             nFormStart,
                                             nFormEnd,
                                             false );
        }
        else
        {
            // find the next control; if that is a fixed text
            // fixed line or group box, then return NULL
            while( nIndex < nFormEnd )
            {
                nIndex++;
                vcl::Window* pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nIndex,
                                                 nIndex,
                                                 false );
                if( pSWindow && isVisibleInLayout(pSWindow) && ! (pSWindow->GetStyle() & WB_NOLABEL) )
                {
                    WindowType nType = pSWindow->GetType();
                    if( nType != WINDOW_FIXEDTEXT   &&
                        nType != WINDOW_FIXEDLINE   &&
                        nType != WINDOW_GROUPBOX )
                    {
                        pWindow = pSWindow;
                    }
                    else if( bThisIsAGroupControl && ( nType == WINDOW_FIXEDTEXT ) )
                    {
                        pWindow = pSWindow;
                    }
                    break;
                }
            }
        }
    }

    return pWindow;
}

namespace vcl {

Window* Window::getLegacyNonLayoutAccessibleRelationLabelFor() const
{
    Window* pWindow = nullptr;
    Window* pFrameWindow = ImplGetFrameWindow();

    WinBits nFrameStyle = pFrameWindow->GetStyle();
    if( ! ( nFrameStyle & WB_DIALOGCONTROL )
        || ( nFrameStyle & WB_NODIALOGCONTROL )
        )
        return nullptr;

    if ( mpWindowImpl->mpRealParent )
        pWindow = mpWindowImpl->mpRealParent->GetParentLabelFor( this );

    if( pWindow )
        return pWindow;

    sal_Unicode nAccel = getAccel( GetText() );

    pWindow = ImplGetLabelFor( pFrameWindow, GetType(), const_cast<Window*>(this), nAccel );
    if( ! pWindow && mpWindowImpl->mpRealParent )
        pWindow = ImplGetLabelFor( mpWindowImpl->mpRealParent, GetType(), const_cast<Window*>(this), nAccel );
    return pWindow;
}

static Window* ImplGetLabeledBy( Window* pFrameWindow, WindowType nMyType, Window* pLabeled )
{
    Window* pWindow = nullptr;
    if ( (nMyType != WINDOW_GROUPBOX) && (nMyType != WINDOW_FIXEDLINE) )
    {
        // search for a control that labels this window
        // a label is considered the last fixed text, fixed line or group box
        // that comes before this control; with the exception of push buttons
        // which are labeled only if the fixed text, fixed line or group box
        // is directly before the control

        // get form start and form end and index of this control
        sal_uInt16 nIndex, nFormStart, nFormEnd;
        Window* pSWindow = ::ImplFindDlgCtrlWindow( pFrameWindow,
                                                    pLabeled,
                                                    nIndex,
                                                    nFormStart,
                                                    nFormEnd );
        if( pSWindow && nIndex != nFormStart )
        {
            if( nMyType == WINDOW_PUSHBUTTON        ||
                nMyType == WINDOW_HELPBUTTON        ||
                nMyType == WINDOW_OKBUTTON      ||
                nMyType == WINDOW_CANCELBUTTON )
            {
                nFormStart = nIndex-1;
            }
            for( sal_uInt16 nSearchIndex = nIndex-1; nSearchIndex >= nFormStart; nSearchIndex-- )
            {
                sal_uInt16 nFoundIndex = 0;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nSearchIndex,
                                                 nFoundIndex,
                                                 false );
                if( pSWindow && isVisibleInLayout(pSWindow) && !(pSWindow->GetStyle() & WB_NOLABEL) )
                {
                    WindowType nType = pSWindow->GetType();
                    if ( ( nType == WINDOW_FIXEDTEXT    ||
                          nType == WINDOW_FIXEDLINE ||
                          nType == WINDOW_GROUPBOX ) )
                    {
                        // a fixed text can't be labeld by a fixed text.
                        if ( ( nMyType != WINDOW_FIXEDTEXT ) || ( nType != WINDOW_FIXEDTEXT ) )
                            pWindow = pSWindow;
                        break;
                    }
                }
                if( nFoundIndex > nSearchIndex || nSearchIndex == 0 )
                    break;
            }
        }
    }
    return pWindow;
}

Window* Window::getLegacyNonLayoutAccessibleRelationLabeledBy() const
{
    Window* pWindow = nullptr;
    Window* pFrameWindow = ImplGetFrameWindow();

    if ( mpWindowImpl->mpRealParent )
    {
        pWindow = mpWindowImpl->mpRealParent->GetParentLabeledBy( this );

        if( pWindow )
            return pWindow;
    }

    // #i62723#, #104191# checkboxes and radiobuttons are not supposed to have labels
    if( GetType() == WINDOW_CHECKBOX || GetType() == WINDOW_RADIOBUTTON )
        return nullptr;

//    if( ! ( GetType() == WINDOW_FIXEDTEXT     ||
//            GetType() == WINDOW_FIXEDLINE     ||
//            GetType() == WINDOW_GROUPBOX ) )
    // #i100833# MT 2010/02: Group box and fixed lines can also label a fixed text.
    // See tools/options/print for example.

    pWindow = ImplGetLabeledBy( pFrameWindow, GetType(), const_cast<Window*>(this) );
    if( ! pWindow && mpWindowImpl->mpRealParent )
        pWindow = ImplGetLabeledBy( mpWindowImpl->mpRealParent, GetType(), const_cast<Window*>(this) );

    return pWindow;
}

Window* Window::getLegacyNonLayoutAccessibleRelationMemberOf() const
{
    Window* pWindow = nullptr;
    Window* pFrameWindow = GetParent();
    if ( !pFrameWindow )
    {
        pFrameWindow = ImplGetFrameWindow();
    }
    // if( ! ( GetType() == WINDOW_FIXEDTEXT        ||
    if( !( GetType() == WINDOW_FIXEDLINE ||
        GetType() == WINDOW_GROUPBOX ) )
    {
        // search for a control that makes member of this window
        // it is considered the last fixed line or group box
        // that comes before this control; with the exception of push buttons
        // which are labeled only if the fixed line or group box
        // is directly before the control
        // get form start and form end and index of this control
        sal_uInt16 nIndex, nFormStart, nFormEnd;
        Window* pSWindow = ::ImplFindDlgCtrlWindow( pFrameWindow,
            const_cast<Window*>(this),
            nIndex,
            nFormStart,
            nFormEnd );
        if( pSWindow && nIndex != nFormStart )
        {
            if( GetType() == WINDOW_PUSHBUTTON      ||
                GetType() == WINDOW_HELPBUTTON      ||
                GetType() == WINDOW_OKBUTTON        ||
                GetType() == WINDOW_CANCELBUTTON )
            {
                nFormStart = nIndex-1;
            }
            for( sal_uInt16 nSearchIndex = nIndex-1; nSearchIndex >= nFormStart; nSearchIndex-- )
            {
                sal_uInt16 nFoundIndex = 0;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                    nSearchIndex,
                    nFoundIndex,
                    false );
                if( pSWindow && pSWindow->IsVisible() &&
                    ( pSWindow->GetType() == WINDOW_FIXEDLINE   ||
                    pSWindow->GetType() == WINDOW_GROUPBOX ) )
                {
                    pWindow = pSWindow;
                    break;
                }
                if( nFoundIndex > nSearchIndex || nSearchIndex == 0 )
                    break;
            }
        }
    }
    return pWindow;
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

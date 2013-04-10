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

#ifndef SD_TOOLPANEL_FOCUS_MANAGER_HXX
#define SD_TOOLPANEL_FOCUS_MANAGER_HXX

#include <tools/link.hxx>

#include <memory>

class KeyCode;
class VclSimpleEvent;
class Window;

namespace sd { namespace toolpanel {

/** On certain key presses the focus is moved from one window to another.
    For this to work every window that wants its focus managed has to
    register or be registered and tell where to put the focus on what key
    press.
*/
class FocusManager
{
public:
    /** Return an instance of the focus manager.
    */
    static FocusManager& Instance (void);

    /** Register a link from one window to another so that any time the
        specified key is pressed while the source window is focused, the
        focus is transferred to the target window.
        @param pSource
            The window from which the focus will be transferred.
        @param pTarget
            The window to which the focus will be transferred.
        @param rKey
            The key for which the focus is transferred from the source
            window to the target window.
    */
    void RegisterLink (
        ::Window* pSource,
        ::Window* pTarget,
        const KeyCode& rKey);

    /** Register a link that will move the focus from the source window to
        the target window when the source window is focused and KEY_ESCAPE
        is pressed.
        @param pSource
            The window from which the focus will be transferred.
        @param pTarget
            The window to which the focus will be transferred.
    */
    void RegisterUpLink (::Window* pSource, ::Window* pTarget);

    /** Register a link that will move the focus from the source window to
        the target window when the source window is focused and KEY_RETURN
        is pressed.
        @param pSource
            The window from which the focus will be transferred.
        @param pTarget
            The window to which the focus will be transferred.
    */
    void RegisterDownLink (::Window* pSource, ::Window* pTarget);

    /** Remove all links from the source window to the target window.  When
        there are links from the target window to the source window then
        these are not touced.
    */
    void RemoveLinks (
        ::Window* pSource,
        ::Window* pTarget);

    /** Let the focus manager transfer the focus from the specified source
        window to a target window that is determined according the the
        registered links and the given key code.
        When there is no rule for this combination of source window and key
        code then the focus stays where it is.
    */
    bool TransferFocus (::Window* pSource, const KeyCode& rCode);

private:
    friend struct FocusManagerCreator;

    class LinkMap;
    ::std::auto_ptr<LinkMap> mpLinks;

    FocusManager (void);
    ~FocusManager (void);

    /** Clear the list of focus transfer links.  This removes all window
        listeners.
    */
    void Clear (void);

    /** Remove all links from or to the given window.
    */
    void RemoveLinks (::Window* pWindow);

    /** Unregister as event listener from the given window when there are no
        links from this window anymore.
    */
    void RemoveUnusedEventListener (::Window* pWindow);

    /** Listen for key events and on KEY_RETURN go down and on
        KEY_ESCAPE go up.
    */
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

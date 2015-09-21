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

#ifndef INCLUDED_VCL_INC_OSX_A11YFOCUSTRACKER_HXX
#define INCLUDED_VCL_INC_OSX_A11YFOCUSTRACKER_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>

#include "keyboardfocuslistener.hxx"

#include <rtl/instance.hxx>

#include <tools/link.hxx>
#include <vcl/vclevent.hxx>
#include <set>

namespace vcl { class Window; }
class ToolBox;
class DocumentFocusListener;

// - AquaA11yFocusTracker -

class AquaA11yFocusTracker : public rtl::Static< AquaA11yFocusTracker, AquaA11yFocusTracker>
{

public:
    AquaA11yFocusTracker();

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getFocusedObject() { return m_xFocusedObject; };

    // sets the currently focus object and notifies the FocusEventListener (if any)
    void setFocusedObject(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible);

    // may evolve to add/remove later
    void setFocusListener(const rtl::Reference< KeyboardFocusListener >& aFocusListener) { m_aFocusListener = aFocusListener; };

protected:

    // received a WINDOW_GETFOCUS event for this window
    void window_got_focus(vcl::Window *pWindow);

    // received a TOOLBOX_HIGHLIGHT event for this window
    void toolbox_highlight_on(vcl::Window *pWindow);

    // received a TOOLBOX_HIGHLIGHTOFF event for this window
    void toolbox_highlight_off(vcl::Window *pWindow);

    // received a TABPAGE_ACTIVATE event for this window
    void tabpage_activated(vcl::Window *pWindow);

    // received a MENU_HIGHLIGHT event for this window
    void menu_highlighted(const ::VclMenuEvent *pEvent);

    // toolbox items are widgets in gtk+ and Cocoa
    void notify_toolbox_item_focus(ToolBox *pToolBox);

    // toolbox item opened a floating window (e.g. color chooser)
    void toolbox_open_floater(vcl::Window *pWindow);

    // callback function for Application::addEventListener
    static void WindowEventHandler(void * pThis, VclSimpleEvent&);

private:
    // the accessible object that has the keyboard focus (if any)
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xFocusedObject;

    // the listener for focus events
    rtl::Reference< KeyboardFocusListener > m_aFocusListener;

    // the list of Windows that need deeper (focus) investigation
    std::set<VclPtr<vcl::Window>> m_aDocumentWindowList;

    // the link object needed for Application::addEventListener
    Link<> m_aWindowEventLink;

    // the UNO XAccessibilityEventListener for Documents and other non VCL objects
    const ::com::sun::star::uno::Reference< DocumentFocusListener > m_xDocumentFocusListener;
};

#endif // INCLUDED_VCL_INC_OSX_A11YFOCUSTRACKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

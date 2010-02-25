/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _AQUA11YFOCUSTRACKER_HXX_
#define _AQUA11YFOCUSTRACKER_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#include "keyboardfocuslistener.hxx"

#include <rtl/instance.hxx>

#include <tools/link.hxx>
#include <vcl/vclevent.hxx>
#include <set>

class Window;
class ToolBox;
class DocumentFocusListener;

// ------------------------
// - AquaA11yFocusTracker -
// ------------------------

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
    virtual void window_got_focus(Window *pWindow);

    // received a TOOLBOX_HIGHLIGHT event for this window
    virtual void toolbox_highlight_on(Window *pWindow);

    // received a TOOLBOX_HIGHLIGHTOFF event for this window
    virtual void toolbox_highlight_off(Window *pWindow);

    // received a TABPAGE_ACTIVATE event for this window
    virtual void tabpage_activated(Window *pWindow);

    // received a MENU_HIGHLIGHT event for this window
    virtual void menu_highlighted(const ::VclMenuEvent *pEvent);

    // toolbox items are widgets in gtk+ and Cocoa
    virtual void notify_toolbox_item_focus(ToolBox *pToolBox);

    // toolbox item opened a floating window (e.g. color chooser)
    virtual void toolbox_open_floater(Window *pWindow);

    // callback function for Application::addEventListener
    static long WindowEventHandler(AquaA11yFocusTracker *pFocusTracker, ::VclSimpleEvent const *pEvent);

private:
    // the accessible object that has the keyboard focus (if any)
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xFocusedObject;

    // the listener for focus events
    rtl::Reference< KeyboardFocusListener > m_aFocusListener;

    // the list of Windows that need deeper (focus) investigation
    std::set< Window *> m_aDocumentWindowList;

    // the link object needed for Application::addEventListener
    Link m_aWindowEventLink;

    // the UNO XAccessibilityEventListener for Documents and other non VCL objects
    const ::com::sun::star::uno::Reference< DocumentFocusListener > m_xDocumentFocusListener;
};

#endif  // _AQUA11YFOCUSTRACKER_HXX_

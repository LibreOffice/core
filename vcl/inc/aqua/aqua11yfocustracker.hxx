/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

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
#ifndef SFX_SIDEBAR_CONTROL_FACTORY_HXX
#define SFX_SIDEBAR_CONTROL_FACTORY_HXX

#include <sfx2/dllapi.h>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/frame/XFrame.hpp>
class ToolBox;

namespace sfx2 { namespace sidebar {

class ToolBoxBackground;

/** Factory for controls used in sidebar panels.
    The reason to use this factory instead of creating the controls
    directly is that this way the sidebar has a little more control
    over look and feel of its controls.
*/
class SFX2_DLLPUBLIC ControlFactory
{
public:
    /** Create the menu button for the task bar.
    */
    static CheckBox* CreateMenuButton (Window* pParentWindow);

    static ImageRadioButton* CreateTabItem (Window* pParentWindow);

    /** Create a tool box that does *not* handle its items.  The
        caller has to register callbacks to process, among others,
        click and selection events.
    */
    static SidebarToolBox* CreateToolBox (
        Window* pParentWindow,
        const ResId& rResId);

    /** Create a tool box that *does* handle its items.  All event
        processing is done by toolbox controllers.
    */
    static SidebarToolBox* CreateToolBox (
        Window* pParentWindow,
        const ResId& rResId,
        const ::com::sun::star::uno::Reference<com::sun::star::frame::XFrame>& rxFrame);

    /** Create a window that acts as background of a tool box.
        In general it is slightly larger than the tool box.
        @param pParentWindow
            The parent window of the new background control.
        @param bShowBorder
            When <TRUE/> then the background control is made slightly
            larger then its tool box child, once that is created.
            Otherwise the background control will not be visible.
    */
    static Window* CreateToolBoxBackground (
        Window* pParentWindow,
        const bool bShowBorder = true);

    static ImageRadioButton* CreateCustomImageRadionButton(
        Window* pParentWindow,
        const ResId& rResId );
};


} } // end of namespace sfx2::sidebar

#endif

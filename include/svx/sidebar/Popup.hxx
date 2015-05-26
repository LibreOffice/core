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
#ifndef INCLUDED_SVX_SIDEBAR_POPUP_HXX
#define INCLUDED_SVX_SIDEBAR_POPUP_HXX

#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <svx/sidebar/PopupContainer.hxx>
#include <svx/sidebar/PopupControl.hxx>

#include <boost/function.hpp>

namespace vcl { class Window; }
class ToolBox;

namespace svx { namespace sidebar {

/** A wrapper around a PopupContainer and a PopupControl object.
    Usually used as drop down for a toolbox.  Use Show() to start
    drop down mode and Hide() to end it.
*/
class SVX_DLLPUBLIC Popup
{
public :
    /** Create a Popup wrapper object.
        @param pParent
            Parent window of the PopupContainer, which in turn is the
            parent of the PopupControl.
        @param rControlCreator
            A functor that is called to create the PopupControl object
            (usually an instance of a class derived from
            PopupControl).
    */
    Popup (
        vcl::Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator,
        const ::rtl::OUString& rsAccessibleName);
    virtual ~Popup();

    /** Show the popup.
        @rToolBox
            The tool box is used to determine the position at which
            the popup is displayed.
    */
    void Show (ToolBox& rToolBox);

    /** Hide the popup.
        This method is called automatically when eg. the user clicks
        outside the popup or when the ESC-key is pressed.  The
        application can call Hide() when the popup should be closed
        for other, non-standard reasons.
    */
    void Hide();

    /** If you want to be informed when the popup closes then add a
        callback that is called after that.
    */
    void SetPopupModeEndHandler (const ::boost::function<void()>& rCallback);

    void dispose();

protected:
    VclPtr<PopupControl> mxControl;

    /** Make sure that both PopupContainer and PopupControl objects
        exist.  Calls the maControlCreator functor if necessary.
    */
    void ProvideContainerAndControl();

    /** A derived specialisation class can override this method to do
        additional work.
    */
    void CreateContainerAndControl();

private:
    VclPtr<vcl::Window> mpParent;
    ::boost::function<VclPtr<PopupControl>(PopupContainer*)> maControlCreator;
    ::boost::function<void()> maPopupModeEndCallback;
    const ::rtl::OUString msAccessibleName;
    VclPtr<PopupContainer> mxContainer;

    DECL_LINK(PopupModeEndHandler, void*);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

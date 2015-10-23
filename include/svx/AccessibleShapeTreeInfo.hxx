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

#ifndef INCLUDED_SVX_ACCESSIBLESHAPETREEINFO_HXX
#define INCLUDED_SVX_ACCESSIBLESHAPETREEINFO_HXX

#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/svxdllapi.h>
#include <vcl/vclptr.hxx>

class SdrView;
namespace vcl { class Window; }

namespace accessibility {

/** This class bundles all information that is passed down the tree of
    accessible shapes so that each shape has access to that info.

    There are basically four members that can be set and queried:
    <ul>
    <li>The model broadcaster is used for getting notified about shape
    changes.  Using this broadcaster makes in unnecessary to register at
    each shape separately.</li>
    <li>The view forwarder is responsible for transformation between
    coordinate systems and for providing the visible area both with respect
    to a specific window.</li>
    <li>The SdrView is used for creating accessible edit engines.</li>
    <li>The Window is used for creating accessible edit engines.</li>
    </ul>
*/
class SVX_DLLPUBLIC AccessibleShapeTreeInfo
{
public:
    /** Use this constructor to create an empty object that is filled later
        with more meaningfull data.
    */
    AccessibleShapeTreeInfo();

    /** Create a copy of the given shape info.
        @param rInfo
            The shape tree info object to copy.
    */
    AccessibleShapeTreeInfo (const AccessibleShapeTreeInfo& rInfo);

    ~AccessibleShapeTreeInfo();

    AccessibleShapeTreeInfo& operator= (const AccessibleShapeTreeInfo& rInfo);

    /** Deprecated.  Don't use this method.
    */
    void SetDocumentWindow (const css::uno::Reference<
        css::accessibility::XAccessibleComponent>& rxViewWindow);

    /** Deprecated.  Don't use this method.
    */
    css::uno::Reference<
        css::accessibility::XAccessibleComponent>
        GetDocumentWindow() const { return mxDocumentWindow;}

    /** Set a new broadcaster that sends events indicating shape changes.
        The broadcaster usually is or belongs to a document model.
        @param rxModelBroadcaster
            The new broadcaster.  It replaces the current one.  An empty
            reference may be passed to unset the broadcaster
    */
    void SetModelBroadcaster (const css::uno::Reference<
        css::document::XEventBroadcaster>& rxModelBroadcaster);

    /** Return the current model broadcaster.
        @return
            The returned reference may be empty if the broadcaster has not
            been set or has been set to an empty reference.
    */
    css::uno::Reference<
        css::document::XEventBroadcaster>
        GetModelBroadcaster() const { return mxModelBroadcaster;}

    /** Set the view that will be used to construct SvxTextEditSources which
        in turn are used to create accessible edit engines.
        @param pView
            The new SdrView that replaces the current one.  A NULL pointer
            may be passed to unset the view.
    */
    void SetSdrView (SdrView* pView);

    /** Return the current SdrView.
        @return
            The returned value may be NULL.
    */
    SdrView* GetSdrView() const { return mpView;}

    /** Set a new controller.  This will usually but not necessarily
        correspond to the SdrView.
        @param rxController
            The new controller that replaces the current one.  An empty
            reference may be passed to unset the controller.
    */
    void SetController (const css::uno::Reference<
        css::frame::XController>& rxController);

    /** Return the currently set controller.
        @return
            The reference to the currently set controller may be empty.
    */
    css::uno::Reference<
        css::frame::XController>
        GetController() const { return mxController;}

    /** Set the window that is used to construct SvxTextEditSources which in
        turn is used to create accessible edit engines.
    */
    void SetWindow (vcl::Window* pWindow);

    /** Return the current Window.
        @return
            The returned value may be NULL.
    */
    vcl::Window* GetWindow() const { return mpWindow;}

    /** The view forwarder allows the transformation between internal
        and pixel coordinates and can be asked for the visible area.
        @param pViewForwarder
            This view forwarder replaces the current one.
    */
    void SetViewForwarder (const IAccessibleViewForwarder* pViewForwarder);

    /** Return the current view forwarder.
        @return
            The returned pointer may be NULL.
    */
    const IAccessibleViewForwarder* GetViewForwarder() const { return mpViewForwarder;}

private:
    /** Deprecated.
    */
    css::uno::Reference<
        css::accessibility::XAccessibleComponent> mxDocumentWindow;

    /** this broadcaster sends events indicating shape changes.
        The broadcaster usually is or belongs to a document model.

        This once was named mxControllerBroadcaster.
    */
    css::uno::Reference<
        css::document::XEventBroadcaster> mxModelBroadcaster;

    /** This view is necessary to construct an SvxTextEditSource which in
        turn is used to create an accessible edit engine.
    */
    SdrView* mpView;

    /** The controller is used e.g. for obtaining the selected shapes.
    */
    css::uno::Reference<
        css::frame::XController> mxController;

    /** This window is necessary to construct an SvxTextEditSource which in
        turn is used to create an accessible edit engine.
    */
    VclPtr<vcl::Window> mpWindow;

    /** The view forwarder allows the transformation between internal
        and pixel coordinates and can be asked for the visible area.
    */
    const IAccessibleViewForwarder* mpViewForwarder;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

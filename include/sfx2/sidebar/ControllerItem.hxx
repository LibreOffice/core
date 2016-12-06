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
#ifndef INCLUDED_SFX2_SIDEBAR_CONTROLLERITEM_HXX
#define INCLUDED_SFX2_SIDEBAR_CONTROLLERITEM_HXX

#include <sfx2/ctrlitem.hxx>

#include <cppuhelper/basemutex.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/frame/XFrame.hpp>


class SfxViewFrame;
class ToolBox;


namespace sfx2 { namespace sidebar {

/** The sfx2::sidebar::ControllerItem is a wrapper around the
    SfxControllerItem that becomes necessary to allow objects (think
    sidebar panels) to receive state changes without having one
    SfxControllerItem per supported item as base class (which is not
    possible in C++ anyway).

    It also gives access to the label and icon of a slot/command.
*/
class SFX2_DLLPUBLIC ControllerItem
    : public SfxControllerItem
{
public:
    class SFX2_DLLPUBLIC ItemUpdateReceiverInterface
    {
    public:
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled) = 0;
        virtual ~ItemUpdateReceiverInterface();
    };

    /** This is the preferred constructor that allows the created
        controller item to return non-empty values for GetLabel() and
        GetIcon() calls.
    */
    ControllerItem (
        const sal_uInt16 nSlotId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver,
        const ::rtl::OUString& rsCommandName,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** This is the simpler constructor variant that still exists for
        compatibility resons.  Note that GetLabel() and GetIcon() will
        return empty strings/images.
    */
    ControllerItem (
        const sal_uInt16 nId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver);

    /// releases our action listener
    virtual void dispose() override;

    virtual ~ControllerItem() override;

    /** Returns </TRUE> when the slot/command has not been disabled.
        Changes of this state are notified via the
        ItemUpdateReceiverInterface::NotifyContextChang() method.
    */
    bool IsEnabled (const SfxItemState eState) const;

    /** Force the controller item to call its NotifyItemUpdate
        callback with up-to-date data.
    */
    void RequestUpdate();

    /** Do not call.  Used by local class only.  Should be a member of
        a local and hidden interface.
    */
    void NotifyFrameContextChange();
    /** Do not call.  Used by local class only.  Should be a member of
        a local and hidden interface.
    */
    void ResetFrame();

protected:

    virtual void StateChanged (sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState) override;

private:
    ItemUpdateReceiverInterface& mrItemUpdateReceiver;
    css::uno::Reference<css::frame::XFrame> mxFrame;
    css::uno::Reference<css::lang::XComponent> mxFrameActionListener;
    const ::rtl::OUString msCommandName;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

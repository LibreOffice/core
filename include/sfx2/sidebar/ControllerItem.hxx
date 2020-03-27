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

namespace com::sun::star::lang { class XComponent; }


namespace sfx2 { namespace sidebar {

/** The sfx2::sidebar::ControllerItem is a wrapper around the
    SfxControllerItem that becomes necessary to allow objects (think
    sidebar panels) to receive state changes without having one
    SfxControllerItem per supported item as base class (which is not
    possible in C++ anyway).

    It also gives access to the label and icon of a slot/command.
*/
class SFX2_DLLPUBLIC ControllerItem final
    : public SfxControllerItem
{
public:
    class SFX2_DLLPUBLIC ItemUpdateReceiverInterface
    {
    public:
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState) = 0;
        virtual void GetControlState(
            const sal_uInt16 nSId,
            boost::property_tree::ptree& rState) = 0;
        virtual ~ItemUpdateReceiverInterface();
    };

    /** This is the simpler constructor variant that still exists for
        compatibility reasons. Note that GetLabel() and GetIcon() will
        return empty strings/images.
    */
    ControllerItem (
        const sal_uInt16 nId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver);

    /// releases our action listener
    virtual void dispose() override;

    virtual ~ControllerItem() override;

    /** Force the controller item to call its NotifyItemUpdate
        callback with up-to-date data.
    */
    void RequestUpdate();

private:

    virtual void StateChanged (sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState) override;
    virtual void GetControlState (sal_uInt16 nSId, boost::property_tree::ptree& rState) override;

    ItemUpdateReceiverInterface& mrItemUpdateReceiver;
    css::uno::Reference<css::lang::XComponent> mxFrameActionListener;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

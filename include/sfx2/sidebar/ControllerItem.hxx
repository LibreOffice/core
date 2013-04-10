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
#ifndef SVX_SIDEBAR_CONTROLLER_ITEM_HXX
#define SVX_SIDEBAR_CONTROLLER_ITEM_HXX

#include <sfx2/ctrlitem.hxx>

#include <boost/function.hpp>


namespace sfx2 { namespace sidebar {

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
            const SfxPoolItem* pState) = 0;
        virtual ~ItemUpdateReceiverInterface();
    };
    ControllerItem (
        const sal_uInt16 nId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver);
    virtual ~ControllerItem();

    virtual void StateChanged (sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState);

private:
    ItemUpdateReceiverInterface& mrItemUpdateReceiver;
};

} } // end of namespace sfx2::sidebar

#endif

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

#ifndef SVX_SIDEBAR_CONTROLLER_ITEM_HXX
#define SVX_SIDEBAR_CONTROLLER_ITEM_HXX

#include <sfx2/ctrlitem.hxx>

#include <boost/function.hpp>


namespace sfx2 { namespace sidebar {

class SFX2_DLLPUBLIC ControllerItem
    : public SfxControllerItem
{
public:
    class ItemUpdateReceiverInterface
    {
    public:
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState) = 0;
    };
    ControllerItem (
        const sal_uInt16 nId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver);
    virtual ~ControllerItem (void);

    virtual void StateChanged (sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState);

private:
    ItemUpdateReceiverInterface& mrItemUpdateReceiver;
};

} } // end of namespace sfx2::sidebar

#endif

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



#ifndef _SVX_CLIPBOARDCTL_HXX_
#define _SVX_CLIPBOARDCTL_HXX_

#include <sfx2/tbxctrl.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svl/poolitem.hxx>


#include "svx/svxdllapi.h"

class PopupMenu;
class SvxClipboardFmtItem;


class SVX_DLLPUBLIC SvxClipBoardControl : public SfxToolBoxControl
{
    SfxPoolItem*            pClipboardFmtItem;
    PopupMenu*              pPopup;
    sal_uInt16                  nItemId;
    sal_Bool                    bDisabled;

    void                    DelPopup();

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxClipBoardControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxClipBoardControl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};


#endif


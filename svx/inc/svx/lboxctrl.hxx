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



#ifndef _SVX_LBOXCTRL_HXX_
#define _SVX_LBOXCTRL_HXX_

#include <sfx2/tbxctrl.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include "svx/svxdllapi.h"

class ToolBox;
class SvxPopupWindowListBox;

/////////////////////////////////////////////////////////////////

class SvxListBoxControl : public SfxToolBoxControl
{
protected:
    String                  aActionStr;
    SvxPopupWindowListBox * pPopupWin;

    void    Impl_SetInfo( sal_uInt16 nCount );

    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( SelectHdl, void * );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxListBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxListBoxControl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};

/////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxUndoRedoControl : public SvxListBoxControl
{
    std::vector< ::rtl::OUString > aUndoRedoList;
    ::rtl::OUString                aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxUndoRedoControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual ~SvxUndoRedoControl();
    virtual void StateChanged( sal_uInt16 nSID,
                               SfxItemState eState,
                               const SfxPoolItem* pState );

    virtual SfxPopupWindow*  CreatePopupWindow();
};

#endif


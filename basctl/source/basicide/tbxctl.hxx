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


#ifndef _BASIDE_TBXCTL_HXX
#define _BASIDE_TBXCTL_HXX

#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>

/*
#ifdef _BASIDE_POPUPWINDOWTBX

// class PopupWindowTbx --------------------------------------------------

class PopupWindowTbx : public SfxPopupWindow
{
private:
    SfxToolBoxManager   aTbx;
    Link                aSelectLink;

    DECL_LINK( SelectHdl, void* );

public:
    PopupWindowTbx( sal_uInt16 nId, WindowAlign eAlign,
                    ResId aRIdWin, ResId aRIdTbx, SfxBindings& rBind );
    ~PopupWindowTbx();

    void                    StartSelection()
                                { aTbx.GetToolBox().StartSelection(); }
    void                    Update();

    virtual SfxPopupWindow* Clone() const;
    virtual void            PopupModeEnd();
};
#endif
*/
//-------------------
// class TbxControls
//-------------------
class TbxControls : public SfxToolBoxControl
{
private:

    struct StateChangedInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
        bool bDisabled;
    };

    sal_uInt16                  nLastSlot;

protected:
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );
public:
    SFX_DECL_TOOLBOX_CONTROL();

    TbxControls(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~TbxControls() {}

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();

    using                       SfxToolBoxControl::Select;
    void                        Select( sal_uInt16 nModifier );
};


#endif // _BASIDE_TBXCTL_HXX


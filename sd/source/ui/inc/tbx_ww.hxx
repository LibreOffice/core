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




#ifndef _SD_TBX_WW_HXX
#define _SD_TBX_WW_HXX

#include <sdresid.hxx>
#include <sfx2/tbxctrl.hxx>

//------------------------------------------------------------------------

/*
class SdPopupWindowTbx : public SfxPopupWindow
{
private:
    SfxToolBoxManager   aTbx;
    SdResId             aSdResIdWin;
    SdResId             aSdResIdTbx;
    WindowAlign         eTbxAlign;
    Link                aSelectLink;

    DECL_LINK( TbxSelectHdl, ToolBox * );

public:
    SdPopupWindowTbx( sal_uInt16 nId, WindowAlign eAlign,
                      SdResId aRIdWin, SdResId aRIdTbx, SfxBindings& rBindings );
    ~SdPopupWindowTbx();

    virtual SfxPopupWindow* Clone() const;

    virtual void PopupModeEnd();

    void         StartSelection() { aTbx.GetToolBox().StartSelection(); }
    void         Update();

private:

    // Adapt the toolbox to whether CTL is active or not.  If CTL is not
    //    active then all toolbox items that describe CTL specific shapes are
    //    deleted from it and its size is re-calculated.
    void AdaptToCTL (void);
};
*/
//------------------------------------------------------------------------

class SdTbxControl : public SfxToolBoxControl
{
private:
    bool    IsCheckable( sal_uInt16 nSId );

public:
            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SdTbxControl() {}

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
};

#endif      // _SD_TBX_WW_HXX


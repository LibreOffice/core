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



#ifndef SC_TBINSERT_HXX
#define SC_TBINSERT_HXX

#include <sfx2/tbxctrl.hxx>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class ScTbxInsertCtrl : public SfxToolBoxControl
{
    sal_uInt16                  nLastSlotId;

    using SfxToolBoxControl::Select;
    virtual void            Select( sal_Bool bMod1 = sal_False );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    ScTbxInsertCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    ~ScTbxInsertCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
class ScTbxInsertPopup : public SfxPopupWindow
{
    SfxToolBoxManager   aTbx;
    ResId               aRIdWinTemp;
    ResId               aRIdTbxTemp;

    Link                aTbxClickHdl;

    DECL_LINK( TbxSelectHdl, ToolBox* );
    DECL_LINK( TbxClickHdl, ToolBox* );

protected:
    virtual void        PopupModeEnd();

public:
    ScTbxInsertPopup( sal_uInt16 nId, WindowAlign eAlign,
                        const ResId& rRIdWin, const ResId& rRIdTbx,
                        SfxBindings& rBindings );
    ~ScTbxInsertPopup();

    virtual SfxPopupWindow* Clone() const;
    void StartSelection();
};
*/


#endif


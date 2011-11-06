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


#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#define _SVX_VERT_TEXT_TBXCTRL_HXX

#include <sfx2/tbxctrl.hxx>
#include "svx/svxdllapi.h"

/* -----------------------------27.04.01 15:38--------------------------------
    control to remove/insert cjk settings dependent vertical text toolbox item
 ---------------------------------------------------------------------------*/
class SvxVertCTLTextTbxCtrl : public SfxToolBoxControl
{
    sal_Bool bCheckVertical; //determines whether vertical mode or CTL mode has to be checked
public:
    SvxVertCTLTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxVertCTLTextTbxCtrl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    void    SetVert(sal_Bool bSet) {bCheckVertical = bSet;}

};
/* -----------------------------12.09.2002 11:50------------------------------

 ---------------------------------------------------------------------------*/
class SVX_DLLPUBLIC SvxCTLTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxCTLTextTbxCtrl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};
/* -----------------------------12.09.2002 11:50------------------------------

 ---------------------------------------------------------------------------*/
class SVX_DLLPUBLIC SvxVertTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxVertTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};

#endif

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

#ifndef _FILLCTRL_HXX
#define _FILLCTRL_HXX

#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include "svx/svxdllapi.h"

class XFillStyleItem;
class XFillColorItem;
class XFillGradientItem;
class XFillHatchItem;
class XFillBitmapItem;
class FillControl;
class SvxFillTypeBox;
class SvxFillAttrBox;
class ListBox;

/*************************************************************************
|*
|* Klassen fuer Flaechenattribute (Controls und Controller)
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFillToolBoxControl : public SfxToolBoxControl
{
private:
    XFillStyleItem*     mpStyleItem;
    XFillColorItem*     mpColorItem;
    XFillGradientItem*  mpGradientItem;
    XFillHatchItem*     mpHatchItem;
    XFillBitmapItem*    mpBitmapItem;

    FillControl*        mpFillControl;
    SvxFillTypeBox*     mpFillTypeLB;
    SvxFillAttrBox*     mpFillAttrLB;

    XFillStyle          meLastXFS;

    /// bitfield
    bool                mbUpdate : 1;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFillToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    ~SvxFillToolBoxControl();

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    void Update(const SfxPoolItem* pState);
    virtual Window* CreateItemWindow(Window* pParent);
};

//========================================================================

class FillControl : public Window
{
private:
    friend class SvxFillToolBoxControl;

    SvxFillTypeBox*     mpLbFillType;
    SvxFillAttrBox*     mpLbFillAttr;
    Size                maLogicalFillSize;
    Size                maLogicalAttrSize;

    //
    sal_uInt16          mnLastFillTypeControlSelectEntryPos;
    sal_uInt16          mnLastFillAttrControlSelectEntryPos;

    /// bitfield
    bool                mbFillTypeChanged : 1;

    DECL_LINK(SelectFillTypeHdl,ListBox *);
    DECL_LINK(SelectFillAttrHdl,ListBox *);

    virtual void DataChanged(const DataChangedEvent& rDCEvt);

    void InitializeFillStyleAccordingToGivenFillType(XFillStyle aFillStyle);
    void updateLastFillTypeControlSelectEntryPos();
    void updateLastFillAttrControlSelectEntryPos();

public:
    FillControl(Window* pParent, WinBits nStyle = 0);
    ~FillControl();

    virtual void Resize();
};

#endif // _FILLCTRL_HXX

// eof

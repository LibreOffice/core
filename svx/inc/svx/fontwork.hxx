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


#ifndef _SVX_FONTWORK_HXX
#define _SVX_FONTWORK_HXX

// include ---------------------------------------------------------------

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/valueset.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/xenum.hxx>
#include <svx/dlgctrl.hxx>
#include "svx/svxdllapi.h"

// forward ---------------------------------------------------------------

class SdrView;
class SdrPageView;
class SdrObject;

class XFormTextAdjustItem;
class XFormTextDistanceItem;
class XFormTextStartItem;
class XFormTextMirrorItem;
class XFormTextHideFormItem;
class XFormTextOutlineItem;
class XFormTextShadowItem;
class XFormTextShadowColorItem;
class XFormTextShadowXValItem;
class XFormTextShadowYValItem;

/*************************************************************************
|*
|* ControllerItem fuer Fontwork
|*
\************************************************************************/

class SvxFontWorkDialog;

class SvxFontWorkControllerItem : public SfxControllerItem
{
    SvxFontWorkDialog  &rFontWorkDlg;

protected:
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState);

public:
    SvxFontWorkControllerItem(sal_uInt16 nId, SvxFontWorkDialog&, SfxBindings&);
};

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Fontwork-Dialog
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFontWorkChildWindow : public SfxChildWindow
{
 public:
    SvxFontWorkChildWindow(Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);
    SFX_DECL_CHILDWINDOW(SvxFontWorkChildWindow);
};

/*************************************************************************
|*
|* Floating Window zur Attributierung von Texteffekten
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFontWorkDialog : public SfxDockingWindow
{
#define CONTROLLER_COUNT 11

    SvxFontWorkControllerItem* pCtrlItems[CONTROLLER_COUNT];

    ToolBox         aTbxStyle;
    ToolBox         aTbxAdjust;

    FixedImage      aFbDistance;
    MetricField     aMtrFldDistance;
    FixedImage      aFbTextStart;
    MetricField     aMtrFldTextStart;

    ToolBox         aTbxShadow;

    FixedImage      aFbShadowX;
    MetricField     aMtrFldShadowX;
    FixedImage      aFbShadowY;
    MetricField     aMtrFldShadowY;

    ColorLB         aShadowColorLB;

    SfxBindings&    rBindings;
    Timer           aInputTimer;
    sal_Bool            bUserZoomedIn;

    sal_uInt16          nLastStyleTbxId;
    sal_uInt16          nLastAdjustTbxId;
    sal_uInt16          nLastShadowTbxId;
    long            nSaveShadowX;
    long            nSaveShadowY;
    long            nSaveShadowAngle;
    long            nSaveShadowSize;

    ImageList       maImageList;
    ImageList       maImageListH;

    const XColorTable* pColorTable;

#ifdef _SVX_FONTWORK_CXX
 friend class SvxFontWorkChildWindow;
 friend class SvxFontWorkControllerItem;

    DECL_LINK( SelectStyleHdl_Impl, void * );
    DECL_LINK( SelectAdjustHdl_Impl, void * );
    DECL_LINK( SelectShadowHdl_Impl, void * );

    DECL_LINK( ModifyInputHdl_Impl, void * );
    DECL_LINK( InputTimoutHdl_Impl, void * );

    DECL_LINK( ColorSelectHdl_Impl, void * );

    void SetStyle_Impl(const XFormTextStyleItem*);
    void SetAdjust_Impl(const XFormTextAdjustItem*);
    void SetDistance_Impl(const XFormTextDistanceItem*);
    void SetStart_Impl(const XFormTextStartItem*);
    void SetMirror_Impl(const XFormTextMirrorItem*);
    void SetShowForm_Impl(const XFormTextHideFormItem*);
    void SetOutline_Impl(const XFormTextOutlineItem*);
    void SetShadow_Impl(const XFormTextShadowItem*, sal_Bool bRestoreValues = sal_False);
    void SetShadowColor_Impl(const XFormTextShadowColorItem*);
    void SetShadowXVal_Impl(const XFormTextShadowXValItem*);
    void SetShadowYVal_Impl(const XFormTextShadowYValItem*);
#endif

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void ApplyImageList();

 protected:
    virtual void    Zoom();
    virtual SfxChildAlignment CheckAlignment( SfxChildAlignment eActAlign,
                                              SfxChildAlignment eAlign );

 public:
    SvxFontWorkDialog(  SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent,
                        const ResId& rResId );
    ~SvxFontWorkDialog();

    void SetColorTable(const XColorTable* pTable);
    void SetActive(sal_Bool bActivate = sal_True);
};

#endif      // _SVX_FONTWORK_HXX


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontwork.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:53:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_FONTWORK_HXX
#define _SVX_FONTWORK_HXX

// include ---------------------------------------------------------------

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _XENUM_HXX
#include <svx/xenum.hxx>
#endif
#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// forward ---------------------------------------------------------------

class SdrView;
class SdrPageView;
class SdrObject;

class XFormTextAdjustItem;
class XFormTextDistanceItem;
class XFormTextStartItem;
class XFormTextMirrorItem;
class XFormTextStdFormItem;
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
    virtual void StateChanged(USHORT nSID, SfxItemState eState,
                              const SfxPoolItem* pState);

public:
    SvxFontWorkControllerItem(USHORT nId, SvxFontWorkDialog&, SfxBindings&);
};

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Fontwork-Dialog
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFontWorkChildWindow : public SfxChildWindow
{
 public:
    SvxFontWorkChildWindow(Window*, USHORT, SfxBindings*, SfxChildWinInfo*);
    SFX_DECL_CHILDWINDOW(SvxFontWorkChildWindow);
};

/*************************************************************************
|*
|* Floating Window zur Attributierung von Texteffekten
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFontWorkDialog : public SfxDockingWindow
{
 #define CONTROLLER_COUNT 12

    SvxFontWorkControllerItem* pCtrlItems[CONTROLLER_COUNT];

    ValueSet        aFormSet;

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
    BOOL            bUserZoomedIn;

    USHORT          nLastStyleTbxId;
    USHORT          nLastAdjustTbxId;
    USHORT          nLastShadowTbxId;
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

    DECL_LINK( FormSelectHdl_Impl, void * );
    DECL_LINK( ColorSelectHdl_Impl, void * );

    void SetStyle_Impl(const XFormTextStyleItem*);
    void SetAdjust_Impl(const XFormTextAdjustItem*);
    void SetDistance_Impl(const XFormTextDistanceItem*);
    void SetStart_Impl(const XFormTextStartItem*);
    void SetMirror_Impl(const XFormTextMirrorItem*);
    void SetStdForm_Impl(const XFormTextStdFormItem*);
    void SetShowForm_Impl(const XFormTextHideFormItem*);
    void SetOutline_Impl(const XFormTextOutlineItem*);
    void SetShadow_Impl(const XFormTextShadowItem*,
                        BOOL bRestoreValues = FALSE);
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
    void SetActive(BOOL bActivate = TRUE);

    void CreateStdFormObj(SdrView& rView, SdrPageView& rPV,
                          const SfxItemSet& rAttr, SdrObject& rOldObj,
                          XFormTextStdForm eForm);
};

#endif      // _SVX_FONTWORK_HXX


/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef SVX_SIDEBAR_PARA_PROPERTY_PANEL_HXX
#define SVX_SIDEBAR_PARA_PROPERTY_PANEL_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <editeng/lspcitem.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <svx/relfld.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/scoped_ptr.hpp>

#include <svx/sidebar/ColorPopup.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>

#include "ParaBulletsPopup.hxx"
#include "ParaNumberingPopup.hxx"
#include "ParaLineSpacingPopup.hxx"

class FloatingWindow;
class ToolBox;

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace svx { namespace sidebar {

class PopupControl;
class PopupContainer;

class ParaPropertyPanel
    : public PanelLayout,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static ParaPropertyPanel* Create (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);

    virtual void DataChanged (const DataChangedEvent& rEvent);
    SfxBindings* GetBindings();

    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    sal_uInt16 GetBulletTypeIndex(){ return mnBulletTypeIndex; }
    sal_uInt16 GetNumTypeIndex(){ return mnNumTypeIndex; }
    FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );

    void EndSpacingPopupMode (void);
    void EndBulletsPopupMode (void);
    void EndNumberingPopupMode (void);

private:
    /**********************************************************
     **
     ** UI controls
     **
    ***********************************************************/
    //Alignment
    ToolBox*            mpAlignToolBox;
    ToolBox*            mpTBxVertAlign;
    //NumBullet&Backcolor
    ToolBox*            mpTBxNumBullet;
    ToolBox*            mpTBxBackColor;
    //Paragraph spacing
    ToolBox*            mpTbxUL_IncDec;
    SvxRelativeField*   mpTopDist;
    SvxRelativeField*   mpBottomDist;
    ToolBox*            mpLineSPTbx;
    ToolBox*            mpTbxIndent_IncDec;
    ToolBox*            mpTbxProDemote;
    SvxRelativeField*   mpLeftIndent;
    SvxRelativeField*   mpRightIndent;
    SvxRelativeField*   mpFLineIndent;

    ::boost::scoped_ptr< ::svx::ToolboxButtonColorUpdater > mpColorUpdater;

    /**********************************************************
     **
     ** Resources
     **
    ***********************************************************/

    Image  maSpace3;
    Image  maIndHang;

    ImageList   maNumBImageList;
    ImageList   maNumBImageListRTL;
    Image       maImgBackColorHigh;
    Image       maImgBackColor;

    /****************************************************************
    **
    ** Data Member
    **
    *****************************************************************/
    long                maTxtLeft;
    //Line spacing
    SvxLineSpacingItem  *mpLnSPItem;
    SfxItemState            meLnSpState;
    bool                    mbOutLineLeft;
    bool                    mbOutLineRight;
    long                    maUpper;
    long                    maLower;

    sal_uInt16          mnBulletTypeIndex;
    sal_uInt16          mnNumTypeIndex;
    Color               maColor;
    bool                    mbColorAvailable;
    FieldUnit                       m_eMetricUnit;
    FieldUnit                       m_last_eMetricUnit;
    SfxMapUnit                      m_eLRSpaceUnit;
    SfxMapUnit                      m_eULSpaceUnit;
    /****************************************************************
    **
    ** Control Items
    **
    *****************************************************************/

    ::sfx2::sidebar::ControllerItem  maLeftAlignControl;
    ::sfx2::sidebar::ControllerItem  maCenterAlignControl;
    ::sfx2::sidebar::ControllerItem  maRightAlignControl;
    ::sfx2::sidebar::ControllerItem  maJustifyAlignControl;
    ::sfx2::sidebar::ControllerItem  maLRSpaceControl;
    ::sfx2::sidebar::ControllerItem  maLNSpaceControl;
    ::sfx2::sidebar::ControllerItem  maULSpaceControl;
    ::sfx2::sidebar::ControllerItem  maOutLineLeftControl;
    ::sfx2::sidebar::ControllerItem  maOutLineRightControl;
    ::sfx2::sidebar::ControllerItem  maDecIndentControl;
    ::sfx2::sidebar::ControllerItem  maIncIndentControl;
    ::sfx2::sidebar::ControllerItem  maVertTop;
    ::sfx2::sidebar::ControllerItem  maVertCenter;
    ::sfx2::sidebar::ControllerItem  maVertBottom;
    ::sfx2::sidebar::ControllerItem  maBulletOnOff;
    ::sfx2::sidebar::ControllerItem  maNumberOnOff;
    ::sfx2::sidebar::ControllerItem  maBackColorControl;
    ::sfx2::sidebar::ControllerItem  m_aMetricCtl;
    ::sfx2::sidebar::ControllerItem  maBulletNumRuleIndex;
    ::sfx2::sidebar::ControllerItem  maNumNumRuleIndex;

    cssu::Reference<css::frame::XFrame> mxFrame;
    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;
    ParaLineSpacingPopup maLineSpacePopup;
    ParaBulletsPopup maBulletsPopup;
    ParaNumberingPopup maNumberingPopup;
    ColorPopup maBGColorPopup;
    cssu::Reference<css::ui::XSidebar> mxSidebar;


    ParaPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~ParaPropertyPanel (void);

    void InitImageList(::boost::scoped_ptr<ToolBox>& rTbx, ImageList& rImglst, ImageList& rImgHlst);

    DECL_LINK(AlignStyleModifyHdl_Impl, ToolBox*);
    DECL_LINK(VertTbxSelectHandler, ToolBox*);
    DECL_LINK(NumBTbxSelectHandler, ToolBox*);
    DECL_LINK(ModifyIndentHdl_Impl, void*);
    DECL_LINK(ClickIndent_IncDec_Hdl_Impl, ToolBox*);
    DECL_LINK(ClickProDemote_Hdl_Impl, ToolBox*);
    DECL_LINK(ULSpaceHdl_Impl, void*);
    DECL_LINK(ClickUL_IncDec_Hdl_Impl, ToolBox*);
    DECL_LINK(ImplPopupModeEndHdl, FloatingWindow* );

    void VertStateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    void ParaBKGStateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    void StateChangedAlignmentImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedIndentImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedLnSPImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangedULImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangeOutLineImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void StateChangeIncDecImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    // Add toggle state for numbering and bullet icons
    void StateChangeBulletNumImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    //Modified for Numbering&Bullets Dialog UX Enh
    //Handing the transferred the num rule index data of the current selection
    void StateChangeBulletNumRuleImpl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

    void initial();
    void ReSize(bool bSize);


    PopupControl* CreateLineSpacingControl (PopupContainer* pParent);
    PopupControl* CreateBulletsPopupControl (PopupContainer* pParent);
    PopupControl* CreateNumberingPopupControl (PopupContainer* pParent);
    PopupControl* CreateBGColorPopupControl (PopupContainer* pParent);
    DECL_LINK(ClickLineSPDropDownHdl_Impl, ToolBox*);
    DECL_LINK(NumBTbxDDHandler, ToolBox*);
    DECL_LINK(ToolBoxBackColorDDHandler, ToolBox *);

    void InitToolBoxAlign();
    void InitToolBoxVertAlign();
    void InitToolBoxIndent();
    void InitToolBoxBGColor();
    void InitToolBoxBulletsNumbering();
    void InitToolBoxSpacing();
    void InitToolBoxLineSpacing();

    void SetBGColor (const OUString& rsColorName, const Color aColor);
};

} } // end of namespace ::svx::sidebar

#endif


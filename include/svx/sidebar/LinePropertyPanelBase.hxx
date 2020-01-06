/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX

#include <vcl/weld.hxx>
#include <memory>
#include <svl/poolitem.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/xtable.hxx>
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/svxdllapi.h>

class ToolbarUnoDispatcher;
class XLineStyleItem;
class XLineDashItem;
class XLineStartItem;
class XLineWidthItem;
class XLineEndItem;
class XLineEndList;
class XLineJointItem;
class XLineCapItem;
class XLineTransparenceItem;
class XDashList;

namespace svx
{
namespace sidebar
{

class SVX_DLLPUBLIC LinePropertyPanelBase : public PanelLayout
{
public:
    virtual ~LinePropertyPanelBase() override;
    virtual void dispose() override;

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    void SetWidth(long nWidth);
    void SetWidthIcon(int n);
    void SetWidthIcon();

    // constructor/destructor
    LinePropertyPanelBase(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void setLineWidth(const XLineWidthItem& rItem) = 0;

protected:

    virtual void setLineStyle(const XLineStyleItem& rItem) = 0;
    virtual void setLineDash(const XLineDashItem& rItem) = 0;
    virtual void setLineEndStyle(const XLineEndItem* pItem) = 0;
    virtual void setLineStartStyle(const XLineStartItem* pItem) = 0;
    virtual void setLineTransparency(const XLineTransparenceItem& rItem) = 0;
    virtual void setLineJoint(const XLineJointItem* pItem) = 0;
    virtual void setLineCap(const XLineCapItem* pItem) = 0;


    void updateLineStyle(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineDash(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineTransparence(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineWidth(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineStart(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineEnd(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineJoint(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineCap(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);

    void FillLineEndList();
    void FillLineStyleList();
    void SelectEndStyle(bool bStart);
    void SelectLineStyle();
    void ActivateControls();

    void setMapUnit(MapUnit eMapUnit);

    void disableArrowHead();

protected:

    std::unique_ptr<weld::Toolbar> mxTBColor;
    std::unique_ptr<ToolbarUnoDispatcher> mxColorDispatch;

private:
    //ui controls
    std::unique_ptr<weld::Label> mxFTWidth;
    std::unique_ptr<weld::Toolbar> mxTBWidth;
    std::unique_ptr<SvxLineLB> mxLBStyle;
    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::MetricSpinButton> mxMFTransparent;
    std::unique_ptr<SvxLineEndLB> mxLBStart;
    std::unique_ptr<SvxLineEndLB> mxLBEnd;
    std::unique_ptr<weld::Label> mxFTEdgeStyle;
    std::unique_ptr<weld::ComboBox> mxLBEdgeStyle;
    std::unique_ptr<weld::Label> mxFTCapStyle;
    std::unique_ptr<weld::ComboBox> mxLBCapStyle;
    std::unique_ptr<weld::Widget> mxGridLineProps;
    std::unique_ptr<weld::Widget> mxBoxArrowProps;
    //popup windows
    std::unique_ptr<LineWidthPopup> mxLineWidthPopup;

    std::unique_ptr<XLineStyleItem> mpStyleItem;
    std::unique_ptr<XLineDashItem>  mpDashItem;

    sal_uInt16      mnTrans;
    MapUnit         meMapUnit;
    sal_Int32       mnWidthCoreValue;
    XLineEndListRef mxLineEndList;
    XDashListRef    mxLineStyleList;
    std::unique_ptr<XLineStartItem> mpStartItem;
    std::unique_ptr<XLineEndItem>   mpEndItem;

    // images from resource
    OUString maIMGNone;

    // multi-images
    OUString maIMGWidthIcon[8];

    bool                mbWidthValuable : 1;
    bool mbArrowSupported;

    void Initialize();

    DECL_LINK(ChangeLineStyleHdl, weld::ComboBox&, void);
    DECL_LINK(ToolboxWidthSelectHdl, const OString&, void);
    DECL_LINK(ChangeTransparentHdl, weld::MetricSpinButton&, void );
    DECL_LINK(ChangeStartHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeEndHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeEdgeStyleHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeCapStyleHdl, weld::ComboBox&, void);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SVX_PAGECTRL_HXX
#define INCLUDED_SVX_PAGECTRL_HXX

#include <vcl/customweld.hxx>
#include <svx/svxdllapi.h>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

enum class SvxPageUsage;
enum class SvxFrameDirection;

class SVX_DLLPUBLIC SvxPageWindow : public weld::CustomWidgetController
{
private:
    Size aWinSize;
    Size aSize;

    tools::Long nTop;
    tools::Long nBottom;
    tools::Long nLeft;
    tools::Long nRight;

    bool bResetBackground;
    bool bFrameDirection;
    SvxFrameDirection nFrameDirection;

    tools::Long nHdLeft;
    tools::Long nHdRight;
    tools::Long nHdDist;
    tools::Long nHdHeight;

    tools::Long nFtLeft;
    tools::Long nFtRight;
    tools::Long nFtDist;
    tools::Long nFtHeight;

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maHeaderFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFooterFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maPageFillAttributes;

    bool bFooter : 1;
    bool bHeader : 1;
    bool bTable : 1;
    bool bHorz : 1;
    bool bVert : 1;

    SvxPageUsage eUsage;

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void DrawPage(vcl::RenderContext& rRenderContext, const Point& rPoint,
                          const bool bSecond, const bool bEnabled);

    static void drawFillAttributes(vcl::RenderContext& rRenderContext,
                            const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
                            const tools::Rectangle& rPaintRange, const tools::Rectangle& rDefineRange);

public:
    SvxPageWindow();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SvxPageWindow() override;

    void setHeaderFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes)
    {
        maHeaderFillAttributes = rFillAttributes;
    }
    void setFooterFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes)
    {
        maFooterFillAttributes = rFillAttributes;
    }
    void setPageFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes)
    {
        maPageFillAttributes = rFillAttributes;
    }
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& getPageFillAttributes() const
    {
        return maPageFillAttributes;
    }
    void SetSize(const Size& rSize)
    {
        aSize = rSize;
    }
    const Size& GetSize() const
    {
        return aSize;
    }

    void SetTop(tools::Long nNew) { nTop = nNew; }
    void SetBottom(tools::Long nNew) { nBottom = nNew; }
    void SetLeft(tools::Long nNew) { nLeft = nNew; }
    void SetRight(tools::Long nNew) { nRight = nNew; }

    tools::Long GetTop() const { return nTop; }
    tools::Long GetBottom() const { return nBottom; }
    tools::Long GetLeft() const { return nLeft; }
    tools::Long GetRight() const { return nRight; }

    void SetHdLeft(tools::Long nNew) { nHdLeft = nNew; }
    void SetHdRight(tools::Long nNew) { nHdRight = nNew; }
    void SetHdDist(tools::Long nNew) { nHdDist = nNew; }
    void SetHdHeight(tools::Long nNew) { nHdHeight = nNew; }

    tools::Long GetHdLeft() const { return nHdLeft; }
    tools::Long GetHdRight() const { return nHdRight; }
    tools::Long GetHdDist() const { return nHdDist; }
    tools::Long GetHdHeight() const { return nHdHeight; }

    void SetFtLeft(tools::Long nNew) { nFtLeft = nNew; }
    void SetFtRight(tools::Long nNew) { nFtRight = nNew; }
    void SetFtDist(tools::Long nNew) { nFtDist = nNew; }
    void SetFtHeight(tools::Long nNew) { nFtHeight = nNew; }

    tools::Long GetFtLeft() const { return nFtLeft; }
    tools::Long GetFtRight() const { return nFtRight; }
    tools::Long GetFtDist() const { return nFtDist; }
    tools::Long GetFtHeight() const { return nFtHeight; }

    void SetUsage(SvxPageUsage eU) { eUsage = eU; }
    SvxPageUsage GetUsage() const { return eUsage; }

    void SetHeader( bool bNew ) { bHeader = bNew; }
    void SetFooter( bool bNew ) { bFooter = bNew; }
    void SetTable( bool bNew ) { bTable = bNew; }
    void SetHorz( bool bNew ) { bHorz = bNew; }
    void SetVert( bool bNew ) { bVert = bNew; }

    void EnableFrameDirection(bool bEnable);
    void SetFrameDirection(SvxFrameDirection nDirection);

    void ResetBackground();
};

#endif // INCLUDED_SVX_PAGECTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

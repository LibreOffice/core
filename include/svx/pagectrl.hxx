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

#include <svx/svxdllapi.h>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <vcl/weld/customweld.hxx>

enum class SvxPageUsage;
enum class SvxFrameDirection;

class SVX_DLLPUBLIC SvxPageWindow : public weld::CustomWidgetController
{
private:
    Size m_aWinSize;
    Size m_aSize;

    tools::Long m_nTop;
    tools::Long m_nBottom;
    tools::Long m_nLeft;
    tools::Long m_nRight;

    bool m_bResetBackground;
    bool m_bFrameDirection;
    SvxFrameDirection m_nFrameDirection;

    tools::Long m_nHdLeft;
    tools::Long m_nHdRight;
    tools::Long m_nHdDist;
    tools::Long m_nHdHeight;

    tools::Long m_nFtLeft;
    tools::Long m_nFtRight;
    tools::Long m_nFtDist;
    tools::Long m_nFtHeight;

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maHeaderFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFooterFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maPageFillAttributes;

    bool m_bFooter : 1;
    bool m_bHeader : 1;
    bool m_bTable : 1;
    bool m_bHorz : 1;
    bool m_bVert : 1;

    SvxPageUsage m_eUsage;

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
        m_aSize = rSize;
    }
    const Size& GetSize() const
    {
        return m_aSize;
    }

    void SetTop(tools::Long nNew) { m_nTop = nNew; }
    void SetBottom(tools::Long nNew) { m_nBottom = nNew; }
    void SetLeft(tools::Long nNew) { m_nLeft = nNew; }
    void SetRight(tools::Long nNew) { m_nRight = nNew; }

    tools::Long GetTop() const { return m_nTop; }
    tools::Long GetBottom() const { return m_nBottom; }
    tools::Long GetLeft() const { return m_nLeft; }
    tools::Long GetRight() const { return m_nRight; }

    void SetHdLeft(tools::Long nNew) { m_nHdLeft = nNew; }
    void SetHdRight(tools::Long nNew) { m_nHdRight = nNew; }
    void SetHdDist(tools::Long nNew) { m_nHdDist = nNew; }
    void SetHdHeight(tools::Long nNew) { m_nHdHeight = nNew; }

    tools::Long GetHdLeft() const { return m_nHdLeft; }
    tools::Long GetHdRight() const { return m_nHdRight; }
    tools::Long GetHdDist() const { return m_nHdDist; }
    tools::Long GetHdHeight() const { return m_nHdHeight; }

    void SetFtLeft(tools::Long nNew) { m_nFtLeft = nNew; }
    void SetFtRight(tools::Long nNew) { m_nFtRight = nNew; }
    void SetFtDist(tools::Long nNew) { m_nFtDist = nNew; }
    void SetFtHeight(tools::Long nNew) { m_nFtHeight = nNew; }

    tools::Long GetFtLeft() const { return m_nFtLeft; }
    tools::Long GetFtRight() const { return m_nFtRight; }
    tools::Long GetFtDist() const { return m_nFtDist; }
    tools::Long GetFtHeight() const { return m_nFtHeight; }

    void SetUsage(SvxPageUsage eU) { m_eUsage = eU; }
    SvxPageUsage GetUsage() const { return m_eUsage; }

    void SetHeader( bool bNew ) { m_bHeader = bNew; }
    void SetFooter( bool bNew ) { m_bFooter = bNew; }
    void SetTable( bool bNew ) { m_bTable = bNew; }
    void SetHorz( bool bNew ) { m_bHorz = bNew; }
    void SetVert( bool bNew ) { m_bVert = bNew; }

    void EnableFrameDirection(bool bEnable);
    void SetFrameDirection(SvxFrameDirection nDirection);

    void ResetBackground();
};

#endif // INCLUDED_SVX_PAGECTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

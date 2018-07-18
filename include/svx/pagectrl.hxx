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
#include <vcl/window.hxx>
#include <svx/svxdllapi.h>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

class SvxBoxItem;
enum class SvxPageUsage;
enum class SvxFrameDirection;

class SVX_DLLPUBLIC SvxPageWindow : public vcl::Window
{
    using Window::GetBorder;

private:
    Size aWinSize;
    Size aSize;

    long nTop;
    long nBottom;
    long nLeft;
    long nRight;

    long nHdLeft;
    long nHdRight;
    long nHdDist;
    long nHdHeight;

    long nFtLeft;
    long nFtRight;
    long nFtDist;
    long nFtHeight;

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maHeaderFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFooterFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maPageFillAttributes;

    bool bFooter : 1;
    bool bHeader : 1;

    SvxPageUsage eUsage;

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void DrawPage(vcl::RenderContext& rRenderContext, const Point& rPoint,
                          const bool bSecond, const bool bEnabled);

    void drawFillAttributes(vcl::RenderContext& rRenderContext,
                            const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
                            const tools::Rectangle& rPaintRange, const tools::Rectangle& rDefineRange);

public:
    SvxPageWindow(vcl::Window* pParent);
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

    void SetTop(long nNew) { nTop = nNew; }
    void SetBottom(long nNew) { nBottom = nNew; }
    void SetLeft(long nNew) { nLeft = nNew; }
    void SetRight(long nNew) { nRight = nNew; }

    long GetTop() const { return nTop; }
    long GetBottom() const { return nBottom; }
    long GetLeft() const { return nLeft; }
    long GetRight() const { return nRight; }

    void SetHdLeft(long nNew) { nHdLeft = nNew; }
    void SetHdRight(long nNew) { nHdRight = nNew; }
    void SetHdDist(long nNew) { nHdDist = nNew; }
    void SetHdHeight(long nNew) { nHdHeight = nNew; }

    long GetHdDist() const { return nHdDist; }
    long GetHdHeight() const { return nHdHeight; }

    void SetFtLeft(long nNew) { nFtLeft = nNew; }
    void SetFtRight(long nNew) { nFtRight = nNew; }
    void SetFtDist(long nNew) { nFtDist = nNew; }
    void SetFtHeight(long nNew) { nFtHeight = nNew; }

    long GetFtDist() const { return nFtDist; }
    long GetFtHeight() const { return nFtHeight; }

    void SetUsage(SvxPageUsage eU) { eUsage = eU; }
    SvxPageUsage GetUsage() const { return eUsage; }

    void SetHeader( bool bNew ) { bHeader = bNew; }
    void SetFooter( bool bNew ) { bFooter = bNew; }

    virtual Size GetOptimalSize() const override;
};

class SVX_DLLPUBLIC PageWindow : public weld::CustomWidgetController
{
private:
    Size aWinSize;
    Size aSize;

    long nTop;
    long nBottom;
    long nLeft;
    long nRight;

    bool bResetBackground;
    bool bFrameDirection;
    SvxFrameDirection nFrameDirection;

    long nHdLeft;
    long nHdRight;
    long nHdDist;
    long nHdHeight;

    long nFtLeft;
    long nFtRight;
    long nFtDist;
    long nFtHeight;

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

    void drawFillAttributes(vcl::RenderContext& rRenderContext,
                            const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
                            const tools::Rectangle& rPaintRange, const tools::Rectangle& rDefineRange);

public:
    PageWindow();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~PageWindow() override;

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
    void SetSize(const Size& rSize)
    {
        aSize = rSize;
    }
    const Size& GetSize() const
    {
        return aSize;
    }

    void SetTop(long nNew) { nTop = nNew; }
    void SetBottom(long nNew) { nBottom = nNew; }
    void SetLeft(long nNew) { nLeft = nNew; }
    void SetRight(long nNew) { nRight = nNew; }

    long GetTop() const { return nTop; }
    long GetBottom() const { return nBottom; }
    long GetLeft() const { return nLeft; }
    long GetRight() const { return nRight; }

    void SetHdLeft(long nNew) { nHdLeft = nNew; }
    void SetHdRight(long nNew) { nHdRight = nNew; }
    void SetHdDist(long nNew) { nHdDist = nNew; }
    void SetHdHeight(long nNew) { nHdHeight = nNew; }

    long GetHdLeft() const { return nHdLeft; }
    long GetHdRight() const { return nHdRight; }
    long GetHdDist() const { return nHdDist; }
    long GetHdHeight() const { return nHdHeight; }

    void SetFtLeft(long nNew) { nFtLeft = nNew; }
    void SetFtRight(long nNew) { nFtRight = nNew; }
    void SetFtDist(long nNew) { nFtDist = nNew; }
    void SetFtHeight(long nNew) { nFtHeight = nNew; }

    long GetFtLeft() const { return nFtLeft; }
    long GetFtRight() const { return nFtRight; }
    long GetFtDist() const { return nFtDist; }
    long GetFtHeight() const { return nFtHeight; }

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

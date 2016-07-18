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

#ifndef INCLUDED_SFX2_THUMBNAILVIEWITEM_HXX
#define INCLUDED_SFX2_THUMBNAILVIEWITEM_HXX

#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <osl/mutex.hxx>
#include <vcl/bitmapex.hxx>
#include <sfx2/dllapi.h>

#include <com/sun/star/accessibility/XAccessible.hpp>

#define THUMBNAILVIEW_ITEM_NONEITEM      0xFFFE

const int THUMBNAILVIEW_ITEM_CORNER = 5;

class ThumbnailView;
class MouseEvent;

namespace basegfx {
    class B2DPolygon;
}

namespace drawinglayer {
    namespace processor2d {
        class BaseProcessor2D;
    }

    namespace primitive2d {
        class PolygonHairlinePrimitive2D;
    }
}

struct ThumbnailItemAttributes
{
    sal_uInt32 nMaxTextLength;
    basegfx::BColor aFillColor;
    basegfx::BColor aTextColor;
    basegfx::BColor aHighlightColor;
    basegfx::BColor aHighlightTextColor;
    basegfx::BColor aSelectHighlightColor;
    basegfx::BColor aSelectHighlightTextColor;
    double fHighlightTransparence;
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr;
};

class SFX2_DLLPUBLIC ThumbnailViewItem
{
public:

    ThumbnailView &mrParent;
    sal_uInt16 mnId;
    bool mbVisible;
    bool mbSelected;
    bool mbHover;
    BitmapEx maPreview1;
    OUString maTitle;
    OUString maHelpText;
    css::uno::Reference< css::accessibility::XAccessible > mxAcc;

    ThumbnailViewItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~ThumbnailViewItem ();

    bool isVisible () const { return mbVisible; }

    void show (bool bVisible);

    bool isSelected () const { return mbSelected; }

    void setSelection (bool state);

    bool isHighlighted () const { return mbHover; }

    void setHighlight (bool state);

    /** Updates own highlight status based on the aPoint position.

        Returns rectangle that needs to be invalidated.
    */
    virtual Rectangle updateHighlight(bool bVisible, const Point& rPoint);

    /// Text to be used for the tooltip.

    void setHelpText (const OUString &sText) { maHelpText = sText; }

    virtual OUString getHelpText() const { return maHelpText; };
    virtual OUString getTitle() const { return maTitle; };

    void setTitle (const OUString& rTitle);

    css::uno::Reference< css::accessibility::XAccessible > const &
                        GetAccessible( bool bIsTransientChildrenDisabled );

    void setDrawArea (const Rectangle &area);

    const Rectangle& getDrawArea () const { return maDrawArea; }

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLength,
                                         const ThumbnailItemAttributes *pAttrs);

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);
    void addTextPrimitives (const OUString& rText, const ThumbnailItemAttributes *pAttrs, Point aPos, drawinglayer::primitive2d::Primitive2DContainer& rSeq);

    static drawinglayer::primitive2d::PolygonHairlinePrimitive2D*
        createBorderLine (const basegfx::B2DPolygon &rPolygon);

    virtual void MouseButtonUp(const MouseEvent&) {}

protected:

    Point maTextPos;
    Point maPrev1Pos;
    Rectangle maDrawArea;
};

#endif // INCLUDED_SFX2_THUMBNAILVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

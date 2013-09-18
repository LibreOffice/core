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

#ifndef THUMBNAILVIEWITEM_HXX
#define THUMBNAILVIEWITEM_HXX

#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <osl/mutex.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/vclmedit.hxx>
#include "sfx2/dllapi.h"

#include <com/sun/star/accessibility/XAccessible.hpp>

#define THUMBNAILVIEW_ITEM_NONEITEM      0xFFFE

class CheckBox;
class Font;
class Window;
class ThumbnailView;

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
    sal_uInt32 nMaxTextLenght;
    basegfx::BColor aFillColor;
    basegfx::BColor aHighlightColor;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >* mpxAcc;

    ThumbnailViewItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~ThumbnailViewItem ();

    bool isVisible () const { return mbVisible; }

    void show (bool bVisible);

    bool isSelected () const { return mbSelected; }

    void setSelection (bool state);

    bool isHighlighted () const { return mbHover; }

    void setHighlight (bool state);

    virtual void setEditTitle (bool edit, bool bChangeFocus = true);
    void updateTitleEditSize ();
    virtual void setTitle (const OUString& rTitle);

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetAccessible( bool bIsTransientChildrenDisabled );

    void setDrawArea (const Rectangle &area);

    const Rectangle& getDrawArea () const { return maDrawArea; }
    Rectangle getTextArea () const;

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLenght,
                                         const ThumbnailItemAttributes *pAttrs);

    const Point& getTextPos () const { return maTextPos; }

    const Point& getPrev1Pos () const { return maPrev1Pos; }

    void setSelectClickHdl (const Link &link);

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);
    void addTextPrimitives (const OUString& rText, const ThumbnailItemAttributes *pAttrs, Point aPos, drawinglayer::primitive2d::Primitive2DSequence& rSeq);

    static drawinglayer::primitive2d::PolygonHairlinePrimitive2D*
        createBorderLine (const basegfx::B2DPolygon &rPolygon);

protected:

    Point maTextPos;
    Point maPrev1Pos;
    Rectangle maDrawArea;
    Link maClickHdl;
    bool mbEditTitle;
    VclMultiLineEdit* mpTitleED;
    Rectangle maTextEditMaxArea;
};

#endif // THUMBNAILVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

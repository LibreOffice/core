/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef THUMBNAILVIEWITEM_HXX
#define THUMBNAILVIEWITEM_HXX

#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <osl/mutex.hxx>
#include <vcl/bitmapex.hxx>
#include "sfx2/dllapi.h"

#include <com/sun/star/accessibility/XAccessible.hpp>

#define THUMBNAILVIEW_ITEM_NONEITEM      0xFFFE

class CheckBox;
class Font;
class Window;
class ThumbnailView;

namespace basegfx {
    class B2DPoint;
}

namespace drawinglayer {
    namespace processor2d {
        class BaseProcessor2D;
    }

    namespace primitive2d {
        class BorderLinePrimitive2D;
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
    rtl::OUString maTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >* mpxAcc;

    ThumbnailViewItem (ThumbnailView &rView, Window *pParent);

    virtual ~ThumbnailViewItem ();

    bool isVisible () const { return mbVisible; }

    void show (bool bVisible);

    bool isSelected () const { return mbSelected; }

    void setSelection (bool state);

    bool isHighlighted () const { return mbHover; }

    void setHighlight (bool state);

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetAccessible( bool bIsTransientChildrenDisabled );

    void setDrawArea (const Rectangle &area);

    const Rectangle& getDrawArea () const { return maDrawArea; }

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLenght,
                                         const ThumbnailItemAttributes *pAttrs);

    const Point& getTextPos () const { return maTextPos; }

    const Point& getPrev1Pos () const { return maPrev1Pos; }

    void setSelectionMode (bool mode);

    void setSelectClickHdl (const Link &link);

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);

    static drawinglayer::primitive2d::BorderLinePrimitive2D*
        createBorderLine (const basegfx::B2DPoint &rStart, const basegfx::B2DPoint &rEnd);

private:

    DECL_LINK (OnClick, CheckBox *);

protected:

    bool mbMode;
    Point maTextPos;
    Point maPrev1Pos;
    Rectangle maDrawArea;
    Link maClickHdl;
    CheckBox *mpSelectBox;
};

#endif // THUMBNAILVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

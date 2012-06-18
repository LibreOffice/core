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

#include <osl/mutex.hxx>
#include <vcl/bitmapex.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>

#define THUMBNAILVIEW_ITEM_NONEITEM      0xFFFE

class CheckBox;
class Window;
class ThumbnailView;

struct ThumbnailViewItem
{
    ThumbnailView &mrParent;
    sal_uInt16 mnId;
    bool mbVisible;
    bool mbSelected;
    bool mbHover;
    BitmapEx maPreview1;
    BitmapEx maPreview2;
    rtl::OUString maText;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >* mpxAcc;

    ThumbnailViewItem (ThumbnailView &rView, Window *pParent);

    ~ThumbnailViewItem ();

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

    void calculateItemsPosition ();

    const Point& getTextPos () const { return maTextPos; }

    const Point& getPrev1Pos () const { return maPrev1Pos; }

    const Point& getPrev2Pos () const { return maPrev2Pos; }

    void setSelectionMode (bool mode);

    void setSelectClickHdl (const Link &link);

    bool isInsideTitle (const Point &pt) const;

    void Paint (const Rectangle &aRect);

private:

    DECL_LINK (OnClick, CheckBox *);

private:

    bool mbMode;
    Point maTextPos;
    Point maPrev1Pos;
    Point maPrev2Pos;
    Rectangle maDrawArea;
    Link maClickHdl;
    CheckBox *mpSelectBox;
};

#endif // THUMBNAILVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

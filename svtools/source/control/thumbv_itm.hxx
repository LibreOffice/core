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

#ifndef THUMBV_ITM_HXX
#define THUMBV_ITM_HXX

#include <osl/mutex.hxx>
#include <tools/color.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>

#define THUMBNAILVIEW_ITEM_NONEITEM      0xFFFE

enum ThumbnailViewItemType
{
    THUMBNAILITEM_NONE,
    THUMBNAILITEM_IMAGE,
    THUMBNAILITEM_COLOR,
    THUMBNAILITEM_USERDRAW
};

class ThumbnailView;

struct ThumbnailViewItem
{
    ThumbnailView &mrParent;
    sal_uInt16 mnId;
    sal_uInt8 meType;
    bool mbVisible;
    Image maImage;
    Color maColor;
    rtl::OUString maText;
    void* mpData;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >* mpxAcc;

    ThumbnailViewItem ( ThumbnailView& rParent );
    ~ThumbnailViewItem ();

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetAccessible( bool bIsTransientChildrenDisabled );
};

#endif // THUMBV_ITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef LAYOUT_CORE_BOX_HXX
#define LAYOUT_CORE_BOX_HXX

#include <layout/core/box-base.hxx>

#include <com/sun/star/awt/Point.hpp>

namespace layoutimpl
{

class Box : public Box_Base
{
protected:
    // Box properties (i.e. affect all children)
    sal_Int32 mnSpacing;
    sal_Bool mbHomogeneous;
    sal_Bool mbHorizontal;  // false for Vertical
    bool mbHasFlowChildren;

public:
    // Children properties
    struct ChildData : public Box_Base::ChildData
    {
        sal_Int32 mnPadding;
        sal_Bool mbExpand;
        sal_Bool mbFill;
        ChildData( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
    };

    struct ChildProps : public Box_Base::ChildProps
    {
        ChildProps( ChildData *pData );
    };

protected:
    ChildData *createChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
    ChildProps *createChildProps( Box_Base::ChildData* pData );

public:
    Box( bool horizontal );

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException);

    // helper: mix of getMinimumSize() and getHeightForWidth()
    css::awt::Size calculateSize( long nWidth = 0 );

private:
    /* Helpers to deal with the joint Box directions. */
    inline int primDim (const css::awt::Size &size)
    { if (mbHorizontal) return size.Width; else return size.Height; }
    inline int secDim (const css::awt::Size &size)
    { if (mbHorizontal) return size.Height; else return size.Width; }
    inline int primDim (const css::awt::Point &point)
    { if (mbHorizontal) return point.X; else return point.Y; }
    inline int secDim (const css::awt::Point &point)
    { if (mbHorizontal) return point.Y; else return point.X; }
};

struct VBox : public Box
{ VBox() : Box (false) {} };

struct HBox : public Box
{ HBox() : Box (true) {} };

} //  namespace layoutimpl

#endif /* LAYOUT_CORE_BOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

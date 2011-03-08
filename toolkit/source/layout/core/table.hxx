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

#ifndef LAYOUT_CORE_TABLE_HXX
#define LAYOUT_CORE_TABLE_HXX

#include <layout/core/box-base.hxx>

namespace layoutimpl
{

class Table : public Box_Base
{
public:
    // Children properties
    struct ChildData : public Box_Base::ChildData
    {
        sal_Bool mbExpand[ 2 ];
        sal_Int32 mnColSpan;
        sal_Int32 mnRowSpan;
        int mnLeftCol;
        int mnRightCol;
        int mnTopRow;
        int mnBottomRow;

        ChildData( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
        bool isVisible();
    };

    struct ChildProps : public Box_Base::ChildProps
    {
        ChildProps( ChildData *pData );
    };

protected:

    // a group of children; either a column or a row
    struct GroupData
    {
        sal_Bool mbExpand;
        int mnSize;  // request size (width or height)
        GroupData() : mbExpand( false ), mnSize( 0 ) {}
    };

    // Table properties
    sal_Int32 mnColsLen;
    std::vector< GroupData > maCols;
    std::vector< GroupData > maRows;
    int mnColExpandables, mnRowExpandables;

    ChildData *createChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
    ChildProps *createChildProps( Box_Base::ChildData* pData );

public:
    Table();

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);

    // unimplemented:
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException)
    { return false; }
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 /*nWidth*/ )
    throw(css::uno::RuntimeException)
    { return maRequisition.Height; }
};

} //  namespace layoutimpl

#endif /* LAYOUT_CORE_TABLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

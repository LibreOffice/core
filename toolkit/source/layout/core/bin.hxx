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

/* A few simple binary containers */

#ifndef LAYOUT_CORE_BIN_HXX
#define LAYOUT_CORE_BIN_HXX

#include <layout/core/container.hxx>

namespace layoutimpl
{

class Bin : public Container
{
protected:
    // Child
    css::awt::Size maChildRequisition;
    css::uno::Reference< css::awt::XLayoutConstrains > mxChild;

public:
    Bin();
    virtual ~Bin() {}

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference
                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException);

    // css::awt::XLayoutConstrains
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);
};

// Align gives control over child position on the allocated space.
class Align : public Bin
{
    friend class AlignChildProps;
protected:
    // properties
    float fHorAlign, fVerAlign;
    float fHorFill, fVerFill;

public:
    Align();

    bool emptyVisible ();

    // css::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);
};

// Makes child request its or a specified size, whatever is larger.
class MinSize : public Bin
{
protected:
    // properties
    long mnMinWidth, mnMinHeight;

public:
    MinSize();

    bool emptyVisible ();
    // css::awt::XLayoutContainer
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);
};

} //  namespace layoutimpl

#endif /* LAYOUT_CORE_BIN_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

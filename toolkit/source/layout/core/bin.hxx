/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

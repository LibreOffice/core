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



#ifndef LAYOUT_CORE_FLOW_HXX
#define LAYOUT_CORE_FLOW_HXX

#include <layout/core/container.hxx>

#include <list>

namespace layoutimpl
{

class Flow : public Container
{
protected:
    // Box properties (i.e. affect all children)
    sal_Int32 mnSpacing;
    sal_Bool mbHomogeneous;

public:
    // Children properties
    struct ChildData
    {
        css::awt::Size aRequisition;
        css::uno::Reference< css::awt::XLayoutConstrains > xChild;
        css::uno::Reference< css::beans::XPropertySet >    xProps;
        bool isVisible();
    };

protected:
    std::list< ChildData * > maChildren;
    long mnEachWidth;  // on homogeneous, the width of every child

public:
    Flow();

    bool emptyVisible ();

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference
                                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException);

private:
    // shared between getMinimumSize() and getHeightForWidth()
    css::awt::Size calculateSize( long nMaxWidth );
};

} //  namespace layoutimpl

#endif /* LAYOUT_FLOW_CORE_HXX */

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

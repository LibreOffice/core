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

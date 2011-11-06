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



#include <table.hxx>

#include <sal/macros.h>
#include <osl/mutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/awt/PosSize.hpp>
#include <tools/debug.hxx>

// fixed point precision for distributing error
#define FIXED_PT 16

namespace layoutimpl
{

using namespace com::sun::star;

Table::ChildProps::ChildProps( Table::ChildData *pData )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "XExpand" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &( pData->mbExpand[ 0 ] ) );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "YExpand" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &( pData->mbExpand[ 1 ] ) );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "ColSpan" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &( pData->mnColSpan ) );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "RowSpan" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &( pData->mnRowSpan ) );
}

bool Table::ChildData::isVisible()
{
    return Box_Base::ChildData::isVisible()
        && ( mnColSpan > 0 ) && ( mnRowSpan > 0 );
}

Table::Table()
    : Box_Base()
    , mnColsLen( 1 )// another default value could be 0xffff for infinite columns( = 1 row )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Columns" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnColsLen );
}

Table::ChildData::ChildData( uno::Reference< awt::XLayoutConstrains > const& xChild )
    : Box_Base::ChildData( xChild )
//    , mbExpand( { 1, 1 } )
    , mnColSpan( 1 )
    , mnRowSpan( 1 )
    , mnLeftCol( 0 )
    , mnRightCol( 0 )
    , mnTopRow( 0 )
    , mnBottomRow( 0 )
{
    mbExpand[ 0 ] = 1;
    mbExpand[ 1 ] = 1;
}

Table::ChildData*
Table::createChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
{
    return new ChildData( xChild );
}

Table::ChildProps*
Table::createChildProps( Box_Base::ChildData *pData )
{
    return new ChildProps( static_cast<Table::ChildData*> ( pData ) );
}

void SAL_CALL
Table::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw( uno::RuntimeException, awt::MaxChildrenException )
{
    if ( xChild.is() )
    {
        Box_Base::addChild( xChild );
        // cause of flicker
        allocateChildAt( xChild, awt::Rectangle( 0,0,0,0 ) );
    }
}

awt::Size SAL_CALL
Table::getMinimumSize() throw( uno::RuntimeException )
{
    int nRowsLen = 0;

    // 1. layout the table -- adjust to cope with row-spans...
    {
        // temporary 1D representation of the table
        std::vector< ChildData *> aTable;

        int col = 0;
        int row = 0;
        for ( std::list<Box_Base::ChildData *>::iterator it
                  = maChildren.begin(); it != maChildren.end(); it++ )
        {
            ChildData *child = static_cast<Table::ChildData*> ( *it );
            if ( !child->isVisible() )
                continue;

            while ( col + SAL_MIN( child->mnColSpan, mnColsLen ) > mnColsLen )
            {
                col = 0;
                row++;

                unsigned int i = col +( row*mnColsLen );
                while ( aTable.size() > i && !aTable[ i ] )
                    i++;

                col = i % mnColsLen;
                row = i / mnColsLen;
            }

            child->mnLeftCol = col;
            child->mnRightCol = SAL_MIN( col + child->mnColSpan, mnColsLen );
            child->mnTopRow = row;
            child->mnBottomRow = row + child->mnRowSpan;

            col += child->mnColSpan;

            unsigned int start = child->mnLeftCol +( child->mnTopRow*mnColsLen );
            unsigned int end =( child->mnRightCol-1 ) +( ( child->mnBottomRow-1 )*mnColsLen );
            if ( aTable.size() < end+1 )
                aTable.resize( end+1, NULL );
            for ( unsigned int i = start; i < end; i++ )
                aTable[ i ] = child;

            nRowsLen = SAL_MAX( nRowsLen, child->mnBottomRow );
        }
    }

    // 2. calculate columns/rows sizes
    for ( int g = 0; g < 2; g++ )
    {
        std::vector< GroupData > &aGroup = g == 0 ? maCols : maRows;

        aGroup.clear();
        aGroup.resize( g == 0 ? mnColsLen : nRowsLen );

        // 2.1 base sizes on one-column/row children
        for ( std::list<Box_Base::ChildData *>::iterator it
                  = maChildren.begin(); it != maChildren.end(); it++ )
        {
            ChildData *child = static_cast<Table::ChildData*> ( *it );
            if ( !child->isVisible() )
                continue;
            const int nFirstAttach = g == 0 ? child->mnLeftCol : child->mnTopRow;
            const int nLastAttach  = g == 0 ? child->mnRightCol : child->mnBottomRow;

            if ( nFirstAttach == nLastAttach-1 )
            {
                child->maRequisition = child->mxChild->getMinimumSize();
                int attach = nFirstAttach;
                int child_size = g == 0 ? child->maRequisition.Width
                    : child->maRequisition.Height;
                aGroup[ attach ].mnSize = SAL_MAX( aGroup[ attach ].mnSize,
                                                   child_size );
                if ( child->mbExpand[ g ] )
                    aGroup[ attach ].mbExpand = true;
            }
        }

        // 2.2 make sure multiple-columns/rows children fit
        for ( std::list<Box_Base::ChildData *>::iterator it
                  = maChildren.begin(); it != maChildren.end(); it++ )
        {
            ChildData *child = static_cast<Table::ChildData*> ( *it );
            if ( !child->isVisible() )
                continue;
            const int nFirstAttach = g == 0 ? child->mnLeftCol : child->mnTopRow;
            const int nLastAttach  = g == 0 ? child->mnRightCol : child->mnBottomRow;

            if ( nFirstAttach != nLastAttach-1 )
            {
                child->maRequisition = child->mxChild->getMinimumSize();
                int size = 0;
                int expandables = 0;
                for ( int i = nFirstAttach; i < nLastAttach; i++ )
                {
                    size += aGroup[ i ].mnSize;
                    if ( aGroup[ i ].mbExpand )
                        expandables++;
                }

                int child_size = g == 0 ? child->maRequisition.Width
                    : child->maRequisition.Height;
                int extra = child_size - size;
                if ( extra > 0 )
                {
                    if ( expandables )
                        extra /= expandables;
                    else
                        extra /= nLastAttach - nFirstAttach;

                    for ( int i = nFirstAttach; i < nLastAttach; i++ )
                        if ( expandables == 0 || aGroup[ i ].mbExpand )
                            aGroup[ i ].mnSize += extra;
                }
            }
        }
    }

    // 3. Sum everything up
    mnColExpandables =( mnRowExpandables = 0 );
    maRequisition.Width =( maRequisition.Height = 0 );
    for ( std::vector<GroupData>::iterator it = maCols.begin();
          it != maCols.end(); it++ )
    {
        maRequisition.Width += it->mnSize;
        if ( it->mbExpand )
            mnColExpandables++;
    }
    for ( std::vector<GroupData>::iterator it = maRows.begin();
          it != maRows.end(); it++ )
    {
        maRequisition.Height += it->mnSize;
        if ( it->mbExpand )
            mnRowExpandables++;
    }

    return maRequisition;
}

void SAL_CALL
Table::allocateArea( const awt::Rectangle &rArea )
    throw( uno::RuntimeException )
{
    maAllocation = rArea;
    if ( maCols.size() == 0 || maRows.size() == 0 )
        return;

    int nExtraSize[ 2 ] = { SAL_MAX( rArea.Width - maRequisition.Width, 0 ),
                            SAL_MAX( rArea.Height - maRequisition.Height, 0 ) };
    // split it
    nExtraSize[ 0 ] /= mnColExpandables ? mnColExpandables : mnColsLen;
    nExtraSize[ 1 ] /= mnRowExpandables ? mnRowExpandables : maRows.size();

    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = static_cast<Table::ChildData*> ( *it );
        if ( !child->isVisible() )
            continue;

        awt::Rectangle rChildArea( rArea.X, rArea.Y, 0, 0 );

        for ( int g = 0; g < 2; g++ )
        {
            std::vector< GroupData > &aGroup = g == 0 ? maCols : maRows;
            const int nFirstAttach = g == 0 ? child->mnLeftCol : child->mnTopRow;
            const int nLastAttach  = g == 0 ? child->mnRightCol : child->mnBottomRow;

            for ( int i = 0; i < nFirstAttach; i++ )
            {
                int gSize = aGroup[ i ].mnSize;
                if ( aGroup[ i ].mbExpand )
                    gSize += nExtraSize[ g ];
                if ( g == 0 )
                    rChildArea.X += gSize;
                else
                    rChildArea.Y += gSize;
            }
            for ( int i = nFirstAttach; i < nLastAttach; i++ )
            {
                int gSize = aGroup[ i ].mnSize;
                if ( aGroup[ i ].mbExpand )
                    gSize += nExtraSize[ g ];
                if ( g == 0 )
                    rChildArea.Width  += gSize;
                else
                    rChildArea.Height += gSize;
            }
        }

        allocateChildAt( child->mxChild, rChildArea );
    }
}

} // namespace layoutimpl

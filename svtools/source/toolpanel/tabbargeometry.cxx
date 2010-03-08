/*************************************************************************
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

#include "precompiled_svtools.hxx"

#include "tabbargeometry.hxx"

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <vcl/window.hxx>

#include <algorithm>

// the width (or height, depending on alignment) of the scroll buttons
#define BUTTON_FLOW_WIDTH       20
// the space between the scroll buttons and the items
#define BUTTON_FLOW_SPACE       2
// outer space to apply between the tab bar borders and any content. Note that those refer to a "normalized" geometry,
// i.e. if the tab bar were aligned at the top
#define OUTER_SPACE_LEFT        2
#define OUTER_SPACE_TOP         4
#define OUTER_SPACE_RIGHT       4
#define OUTER_SPACE_BOTTOM      2

// outer space to apply between the area for the items, and the actual items. They refer to a normalized geometry.
#define ITEMS_INSET_LEFT        2
#define ITEMS_INSET_TOP         2
#define ITEMS_INSET_RIGHT       4
#define ITEMS_INSET_BOTTOM      0

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        static void lcl_transform( Rectangle& io_rRect, const ::basegfx::B2DHomMatrix& i_rTransformation )
        {
            ::basegfx::B2DRange aRect( io_rRect.Left(), io_rRect.Top(), io_rRect.Right(), io_rRect.Bottom() );
            aRect.transform( i_rTransformation );
            io_rRect.Left() = long( aRect.getMinX() );
            io_rRect.Top() = long( aRect.getMinY() );
            io_rRect.Right() = long( aRect.getMaxX() );
            io_rRect.Bottom() = long( aRect.getMaxY() );
        }

        //--------------------------------------------------------------------------------------------------------------
        /** transforms the given, possible rotated playground,
        */
        void lcl_rotate( const Rectangle& i_rReference, Rectangle& io_rArea, const bool i_bRight )
        {
            // step 1: move the to-be-upper-left corner (left/bottom) of the rectangle to (0,0)
            ::basegfx::B2DHomMatrix aTransformation;
            aTransformation.translate(
                i_bRight ? -i_rReference.Left() : -i_rReference.Right(),
                i_bRight ? -i_rReference.Bottom() : -i_rReference.Top()
            );

            // step 2: rotate by -90 degrees
            aTransformation.rotate( i_bRight ? +F_PI2 : -F_PI2 );
                // note:
                // on the screen, the ordinate goes top-down, while basegfx calculates in a system where the
                // ordinate goes bottom-up; thus the "wrong" sign before F_PI2 here

            // step 3: move back to original coordinates
            aTransformation.translate( i_rReference.Left(), i_rReference.Top() );

            // apply transformation
            lcl_transform( io_rArea, aTransformation );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void lcl_mirrorHorizontally( const Rectangle& i_rReferenceArea, Rectangle& io_rArea )
    {
        io_rArea.Left() = i_rReferenceArea.Left() + i_rReferenceArea.Right() - io_rArea.Left();
        io_rArea.Right() = i_rReferenceArea.Left() + i_rReferenceArea.Right() - io_rArea.Right();
        ::std::swap( io_rArea.Left(), io_rArea.Right() );
    }

    //==================================================================================================================
    //= NormalizedArea
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    NormalizedArea::NormalizedArea( const Rectangle& i_rReference, const bool i_bIsVertical )
        :m_aReference( i_bIsVertical ? Rectangle( i_rReference.TopLeft(), Size( i_rReference.GetHeight(), i_rReference.GetWidth() ) ) : i_rReference )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle NormalizedArea::getTransformed( const Rectangle& i_rArea, const TabAlignment i_eTargetAlignment ) const
    {
        OSL_ENSURE( ( i_eTargetAlignment == TABS_LEFT ) || ( i_eTargetAlignment == TABS_RIGHT ),
            "NormalizedArea::getTransformed: unsupported alignment!" );

        Rectangle aResult( i_rArea );

        if  (   ( i_eTargetAlignment == TABS_RIGHT )
            ||  ( i_eTargetAlignment == TABS_LEFT )
            )
        {
            lcl_rotate( m_aReference, aResult, true );

            if ( i_eTargetAlignment == TABS_LEFT )
            {
                Rectangle aReference( m_aReference );
                aReference.Transpose();
                lcl_mirrorHorizontally( aReference, aResult );
            }
        }
        return aResult;
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle NormalizedArea::getNormalized( const Rectangle& i_rArea, const TabAlignment i_eTargetAlignment ) const
    {
        OSL_ENSURE( ( i_eTargetAlignment == TABS_LEFT ) || ( i_eTargetAlignment == TABS_RIGHT ),
            "NormalizedArea::gerNormalized: unsupported alignment!" );

        Rectangle aResult( i_rArea );

        if  (   ( i_eTargetAlignment == TABS_RIGHT )
            ||  ( i_eTargetAlignment == TABS_LEFT )
            )
        {
            Rectangle aReference( m_aReference );
            lcl_rotate( m_aReference, aReference, true );

            if ( i_eTargetAlignment == TABS_LEFT )
            {
                lcl_mirrorHorizontally( aReference, aResult );
            }

            lcl_rotate( aReference, aResult, false );
        }
        return aResult;
    }

    //==================================================================================================================
    //= TabBarGeometry
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TabBarGeometry::TabBarGeometry( const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :m_eTabAlignment( i_eAlignment )
        ,m_eTabItemContent( i_eItemContent )
        ,m_aItemsInset()
        ,m_aNormalizedPlayground( Rectangle(), false )
        ,m_aButtonBackRect()
        ,m_aItemsRect()
        ,m_aButtonForwardRect()
    {
        // calculate the items' inset
        const Rectangle aArtificial( Point( 0, 0 ), Size( 10, 10 ) );
        const Rectangle aInsetRect(
            Point( ITEMS_INSET_LEFT, ITEMS_INSET_TOP ),
            Size(
                aArtificial.GetWidth() - ITEMS_INSET_LEFT - ITEMS_INSET_RIGHT,
                aArtificial.GetHeight() - ITEMS_INSET_TOP - ITEMS_INSET_BOTTOM
            )
        );

        const NormalizedArea aNormalized( aArtificial, false );
        const Rectangle aTransformedInner( aNormalized.getTransformed( aInsetRect, getAlignment() ) );

        m_aItemsInset.nLeft   = aTransformedInner.Left()            - aNormalized.getReference().Left();
        m_aItemsInset.nTop    = aTransformedInner.Top()             - aNormalized.getReference().Top() ;
        m_aItemsInset.nRight  = aNormalized.getReference().Right()  - aTransformedInner.Right()        ;
        m_aItemsInset.nBottom = aNormalized.getReference().Bottom() - aTransformedInner.Bottom()       ;
    }

    //------------------------------------------------------------------------------------------------------------------
    TabBarGeometry::~TabBarGeometry()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TabBarGeometry::impl_fitItems( ItemDescriptors& io_rItems ) const
    {
        if ( io_rItems.empty() )
            // nothing to do, "no items" perfectly fit into any space we have ...
            return true;

        // the available size
        Size aOutputSize( getItemsRect().GetSize() );
        // shrunk by the outer space
        aOutputSize.Width() -= m_aItemsInset.nRight;
        aOutputSize.Height() -= m_aItemsInset.nBottom;
        const Rectangle aFitInto( Point( 0, 0 ), aOutputSize );

        TabItemContent eItemContent( getItemContent() );
        if ( eItemContent == TABITEM_AUTO )
        {
            // the "content modes" to try
            TabItemContent eTryThis[] =
            {
                TABITEM_IMAGE_ONLY,     // assumed to have the smallest rects
                TABITEM_TEXT_ONLY,
                TABITEM_IMAGE_AND_TEXT  // assumed to have the largest rects
            };


            // determine which of the different version fits
            eItemContent = eTryThis[0];
            size_t nTryIndex = 2;
            while ( nTryIndex > 0 )
            {
                const Point aBottomRight( io_rItems.rbegin()->GetRect( eTryThis[ nTryIndex ] ).BottomRight() );
                if ( aFitInto.IsInside( aBottomRight ) )
                {
                    eItemContent = eTryThis[ nTryIndex ];
                    break;
                }
                --nTryIndex;
            }
        }

        // propagate to the items
        for (   ItemDescriptors::iterator item = io_rItems.begin();
                item != io_rItems.end();
                ++item
            )
        {
            item->eContent = eItemContent;
        }

        const ItemDescriptor& rLastItem( *io_rItems.rbegin() );
        const Point aLastItemBottomRight( rLastItem.GetCurrentRect().BottomRight() );
        return aFitInto.IsInside( aLastItemBottomRight );
    }

    //------------------------------------------------------------------------------------------------------------------
    Size TabBarGeometry::getOptimalSize( ItemDescriptors& io_rItems, const bool i_bMinimalSize ) const
    {
        if ( io_rItems.empty() )
            return Size(
                m_aItemsInset.nLeft + m_aItemsInset.nRight,
                m_aItemsInset.nTop + m_aItemsInset.nBottom
            );

        // the rect of the last item
        const Rectangle& rLastItemRect( i_bMinimalSize ? io_rItems.rbegin()->aIconOnlyArea : io_rItems.rbegin()->aCompleteArea );
        const Point aBottomRight( rLastItemRect.BottomRight() );
        return Size(
                    aBottomRight.X() + 1 + m_aItemsInset.nRight,
                    aBottomRight.Y() + 1 + m_aItemsInset.nBottom
                );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TabBarGeometry::relayout( const Size& i_rActualOutputSize, ItemDescriptors& io_rItems )
    {
        m_aNormalizedPlayground = NormalizedArea( Rectangle( Point(), i_rActualOutputSize ), isVertical() );
        const Size aNormalizedSize( m_aNormalizedPlayground.getReferenceSize() );

        // assume all items fit
        Point aButtonBackPos( OUTER_SPACE_LEFT, OUTER_SPACE_TOP );
        m_aButtonBackRect = m_aNormalizedPlayground.getTransformed( Rectangle( aButtonBackPos, Size( 1, 1 ) ), getAlignment() );
        m_aButtonBackRect.SetEmpty();   // do this after the transformation, it cannot cope with empty rects

        Point aButtonForwardPos( aNormalizedSize.Width(), OUTER_SPACE_TOP );
        m_aButtonForwardRect = m_aNormalizedPlayground.getTransformed( Rectangle( aButtonForwardPos, Size( 1, 1 ) ), getAlignment() );
        m_aButtonBackRect.SetEmpty();   // do this after the transformation, it cannot cope with empty rects

        Point aItemsPos( OUTER_SPACE_LEFT, 0 );
        Size aItemsSize( aNormalizedSize.Width() - OUTER_SPACE_LEFT - OUTER_SPACE_RIGHT, aNormalizedSize.Height() );
        m_aItemsRect = m_aNormalizedPlayground.getTransformed( Rectangle( aItemsPos, aItemsSize ), m_eTabAlignment );

        if ( !impl_fitItems( io_rItems ) )
        {
            // assumption was wrong, the items do not fit => calculate rects for the scroll buttons
            const Size aButtonSize( BUTTON_FLOW_WIDTH, aNormalizedSize.Height() - OUTER_SPACE_TOP - OUTER_SPACE_BOTTOM );

            aButtonBackPos = Point( OUTER_SPACE_LEFT, OUTER_SPACE_TOP );
            m_aButtonBackRect = m_aNormalizedPlayground.getTransformed( Rectangle( aButtonBackPos, aButtonSize ), m_eTabAlignment );

            aButtonForwardPos = Point( aNormalizedSize.Width() - BUTTON_FLOW_WIDTH - OUTER_SPACE_RIGHT, OUTER_SPACE_TOP );
            m_aButtonForwardRect = m_aNormalizedPlayground.getTransformed( Rectangle( aButtonForwardPos, aButtonSize ), m_eTabAlignment );

            aItemsPos.X() = aButtonBackPos.X() + aButtonSize.Width() + BUTTON_FLOW_SPACE;
            aItemsSize.Width() = aButtonForwardPos.X() - BUTTON_FLOW_SPACE - aItemsPos.X();
            m_aItemsRect = m_aNormalizedPlayground.getTransformed( Rectangle( aItemsPos, aItemsSize ), m_eTabAlignment );

            // fit items, again. In the TABITEM_AUTO case, the smaller playground for the items might lead to another
            // item content.
            impl_fitItems( io_rItems );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Point TabBarGeometry::getFirstItemPosition() const
    {
        return Point( m_aItemsInset.nLeft, m_aItemsInset.nTop );
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................

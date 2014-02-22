/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "tabbargeometry.hxx"

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <vcl/window.hxx>

#include <algorithm>


#define BUTTON_FLOW_WIDTH       20

#define BUTTON_FLOW_SPACE       2


#define OUTER_SPACE_LEFT        2
#define OUTER_SPACE_TOP         4
#define OUTER_SPACE_RIGHT       4
#define OUTER_SPACE_BOTTOM      2


#define ITEMS_INSET_LEFT        4
#define ITEMS_INSET_TOP         3
#define ITEMS_INSET_RIGHT       4
#define ITEMS_INSET_BOTTOM      0


namespace svt
{


    
    
    
    namespace
    {
        
        static void lcl_transform( Rectangle& io_rRect, const ::basegfx::B2DHomMatrix& i_rTransformation )
        {
            ::basegfx::B2DRange aRect( io_rRect.Left(), io_rRect.Top(), io_rRect.Right(), io_rRect.Bottom() );
            aRect.transform( i_rTransformation );
            io_rRect.Left() = long( aRect.getMinX() );
            io_rRect.Top() = long( aRect.getMinY() );
            io_rRect.Right() = long( aRect.getMaxX() );
            io_rRect.Bottom() = long( aRect.getMaxY() );
        }

        
        /** transforms the given, possible rotated playground,
        */
        void lcl_rotate( const Rectangle& i_rReference, Rectangle& io_rArea, const bool i_bRight )
        {
            
            ::basegfx::B2DHomMatrix aTransformation;
            aTransformation.translate(
                i_bRight ? -i_rReference.Left() : -i_rReference.Right(),
                i_bRight ? -i_rReference.Bottom() : -i_rReference.Top()
            );

            
            aTransformation.rotate( i_bRight ? +F_PI2 : -F_PI2 );
                
                
                

            
            aTransformation.translate( i_rReference.Left(), i_rReference.Top() );

            
            lcl_transform( io_rArea, aTransformation );
        }
    }

    
    void lcl_mirrorHorizontally( const Rectangle& i_rReferenceArea, Rectangle& io_rArea )
    {
        io_rArea.Left() = i_rReferenceArea.Left() + i_rReferenceArea.Right() - io_rArea.Left();
        io_rArea.Right() = i_rReferenceArea.Left() + i_rReferenceArea.Right() - io_rArea.Right();
        ::std::swap( io_rArea.Left(), io_rArea.Right() );
    }

    
    void lcl_mirrorVertically( const Rectangle& i_rReferenceArea, Rectangle& io_rArea )
    {
        io_rArea.Top() = i_rReferenceArea.Top() + i_rReferenceArea.Bottom() - io_rArea.Top();
        io_rArea.Bottom() = i_rReferenceArea.Top() + i_rReferenceArea.Bottom() - io_rArea.Bottom();
        ::std::swap( io_rArea.Top(), io_rArea.Bottom() );
    }

    
    
    
    
    NormalizedArea::NormalizedArea()
        :m_aReference()
    {
    }

    
    NormalizedArea::NormalizedArea( const Rectangle& i_rReference, const bool i_bIsVertical )
        :m_aReference( i_bIsVertical ? Rectangle( i_rReference.TopLeft(), Size( i_rReference.GetHeight(), i_rReference.GetWidth() ) ) : i_rReference )
    {
    }

    
    Rectangle NormalizedArea::getTransformed( const Rectangle& i_rArea, const TabAlignment i_eTargetAlignment ) const
    {
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
        else if  ( i_eTargetAlignment == TABS_BOTTOM )
        {
            lcl_mirrorVertically( m_aReference, aResult );
        }

        return aResult;
    }

    
    Rectangle NormalizedArea::getNormalized( const Rectangle& i_rArea, const TabAlignment i_eTargetAlignment ) const
    {
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
        else if  ( i_eTargetAlignment == TABS_BOTTOM )
        {
            lcl_mirrorVertically( m_aReference, aResult );
        }
        return aResult;
    }

    
    
    
    
    TabBarGeometry::TabBarGeometry( const TabItemContent i_eItemContent )
        :m_eTabItemContent( i_eItemContent )
        ,m_aItemsInset()
        ,m_aButtonBackRect()
        ,m_aItemsRect()
        ,m_aButtonForwardRect()
    {
        m_aItemsInset.Left()   = ITEMS_INSET_LEFT;
        m_aItemsInset.Top()    = ITEMS_INSET_TOP;
        m_aItemsInset.Right()  = ITEMS_INSET_RIGHT;
        m_aItemsInset.Bottom() = ITEMS_INSET_BOTTOM;
    }

    
    TabBarGeometry::~TabBarGeometry()
    {
    }

    
    bool TabBarGeometry::impl_fitItems( ItemDescriptors& io_rItems ) const
    {
        if ( io_rItems.empty() )
            
            return true;

        
        Size aOutputSize( getItemsRect().GetSize() );
        
        aOutputSize.Width() -= m_aItemsInset.Right();
        aOutputSize.Height() -= m_aItemsInset.Bottom();
        const Rectangle aFitInto( Point( 0, 0 ), aOutputSize );

        TabItemContent eItemContent( getItemContent() );
        if ( eItemContent == TABITEM_AUTO )
        {
            
            TabItemContent eTryThis[] =
            {
                TABITEM_IMAGE_ONLY,     
                TABITEM_TEXT_ONLY,
                TABITEM_IMAGE_AND_TEXT  
            };


            
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

        
        for (   ItemDescriptors::iterator item = io_rItems.begin();
                item != io_rItems.end();
                ++item
            )
        {
            item->eContent = eItemContent;
        }

        const ItemDescriptor& rLastItem( *io_rItems.rbegin() );
        const Point aLastItemBottomRight( rLastItem.GetCurrentRect().BottomRight() );
        return  aFitInto.Left() <= aLastItemBottomRight.X()
            &&  aFitInto.Right() >= aLastItemBottomRight.X();
    }

    
    Size TabBarGeometry::getOptimalSize(ItemDescriptors& io_rItems) const
    {
        if ( io_rItems.empty() )
            return Size(
                m_aItemsInset.Left() + m_aItemsInset.Right(),
                m_aItemsInset.Top() + m_aItemsInset.Bottom()
            );

        
        const Rectangle& rLastItemRect(io_rItems.rbegin()->aCompleteArea);
        return Size(
                    rLastItemRect.Left() + 1 + m_aItemsInset.Right(),
                    rLastItemRect.Top() + 1 + rLastItemRect.Bottom() + m_aItemsInset.Bottom()
                );
    }

    
    void TabBarGeometry::relayout( const Size& i_rActualOutputSize, ItemDescriptors& io_rItems )
    {
        
        Point aButtonBackPos( OUTER_SPACE_LEFT, OUTER_SPACE_TOP );
        m_aButtonBackRect = Rectangle( aButtonBackPos, Size( 1, 1 ) );
        m_aButtonBackRect.SetEmpty();

        Point aButtonForwardPos( i_rActualOutputSize.Width(), OUTER_SPACE_TOP );
        m_aButtonForwardRect = Rectangle( aButtonForwardPos, Size( 1, 1 ) );
        m_aButtonForwardRect.SetEmpty();

        Point aItemsPos( OUTER_SPACE_LEFT, 0 );
        Size aItemsSize( i_rActualOutputSize.Width() - OUTER_SPACE_LEFT - OUTER_SPACE_RIGHT, i_rActualOutputSize.Height() );
        m_aItemsRect = Rectangle( aItemsPos, aItemsSize );

        if ( !impl_fitItems( io_rItems ) )
        {
            
            const Size aButtonSize( BUTTON_FLOW_WIDTH, i_rActualOutputSize.Height() - OUTER_SPACE_TOP - OUTER_SPACE_BOTTOM );

            aButtonBackPos = Point( OUTER_SPACE_LEFT, OUTER_SPACE_TOP );
            m_aButtonBackRect = Rectangle( aButtonBackPos, aButtonSize );

            aButtonForwardPos = Point( i_rActualOutputSize.Width() - BUTTON_FLOW_WIDTH - OUTER_SPACE_RIGHT, OUTER_SPACE_TOP );
            m_aButtonForwardRect = Rectangle( aButtonForwardPos, aButtonSize );

            aItemsPos.X() = aButtonBackPos.X() + aButtonSize.Width() + BUTTON_FLOW_SPACE;
            aItemsSize.Width() = aButtonForwardPos.X() - BUTTON_FLOW_SPACE - aItemsPos.X();
            m_aItemsRect = Rectangle( aItemsPos, aItemsSize );

            
            
            impl_fitItems( io_rItems );
        }
    }

    
    Point TabBarGeometry::getFirstItemPosition() const
    {
        return Point( m_aItemsInset.Left(), m_aItemsInset.Top() );
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

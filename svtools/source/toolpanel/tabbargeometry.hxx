/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef TABBARGEOMETRY_HXX
#define TABBARGEOMETRY_HXX

#include "svtools/toolpanel/tabalignment.hxx"

#include "tabitemdescriptor.hxx"

#include <tools/gen.hxx>
#include <tools/svborder.hxx>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    //==================================================================================================================
    //= NormalizedArea
    //==================================================================================================================
    /** a rectangle which automatically translates between unrotated and rotated geometry.

        It can be operated as if it were an unrotated area, but is able to provide corrdinates of rotated objects,
        relative to its playground.
    */
    class NormalizedArea
    {
    public:
        NormalizedArea();
        NormalizedArea( const Rectangle& i_rReference, const bool i_bIsVertical );

        /** transforms a rectangle, relative to our playground, into a coordinate system defined by the given alignment
            @param i_rArea
                the area which is to be transformed.
        */
        Rectangle   getTransformed(
                        const Rectangle& i_rArea,
                        const TabAlignment i_eTargetAlignment
                    ) const;

        /** normalizes an already transformed rectangle
            @param i_rArea
                the area which is to be normalized.
        */
        Rectangle   getNormalized(
                        const Rectangle& i_rArea,
                        const TabAlignment i_eTargetAlignment
                    ) const;


        Size        getReferenceSize() const { return m_aReference.GetSize(); }
        const Rectangle&
                    getReference() const { return m_aReference; }

    private:
        // the normalized reference area
        Rectangle   m_aReference;
    };

    //==================================================================================================================
    //= TabBarGeometry
    //==================================================================================================================
    class TabBarGeometry_Impl;
    class TabBarGeometry
    {
    public:
        TabBarGeometry( const TabItemContent i_eItemContent );
        ~TabBarGeometry();

        // retrieves the rectangle to be occupied by the button for scrolling backward through the items
        const Rectangle&    getButtonBackRect() const { return m_aButtonBackRect; }
        // retrieves the rectangle to be occupied by the items
        const Rectangle&    getItemsRect() const { return m_aItemsRect; }
        // retrieves the rectangle to be occupied by the button for scrolling forward through the items
        const Rectangle&    getButtonForwardRect() const { return m_aButtonForwardRect; }

        inline TabItemContent
                            getItemContent() const { return m_eTabItemContent; }
        inline void         setItemContent( const TabItemContent i_eItemContent ) { m_eTabItemContent = i_eItemContent; }

        /** adjusts the sizes of the buttons and the item's playground, plus the sizes of the items
        */
        void    relayout( const Size& i_rActualOutputSize, ItemDescriptors& io_rItems );

        /** calculates the optimal size of the tab bar, depending on the item's sizes
        */
        Size    getOptimalSize( ItemDescriptors& io_rItems, const bool i_bMinimalSize ) const;

        /** retrieves the position where the first item should start, relative to the item rect
        */
        Point   getFirstItemPosition() const;

    private:
        bool    impl_fitItems( ItemDescriptors& io_rItems ) const;

    private:
        /// specifies the content to be displayed in the tab items
        TabItemContent      m_eTabItemContent;
        /// specifies the inset to be used in the items area, depends on the actual alignment
        SvBorder            m_aItemsInset;
        // the (logical) rectangle to be used for the "back" button, empty if the button is not needed
        Rectangle           m_aButtonBackRect;
        // the (logical) rectangle to be used for the items
        Rectangle           m_aItemsRect;
        // the (logical) rectangle to be used for the "forward" button, empty if the button is not needed
        Rectangle           m_aButtonForwardRect;
    };

//......................................................................................................................
} // namespace svt
//......................................................................................................................

#endif // TABBARGEOMETRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

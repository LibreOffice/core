/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABBARGEOMETRY_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABBARGEOMETRY_HXX

#include "svtools/toolpanel/tabalignment.hxx"

#include "tabitemdescriptor.hxx"

#include <tools/gen.hxx>
#include <tools/svborder.hxx>


namespace svt
{



    //= NormalizedArea

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


    //= TabBarGeometry

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
        Size    getOptimalSize(ItemDescriptors& io_rItems) const;

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


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABBARGEOMETRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

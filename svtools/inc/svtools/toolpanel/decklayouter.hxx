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

#ifndef SVT_DECKLAYOUTER_HXX
#define SVT_DECKLAYOUTER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ref.hxx>

#include <boost/optional.hpp>

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
} } } }
class Rectangle;
class Point;

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IDeckLayouter
    //====================================================================
    class IDeckLayouter : public ::rtl::IReference
    {
    public:
        /** re-arranges the elements of the tool deck, taking into account the
            available space for the complete deck.

            @param i_rDeckPlayground
                the playground for the complete tool panel deck
            @return
                the content area for a single tool panel
        */
        virtual ::Rectangle Layout( const ::Rectangle& i_rDeckPlayground ) = 0;

        /** destroys the instance

            Since the layouter is ref-counted, but might keep references to non-ref-counted objects
            (in particular, the ToolPanelDeck, which is a VCL-Window, and thus cannot be ref-counted),
            Destroy is the definitive way to dispose the instance. Technically, it's still alive afterwards,
            but non-functional.
        */
        virtual void        Destroy() = 0;

        /** assuming that a layouter neesds to provide some kind of panel selector control, this method
            requests to set the focus to this control.
        */
        virtual void        SetFocusToPanelSelector() = 0;

        /** returns the number of components in the XAccessible hierarchy which are needed to represent all elements
            the layouter is responsible form.

            Note that the implementation must guarantee that the count is fixed over the life time of the layouter.
        */
        virtual size_t      GetAccessibleChildCount() const = 0;

        /** retrieves the XAccessible implementation for the <code>i_nChildIndex</code>'th child in the XAccessible
            hierarchy.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            ) = 0;

        virtual ~IDeckLayouter()
        {
        }
    };

    typedef ::rtl::Reference< IDeckLayouter >   PDeckLayouter;

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_DECKLAYOUTER_HXX

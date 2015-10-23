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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_DECKLAYOUTER_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_DECKLAYOUTER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <boost/optional.hpp>

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
} } } }
class Rectangle;
class Point;


namespace svt
{



    //= IDeckLayouter

    class IDeckLayouter : public salhelper::SimpleReferenceObject
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

        /** assuming that a layouter needs to provide some kind of panel selector control, this method
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
        virtual css::uno::Reference< css::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const css::uno::Reference< css::accessibility::XAccessible >& i_rParentAccessible
                            ) = 0;

        virtual ~IDeckLayouter()
        {
        }
    };

    typedef ::rtl::Reference< IDeckLayouter >   PDeckLayouter;


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_DECKLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

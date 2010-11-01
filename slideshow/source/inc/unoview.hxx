/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef INCLUDED_SLIDESHOW_UNOVIEW_HXX
#define INCLUDED_SLIDESHOW_UNOVIEW_HXX

#include "view.hxx"
#include <com/sun/star/uno/Reference.hxx>

#include <vector>

namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }


/* Definition of UnoView interface */

namespace slideshow
{
    namespace internal
    {
        /** Extend View with UNO interface retrieval.

            This interface extends View with an UNO interface
            retrieval, to be used for Views which are set from
            external API.
         */
        class UnoView : public View
        {
        public:
            /** Retrieve the underlying UNO slide view.
             */
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowView > getUnoView() const = 0;

            /** Dispose view

                This needs to be different from Disposable interface,
                as the UNO XComponent also provides a dispose() (only
                with a different calling convention under Windows).
             */
            virtual void _dispose() = 0;

            /** Return whether the sound play back is enabled.
            */
            virtual bool isSoundEnabled (void) const = 0;

            /** Tell the view whether it may play sounds.  Disabling this
                can be used to prevent different views to play the same
                sounds at the same time.
            */
            virtual void setIsSoundEnabled (const bool bValue) = 0;
        };

        typedef ::boost::shared_ptr< UnoView >      UnoViewSharedPtr;
        typedef ::std::vector< UnoViewSharedPtr >   UnoViewVector;
    }
}

#endif /* INCLUDED_SLIDESHOW_UNOVIEW_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

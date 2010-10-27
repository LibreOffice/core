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

#ifndef INCLUDED_SLIDESHOW_GDIMTFTOOLS_HXX
#define INCLUDED_SLIDESHOW_GDIMTFTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <basegfx/range/b2drectangle.hxx>
#include <boost/shared_ptr.hpp>

#include "tools.hxx"

#include <vector>

class MetaAction;
class GDIMetaFile;
class Graphic;

// -----------------------------------------------------------------------------

namespace slideshow
{
    namespace internal
    {
        /// meta file loading specialities:
        enum mtf_load_flags {
            /// no flags
            MTF_LOAD_NONE = 0,
            /// annotate text actions with verbose comments,
            /// denoting logical and physical text entities.
            MTF_LOAD_VERBOSE_COMMENTS = 1,
            /// the source of the metafile might be a foreign
            /// application. The metafile is checked against unsupported
            /// content, and, if necessary, returned as a pre-rendererd
            /// bitmap.
            MTF_LOAD_FOREIGN_SOURCE = 2,
            /// retrieve a meta file for the page background only
            MTF_LOAD_BACKGROUND_ONLY = 4,
            /// retrieve the drawing layer scroll text metafile
            MTF_LOAD_SCROLL_TEXT_MTF = 8
        };

        // Animation info
        // ==============

        struct MtfAnimationFrame
        {
            MtfAnimationFrame( const GDIMetaFileSharedPtr& rMtf,
                               double                      nDuration ) :
                mpMtf( rMtf ),
                mnDuration( nDuration )
            {
            }

            /// Enables STL algos to be used for duration extraction
            double getDuration() const
            {
                return mnDuration;
            }

            GDIMetaFileSharedPtr    mpMtf;
            double                  mnDuration;
        };

        typedef ::std::vector< MtfAnimationFrame > VectorOfMtfAnimationFrames;


        /** Retrieve a meta file for the given shape

            @param xShape
            XShape to retrieve a metafile for.

            @param xContainingPage
            The page that contains this shape. Needed for proper
            import (currently, the UnoGraphicExporter needs this
            information).

            @param o_rMtf
            Metafile to extract shape content into
        */
        bool getMetaFile( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&     xSource,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >&   xContainingPage,
                          GDIMetaFile&                                                                      o_rMtf,
                          int                                                                               mtfLoadFlags,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XComponentContext >&                                   rxContext );

        /** Remove all text actions from the given metafile.
         */
        void removeTextActions( GDIMetaFile& io_rMtf );

        /** Gets the next action offset for iterating meta actions which is most
            often returns 1.
        */
        sal_Int32 getNextActionOffset( MetaAction * pCurrAct );

        /** Extract a vector of animation frames from given Graphic.

            @param o_rFrames
            Resulting vector of animated metafiles

            @param o_rLoopCount
            Number of times the bitmap animation shall be repeated

            @param o_eCycleMode
            Repeat mode (normal, or ping-pong mode)

            @param rGraphic
            Input graphic object, to extract animations from
         */
        bool getAnimationFromGraphic( VectorOfMtfAnimationFrames& o_rFrames,
                                      ::std::size_t&              o_rLoopCount,
                                      CycleMode&                  o_eCycleMode,
                                      const Graphic&              rGraphic );

        /** Retrieve scroll text animation rectangles from given metafile

            @return true, if both rectangles have been found, false
            otherwise.
         */
        bool getRectanglesFromScrollMtf( ::basegfx::B2DRectangle&       o_rScrollRect,
                                         ::basegfx::B2DRectangle&       o_rPaintRect,
                                         const GDIMetaFileSharedPtr&    rMtf );
    }
}

#endif /* INCLUDED_SLIDESHOW_GDIMTFTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

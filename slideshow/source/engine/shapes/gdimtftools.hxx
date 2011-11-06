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

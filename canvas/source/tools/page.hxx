/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: page.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CANVAS_PAGE_HXX
#define INCLUDED_CANVAS_PAGE_HXX

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/isurface.hxx>

#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "surfacerect.hxx"

namespace canvas
{
    class PageFragment;

    typedef ::boost::shared_ptr< PageFragment > FragmentSharedPtr;

    /** One page of IRenderModule-provided texture space
     */
    class Page
    {
    public:
        Page( const IRenderModuleSharedPtr& rRenderModule );

        FragmentSharedPtr        allocateSpace( const ::basegfx::B2ISize& rSize );
        bool                     nakedFragment( const FragmentSharedPtr& pFragment );
        void                     free( const FragmentSharedPtr& pFragment );
        const ISurfaceSharedPtr& getSurface() const { return mpSurface; }
        bool                     isValid() const;
        void                     validate();

    private:
        typedef std::list<FragmentSharedPtr> FragmentContainer_t;

        IRenderModuleSharedPtr  mpRenderModule;
        ISurfaceSharedPtr       mpSurface;
        FragmentContainer_t     mpFragments;

        bool insert( SurfaceRect& r );
        bool isValidLocation( const SurfaceRect& r ) const;
    };

    typedef ::boost::shared_ptr< Page > PageSharedPtr;


    /** A part of a page, which gets allocated to a surface
     */
    class PageFragment
    {
    public:
        PageFragment( const SurfaceRect& r,
                      Page*              pPage ) :
            mpPage(pPage),
            maRect(r),
            mpBuffer(),
            maSourceOffset()
        {
        }

        /// Creates a 'naked' fragment.
        PageFragment( const ::basegfx::B2ISize& rSize ) :
            mpPage(NULL),
            maRect(rSize),
            mpBuffer(),
            maSourceOffset()
        {
        }

        bool                        isNaked() const { return (mpPage == NULL); }
        const SurfaceRect&          getRect() const { return maRect; }
        const ::basegfx::B2IPoint&  getPos() const { return maRect.maPos; }
        const ::basegfx::B2ISize&   getSize() const { return maRect.maSize; }
        void                        setColorBuffer( const IColorBufferSharedPtr& pColorBuffer ) { mpBuffer=pColorBuffer; }
        void                        setSourceOffset( const ::basegfx::B2IPoint& rOffset ) { maSourceOffset=rOffset; }
        void                        setPage( Page* pPage ) { mpPage=pPage; }

        void free( const FragmentSharedPtr& pFragment )
        {
            if(mpPage)
                mpPage->free(pFragment);

            mpPage=NULL;
        }

        bool select( bool bRefresh )
        {
            // request was made to select this fragment,
            // but this fragment has not been located on any
            // of the available pages, we need to hurry now.
            if(!(mpPage))
                return false;

            ISurfaceSharedPtr pSurface(mpPage->getSurface());

            // select this surface before wiping the contents
            // since a specific implementation could trigger
            // a rendering operation here...
            if(!(pSurface->selectTexture()))
                return false;

            // call refresh() if requested, otherwise we're up to date...
            return bRefresh ? refresh() : true;
        }

        bool refresh()
        {
            if(!(mpPage))
                return false;

            ISurfaceSharedPtr pSurface(mpPage->getSurface());

            return pSurface->update( maRect.maPos,
                                     ::basegfx::B2IRectangle(
                                         maSourceOffset,
                                         maSourceOffset + maRect.maSize ),
                                     *mpBuffer );
        }

    private:
        Page*                 mpPage;
        SurfaceRect           maRect;
        IColorBufferSharedPtr mpBuffer;
        ::basegfx::B2IPoint   maSourceOffset;
    };
}

#endif

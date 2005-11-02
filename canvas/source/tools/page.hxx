/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: page.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:54:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_PAGE_HXX
#define INCLUDED_CANVAS_PAGE_HXX

#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif
#ifndef _BGFX_RANGE_B2IRECTANGLE_HXX
#include <basegfx/range/b2irectangle.hxx>
#endif
#ifndef INCLUDED_CANVAS_ICOLORBUFFER_HXX
#include <canvas/rendering/icolorbuffer.hxx>
#endif
#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#include <canvas/rendering/irendermodule.hxx>
#endif
#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#include <canvas/rendering/isurface.hxx>
#endif

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

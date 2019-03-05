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

#ifndef INCLUDED_CANVAS_SOURCE_TOOLS_PAGE_HXX
#define INCLUDED_CANVAS_SOURCE_TOOLS_PAGE_HXX

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/isurface.hxx>

#include <memory>
#include <vector>
#include "surfacerect.hxx"

namespace canvas
{
    class PageFragment;

    typedef std::shared_ptr< PageFragment > FragmentSharedPtr;

    /** One page of IRenderModule-provided texture space
     */
    class Page
    {
    public:
        explicit Page( const std::shared_ptr<IRenderModule>& rRenderModule );

        FragmentSharedPtr        allocateSpace( const ::basegfx::B2ISize& rSize );
        bool                     nakedFragment( const FragmentSharedPtr& pFragment );
        void                     free( const FragmentSharedPtr& pFragment );
        const std::shared_ptr<ISurface>& getSurface() const { return mpSurface; }
        bool                     isValid() const;
        void                     validate();

    private:
        typedef std::vector<FragmentSharedPtr> FragmentContainer_t;

        std::shared_ptr<IRenderModule>  mpRenderModule;
        std::shared_ptr<ISurface>       mpSurface;
        FragmentContainer_t     mpFragments;

        bool insert( SurfaceRect& r );
        bool isValidLocation( const SurfaceRect& r ) const;
    };

    typedef std::shared_ptr< Page > PageSharedPtr;


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
        explicit PageFragment( const ::basegfx::B2ISize& rSize ) :
            mpPage(nullptr),
            maRect(rSize),
            mpBuffer(),
            maSourceOffset()
        {
        }

        bool                        isNaked() const { return (mpPage == nullptr); }
        const SurfaceRect&          getRect() const { return maRect; }
        const ::basegfx::B2IPoint&  getPos() const { return maRect.maPos; }
        const ::basegfx::B2ISize&   getSize() const { return maRect.maSize; }
        void                        setColorBuffer( const std::shared_ptr<IColorBuffer>& pColorBuffer ) { mpBuffer=pColorBuffer; }
        void                        setSourceOffset( const ::basegfx::B2IPoint& rOffset ) { maSourceOffset=rOffset; }
        void                        setPage( Page* pPage ) { mpPage=pPage; }

        void free( const FragmentSharedPtr& pFragment )
        {
            if(mpPage)
                mpPage->free(pFragment);

            mpPage=nullptr;
        }

        bool select( bool bRefresh )
        {
            // request was made to select this fragment,
            // but this fragment has not been located on any
            // of the available pages, we need to hurry now.
            if(!mpPage)
                return false;

            std::shared_ptr<ISurface> pSurface(mpPage->getSurface());

            // select this surface before wiping the contents
            // since a specific implementation could trigger
            // a rendering operation here...
            if(!(pSurface->selectTexture()))
                return false;

            // call refresh() if requested, otherwise we're up to date...
            return !bRefresh || refresh();
        }

        bool refresh()
        {
            if(!mpPage)
                return false;

            std::shared_ptr<ISurface> pSurface(mpPage->getSurface());

            return pSurface->update( maRect.maPos,
                                     ::basegfx::B2IRectangle(
                                         maSourceOffset,
                                         maSourceOffset + maRect.maSize ),
                                     *mpBuffer );
        }

    private:
        Page*                 mpPage;
        SurfaceRect           maRect;
        std::shared_ptr<IColorBuffer> mpBuffer;
        ::basegfx::B2IPoint   maSourceOffset;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

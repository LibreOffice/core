/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: page.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <boost/bind.hpp>
#include "page.hxx"

namespace canvas
{
    Page::Page( const IRenderModuleSharedPtr &rRenderModule ) :
        mpRenderModule(rRenderModule),
        mpSurface(rRenderModule->createSurface(::basegfx::B2ISize()))
    {
    }

    void Page::validate()
    {
        if(!(isValid()))
        {
            ::std::for_each( mpFragments.begin(),
                             mpFragments.end(),
                             ::boost::mem_fn(&PageFragment::refresh));
        }
    }

    bool Page::isValid() const
    {
        return mpSurface && mpSurface->isValid();
    }

    FragmentSharedPtr Page::allocateSpace( const ::basegfx::B2ISize& rSize )
    {
        SurfaceRect rect(rSize);
        if(insert(rect))
        {
            FragmentSharedPtr pFragment(new PageFragment(rect,this));
            mpFragments.push_back(pFragment);
            return pFragment;
        }

        return FragmentSharedPtr();
    }

    bool Page::nakedFragment( const FragmentSharedPtr& pFragment )
    {
        SurfaceRect rect(pFragment->getSize());
        if(insert(rect))
        {
            pFragment->setPage(this);
            mpFragments.push_back(pFragment);
            return true;
        }

        return false;
    }

    void Page::free( const FragmentSharedPtr& pFragment )
    {
        // the fragment passes as argument is no longer
        // dedicated to this page. either it is about to
        // be relocated to some other page or it will
        // currently be deleted. in either case, simply
        // remove the reference from our internal storage.
        FragmentContainer_t::iterator it(
            std::remove(
                mpFragments.begin(),mpFragments.end(),pFragment));
        mpFragments.erase(it,mpFragments.end());
    }

    bool Page::insert( SurfaceRect& r )
    {
        const FragmentContainer_t::const_iterator aEnd(mpFragments.end());
        FragmentContainer_t::const_iterator       it(mpFragments.begin());
        while(it != aEnd)
        {
            const SurfaceRect &rect = (*it)->getRect();
            const sal_Int32 x = rect.maPos.getX();
            const sal_Int32 y = rect.maPos.getY();
            // to avoid interpolation artifacts from other textures,
            // one pixel gap between them
            const sal_Int32 w = rect.maSize.getX()+1;
            const sal_Int32 h = rect.maSize.getY()+1;

            // probe location to the right
            r.maPos.setX(x+w);
            r.maPos.setY(y);
            if(isValidLocation(r))
                return true;

            // probe location at bottom
            r.maPos.setX(x);
            r.maPos.setY(y+h);
            if(isValidLocation(r))
                return true;

            ++it;
        }

        r.maPos.setX(0);
        r.maPos.setY(0);

        return isValidLocation(r);
    }

    bool Page::isValidLocation( const SurfaceRect& r ) const
    {
        // the rectangle passed as argument has a valid
        // location if and only if there's no intersection
        // with existing areas.
        SurfaceRect aBoundary(mpRenderModule->getPageSize()-basegfx::B2IVector(1,1));
        if( !r.inside(aBoundary) )
            return false;

        const FragmentContainer_t::const_iterator aEnd(mpFragments.end());
        FragmentContainer_t::const_iterator       it(mpFragments.begin());
        while(it != aEnd)
        {
            if(r.intersection((*it)->getRect()))
                return false;

            ++it;
        }

        return true;
    }
}

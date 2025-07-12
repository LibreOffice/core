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

#include <sal/config.h>
#include <basegfx/vector/b2ivector.hxx>
#include "page.hxx"

namespace vcl_canvas
{
    Page::Page( const std::shared_ptr<IRenderModule> &rRenderModule ) :
        mpRenderModule(rRenderModule),
        mpSurface(rRenderModule->createSurface(::basegfx::B2IVector()))
    {
    }

    void Page::validate()
    {
        if(!(isValid()))
        {
            for( const auto& rFragmentPtr : mpFragments )
                rFragmentPtr->refresh();
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
            FragmentSharedPtr pFragment = std::make_shared<PageFragment>(rect,this);
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
        std::erase(mpFragments, pFragment);
    }

    bool Page::insert( SurfaceRect& r )
    {
        for( const auto& pFragment : mpFragments )
        {
            const SurfaceRect &rect = pFragment->getRect();
            const sal_Int32 x = rect.maPos.getX();
            const sal_Int32 y = rect.maPos.getY();
            // to avoid interpolation artifacts from other textures,
            // one pixel gap between them
            const sal_Int32 w = rect.maSize.getWidth() + 1;
            const sal_Int32 h = rect.maSize.getHeight() + 1;

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
        basegfx::B2ISize aSize(mpRenderModule->getPageSize().getX(), mpRenderModule->getPageSize().getY());
        SurfaceRect aBoundary(aSize);
        if( !r.inside(aBoundary) )
            return false;

        for( const auto& pFragment : mpFragments )
        {
            if( r.intersection( pFragment->getRect() ) )
                return false;
        }

        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

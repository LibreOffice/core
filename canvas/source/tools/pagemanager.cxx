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

#include "pagemanager.hxx"

namespace canvas
{
    FragmentSharedPtr PageManager::allocateSpace( const ::basegfx::B2ISize& rSize )
    {
        // we are asked to find a location for the requested size.
        // first we try to satisfy the request from the
        // remaining space in the existing pages.
        for( const auto& pPage : maPages )
        {
            FragmentSharedPtr pFragment( pPage->allocateSpace(rSize) );
            if(pFragment)
            {
                // the page created a new fragment, since we maybe want
                // to consolidate sparse pages we keep a reference to
                // the fragment.
                maFragments.push_back(pFragment);
                return pFragment;
            }
        }

        // otherwise try to create a new page and allocate space there...
        PageSharedPtr pPage(new Page(mpRenderModule));
        if(pPage->isValid())
        {
            maPages.push_back(pPage);
            FragmentSharedPtr pFragment(pPage->allocateSpace(rSize));
            if (pFragment)
                maFragments.push_back(pFragment);
            return pFragment;
        }

        // the rendermodule failed to create a new page [maybe out
        // of videomemory], and all other pages could not take
        // the new request. we decide to create a 'naked' fragment
        // which will receive its location later.
        FragmentSharedPtr pFragment(new PageFragment(rSize));
        maFragments.push_back(pFragment);
        return pFragment;
    }

    void PageManager::free( const FragmentSharedPtr& pFragment )
    {
        // erase the reference to the given fragment from our
        // internal container.
        FragmentContainer_t::iterator it(
            std::remove(
                maFragments.begin(),maFragments.end(),pFragment));
        maFragments.erase(it,maFragments.end());

        // let the fragment itself know about it...
        // we need to pass 'this' as argument since the fragment
        // needs to pass this to the page and can't create
        // shared_ptr from itself...
        pFragment->free(pFragment);
    }

    void PageManager::nakedFragment( const FragmentSharedPtr& pFragment )
    {
        if(maPages.empty())
            return;

        // okay, one last chance is left, we try all available
        // pages again. maybe some other fragment was deleted
        // and we can exploit the space.
        while( !( relocate( pFragment ) ) )
        {
            // no way, we need to free up some space...
            // TODO(F1): this is a heuristic, could
            // be designed as a policy.
            auto       aEnd( maFragments.cend() );
            auto       aCurrMax( aEnd );
            sal_uInt32 nCurrMaxArea = 0;
            for( auto aCurr = maFragments.begin(); aCurr != aEnd; ++aCurr )
            {
                if( *aCurr && !( ( *aCurr )->isNaked() ) )
                {
                    const ::basegfx::B2ISize& rSize( ( *aCurr )->getSize() );
                    sal_uInt32                nArea( rSize.getX() * rSize.getY() );

                    if( nCurrMaxArea < nArea )
                    {
                        aCurrMax = aCurr;
                        nCurrMaxArea = nArea;
                    }
                }
            }
            // this does not erase the candidate,
            // but makes it 'naked'...
            if( aCurrMax != aEnd )
                ( *aCurrMax )->free( *aCurrMax );
            else
                break;
        }
    }

    bool PageManager::relocate( const FragmentSharedPtr& pFragment )
    {
        // the fragment passed as argument is assumed to
        // be naked, that is it is not located on any page.
        // we try all available pages again, maybe some
        // other fragment was deleted and we can exploit the space.
        for( const auto& pPage : maPages )
        {
            // if the page at hand takes the fragment, we immediately
            // call select() to pull the information from the associated
            // image to the hardware surface.
            if( pPage->nakedFragment( pFragment ) )
            {
                // dirty, since newly allocated.
                pFragment->select(true);
                return true;
            }
        }
        return false;
    }

    void PageManager::validatePages()
    {
        for( const auto& rPagePtr : maPages )
            rPagePtr->validate();
    }

    ::basegfx::B2ISize PageManager::getPageSize() const
    {
        return mpRenderModule->getPageSize();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

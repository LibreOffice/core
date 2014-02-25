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


#include <boost/mem_fn.hpp>
#include "pagemanager.hxx"

namespace canvas
{


    // PageManager



    // PageManager::allocateSpace


    FragmentSharedPtr PageManager::allocateSpace( const ::basegfx::B2ISize& rSize )
    {
        // we are asked to find a location for the requested size.
        // first we try to satisfy the request from the
        // remaining space in the existing pages.
        const PageContainer_t::iterator aEnd(maPages.end());
        PageContainer_t::iterator       it(maPages.begin());
        while(it != aEnd)
        {
            FragmentSharedPtr pFragment((*it)->allocateSpace(rSize));
            if(pFragment)
            {
                // the page created a new fragment, since we maybe want
                // to consolidate sparse pages we keep a reference to
                // the fragment.
                maFragments.push_back(pFragment);
                return pFragment;
            }

            ++it;
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


    // PageManager::free


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


    // PageManager::nakedFragment


    void PageManager::nakedFragment( const FragmentSharedPtr& pFragment )
    {
        if(maPages.empty())
            return;

        // okay, one last chance is left, we try all available
        // pages again. maybe some other fragment was deleted
        // and we can exploit the space.
        while(!(relocate(pFragment)))
        {
            // no way, we need to free up some space...
            // TODO(F1): this is a heuristic, could
            // be designed as a policy.
            const FragmentContainer_t::const_iterator aEnd(maFragments.end());
            FragmentContainer_t::const_iterator       candidate(maFragments.begin());
            while(candidate != aEnd)
            {
                if(*candidate && !((*candidate)->isNaked()))
                    break;
                ++candidate;
            }

            if (candidate != aEnd)
            {
                const ::basegfx::B2ISize& rSize((*candidate)->getSize());
                sal_uInt32                nMaxArea(rSize.getX()*rSize.getY());

                FragmentContainer_t::const_iterator it(candidate);
                while(it != aEnd)
                {
                    if (*it && !((*it)->isNaked()))
                    {
                        const ::basegfx::B2ISize& rCandidateSize((*it)->getSize());
                        const sal_uInt32 nArea(rCandidateSize.getX()*rCandidateSize.getY());
                        if(nArea > nMaxArea)
                        {
                            candidate=it;
                            nMaxArea=nArea;
                        }
                    }

                    ++it;
                }

                // this does not erase the candidate,
                // but makes it 'naked'...
                (*candidate)->free(*candidate);
            }
            else
                break;
        }
    }


    // PageManager::relocate


    bool PageManager::relocate( const FragmentSharedPtr& pFragment )
    {
        // the fragment passed as argument is assumed to
        // be naked, that is it is not located on any page.
        // we try all available pages again, maybe some
        // other fragment was deleted and we can exploit the space.
        const PageContainer_t::iterator aEnd(maPages.end());
        PageContainer_t::iterator       it(maPages.begin());
        while(it != aEnd)
        {
            // if the page at hand takes the fragment, we immediatelly
            // call select() to pull the information from the associated
            // image to the hardware surface.
            if((*it)->nakedFragment(pFragment))
            {
                // dirty, since newly allocated.
                pFragment->select(true);
                return true;
            }

            ++it;
        }

        return false;
    }


    // PageManager::validatePages


    void PageManager::validatePages()
    {
        ::std::for_each( maPages.begin(),
                         maPages.end(),
                         ::boost::mem_fn(&Page::validate));
    }


    // PageManager::getPageSize


    ::basegfx::B2ISize PageManager::getPageSize() const
    {
        return mpRenderModule->getPageSize();
    }


    // PageManager::getRenderModule


    canvas::IRenderModuleSharedPtr PageManager::getRenderModule() const
    {
        return mpRenderModule;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

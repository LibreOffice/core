/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <boost/mem_fn.hpp>
#include "pagemanager.hxx"

namespace canvas
{

    
    
    

    
    
    

    FragmentSharedPtr PageManager::allocateSpace( const ::basegfx::B2ISize& rSize )
    {
        
        
        
        const PageContainer_t::iterator aEnd(maPages.end());
        PageContainer_t::iterator       it(maPages.begin());
        while(it != aEnd)
        {
            FragmentSharedPtr pFragment((*it)->allocateSpace(rSize));
            if(pFragment)
            {
                
                
                
                maFragments.push_back(pFragment);
                return pFragment;
            }

            ++it;
        }

        
        PageSharedPtr pPage(new Page(mpRenderModule));
        if(pPage->isValid())
        {
            maPages.push_back(pPage);
            FragmentSharedPtr pFragment(pPage->allocateSpace(rSize));
            if (pFragment)
                maFragments.push_back(pFragment);
            return pFragment;
        }

        
        
        
        
        FragmentSharedPtr pFragment(new PageFragment(rSize));
        maFragments.push_back(pFragment);
        return pFragment;
    }

    
    
    

    void PageManager::free( const FragmentSharedPtr& pFragment )
    {
        
        
        FragmentContainer_t::iterator it(
            std::remove(
                maFragments.begin(),maFragments.end(),pFragment));
        maFragments.erase(it,maFragments.end());

        
        
        
        
        pFragment->free(pFragment);
    }

    
    
    

    void PageManager::nakedFragment( const FragmentSharedPtr& pFragment )
    {
        if(maPages.empty())
            return;

        
        
        
        while(!(relocate(pFragment)))
        {
            
            
            
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

                
                
                (*candidate)->free(*candidate);
            }
            else
                break;
        }
    }

    
    
    

    bool PageManager::relocate( const FragmentSharedPtr& pFragment )
    {
        
        
        
        
        const PageContainer_t::iterator aEnd(maPages.end());
        PageContainer_t::iterator       it(maPages.begin());
        while(it != aEnd)
        {
            
            
            
            if((*it)->nakedFragment(pFragment))
            {
                
                pFragment->select(true);
                return true;
            }

            ++it;
        }

        return false;
    }

    
    
    

    void PageManager::validatePages()
    {
        ::std::for_each( maPages.begin(),
                         maPages.end(),
                         ::boost::mem_fn(&Page::validate));
    }

    
    
    

    ::basegfx::B2ISize PageManager::getPageSize() const
    {
        return mpRenderModule->getPageSize();
    }

    
    
    

    canvas::IRenderModuleSharedPtr PageManager::getRenderModule() const
    {
        return mpRenderModule;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

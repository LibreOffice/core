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


#include <unoviewcontainer.hxx>

#include <osl/diagnose.h>

#include <algorithm>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        UnoViewContainer::UnoViewContainer() :
            maViews()
        {
        }

        bool UnoViewContainer::addView( const std::shared_ptr< UnoView >& rView )
        {
            // check whether same view is already added

            uno::Reference< presentation::XSlideShowView > rTmpView = rView->getUnoView();
            // already added?
            if( ::std::any_of( maViews.begin(),
                               maViews.end(),
                               [&rTmpView]( const std::shared_ptr< UnoView >& pView )
                               { return rTmpView == pView->getUnoView(); } ) )
            {
                // yes, nothing to do
                return false;
            }

            // add locally
            maViews.push_back( rView );

            return true;
        }

        std::shared_ptr< UnoView > UnoViewContainer::removeView( const uno::Reference< presentation::XSlideShowView >& xView )
        {
            // check whether same view is already added
            const std::vector< std::shared_ptr< UnoView > >::iterator aEnd( maViews.end() );
            std::vector< std::shared_ptr< UnoView > >::iterator aIter;

            // added in the first place?
            if( (aIter=::std::find_if( maViews.begin(),
                                       aEnd,
                                       [&xView]( const std::shared_ptr< UnoView >& pView )
                                       { return xView == pView->getUnoView(); } )) == aEnd )
            {
                // nope, nothing to do
                return std::shared_ptr< UnoView >();
            }

            OSL_ENSURE(
                ::std::count_if(
                    maViews.begin(),
                    aEnd,
                    [&xView]( const std::shared_ptr< UnoView >& pView )
                    { return xView == pView->getUnoView(); } ) == 1,
                "UnoViewContainer::removeView(): View was added multiple times" );

            std::shared_ptr< UnoView > pView( *aIter );

            // actually erase from container
            maViews.erase( aIter );

            return pView;
        }

        void UnoViewContainer::dispose()
        {
            for( const auto& pView : maViews )
                pView->_dispose();
            maViews.clear();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

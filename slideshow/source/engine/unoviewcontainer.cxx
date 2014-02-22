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


#include <canvas/debug.hxx>
#include <unoviewcontainer.hxx>

#include <boost/bind.hpp>

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

        bool UnoViewContainer::addView( const UnoViewSharedPtr& rView )
        {
            
            const UnoViewVector::iterator aEnd( maViews.end() );

            
            if( ::std::find_if( maViews.begin(),
                                aEnd,
                                ::boost::bind(
                                    ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                                    ::boost::cref( rView->getUnoView() ),
                                    ::boost::bind(
                                        &UnoView::getUnoView,
                                        _1 ) ) ) != aEnd )
            {
                
                return false;
            }

            
            maViews.push_back( rView );

            return true;
        }

        UnoViewSharedPtr UnoViewContainer::removeView( const uno::Reference< presentation::XSlideShowView >& xView )
        {
            
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;

            
            if( (aIter=::std::find_if( maViews.begin(),
                                       aEnd,
                                       ::boost::bind(
                                           ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                                           ::boost::cref( xView ),
                                           ::boost::bind(
                                               &UnoView::getUnoView,
                                               _1 ) ) ) ) == aEnd )
            {
                
                return UnoViewSharedPtr();
            }

            OSL_ENSURE(
                ::std::count_if(
                    maViews.begin(),
                    aEnd,
                    ::boost::bind(
                        ::std::equal_to< uno::Reference< presentation::XSlideShowView > >(),
                        ::boost::cref( xView ),
                        ::boost::bind(
                            &UnoView::getUnoView,
                            _1 ))) == 1,
                "UnoViewContainer::removeView(): View was added multiple times" );

            UnoViewSharedPtr pView( *aIter );

            
            maViews.erase( aIter );

            return pView;
        }

        void UnoViewContainer::dispose()
        {
            ::std::for_each( maViews.begin(),
                             maViews.end(),
                             ::boost::mem_fn(&UnoView::_dispose) );
            maViews.clear();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

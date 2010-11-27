/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "timer.hxx"

#include <vector>
#include <list>
#include <vcl/timer.hxx>
#include <com/sun/star/awt/XLayoutContainer.hpp>

namespace layoutimpl
{
using namespace ::com::sun::star;

class AllocateTimer : public Timer
{
    typedef std::list< uno::Reference< awt::XLayoutContainer > > ContainerList;
    ContainerList mxContainers;
    uno::Reference< awt::XLayoutContainer > mxLastAdded;

public:
    AllocateTimer()
    {
        // timer set to 0 -- just process it as soon as it gets idle
        SetTimeout( 0 );
    }

    static inline bool isParentOf( uno::Reference< awt::XLayoutContainer > xParent,
                                   uno::Reference< awt::XLayoutContainer > xWidget )
    {
        while ( xWidget.is() )
        {
            if ( xWidget == xParent )
                return true;
            xWidget = uno::Reference< awt::XLayoutContainer >( xWidget->getParent(), uno::UNO_QUERY );
        }
        return false;
    }

    static inline void eraseChildren( ContainerList::iterator &it, ContainerList &list )
    {
        ContainerList::iterator jt = list.begin();
        while ( jt != list.end() )
        {
            if ( it != jt && isParentOf( *it, *jt ) )
                jt = list.erase( jt );
            else
                ++jt;
        }
    }

    static inline bool isContainerDamaged( uno::Reference< awt::XLayoutContainer > xContainer )
    {
        uno::Reference< awt::XLayoutConstrains > xConstrains( xContainer, uno::UNO_QUERY );
        awt::Size lastReq( xContainer->getRequestedSize() );
        awt::Size curReq( xConstrains->getMinimumSize() );
        return lastReq.Width != curReq.Width || lastReq.Height != curReq.Height;
    }

    void add( const uno::Reference< awt::XLayoutContainer > &xContainer )
    {
        // small optimization
        if ( mxLastAdded == xContainer )
            return;
        mxLastAdded = xContainer;

        mxContainers.push_back( xContainer );
    }

    virtual void Timeout()
    {
        mxLastAdded = uno::Reference< awt::XLayoutContainer >();

        // 1. remove duplications and children
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); ++it )
            eraseChildren( it, mxContainers );

        // 2. check damage extent
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); ++it )
        {
            uno::Reference< awt::XLayoutContainer > xContainer = *it;
            while ( xContainer->getParent().is() && isContainerDamaged( xContainer ) )
            {
                xContainer = uno::Reference< awt::XLayoutContainer >(
                    xContainer->getParent(), uno::UNO_QUERY );
            }

            if ( *it != xContainer )
            {
                // 2.2 replace it with parent
                *it = xContainer;

                // 2.3 remove children of new parent
                eraseChildren( it, mxContainers );
            }
        }

        // 3. force re-calculations
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); ++it )
            (*it)->allocateArea( (*it)->getAllocatedArea() );
    }
};

static void AddResizeTimeout( const uno::Reference< awt::XLayoutContainer > &xCont )
{
    static AllocateTimer timer;
    timer.add( xCont );
    timer.Start();
}

LayoutUnit::LayoutUnit() : LayoutUnit_Base()
{
}

void SAL_CALL LayoutUnit::queueResize( const uno::Reference< awt::XLayoutContainer > &xContainer )
    throw( uno::RuntimeException )
{
    AddResizeTimeout( xContainer );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

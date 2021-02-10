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

#include <helper/ocomponentaccess.hxx>
#include <helper/ocomponentenumeration.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

namespace framework{

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::osl;

//  constructor

OComponentAccess::OComponentAccess( const css::uno::Reference< XDesktop >& xOwner )
        :   m_xOwner        ( xOwner                        )
{
    // Safe impossible cases
    SAL_WARN_IF( !xOwner.is(), "fwk", "OComponentAccess::OComponentAccess(): Invalid parameter detected!" );
}

//  destructor

OComponentAccess::~OComponentAccess()
{
}

//  XEnumerationAccess
css::uno::Reference< XEnumeration > SAL_CALL OComponentAccess::createEnumeration()
{
    SolarMutexGuard g;

    // Set default return value, if method failed.
    // If no desktop exist and there is no task container - return an empty enumeration!
    css::uno::Reference< XEnumeration > xReturn;

    // Try to "lock" the desktop for access to task container.
    css::uno::Reference< XInterface > xLock = m_xOwner.get();
    if ( xLock.is() )
    {
        // Desktop exist => pointer to task container must be valid.
        // Initialize a new enumeration ... if some tasks and his components exist!
        // (OTasksEnumeration will make an assert, if we initialize the new instance without valid values!)

        std::vector< css::uno::Reference< XComponent > > seqComponents;
        impl_collectAllChildComponents( css::uno::Reference< XFramesSupplier >( xLock, UNO_QUERY ), seqComponents );
        xReturn = new OComponentEnumeration( seqComponents );
    }

    // Return result of this operation.
    return xReturn;
}

//  XElementAccess
Type SAL_CALL OComponentAccess::getElementType()
{
    // Elements in list an enumeration are components!
    // Return the uno-type of XComponent.
    return cppu::UnoType<XComponent>::get();
}

//  XElementAccess
sal_Bool SAL_CALL OComponentAccess::hasElements()
{
    SolarMutexGuard g;

    // Set default return value, if method failed.
    bool bReturn = false;

    // Try to "lock" the desktop for access to task container.
    css::uno::Reference< XFramesSupplier > xLock( m_xOwner.get(), UNO_QUERY );
    if ( xLock.is() )
    {
        // Ask container of owner for existing elements.
        bReturn = xLock->getFrames()->hasElements();
    }

    // Return result of this operation.
    return bReturn;
}


void OComponentAccess::impl_collectAllChildComponents(  const css::uno::Reference< XFramesSupplier >&         xNode           ,
                                                               std::vector< css::uno::Reference< XComponent > >& seqComponents   )
{
    // If valid node was given ...
    if( !xNode.is() )
        return;

    // ... continue collection at these.

    // Get the container of current node, collect the components of existing child frames
    // and go down to next level in tree (recursive!).

    const css::uno::Reference< XFrames >                xContainer  = xNode->getFrames();
    const Sequence< css::uno::Reference< XFrame > > seqFrames   = xContainer->queryFrames( FrameSearchFlag::CHILDREN );

    const sal_Int32 nFrameCount = seqFrames.getLength();
    for( sal_Int32 nFrame=0; nFrame<nFrameCount; ++nFrame )
    {
        css::uno::Reference< XComponent > xComponent = impl_getFrameComponent( seqFrames[nFrame] );
        if( xComponent.is() )
        {
            seqComponents.push_back( xComponent );
        }
    }
    // ... otherwise break a recursive path and go back at current stack!
}

css::uno::Reference< XComponent > OComponentAccess::impl_getFrameComponent( const css::uno::Reference< XFrame >& xFrame ) const
{
    // Set default return value, if method failed.
    css::uno::Reference< XComponent > xComponent;
    // Does no controller exists?
    css::uno::Reference< XController > xController = xFrame->getController();
    if ( !xController.is() )
    {
        // Controller not exist - use the VCL-component.
        xComponent = xFrame->getComponentWindow();
    }
    else
    {
        // Does no model exists?
        css::uno::Reference< XModel > xModel = xController->getModel();
        if ( xModel.is() )
        {
            // Model exist - use the model as component.
            xComponent = xModel;
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent = xController;
        }
    }

    return xComponent;
}


}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

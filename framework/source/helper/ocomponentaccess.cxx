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

#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
OComponentAccess::OComponentAccess( const css::uno::Reference< XDesktop >& xOwner )
        //  Init baseclasses first
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        // Init member
        ,   m_xOwner        ( xOwner                        )
{
    // Safe impossible cases
    LOG_ASSERT( impldbg_checkParameter_OComponentAccessCtor( xOwner ), "OComponentAccess::OComponentAccess()\nInvalid parameter detected!\n" )
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
OComponentAccess::~OComponentAccess()
{
}

//*****************************************************************************************************************
//  XEnumerationAccess
//*****************************************************************************************************************
css::uno::Reference< XEnumeration > SAL_CALL OComponentAccess::createEnumeration() throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Set default return value, if method failed.
    // If no desktop exist and there is no task container - return an empty enumeration!
    css::uno::Reference< XEnumeration > xReturn = css::uno::Reference< XEnumeration >();

    // Try to "lock" the desktop for access to task container.
    css::uno::Reference< XInterface > xLock = m_xOwner.get();
    if ( xLock.is() == sal_True )
    {
        // Desktop exist => pointer to task container must be valid.
        // Initialize a new enumeration ... if some tasks and his components exist!
        // (OTasksEnumeration will make an assert, if we initialize the new instance without valid values!)

        Sequence< css::uno::Reference< XComponent > > seqComponents;
        impl_collectAllChildComponents( css::uno::Reference< XFramesSupplier >( xLock, UNO_QUERY ), seqComponents );
        OComponentEnumeration* pEnumeration = new OComponentEnumeration( seqComponents );
        xReturn = css::uno::Reference< XEnumeration >( (OWeakObject*)pEnumeration, UNO_QUERY );
    }

    // Return result of this operation.
    return xReturn;
}

//*****************************************************************************************************************
//  XElementAccess
//*****************************************************************************************************************
Type SAL_CALL OComponentAccess::getElementType() throw( RuntimeException )
{
    // Elements in list an enumeration are components!
    // Return the uno-type of XComponent.
    return ::getCppuType((const css::uno::Reference< XComponent >*)NULL);
}

//*****************************************************************************************************************
//  XElementAccess
//*****************************************************************************************************************
sal_Bool SAL_CALL OComponentAccess::hasElements() throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Set default return value, if method failed.
    sal_Bool bReturn = sal_False;

    // Try to "lock" the desktop for access to task container.
    css::uno::Reference< XFramesSupplier > xLock( m_xOwner.get(), UNO_QUERY );
    if ( xLock.is() == sal_True )
    {
        // Ask container of owner for existing elements.
        bReturn = xLock->getFrames()->hasElements();
    }

    // Return result of this operation.
    return bReturn;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void OComponentAccess::impl_collectAllChildComponents(  const   css::uno::Reference< XFramesSupplier >&         xNode           ,
                                                                 Sequence< css::uno::Reference< XComponent > >& seqComponents   )
{
    // If valid node was given ...
    if( xNode.is() == sal_True )
    {
        // ... continue collection at these.

        // Get the container of current node, collect the components of existing child frames
        // and go down to next level in tree (recursive!).

        sal_Int32 nComponentCount = seqComponents.getLength();

        const css::uno::Reference< XFrames >                xContainer  = xNode->getFrames();
        const Sequence< css::uno::Reference< XFrame > > seqFrames   = xContainer->queryFrames( FrameSearchFlag::CHILDREN );

        const sal_Int32 nFrameCount = seqFrames.getLength();
        for( sal_Int32 nFrame=0; nFrame<nFrameCount; ++nFrame )
        {
            css::uno::Reference< XComponent > xComponent = impl_getFrameComponent( seqFrames[nFrame] );
            if( xComponent.is() == sal_True )
            {
                nComponentCount++;
                seqComponents.realloc( nComponentCount );
                seqComponents[nComponentCount-1] = xComponent;
            }
        }
    }
    // ... otherwise break a recursive path and go back at current stack!
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
css::uno::Reference< XComponent > OComponentAccess::impl_getFrameComponent( const css::uno::Reference< XFrame >& xFrame ) const
{
    // Set default return value, if method failed.
    css::uno::Reference< XComponent > xComponent = css::uno::Reference< XComponent >();
    // Does no controller exists?
    css::uno::Reference< XController > xController = xFrame->getController();
    if ( xController.is() == sal_False )
    {
        // Controller not exist - use the VCL-component.
        xComponent = css::uno::Reference< XComponent >( xFrame->getComponentWindow(), UNO_QUERY );
    }
    else
    {
        // Does no model exists?
        css::uno::Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
        if ( xModel.is() == sal_True )
        {
            // Model exist - use the model as component.
            xComponent = css::uno::Reference< XComponent >( xModel, UNO_QUERY );
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent = css::uno::Reference< XComponent >( xController, UNO_QUERY );
        }
    }

    return xComponent;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
sal_Bool OComponentAccess::impldbg_checkParameter_OComponentAccessCtor( const   css::uno::Reference< XDesktop >&      xOwner  )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xOwner       ==  NULL        )   ||
            ( xOwner.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

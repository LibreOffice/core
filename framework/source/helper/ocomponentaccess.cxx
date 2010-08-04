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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <helper/ocomponentaccess.hxx>
#include <helper/ocomponentenumeration.hxx>

#ifndef _FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/FrameSearchFlag.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
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

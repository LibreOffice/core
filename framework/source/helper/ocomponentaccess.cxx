/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ocomponentaccess.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:58:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
#include <helper/ocomponentaccess.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OCOMPONENTENUMERATION_HXX_
#include <helper/ocomponentenumeration.hxx>
#endif

#ifndef _FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
OComponentAccess::OComponentAccess( const Reference< XDesktop >& xOwner )
        //  Init baseclasses first
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        ,   OWeakObject     (                               )
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
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3(    OComponentAccess                        ,
                        OWeakObject                             ,
                        DIRECT_INTERFACE(XTypeProvider      )   ,
                        DIRECT_INTERFACE(XEnumerationAccess )   ,
                        DIRECT_INTERFACE(XElementAccess     )   )

DEFINE_XTYPEPROVIDER_3( OComponentAccess                        ,
                        XTypeProvider                           ,
                        XEnumerationAccess                      ,
                        XElementAccess                          )

//*****************************************************************************************************************
//  XEnumerationAccess
//*****************************************************************************************************************
Reference< XEnumeration > SAL_CALL OComponentAccess::createEnumeration() throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Set default return value, if method failed.
    // If no desktop exist and there is no task container - return an empty enumeration!
    Reference< XEnumeration > xReturn = Reference< XEnumeration >();

    // Try to "lock" the desktop for access to task container.
    Reference< XInterface > xLock = m_xOwner.get();
    if ( xLock.is() == sal_True )
    {
        // Desktop exist => pointer to task container must be valid.
        // Initialize a new enumeration ... if some tasks and his components exist!
        // (OTasksEnumeration will make an assert, if we initialize the new instance without valid values!)

        Sequence< Reference< XComponent > > seqComponents;
        impl_collectAllChildComponents( Reference< XFramesSupplier >( xLock, UNO_QUERY ), seqComponents );
        OComponentEnumeration* pEnumeration = new OComponentEnumeration( seqComponents );
        xReturn = Reference< XEnumeration >( (OWeakObject*)pEnumeration, UNO_QUERY );
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
    return ::getCppuType((const Reference< XComponent >*)NULL);
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
    Reference< XFramesSupplier > xLock( m_xOwner.get(), UNO_QUERY );
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
void OComponentAccess::impl_collectAllChildComponents(  const   Reference< XFramesSupplier >&           xNode           ,
                                                                 Sequence< Reference< XComponent > >&   seqComponents   )
{
    // If valid node was given ...
    if( xNode.is() == sal_True )
    {
        // ... continue collection at these.

        // Get the container of current node, collect the components of existing child frames
        // and go down to next level in tree (recursive!).

        sal_Int32 nComponentCount = seqComponents.getLength();

        const Reference< XFrames >              xContainer  = xNode->getFrames();
        const Sequence< Reference< XFrame > >   seqFrames   = xContainer->queryFrames( FrameSearchFlag::CHILDREN );

        const sal_Int32 nFrameCount = seqFrames.getLength();
        for( sal_Int32 nFrame=0; nFrame<nFrameCount; ++nFrame )
        {
            Reference< XComponent > xComponent = impl_getFrameComponent( seqFrames[nFrame] );
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
Reference< XComponent > OComponentAccess::impl_getFrameComponent( const Reference< XFrame >& xFrame ) const
{
    // Set default return value, if method failed.
    Reference< XComponent > xComponent = Reference< XComponent >();
    // Does no controller exists?
    Reference< XController > xController = xFrame->getController();
    if ( xController.is() == sal_False )
    {
        // Controller not exist - use the VCL-component.
        xComponent = Reference< XComponent >( xFrame->getComponentWindow(), UNO_QUERY );
    }
    else
    {
        // Does no model exists?
        Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
        if ( xModel.is() == sal_True )
        {
            // Model exist - use the model as component.
            xComponent = Reference< XComponent >( xModel, UNO_QUERY );
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent = Reference< XComponent >( xController, UNO_QUERY );
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
sal_Bool OComponentAccess::impldbg_checkParameter_OComponentAccessCtor( const   Reference< XDesktop >&      xOwner  )
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

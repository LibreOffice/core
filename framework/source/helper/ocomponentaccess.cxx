/*************************************************************************
 *
 *  $RCSfile: ocomponentaccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-10-06 11:34:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
#include <helper/ocomponentaccess.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OCOMPONENTENUMERATION_HXX_
#include <helper/ocomponentenumeration.hxx>
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
OComponentAccess::OComponentAccess( const   Reference< XDesktop >&      xOwner  ,
                                            Mutex&                      aMutex  )
        //  Init baseclasses first
        :   OWeakObject     (           )
        // Init member
        ,   m_aMutex        ( aMutex    )
        ,   m_xOwner        ( xOwner    )
{
    // Safe impossible cases
    LOG_ASSERT( impldbg_checkParameter_OComponentAccessCtor( xOwner, aMutex ), "OComponentAccess::OComponentAccess()\nInvalid parameter detected!\n" )
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
    LOCK_MUTEX( aGuard, m_aMutex, "OComponentAccess::createEnumeration()" )

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
    LOCK_MUTEX( aGuard, m_aMutex, "OComponentAccess::hasElements()" )

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

            impl_collectAllChildComponents( Reference< XFramesSupplier>( seqFrames[nFrame], UNO_QUERY ), seqComponents );
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
sal_Bool OComponentAccess::impldbg_checkParameter_OComponentAccessCtor( const   Reference< XDesktop >&      xOwner  ,
                                                                                Mutex&                      aMutex  ) const
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xOwner       ==  NULL        )   ||
            ( &aMutex       ==  NULL        )   ||
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

/*************************************************************************
 *
 *  $RCSfile: oframes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-09-26 13:01:15 $
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

#ifndef __FRAMEWORK_HELPER_OFRAMES_HXX_
#include <helper/oframes.hxx>
#endif

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

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
using namespace ::std                           ;

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
OFrames::OFrames(   const   Reference< XMultiServiceFactory >&  xFactory        ,
                            Mutex&                              aMutex          ,
                    const   Reference< XFrame >&                xOwner          ,
                            FrameContainer*                     pFrameContainer )
        //  Init baseclasses first
        :   OWeakObject                 (                   )
        // Init member
        ,   m_xFactory                  ( xFactory          )
        ,   m_aMutex                    ( aMutex            )
        ,   m_xOwner                    ( xOwner            )
        ,   m_pFrameContainer           ( pFrameContainer   )
        ,   m_bRecursiveSearchProtection( PROTECTION_OFF    )
{
    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_OFramesCtor( xFactory, aMutex, xOwner, pFrameContainer ), "OFrames::OFrames()\nInvalid parameter detected!\n" )
}

//*****************************************************************************************************************
//  (proteced!) destructor
//*****************************************************************************************************************
OFrames::~OFrames()
{
    // Reset instance, free memory ....
    impl_resetObject();
}

//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   OFrames                         ,
                            OWeakObject                     ,
                            DIRECT_INTERFACE(XFrames        ),
                            DIRECT_INTERFACE(XIndexAccess   ),
                            DIRECT_INTERFACE(XElementAccess )
                        )

//*****************************************************************************************************************
//  XFrames
//*****************************************************************************************************************
void SAL_CALL OFrames::append( const Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::append()" )

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_append( xFrame ), "OFrames::append()\nInvalid parameter detected!\n" )

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Append frame to the end of the container ...
        m_pFrameContainer->append( xFrame );
        // Set owner of this instance as parent of the new frame in container!
        xFrame->setCreator( xOwner );
    }
    // Else; Do nothing! Ouer owner is dead.
    LOG_ASSERT( !(xOwner.is()==sal_False), "OFrames::append()\nOuer owner is dead - you can't append any frames ...!\n" )
}

//*****************************************************************************************************************
//  XFrames
//*****************************************************************************************************************
void SAL_CALL OFrames::remove( const Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::remove()" )

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_remove( xFrame ), "OFrames::remove()\nInvalid parameter detected!\n" )

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Search frame and remove it from container ...
        m_pFrameContainer->remove( xFrame );
        // Don't reset owner-property of removed frame!
        // This must do the caller of this method himself.
        // See documentation of interface XFrames for further informations.
    }
    // Else; Do nothing! Ouer owner is dead.
    LOG_ASSERT( !(xOwner.is()==sal_False), "OFrames::remove()\nOuer owner is dead - you can't remove any frames ...!\n" )
}

//*****************************************************************************************************************
//  XFrames
//*****************************************************************************************************************
Sequence< Reference< XFrame > > SAL_CALL OFrames::queryFrames( sal_Int32 nSearchFlags ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::queryFrames()" )

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_queryFrames( nSearchFlags ), "OFrames::queryFrames()\nInvalid parameter detected!\n" )

    // Set default return value. (empty sequence)
    Sequence< Reference< XFrame > > seqFrames;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Work only, if search was not started here ...!
        if( m_bRecursiveSearchProtection == PROTECTION_OFF )
        {
            // This class is a helper for services, which must implement XFrames.
            // His parent and childs are MY parent and childs to.
            // All searchflags are supported by this implementation!
            // If some flags should not be supported - don't call me with this flags!!!

            //_____________________________________________________________________________________________________________
            // Search with AUTO-flag is not supported yet!
            // We think about right implementation.
            LOG_ASSERT( !(nSearchFlags & FrameSearchFlag::AUTO), "OFrames::queryFrames()\nSearch with AUTO-flag is not supported yet!\nWe think about right implementation.\n" )
            // If searched for tasks ...
            // Its not supported yet.
            LOG_ASSERT( !(nSearchFlags & FrameSearchFlag::AUTO), "OFrames::queryFrames()\nSearch for tasks not supported yet!\n" )

            //_____________________________________________________________________________________________________________
            // Search for ALL and GLOBAL is superflous!
            // We support all necessary flags, from which these two flags are derived.
            //      ALL     = PARENT + SELF  + CHILDREN + SIBLINGS
            //      GLOBAL  = ALL    + TASKS

            //_____________________________________________________________________________________________________________
            // Add parent to list ... if any exist!
            if( nSearchFlags & FrameSearchFlag::PARENT )
            {
                Reference< XFrame > xParent( xOwner->getCreator(), UNO_QUERY );
                if( xParent.is() == sal_True )
                {
                    Sequence< Reference< XFrame > > seqParent( 1 );
                    seqParent[0] = xParent;
                    impl_appendSequence( seqFrames, seqParent );
                }
            }

            //_____________________________________________________________________________________________________________
            // Add owner to list if SELF is searched.
            if( nSearchFlags & FrameSearchFlag::SELF )
            {
                Sequence< Reference< XFrame > > seqSelf( 1 );
                seqSelf[0] = xOwner;
                impl_appendSequence( seqFrames, seqSelf );
            }

            //_____________________________________________________________________________________________________________
            // Add SIBLINGS to list.
            if( nSearchFlags & FrameSearchFlag::SIBLINGS )
            {
                // Else; start a new search.
                // Protect this instance against recursive calls from parents.
                m_bRecursiveSearchProtection = PROTECTION_ON;
                // Ask parent of my owner for frames and append results to return list.
                Reference< XFramesSupplier > xParent( xOwner->getCreator(), UNO_QUERY );
                // If a parent exist ...
                if ( xParent.is() == sal_True )
                {
                    // ... ask him for right frames.
                    impl_appendSequence( seqFrames, xParent->getFrames()->queryFrames( nSearchFlags ) );
                }
                // We have all searched informations.
                // Reset protection-mode.
                m_bRecursiveSearchProtection = PROTECTION_OFF;
            }

            //_____________________________________________________________________________________________________________
            // If searched for children, step over all elements in container and collect the informations.
            if ( nSearchFlags & FrameSearchFlag::CHILDREN )
            {
                // Don't search for parents, siblings and self at childrens!
                // These things are supported by this instance himself.
                sal_Int32 nChildSearchFlags = FrameSearchFlag::SELF | FrameSearchFlag::CHILDREN;
                // Step over all items of container and ask childrens for frames.
                // But first lock the container! see class FrameContainer for further informations.
                m_pFrameContainer->lock();
                sal_uInt32 nCount = m_pFrameContainer->getCount();
                for ( sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex )
                {
                    // We don't must control this conversion.
                    // We have done this at append()!
                    Reference< XFramesSupplier > xItem( (*m_pFrameContainer)[nIndex], UNO_QUERY );
                    impl_appendSequence( seqFrames, xItem->getFrames()->queryFrames( nChildSearchFlags ) );
                }
                // Don't forget to unlock the container.
                m_pFrameContainer->unlock();
            }
        }
    }
    // Else; Do nothing! Ouer owner is dead.
    LOG_ASSERT( !(xOwner.is()==sal_False), "OFrames::queryFrames()\nOuer owner is dead - you can't query for frames ...!\n" )

    // Resturn result of this operation.
    return seqFrames;
}

//*****************************************************************************************************************
//  XIndexAccess
//*****************************************************************************************************************
sal_Int32 SAL_CALL OFrames::getCount() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::getCount()" )

    // Set default return value.
    sal_Int32 nCount = 0;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Set CURRENT size of container for return.
        nCount = m_pFrameContainer->getCount();
    }

    // Return result.
    return nCount;
}

//*****************************************************************************************************************
//  XIndexAccess
//*****************************************************************************************************************
Any SAL_CALL OFrames::getByIndex( sal_Int32 nIndex ) throw( IndexOutOfBoundsException   ,
                                                            WrappedTargetException      ,
                                                            RuntimeException            )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::getByIndex()" )

    // Set default return value.
    Any aReturnValue;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Get element form container.
        // (If index not valid, FrameContainer return NULL!)
        aReturnValue <<= (*m_pFrameContainer)[nIndex];
    }

    // Return result of this operation.
    return aReturnValue;
}

//*****************************************************************************************************************
//  XElementAccess
//*****************************************************************************************************************
Type SAL_CALL OFrames::getElementType() throw( RuntimeException )
{
    // This "container" support XFrame-interfaces only!
    return ::getCppuType( (const Reference< XFrame >*)NULL );
}

//*****************************************************************************************************************
//  XElementAccess
//*****************************************************************************************************************
sal_Bool SAL_CALL OFrames::hasElements() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "OFrames::hasElements()" )

    // Set default return value.
    sal_Bool bHasElements = sal_False;
    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // If some elements exist ...
        if ( m_pFrameContainer->getCount() > 0 )
        {
            // ... change this state value!
            bHasElements = sal_True;
        }
    }
    // Return result of this operation.
    return bHasElements;
}

//*****************************************************************************************************************
//  proteced method
//*****************************************************************************************************************
void OFrames::impl_resetObject()
{
    // Attention:
    // Write this for multiple calls - NOT AT THE SAME TIME - but for more then one call again)!
    // It exist two ways to call this method. From destructor and from disposing().
    // I can't say, which one is the first. Normaly the disposing-call - but other way ....

    // This instance can't work if the weakreference to owner is invalid!
    // Destroy this to reset this object.
    m_xOwner = WeakReference< XFrame >();
    // Reset pointer to shared container to!
    m_pFrameContainer = NULL;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void OFrames::impl_appendSequence(          Sequence< Reference< XFrame > >&    seqDestination  ,
                                     const  Sequence< Reference< XFrame > >&    seqSource       )
{
    // Get some informations about the sequences.
    sal_Int32                       nSourceCount        = seqSource.getLength();
    sal_Int32                       nDestinationCount   = seqDestination.getLength();
    const Reference< XFrame >*      pSourceAccess       = seqSource.getConstArray();
    Reference< XFrame >*            pDestinationAccess  = seqDestination.getArray();

    // Get memory for result list.
    Sequence< Reference< XFrame > > seqResult           ( nSourceCount + nDestinationCount );
    Reference< XFrame >*            pResultAccess       = seqResult.getArray();
    sal_Int32                       nResultPosition     = 0;

    // Copy all items from first sequence.
    for ( sal_Int32 nSourcePosition=0; nSourcePosition<nSourceCount; ++nSourcePosition )
    {
        pResultAccess[nResultPosition] = pSourceAccess[nSourcePosition];
        ++nResultPosition;
    }

    // Don't manipulate nResultPosition between these two loops!
    // Its the current position in the result list.

    // Copy all items from second sequence.
    for ( sal_Int32 nDestinationPosition=0; nDestinationPosition<nDestinationCount; ++nDestinationPosition )
    {
        pResultAccess[nResultPosition] = pDestinationAccess[nDestinationPosition];
        ++nResultPosition;
    }

    // Return result of this operation.
    seqDestination.realloc( 0 );
    seqDestination = seqResult;
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
// An instance of this class can only work with valid initialization.
// We share the mutex with ouer owner class, need a valid factory to instanciate new services and
// use the access to ouer owner for some operations.
sal_Bool OFrames::impldbg_checkParameter_OFramesCtor(   const   Reference< XMultiServiceFactory >&  xFactory        ,
                                                                Mutex&                              aMutex          ,
                                                        const   Reference< XFrame >&                xOwner          ,
                                                                FrameContainer*                     pFrameContainer )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory         ==  NULL        )   ||
            ( &aMutex           ==  NULL        )   ||
            ( &xOwner           ==  NULL        )   ||
            ( xFactory.is()     ==  sal_False   )   ||
            ( xOwner.is()       ==  sal_False   )   ||
            ( m_pFrameContainer ==  NULL        )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// Its only allowed to add valid references to container.
// AND - alle frames must support XFrames-interface!
sal_Bool OFrames::impldbg_checkParameter_append( const Reference< XFrame >& xFrame )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame       ==  NULL        )   ||
            ( xFrame.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// Its only allowed to add valid references to container...
// ... => You can only delete valid references!
sal_Bool OFrames::impldbg_checkParameter_remove( const Reference< XFrame >& xFrame )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFrame       ==  NULL        )   ||
            ( xFrame.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// A search for frames must initiate with right flags.
// Some one are superflous and not supported yet. But here we control only the range of incoming parameter!
sal_Bool OFrames::impldbg_checkParameter_queryFrames( sal_Int32 nSearchFlags )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            (    nSearchFlags != FrameSearchFlag::AUTO        ) &&
            ( !( nSearchFlags &  FrameSearchFlag::PARENT    ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::SELF      ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::CHILDREN  ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::CREATE    ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::SIBLINGS  ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::TASKS     ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::ALL       ) ) &&
            ( !( nSearchFlags &  FrameSearchFlag::GLOBAL    ) )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework

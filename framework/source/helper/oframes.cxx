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

#include <helper/oframes.hxx>

#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
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
using namespace ::std                           ;

using rtl::OUString;

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
OFrames::OFrames(   const   css::uno::Reference< XMultiServiceFactory >&    xFactory        ,
                    const   css::uno::Reference< XFrame >&              xOwner          ,
                            FrameContainer*                     pFrameContainer )
        //  Init baseclasses first
        :   ThreadHelpBase              ( &Application::GetSolarMutex() )
        // Init member
        ,   m_xFactory                  ( xFactory                      )
        ,   m_xOwner                    ( xOwner                        )
        ,   m_pFrameContainer           ( pFrameContainer               )
        ,   m_bRecursiveSearchProtection( sal_False                     )
{
    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_OFramesCtor( xFactory, xOwner, pFrameContainer ), "OFrames::OFrames()\nInvalid parameter detected!\n" )
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
//  XFrames
//*****************************************************************************************************************
void SAL_CALL OFrames::append( const css::uno::Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_append( xFrame ), "OFrames::append()\nInvalid parameter detected!\n" )

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
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
void SAL_CALL OFrames::remove( const css::uno::Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_remove( xFrame ), "OFrames::remove()\nInvalid parameter detected!\n" )

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
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
Sequence< css::uno::Reference< XFrame > > SAL_CALL OFrames::queryFrames( sal_Int32 nSearchFlags ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    LOG_ASSERT( impldbg_checkParameter_queryFrames( nSearchFlags ), "OFrames::queryFrames()\nInvalid parameter detected!\n" )

    // Set default return value. (empty sequence)
    Sequence< css::uno::Reference< XFrame > > seqFrames;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() == sal_True )
    {
        // Work only, if search was not started here ...!
        if( m_bRecursiveSearchProtection == sal_False )
        {
            // This class is a helper for services, which must implement XFrames.
            // His parent and children are MY parent and children to.
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
                css::uno::Reference< XFrame > xParent( xOwner->getCreator(), UNO_QUERY );
                if( xParent.is() == sal_True )
                {
                    Sequence< css::uno::Reference< XFrame > > seqParent( 1 );
                    seqParent[0] = xParent;
                    impl_appendSequence( seqFrames, seqParent );
                }
            }

            //_____________________________________________________________________________________________________________
            // Add owner to list if SELF is searched.
            if( nSearchFlags & FrameSearchFlag::SELF )
            {
                Sequence< css::uno::Reference< XFrame > > seqSelf( 1 );
                seqSelf[0] = xOwner;
                impl_appendSequence( seqFrames, seqSelf );
            }

            //_____________________________________________________________________________________________________________
            // Add SIBLINGS to list.
            if( nSearchFlags & FrameSearchFlag::SIBLINGS )
            {
                // Else; start a new search.
                // Protect this instance against recursive calls from parents.
                m_bRecursiveSearchProtection = sal_True;
                // Ask parent of my owner for frames and append results to return list.
                css::uno::Reference< XFramesSupplier > xParent( xOwner->getCreator(), UNO_QUERY );
                // If a parent exist ...
                if ( xParent.is() == sal_True )
                {
                    // ... ask him for right frames.
                    impl_appendSequence( seqFrames, xParent->getFrames()->queryFrames( nSearchFlags ) );
                }
                // We have all searched informations.
                // Reset protection-mode.
                m_bRecursiveSearchProtection = sal_False;
            }

            //_____________________________________________________________________________________________________________
            // If searched for children, step over all elements in container and collect the informations.
            if ( nSearchFlags & FrameSearchFlag::CHILDREN )
            {
                // Don't search for parents, siblings and self at childrens!
                // These things are supported by this instance himself.
                sal_Int32 nChildSearchFlags = FrameSearchFlag::SELF | FrameSearchFlag::CHILDREN;
                // Step over all items of container and ask childrens for frames.
                sal_uInt32 nCount = m_pFrameContainer->getCount();
                for ( sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex )
                {
                    // We don't must control this conversion.
                    // We have done this at append()!
                    css::uno::Reference< XFramesSupplier > xItem( (*m_pFrameContainer)[nIndex], UNO_QUERY );
                    impl_appendSequence( seqFrames, xItem->getFrames()->queryFrames( nChildSearchFlags ) );
                }
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
    ResetableGuard aGuard( m_aLock );

    // Set default return value.
    sal_Int32 nCount = 0;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
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
    ResetableGuard aGuard( m_aLock );

      sal_uInt32 nCount = m_pFrameContainer->getCount();
      if ( nIndex < 0 || ( sal::static_int_cast< sal_uInt32 >( nIndex ) >= nCount ))
          throw IndexOutOfBoundsException( OUString("OFrames::getByIndex - Index out of bounds"),
                                           (OWeakObject *)this );

    // Set default return value.
    Any aReturnValue;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
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
    return ::getCppuType( (const css::uno::Reference< XFrame >*)NULL );
}

//*****************************************************************************************************************
//  XElementAccess
//*****************************************************************************************************************
sal_Bool SAL_CALL OFrames::hasElements() throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Set default return value.
    sal_Bool bHasElements = sal_False;
    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
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
void OFrames::impl_appendSequence(          Sequence< css::uno::Reference< XFrame > >&  seqDestination  ,
                                     const  Sequence< css::uno::Reference< XFrame > >&  seqSource       )
{
    // Get some informations about the sequences.
    sal_Int32                       nSourceCount        = seqSource.getLength();
    sal_Int32                       nDestinationCount   = seqDestination.getLength();
    const css::uno::Reference< XFrame >*        pSourceAccess       = seqSource.getConstArray();
    css::uno::Reference< XFrame >*          pDestinationAccess  = seqDestination.getArray();

    // Get memory for result list.
    Sequence< css::uno::Reference< XFrame > >   seqResult           ( nSourceCount + nDestinationCount );
    css::uno::Reference< XFrame >*          pResultAccess       = seqResult.getArray();
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
sal_Bool OFrames::impldbg_checkParameter_OFramesCtor(   const   css::uno::Reference< XMultiServiceFactory >&    xFactory        ,
                                                        const   css::uno::Reference< XFrame >&              xOwner          ,
                                                                FrameContainer*                     pFrameContainer )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory         ==  NULL        )   ||
            ( &xOwner           ==  NULL        )   ||
            ( xFactory.is()     ==  sal_False   )   ||
            ( xOwner.is()       ==  sal_False   )   ||
            ( pFrameContainer   ==  NULL        )
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
sal_Bool OFrames::impldbg_checkParameter_append( const css::uno::Reference< XFrame >& xFrame )
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
sal_Bool OFrames::impldbg_checkParameter_remove( const css::uno::Reference< XFrame >& xFrame )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

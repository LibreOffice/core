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

#include <helper/oframes.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

namespace framework{

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::osl;
using namespace ::std;

//  constructor

OFrames::OFrames( const   css::uno::Reference< XFrame >&              xOwner          ,
                            FrameContainer*                     pFrameContainer )
        :   m_xOwner                    ( xOwner                        )
        ,   m_pFrameContainer           ( pFrameContainer               )
        ,   m_bRecursiveSearchProtection( false                     )
{
    // An instance of this class can only work with valid initialization.
    // We share the mutex with our owner class, need a valid factory to instantiate new services and
    // use the access to our owner for some operations.
    SAL_WARN_IF( !xOwner.is() || !pFrameContainer, "fwk", "OFrames::OFrames(): Invalid parameter detected!" );
}

//  (protected!) destructor

OFrames::~OFrames()
{
    // Reset instance, free memory ....
    impl_resetObject();
}

//  XFrames
void SAL_CALL OFrames::append( const css::uno::Reference< XFrame >& xFrame )
{
    SolarMutexGuard g;

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    SAL_WARN_IF( !xFrame.is(), "fwk", "OFrames::append(): Invalid parameter detected!" );

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // Append frame to the end of the container ...
        m_pFrameContainer->append( xFrame );
        // Set owner of this instance as parent of the new frame in container!
        xFrame->setCreator( xOwner );
    }
    // Else; Do nothing! Our owner is dead.
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::append():Our owner is dead - you can't append any frames ...!" );
}

//  XFrames
void SAL_CALL OFrames::remove( const css::uno::Reference< XFrame >& xFrame )
{
    SolarMutexGuard g;

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    SAL_WARN_IF( !xFrame.is(), "fwk", "OFrames::remove(): Invalid parameter detected!" );

    // Do the follow only, if owner instance valid!
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // Search frame and remove it from container ...
        m_pFrameContainer->remove( xFrame );
        // Don't reset owner-property of removed frame!
        // This must do the caller of this method himself.
        // See documentation of interface XFrames for further information.
    }
    // Else; Do nothing! Our owner is dead.
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::remove(): Our owner is dead - you can't remove any frames ...!" );
}

//  XFrames
Sequence< css::uno::Reference< XFrame > > SAL_CALL OFrames::queryFrames( sal_Int32 nSearchFlags )
{
    SolarMutexGuard g;

    // Safe impossible cases
    // Method is not defined for ALL incoming parameters!
    SAL_WARN_IF( !impldbg_checkParameter_queryFrames( nSearchFlags ), "fwk", "OFrames::queryFrames(): Invalid parameter detected!" );

    // Set default return value. (empty sequence)
    Sequence< css::uno::Reference< XFrame > > seqFrames;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // Work only, if search was not started here ...!
        if( !m_bRecursiveSearchProtection )
        {
            // This class is a helper for services, which must implement XFrames.
            // His parent and children are MY parent and children to.
            // All searchflags are supported by this implementation!
            // If some flags should not be supported - don't call me with this flags!!!

            // Search with AUTO-flag is not supported yet!
            // We think about right implementation.
            SAL_WARN_IF( (nSearchFlags & FrameSearchFlag::AUTO), "fwk", "OFrames::queryFrames(): Search with AUTO-flag is not supported yet!" );

            // Search for ALL and GLOBAL is superfluous!
            // We support all necessary flags, from which these two flags are derived.
            //      ALL     = PARENT + SELF  + CHILDREN + SIBLINGS
            //      GLOBAL  = ALL    + TASKS

            // Add parent to list ... if any exist!
            if( nSearchFlags & FrameSearchFlag::PARENT )
            {
                css::uno::Reference< XFrame > xParent( xOwner->getCreator(), UNO_QUERY );
                if( xParent.is() )
                {
                    Sequence< css::uno::Reference< XFrame > > seqParent( 1 );
                    seqParent[0] = xParent;
                    impl_appendSequence( seqFrames, seqParent );
                }
            }

            // Add owner to list if SELF is searched.
            if( nSearchFlags & FrameSearchFlag::SELF )
            {
                Sequence< css::uno::Reference< XFrame > > seqSelf( 1 );
                seqSelf[0] = xOwner;
                impl_appendSequence( seqFrames, seqSelf );
            }

            // Add SIBLINGS to list.
            if( nSearchFlags & FrameSearchFlag::SIBLINGS )
            {
                // Else; start a new search.
                // Protect this instance against recursive calls from parents.
                m_bRecursiveSearchProtection = true;
                // Ask parent of my owner for frames and append results to return list.
                css::uno::Reference< XFramesSupplier > xParent( xOwner->getCreator(), UNO_QUERY );
                // If a parent exist ...
                if ( xParent.is() )
                {
                    // ... ask him for right frames.
                    impl_appendSequence( seqFrames, xParent->getFrames()->queryFrames( nSearchFlags ) );
                }
                // We have all searched information.
                // Reset protection-mode.
                m_bRecursiveSearchProtection = false;
            }

            // If searched for children, step over all elements in container and collect the information.
            if ( nSearchFlags & FrameSearchFlag::CHILDREN )
            {
                // Don't search for parents, siblings and self at children!
                // These things are supported by this instance himself.
                sal_Int32 const nChildSearchFlags = FrameSearchFlag::SELF | FrameSearchFlag::CHILDREN;
                // Step over all items of container and ask children for frames.
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
    // Else; Do nothing! Our owner is dead.
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::queryFrames(): Our owner is dead - you can't query for frames ...!" );

    // Return result of this operation.
    return seqFrames;
}

//  XIndexAccess
sal_Int32 SAL_CALL OFrames::getCount()
{
    SolarMutexGuard g;

    // Set default return value.
    sal_Int32 nCount = 0;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // Set CURRENT size of container for return.
        nCount = m_pFrameContainer->getCount();
    }

    // Return result.
    return nCount;
}

//  XIndexAccess

Any SAL_CALL OFrames::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard g;

    sal_uInt32 nCount = m_pFrameContainer->getCount();
    if ( nIndex < 0 || ( sal::static_int_cast< sal_uInt32 >( nIndex ) >= nCount ))
          throw IndexOutOfBoundsException("OFrames::getByIndex - Index out of bounds",
                                           static_cast<OWeakObject *>(this) );

    // Set default return value.
    Any aReturnValue;

    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // Get element form container.
        // (If index not valid, FrameContainer return NULL!)
            aReturnValue <<= (*m_pFrameContainer)[nIndex];
    }

    // Return result of this operation.
    return aReturnValue;
}

//  XElementAccess
Type SAL_CALL OFrames::getElementType()
{
    // This "container" support XFrame-interfaces only!
    return cppu::UnoType<XFrame>::get();
}

//  XElementAccess
sal_Bool SAL_CALL OFrames::hasElements()
{
    SolarMutexGuard g;

    // Set default return value.
    bool bHasElements = false;
    // Do the follow only, if owner instance valid.
    // Lock owner for follow operations - make a "hard reference"!
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        // If some elements exist ...
        if ( m_pFrameContainer->getCount() > 0 )
        {
            // ... change this state value!
            bHasElements = true;
        }
    }
    // Return result of this operation.
    return bHasElements;
}

//  protected method

void OFrames::impl_resetObject()
{
    // Attention:
    // Write this for multiple calls - NOT AT THE SAME TIME - but for more than one call again)!
    // It exist two ways to call this method. From destructor and from disposing().
    // I can't say, which one is the first. Normally the disposing-call - but other way ....

    // This instance can't work if the weakreference to owner is invalid!
    // Destroy this to reset this object.
    m_xOwner.clear();
    // Reset pointer to shared container to!
    m_pFrameContainer = nullptr;
}

void OFrames::impl_appendSequence(          Sequence< css::uno::Reference< XFrame > >&  seqDestination  ,
                                     const  Sequence< css::uno::Reference< XFrame > >&  seqSource       )
{
    // Get some information about the sequences.
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
    // It's the current position in the result list.

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

//  debug methods

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the author or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

// A search for frames must initiate with right flags.
// Some one are superfluous and not supported yet. But here we control only the range of incoming parameter!
bool OFrames::impldbg_checkParameter_queryFrames( sal_Int32 nSearchFlags )
{
    // Set default return value.
    bool bOK = true;
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
        bOK = false;
    }
    // Return result of check.
    return bOK;
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

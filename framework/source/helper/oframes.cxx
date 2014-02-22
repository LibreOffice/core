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

#include <helper/oframes.hxx>

#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::std                           ;





OFrames::OFrames( const   css::uno::Reference< XFrame >&              xOwner          ,
                            FrameContainer*                     pFrameContainer )
        
        :   ThreadHelpBase              ( &Application::GetSolarMutex() )
        
        ,   m_xOwner                    ( xOwner                        )
        ,   m_pFrameContainer           ( pFrameContainer               )
        ,   m_bRecursiveSearchProtection( sal_False                     )
{
    
    
    SAL_WARN_IF( !impldbg_checkParameter_OFramesCtor( xOwner, pFrameContainer ), "fwk", "OFrames::OFrames(): Invalid parameter detected!" );
}




OFrames::~OFrames()
{
    
    impl_resetObject();
}




void SAL_CALL OFrames::append( const css::uno::Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    SAL_WARN_IF( !impldbg_checkParameter_append( xFrame ), "fwk", "OFrames::append(): Invalid parameter detected!" );

    
    
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        m_pFrameContainer->append( xFrame );
        
        xFrame->setCreator( xOwner );
    }
    
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::append():Our owner is dead - you can't append any frames ...!" );
}




void SAL_CALL OFrames::remove( const css::uno::Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    SAL_WARN_IF( !impldbg_checkParameter_remove( xFrame ), "fwk", "OFrames::remove(): Invalid parameter detected!" );

    
    
    css::uno::Reference< XFramesSupplier > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        m_pFrameContainer->remove( xFrame );
        
        
        
    }
    
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::remove(): Our owner is dead - you can't remove any frames ...!" );
}




Sequence< css::uno::Reference< XFrame > > SAL_CALL OFrames::queryFrames( sal_Int32 nSearchFlags ) throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    SAL_WARN_IF( !impldbg_checkParameter_queryFrames( nSearchFlags ), "fwk", "OFrames::queryFrames(): Invalid parameter detected!" );

    
    Sequence< css::uno::Reference< XFrame > > seqFrames;

    
    
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        if( m_bRecursiveSearchProtection == sal_False )
        {
            
            
            
            

            
            
            
            SAL_WARN_IF( (nSearchFlags & FrameSearchFlag::AUTO), "fwk", "OFrames::queryFrames(): Search with AUTO-flag is not supported yet!" );

            
            
            
            
            

            
            
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

            
            
            if( nSearchFlags & FrameSearchFlag::SELF )
            {
                Sequence< css::uno::Reference< XFrame > > seqSelf( 1 );
                seqSelf[0] = xOwner;
                impl_appendSequence( seqFrames, seqSelf );
            }

            
            
            if( nSearchFlags & FrameSearchFlag::SIBLINGS )
            {
                
                
                m_bRecursiveSearchProtection = sal_True;
                
                css::uno::Reference< XFramesSupplier > xParent( xOwner->getCreator(), UNO_QUERY );
                
                if ( xParent.is() )
                {
                    
                    impl_appendSequence( seqFrames, xParent->getFrames()->queryFrames( nSearchFlags ) );
                }
                
                
                m_bRecursiveSearchProtection = sal_False;
            }

            
            
            if ( nSearchFlags & FrameSearchFlag::CHILDREN )
            {
                
                
                sal_Int32 nChildSearchFlags = FrameSearchFlag::SELF | FrameSearchFlag::CHILDREN;
                
                sal_uInt32 nCount = m_pFrameContainer->getCount();
                for ( sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex )
                {
                    
                    
                    css::uno::Reference< XFramesSupplier > xItem( (*m_pFrameContainer)[nIndex], UNO_QUERY );
                    impl_appendSequence( seqFrames, xItem->getFrames()->queryFrames( nChildSearchFlags ) );
                }
            }
        }
    }
    
    SAL_WARN_IF( !xOwner.is(), "fwk", "OFrames::queryFrames(): Our owner is dead - you can't query for frames ...!" );

    
    return seqFrames;
}




sal_Int32 SAL_CALL OFrames::getCount() throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    sal_Int32 nCount = 0;

    
    
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        nCount = m_pFrameContainer->getCount();
    }

    
    return nCount;
}




Any SAL_CALL OFrames::getByIndex( sal_Int32 nIndex ) throw( IndexOutOfBoundsException   ,
                                                            WrappedTargetException      ,
                                                            RuntimeException            )
{
    
    ResetableGuard aGuard( m_aLock );

      sal_uInt32 nCount = m_pFrameContainer->getCount();
      if ( nIndex < 0 || ( sal::static_int_cast< sal_uInt32 >( nIndex ) >= nCount ))
          throw IndexOutOfBoundsException("OFrames::getByIndex - Index out of bounds",
                                           (OWeakObject *)this );

    
    Any aReturnValue;

    
    
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        
            aReturnValue <<= (*m_pFrameContainer)[nIndex];
    }

    
    return aReturnValue;
}




Type SAL_CALL OFrames::getElementType() throw( RuntimeException )
{
    
    return ::getCppuType( (const css::uno::Reference< XFrame >*)NULL );
}




sal_Bool SAL_CALL OFrames::hasElements() throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    sal_Bool bHasElements = sal_False;
    
    
    css::uno::Reference< XFrame > xOwner( m_xOwner.get(), UNO_QUERY );
    if ( xOwner.is() )
    {
        
        if ( m_pFrameContainer->getCount() > 0 )
        {
            
            bHasElements = sal_True;
        }
    }
    
    return bHasElements;
}




void OFrames::impl_resetObject()
{
    
    
    
    

    
    
    m_xOwner = WeakReference< XFrame >();
    
    m_pFrameContainer = NULL;
}




void OFrames::impl_appendSequence(          Sequence< css::uno::Reference< XFrame > >&  seqDestination  ,
                                     const  Sequence< css::uno::Reference< XFrame > >&  seqSource       )
{
    
    sal_Int32                       nSourceCount        = seqSource.getLength();
    sal_Int32                       nDestinationCount   = seqDestination.getLength();
    const css::uno::Reference< XFrame >*        pSourceAccess       = seqSource.getConstArray();
    css::uno::Reference< XFrame >*          pDestinationAccess  = seqDestination.getArray();

    
    Sequence< css::uno::Reference< XFrame > >   seqResult           ( nSourceCount + nDestinationCount );
    css::uno::Reference< XFrame >*          pResultAccess       = seqResult.getArray();
    sal_Int32                       nResultPosition     = 0;

    
    for ( sal_Int32 nSourcePosition=0; nSourcePosition<nSourceCount; ++nSourcePosition )
    {
        pResultAccess[nResultPosition] = pSourceAccess[nSourcePosition];
        ++nResultPosition;
    }

    
    

    
    for ( sal_Int32 nDestinationPosition=0; nDestinationPosition<nDestinationCount; ++nDestinationPosition )
    {
        pResultAccess[nResultPosition] = pDestinationAccess[nDestinationPosition];
        ++nResultPosition;
    }

    
    seqDestination.realloc( 0 );
    seqDestination = seqResult;
}





/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/





sal_Bool OFrames::impldbg_checkParameter_OFramesCtor(   const   css::uno::Reference< XFrame >&              xOwner          ,
                                                                FrameContainer*                     pFrameContainer )
{
    return xOwner.is() && pFrameContainer != 0;
}




sal_Bool OFrames::impldbg_checkParameter_append( const css::uno::Reference< XFrame >& xFrame )
{
    return xFrame.is();
}




sal_Bool OFrames::impldbg_checkParameter_remove( const css::uno::Reference< XFrame >& xFrame )
{
    return xFrame.is();
}




sal_Bool OFrames::impldbg_checkParameter_queryFrames( sal_Int32 nSearchFlags )
{
    
    sal_Bool bOK = sal_True;
    
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
    
    return bOK ;
}

}       

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

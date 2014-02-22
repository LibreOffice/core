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

#include <helper/ocomponentaccess.hxx>
#include <helper/ocomponentenumeration.hxx>

#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>

#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;




OComponentAccess::OComponentAccess( const css::uno::Reference< XDesktop >& xOwner )
        
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        
        ,   m_xOwner        ( xOwner                        )
{
    
    SAL_WARN_IF( !impldbg_checkParameter_OComponentAccessCtor( xOwner ), "fwk", "OComponentAccess::OComponentAccess(): Invalid parameter detected!" );
}




OComponentAccess::~OComponentAccess()
{
}




css::uno::Reference< XEnumeration > SAL_CALL OComponentAccess::createEnumeration() throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    css::uno::Reference< XEnumeration > xReturn = css::uno::Reference< XEnumeration >();

    
    css::uno::Reference< XInterface > xLock = m_xOwner.get();
    if ( xLock.is() )
    {
        
        
        

        Sequence< css::uno::Reference< XComponent > > seqComponents;
        impl_collectAllChildComponents( css::uno::Reference< XFramesSupplier >( xLock, UNO_QUERY ), seqComponents );
        OComponentEnumeration* pEnumeration = new OComponentEnumeration( seqComponents );
        xReturn = css::uno::Reference< XEnumeration >( (OWeakObject*)pEnumeration, UNO_QUERY );
    }

    
    return xReturn;
}




Type SAL_CALL OComponentAccess::getElementType() throw( RuntimeException )
{
    
    
    return ::getCppuType((const css::uno::Reference< XComponent >*)NULL);
}




sal_Bool SAL_CALL OComponentAccess::hasElements() throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    sal_Bool bReturn = sal_False;

    
    css::uno::Reference< XFramesSupplier > xLock( m_xOwner.get(), UNO_QUERY );
    if ( xLock.is() )
    {
        
        bReturn = xLock->getFrames()->hasElements();
    }

    
    return bReturn;
}




void OComponentAccess::impl_collectAllChildComponents(  const   css::uno::Reference< XFramesSupplier >&         xNode           ,
                                                                 Sequence< css::uno::Reference< XComponent > >& seqComponents   )
{
    
    if( xNode.is() )
    {
        

        
        

        sal_Int32 nComponentCount = seqComponents.getLength();

        const css::uno::Reference< XFrames >                xContainer  = xNode->getFrames();
        const Sequence< css::uno::Reference< XFrame > > seqFrames   = xContainer->queryFrames( FrameSearchFlag::CHILDREN );

        const sal_Int32 nFrameCount = seqFrames.getLength();
        for( sal_Int32 nFrame=0; nFrame<nFrameCount; ++nFrame )
        {
            css::uno::Reference< XComponent > xComponent = impl_getFrameComponent( seqFrames[nFrame] );
            if( xComponent.is() )
            {
                nComponentCount++;
                seqComponents.realloc( nComponentCount );
                seqComponents[nComponentCount-1] = xComponent;
            }
        }
    }
    
}




css::uno::Reference< XComponent > OComponentAccess::impl_getFrameComponent( const css::uno::Reference< XFrame >& xFrame ) const
{
    
    css::uno::Reference< XComponent > xComponent = css::uno::Reference< XComponent >();
    
    css::uno::Reference< XController > xController = xFrame->getController();
    if ( !xController.is() )
    {
        
        xComponent = css::uno::Reference< XComponent >( xFrame->getComponentWindow(), UNO_QUERY );
    }
    else
    {
        
        css::uno::Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
        if ( xModel.is() )
        {
            
            xComponent = css::uno::Reference< XComponent >( xModel, UNO_QUERY );
        }
        else
        {
            
            xComponent = css::uno::Reference< XComponent >( xController, UNO_QUERY );
        }
    }

    return xComponent;
}





/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/


sal_Bool OComponentAccess::impldbg_checkParameter_OComponentAccessCtor( const   css::uno::Reference< XDesktop >&      xOwner  )
{
    return xOwner.is();
}

}       

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <helper/ocomponentenumeration.hxx>

#include <threadhelp/resetableguard.hxx>

#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;




OComponentEnumeration::OComponentEnumeration( const Sequence< css::uno::Reference< XComponent > >& seqComponents )
        
        
        
        
        
        
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        
        ,   m_nPosition     ( 0                             )   
        ,   m_seqComponents ( seqComponents                 )
{
    
    
    SAL_WARN_IF( !impldbg_checkParameter_OComponentEnumerationCtor( seqComponents ), "fwk", "OComponentEnumeration::OComponentEnumeration(): Invalid parameter detected!" );
}




OComponentEnumeration::~OComponentEnumeration()
{
    
    impl_resetObject();
}




void SAL_CALL OComponentEnumeration::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    (void) aEvent;
    SAL_WARN_IF( !impldbg_checkParameter_disposing( aEvent ), "fwk", "OComponentEnumeration::disposing(): Invalid parameter detected!" );

    
    impl_resetObject();
}




sal_Bool SAL_CALL OComponentEnumeration::hasMoreElements() throw( RuntimeException )
{
    
    ResetableGuard aGuard( m_aLock );

    
    
    
    
    return ( m_nPosition < (sal_uInt32)(m_seqComponents.getLength()) );
}




Any SAL_CALL OComponentEnumeration::nextElement() throw(    NoSuchElementException  ,
                                                             WrappedTargetException ,
                                                            RuntimeException        )
{
    
    ResetableGuard aGuard( m_aLock );

    
    if ( hasMoreElements() == sal_False )
    {
        
        throw NoSuchElementException();
    }

    
    Any aComponent;
    aComponent <<= m_seqComponents[m_nPosition];
    
    ++m_nPosition;

    
    return aComponent;
}




void OComponentEnumeration::impl_resetObject()
{
    
    
    
    

    
    m_seqComponents.realloc( 0 );
    
    
    
    
    
    m_nPosition = 0 ;
}





/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/



sal_Bool OComponentEnumeration::impldbg_checkParameter_OComponentEnumerationCtor( const Sequence< css::uno::Reference< XComponent > >& seqComponents )
{
    
    sal_Bool bOK = sal_True;
    
    if  (
            ( &seqComponents == NULL )
        )
    {
        bOK = sal_False ;
    }
    
    return bOK ;
}


sal_Bool OComponentEnumeration::impldbg_checkParameter_disposing( const EventObject& aEvent )
{
    return aEvent.Source.is();
}

}       

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

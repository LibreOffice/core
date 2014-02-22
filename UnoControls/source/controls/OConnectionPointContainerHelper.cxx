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

#include "OConnectionPointContainerHelper.hxx"

#include "OConnectionPointHelper.hxx"





using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;

namespace unocontrols{





OConnectionPointContainerHelper::OConnectionPointContainerHelper( Mutex& aMutex )
    : m_aSharedMutex        ( aMutex    )
    , m_aMultiTypeContainer ( aMutex    )
{
}

OConnectionPointContainerHelper::~OConnectionPointContainerHelper()
{
}





Any SAL_CALL OConnectionPointContainerHelper::queryInterface( const Type& aType ) throw( RuntimeException )
{
    
    

    
    Any aReturn ( ::cppu::queryInterface(   aType                                               ,
                                               static_cast< XConnectionPointContainer*  > ( this )
                                        )
                );

    
    if ( !aReturn.hasValue() )
    {
        
        aReturn = OWeakObject::queryInterface( aType );
    }

    return aReturn ;
}





void SAL_CALL OConnectionPointContainerHelper::acquire() throw()
{
    
    

    
    OWeakObject::acquire();
}





void SAL_CALL OConnectionPointContainerHelper::release() throw()
{
    
    

    
    OWeakObject::release();
}





Sequence< Type > SAL_CALL OConnectionPointContainerHelper::getConnectionPointTypes() throw( RuntimeException )
{
    
    return m_aMultiTypeContainer.getContainedTypes();
}





Reference< XConnectionPoint > SAL_CALL OConnectionPointContainerHelper::queryConnectionPoint( const Type& aType ) throw( RuntimeException )
{
    
    Reference< XConnectionPoint > xConnectionPoint;

    
    OInterfaceContainerHelper* pSpecialContainer = m_aMultiTypeContainer.getContainer( aType );
    if ( pSpecialContainer && pSpecialContainer->getLength() > 0 )
    {
        
        MutexGuard aGuard( m_aSharedMutex );
        
        OConnectionPointHelper* pNewConnectionPoint = new OConnectionPointHelper( m_aSharedMutex, this, aType );
        xConnectionPoint = Reference< XConnectionPoint >( (OWeakObject*)pNewConnectionPoint, UNO_QUERY );
    }

    return xConnectionPoint ;
}





void SAL_CALL OConnectionPointContainerHelper::advise(  const   Type&                       aType       ,
                                                        const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    
    m_aMultiTypeContainer.addInterface( aType, xListener );
}





void SAL_CALL OConnectionPointContainerHelper::unadvise(    const   Type&                       aType       ,
                                                            const   Reference< XInterface >&    xListener   ) throw( RuntimeException )
{
    
    m_aMultiTypeContainer.removeInterface( aType, xListener );
}






OMultiTypeInterfaceContainerHelper& OConnectionPointContainerHelper::impl_getMultiTypeContainer()
{
    
    
    return m_aMultiTypeContainer;
}

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

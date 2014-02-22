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

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <svx/unoprov.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

class SvxShapeCollectionMutex
{
public:
    ::osl::Mutex maMutex;
};

class SvxShapeCollection :
    public cppu::WeakAggImplHelper3<drawing::XShapes, lang::XServiceInfo, lang::XComponent>,
    public SvxShapeCollectionMutex
{
private:
    cppu::OInterfaceContainerHelper maShapeContainer;

    cppu::OBroadcastHelper mrBHelper;

    virtual void disposing() throw();

public:
    SvxShapeCollection() throw();
    virtual ~SvxShapeCollection() throw();

    
    virtual void SAL_CALL release() throw();

    
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

SvxShapeCollection::SvxShapeCollection() throw()
: maShapeContainer( maMutex ), mrBHelper( maMutex )
{
}


SvxShapeCollection::~SvxShapeCollection() throw()
{
}


void SvxShapeCollection::release() throw()
{
    uno::Reference< uno::XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_atomic_decrement( &m_refCount ) == 0)
        {
            if (! mrBHelper.bDisposed)
            {
                uno::Reference< uno::XInterface > xHoldAlive( (uno::XWeak*)this );
                
                try
                {
                    dispose();
                }
                catch(::com::sun::star::uno::Exception&)
                {
                    
                }

                
                OSL_ASSERT( m_refCount == 1 );
                
                return;
            }
        }
        
        osl_atomic_increment( &m_refCount );
    }
    OWeakAggObject::release();
}


void SvxShapeCollection::disposing() throw()
{
    maShapeContainer.clear();
}


void SvxShapeCollection::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    
    
    
    uno::Reference< lang::XComponent > xSelf( this );

    
    
    bool bDoDispose = false;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        
        mrBHelper.bInDispose = sal_True;
        bDoDispose = true;
    }
    }

    
    if( bDoDispose )
    {
        
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( (lang::XComponent *)this ) );
            document::EventObject aEvt;
            aEvt.Source = xSource;
            
            
            mrBHelper.aLC.disposeAndClear( aEvt );
            
            disposing();
        }
        catch(const ::com::sun::star::uno::Exception&)
        {
            
            
            
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw;
        }

        
        
        mrBHelper.bDisposed = sal_True;
        mrBHelper.bInDispose = sal_False;
    }
    else
    {
        
        
        OSL_TRACE( "OComponentHelper::dispose() - dispose called twice" );
    }
}


void SAL_CALL SvxShapeCollection::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.addListener( ::getCppuType( &aListener ) , aListener );
}


void SAL_CALL SvxShapeCollection::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.removeListener( ::getCppuType( &aListener ) , aListener );
}



void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.addInterface( xShape );
}


void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.removeInterface( xShape );
}


sal_Int32 SAL_CALL SvxShapeCollection::getCount() throw( uno::RuntimeException )
{
    return maShapeContainer.getLength();
}


uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< Reference< uno::XInterface> > xElements( maShapeContainer.getElements() );


    return uno::makeAny( Reference< drawing::XShape>(static_cast< drawing::XShape* >( xElements.getArray()[Index].get())) );
}


uno::Type SAL_CALL SvxShapeCollection::getElementType() throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape >*)0);
}

sal_Bool SAL_CALL SvxShapeCollection::hasElements() throw( uno::RuntimeException )
{
    return getCount() != 0;
}


OUString SAL_CALL SvxShapeCollection::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString("com.sun.star.drawing.SvxShapeCollection");
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return cppu::supportsService( this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq(2);
    aSeq.getArray()[0] = "com.sun.star.drawing.Shapes";
    aSeq.getArray()[1] = "com.sun.star.drawing.ShapeCollection";
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_drawing_SvxShapeCollection_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvxShapeCollection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

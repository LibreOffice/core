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
#ifndef __PLUGIN_MODEL_HXX
#define __PLUGIN_MODEL_HXX

#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weakagg.hxx>
#include <rtl/alloc.h>

#include <list>

using namespace com::sun::star::uno;

class BroadcasterHelperHolder
{
protected:
    ::cppu::OBroadcastHelper  m_aHelper;
public:
    BroadcasterHelperHolder( osl::Mutex& rMutex ) :
            m_aHelper( rMutex ) {}
    ~BroadcasterHelperHolder() {}

    ::cppu::OBroadcastHelper& getHelper() { return m_aHelper; }

};

class PluginModel : public BroadcasterHelperHolder,
                    public cppu::OPropertySetHelper,
                    public cppu::OPropertyArrayHelper,
                    public cppu::OWeakAggObject,
                    public com::sun::star::lang::XComponent,
                    public com::sun::star::io::XPersistObject,
                    public com::sun::star::awt::XControlModel
{
  private:
    rtl::OUString m_aCreationURL;
    rtl::OUString m_aMimeType;

    std::list< Reference< com::sun::star::lang::XEventListener > >
        m_aDisposeListeners;
  public:
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize ) throw()
        { return rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem ) throw()
        { rtl_freeMemory( pMem ); }

    PluginModel();
    PluginModel( const rtl::OUString& rURL, const rtl::OUString& rMimeType );
    virtual ~PluginModel();


    const rtl::OUString& getCreationURL() { return m_aCreationURL; }
    void setMimeType( const rtl::OUString& rMime ) { m_aMimeType = rMime; }

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( com::sun::star::uno::RuntimeException )
        { return OWeakAggObject::queryInterface( rType ); }
    virtual void SAL_CALL acquire()  throw()
    { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release()  throw()
    { OWeakAggObject::release(); }

    virtual Any SAL_CALL queryAggregation( const Type& ) throw( com::sun::star::uno::RuntimeException );


    // com::sun::star::lang::XTypeProvider

    static Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_Static(void) throw(  );
    static rtl::OUString SAL_CALL getImplementationName_Static() throw(  )
    {
        /** the soplayer uses this name in its source! maybe not after 5.2 */
        return rtl::OUString( "com.sun.star.extensions.PluginModel" );
    }

    // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual sal_Bool  SAL_CALL convertFastPropertyValue( Any & rConvertedValue,
                                                         Any & rOldValue,
                                                         sal_Int32 nHandle,
                                                         const Any& rValue ) throw();
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                            const Any& rValue )
        throw(::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const throw();
    virtual Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw();

    // com::sun::star::io::XPersistObject
    virtual rtl::OUString SAL_CALL getServiceName() throw();
    virtual void SAL_CALL write(const Reference< com::sun::star::io::XObjectOutputStream > & OutStream) throw();
    virtual void SAL_CALL read(const Reference< com::sun::star::io::XObjectInputStream > & InStream) throw();

    // com::sun::star::lang::XComponent
    virtual void SAL_CALL addEventListener( const Reference< com::sun::star::lang::XEventListener > & l ) throw();
    virtual void SAL_CALL removeEventListener( const Reference< com::sun::star::lang::XEventListener > & l ) throw();
    virtual void SAL_CALL dispose() throw();
  private:
    using cppu::OPropertySetHelper::getFastPropertyValue;
};
Reference< XInterface >  SAL_CALL PluginModel_CreateInstance( const Reference< com::sun::star::lang::XMultiServiceFactory >  & ) throw( Exception );

#endif // __PLUGIN_MODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

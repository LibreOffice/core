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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MODEL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MODEL_HXX

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
};

class PluginModel : public BroadcasterHelperHolder,
                    public cppu::OPropertySetHelper,
                    public cppu::OPropertyArrayHelper,
                    public cppu::OWeakAggObject,
                    public css::lang::XComponent,
                    public css::io::XPersistObject,
                    public css::awt::XControlModel
{
  private:
    OUString m_aCreationURL;
    OUString m_aMimeType;

    std::list< Reference< css::lang::XEventListener > >
        m_aDisposeListeners;
  public:
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize ) throw()
        { return rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem ) throw()
        { rtl_freeMemory( pMem ); }

    PluginModel();
    PluginModel( const OUString& rURL, const OUString& rMimeType );
    virtual ~PluginModel();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        { return OWeakAggObject::queryInterface( rType ); }
    virtual void SAL_CALL acquire()  throw() SAL_OVERRIDE
    { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release()  throw() SAL_OVERRIDE
    { OWeakAggObject::release(); }

    virtual Any SAL_CALL queryAggregation( const Type& ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


    // css::lang::XTypeProvider

    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static() throw(  );
    static OUString SAL_CALL getImplementationName_Static() throw(  )
    {
        /** the soplayer uses this name in its source! maybe not after 5.2 */
        return OUString( "com.sun.star.extensions.PluginModel" );
    }

    // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
    virtual sal_Bool  SAL_CALL convertFastPropertyValue( Any & rConvertedValue,
                                                         Any & rOldValue,
                                                         sal_Int32 nHandle,
                                                         const Any& rValue ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                            const Any& rValue )
        throw(css::uno::Exception, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const throw() SAL_OVERRIDE;
    virtual Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(std::exception) SAL_OVERRIDE;

    // css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const Reference< css::io::XObjectOutputStream > & OutStream) throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const Reference< css::io::XObjectInputStream > & InStream) throw(std::exception) SAL_OVERRIDE;

    // css::lang::XComponent
    virtual void SAL_CALL addEventListener( const Reference< css::lang::XEventListener > & l ) throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const Reference< css::lang::XEventListener > & l ) throw(std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL dispose() throw(std::exception) SAL_OVERRIDE;
  private:
    using cppu::OPropertySetHelper::getFastPropertyValue;
};
Reference< XInterface >  SAL_CALL PluginModel_CreateInstance( const Reference< css::lang::XMultiServiceFactory >  & ) throw( Exception );

#endif // INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_MODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

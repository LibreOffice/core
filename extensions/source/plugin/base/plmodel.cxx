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

#include <plugin/model.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/queryinterface.hxx>

using namespace com::sun::star::uno;


Reference< XInterface > SAL_CALL PluginModel_CreateInstance( const Reference< css::lang::XMultiServiceFactory >  & ) throw( Exception )
{
    Reference< XInterface >  xService = *new PluginModel();
    return xService;
}

Any PluginModel::queryAggregation( const Type& type ) throw( RuntimeException, std::exception )
{
    Any aRet( ::cppu::queryInterface( type,
                                      static_cast< css::lang::XComponent* >(this),
                                      static_cast< css::io::XPersistObject* >(this ),
                                      static_cast< css::awt::XControlModel* >(this),
                                      static_cast< css::beans::XPropertySet* >(this),
                                      static_cast< css::beans::XMultiPropertySet* >(this),
                                      static_cast< css::beans::XFastPropertySet* >(this)
        ) );
    return aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( type );
}


// XPluginManager_Impl
Sequence< OUString > PluginModel::getSupportedServiceNames_Static() throw()
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.plugin.PluginModel";
    return aSNS;
}


static const char* aCreationURL = "URL";
static const char* aMime = "TYPE";

static ::osl::Mutex aPropertyMutex;

static css::beans::Property aProps[] =
{
    css::beans::Property(
        OUString::createFromAscii( aMime ),
        1,
        ::cppu::UnoType<OUString>::get(),
        css::beans::PropertyAttribute::BOUND ),
    css::beans::Property(
        OUString::createFromAscii( aCreationURL ),
        2,
        ::cppu::UnoType<OUString>::get(),
        css::beans::PropertyAttribute::BOUND )
};

PluginModel::PluginModel() :
        BroadcasterHelperHolder( aPropertyMutex ),
        OPropertySetHelper( m_aHelper ),
        OPropertyArrayHelper( aProps, 2 )
{
}

PluginModel::PluginModel(const OUString& rURL, const OUString& rMimeType ) :
        BroadcasterHelperHolder( aPropertyMutex ),
        OPropertySetHelper( m_aHelper ),
        OPropertyArrayHelper( aProps, 2 ),
        m_aCreationURL( rURL ),
        m_aMimeType( rMimeType )
{
}

PluginModel::~PluginModel()
{
}

Reference< css::beans::XPropertySetInfo >  PluginModel::getPropertySetInfo() throw(std::exception)
{
    static Reference< css::beans::XPropertySetInfo > aInfo =
        createPropertySetInfo( *this );
    return aInfo;
}

::cppu::IPropertyArrayHelper& PluginModel::getInfoHelper()
{
    return *this;
}

sal_Bool PluginModel::convertFastPropertyValue( Any & rConvertedValue,
                                                Any & rOldValue,
                                                sal_Int32 nHandle,
                                                const Any& rValue ) throw()
{
    if( nHandle == 1 || nHandle == 2 )
    {
        if( rValue.getValueTypeClass() == TypeClass_STRING )
        {
            rConvertedValue = rValue;
            if( nHandle == 2 )
                rOldValue <<= m_aCreationURL;
            else if( nHandle == 1 )
                rOldValue <<= m_aMimeType;
            return sal_True;
        }
    }
    return sal_False;
}

void PluginModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                    const Any& rValue )
    throw(css::uno::Exception, std::exception)
{
    if( rValue.getValueTypeClass() == TypeClass_STRING ) // FIXME wrong type!

    {
        if( nHandle == 2 )
            rValue >>= m_aCreationURL;
        else if( nHandle == 1 )
            rValue >>= m_aMimeType;
    }
    else
        throw css::lang::IllegalArgumentException();
}

void PluginModel::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const throw()
{
    if( nHandle == 2 )
        rValue <<= m_aCreationURL;
    else if( nHandle == 1 )
        rValue <<= m_aMimeType;
}

//---- css::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::addEventListener( const Reference< css::lang::XEventListener > & l ) throw(std::exception)
{
    m_aDisposeListeners.push_back( l );
}

//---- css::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::removeEventListener( const Reference< css::lang::XEventListener > & l ) throw(std::exception)
{
    m_aDisposeListeners.remove( l );
}

//---- css::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::dispose() throw(std::exception)
{
    // send disposing events
    css::lang::EventObject aEvt;
    aEvt.Source = static_cast<cppu::OWeakObject*>(this);
    ::std::list< Reference< css::lang::XEventListener > > aLocalListeners = m_aDisposeListeners;
    for( ::std::list< Reference< css::lang::XEventListener > >::iterator it = aLocalListeners.begin();
         it != aLocalListeners.end(); ++it )
        (*it)->disposing( aEvt );

    m_aDisposeListeners.clear();

    disposing();
}


// css::io::XPersistObject
OUString PluginModel::getServiceName() throw(std::exception)
{
    return OUString("com.sun.star.plugin.PluginModel");
}

void PluginModel::write(const Reference< css::io::XObjectOutputStream > & OutStream) throw(std::exception)
{
    OutStream->writeUTF( m_aCreationURL );
}

void PluginModel::read(const Reference< css::io::XObjectInputStream > & InStream) throw(std::exception)
{
    m_aCreationURL = InStream->readUTF();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

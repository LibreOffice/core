/*************************************************************************
 *
 *  $RCSfile: plmodel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <plugin/model.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

using namespace com::sun::star::uno;

//==================================================================================================
Reference< XInterface > SAL_CALL PluginModel_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & ) throw( Exception )
{
    Reference< XInterface >  xService = *new PluginModel();
    return xService;
}

Any PluginModel::queryAggregation( const Type& type )
{
    Any aRet( ::cppu::queryInterface( type,
                                      static_cast< ::com::sun::star::lang::XComponent* >(this),
                                      static_cast< ::com::sun::star::io::XPersistObject* >(this ),
                                      static_cast< ::com::sun::star::awt::XControlModel* >(this),
                                      static_cast< ::com::sun::star::beans::XPropertySet* >(this),
                                      static_cast< ::com::sun::star::beans::XMultiPropertySet* >(this),
                                      static_cast< ::com::sun::star::beans::XFastPropertySet* >(this)
        ) );
    return aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( type );
}


// ::com::sun::star::lang::XServiceInfo
::rtl::OUString PluginModel::getImplementationName() throw(  )

{
    return getImplementationName_Static();
}

// ::com::sun::star::lang::XServiceInfo
sal_Bool PluginModel::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// ::com::sun::star::lang::XServiceInfo
Sequence< ::rtl::OUString > PluginModel::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}

// XPluginManager_Impl
Sequence< ::rtl::OUString > PluginModel::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.plugin.PluginModel" );
    return aSNS;
}


static char* aCreationURL = "URL";

static ::osl::Mutex aPropertyMutex;

static ::com::sun::star::beans::Property aProps[] =
{
    ::com::sun::star::beans::Property(
        ::rtl::OUString::createFromAscii( aCreationURL ),
        -1,
        ::getCppuType((const ::rtl::OUString*)0),
        ::com::sun::star::beans::PropertyAttribute::BOUND )
};

PluginModel::PluginModel() :
        BroadcasterHelperHolder( aPropertyMutex ),
        OPropertySetHelper( m_aHelper ),
        OPropertyArrayHelper( aProps, 1 )
{
}

PluginModel::PluginModel(const ::rtl::OUString& rURL) :
        BroadcasterHelperHolder( aPropertyMutex ),
        OPropertySetHelper( m_aHelper ),
        OPropertyArrayHelper( aProps, 1 ),
        m_aCreationURL( rURL )
{
}

PluginModel::~PluginModel()
{
}

Reference< ::com::sun::star::beans::XPropertySetInfo >  PluginModel::getPropertySetInfo()
{
    static Reference< ::com::sun::star::beans::XPropertySetInfo > aInfo =
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
                                            const Any& rValue )
{
    if( rValue.getValueTypeClass() == typelib_TypeClass_STRING )

    {
        rConvertedValue = rValue;
        rOldValue <<= m_aCreationURL;
        return sal_True;
    }
    return sal_False;
}

void PluginModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                    const Any& rValue )
    throw( ::com::sun::star::lang::IllegalArgumentException )
{
    if( rValue.getValueTypeClass() == typelib_TypeClass_STRING )

    {
        rValue >>= m_aCreationURL;
    }
    else
        throw ::com::sun::star::lang::IllegalArgumentException();
}

void PluginModel::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    rValue <<= m_aCreationURL;
}

//---- ::com::sun::star::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::addEventListener( const Reference< ::com::sun::star::lang::XEventListener > & l )
{
    m_aDisposeListeners.push_back( l );
}

//---- ::com::sun::star::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::removeEventListener( const Reference< ::com::sun::star::lang::XEventListener > & l )
{
    m_aDisposeListeners.remove( l );
}

//---- ::com::sun::star::lang::XComponent ----------------------------------------------------------------------------------
void PluginModel::dispose(void)
{
    // send disposing events
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    ::std::list< Reference< ::com::sun::star::lang::XEventListener > > aLocalListeners = m_aDisposeListeners;
    for( ::std::list< Reference< ::com::sun::star::lang::XEventListener > >::iterator it = aLocalListeners.begin();
         it != aLocalListeners.end(); ++it )
        (*it)->disposing( aEvt );

    m_aDisposeListeners.clear();

    disposing();
}


// ::com::sun::star::io::XPersistObject
::rtl::OUString PluginModel::getServiceName()
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.plugin.PluginModel" );
}

void PluginModel::write(const Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream)
{
    OutStream->writeUTF( m_aCreationURL );
}

void PluginModel::read(const Reference< ::com::sun::star::io::XObjectInputStream > & InStream)
{
    m_aCreationURL = InStream->readUTF();
}

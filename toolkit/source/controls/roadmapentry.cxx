/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <toolkit/controls/roadmapentry.hxx>

#ifndef _RTL_USTRING_HXX_
#include <rtl/OUString.hxx>
#endif


#include <com/sun/star/beans/PropertyAttribute.hpp>


ORoadmapEntry::ORoadmapEntry() : ORoadmapEntry_Base( )
                                ,OPropertyContainer( GetBroadcastHelper() )
{
    // registerProperty or registerMayBeVoidProperty or registerPropertyNoMember

    registerProperty( ::rtl::OUString::createFromAscii( "Label" ), RM_PROPERTY_ID_LABEL,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_sLabel, ::getCppuType( &m_sLabel ) );
    m_nID = -1;
    registerProperty( ::rtl::OUString::createFromAscii( "ID" ), RM_PROPERTY_ID_ID,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_nID, ::getCppuType( &m_nID ) );
    m_bEnabled = sal_True;
    registerProperty( ::rtl::OUString::createFromAscii( "Enabled" ), RM_PROPERTY_ID_ENABLED,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bEnabled, ::getCppuType( &m_bEnabled ) );

    registerProperty( ::rtl::OUString::createFromAscii( "Interactive" ), RM_PROPERTY_ID_INTERACTIVE,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bInteractive, ::getCppuType( &m_bInteractive ) );


    // ...

    // Note that the list of registered properties has to be fixed: Different
    // instances of this class have to register the same set of properties with
    // the same attributes.
    //
    // This is because all instances of the class share the same PropertySetInfo
    // which has been built from the registered property of _one_ instance.
}

//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
    // order matters:
    //  the first is the class name
    //  the second is the class which implements the ref-counting
    //  the third up to n-th (when using IMPLEMENT_FORWARD_*3 and so on) are other base classes
    //  whose XInterface and XTypeProvider implementations should be merged

//--------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star:: beans::XPropertySetInfo > SAL_CALL
    ORoadmapEntry::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >(
        createPropertySetInfo( getInfoHelper() ) );
}

::rtl::OUString SAL_CALL ORoadmapEntry::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aStr = ::rtl::OUString::createFromAscii("com.sun.star.comp.toolkit.RoadmapItem");
    return aStr;
}

sal_Bool SAL_CALL ORoadmapEntry::supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException)
{
    return ServiceName.equals( ::rtl::OUString::createFromAscii( "com.sun.star.awt.RoadmapItem" ) );
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ORoadmapEntry::getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.RoadmapItem" );
    return aRet;
}
//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ORoadmapEntry::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ORoadmapEntry::createArrayHelper() const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

/*************************************************************************
 *
 *  $RCSfile: roadmapentry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:44:26 $
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

#ifndef _TOOLKIT_ROADMAPENTRY_HXX_
#include <toolkit/controls/roadmapentry.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/OUString.hxx>
#endif


#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif


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

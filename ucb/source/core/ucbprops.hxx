/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucbprops.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:17:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UCBPROPS_HXX
#define _UCBPROPS_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

//=========================================================================

#define PROPERTIES_MANAGER_SERVICE_NAME "com.sun.star.ucb.PropertiesManager"

//============================================================================
//
// class UcbPropertiesManager.
//
//============================================================================

class UcbPropertiesManager :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::beans::XPropertySetInfo
{
    com::sun::star::uno::Sequence< com::sun::star::beans::Property >*
                                m_pProps;
    osl::Mutex                  m_aMutex;

private:
    sal_Bool queryProperty( const rtl::OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    UcbPropertiesManager( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory >&
                                rxSMgr );
    virtual ~UcbPropertiesManager();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const rtl::OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( com::sun::star::uno::RuntimeException );
};

#endif /* !_UCBPROPS_HXX */


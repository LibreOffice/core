/*************************************************************************
 *
 *  $RCSfile: hierarchydatasource.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2001-07-03 11:13:55 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef _HIERARCHYDATASOURCE_HXX
#define _HIERARCHYDATASOURCE_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace cppu { class OInterfaceContainerHelper; }

namespace hierarchy_ucp {

//=========================================================================

class HierarchyDataSource : public cppu::OWeakObject,
                            public com::sun::star::lang::XServiceInfo,
                            public com::sun::star::lang::XTypeProvider,
                            public com::sun::star::lang::XComponent,
                            public com::sun::star::lang::XMultiServiceFactory
{
    osl::Mutex m_aMutex;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
    cppu::OInterfaceContainerHelper * m_pDisposeEventListeners;

public:
    HierarchyDataSource( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory > &
                                rxServiceMgr );
    virtual ~HierarchyDataSource();

    // XInterface
    XINTERFACE_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener > & xListener )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener > & aListener )
        throw ( com::sun::star::uno::RuntimeException );

    // XMultiServiceFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstance( const rtl::OUString & aServiceSpecifier )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // Non-Interface methods

private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments,
                                 bool bCheckArgs )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
    getConfigProvider();

    bool
    createConfigPath( const rtl::OUString & rInPath, rtl::OUString & rOutPath );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYDATASOURCE_HXX */

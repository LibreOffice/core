/*************************************************************************
 *
 *  $RCSfile: dp_backend.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:09:08 $
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

#if ! defined INCLUDED_DP_REGISTRY_H
#define INCLUDED_DP_REGISTRY_H

#include "dp_props.h"
#include "rtl/ref.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase4.hxx"
#include "tools/inetmime.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include <memory>
#include <hash_map>


namespace dp_registry
{
namespace backend
{

namespace css = ::com::sun::star;

class PackageRegistryBackend;

//==============================================================================
class Package : public ::cppu::ImplInheritanceHelper1<
    ::dp_misc::PropertyComponentBase, css::deployment::XPackage >
{
protected:
    ::rtl::Reference< PackageRegistryBackend > m_myBackend;

    enum {
        COMPOSITION,
//         DEPENDENT_URLS,
        DESCRIPTION,
        DISPLAY_NAME,
        IS_REGISTERED,
        NAME,
        MEDIA_TYPE,
        URL
    };
    ::std::auto_ptr< ::cppu::IPropertyArrayHelper > m_property_array_helper;
    bool m_package_composition;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any & rConvertedValue, css::uno::Any & rOldValue,
        sal_Int32 nHandle, css::uno::Any const & rValue )
        throw (css::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle, css::uno::Any const & rValue )
        throw (css::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any & rValue, sal_Int32 nHandle ) const;

    ::rtl::OUString m_url;
//     css::uno::Sequence< ::rtl::OUString > m_dependentURLs;
    ::rtl::OUString m_mediaType;
    ::rtl::OUString m_name;
    ::rtl::OUString m_displayName;
    ::rtl::OUString m_description;

    inline void check() const { check_undisposed(); }
    virtual void SAL_CALL disposing();

    /** guarded: */
    enum t_Registered { REG_VOID, REG_REGISTERED, REG_NOT_REGISTERED };
    virtual t_Registered getRegStatus(
        ::osl::ResettableMutexGuard & guard,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        = 0;
    virtual bool processPackage(
        bool register_package,
        ::osl::ResettableMutexGuard & guard,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        = 0;

    void fireIsRegistered( t_Registered newStatus, t_Registered oldStatus );
    void fireCurrentIsRegistered(
        t_Registered oldStatus,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv );

    virtual ~Package();
    inline Package(
        ::rtl::Reference< PackageRegistryBackend > const & myBackend,
        ::rtl::OUString const & url,
        ::rtl::OUString const & mediaType,
        ::rtl::OUString const & name,
        ::rtl::OUString const & displayName,
        ::rtl::OUString const & description,
        bool package_composition = false )
        : m_myBackend( myBackend ),
          m_package_composition( package_composition ),
          m_url( url ),
          m_mediaType( mediaType ),
          m_name( name ),
          m_displayName( displayName ),
          m_description( description )
        {}

public:
    virtual ::rtl::OUString toString() const;

    // XPackage
    virtual sal_Bool SAL_CALL registerPackage(
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL revokePackage(
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::uno::RuntimeException);
};

//==============================================================================
class PackageRegistryBackend : public ::cppu::ImplInheritanceHelper4<
    ::dp_misc::PropertyComponentBase,
    css::lang::XInitialization,
    css::lang::XServiceInfo,
    css::lang::XEventListener,
    css::deployment::XPackageRegistry >
{
    css::uno::Reference< css::uno::XComponentContext > m_xComponentContext;
    css::uno::Reference< css::deployment::XPackageRegistry > m_xPackageRegistry;
    ::rtl::OUString m_cache_path;

    ::rtl::OUString m_implName;
    css::uno::Sequence< ::rtl::OUString > m_supported_media_types;

    typedef ::std::hash_map<
        ::rtl::OUString, css::uno::WeakReference< css::deployment::XPackage >,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_bound;

    enum PropertyHandles { SUPPORTED_MEDIA_TYPES };
    ::std::auto_ptr< ::cppu::IPropertyArrayHelper > m_property_array_helper;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any & rConvertedValue, css::uno::Any & rOldValue,
        sal_Int32 nHandle, css::uno::Any const & rValue )
        throw (css::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle, css::uno::Any const & rValue )
        throw (css::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any & rValue, sal_Int32 nHandle ) const;

protected:
    virtual css::uno::Reference< css::deployment::XPackage > createPackage(
        ::rtl::OUString const & url,
        ::rtl::OUString const & mediaType, ::rtl::OUString const & subType,
        INetContentTypeParameterList const & params,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        = 0;

    inline void check() const { check_undisposed(); }
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryBackend();
    inline PackageRegistryBackend(
        css::uno::Reference< css::uno::XComponentContext > const &
        xComponentContext,
        ::rtl::OUString const & implName,
        css::uno::Sequence< ::rtl::OUString > const & supported_media_types )
        : m_xComponentContext( xComponentContext ),
          m_implName( implName ),
          m_supported_media_types( supported_media_types )
        {}

public:
    inline css::uno::Reference< css::uno::XComponentContext > const &
    getComponentContext() const { return m_xComponentContext; }

    inline css::uno::Reference< css::deployment::XPackageRegistry >
    const & getPackageRegistry() const { return m_xPackageRegistry; }

    inline ::rtl::OUString const & getCachePath() const
        { return m_cache_path; }
    inline bool transientMode() const
        { return m_cache_path.getLength() == 0; }

    void ensure_no_running_office() const;
    void ensure_persistentMode() const;

    void bind_error(
        ::rtl::OUString const & url, css::uno::Any const & cause,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv );

    virtual ::rtl::OUString toString() const;

    // XInitialization
    void SAL_CALL initialize( css::uno::Sequence< css::uno::Any > const & args )
        throw (css::uno::Exception);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
        ::rtl::OUString const & serviceName )
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & source )
        throw (css::uno::RuntimeException);

    // XPackageRegistry
    virtual sal_Bool SAL_CALL bindPackage(
        css::uno::Reference< css::deployment::XPackage > & xPackage,
        ::rtl::OUString const & url,
        ::rtl::OUString const & mediaType,
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::uno::RuntimeException);
//     virtual ::rtl::OUString SAL_CALL detectMediaType(
//         ::rtl::OUString const & url ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL repair(
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::uno::RuntimeException) { return true; }
};

}
}

#endif


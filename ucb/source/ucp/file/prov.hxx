/*************************************************************************
 *
 *  $RCSfile: prov.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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

#ifndef _PROV_HXX_
#define _PROV_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

// FileProvider



namespace fileaccess {

    // Forward declaration

    class BaseContent;
    class shell;

    class FileProvider:
        public cppu::OWeakObject,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::ucb::XContentProvider,
        public com::sun::star::ucb::XContentIdentifierFactory,
        public com::sun::star::beans::XPropertySet
    {
        friend class BaseContent;
    public:

        FileProvider( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );
        ~FileProvider();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        release(
            void )
            throw( com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
        getImplementationName(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        supportsService(
            const rtl::OUString& ServiceName )
            throw(com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( com::sun::star::uno::RuntimeException );


        static com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

#if SUPD > 583
        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        CreateInstance(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );
#else
        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateInstance(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );
#endif
        // XContentProvider
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( com::sun::star::ucb::IllegalIdentifierException,
                   com::sun::star::uno::RuntimeException);

        // XContentIdentifierFactory

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        createContentIdentifier(
            const rtl::OUString& ContentId )
            throw( com::sun::star::uno::RuntimeException );


        virtual sal_Int32 SAL_CALL
        compareContentIds(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id1,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id2 )
            throw( com::sun::star::uno::RuntimeException );

        // XProperySet

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        setPropertyValue(
            const rtl::OUString& aPropertyName,
            const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::PropertyVetoException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Any SAL_CALL
        getPropertyValue(
            const rtl::OUString& PropertyName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addPropertyChangeListener(
            const rtl::OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        removePropertyChangeListener(
            const rtl::OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addVetoableChangeListener(
            const rtl::OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeVetoableChangeListener(
            const rtl::OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

    private:
        // Members
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >  m_xMultiServiceFactory;

        void SAL_CALL initProperties( void );
        rtl::OUString m_HostName;
        sal_Int32     m_FileSystemNotation;
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >     m_xPropertySetInfo;

        shell*                                                                        m_pMyShell;
    };

}       // end namespace fileaccess

#endif


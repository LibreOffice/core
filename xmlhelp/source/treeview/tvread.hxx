/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

#include <vector>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include "com/sun/star/ucb/XSimpleFileAccess3.hpp"

namespace treeview {


    class ConfigData
    {
    public:
        enum {
            PRODUCTNAME, PRODUCTVERSION, VENDORNAME, VENDORVERSION,
            VENDORSHORT };
        ConfigData();
        int                    m_vAdd[5];
        rtl::OUString          m_vReplacement[5];
        rtl::OUString          prodName,prodVersion,vendName,vendVersion,vendShort;

        std::vector< sal_uInt64 >       vFileLen;
        std::vector< rtl::OUString >    vFileURL;
        rtl::OUString locale,system;
        rtl::OUString appendix;

        void SAL_CALL replaceName( rtl::OUString& oustring ) const;
    };


    class TVDom;
    class TVChildTarget;

    class TVBase
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::container::XNameAccess,
          public com::sun::star::container::XHierarchicalNameAccess,
          public com::sun::star::util::XChangesNotifier,
          public com::sun::star::lang::XComponent
    {
        friend class TVChildTarget;

    public:

        virtual ~TVBase() { }

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XNameAccess

        virtual com::sun::star::uno::Type SAL_CALL
        getElementType(  )
            throw( com::sun::star::uno::RuntimeException )
        {
            return getCppuVoidType();
        }


        virtual sal_Bool SAL_CALL hasElements()
            throw( com::sun::star::uno::RuntimeException )
        {
            return true;
        }

        // XChangesNotifier

        virtual void SAL_CALL
        addChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            // read only
            (void)aListener;
        }

        virtual void SAL_CALL
        removeChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            // read only
            (void)aListener;
        }


        // XComponent

        virtual void SAL_CALL dispose( )
            throw( com::sun::star::uno::RuntimeException )
        {
        }

        virtual void SAL_CALL addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException )
        {
            (void)aListener;
        }


        // Abstract functions
        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException) = 0;

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException ) = 0;

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException ) = 0;


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException ) = 0;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException ) = 0;

    }; // end class TVBase





    class TVRead
        : public TVBase
    {
        friend class TVChildTarget;

    public:
        TVRead( const ConfigData& configData,TVDom* tvDom = 0 );

        ~TVRead();

        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


    private:

        rtl::OUString                    Title;
        rtl::OUString                    TargetURL;
        rtl::Reference< TVChildTarget >  Children;

    };  // end class TVRead






    class TVChildTarget
        : public TVBase
    {
    public:

        TVChildTarget( const ConfigData& configData,TVDom* tvDom );

        TVChildTarget( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

        ~TVChildTarget();

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const rtl::OUString& aName )
            throw( com::sun::star::uno::RuntimeException );


    private:
        std::vector< rtl::Reference< TVRead > >   Elements;

        ConfigData init(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        getConfiguration(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext ) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >
        getHierAccess( const ::com::sun::star::uno::Reference<  ::com::sun::star::lang::XMultiServiceFactory >& rxProvider,
                       const char* file ) const;

      ::rtl::OUString
      getKey( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key ) const;

      sal_Bool
      getBooleanKey(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                    const char* key) const;

      void subst(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSgr,
            rtl::OUString& instpath ) const;

    bool SearchAndInsert(TVDom* p, TVDom* tvDom);

    void Check(TVDom* tvDom);

    };  // end class TVChildTarget


    enum IteratorState
    {
        USER_EXTENSIONS,
        SHARED_EXTENSIONS,
        BUNDLED_EXTENSIONS,
        END_REACHED
    };

    class ExtensionIteratorBase
    {
    public:
        ExtensionIteratorBase( const rtl::OUString& aLanguage );
        void init( void );

    private:
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetHelpPackageFromPackage
            ( const com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
              com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );

    protected:
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextUserHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextSharedHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextBundledHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );

        void implGetLanguageVectorFromPackage( ::std::vector< ::rtl::OUString > &rv,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

        osl::Mutex                                                                  m_aMutex;
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    m_xContext;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess3 >   m_xSFA;

        IteratorState                                                               m_eState;
        rtl::OUString                                                               m_aLanguage;

        com::sun::star::uno::Sequence< com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > >                              m_aUserPackagesSeq;
        bool                                                                        m_bUserPackagesLoaded;

        com::sun::star::uno::Sequence< com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > >                              m_aSharedPackagesSeq;
        bool                                                                        m_bSharedPackagesLoaded;

        com::sun::star::uno::Sequence< com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > >                              m_aBundledPackagesSeq;
        bool                                                                        m_bBundledPackagesLoaded;

        int                                                                         m_iUserPackage;
        int                                                                         m_iSharedPackage;
        int                                                                         m_iBundledPackage;

    }; // end class ExtensionIteratorBase


    //===================================================================
    class TreeFileIterator : public ExtensionIteratorBase
    {
    public:
        TreeFileIterator( const rtl::OUString& aLanguage )
            : ExtensionIteratorBase( aLanguage )
        {}

        rtl::OUString nextTreeFile( sal_Int32& rnFileSize );

    private:
        rtl::OUString expandURL( const rtl::OUString& aURL );
        rtl::OUString implGetTreeFileFromPackage( sal_Int32& rnFileSize,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

    }; // end class TreeFileIterator


}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

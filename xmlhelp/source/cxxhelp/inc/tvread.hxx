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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_TVREAD_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_TVREAD_HXX

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
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <cppuhelper/implbase.hxx>

namespace treeview {

    class ConfigData
    {
    public:
        enum {
            PRODUCTNAME, PRODUCTVERSION, VENDORNAME, VENDORVERSION,
            VENDORSHORT };
        ConfigData();
        int                    m_vAdd[5];
        OUString          m_vReplacement[5];
        OUString          prodName,prodVersion,vendName,vendVersion,vendShort;

        std::vector< sal_uInt64 >       vFileLen;
        std::vector< OUString >    vFileURL;
        OUString locale,system;
        OUString appendix;

        void SAL_CALL replaceName( OUString& oustring ) const;
    };

    class TVDom;
    class TVChildTarget;

    class TVBase : public cppu::WeakImplHelper <
        css::container::XNameAccess,
        css::container::XHierarchicalNameAccess,
        css::util::XChangesNotifier,
        css::lang::XComponent >
    {
        friend class TVChildTarget;

    public:

        virtual ~TVBase() { }

        // XNameAccess

        virtual com::sun::star::uno::Type SAL_CALL
        getElementType(  )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            return cppu::UnoType<void>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            return true;
        }

        // XChangesNotifier

        virtual void SAL_CALL
        addChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            // read only
            (void)aListener;
        }

        virtual void SAL_CALL
        removeChangesListener(
            const com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >& aListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            // read only
            (void)aListener;
        }

        // XComponent

        virtual void SAL_CALL dispose( )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
        }

        virtual void SAL_CALL addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            (void)aListener;
        }

        // Abstract functions
        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE = 0;

    }; // end class TVBase

    class TVRead
        : public TVBase
    {
        friend class TVChildTarget;

    public:
        TVRead( const ConfigData& configData,TVDom* tvDom = 0 );

        virtual ~TVRead();

        // XNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    private:

        OUString                    Title;
        OUString                    TargetURL;
        rtl::Reference< TVChildTarget >  Children;

    };  // end class TVRead

    class TVChildTarget
        : public TVBase
    {
    public:

        TVChildTarget( const ConfigData& configData,TVDom* tvDom );

        TVChildTarget( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );

        virtual ~TVChildTarget();

        virtual com::sun::star::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XHierarchicalNameAccess

        virtual com::sun::star::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( com::sun::star::container::NoSuchElementException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    private:
        std::vector< rtl::Reference< TVRead > >   Elements;

        static ConfigData init(
            const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        getConfiguration(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        static ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >
        getHierAccess( const ::com::sun::star::uno::Reference<  ::com::sun::star::lang::XMultiServiceFactory >& rxProvider,
                       const char* file );

        static OUString
        getKey( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key );

        static bool
        getBooleanKey(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                    const char* key);

        static void subst( OUString& instpath );

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
        ExtensionIteratorBase( const OUString& aLanguage );
        void init();

    private:
        static com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetHelpPackageFromPackage
            ( const com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
              com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );

    protected:
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextUserHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextSharedHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );
        com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > implGetNextBundledHelpPackage
            ( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );

        void implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

        osl::Mutex                                                                  m_aMutex;
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    m_xContext;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess3 >   m_xSFA;

        IteratorState                                                               m_eState;
        OUString                                                               m_aLanguage;

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

    class TreeFileIterator : public ExtensionIteratorBase
    {
    public:
        TreeFileIterator( const OUString& aLanguage )
            : ExtensionIteratorBase( aLanguage )
        {}

        OUString nextTreeFile( sal_Int32& rnFileSize );

    private:
        OUString expandURL( const OUString& aURL );
        OUString implGetTreeFileFromPackage( sal_Int32& rnFileSize,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

    }; // end class TreeFileIterator

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

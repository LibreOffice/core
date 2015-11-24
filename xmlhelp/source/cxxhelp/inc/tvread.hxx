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

        virtual css::uno::Type SAL_CALL
        getElementType(  )
            throw( css::uno::RuntimeException, std::exception ) override
        {
            return cppu::UnoType<void>::get();
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw( css::uno::RuntimeException, std::exception ) override
        {
            return true;
        }

        // XChangesNotifier

        virtual void SAL_CALL
        addChangesListener(
            const css::uno::Reference< css::util::XChangesListener >& aListener )
            throw( css::uno::RuntimeException, std::exception ) override
        {
            // read only
            (void)aListener;
        }

        virtual void SAL_CALL
        removeChangesListener(
            const css::uno::Reference< css::util::XChangesListener >& aListener )
            throw( css::uno::RuntimeException, std::exception ) override
        {
            // read only
            (void)aListener;
        }

        // XComponent

        virtual void SAL_CALL dispose( )
            throw( css::uno::RuntimeException, std::exception ) override
        {
        }

        virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
            throw( css::uno::RuntimeException, std::exception ) override
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& aListener )
            throw( css::uno::RuntimeException, std::exception ) override
        {
            (void)aListener;
        }

        // Abstract functions
        // XNameAccess

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override = 0;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( css::uno::RuntimeException, std::exception ) override = 0;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override = 0;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::uno::RuntimeException, std::exception ) override = 0;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override = 0;

    }; // end class TVBase

    class TVRead
        : public TVBase
    {
        friend class TVChildTarget;

    public:
        TVRead( const ConfigData& configData,TVDom* tvDom = nullptr );

        virtual ~TVRead();

        // XNameAccess

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override;

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

        TVChildTarget( const css::uno::Reference< css::uno::XComponentContext >& xContext );

        virtual ~TVChildTarget();

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName )
            throw( css::container::NoSuchElementException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName )
            throw( css::uno::RuntimeException, std::exception ) override;

    private:
        std::vector< rtl::Reference< TVRead > >   Elements;

        static ConfigData init(
            const css::uno::Reference< css::uno::XComponentContext >& xContext );

        static css::uno::Reference< css::lang::XMultiServiceFactory >
        getConfiguration(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        static css::uno::Reference< css::container::XHierarchicalNameAccess >
        getHierAccess( const css::uno::Reference<  css::lang::XMultiServiceFactory >& rxProvider,
                       const char* file );

        static OUString
        getKey( const css::uno::Reference< css::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key );

        static bool
        getBooleanKey(
                    const css::uno::Reference< css::container::XHierarchicalNameAccess >& xHierAccess,
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
        static css::uno::Reference< css::deployment::XPackage > implGetHelpPackageFromPackage
            ( const css::uno::Reference< css::deployment::XPackage > xPackage,
              css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );

    protected:
        css::uno::Reference< css::deployment::XPackage > implGetNextUserHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextSharedHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextBundledHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );

        void implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
            css::uno::Reference< css::deployment::XPackage > xPackage );

        osl::Mutex                                                                  m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::Reference< css::ucb::XSimpleFileAccess3 >   m_xSFA;

        IteratorState                                                               m_eState;
        OUString                                                               m_aLanguage;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                              m_aUserPackagesSeq;
        bool                                                                        m_bUserPackagesLoaded;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                              m_aSharedPackagesSeq;
        bool                                                                        m_bSharedPackagesLoaded;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                              m_aBundledPackagesSeq;
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
            css::uno::Reference< css::deployment::XPackage > xPackage );

    }; // end class TreeFileIterator

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

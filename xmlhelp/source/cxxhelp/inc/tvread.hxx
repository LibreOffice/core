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

#include <vector>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

namespace treeview {

    class ConfigData
    {
    public:
        enum {
            PRODUCTNAME, PRODUCTVERSION, VENDORNAME, VENDORVERSION,
            VENDORSHORT };
        ConfigData();
        int                    m_vAdd[5] = {};
        OUString          m_vReplacement[5];

        std::vector< sal_uInt64 >       vFileLen;
        std::vector< OUString >    vFileURL;
        OUString locale,system;
        OUString appendix;

        void replaceName( OUString& oustring ) const;
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

        // XNameAccess

        virtual css::uno::Type SAL_CALL
        getElementType(  ) override
        {
            return cppu::UnoType<void>::get();
        }

        virtual sal_Bool SAL_CALL hasElements() override
        {
            return true;
        }

        // XChangesNotifier

        virtual void SAL_CALL
        addChangesListener(
            const css::uno::Reference< css::util::XChangesListener >& ) override
        {
            // read only
        }

        virtual void SAL_CALL
        removeChangesListener(
            const css::uno::Reference< css::util::XChangesListener >& ) override
        {
            // read only
        }

        // XComponent

        virtual void SAL_CALL dispose( ) override
        {
        }

        virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& ) override
        {}

        virtual void SAL_CALL
        removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& ) override
        {}

        // Abstract functions
        // XNameAccess

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName ) override = 0;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( ) override = 0;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName ) override = 0;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName ) override = 0;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName ) override = 0;

    }; // end class TVBase

    class TVRead final
        : public TVBase
    {
        friend class TVChildTarget;

    public:
        TVRead( const ConfigData& configData, TVDom* tvDom );

        virtual ~TVRead() override;

        // XNameAccess

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( ) override;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName ) override;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName ) override;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName ) override;

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

        virtual ~TVChildTarget() override;

        virtual css::uno::Any SAL_CALL
        getByName( const OUString& aName ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames( ) override;

        virtual sal_Bool SAL_CALL
        hasByName( const OUString& aName ) override;

        // XHierarchicalNameAccess

        virtual css::uno::Any SAL_CALL
        getByHierarchicalName( const OUString& aName ) override;

        virtual sal_Bool SAL_CALL
        hasByHierarchicalName( const OUString& aName ) override;

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

        std::unique_ptr<TVDom> SearchAndInsert(std::unique_ptr<TVDom> p, TVDom* tvDom);

        void Check(TVDom* tvDom);

    };  // end class TVChildTarget

    enum class IteratorState
    {
        UserExtensions,
        SharedExtensions,
        BundledExtensions,
        EndReached
    };

    class TreeFileIterator
    {
    public:
        TreeFileIterator( const OUString& aLanguage );
        OUString nextTreeFile( sal_Int32& rnFileSize );

    private:
        static css::uno::Reference< css::deployment::XPackage > implGetHelpPackageFromPackage
            ( const css::uno::Reference< css::deployment::XPackage >& xPackage,
              css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );

        css::uno::Reference< css::deployment::XPackage > implGetNextUserHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextSharedHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextBundledHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );

        void implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

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

        OUString expandURL( const OUString& aURL );
        OUString implGetTreeFileFromPackage( sal_Int32& rnFileSize,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

    }; // end class TreeFileIterator

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

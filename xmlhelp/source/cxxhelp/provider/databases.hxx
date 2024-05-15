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

#include <sal/config.h>

#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>

// Forward declaration

namespace helpdatafileproxy {

    class Hdf;

}

namespace chelp {

    class Databases;
    class URLParameter;

    class StaticModuleInformation
    {
    private:

        OUString m_aStartId;
        OUString m_aProgramSwitch;
        OUString m_aTitle;
        int m_nOrder;

    public:

        StaticModuleInformation( OUString aTitle,
                                 OUString aStartId,
                                 OUString aProgramSwitch,
                                 std::u16string_view aOrder )
            : m_aStartId(std::move( aStartId )),
              m_aProgramSwitch(std::move( aProgramSwitch )),
              m_aTitle(std::move( aTitle )),
              m_nOrder( o3tl::toInt32(aOrder) )
        {
        }

        const OUString& get_title() const { return m_aTitle; }
        const OUString& get_id() const { return m_aStartId; }
        const OUString& get_program() const { return m_aProgramSwitch; }
        int get_order() const { return m_nOrder; }
    };  // end class StaticModuleInformation

    class KeywordInfo
    {
    public:

        class KeywordElement
        {
            friend struct KeywordElementComparator;
            friend class KeywordInfo;

        public:

            KeywordElement( Databases const * pDatabases,
                            helpdatafileproxy::Hdf* pHdf,
                            OUString  key,
                            std::u16string_view ids );

        private:

            OUString key;
            css::uno::Sequence< OUString > listId;
            css::uno::Sequence< OUString > listAnchor;
            css::uno::Sequence< OUString > listTitle;

            void init( Databases const *pDatabases,helpdatafileproxy::Hdf* pHdf, std::u16string_view ids );
        };

        explicit KeywordInfo( const std::vector< KeywordElement >& aVector );

        css::uno::Sequence< OUString >&
        getKeywordList() { return listKey; }

        css::uno::Sequence< css::uno::Sequence< OUString > >&
        getIdList() { return listId; }

        css::uno::Sequence< css::uno::Sequence< OUString > >&
        getAnchorList() { return listAnchor; }

        css::uno::Sequence< css::uno::Sequence< OUString > >&
        getTitleList() { return listTitle; }

    private:

        css::uno::Sequence< OUString > listKey;
        css::uno::Sequence< css::uno::Sequence< OUString > > listId,listAnchor,listTitle;
    };  // end class KeywordInfo

    class Databases
    {
    public:

        /**
         *  Input is the installdirectory in system dependent notation
         */

        Databases( bool showBasic,
                 const OUString& instPath,
                 const OUString& productName,
                 const OUString& productVersion,
                 const OUString& styleSheet,
                 css::uno::Reference< css::uno::XComponentContext > const & xContext );

        ~Databases();

        static OString getImageTheme();

        OUString getInstallPathAsURL();
        OUString getInstallPathAsURL(std::unique_lock<std::mutex>& rGuard);

        const std::vector< OUString >& getModuleList( const OUString& Language );

        StaticModuleInformation* getStaticInformationForModule( std::u16string_view Module,
                                                                const OUString& Language );

        bool checkModuleMatchForExtension( std::u16string_view Database, const OUString& doclist );
        KeywordInfo* getKeyword( const OUString& Module,
                                 const OUString& Language );

        helpdatafileproxy::Hdf* getHelpDataFile( std::u16string_view Module,
                         const OUString& Language, bool helpText = false,
                         const OUString* pExtensionPath = nullptr );
        helpdatafileproxy::Hdf* getHelpDataFile(std::unique_lock<std::mutex>& rGuard,
                         std::u16string_view Module,
                         const OUString& Language, bool helpText = false,
                         const OUString* pExtensionPath = nullptr );


        /**
         *  The following method returns the Collator for the given language-country combination
         */
        css::uno::Reference< css::i18n::XCollator >
        getCollator(std::unique_lock<std::mutex>& rGuard, const OUString& Language);

        /**
         *  Returns the cascading style sheet used to format the HTML-output.
         *  First try is language directory, second try is main installation directory.
         */

        void cascadingStylesheet( const OUString& Language,
                                  OStringBuffer& buffer );

        /**
         *  Changes the stylesheet for further reads.
         */

        void changeCSS(const OUString& newStyleSheet);

        /**
         *  Returns the active help text for the given module, language and id.
         */

        void setActiveText( const OUString& Module,
                            const OUString& Language,
                            std::u16string_view Id,
                            OStringBuffer& buffer );

        /**
         *  Has the purpose of forcing the jarfile to stay open
         */

        css::uno::Reference< css::container::XHierarchicalNameAccess >
        jarFile(std::unique_lock<std::mutex>& rGuard, std::u16string_view jar,
                 const OUString& Language );

        css::uno::Reference< css::container::XHierarchicalNameAccess >
        findJarFileForPath( const OUString& jar, const OUString& Language,
            const OUString& path, OUString* o_pExtensionPath = nullptr,
            OUString* o_pExtensionRegistryPath = nullptr );

        /**
         *  Maps a given language-locale combination to language or locale.
         */
        OUString processLang( const OUString& Language );
        OUString processLang( std::unique_lock<std::mutex>& rGuard, const OUString& Language );

        void replaceName( OUString& oustring ) const;

        const OUString& getProductName() const { return m_vReplacement[0]; }
        const OUString& getProductVersion() const { return m_vReplacement[1]; }

        OUString expandURL( const OUString& aURL );
        OUString expandURL( std::unique_lock<std::mutex>& rGuard, const OUString& aURL );

        static OUString expandURL( const OUString& aURL,
            const css::uno::Reference< css::uno::XComponentContext >& xContext );

    private:

        std::mutex                                               m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >       m_xContext;
        css::uno::Reference< css::lang::XMultiComponentFactory > m_xSMgr;
        css::uno::Reference< css::ucb::XSimpleFileAccess3 >      m_xSFA;

        bool   m_bShowBasic;

        std::vector<char> m_vCustomCSSDoc;
        OUString m_aCSS;

        enum {
            PRODUCTNAME = 0,
            PRODUCTVERSION,
            VENDORNAME,
            VENDORVERSION,
            VENDORSHORT,
            NEWPRODUCTNAME,
            NEWPRODUCTVERSION
        };

        int                    m_vAdd[7];
        OUString          m_vReplacement[7];

        OUString          m_aInstallDirectory;                // Installation directory

        std::vector< OUString >    m_avModules;

        typedef std::unordered_map< OUString, std::unique_ptr<helpdatafileproxy::Hdf> >   DatabasesTable;
        DatabasesTable m_aDatabases;         // Language and module dependent databases

        typedef std::unordered_map< OUString,OUString > LangSetTable;
        LangSetTable m_aLangSet;   // Mapping to of lang-country to lang

        typedef std::unordered_map< OUString, std::unique_ptr<StaticModuleInformation> > ModInfoTable;
        ModInfoTable m_aModInfo;   // Module information

        typedef std::unordered_map< OUString, std::unique_ptr<KeywordInfo> > KeywordInfoTable;
        KeywordInfoTable m_aKeywordInfo;   // Module information

        typedef
        std::unordered_map<
             OUString,
             css::uno::Reference< css::container::XHierarchicalNameAccess > > ZipFileTable;
        ZipFileTable m_aZipFileTable;   // No closing of an once opened jarfile

        typedef
        std::unordered_map<
             OUString,
             css::uno::Reference< css::i18n::XCollator > > CollatorTable;
        CollatorTable    m_aCollatorTable;


        typedef
        std::unordered_set<
            OString >       EmptyActiveTextSet;
        EmptyActiveTextSet  m_aEmptyActiveTextSet;

        // methods

        void setInstallPath( const OUString& aInstallDirectory );

    }; // end class Databases

    enum class IteratorState
    {
        InitialModule,
        //SHARED_MODULE,        // Later, avoids redundancies in help compiling
        UserExtensions,
        SharedExtensions,
        BundledExtensions,
        EndReached
    };

    // Hashtable to cache extension help status
    typedef std::unordered_map
    <
        OUString,
        bool
    >
    ExtensionHelpExistenceMap;

    class ExtensionIteratorBase
    {
        static ExtensionHelpExistenceMap    aHelpExistenceMap;

    public:
        ExtensionIteratorBase( css::uno::Reference< css::uno::XComponentContext > const & xContext,
            Databases& rDatabases, OUString aInitialModule, OUString aLanguage );
        ExtensionIteratorBase( Databases& rDatabases, OUString  aInitialModule,
            OUString  aLanguage );
        void init();

    private:
        static css::uno::Reference< css::deployment::XPackage > implGetHelpPackageFromPackage
            ( const css::uno::Reference< css::deployment::XPackage >& xPackage,
              css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );

    protected:
        css::uno::Reference< css::deployment::XPackage > implGetNextUserHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextSharedHelpPackage
            ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        css::uno::Reference< css::deployment::XPackage > implGetNextBundledHelpPackage
        ( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle );
        OUString implGetFileFromPackage( std::u16string_view rFileExtension,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );
        OUString implGetFileFromPackage(std::unique_lock<std::mutex>& rGuard,
            std::u16string_view rFileExtension,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );
        void implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::Reference< css::ucb::XSimpleFileAccess3 >   m_xSFA;
        Databases&                                            m_rDatabases;

        IteratorState                                         m_eState;

        OUString                                              m_aInitialModule;
        OUString                                              m_aLanguage;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                   m_aUserPackagesSeq;
        bool                                                  m_bUserPackagesLoaded;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                   m_aSharedPackagesSeq;
        bool                                                  m_bSharedPackagesLoaded;

        css::uno::Sequence< css::uno::Reference
            < css::deployment::XPackage > >                   m_aBundledPackagesSeq;
        bool                                                  m_bBundledPackagesLoaded;

        int                                                   m_iUserPackage;
        int                                                   m_iSharedPackage;
        int                                                   m_iBundledPackage;

    }; // end class ExtensionIteratorBase

    class DataBaseIterator : public ExtensionIteratorBase
    {
    public:
        DataBaseIterator( css::uno::Reference< css::uno::XComponentContext > const & xContext,
            Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage, bool bHelpText )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
                , m_bHelpText( bHelpText )
        {}
        DataBaseIterator( Databases& rDatabases, const OUString& aInitialModule,
            const OUString& aLanguage, bool bHelpText )
                : ExtensionIteratorBase( rDatabases, aInitialModule, aLanguage )
                , m_bHelpText( bHelpText )
        {}

        helpdatafileproxy::Hdf* nextHdf( OUString* o_pExtensionPath = nullptr, OUString* o_pExtensionRegistryPath = nullptr );

    private:
        helpdatafileproxy::Hdf* implGetHdfFromPackage(
            const css::uno::Reference< css::deployment::XPackage >& xPackage,
            OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath );

        bool                                                                        m_bHelpText;

    }; // end class DataBaseIterator

    class KeyDataBaseFileIterator : public ExtensionIteratorBase
    {
    public:
        KeyDataBaseFileIterator( css::uno::Reference< css::uno::XComponentContext > const & xContext,
            Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
        {}
        //Returns a file URL
        OUString nextDbFile(std::unique_lock<std::mutex>& rGuard, bool& o_rbExtension);

    private:
        OUString implGetDbFileFromPackage(std::unique_lock<std::mutex>& rGuard,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

    }; // end class KeyDataBaseFileIterator

    class JarFileIterator : public ExtensionIteratorBase
    {
    public:
        JarFileIterator( css::uno::Reference< css::uno::XComponentContext > const & xContext,
            Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
        {}

        css::uno::Reference< css::container::XHierarchicalNameAccess >
            nextJarFile(std::unique_lock<std::mutex>& rGuard,
                css::uno::Reference<css::deployment::XPackage>& o_xParentPackageBundle,
                            OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath );

    private:
        css::uno::Reference< css::container::XHierarchicalNameAccess >
            implGetJarFromPackage(std::unique_lock<std::mutex>& rGuard,
                const css::uno::Reference< css::deployment::XPackage >& xPackage,
                OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath );

    }; // end class JarFileIterator

    class IndexFolderIterator : public ExtensionIteratorBase
    {
    public:
        IndexFolderIterator( Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage )
            : ExtensionIteratorBase( rDatabases, aInitialModule, aLanguage )
        {}

        OUString nextIndexFolder( bool& o_rbExtension, bool& o_rbTemporary );
        void deleteTempIndexFolder( std::u16string_view aIndexFolder );

    private:
        OUString implGetIndexFolderFromPackage( bool& o_rbTemporary,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

    }; // end class KeyDataBaseFileIterator

}      // end namespace chelp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

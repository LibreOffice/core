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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_DATABASES_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_DATABASES_HXX

#include <sal/config.h>

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
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

        StaticModuleInformation( const OUString& aTitle,
                                 const OUString& aStartId,
                                 const OUString& aProgramSwitch,
                                 const OUString& aOrder )
            : m_aStartId( aStartId ),
              m_aProgramSwitch( aProgramSwitch ),
              m_aTitle( aTitle ),
              m_nOrder( aOrder.toInt32() )
        {
        }

        ~StaticModuleInformation() { }

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

            KeywordElement( Databases* pDatabases,
                            helpdatafileproxy::Hdf* pHdf,
                            OUString& key,
                            OUString& ids );

        private:

            OUString key;
            css::uno::Sequence< OUString > listId;
            css::uno::Sequence< OUString > listAnchor;
            css::uno::Sequence< OUString > listTitle;

            void init( Databases *pDatabases,helpdatafileproxy::Hdf* pHdf,const OUString& ids );
        };

        explicit KeywordInfo( const std::vector< KeywordElement >& aVector );

        ~KeywordInfo() { };

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

        OString getImageTheme();

        OUString getInstallPathAsURL();

        const std::vector< OUString >& getModuleList( const OUString& Language );

        StaticModuleInformation* getStaticInformationForModule( const OUString& Module,
                                                                const OUString& Language );

        bool checkModuleMatchForExtension( const OUString& Database, const OUString& doclist );
        KeywordInfo* getKeyword( const OUString& Module,
                                 const OUString& Language );

        helpdatafileproxy::Hdf* getHelpDataFile( const OUString& Module,
                         const OUString& Language, bool helpText = false,
                         const OUString* pExtensionPath = nullptr );

        /**
         *  The following method returns the Collator for the given language-country combination
         */

        css::uno::Reference< css::i18n::XCollator >
        getCollator( const OUString& Language,
                     const OUString& System );   // System not used by current implementation
        //                                            // of XCollator

        /**
         *  Returns the cascading style sheet used to format the HTML-output.
         *  First try is language directory, second try is main installation directory.
         */

        void cascadingStylesheet( const OUString& Language,
                                  char** buffer,
                                  int* byteCount );

        /**
         *  Changes the stylesheet for further reads.
         */

        void changeCSS(const OUString& newStyleSheet);

        /**
         *  Returns the active help text for the given module, language and id.
         */

        void setActiveText( const OUString& Module,
                            const OUString& Language,
                            const OUString& Id,
                            char** buffer,
                            int* byteCount );

        /**
         *  Has the purpose of forcing the jarfile to stay open
         */

        css::uno::Reference< css::container::XHierarchicalNameAccess >
        jarFile( const OUString& jar,
                 const OUString& Language );

        css::uno::Reference< css::container::XHierarchicalNameAccess >
        findJarFileForPath( const OUString& jar, const OUString& Language,
            const OUString& path, OUString* o_pExtensionPath = nullptr,
            OUString* o_pExtensionRegistryPath = nullptr );

        /**
         *  Maps a given language-locale combination to language.
         */

        OUString processLang( const OUString& Language );

        /**
         *  Maps a given language-locale combination to locale.
         *  The returned string maybe empty
         */

        static OUString country( const OUString& Language );

        void replaceName( OUString& oustring ) const;

        const OUString& getProductName() const { return m_vReplacement[0]; }
        const OUString& getProductVersion() const { return m_vReplacement[1]; }

        OUString expandURL( const OUString& aURL );

        static OUString expandURL( const OUString& aURL,
            const css::uno::Reference< css::uno::XComponentContext >& xContext );

    private:

        osl::Mutex                                               m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >       m_xContext;
        css::uno::Reference< css::lang::XMultiComponentFactory > m_xSMgr;
        css::uno::Reference< css::ucb::XSimpleFileAccess3 >      m_xSFA;

        bool   m_bShowBasic;

        int    m_nCustomCSSDocLength;
        char*  m_pCustomCSSDoc;
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
        OUString          newProdName,newProdVersion,
            prodName,prodVersion,vendName,vendVersion,vendShort;

        OUString          m_aInstallDirectory;                // Installation directory

        std::vector< OUString >    m_avModules;

        typedef std::unordered_map< OUString,helpdatafileproxy::Hdf*,OUStringHash >   DatabasesTable;
        DatabasesTable m_aDatabases;         // Language and module dependent databases

        typedef std::unordered_map< OUString,OUString,OUStringHash > LangSetTable;
        LangSetTable m_aLangSet;   // Mapping to of lang-country to lang

        typedef std::unordered_map< OUString,StaticModuleInformation*,OUStringHash > ModInfoTable;
        ModInfoTable m_aModInfo;   // Module information

        typedef std::unordered_map< OUString,KeywordInfo*,OUStringHash > KeywordInfoTable;
        KeywordInfoTable m_aKeywordInfo;   // Module information

        typedef
        std::unordered_map<
             OUString,
             css::uno::Reference< css::container::XHierarchicalNameAccess >,
             OUStringHash >         ZipFileTable;
        ZipFileTable m_aZipFileTable;   // No closing of an once opened jarfile

        typedef
        std::unordered_map<
             OUString,
             css::uno::Reference< css::i18n::XCollator >,
             OUStringHash >      CollatorTable;
        CollatorTable    m_aCollatorTable;


        typedef
        std::unordered_set<
            OString,
            OStringHash >      EmptyActiveTextSet;
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
        bool,
        OUStringHash
    >
    ExtensionHelpExistanceMap;

    class ExtensionIteratorBase
    {
        static ExtensionHelpExistanceMap    aHelpExistanceMap;

    public:
        ExtensionIteratorBase( css::uno::Reference< css::uno::XComponentContext > const & xContext,
            Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage );
        ExtensionIteratorBase( Databases& rDatabases, const OUString& aInitialModule,
            const OUString& aLanguage );
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
        OUString implGetFileFromPackage( const OUString& rFileExtension,
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
        OUString nextDbFile( bool& o_rbExtension );

    private:
        OUString implGetDbFileFromPackage(
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
            nextJarFile( css::uno::Reference< css::deployment::XPackage >& o_xParentPackageBundle,
                            OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath );

    private:
        css::uno::Reference< css::container::XHierarchicalNameAccess >
            implGetJarFromPackage(const css::uno::Reference< css::deployment::XPackage >& xPackage,
                OUString* o_pExtensionPath = nullptr, OUString* o_pExtensionRegistryPath = nullptr );

    }; // end class JarFileIterator

    class IndexFolderIterator : public ExtensionIteratorBase
    {
    public:
        IndexFolderIterator( Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage )
            : ExtensionIteratorBase( rDatabases, aInitialModule, aLanguage )
        {}

        OUString nextIndexFolder( bool& o_rbExtension, bool& o_rbTemporary );
        void deleteTempIndexFolder( const OUString& aIndexFolder );

    private:
        OUString implGetIndexFolderFromPackage( bool& o_rbTemporary,
            const css::uno::Reference< css::deployment::XPackage >& xPackage );

    }; // end class KeyDataBaseFileIterator

}      // end namespace chelp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

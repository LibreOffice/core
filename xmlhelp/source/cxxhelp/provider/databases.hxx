/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DATABASES_HXX_
#define _DATABASES_HXX_

#include <set>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
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
#include "com/sun/star/ucb/XSimpleFileAccess2.hpp"

// Forward declaration


namespace berkeleydbproxy {

    class Db;

}


namespace chelp {


    class Databases;
    class URLParameter;

    class StaticModuleInformation
    {
    private:

        rtl::OUString m_aStartId;
        rtl::OUString m_aProgramSwitch;
        rtl::OUString m_aTitle;
        rtl::OUString m_aHeading;
        rtl::OUString m_aFulltext;
        int m_nOrder;


    public:

        StaticModuleInformation( rtl::OUString aTitle,
                                 rtl::OUString aStartId,
                                 rtl::OUString aProgramSwitch,
                                 rtl::OUString aHeading,
                                 rtl::OUString aFulltext,
                                 rtl::OUString aOrder )
            : m_aStartId( aStartId ),
              m_aProgramSwitch( aProgramSwitch ),
              m_aTitle( aTitle ),
              m_aHeading( aHeading ),
              m_aFulltext( aFulltext ),
              m_nOrder( aOrder.toInt32() )
        {
        }

        ~StaticModuleInformation() { }

        rtl::OUString get_title() const { return m_aTitle; }
        rtl::OUString get_id() const { return m_aStartId; }
        rtl::OUString get_program() const { return m_aProgramSwitch; }
        rtl::OUString get_heading() const { return m_aHeading; }
        rtl::OUString get_fulltext() const { return m_aFulltext; }
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
                            berkeleydbproxy::Db* pDb,
                            rtl::OUString& key,
                            rtl::OUString& ids );

        private:

            rtl::OUString key;
            com::sun::star::uno::Sequence< rtl::OUString > listId;
            com::sun::star::uno::Sequence< rtl::OUString > listAnchor;
            com::sun::star::uno::Sequence< rtl::OUString > listTitle;

            void init( Databases *pDatabases,berkeleydbproxy::Db* pDb,const rtl::OUString& ids );
        };


        KeywordInfo( const std::vector< KeywordElement >& aVector );

        ~KeywordInfo() { };

        com::sun::star::uno::Sequence< rtl::OUString >&
        getKeywordList() { return listKey; }

        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > >&
        getIdList() { return listId; }

        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > >&
        getAnchorList() { return listAnchor; }

        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > >&
        getTitleList() { return listTitle; }

    private:

        com::sun::star::uno::Sequence< rtl::OUString > listKey;
        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< rtl::OUString > > listId,listAnchor,listTitle;
    };  // end class KeywordInfo



    class Databases
    {
    public:

        struct eq
        {
            bool operator()( const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
            {
                return rKey1.compareTo( rKey2 ) == 0;
            }
        };

        struct ha
        {
            size_t operator()( const rtl::OUString& rName ) const
            {
                return rName.hashCode();
            }
        };


        /**
         *  Input is the installdirectory in system dependent notation
         */

        Databases( sal_Bool showBasic,
                 const rtl::OUString& instPath,
                 const com::sun::star::uno::Sequence< rtl::OUString >& imagesZipPaths,
                 const rtl::OUString& productName,
                 const rtl::OUString& productVersion,
                 const rtl::OUString& styleSheet,
                 com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext );

        ~Databases();

        rtl::OString getImagesZipFileURL();

        rtl::OUString getInstallPathAsURL();

        const std::vector< rtl::OUString >& getModuleList( const rtl::OUString& Language );

        StaticModuleInformation* getStaticInformationForModule( const rtl::OUString& Module,
                                                                const rtl::OUString& Language );

        bool checkModuleMatchForExtension( const rtl::OUString& Database, const rtl::OUString& doclist );
        KeywordInfo* getKeyword( const rtl::OUString& Module,
                                 const rtl::OUString& Language );

        berkeleydbproxy::Db* getBerkeley( const rtl::OUString& Module,
                         const rtl::OUString& Language, bool helpText = false,
                         const rtl::OUString* pExtensionPath = NULL );


        /**
         *  The following method returns the Collator for the given language-country combination
         */

        com::sun::star::uno::Reference< com::sun::star::i18n::XCollator >
        getCollator( const rtl::OUString& Language,
                     const rtl::OUString& System );   // System not used by current implementation
        //                                            // of XCollator

        /**
         *  Returns a copy of the popupfile
         */

        void popupDocument( URLParameter* urlPar,char **buffer,int *byteCount );


        /**
         *  Returns the cascading stlye sheet used to format the HTML-output.
         *  First try is language directory, second try is main installation directory.
         */

        void cascadingStylesheet( const rtl::OUString& Language,
                                  char** buffer,
                                  int* byteCount );


        /**
         *  Changes the the stylesheet for further reads.
         */

        void changeCSS(const rtl::OUString& newStyleSheet);


        /**
         *  Returns the active help text for the given module, language and id.
         */

        void setActiveText( const rtl::OUString& Module,
                            const rtl::OUString& Language,
                            const rtl::OUString& Id,
                            char** buffer,
                            int* byteCount );

        /**
         *  Has the purpose of forcing the the jarfile to stay open
         */

        com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >
        jarFile( const rtl::OUString& jar,
                 const rtl::OUString& Language );

        com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >
        findJarFileForPath( const rtl::OUString& jar, const rtl::OUString& Language,
            const rtl::OUString& path, rtl::OUString* o_pExtensionPath = NULL,
            rtl::OUString* o_pExtensionRegistryPath = NULL );

        /**
         *  Maps a given language-locale combination to language.
         */

        rtl::OUString processLang( const rtl::OUString& Language );


        /**
         *  Maps a given language-locale combination to locale.
         *  The returned string maybe empty
         */

        rtl::OUString country( const rtl::OUString& Language );


        void replaceName( rtl::OUString& oustring ) const;

        rtl::OUString getProductName() const { return m_vReplacement[0]; }
        rtl::OUString getProductVersion() const { return m_vReplacement[1]; }

        rtl::OUString expandURL( const rtl::OUString& aURL );

        static rtl::OUString expandURL( const rtl::OUString& aURL,
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext );

    private:

        osl::Mutex                                                                     m_aMutex;
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >       m_xContext;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiComponentFactory > m_xSMgr;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess2 >      m_xSFA;

        sal_Bool m_bShowBasic;
        char*  m_pErrorDoc;

        int    m_nCustomCSSDocLength;
        char*  m_pCustomCSSDoc;
        rtl::OUString m_aCSS;

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
        rtl::OUString          m_vReplacement[7];
        rtl::OUString          newProdName,newProdVersion,
            prodName,prodVersion,vendName,vendVersion,vendShort;

        rtl::OUString          m_aInstallDirectory;                // Installation directory
        com::sun::star::uno::Sequence< rtl::OUString > m_aImagesZipPaths;
        rtl::OString           m_aImagesZipFileURL;
        sal_Int16              m_nSymbolsStyle;

        std::vector< rtl::OUString >    m_avModules;

        typedef boost::unordered_map< rtl::OUString,berkeleydbproxy::Db*,ha,eq >   DatabasesTable;
        DatabasesTable m_aDatabases;         // Language and module dependent databases

        typedef  boost::unordered_map< rtl::OUString,rtl::OUString,ha,eq > LangSetTable;
        LangSetTable m_aLangSet;   // Mapping to of lang-country to lang

        typedef boost::unordered_map< rtl::OUString,StaticModuleInformation*,ha,eq > ModInfoTable;
        ModInfoTable m_aModInfo;   // Module information

        typedef boost::unordered_map< rtl::OUString,KeywordInfo*,ha,eq > KeywordInfoTable;
        KeywordInfoTable m_aKeywordInfo;   // Module information

        typedef
        boost::unordered_map<
        rtl::OUString,
             ::com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >,
            ha,
            eq >         ZipFileTable;
        ZipFileTable m_aZipFileTable;   // No closing of an once opened jarfile

        typedef
        boost::unordered_map<
        rtl::OUString,
             ::com::sun::star::uno::Reference< com::sun::star::i18n::XCollator >,
            ha,
            eq >      CollatorTable;
        CollatorTable    m_aCollatorTable;


        struct ostring_eq
        {
            bool operator()( const rtl::OString& rKey1, const rtl::OString& rKey2 ) const
            {
                return rKey1.compareTo( rKey2 ) == 0;
            }
        };

        struct ostring_ha
        {
            size_t operator()( const rtl::OString& rName ) const
            {
                return rName.hashCode();
            }
        };

        typedef
        boost::unordered_set<
            rtl::OString,
            ostring_ha,
            ostring_eq >      EmptyActiveTextSet;
        EmptyActiveTextSet  m_aEmptyActiveTextSet;


        // methods

        void setInstallPath( const rtl::OUString& aInstallDirectory );

    }; // end class Databases


    //===================================================================
    enum IteratorState
    {
        INITIAL_MODULE,
        //SHARED_MODULE,        // Later, avoids redundancies in help compiling
        USER_EXTENSIONS,
        SHARED_EXTENSIONS,
        BUNDLED_EXTENSIONS,
        END_REACHED
    };

    // Hashtable to cache extension help status
    typedef boost::unordered_map
    <
        ::rtl::OUString,
        bool,
        Databases::ha,
        Databases::eq
    >
    ExtensionHelpExistanceMap;


    class ExtensionIteratorBase
    {
        static ExtensionHelpExistanceMap    aHelpExistanceMap;

    public:
        ExtensionIteratorBase( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext,
            Databases& rDatabases, const rtl::OUString& aInitialModule, const rtl::OUString& aLanguage );
        ExtensionIteratorBase( Databases& rDatabases, const rtl::OUString& aInitialModule,
            const rtl::OUString& aLanguage );
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
        rtl::OUString implGetFileFromPackage( const rtl::OUString& rFileExtension,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );
        void implGetLanguageVectorFromPackage( ::std::vector< ::rtl::OUString > &rv,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    m_xContext;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess2 >   m_xSFA;
        Databases&                                                                  m_rDatabases;

        IteratorState                                                               m_eState;
        rtl::OUString                                                               m_aExtensionPath;

        rtl::OUString                                                               m_aInitialModule;
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
    class DataBaseIterator : public ExtensionIteratorBase
    {
    public:
        DataBaseIterator( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext,
            Databases& rDatabases, const rtl::OUString& aInitialModule, const rtl::OUString& aLanguage, bool bHelpText )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
                , m_bHelpText( bHelpText )
        {}
        DataBaseIterator( Databases& rDatabases, const rtl::OUString& aInitialModule,
            const rtl::OUString& aLanguage, bool bHelpText )
                : ExtensionIteratorBase( rDatabases, aInitialModule, aLanguage )
                , m_bHelpText( bHelpText )
        {}

        berkeleydbproxy::Db* nextDb( rtl::OUString* o_pExtensionPath = NULL, rtl::OUString* o_pExtensionRegistryPath = NULL );


    private:
        berkeleydbproxy::Db* implGetDbFromPackage(
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
            rtl::OUString* o_pExtensionPath, rtl::OUString* o_pExtensionRegistryPath );

        bool                                                                        m_bHelpText;

    }; // end class DataBaseIterator

    //===================================================================
    class KeyDataBaseFileIterator : public ExtensionIteratorBase
    {
    public:
        KeyDataBaseFileIterator( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext,
            Databases& rDatabases, const rtl::OUString& aInitialModule, const rtl::OUString& aLanguage )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
        {}
        //Returns a file URL
        rtl::OUString nextDbFile( bool& o_rbExtension );

    private:
        rtl::OUString implGetDbFileFromPackage(
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

    }; // end class KeyDataBaseFileIterator

    //===================================================================
    class JarFileIterator : public ExtensionIteratorBase
    {
    public:
        JarFileIterator( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext,
            Databases& rDatabases, const rtl::OUString& aInitialModule, const rtl::OUString& aLanguage )
                : ExtensionIteratorBase( xContext, rDatabases, aInitialModule, aLanguage )
        {}

        com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >
            nextJarFile( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle,
                            rtl::OUString* o_pExtensionPath = NULL, rtl::OUString* o_pExtensionRegistryPath = NULL );

    private:
        com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >
            implGetJarFromPackage(com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
                rtl::OUString* o_pExtensionPath = NULL, rtl::OUString* o_pExtensionRegistryPath = NULL );

    }; // end class JarFileIterator

    //===================================================================
    class IndexFolderIterator : public ExtensionIteratorBase
    {
    public:
        IndexFolderIterator( Databases& rDatabases, const rtl::OUString& aInitialModule, const rtl::OUString& aLanguage )
            : ExtensionIteratorBase( rDatabases, aInitialModule, aLanguage )
        {}

        rtl::OUString nextIndexFolder( bool& o_rbExtension, bool& o_rbTemporary );
        void deleteTempIndexFolder( const rtl::OUString& aIndexFolder );

    private:
        rtl::OUString implGetIndexFolderFromPackage( bool& o_rbTemporary,
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

    }; // end class KeyDataBaseFileIterator

    //===================================================================

}      // end namespace chelp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

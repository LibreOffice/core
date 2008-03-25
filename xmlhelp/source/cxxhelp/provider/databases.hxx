/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databases.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:21:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DATABASES_HXX_
#define _DATABASES_HXX_

#ifndef INCLUDED_STL_SET
#include <set>
#define INCLUDED_STL_SET
#endif
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include <hash_map>
#include <hash_set>
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif
#ifndef _COM_SUN_STAR_DEPLOYMENT_XPACKAGE_HPP_
#include <com/sun/star/deployment/XPackage.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include "com/sun/star/ucb/XSimpleFileAccess.hpp"
#endif

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

        rtl::OUString get_title()     { return m_aTitle; }
        rtl::OUString get_id()        { return m_aStartId; }
        rtl::OUString get_program()   { return m_aProgramSwitch; }
        rtl::OUString get_heading()   { return m_aHeading; }
        rtl::OUString get_fulltext()  { return m_aFulltext; }
        int get_order() { return m_nOrder; }
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
                 const rtl::OUString& vendorName,
                 const rtl::OUString& vendorVersion,
                 const rtl::OUString& vendorShort,
                 const rtl::OUString& styleSheet,
                 com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext );

        ~Databases();

        rtl::OString getImagesZipFileURL();

        rtl::OUString getInstallPathAsSystemPath();

        rtl::OUString getInstallPathAsURL();

        rtl::OUString getInstallPathAsURLWithOutEncoding();

        // access method to help files

        rtl::OUString getURLMode();

        const std::vector< rtl::OUString >& getModuleList( const rtl::OUString& Language );

        StaticModuleInformation* getStaticInformationForModule( const rtl::OUString& Module,
                                                                const rtl::OUString& Language );

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
         *  Returns a copy of the errordocument for the given language-locale combination
         */

        void errorDocument( const rtl::OUString& Language,
                            char** buffer,
                            int* byteCount );


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
        findJarFileForPath( const rtl::OUString& jar,
                 const rtl::OUString& Language, const rtl::OUString& path );

        /**
         *  Maps a given language-locale combination to language.
         */

        rtl::OUString lang( const rtl::OUString& Language );


        /**
         *  Maps a given language-locale combination to locale.
         *  The returned string maybe empty
         */

        rtl::OUString country( const rtl::OUString& Language );


        /**
         *  Maps a given System to the variant part of a locale.
         */

        rtl::OUString variant( const rtl::OUString& System );


        void replaceName( rtl::OUString& oustring ) const;

        rtl::OUString getProductName() const { return m_vReplacement[0]; }
        rtl::OUString getProductVersion() const { return m_vReplacement[1]; }
        rtl::OUString getVendorName() const { return m_vReplacement[2]; }
        rtl::OUString getVendorVersion() const { return m_vReplacement[3]; }
        rtl::OUString getVendorShort() const { return m_vReplacement[4]; }

        rtl::OUString expandURL( const rtl::OUString& aURL );

    private:

        osl::Mutex                                                                     m_aMutex;
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >       m_xContext;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiComponentFactory > m_xSMgr;

        sal_Bool m_bShowBasic;
        int    m_nErrorDocLength;
        char*  m_pErrorDoc;

        int    m_nCustomCSSDocLength;
        char*  m_pCustomCSSDoc;
        rtl::OUString m_aCSS;

#define PRODUCTNAME    0
#define PRODUCTVERSION 1
#define VENDORNAME     2
#define VENDORVERSION  3
#define VENDORSHORT    4
#define NEWPRODUCTNAME    5
#define NEWPRODUCTVERSION 6

        int                    m_vAdd[7];
        rtl::OUString          m_vReplacement[7];
        rtl::OUString          newProdName,newProdVersion,
            prodName,prodVersion,vendName,vendVersion,vendShort;

        rtl::OUString          m_aInstallDirectory;                // Installation directory
        com::sun::star::uno::Sequence< rtl::OUString > m_aImagesZipPaths;
        rtl::OString           m_aImagesZipFileURL;
        sal_Int16              m_nSymbolsStyle;
        rtl::OUString          m_aInstallDirectoryWithoutEncoding; // a work around for a Sablot bug.
        rtl::OUString          m_aInstallDirectoryAsSystemPath;    // Installation directory

        std::vector< rtl::OUString >    m_avModules;

        typedef std::hash_map< rtl::OUString,berkeleydbproxy::Db*,ha,eq >   DatabasesTable;
        DatabasesTable m_aDatabases;         // Language and module dependent databases

        typedef  std::hash_map< rtl::OUString,rtl::OUString,ha,eq > LangSetTable;
        LangSetTable m_aLangSet;   // Mapping to of lang-country to lang

        typedef std::hash_map< rtl::OUString,StaticModuleInformation*,ha,eq > ModInfoTable;
        ModInfoTable m_aModInfo;   // Module information

        typedef std::hash_map< rtl::OUString,KeywordInfo*,ha,eq > KeywordInfoTable;
        KeywordInfoTable m_aKeywordInfo;   // Module information

        typedef
        std::hash_map<
        rtl::OUString,
             ::com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >,
            ha,
            eq >         ZipFileTable;
        ZipFileTable m_aZipFileTable;   // No closing of an once opened jarfile

        typedef
        std::hash_map<
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
        std::hash_set<
            rtl::OString,
            ostring_ha,
            ostring_eq >      EmptyActiveTextSet;
        EmptyActiveTextSet  m_aEmptyActiveTextSet;


        // methods

        void setInstallPath( const rtl::OUString& aInstallDirectory );

        static void implCollectXhpFiles( const rtl::OUString& aDir,
            std::vector< rtl::OUString >& o_rXhpFileVector,
            com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess > xSFA );

    }; // end class Databases


    //===================================================================
    enum IteratorState
    {
        INITIAL_MODULE,
        //SHARED_MODULE,        // Later, avoids redundancies in help compiling
        USER_EXTENSIONS,
        SHARED_EXTENSIONS,
        END_REACHED
    };

    // Hashtable to cache extension help status
    typedef std::hash_map
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
        ExtensionIteratorBase( com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext,
            Databases& rDatabases );
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

        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    m_xContext;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess >    m_xSFA;
        Databases&                                                                  m_rDatabases;

        IteratorState                                                               m_eState;
        rtl::OUString                                                               m_aExtensionPath;

        rtl::OUString                                                               m_aInitialModule;
        rtl::OUString                                                               m_aLanguage;
        rtl::OUString                                                               m_aCorrectedLanguage;

        com::sun::star::uno::Sequence< com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > >                              m_aUserPackagesSeq;
        bool                                                                        m_bUserPackagesLoaded;

        com::sun::star::uno::Sequence< com::sun::star::uno::Reference
            < com::sun::star::deployment::XPackage > >                              m_aSharedPackagesSeq;
        bool                                                                        m_bSharedPackagesLoaded;

        int                                                                         m_iUserPackage;
        int                                                                         m_iSharedPackage;

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

        berkeleydbproxy::Db* nextDb( rtl::OUString* o_pExtensionPath = NULL );


    private:
        berkeleydbproxy::Db* implGetDbFromPackage(
            com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage,
            rtl::OUString* o_pExtensionPath );

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
            nextJarFile( com::sun::star::uno::Reference< com::sun::star::deployment::XPackage >& o_xParentPackageBundle );

    private:
        com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess >
            implGetJarFromPackage(com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage );

    }; // end class JarFileIterator

    //===================================================================

}      // end namespace chelp


#endif

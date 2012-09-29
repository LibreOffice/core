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

#include <svtools/templatefoldercache.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <ucbhelper/content.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include "comphelper/processfactory.hxx"

#include <vector>
#include <list>
#include <functional>
#include <algorithm>

//.........................................................................
namespace svt
{
//.........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::uno;

    //=====================================================================
    //= helpers
    //=====================================================================
    //---------------------------------------------------------------------
    SvStream& operator << ( SvStream& _rStorage, const util::DateTime& _rDate )
    {
        _rStorage << _rDate.HundredthSeconds;
        _rStorage << _rDate.Seconds;
        _rStorage << _rDate.Minutes;
        _rStorage << _rDate.Hours;
        _rStorage << _rDate.Day;
        _rStorage << _rDate.Month;
        _rStorage << _rDate.Year;

        return _rStorage;
    }

    //---------------------------------------------------------------------
    SvStream& operator >> ( SvStream& _rStorage, util::DateTime& _rDate )
    {
        _rStorage >> _rDate.HundredthSeconds;
        _rStorage >> _rDate.Seconds;
        _rStorage >> _rDate.Minutes;
        _rStorage >> _rDate.Hours;
        _rStorage >> _rDate.Day;
        _rStorage >> _rDate.Month;
        _rStorage >> _rDate.Year;

        return _rStorage;
    }

    //---------------------------------------------------------------------
    sal_Bool operator == ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return  _rLHS.HundredthSeconds == _rRHS.HundredthSeconds
            &&  _rLHS.Seconds   == _rRHS.Seconds
            &&  _rLHS.Minutes   == _rRHS.Minutes
            &&  _rLHS.Hours     == _rRHS.Hours
            &&  _rLHS.Day       == _rRHS.Day
            &&  _rLHS.Month     == _rRHS.Month
            &&  _rLHS.Year      == _rRHS.Year;
    }

    //---------------------------------------------------------------------
    sal_Bool operator != ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return !( _rLHS == _rRHS );
    }

    //=====================================================================
    //= TemplateContent
    //=====================================================================
    struct TemplateContent;
    typedef ::std::vector< ::rtl::Reference< TemplateContent > >    TemplateFolderContent;
    typedef TemplateFolderContent::const_iterator           ConstFolderIterator;
    typedef TemplateFolderContent::iterator                 FolderIterator;

    /** a struct describing one content in one of the template dirs (or at least it's relevant aspects)
    */
    struct TemplateContent : public ::salhelper::SimpleReferenceObject
    {
    public:

    private:
        INetURLObject           m_aURL;
        String                  m_sLocalName;       // redundant - last segment of m_aURL
        util::DateTime          m_aLastModified;    // date of last modification as reported by UCP
        TemplateFolderContent   m_aSubContents;     // sorted (by name) list of the children

    private:
        inline  void    implResetDate( )
        {
            m_aLastModified.HundredthSeconds = m_aLastModified.Seconds = m_aLastModified.Minutes = m_aLastModified.Hours = 0;
            m_aLastModified.Day = m_aLastModified.Month = m_aLastModified.Year = 0;
        }

    private:
        ~TemplateContent();

    public:
        TemplateContent( const INetURLObject& _rURL );

        // attribute access
        inline String                   getName( ) const                            { return m_sLocalName; }
        inline String                   getURL( ) const                             { return m_aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ); }
        inline void                     setModDate( const util::DateTime& _rDate )  { m_aLastModified = _rDate; }
        inline const util::DateTime&    getModDate( ) const                         { return m_aLastModified; }

        inline TemplateFolderContent&   getSubContents()            { return m_aSubContents; }
        inline const TemplateFolderContent& getSubContents() const  { return m_aSubContents; }

                inline ConstFolderIterator              begin() const   { return m_aSubContents.begin(); }
                inline ConstFolderIterator              end() const             { return m_aSubContents.end(); }
        inline TemplateFolderContent::size_type
                                        size() const    { return m_aSubContents.size(); }

        inline void                     push_back( const ::rtl::Reference< TemplateContent >& _rxNewElement )
                                                        { m_aSubContents.push_back( _rxNewElement ); }
    };

    //---------------------------------------------------------------------
    DBG_NAME( TemplateContent )

    //---------------------------------------------------------------------
    TemplateContent::TemplateContent( const INetURLObject& _rURL )
        :m_aURL( _rURL )
    {
        DBG_CTOR( TemplateContent, NULL );
        DBG_ASSERT( INET_PROT_NOT_VALID != m_aURL.GetProtocol(), "TemplateContent::TemplateContent: invalid URL!" );
        m_sLocalName = m_aURL.getName();
        implResetDate();
    }

    //---------------------------------------------------------------------
    TemplateContent::~TemplateContent()
    {
        DBG_DTOR( TemplateContent, NULL );
    }

    //=====================================================================
    //= stl helpers
    //=====================================================================
    //---------------------------------------------------------------------
    /// compares two TemplateContent by URL
    struct TemplateContentURLLess
        :public ::std::binary_function  <   ::rtl::Reference< TemplateContent >
                                        ,   ::rtl::Reference< TemplateContent >
                                        ,   bool
                                        >
    {
        bool operator() ( const ::rtl::Reference< TemplateContent >& _rxLHS, const ::rtl::Reference< TemplateContent >& _rxRHS ) const
        {
            return  _rxLHS->getURL() < _rxRHS->getURL()
                ?   true
                :   false;
        }
    };

    //---------------------------------------------------------------------
    /// sorts the sib contents of a TemplateFolderContent
    struct SubContentSort : public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
    {
        void operator() ( TemplateFolderContent& _rFolder ) const
        {
            // sort the directory by name
            ::std::sort(
                _rFolder.begin(),
                _rFolder.end(),
                TemplateContentURLLess()
            );

            // sort the sub directories by name
            ::std::for_each(
                _rFolder.begin(),
                _rFolder.end(),
                *this
            );
        }

        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.is() && _rxContent->size() )
            {
                operator()( _rxContent->getSubContents() );
            }
        }
    };
    //---------------------------------------------------------------------
    /** does a deep compare of two template contents
    */
    struct TemplateContentEqual
        :public ::std::binary_function  <   ::rtl::Reference< TemplateContent >
                                        ,   ::rtl::Reference< TemplateContent >
                                        ,   bool
                                        >
    {
        //.................................................................
        bool operator() (const ::rtl::Reference< TemplateContent >& _rLHS, const ::rtl::Reference< TemplateContent >& _rRHS )
        {
            if ( !_rLHS.is() || !_rRHS.is() )
            {
                OSL_FAIL( "TemplateContentEqual::operator(): invalid contents!" );
                return true;
                    // this is not strictly true, in case only one is invalid - but this is a heavy error anyway
            }

            if ( _rLHS->getURL() != _rRHS->getURL() )
                return false;

            if ( _rLHS->getModDate() != _rRHS->getModDate() )
                return false;

            if ( _rLHS->getSubContents().size() != _rRHS->getSubContents().size() )
                return false;

            if ( _rLHS->getSubContents().size() )
            {   // there are children
                // -> compare them
                ::std::pair< FolderIterator, FolderIterator > aFirstDifferent = ::std::mismatch(
                    _rLHS->getSubContents().begin(),
                    _rLHS->getSubContents().end(),
                    _rRHS->getSubContents().begin(),
                    *this
                );
                if ( aFirstDifferent.first != _rLHS->getSubContents().end() )
                    return false;// the sub contents differ
            }

            return true;
        }
    };

    //---------------------------------------------------------------------
    /// base class for functors which act an an SvStream
    struct StorageHelper
    {
    protected:
        SvStream&   m_rStorage;
        StorageHelper( SvStream& _rStorage ) : m_rStorage( _rStorage ) { }
    };

    //---------------------------------------------------------------------
    /// functor which allows storing a string
    struct StoreString
            :public ::std::unary_function< String, void >
            ,public StorageHelper
    {
        StoreString( SvStream& _rStorage ) : StorageHelper( _rStorage ) { }

        void operator() ( const String& _rString ) const
        {
            m_rStorage.WriteUniOrByteString( _rString, m_rStorage.GetStreamCharSet() );
        }
    };

    //---------------------------------------------------------------------
    /// functor which stores the local name of a TemplateContent
    struct StoreLocalContentName
            :public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
            ,public StoreString
    {
        StoreLocalContentName( SvStream& _rStorage ) : StoreString( _rStorage ) { }

        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            SAL_WARN( "svtools.misc", "This method must not be used, the whole URL must be stored!" );

            // use the base class operator with the local name of the content
            StoreString::operator() ( _rxContent->getName() );
        }
    };

    //---------------------------------------------------------------------
    struct StoreContentURL
            :public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
            ,public StoreString
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        StoreContentURL( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StoreString( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }

        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            // use the base class operator with the local name of the content
            String sURL = _rxContent->getURL();
            // #116281# Keep office installtion relocatable. Never store
            // any direct references to office installation directory.
            sURL = m_xOfficeInstDirs->makeRelocatableURL( sURL );
            StoreString::operator() ( sURL );
        }
    };

    //---------------------------------------------------------------------
    /// functor which stores the complete content of a TemplateContent
    struct StoreFolderContent
            :public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
            ,public StorageHelper
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

    public:
        StoreFolderContent( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StorageHelper( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }

        //.................................................................
        void operator() ( const TemplateContent& _rContent ) const
        {
            // store the info about this content
            m_rStorage << _rContent.getModDate();

            // store the info about the children
            // the number
            m_rStorage << (sal_Int32)_rContent.size();
            // their URLs ( the local name is not enough, since URL might be not a hierarchical one, "expand:" for example )
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                StoreContentURL( m_rStorage, m_xOfficeInstDirs )
            );
            // their content
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                *this
            );
        }

        //.................................................................
        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.is() )
            {
                operator()( *_rxContent );
            }
        }
    };

    //---------------------------------------------------------------------
    /// functor which reads a complete TemplateContent instance
    struct ReadFolderContent
            :public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
            ,public StorageHelper
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        ReadFolderContent( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StorageHelper( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }

        //.................................................................
        void operator() ( TemplateContent& _rContent ) const
        {
            // store the info about this content
            util::DateTime aModDate;
            m_rStorage >> aModDate;
            _rContent.setModDate( aModDate );

            // store the info about the children
            // the number
            sal_Int32 nChildren = 0;
            m_rStorage >> nChildren;
            TemplateFolderContent& rChildren = _rContent.getSubContents();
            rChildren.resize( 0 );
            rChildren.reserve( nChildren );
            // initialize them with their (local) names
            while ( nChildren-- )
            {
                String sURL = m_rStorage.ReadUniOrByteString(m_rStorage.GetStreamCharSet());
                sURL = m_xOfficeInstDirs->makeAbsoluteURL( sURL );
                INetURLObject aChildURL( sURL );
                rChildren.push_back( new TemplateContent( aChildURL ) );
            }

            // their content
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                *this
            );
        }

        //.................................................................
        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.is() )
            {
                operator()( *_rxContent );
            }
        }
    };

    //=====================================================================
    //= TemplateFolderCacheImpl
    //=====================================================================
    class TemplateFolderCacheImpl
    {
    private:
        TemplateFolderContent           m_aPreviousState;   // the current state of the template dirs (as found on the HD)
        TemplateFolderContent           m_aCurrentState;    // the previous state of the template dirs (as found in the cache file)

        osl::Mutex                      m_aMutex;
        // will be lazy inited; never access directly; use getOfficeInstDirs().
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        SvStream*                       m_pCacheStream;
        sal_Bool                        m_bNeedsUpdate : 1;
        sal_Bool                        m_bKnowState : 1;
        sal_Bool                        m_bValidCurrentState : 1;
        sal_Bool                        m_bAutoStoreState : 1;

    public:
        TemplateFolderCacheImpl( sal_Bool _bAutoStoreState );
        ~TemplateFolderCacheImpl( );

        sal_Bool    needsUpdate( sal_Bool _bForceCheck );
        void        storeState( sal_Bool _bForceRetrieval );

    private:
        sal_Bool    openCacheStream( sal_Bool _bForRead );
        void        closeCacheStream( );

        /// read the state of the dirs from the cache file
        sal_Bool    readPreviousState();
        /// read the current state of the dirs
        sal_Bool    readCurrentState();

        String      implParseSmart( const String& _rPath );

        sal_Bool    implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot );

        static  rtl::OUString getCacheFileName();
        static  sal_Int32   getMagicNumber();
        static  void        normalize( TemplateFolderContent& _rState );

        // @return <TRUE/> if the states equal
        static  sal_Bool    equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS );

        // late initialize m_xOfficeInstDirs
        uno::Reference< util::XOfficeInstallationDirectories > getOfficeInstDirs();
    };

    //---------------------------------------------------------------------
    TemplateFolderCacheImpl::TemplateFolderCacheImpl( sal_Bool _bAutoStoreState )
        :m_pCacheStream         ( NULL )
        ,m_bNeedsUpdate         ( sal_True )
        ,m_bKnowState           ( sal_False )
        ,m_bValidCurrentState   ( sal_False )
        ,m_bAutoStoreState      ( _bAutoStoreState )
    {
    }

    //---------------------------------------------------------------------
    TemplateFolderCacheImpl::~TemplateFolderCacheImpl( )
    {
        // store the current state if possible and required
        if ( m_bValidCurrentState && m_bAutoStoreState )
            storeState( sal_False );

        closeCacheStream( );
    }

    //---------------------------------------------------------------------
    sal_Int32 TemplateFolderCacheImpl::getMagicNumber()
    {
        sal_Int32 nMagic = 0;
        ( nMagic += (sal_Int8)'T' ) <<= 4;
        ( nMagic += (sal_Int8)'D' ) <<= 4;
        ( nMagic += (sal_Int8)'S' ) <<= 4;
        ( nMagic += (sal_Int8)'C' ) <<= 0;
        return nMagic;
    }

    //---------------------------------------------------------------------
    rtl::OUString TemplateFolderCacheImpl::getCacheFileName()
    {
        return rtl::OUString(".templdir.cache");
    }


    //---------------------------------------------------------------------
    void TemplateFolderCacheImpl::normalize( TemplateFolderContent& _rState )
    {
        SubContentSort()( _rState );
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS )
    {
        if ( _rLHS.size() != _rRHS.size() )
            return sal_False;

        // as both arrays are sorted (by definition - this is a precondition of this method)
        // we can simply go from the front to the back and compare the single elements

        ::std::pair< ConstFolderIterator, ConstFolderIterator > aFirstDifferent = ::std::mismatch(
            _rLHS.begin(),
            _rLHS.end(),
            _rRHS.begin(),
            TemplateContentEqual()
        );

        return aFirstDifferent.first == _rLHS.end();
    }

    //---------------------------------------------------------------------
    void TemplateFolderCacheImpl::storeState( sal_Bool _bForceRetrieval )
    {
        if ( !m_bValidCurrentState || _bForceRetrieval )
            readCurrentState( );

        if ( m_bValidCurrentState && openCacheStream( sal_False ) )
        {
            *m_pCacheStream << getMagicNumber();

            // store the template root folders
            // the size
            *m_pCacheStream << (sal_Int32)m_aCurrentState.size();
            // the complete URLs
            ::std::for_each(
                m_aCurrentState.begin(),
                m_aCurrentState.end(),
                StoreContentURL( *m_pCacheStream, getOfficeInstDirs() )
            );

            // the contents
            ::std::for_each(
                m_aCurrentState.begin(),
                m_aCurrentState.end(),
                StoreFolderContent( *m_pCacheStream, getOfficeInstDirs() )
            );
        }
    }

    //---------------------------------------------------------------------
    String TemplateFolderCacheImpl::implParseSmart( const String& _rPath )
    {
        INetURLObject aParser;
        aParser.SetSmartProtocol( INET_PROT_FILE );
        aParser.SetURL( _rPath, INetURLObject::WAS_ENCODED );
        if ( INET_PROT_NOT_VALID == aParser.GetProtocol() )
        {
            rtl::OUString sURL;
            LocalFileHelper::ConvertPhysicalNameToURL( _rPath, sURL );
            aParser.SetURL( sURL, INetURLObject::WAS_ENCODED );
        }
        return aParser.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }

    //---------------------------------------------------------------------
    void TemplateFolderCacheImpl::closeCacheStream( )
    {
        DELETEZ( m_pCacheStream );
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot )
    {
        try
        {
            // create a content for the current folder root
            Reference< XResultSet > xResultSet;
            Sequence< ::rtl::OUString > aContentProperties( 4);
            aContentProperties[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ));
            aContentProperties[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ));
            aContentProperties[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ));
            aContentProperties[3] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ));

            // get the set of sub contents in the folder
            try
            {
                Reference< XDynamicResultSet > xDynResultSet;

                ::ucbhelper::Content aTemplateRoot( _rxRoot->getURL(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                xDynResultSet = aTemplateRoot.createDynamicCursor( aContentProperties, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                SAL_WARN( "svtools.misc", "TemplateFolderCacheImpl::implReadFolder: caught a CommandAbortedException!" );
                return sal_False;
            }
            catch( ::com::sun::star::uno::Exception& )
            {
            }

            // collect the infos about the sub contents
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );
                Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );

                while ( xResultSet->next() )
                {
                    INetURLObject aSubContentURL( xContentAccess->queryContentIdentifierString() );

                    // a new content instance
                    ::rtl::Reference< TemplateContent > xChild = new TemplateContent( aSubContentURL );

                    // the modified date
                    xChild->setModDate( xRow->getTimestamp( 2 ) );  // date modified
                    if ( xRow->wasNull() )
                        xChild->setModDate( xRow->getTimestamp( 3 ) );  // fallback: date created

                    // push back this content
                    _rxRoot->push_back( xChild );

                    // is it a folder?
                    if ( xRow->getBoolean( 4 ) && !xRow->wasNull() )
                    {   // yes -> step down
                                                ConstFolderIterator aNextLevelRoot = _rxRoot->end();
                        --aNextLevelRoot;
                        implReadFolder( *aNextLevelRoot );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "TemplateFolderCacheImpl::implReadFolder: caught an exception!" );
            return sal_False;
        }
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::readCurrentState()
    {
        // reset
        m_bValidCurrentState = sal_False;
        TemplateFolderContent aTemplateFolderContent;
        m_aCurrentState.swap( aTemplateFolderContent );

        // the template directories from the config
        const SvtPathOptions aPathOptions;
        rtl::OUString aDirs = aPathOptions.GetTemplatePath();

        // loop through all the root-level template folders
        sal_Int32 nIndex = 0;
        do
        {
            String sTemplatePath( aDirs.getToken(0, ';', nIndex) );
            sTemplatePath = aPathOptions.ExpandMacros( sTemplatePath );

            // Make sure excess ".." path segments (from expanding bootstrap
            // variables in paths) are normalized in the same way they are
            // normalized for paths read from the .templdir.cache file (where
            // paths have gone through makeRelocatable URL on writing out and
            // then through makeAbsoluteURL when reading back in), as otherwise
            // equalStates() in needsUpdate() could erroneously consider
            // m_aCurrentState and m_aPreviousState as different:
            sTemplatePath = getOfficeInstDirs()->makeAbsoluteURL(
                getOfficeInstDirs()->makeRelocatableURL(sTemplatePath));

            // create a new entry
            m_aCurrentState.push_back( new TemplateContent( INetURLObject( sTemplatePath ) ) );
            TemplateFolderContent::iterator aCurrentRoot = m_aCurrentState.end();
            --aCurrentRoot;

            if ( !implReadFolder( *aCurrentRoot ) )
                return sal_False;
        }
        while ( nIndex >= 0 );

        // normalize the array (which basically means "sort it")
        normalize( m_aCurrentState );

        m_bValidCurrentState = sal_True;
        return m_bValidCurrentState;
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::readPreviousState()
    {
        DBG_ASSERT( m_pCacheStream, "TemplateFolderCacheImpl::readPreviousState: not to be called without stream!" );

        // reset
        TemplateFolderContent aTemplateFolderContent;
        m_aPreviousState.swap( aTemplateFolderContent );

        // check the magic number
        sal_Int32 nMagic = 0;
        *m_pCacheStream >> nMagic;
        DBG_ASSERT( getMagicNumber() == nMagic, "TemplateFolderCacheImpl::readPreviousState: invalid cache file!" );
        if ( getMagicNumber() != nMagic )
            return sal_False;

        // the root directories
        // their number
        sal_Int32 nRootDirectories = 0;
        *m_pCacheStream >> nRootDirectories;
        // init empty TemplateContens with the URLs
        m_aPreviousState.reserve( nRootDirectories );
        while ( nRootDirectories-- )
        {
            String sURL = m_pCacheStream->ReadUniOrByteString(m_pCacheStream->GetStreamCharSet());
            // #116281# Keep office installtion relocatable. Never store
            // any direct references to office installation directory.
            sURL = getOfficeInstDirs()->makeAbsoluteURL( sURL );
            m_aPreviousState.push_back(
                new TemplateContent( INetURLObject(sURL) ) );
        }

        // read the contents of the root folders
        ::std::for_each(
            m_aPreviousState.begin(),
            m_aPreviousState.end(),
            ReadFolderContent( *m_pCacheStream, getOfficeInstDirs() )
        );

        DBG_ASSERT( !m_pCacheStream->GetErrorCode(), "TemplateFolderCacheImpl::readPreviousState: unknown error during reading the state cache!" );

        // normalize the array (which basically means "sort it")
        normalize( m_aPreviousState );

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::openCacheStream( sal_Bool _bForRead )
    {
        // close any old stream instance
        closeCacheStream( );

        // get the storage directory
        String sStorageURL = implParseSmart( SvtPathOptions().GetStoragePath() );
        INetURLObject aStorageURL( sStorageURL );
        if ( INET_PROT_NOT_VALID == aStorageURL.GetProtocol() )
        {
            OSL_FAIL( "TemplateFolderCacheImpl::openCacheStream: invalid storage path!" );
            return sal_False;
        }

        // append our name
        aStorageURL.Append( getCacheFileName() );

        // open the stream
        m_pCacheStream = UcbStreamHelper::CreateStream( aStorageURL.GetMainURL( INetURLObject::DECODE_TO_IURI ),
            _bForRead ? STREAM_READ | STREAM_NOCREATE : STREAM_WRITE | STREAM_TRUNC );
        DBG_ASSERT( m_pCacheStream, "TemplateFolderCacheImpl::openCacheStream: could not open/create the cache stream!" );
        if ( m_pCacheStream && m_pCacheStream->GetErrorCode() )
        {
            DELETEZ( m_pCacheStream );
        }

        if ( m_pCacheStream )
            m_pCacheStream->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );

        return NULL != m_pCacheStream;
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCacheImpl::needsUpdate( sal_Bool _bForceCheck )
    {
        if ( m_bKnowState && !_bForceCheck )
            return m_bNeedsUpdate;

        m_bNeedsUpdate = sal_True;
        m_bKnowState = sal_True;

        if ( readCurrentState() )
        {
            // open the stream which contains the cached state of the directories
            if ( openCacheStream( sal_True ) )
            {   // opening the stream succeeded
                if ( readPreviousState() )
                {
                    m_bNeedsUpdate = !equalStates( m_aPreviousState, m_aCurrentState );
                }
                else
                {
                    closeCacheStream();
                }
            }
        }
        return m_bNeedsUpdate;
    }

    //---------------------------------------------------------------------
    uno::Reference< util::XOfficeInstallationDirectories >
    TemplateFolderCacheImpl::getOfficeInstDirs()
    {
        if ( !m_xOfficeInstDirs.is() )
        {
            osl::MutexGuard aGuard( m_aMutex );
            if ( !m_xOfficeInstDirs.is() )
            {
                uno::Reference< uno::XComponentContext > xCtx(
                    comphelper::getProcessComponentContext() );
                xCtx->getValueByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "/singletons/com.sun.star.util.theOfficeInstallationDirectories" ) ) )
                    >>= m_xOfficeInstDirs;

                OSL_ENSURE( m_xOfficeInstDirs.is(),
                            "Unable to obtain office directories singleton!" );
            }
        }
        return m_xOfficeInstDirs;
    }

    //=====================================================================
    //= TemplateFolderCache
    //=====================================================================
    //---------------------------------------------------------------------
    TemplateFolderCache::TemplateFolderCache( sal_Bool _bAutoStoreState )
        :m_pImpl( new TemplateFolderCacheImpl( _bAutoStoreState ) )
    {
    }

    //---------------------------------------------------------------------
    TemplateFolderCache::~TemplateFolderCache( )
    {
        DELETEZ( m_pImpl );
    }

    //---------------------------------------------------------------------
    sal_Bool TemplateFolderCache::needsUpdate( sal_Bool _bForceCheck )
    {
        return m_pImpl->needsUpdate( _bForceCheck );
    }

    //---------------------------------------------------------------------
    void TemplateFolderCache::storeState( sal_Bool _bForceRetrieval )
    {
        m_pImpl->storeState( _bForceRetrieval );
    }

//.........................................................................
}   // namespace sfx2
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

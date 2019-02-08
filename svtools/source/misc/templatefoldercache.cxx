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

#include <sal/config.h>
#include <sal/log.hxx>

#include <osl/file.hxx>
#include <svtools/templatefoldercache.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/theOfficeInstallationDirectories.hpp>
#include <ucbhelper/content.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include <comphelper/processfactory.hxx>

#include <vector>
#include <list>
#include <algorithm>


namespace svt
{


    using namespace ::utl;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::uno;


    //= helpers


    static SvStream& WriteDateTime( SvStream& _rStorage, const util::DateTime& _rDate )
    {
        sal_uInt16 hundredthSeconds = static_cast< sal_uInt16 >( _rDate.NanoSeconds / tools::Time::nanoPerCenti );
        _rStorage.WriteUInt16( hundredthSeconds );

        _rStorage.WriteUInt16( _rDate.Seconds );
        _rStorage.WriteUInt16( _rDate.Minutes );
        _rStorage.WriteUInt16( _rDate.Hours );
        _rStorage.WriteUInt16( _rDate.Day );
        _rStorage.WriteUInt16( _rDate.Month );
        _rStorage.WriteInt16( _rDate.Year );

        return _rStorage;
    }


    static SvStream& operator >> ( SvStream& _rStorage, util::DateTime& _rDate )
    {
        sal_uInt16 hundredthSeconds;
        _rStorage.ReadUInt16( hundredthSeconds );
        _rDate.NanoSeconds = static_cast< sal_uInt32 >( hundredthSeconds ) * tools::Time::nanoPerCenti;

        _rStorage.ReadUInt16( _rDate.Seconds );
        _rStorage.ReadUInt16( _rDate.Minutes );
        _rStorage.ReadUInt16( _rDate.Hours );
        _rStorage.ReadUInt16( _rDate.Day );
        _rStorage.ReadUInt16( _rDate.Month );
        _rStorage.ReadInt16( _rDate.Year );

        return _rStorage;
    }

    //= TemplateContent

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
        INetURLObject const           m_aURL;
        util::DateTime          m_aLastModified;    // date of last modification as reported by UCP
        TemplateFolderContent   m_aSubContents;     // sorted (by name) list of the children

    private:
        void    implResetDate( )
        {
            m_aLastModified.NanoSeconds = m_aLastModified.Seconds = m_aLastModified.Minutes = m_aLastModified.Hours = 0;
            m_aLastModified.Day = m_aLastModified.Month = m_aLastModified.Year = 0;
        }

    private:
        virtual ~TemplateContent() override;

    public:
        explicit TemplateContent( const INetURLObject& _rURL );

        // attribute access
        OUString                 getURL( ) const                             { return m_aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ); }
        void                     setModDate( const util::DateTime& _rDate )  { m_aLastModified = _rDate; }
        const util::DateTime&    getModDate( ) const                         { return m_aLastModified; }

        TemplateFolderContent&   getSubContents()            { return m_aSubContents; }
        const TemplateFolderContent& getSubContents() const  { return m_aSubContents; }

                ConstFolderIterator              end() const             { return m_aSubContents.end(); }
        TemplateFolderContent::size_type
                                        size() const    { return m_aSubContents.size(); }

        void                     push_back( const ::rtl::Reference< TemplateContent >& _rxNewElement )
                                                        { m_aSubContents.push_back( _rxNewElement ); }
    };


    TemplateContent::TemplateContent( const INetURLObject& _rURL )
        :m_aURL( _rURL )
    {
        DBG_ASSERT( INetProtocol::NotValid != m_aURL.GetProtocol(), "TemplateContent::TemplateContent: invalid URL!" );
        implResetDate();
    }


    TemplateContent::~TemplateContent()
    {
    }


    //= stl helpers


    /// compares two TemplateContent by URL
    struct TemplateContentURLLess
    {
        bool operator() ( const ::rtl::Reference< TemplateContent >& _rxLHS, const ::rtl::Reference< TemplateContent >& _rxRHS ) const
        {
            return _rxLHS->getURL() < _rxRHS->getURL();
        }
    };


    /// sorts the sib contents of a TemplateFolderContent
    struct SubContentSort
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

    /** does a deep compare of two template contents
    */
    struct TemplateContentEqual
    {

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

            if ( !_rLHS->getSubContents().empty() )
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


    /// base class for functors which act on a SvStream
    struct StorageHelper
    {
    protected:
        SvStream&   m_rStorage;
        explicit StorageHelper( SvStream& _rStorage ) : m_rStorage( _rStorage ) { }
    };


    struct StoreContentURL : public StorageHelper
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        StoreContentURL( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StorageHelper( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }

        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            // use the base class operator with the local name of the content
            OUString sURL = _rxContent->getURL();
            // #116281# Keep office installation relocatable. Never store
            // any direct references to office installation directory.
            sURL = m_xOfficeInstDirs->makeRelocatableURL( sURL );
            m_rStorage.WriteUniOrByteString( sURL, m_rStorage.GetStreamCharSet() );
        }
    };


    /// functor which stores the complete content of a TemplateContent
    struct StoreFolderContent : public StorageHelper
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

    public:
        StoreFolderContent( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StorageHelper( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }


        void operator() ( const TemplateContent& _rContent ) const
        {
            // store the info about this content
            WriteDateTime( m_rStorage, _rContent.getModDate() );

            // store the info about the children
            // the number
            m_rStorage.WriteInt32( _rContent.size() );
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


        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.is() )
            {
                operator()( *_rxContent );
            }
        }
    };


    /// functor which reads a complete TemplateContent instance
    struct ReadFolderContent : public StorageHelper
    {
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        ReadFolderContent( SvStream& _rStorage,
                         const uno::Reference<
                            util::XOfficeInstallationDirectories > &
                                xOfficeInstDirs )
        : StorageHelper( _rStorage ), m_xOfficeInstDirs( xOfficeInstDirs ) { }


        void operator() ( TemplateContent& _rContent ) const
        {
            // store the info about this content
            util::DateTime aModDate;
            m_rStorage >> aModDate;
            _rContent.setModDate( aModDate );

            // store the info about the children
            // the number
            sal_Int32 nChildren = 0;
            m_rStorage.ReadInt32( nChildren );
            TemplateFolderContent& rChildren = _rContent.getSubContents();
            rChildren.resize( 0 );
            rChildren.reserve( nChildren );
            // initialize them with their (local) names
            while ( nChildren-- )
            {
                OUString sURL = m_rStorage.ReadUniOrByteString(m_rStorage.GetStreamCharSet());
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


        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.is() )
            {
                operator()( *_rxContent );
            }
        }
    };


    //= TemplateFolderCacheImpl

    class TemplateFolderCacheImpl
    {
    private:
        TemplateFolderContent           m_aPreviousState;   // the current state of the template dirs (as found on the HD)
        TemplateFolderContent           m_aCurrentState;    // the previous state of the template dirs (as found in the cache file)

        osl::Mutex                      m_aMutex;
        // will be lazy inited; never access directly; use getOfficeInstDirs().
        uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

        std::unique_ptr<SvStream>       m_pCacheStream;
        bool                            m_bNeedsUpdate : 1;
        bool                            m_bKnowState : 1;
        bool                            m_bValidCurrentState : 1;
        bool const                      m_bAutoStoreState : 1;

    public:
        explicit TemplateFolderCacheImpl( bool _bAutoStoreState );
        ~TemplateFolderCacheImpl( );

        bool        needsUpdate();
        void        storeState();

    private:
        bool        openCacheStream( bool _bForRead );
        void        closeCacheStream( );

        /// read the state of the dirs from the cache file
        bool        readPreviousState();
        /// read the current state of the dirs
        bool        readCurrentState();

        static OUString    implParseSmart( const OUString& _rPath );

        bool        implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot );

        static  sal_Int32   getMagicNumber();
        static  void        normalize( TemplateFolderContent& _rState );

        // @return <TRUE/> if the states equal
        static  bool        equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS );

        // late initialize m_xOfficeInstDirs
        uno::Reference< util::XOfficeInstallationDirectories > getOfficeInstDirs();
    };


    TemplateFolderCacheImpl::TemplateFolderCacheImpl( bool _bAutoStoreState )
        :m_bNeedsUpdate         ( true )
        ,m_bKnowState           ( false )
        ,m_bValidCurrentState   ( false )
        ,m_bAutoStoreState      ( _bAutoStoreState )
    {
    }


    TemplateFolderCacheImpl::~TemplateFolderCacheImpl( )
    {
        // store the current state if possible and required
        if ( m_bValidCurrentState && m_bAutoStoreState )
            storeState();

        closeCacheStream( );
    }


    sal_Int32 TemplateFolderCacheImpl::getMagicNumber()
    {
        return (sal_Int8('T') << 12)
                | (sal_Int8('D') << 8)
                | (sal_Int8('S') << 4)
                | (sal_Int8('C'));
    }


    void TemplateFolderCacheImpl::normalize( TemplateFolderContent& _rState )
    {
        SubContentSort()( _rState );
    }


    bool TemplateFolderCacheImpl::equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS )
    {
        if ( _rLHS.size() != _rRHS.size() )
            return false;

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


    void TemplateFolderCacheImpl::storeState()
    {
        if ( !m_bValidCurrentState )
            readCurrentState( );

        if ( !(m_bValidCurrentState && openCacheStream( false )) )
            return;

        m_pCacheStream->WriteInt32( getMagicNumber() );

        // store the template root folders
        // the size
        m_pCacheStream->WriteInt32( m_aCurrentState.size() );
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


    OUString TemplateFolderCacheImpl::implParseSmart( const OUString& _rPath )
    {
        INetURLObject aParser;
        aParser.SetSmartProtocol( INetProtocol::File );
        aParser.SetURL( _rPath );
        if ( INetProtocol::NotValid == aParser.GetProtocol() )
        {
            OUString sURL;
            osl::FileBase::getFileURLFromSystemPath( _rPath, sURL );
            aParser.SetURL( sURL );
        }
        return aParser.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    }


    void TemplateFolderCacheImpl::closeCacheStream( )
    {
        m_pCacheStream.reset();
    }


    bool TemplateFolderCacheImpl::implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot )
    {
        try
        {
            // create a content for the current folder root
            Reference< XResultSet > xResultSet;
            Sequence< OUString > aContentProperties( 4);
            aContentProperties[0] = "Title";
            aContentProperties[1] = "DateModified";
            aContentProperties[2] = "DateCreated";
            aContentProperties[3] = "IsFolder";

            // get the set of sub contents in the folder
            try
            {
                Reference< XDynamicResultSet > xDynResultSet;

                ::ucbhelper::Content aTemplateRoot( _rxRoot->getURL(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                xDynResultSet = aTemplateRoot.createDynamicCursor( aContentProperties );
                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                SAL_WARN( "svtools.misc", "TemplateFolderCacheImpl::implReadFolder: caught a CommandAbortedException!" );
                return false;
            }
            catch( css::uno::Exception& )
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
            return false;
        }
        return true;
    }


    bool TemplateFolderCacheImpl::readCurrentState()
    {
        // reset
        m_bValidCurrentState = false;
        TemplateFolderContent aTemplateFolderContent;
        m_aCurrentState.swap( aTemplateFolderContent );

        // the template directories from the config
        const SvtPathOptions aPathOptions;
        const OUString& aDirs = aPathOptions.GetTemplatePath();

        // loop through all the root-level template folders
        sal_Int32 nIndex = 0;
        do
        {
            OUString sTemplatePath( aDirs.getToken(0, ';', nIndex) );
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
                return false;
        }
        while ( nIndex >= 0 );

        // normalize the array (which basically means "sort it")
        normalize( m_aCurrentState );

        m_bValidCurrentState = true;
        return m_bValidCurrentState;
    }


    bool TemplateFolderCacheImpl::readPreviousState()
    {
        DBG_ASSERT( m_pCacheStream, "TemplateFolderCacheImpl::readPreviousState: not to be called without stream!" );

        // reset
        TemplateFolderContent aTemplateFolderContent;
        m_aPreviousState.swap( aTemplateFolderContent );

        // check the magic number
        sal_Int32 nMagic = 0;
        m_pCacheStream->ReadInt32( nMagic );
        DBG_ASSERT( getMagicNumber() == nMagic, "TemplateFolderCacheImpl::readPreviousState: invalid cache file!" );
        if ( getMagicNumber() != nMagic )
            return false;

        // the root directories
        // their number
        sal_Int32 nRootDirectories = 0;
        m_pCacheStream->ReadInt32( nRootDirectories );
        // init empty TemplateContents with the URLs
        m_aPreviousState.reserve( nRootDirectories );
        while ( nRootDirectories-- )
        {
            OUString sURL = m_pCacheStream->ReadUniOrByteString(m_pCacheStream->GetStreamCharSet());
            // #116281# Keep office installation relocatable. Never store
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

        return true;
    }


    bool TemplateFolderCacheImpl::openCacheStream( bool _bForRead )
    {
        // close any old stream instance
        closeCacheStream( );

        // get the storage directory
        OUString sStorageURL = implParseSmart( SvtPathOptions().GetStoragePath() );
        INetURLObject aStorageURL( sStorageURL );
        if ( INetProtocol::NotValid == aStorageURL.GetProtocol() )
        {
            OSL_FAIL( "TemplateFolderCacheImpl::openCacheStream: invalid storage path!" );
            return false;
        }

        // append our name
        aStorageURL.Append( ".templdir.cache" );

        // open the stream
        m_pCacheStream = UcbStreamHelper::CreateStream( aStorageURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ),
            _bForRead ? StreamMode::READ | StreamMode::NOCREATE : StreamMode::WRITE | StreamMode::TRUNC );
        DBG_ASSERT( m_pCacheStream, "TemplateFolderCacheImpl::openCacheStream: could not open/create the cache stream!" );
        if ( m_pCacheStream && m_pCacheStream->GetErrorCode() )
        {
            m_pCacheStream.reset();
        }

        if ( m_pCacheStream )
            m_pCacheStream->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );

        return nullptr != m_pCacheStream;
    }


    bool TemplateFolderCacheImpl::needsUpdate()
    {
        if ( m_bKnowState )
            return m_bNeedsUpdate;

        m_bNeedsUpdate = true;
        m_bKnowState = true;

        if ( readCurrentState() )
        {
            // open the stream which contains the cached state of the directories
            if ( openCacheStream( true ) )
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
                m_xOfficeInstDirs = util::theOfficeInstallationDirectories::get(xCtx);
            }
        }
        return m_xOfficeInstDirs;
    }


    //= TemplateFolderCache


    TemplateFolderCache::TemplateFolderCache( bool _bAutoStoreState )
        :m_pImpl( new TemplateFolderCacheImpl( _bAutoStoreState ) )
    {
    }


    TemplateFolderCache::~TemplateFolderCache( )
    {
    }


    bool TemplateFolderCache::needsUpdate()
    {
        return m_pImpl->needsUpdate();
    }


    void TemplateFolderCache::storeState()
    {
        m_pImpl->storeState();
    }


}   // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

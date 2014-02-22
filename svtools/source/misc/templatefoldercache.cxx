/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svtools/templatefoldercache.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/theOfficeInstallationDirectories.hpp>
#include <ucbhelper/content.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include "comphelper/processfactory.hxx"

#include <vector>
#include <list>
#include <functional>
#include <algorithm>


namespace svt
{


    using namespace ::utl;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::uno;

    
    
    
    
    SvStream& WriteDateTime( SvStream& _rStorage, const util::DateTime& _rDate )
    {
        sal_uInt16 hundredthSeconds = static_cast< sal_uInt16 >( _rDate.NanoSeconds / Time::nanoPerCenti );
        _rStorage.WriteUInt16( hundredthSeconds );

        _rStorage.WriteUInt16( _rDate.Seconds );
        _rStorage.WriteUInt16( _rDate.Minutes );
        _rStorage.WriteUInt16( _rDate.Hours );
        _rStorage.WriteUInt16( _rDate.Day );
        _rStorage.WriteUInt16( _rDate.Month );
        _rStorage.WriteInt16( _rDate.Year );

        return _rStorage;
    }

    
    SvStream& operator >> ( SvStream& _rStorage, util::DateTime& _rDate )
    {
        sal_uInt16 hundredthSeconds;
        _rStorage.ReadUInt16( hundredthSeconds );
        _rDate.NanoSeconds = static_cast< sal_uInt32 >( hundredthSeconds ) * Time::nanoPerCenti;

        _rStorage.ReadUInt16( _rDate.Seconds );
        _rStorage.ReadUInt16( _rDate.Minutes );
        _rStorage.ReadUInt16( _rDate.Hours );
        _rStorage.ReadUInt16( _rDate.Day );
        _rStorage.ReadUInt16( _rDate.Month );
        _rStorage.ReadInt16( _rDate.Year );

        return _rStorage;
    }

    
    sal_Bool operator == ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return  _rLHS.NanoSeconds == _rRHS.NanoSeconds
            &&  _rLHS.Seconds   == _rRHS.Seconds
            &&  _rLHS.Minutes   == _rRHS.Minutes
            &&  _rLHS.Hours     == _rRHS.Hours
            &&  _rLHS.Day       == _rRHS.Day
            &&  _rLHS.Month     == _rRHS.Month
            &&  _rLHS.Year      == _rRHS.Year
            &&  _rLHS.IsUTC     == _rRHS.IsUTC;
    }

    
    sal_Bool operator != ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return !( _rLHS == _rRHS );
    }

    
    
    
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
        OUString                m_sLocalName;       
        util::DateTime          m_aLastModified;    
        TemplateFolderContent   m_aSubContents;     

    private:
        inline  void    implResetDate( )
        {
            m_aLastModified.NanoSeconds = m_aLastModified.Seconds = m_aLastModified.Minutes = m_aLastModified.Hours = 0;
            m_aLastModified.Day = m_aLastModified.Month = m_aLastModified.Year = 0;
        }

    private:
        ~TemplateContent();

    public:
        TemplateContent( const INetURLObject& _rURL );

        
        inline OUString                 getName( ) const                            { return m_sLocalName; }
        inline OUString                 getURL( ) const                             { return m_aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ); }
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

    
    DBG_NAME( TemplateContent )

    
    TemplateContent::TemplateContent( const INetURLObject& _rURL )
        :m_aURL( _rURL )
    {
        DBG_CTOR( TemplateContent, NULL );
        DBG_ASSERT( INET_PROT_NOT_VALID != m_aURL.GetProtocol(), "TemplateContent::TemplateContent: invalid URL!" );
        m_sLocalName = m_aURL.getName();
        implResetDate();
    }

    
    TemplateContent::~TemplateContent()
    {
        DBG_DTOR( TemplateContent, NULL );
    }

    
    
    
    
    
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

    
    
    struct SubContentSort : public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
    {
        void operator() ( TemplateFolderContent& _rFolder ) const
        {
            
            ::std::sort(
                _rFolder.begin(),
                _rFolder.end(),
                TemplateContentURLLess()
            );

            
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
        :public ::std::binary_function  <   ::rtl::Reference< TemplateContent >
                                        ,   ::rtl::Reference< TemplateContent >
                                        ,   bool
                                        >
    {
        
        bool operator() (const ::rtl::Reference< TemplateContent >& _rLHS, const ::rtl::Reference< TemplateContent >& _rRHS )
        {
            if ( !_rLHS.is() || !_rRHS.is() )
            {
                OSL_FAIL( "TemplateContentEqual::operator(): invalid contents!" );
                return true;
                    
            }

            if ( _rLHS->getURL() != _rRHS->getURL() )
                return false;

            if ( _rLHS->getModDate() != _rRHS->getModDate() )
                return false;

            if ( _rLHS->getSubContents().size() != _rRHS->getSubContents().size() )
                return false;

            if ( _rLHS->getSubContents().size() )
            {   
                
                ::std::pair< FolderIterator, FolderIterator > aFirstDifferent = ::std::mismatch(
                    _rLHS->getSubContents().begin(),
                    _rLHS->getSubContents().end(),
                    _rRHS->getSubContents().begin(),
                    *this
                );
                if ( aFirstDifferent.first != _rLHS->getSubContents().end() )
                    return false;
            }

            return true;
        }
    };

    
    
    struct StorageHelper
    {
    protected:
        SvStream&   m_rStorage;
        StorageHelper( SvStream& _rStorage ) : m_rStorage( _rStorage ) { }
    };

    
    
    struct StoreString
            :public ::std::unary_function< OUString, void >
            ,public StorageHelper
    {
        StoreString( SvStream& _rStorage ) : StorageHelper( _rStorage ) { }

        void operator() ( const OUString& _rString ) const
        {
            m_rStorage.WriteUniOrByteString( _rString, m_rStorage.GetStreamCharSet() );
        }
    };

    
    
    struct StoreLocalContentName
            :public ::std::unary_function< ::rtl::Reference< TemplateContent >, void >
            ,public StoreString
    {
        StoreLocalContentName( SvStream& _rStorage ) : StoreString( _rStorage ) { }

        void operator() ( const ::rtl::Reference< TemplateContent >& _rxContent ) const
        {
            SAL_WARN( "svtools.misc", "This method must not be used, the whole URL must be stored!" );

            
            StoreString::operator() ( _rxContent->getName() );
        }
    };

    
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
            
            OUString sURL = _rxContent->getURL();
            
            
            sURL = m_xOfficeInstDirs->makeRelocatableURL( sURL );
            StoreString::operator() ( sURL );
        }
    };

    
    
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

        
        void operator() ( const TemplateContent& _rContent ) const
        {
            
            WriteDateTime( m_rStorage, _rContent.getModDate() );

            
            
            m_rStorage.WriteInt32( (sal_Int32)_rContent.size() );
            
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                StoreContentURL( m_rStorage, m_xOfficeInstDirs )
            );
            
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

        
        void operator() ( TemplateContent& _rContent ) const
        {
            
            util::DateTime aModDate;
            m_rStorage >> aModDate;
            _rContent.setModDate( aModDate );

            
            
            sal_Int32 nChildren = 0;
            m_rStorage.ReadInt32( nChildren );
            TemplateFolderContent& rChildren = _rContent.getSubContents();
            rChildren.resize( 0 );
            rChildren.reserve( nChildren );
            
            while ( nChildren-- )
            {
                OUString sURL = m_rStorage.ReadUniOrByteString(m_rStorage.GetStreamCharSet());
                sURL = m_xOfficeInstDirs->makeAbsoluteURL( sURL );
                INetURLObject aChildURL( sURL );
                rChildren.push_back( new TemplateContent( aChildURL ) );
            }

            
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

    
    
    
    class TemplateFolderCacheImpl
    {
    private:
        TemplateFolderContent           m_aPreviousState;   
        TemplateFolderContent           m_aCurrentState;    

        osl::Mutex                      m_aMutex;
        
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

        
        sal_Bool    readPreviousState();
        
        sal_Bool    readCurrentState();

        OUString    implParseSmart( const OUString& _rPath );

        sal_Bool    implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot );

        static  OUString getCacheFileName();
        static  sal_Int32   getMagicNumber();
        static  void        normalize( TemplateFolderContent& _rState );

        
        static  sal_Bool    equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS );

        
        uno::Reference< util::XOfficeInstallationDirectories > getOfficeInstDirs();
    };

    
    TemplateFolderCacheImpl::TemplateFolderCacheImpl( sal_Bool _bAutoStoreState )
        :m_pCacheStream         ( NULL )
        ,m_bNeedsUpdate         ( sal_True )
        ,m_bKnowState           ( sal_False )
        ,m_bValidCurrentState   ( sal_False )
        ,m_bAutoStoreState      ( _bAutoStoreState )
    {
    }

    
    TemplateFolderCacheImpl::~TemplateFolderCacheImpl( )
    {
        
        if ( m_bValidCurrentState && m_bAutoStoreState )
            storeState( sal_False );

        closeCacheStream( );
    }

    
    sal_Int32 TemplateFolderCacheImpl::getMagicNumber()
    {
        sal_Int32 nMagic = 0;
        ( nMagic += (sal_Int8)'T' ) <<= 4;
        ( nMagic += (sal_Int8)'D' ) <<= 4;
        ( nMagic += (sal_Int8)'S' ) <<= 4;
        ( nMagic += (sal_Int8)'C' ) <<= 0;
        return nMagic;
    }

    
    OUString TemplateFolderCacheImpl::getCacheFileName()
    {
        return OUString(".templdir.cache");
    }


    
    void TemplateFolderCacheImpl::normalize( TemplateFolderContent& _rState )
    {
        SubContentSort()( _rState );
    }

    
    sal_Bool TemplateFolderCacheImpl::equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS )
    {
        if ( _rLHS.size() != _rRHS.size() )
            return sal_False;

        
        

        ::std::pair< ConstFolderIterator, ConstFolderIterator > aFirstDifferent = ::std::mismatch(
            _rLHS.begin(),
            _rLHS.end(),
            _rRHS.begin(),
            TemplateContentEqual()
        );

        return aFirstDifferent.first == _rLHS.end();
    }

    
    void TemplateFolderCacheImpl::storeState( sal_Bool _bForceRetrieval )
    {
        if ( !m_bValidCurrentState || _bForceRetrieval )
            readCurrentState( );

        if ( m_bValidCurrentState && openCacheStream( sal_False ) )
        {
            m_pCacheStream->WriteInt32( getMagicNumber() );

            
            
            m_pCacheStream->WriteInt32( (sal_Int32)m_aCurrentState.size() );
            
            ::std::for_each(
                m_aCurrentState.begin(),
                m_aCurrentState.end(),
                StoreContentURL( *m_pCacheStream, getOfficeInstDirs() )
            );

            
            ::std::for_each(
                m_aCurrentState.begin(),
                m_aCurrentState.end(),
                StoreFolderContent( *m_pCacheStream, getOfficeInstDirs() )
            );
        }
    }

    
    OUString TemplateFolderCacheImpl::implParseSmart( const OUString& _rPath )
    {
        INetURLObject aParser;
        aParser.SetSmartProtocol( INET_PROT_FILE );
        aParser.SetURL( _rPath, INetURLObject::WAS_ENCODED );
        if ( INET_PROT_NOT_VALID == aParser.GetProtocol() )
        {
            OUString sURL;
            LocalFileHelper::ConvertPhysicalNameToURL( _rPath, sURL );
            aParser.SetURL( sURL, INetURLObject::WAS_ENCODED );
        }
        return aParser.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }

    
    void TemplateFolderCacheImpl::closeCacheStream( )
    {
        DELETEZ( m_pCacheStream );
    }

    
    sal_Bool TemplateFolderCacheImpl::implReadFolder( const ::rtl::Reference< TemplateContent >& _rxRoot )
    {
        try
        {
            
            Reference< XResultSet > xResultSet;
            Sequence< OUString > aContentProperties( 4);
            aContentProperties[0] = "Title";
            aContentProperties[1] = "DateModified";
            aContentProperties[2] = "DateCreated";
            aContentProperties[3] = "IsFolder";

            
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

            
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );
                Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );

                while ( xResultSet->next() )
                {
                    INetURLObject aSubContentURL( xContentAccess->queryContentIdentifierString() );

                    
                    ::rtl::Reference< TemplateContent > xChild = new TemplateContent( aSubContentURL );

                    
                    xChild->setModDate( xRow->getTimestamp( 2 ) );  
                    if ( xRow->wasNull() )
                        xChild->setModDate( xRow->getTimestamp( 3 ) );  

                    
                    _rxRoot->push_back( xChild );

                    
                    if ( xRow->getBoolean( 4 ) && !xRow->wasNull() )
                    {   
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

    
    sal_Bool TemplateFolderCacheImpl::readCurrentState()
    {
        
        m_bValidCurrentState = sal_False;
        TemplateFolderContent aTemplateFolderContent;
        m_aCurrentState.swap( aTemplateFolderContent );

        
        const SvtPathOptions aPathOptions;
        OUString aDirs = aPathOptions.GetTemplatePath();

        
        sal_Int32 nIndex = 0;
        do
        {
            OUString sTemplatePath( aDirs.getToken(0, ';', nIndex) );
            sTemplatePath = aPathOptions.ExpandMacros( sTemplatePath );

            
            
            
            
            
            
            
            sTemplatePath = getOfficeInstDirs()->makeAbsoluteURL(
                getOfficeInstDirs()->makeRelocatableURL(sTemplatePath));

            
            m_aCurrentState.push_back( new TemplateContent( INetURLObject( sTemplatePath ) ) );
            TemplateFolderContent::iterator aCurrentRoot = m_aCurrentState.end();
            --aCurrentRoot;

            if ( !implReadFolder( *aCurrentRoot ) )
                return sal_False;
        }
        while ( nIndex >= 0 );

        
        normalize( m_aCurrentState );

        m_bValidCurrentState = sal_True;
        return m_bValidCurrentState;
    }

    
    sal_Bool TemplateFolderCacheImpl::readPreviousState()
    {
        DBG_ASSERT( m_pCacheStream, "TemplateFolderCacheImpl::readPreviousState: not to be called without stream!" );

        
        TemplateFolderContent aTemplateFolderContent;
        m_aPreviousState.swap( aTemplateFolderContent );

        
        sal_Int32 nMagic = 0;
        m_pCacheStream->ReadInt32( nMagic );
        DBG_ASSERT( getMagicNumber() == nMagic, "TemplateFolderCacheImpl::readPreviousState: invalid cache file!" );
        if ( getMagicNumber() != nMagic )
            return sal_False;

        
        
        sal_Int32 nRootDirectories = 0;
        m_pCacheStream->ReadInt32( nRootDirectories );
        
        m_aPreviousState.reserve( nRootDirectories );
        while ( nRootDirectories-- )
        {
            OUString sURL = m_pCacheStream->ReadUniOrByteString(m_pCacheStream->GetStreamCharSet());
            
            
            sURL = getOfficeInstDirs()->makeAbsoluteURL( sURL );
            m_aPreviousState.push_back(
                new TemplateContent( INetURLObject(sURL) ) );
        }

        
        ::std::for_each(
            m_aPreviousState.begin(),
            m_aPreviousState.end(),
            ReadFolderContent( *m_pCacheStream, getOfficeInstDirs() )
        );

        DBG_ASSERT( !m_pCacheStream->GetErrorCode(), "TemplateFolderCacheImpl::readPreviousState: unknown error during reading the state cache!" );

        
        normalize( m_aPreviousState );

        return sal_True;
    }

    
    sal_Bool TemplateFolderCacheImpl::openCacheStream( sal_Bool _bForRead )
    {
        
        closeCacheStream( );

        
        OUString sStorageURL = implParseSmart( SvtPathOptions().GetStoragePath() );
        INetURLObject aStorageURL( sStorageURL );
        if ( INET_PROT_NOT_VALID == aStorageURL.GetProtocol() )
        {
            OSL_FAIL( "TemplateFolderCacheImpl::openCacheStream: invalid storage path!" );
            return sal_False;
        }

        
        aStorageURL.Append( getCacheFileName() );

        
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

    
    sal_Bool TemplateFolderCacheImpl::needsUpdate( sal_Bool _bForceCheck )
    {
        if ( m_bKnowState && !_bForceCheck )
            return m_bNeedsUpdate;

        m_bNeedsUpdate = sal_True;
        m_bKnowState = sal_True;

        if ( readCurrentState() )
        {
            
            if ( openCacheStream( sal_True ) )
            {   
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

    
    
    
    
    TemplateFolderCache::TemplateFolderCache( sal_Bool _bAutoStoreState )
        :m_pImpl( new TemplateFolderCacheImpl( _bAutoStoreState ) )
    {
    }

    
    TemplateFolderCache::~TemplateFolderCache( )
    {
        DELETEZ( m_pImpl );
    }

    
    sal_Bool TemplateFolderCache::needsUpdate( sal_Bool _bForceCheck )
    {
        return m_pImpl->needsUpdate( _bForceCheck );
    }

    
    void TemplateFolderCache::storeState( sal_Bool _bForceRetrieval )
    {
        m_pImpl->storeState( _bForceRetrieval );
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

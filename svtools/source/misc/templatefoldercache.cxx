/*************************************************************************
 *
 *  $RCSfile: templatefoldercache.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SFX2_TEMPLATEFOLDERCACHE_HXX
#include "templatefoldercache.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include "pathoptions.hxx"
#endif

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
    typedef ::std::vector< ::vos::ORef< TemplateContent > > TemplateFolderContent;
        typedef TemplateFolderContent::const_iterator           ConstFolderIterator;
        typedef TemplateFolderContent::iterator                 FolderIterator;

    /** a struct describing one content in one of the template dirs (or at least it's relevant aspects)
    */
    struct TemplateContent : public ::vos::OReference
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
        TemplateContent();
        TemplateContent( const INetURLObject& _rURL );
        TemplateContent( const INetURLObject& _rURL, const util::DateTime& _rLastModified );

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

        inline void                     push_back( const ::vos::ORef< TemplateContent >& _rxNewElement )
                                                        { m_aSubContents.push_back( _rxNewElement ); }
    };

    //---------------------------------------------------------------------
    DBG_NAME( TemplateContent )

    //---------------------------------------------------------------------
    TemplateContent::TemplateContent()
    {
        DBG_CTOR( TemplateContent, NULL );
        implResetDate();
    }

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
    TemplateContent::TemplateContent( const INetURLObject& _rURL, const util::DateTime& _rLastModified )
        :m_aURL( _rURL )
        ,m_aLastModified( _rLastModified )
    {
        DBG_CTOR( TemplateContent, NULL );
        DBG_ASSERT( INET_PROT_NOT_VALID != m_aURL.GetProtocol(), "TemplateContent::TemplateContent: invalid URL!" );
        m_sLocalName = m_aURL.getName();
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
        :public ::std::binary_function  <   ::vos::ORef< TemplateContent >
                                        ,   ::vos::ORef< TemplateContent >
                                        ,   bool
                                        >
    {
        bool operator() ( const ::vos::ORef< TemplateContent >& _rxLHS, const ::vos::ORef< TemplateContent >& _rxRHS ) const
        {
            return  _rxLHS->getURL() < _rxRHS->getURL()
                ?   true
                :   false;
        }
    };

    //---------------------------------------------------------------------
    /// sorts the sib contents of a TemplateFolderContent
    struct SubContentSort : public ::std::unary_function< ::vos::ORef< TemplateContent >, void >
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

        void operator() ( const ::vos::ORef< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.isValid() && _rxContent->size() )
            {
                operator()( _rxContent->getSubContents() );
            }
        }
    };
    //---------------------------------------------------------------------
    /** does a deep compare of two template contents
    */
    struct TemplateContentEqual
        :public ::std::binary_function  <   ::vos::ORef< TemplateContent >
                                        ,   ::vos::ORef< TemplateContent >
                                        ,   bool
                                        >
    {
        //.................................................................
        bool operator() (const ::vos::ORef< TemplateContent >& _rLHS, const ::vos::ORef< TemplateContent >& _rRHS )
        {
            if ( !_rLHS.isValid() || !_rRHS.isValid() )
            {
                DBG_ERROR( "TemplateContentEqual::operator(): invalid contents!" );
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
            m_rStorage.WriteByteString( _rString );
        }
    };

    //---------------------------------------------------------------------
    /// functor which stores the local name of a TemplateContent
    struct StoreLocalContentName
            :public ::std::unary_function< ::vos::ORef< TemplateContent >, void >
            ,public StoreString
    {
        StoreLocalContentName( SvStream& _rStorage ) : StoreString( _rStorage ) { }

        void operator() ( const ::vos::ORef< TemplateContent >& _rxContent ) const
        {
            // use the base class operator with the local name of the content
            StoreString::operator() ( _rxContent->getName() );
        }
    };

    //---------------------------------------------------------------------
    struct StoreContentURL
            :public ::std::unary_function< ::vos::ORef< TemplateContent >, void >
            ,public StoreString
    {
        StoreContentURL( SvStream& _rStorage ) : StoreString( _rStorage ) { }

        void operator() ( const ::vos::ORef< TemplateContent >& _rxContent ) const
        {
            // use the base class operator with the local name of the content
            StoreString::operator() ( _rxContent->getURL() );
        }
    };

    //---------------------------------------------------------------------
    /// functor which stores the complete content of a TemplateContent
    struct StoreFolderContent
            :public ::std::unary_function< ::vos::ORef< TemplateContent >, void >
            ,public StorageHelper
    {
    public:
        StoreFolderContent( SvStream& _rStorage ) : StorageHelper( _rStorage ) { }

        //.................................................................
        void operator() ( const TemplateContent& _rContent ) const
        {
            // store the info about this content
            m_rStorage << _rContent.getModDate();

            // store the info about the children
            // the number
            m_rStorage << (sal_Int32)_rContent.size();
            // their (local!) names
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                StoreLocalContentName( m_rStorage )
            );
            // their content
            ::std::for_each(
                _rContent.getSubContents().begin(),
                _rContent.getSubContents().end(),
                *this
            );
        }

        //.................................................................
        void operator() ( const ::vos::ORef< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.isValid() )
            {
                operator()( *_rxContent );
            }
        }
    };

    //---------------------------------------------------------------------
    /// functor which reads a complete TemplateContent instance
    struct ReadFolderContent
            :public ::std::unary_function< ::vos::ORef< TemplateContent >, void >
            ,public StorageHelper
    {
        ReadFolderContent( SvStream& _rStorage ) : StorageHelper( _rStorage ) { }

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
                String sLocalName;
                m_rStorage.ReadByteString( sLocalName );
                INetURLObject aChildURL( _rContent.getURL() );
                aChildURL.Append( sLocalName );
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
        void operator() ( const ::vos::ORef< TemplateContent >& _rxContent ) const
        {
            if ( _rxContent.isValid() )
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
        void        initTemplDirs( ::std::vector< String >& _rRootDirs );
        sal_Bool    openCacheStream( sal_Bool _bForRead );
        void        closeCacheStream( );

        /// read the state of the dirs from the cache file
        sal_Bool    readPreviousState();
        /// read the current state of the dirs
        sal_Bool    readCurrentState();

        String      implParseSmart( const String& _rPath );

        sal_Bool    implReadFolder( const ::vos::ORef< TemplateContent >& _rxRoot );

        static  String      getCacheFileName();
        static  sal_Int32   getMagicNumber();
        static  void        normalize( TemplateFolderContent& _rState );

        // @return <TRUE/> if the states equal
        static  sal_Bool    equalStates( const TemplateFolderContent& _rLHS, const TemplateFolderContent& _rRHS );
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
    String TemplateFolderCacheImpl::getCacheFileName()
    {
        return String::CreateFromAscii( ".templdir.cache" );
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
                StoreContentURL( *m_pCacheStream )
            );

            // the contents
            ::std::for_each(
                m_aCurrentState.begin(),
                m_aCurrentState.end(),
                StoreFolderContent( *m_pCacheStream )
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
            String sURL;
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
    sal_Bool TemplateFolderCacheImpl::implReadFolder( const ::vos::ORef< TemplateContent >& _rxRoot )
    {
        try
        {
            // create a content for the current folder root
            Reference< XResultSet > xResultSet;
            Sequence< ::rtl::OUString > aContentProperties( 4);
            aContentProperties[0] = ::rtl::OUString::createFromAscii( "Title" );
            aContentProperties[1] = ::rtl::OUString::createFromAscii( "DateModified" );
            aContentProperties[2] = ::rtl::OUString::createFromAscii( "DateCreated" );
            aContentProperties[3] = ::rtl::OUString::createFromAscii( "IsFolder" );

            // get the set of sub contents in the folder
            try
            {
                Reference< XDynamicResultSet > xDynResultSet;

                ::ucb::Content aTemplateRoot( _rxRoot->getURL(), Reference< XCommandEnvironment >() );
                xDynResultSet = aTemplateRoot.createDynamicCursor( aContentProperties, ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS );
                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                DBG_ERRORFILE( "TemplateFolderCacheImpl::implReadFolder: caught a CommandAbortedException!" );
                return sal_False;
            }
            catch( ::com::sun::star::uno::Exception& e )
            {
                e; // make compiler happy
            }

            // collect the infos about the sub contents
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY );
                while ( xResultSet->next() )
                {
                    INetURLObject aSubContentURL( _rxRoot->getURL() );
                    aSubContentURL.Append( xRow->getString( 1 ) );
                    // a new content instance
                    ::vos::ORef< TemplateContent > xChild = new TemplateContent( aSubContentURL );

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
            DBG_ERROR( "TemplateFolderCacheImpl::implReadFolder: caught an exception!" );
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
        String      aDirs = SvtPathOptions().GetTemplatePath();
        sal_uInt16  nDirs = aDirs.GetTokenCount( ';' );

        m_aCurrentState.reserve( nDirs );
        // loop through all the root-level template folders
        for ( sal_uInt16 i=0; i<nDirs; ++i)
        {
            // create a new entry
            m_aCurrentState.push_back( new TemplateContent( INetURLObject( aDirs.GetToken( i, ';' ) ) ) );
            TemplateFolderContent::iterator aCurrentRoot = m_aCurrentState.end();
            --aCurrentRoot;

            if ( !implReadFolder( *aCurrentRoot ) )
                return sal_False;
        }

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
            String sURL;
            m_pCacheStream->ReadByteString( sURL );
            m_aPreviousState.push_back( new TemplateContent( sURL ) );
        }

        // read the contents of the root folders
        ::std::for_each(
            m_aPreviousState.begin(),
            m_aPreviousState.end(),
            ReadFolderContent( *m_pCacheStream )
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
            DBG_ERROR( "TemplateFolderCacheImpl::openCacheStream: invalid storage path!" );
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
    void TemplateFolderCacheImpl::initTemplDirs( ::std::vector< String >& _rRootDirs )
    {
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


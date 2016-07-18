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

#include <string.h>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "tvread.hxx"
#include <expat.h>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/deployment/thePackageManagerFactory.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/pathoptions.hxx>

namespace treeview {

    class TVDom
    {
        friend class TVChildTarget;
        friend class TVRead;

    public:

        explicit TVDom( TVDom* arent = nullptr )
            : kind( other ),
              parent( arent ),
              children( 0 )
        {
        }

        ~TVDom()
        {
            for(TVDom* p : children)
                delete p;
        }

        TVDom* newChild()
        {
            children.push_back( new TVDom( this ) );
            return children.back();
        }

        TVDom* newChild(TVDom* p)
        {
            children.push_back( p );
            p->parent = this;
            return children.back();
        }

        TVDom* getParent() const
        {
            if( parent )
                return parent;
            else
                return const_cast<TVDom*>(this);    // I am my own parent, if I am the root
        }

        enum Kind {
            tree_view,
            tree_node,
            tree_leaf,
            other
        };

        bool isLeaf() const { return kind == TVDom::tree_leaf; }
        void setKind( Kind ind ) { kind = ind; }

        void setApplication( const char* appl )
        {
            application = OUString( appl,
                                         strlen( appl ),
                                         RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const char* itle )
        {
            title += OUString( itle,
                                    strlen( itle ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const XML_Char* itle,int len )
        {
            title += OUString( itle,
                                    len,
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setId( const char* d )
        {
            id = OUString( d,
                                strlen( d ),
                                RTL_TEXTENCODING_UTF8 );
        }

        void setAnchor( const char* nchor )
        {
            anchor = OUString( nchor,
                                    strlen( nchor ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        OUString const & getTargetURL()
        {
            if( targetURL.isEmpty() )
            {
                sal_Int32 len;
                for ( const TVDom* p = this;; p = p->parent )
                {
                    len = p->application.getLength();
                    if ( len != 0 )
                        break;
                }

                OUStringBuffer strBuff( 22 + len + id.getLength() );
                strBuff.append(
                                    "vnd.sun.star.help://"
                                    ).append(id);

                targetURL = strBuff.makeStringAndClear();
            }

            return targetURL;
        }

    private:

        Kind   kind;
        OUString  application;
        OUString  title;
        OUString  id;
        OUString  anchor;
        OUString  targetURL;

        TVDom *parent;
        std::vector< TVDom* > children;
    };

}

using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::deployment;

ConfigData::ConfigData()
    : prodName("%PRODUCTNAME"),
      prodVersion("%PRODUCTVERSION"),
      vendName("%VENDORNAME"),
      vendVersion("%VENDORVERSION"),
      vendShort("%VENDORSHORT")
{
    memset(m_vAdd, 0, sizeof(m_vAdd));
}

void SAL_CALL ConfigData::replaceName( OUString& oustring ) const
{
    sal_Int32 idx = -1,k = 0,off;
    bool cap = false;
    OUStringBuffer aStrBuf( 0 );

    while( ( idx = oustring.indexOf( '%', ++idx ) ) != -1 )
    {
        if( oustring.indexOf( prodName,idx ) == idx )
            off = PRODUCTNAME;
        else if( oustring.indexOf( prodVersion,idx ) == idx )
            off = PRODUCTVERSION;
        else if( oustring.indexOf( vendName,idx ) == idx )
            off = VENDORNAME;
        else if( oustring.indexOf( vendVersion,idx ) == idx )
            off = VENDORVERSION;
        else if( oustring.indexOf( vendShort,idx ) == idx )
            off = VENDORSHORT;
        else
            off = -1;

        if( off != -1 )
        {
            if( ! cap )
            {
                cap = true;
                aStrBuf.ensureCapacity( 256 );
            }

            aStrBuf.append( &oustring.getStr()[k],idx - k );
            aStrBuf.append( m_vReplacement[off] );
            k = idx + m_vAdd[off];
        }
    }

    if( cap )
    {
        if( k < oustring.getLength() )
            aStrBuf.append( &oustring.getStr()[k],oustring.getLength()-k );
        oustring = aStrBuf.makeStringAndClear();
    }
}

// TVRead

TVRead::TVRead( const ConfigData& configData,TVDom* tvDom )
{
    if( ! tvDom )
        return;

    Title = tvDom->title;
    configData.replaceName( Title );
    if( tvDom->isLeaf() )
    {
        TargetURL = ( tvDom->getTargetURL() + configData.appendix );
        if( !tvDom->anchor.isEmpty() )
            TargetURL += "#" + tvDom->anchor;
    }
    else
        Children = new TVChildTarget( configData,tvDom );
}

TVRead::~TVRead()
{
}

// XNameAccess

Any SAL_CALL
TVRead::getByName( const OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    bool found( true );
    Any aAny;
    if( aName == "Title" )
        aAny <<= Title;
    else if( aName == "TargetURL" )
        aAny <<= TargetURL;
    else if( aName == "Children" )
    {
        cppu::OWeakObject* p = Children.get();
        aAny <<= Reference< XInterface >( p );
    }
    else
        found = false;

    if( found )
        return aAny;

    throw NoSuchElementException();
}

Sequence< OUString > SAL_CALL
TVRead::getElementNames( )
    throw( RuntimeException, std::exception )
{
    Sequence< OUString > seq( 3 );

    seq[0] = "Title";
    seq[1] = "TargetURL";
    seq[2] = "Children";

    return seq;
}

sal_Bool SAL_CALL
TVRead::hasByName( const OUString& aName )
    throw( RuntimeException, std::exception )
{
    if( aName == "Title"     ||
        aName == "TargetURL" ||
        aName == "Children" )
        return true;

    return false;
}

// XHierarchicalNameAccess

Any SAL_CALL
TVRead::getByHierarchicalName( const OUString& aName )
    throw( NoSuchElementException,
           RuntimeException, std::exception )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1  &&
        aName.copy( 0,idx ) == "Children" )
        return Children->getByHierarchicalName( aName.copy( 1 + idx ) );

    return getByName( aName );
}

sal_Bool SAL_CALL
TVRead::hasByHierarchicalName( const OUString& aName )
    throw( RuntimeException, std::exception )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1  &&
        aName.copy( 0,idx ) == "Children" )
        return Children->hasByHierarchicalName( aName.copy( 1 + idx ) );

    return hasByName( aName );
}

/**************************************************************************/
/*                                                                        */
/*                      TVChildTarget                                     */
/*                                                                        */
/**************************************************************************/

extern "C" void start_handler(void *userData,
                   const XML_Char *name,
                   const XML_Char **atts)
{
    TVDom::Kind kind;

    if( strcmp( name,"help_section" ) == 0  ||
        strcmp( name,"node" ) == 0 )
        kind = TVDom::tree_node;
    else if( strcmp( name,"topic" ) == 0 )
        kind = TVDom::tree_leaf;
    else
        return;

    TVDom **tvDom = static_cast< TVDom** >( userData );
    TVDom  *p;
    p = *tvDom;

    *tvDom = p->newChild();
    p = *tvDom;

    p->setKind( kind );
    while( *atts )
    {
        if( strcmp( *atts,"application" ) == 0 )
            p->setApplication( *(atts+1) );
        else if( strcmp( *atts,"title" ) == 0 )
            p->setTitle( *(atts+1) );
        else if( strcmp( *atts,"id" ) == 0 )
            p->setId( *(atts+1) );
        else if( strcmp( *atts,"anchor" ) == 0 )
            p->setAnchor( *(atts+1) );

        atts+=2;
    }
}

extern "C" void end_handler(void *userData,
                 const XML_Char *name )
{
    (void)name;

    TVDom **tvDom = static_cast< TVDom** >( userData );
    *tvDom = (*tvDom)->getParent();
}

extern "C" void data_handler( void *userData,
                   const XML_Char *s,
                   int len)
{
    TVDom **tvDom = static_cast< TVDom** >( userData );
    if( (*tvDom)->isLeaf() )
        (*tvDom)->setTitle( s,len );
}

TVChildTarget::TVChildTarget( const ConfigData& configData,TVDom* tvDom )
{
    Elements.resize( tvDom->children.size() );
    for( size_t i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom->children[i] );
}

TVChildTarget::TVChildTarget( const Reference< XComponentContext >& xContext )
{
    ConfigData configData = init( xContext );

    if( configData.locale.isEmpty() || configData.system.isEmpty() )
        return;

    sal_uInt64  ret,len = 0;
    int j = configData.vFileURL.size();

    TVDom tvDom;
    TVDom* pTVDom = &tvDom;

    while( j )
    {
        len = configData.vFileLen[--j];
        char* s = new char[ int(len) ];  // the buffer to hold the installed files
        osl::File aFile( configData.vFileURL[j] );
        aFile.open( osl_File_OpenFlag_Read );
        aFile.read( s,len,ret );
        aFile.close();

        XML_Parser parser = XML_ParserCreate( nullptr );
        XML_SetElementHandler( parser,
                               start_handler,
                               end_handler );
        XML_SetCharacterDataHandler( parser,
                                     data_handler);
        XML_SetUserData( parser,&pTVDom ); // does not return this

        XML_Status const parsed = XML_Parse(parser, s, int(len), j==0);
        SAL_WARN_IF(XML_STATUS_ERROR == parsed, "xmlhelp",
                "TVChildTarget::TVChildTarget(): Tree file parsing failed");

        XML_ParserFree( parser );
        delete[] s;

        Check(pTVDom);
    }
    // now TVDom holds the relevant information

    Elements.resize( tvDom.children.size() );
    for( size_t i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom.children[i] );
}

TVChildTarget::~TVChildTarget()
{
}

void TVChildTarget::Check(TVDom* tvDom)
{
    if (tvDom->children.empty())
    {
        return;
    }

        unsigned i = 0;
        bool h = false;

        while((i<tvDom->children.size()-1) && (!h))
        {
            if (((tvDom->children[i])->application == (tvDom->children[tvDom->children.size()-1])->application) &&
                ((tvDom->children[i])->id == (tvDom->children[tvDom->children.size()-1])->id))
            {
                TVDom* p = tvDom->children[tvDom->children.size()-1];

                for(TVDom* k : p->children)
                    if (!SearchAndInsert(k, tvDom->children[i]))
                        tvDom->children[i]->newChild(k);

                tvDom->children.pop_back();
                h = true;
            }
            ++i;
        }
}

bool TVChildTarget::SearchAndInsert(TVDom* p, TVDom* tvDom)
{
    if (p->isLeaf()) return false;

    bool h = false;
    sal_Int32 max = 0;

    std::vector< TVDom* >::iterator max_It, i;
    max_It = tvDom->children.begin();

    sal_Int32 c_int;
    sal_Int32 p_int = p->id.toInt32();

    for(i = tvDom->children.begin(); i!=tvDom->children.end(); ++i)
        if (!((*i)->isLeaf()) &&
            ((*i)->id.getLength() == p->id.getLength()) &&
            (p->id.replaceAt((*i)->parent->id.getLength(), p->id.getLength()-(*i)->parent->id.getLength(), "") == (*i)->parent->id))      //prefix check
        {
            h = true;
            c_int = (*i)->id.toInt32();

            if (p_int==c_int)
            {
                (*(tvDom->children.insert(i+1, p)))->parent = tvDom;
                return true;
            }
            else if(c_int>max && c_int < p_int)
            {
                max = c_int;
                max_It = i+1;
            }
        }
    if (h) (*(tvDom->children.insert(max_It, p)))->parent = tvDom;
    else
    {
        i = tvDom->children.begin();
        while ((i!=tvDom->children.end()) && (!h))
        {
            h = SearchAndInsert(p, *i);
            ++i;
        }
    }
    return h;
}

Any SAL_CALL
TVChildTarget::getByName( const OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        throw NoSuchElementException();

    cppu::OWeakObject* p = Elements[idx].get();
    return Any( Reference< XInterface >( p ) );
}

Sequence< OUString > SAL_CALL
TVChildTarget::getElementNames( )
    throw( RuntimeException, std::exception )
{
    Sequence< OUString > seq( Elements.size() );
    for( size_t i = 0; i < Elements.size(); ++i )
        seq[i] = OUString::number( 1+i );

    return seq;
}

sal_Bool SAL_CALL
TVChildTarget::hasByName( const OUString& aName )
    throw( RuntimeException, std::exception )
{
    OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        return false;

    return true;
}

// XHierarchicalNameAccess

Any SAL_CALL
TVChildTarget::getByHierarchicalName( const OUString& aName )
    throw( NoSuchElementException,
           RuntimeException, std::exception )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1 )
    {
        OUString num( aName.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;

        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            throw NoSuchElementException();

        return Elements[pref]->getByHierarchicalName( aName.copy( 1 + idx ) );
    }
    else
        return getByName( aName );
}

sal_Bool SAL_CALL
TVChildTarget::hasByHierarchicalName( const OUString& aName )
    throw( RuntimeException, std::exception )
{
    sal_Int32 idx;

    if( ( idx = aName.indexOf( '/' ) ) != -1 )
    {
        OUString num( aName.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;
        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            return false;

        return Elements[pref]->hasByHierarchicalName( aName.copy( 1 + idx ) );
    }
    else
        return hasByName( aName );
}

ConfigData TVChildTarget::init( const Reference< XComponentContext >& xContext )
{
    ConfigData configData;
    Reference< XMultiServiceFactory > sProvider( getConfiguration(xContext) );

    /**********************************************************************/
    /*                       reading Office.Common                        */
    /**********************************************************************/

    Reference< XHierarchicalNameAccess > xHierAccess( getHierAccess( sProvider,
                                                                     "org.openoffice.Office.Common" ) );
    OUString system( getKey( xHierAccess,"Help/System" ) );
    bool showBasic( getBooleanKey(xHierAccess,"Help/ShowBasic") );
    OUString instPath( getKey( xHierAccess,"Path/Current/Help" ) );
    if( instPath.isEmpty() )
      // try to determine path from default
      instPath = "$(instpath)/help";

    // replace anything like $(instpath);
    subst( instPath );

    /**********************************************************************/
    /*                       reading setup                                */
    /**********************************************************************/

    xHierAccess = getHierAccess( sProvider,
                                 "org.openoffice.Setup" );

    OUString setupversion( getKey( xHierAccess,"Product/ooSetupVersion" ) );
    OUString setupextension;

    try
    {
        Reference< lang::XMultiServiceFactory > xConfigProvider = theDefaultProvider::get( xContext );

        uno::Sequence < uno::Any > lParams(1);
        beans::PropertyValue                       aParam ;
        aParam.Name    = "nodepath";
        aParam.Value <<= OUString("/org.openoffice.Setup/Product");
        lParams[0] = uno::makeAny(aParam);

        // open it
        uno::Reference< uno::XInterface > xCFG( xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess",
                    lParams) );

        uno::Reference< container::XNameAccess > xDirectAccess(xCFG, uno::UNO_QUERY);
        uno::Any aRet = xDirectAccess->getByName("ooSetupExtension");

        aRet >>= setupextension;
    }
    catch ( uno::Exception& )
    {
    }

    OUString productVersion( setupversion + " " + setupextension );
    OUString locale( getKey( xHierAccess,"L10N/ooLocale" ) );

    // Determine fileurl from url and locale
    OUString url;
    osl::FileBase::RC errFile = osl::FileBase::getFileURLFromSystemPath( instPath,url );
    if( errFile != osl::FileBase::E_None ) return configData;
    if( !url.endsWith("/") )
        url += "/";
    OUString ret;
    sal_Int32 idx;
    osl::DirectoryItem aDirItem;
    if( osl::FileBase::E_None == osl::DirectoryItem::get( url + locale,aDirItem ) )
        ret = locale;
    else if( ( ( idx = locale.indexOf( '-' ) ) != -1 ||
               ( idx = locale.indexOf( '_' ) ) != -1 ) &&
             osl::FileBase::E_None == osl::DirectoryItem::get( url + locale.copy( 0,idx ),
                                                               aDirItem ) )
        ret = locale.copy( 0,idx );
    else
        {
        locale = "en-US";
        ret = "en";
        }
    url = url + ret;

    // first of all, try do determine whether there are any *.tree files present

    // Start with extensions to set them at the end of the list
    TreeFileIterator aTreeIt( locale );
    OUString aTreeFile;
    sal_Int32 nFileSize;
    while( !(aTreeFile = aTreeIt.nextTreeFile( nFileSize ) ).isEmpty() )
    {
        configData.vFileLen.push_back( nFileSize );
        configData.vFileURL.push_back( aTreeFile );
    }

    osl::Directory aDirectory( url );
    osl::FileStatus aFileStatus(
            osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_FileURL );
    if( osl::Directory::E_None == aDirectory.open() )
    {
        OUString aFileUrl, aFileName;
        while( aDirectory.getNextItem( aDirItem ) == osl::FileBase::E_None &&
               aDirItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None &&
               aFileStatus.isValid( osl_FileStatus_Mask_FileURL ) &&
               aFileStatus.isValid( osl_FileStatus_Mask_FileName ) )
          {
            aFileUrl = aFileStatus.getFileURL();
            aFileName = aFileStatus.getFileName();
            int idx_ = aFileName.lastIndexOf( '.' );
            if( idx_ == -1 )
              continue;

            const sal_Unicode* str = aFileName.getStr();

            if( aFileName.getLength() == idx_ + 5                   &&
                ( str[idx_ + 1] == 't' || str[idx_ + 1] == 'T' )    &&
                ( str[idx_ + 2] == 'r' || str[idx_ + 2] == 'R' )    &&
                ( str[idx_ + 3] == 'e' || str[idx_ + 3] == 'E' )    &&
                ( str[idx_ + 4] == 'e' || str[idx_ + 4] == 'E' ) )
              {
                OUString baseName = aFileName.copy(0,idx_).toAsciiLowerCase();
                if(! showBasic && baseName == "sbasic" )
                  continue;
                osl::File aFile( aFileUrl );
                if( osl::FileBase::E_None == aFile.open( osl_File_OpenFlag_Read ) )
                {
                    // use the file size, not aFileStatus size, in case the
                    // tree file is a symlink
                    sal_uInt64 nSize;
                    aFile.getSize( nSize );
                    configData.vFileLen.push_back( nSize );
                    configData.vFileURL.push_back( aFileUrl );
                    aFile.close();
                }
              }
          }
        aDirectory.close();
    }

    configData.m_vAdd[0] = 12;
    configData.m_vAdd[1] = 15;
    configData.m_vAdd[2] = 11;
    configData.m_vAdd[3] = 14;
    configData.m_vAdd[4] = 12;
    configData.m_vReplacement[0] = utl::ConfigManager::getProductName();
    configData.m_vReplacement[1] = productVersion;
    // m_vReplacement[2...4] (vendorName/-Version/-Short) are empty strings

       configData.system = system;
    configData.locale = locale;
    configData.appendix =
        "?Language=" +
        configData.locale +
        "&System=" +
        configData.system +
        "&UseDB=no";

    return configData;
}

Reference< XMultiServiceFactory >
TVChildTarget::getConfiguration(const Reference< XComponentContext >& rxContext)
{
    Reference< XMultiServiceFactory > xProvider;
    if( rxContext.is() )
    {
        try
        {
            xProvider = theDefaultProvider::get( rxContext );
        }
        catch( const css::uno::Exception& )
        {
            OSL_ENSURE( xProvider.is(),"can not instantiate configuration" );
        }
    }

    return xProvider;
}

Reference< XHierarchicalNameAccess >
TVChildTarget::getHierAccess( const Reference< XMultiServiceFactory >& sProvider,
                              const char* file )
{
    Reference< XHierarchicalNameAccess > xHierAccess;

    if( sProvider.is() )
    {
        Sequence< Any > seq(1);
        OUString sReaderService =
            OUString( "com.sun.star.configuration.ConfigurationAccess" );

        seq[0] <<= OUString::createFromAscii( file );

        try
        {
            xHierAccess =
                Reference< XHierarchicalNameAccess >
                ( sProvider->createInstanceWithArguments( sReaderService,seq ),
                  UNO_QUERY );
        }
        catch( const css::uno::Exception& )
        {
        }
    }

    return xHierAccess;
}

OUString
TVChildTarget::getKey( const Reference< XHierarchicalNameAccess >& xHierAccess,
                       const char* key )
{
    OUString instPath;
    if( xHierAccess.is() )
    {
        Any aAny;
        try
        {
            aAny =
                xHierAccess->getByHierarchicalName( OUString::createFromAscii( key ) );
        }
        catch( const css::container::NoSuchElementException& )
        {
        }
        aAny >>= instPath;
    }
    return instPath;
}

bool
TVChildTarget::getBooleanKey(const Reference<
                             XHierarchicalNameAccess >& xHierAccess,
                             const char* key)
{
  bool ret = false;
  if( xHierAccess.is() )
    {
      Any aAny;
      try
        {
          aAny =
            xHierAccess->getByHierarchicalName(
                                               OUString::createFromAscii(key));
        }
      catch( const css::container::NoSuchElementException& )
        {
        }
      aAny >>= ret;
    }
  return ret;
}

void TVChildTarget::subst( OUString& instpath )
{
    SvtPathOptions aOptions;
    instpath = aOptions.SubstituteVariable( instpath );
}

// class ExtensionIteratorBase

static const char aHelpMediaType[] = "application/vnd.sun.star.help";

ExtensionIteratorBase::ExtensionIteratorBase( const OUString& aLanguage )
        : m_eState( IteratorState::UserExtensions )
        , m_aLanguage( aLanguage )
{
    init();
}

void ExtensionIteratorBase::init()
{
    m_xContext = ::comphelper::getProcessComponentContext();
    if( !m_xContext.is() )
    {
        throw RuntimeException( "ExtensionIteratorBase::init(), no XComponentContext" );
    }

    m_xSFA = ucb::SimpleFileAccess::create(m_xContext);

    m_bUserPackagesLoaded = false;
    m_bSharedPackagesLoaded = false;
    m_bBundledPackagesLoaded = false;
    m_iUserPackage = 0;
    m_iSharedPackage = 0;
    m_iBundledPackage = 0;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetHelpPackageFromPackage
    ( const Reference< deployment::XPackage >& xPackage, Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    o_xParentPackageBundle.clear();

    Reference< deployment::XPackage > xHelpPackage;
    if( !xPackage.is() )
        return xHelpPackage;

    // Check if parent package is registered
    beans::Optional< beans::Ambiguous<sal_Bool> > option( xPackage->isRegistered
        ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() ) );
    bool bRegistered = false;
    if( option.IsPresent )
    {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if( !reg.IsAmbiguous && reg.Value )
            bRegistered = true;
    }
    if( !bRegistered )
        return xHelpPackage;

    if( xPackage->isBundle() )
    {
        Sequence< Reference< deployment::XPackage > > aPkgSeq = xPackage->getBundle
            ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() );
        sal_Int32 nPkgCount = aPkgSeq.getLength();
        const Reference< deployment::XPackage >* pSeq = aPkgSeq.getConstArray();
        for( sal_Int32 iPkg = 0 ; iPkg < nPkgCount ; ++iPkg )
        {
            const Reference< deployment::XPackage > xSubPkg = pSeq[ iPkg ];
            const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xSubPkg->getPackageType();
            OUString aMediaType = xPackageTypeInfo->getMediaType();
            if( aMediaType == aHelpMediaType )
            {
                xHelpPackage = xSubPkg;
                o_xParentPackageBundle = xPackage;
                break;
            }
        }
    }
    else
    {
        const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xPackage->getPackageType();
        OUString aMediaType = xPackageTypeInfo->getMediaType();
        if( aMediaType == aHelpMediaType )
            xHelpPackage = xPackage;
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextUserHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bUserPackagesLoaded )
    {
        Reference< XPackageManager > xUserManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager("user");
        m_aUserPackagesSeq = xUserManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bUserPackagesLoaded = true;
    }

    if( m_iUserPackage == m_aUserPackagesSeq.getLength() )
    {
        m_eState = IteratorState::SharedExtensions;       // Later: SHARED_MODULE
    }
    else
    {
        const Reference< deployment::XPackage >* pUserPackages = m_aUserPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pUserPackages[ m_iUserPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextUserHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextSharedHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bSharedPackagesLoaded )
    {
        Reference< XPackageManager > xSharedManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager("shared");
        m_aSharedPackagesSeq = xSharedManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bSharedPackagesLoaded = true;
    }

    if( m_iSharedPackage == m_aSharedPackagesSeq.getLength() )
    {
        m_eState = IteratorState::BundledExtensions;
    }
    else
    {
        const Reference< deployment::XPackage >* pSharedPackages = m_aSharedPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pSharedPackages[ m_iSharedPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextSharedHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextBundledHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bBundledPackagesLoaded )
    {
        Reference< XPackageManager > xBundledManager =
            thePackageManagerFactory::get( m_xContext )->getPackageManager("bundled");
        m_aBundledPackagesSeq = xBundledManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bBundledPackagesLoaded = true;
    }

    if( m_iBundledPackage == m_aBundledPackagesSeq.getLength() )
    {
        m_eState = IteratorState::EndReached;
    }
    else
    {
        const Reference< deployment::XPackage >* pBundledPackages = m_aBundledPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pBundledPackages[ m_iBundledPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextBundledHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

inline bool isLetter( sal_Unicode c )
{
    return rtl::isAsciiAlpha(c);
}

void ExtensionIteratorBase::implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
    const css::uno::Reference< css::deployment::XPackage >& xPackage )
{
    rv.clear();
    OUString aExtensionPath = xPackage->getURL();
    Sequence< OUString > aEntrySeq = m_xSFA->getFolderContents( aExtensionPath, true );

    const OUString* pSeq = aEntrySeq.getConstArray();
    sal_Int32 nCount = aEntrySeq.getLength();
    for( sal_Int32 i = 0 ; i < nCount ; ++i )
    {
        OUString aEntry = pSeq[i];
        if( m_xSFA->isFolder( aEntry ) )
        {
            sal_Int32 nLastSlash = aEntry.lastIndexOf( '/' );
            if( nLastSlash != -1 )
            {
                OUString aPureEntry = aEntry.copy( nLastSlash + 1 );

                // Check language scheme
                int nLen = aPureEntry.getLength();
                const sal_Unicode* pc = aPureEntry.getStr();
                bool bStartCanBeLanguage = ( nLen >= 2 && isLetter( pc[0] ) && isLetter( pc[1] ) );
                bool bIsLanguage = bStartCanBeLanguage &&
                    ( nLen == 2 || (nLen == 5 && pc[2] == '-' && isLetter( pc[3] ) && isLetter( pc[4] )) );
                if( bIsLanguage )
                    rv.push_back( aPureEntry );
            }
        }
    }
}

// class TreeFileIterator

OUString TreeFileIterator::nextTreeFile( sal_Int32& rnFileSize )
{
    OUString aRetFile;

    while( aRetFile.isEmpty() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }
            case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

        case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextTreeFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aRetFile;
}

OUString TreeFileIterator::expandURL( const OUString& aURL )
{
    static Reference< util::XMacroExpander > xMacroExpander;
    static Reference< uri::XUriReferenceFactory > xFac;

    osl::MutexGuard aGuard( m_aMutex );

    if( !xMacroExpander.is() || !xFac.is() )
    {
        xFac = uri::UriReferenceFactory::create( m_xContext );

        xMacroExpander = util::theMacroExpander::get(m_xContext);
     }

    OUString aRetURL = aURL;
    Reference< uri::XUriReference > uriRef;
    for (;;)
    {
        uriRef.set( xFac->parse( aRetURL ), UNO_QUERY );
        if ( uriRef.is() )
        {
            Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
            if( !sxUri.is() )
                break;

            aRetURL = sxUri->expand( xMacroExpander );
        }
    }
    return aRetURL;
}

OUString TreeFileIterator::implGetTreeFileFromPackage
    ( sal_Int32& rnFileSize, const Reference< deployment::XPackage >& xPackage )
{
    OUString aRetFile;
    OUString aLanguage = m_aLanguage;
    for( sal_Int32 iPass = 0 ; iPass < 2 ; ++iPass )
    {
        aRetFile = expandURL( xPackage->getURL() + "/" + aLanguage + "/help.tree" );
        if( iPass == 0 )
        {
            if( m_xSFA->exists( aRetFile ) )
                break;

            ::std::vector< OUString > av;
            implGetLanguageVectorFromPackage( av, xPackage );
            ::std::vector< OUString >::const_iterator pFound = LanguageTag::getFallback( av, m_aLanguage );
            if( pFound != av.end() )
                aLanguage = *pFound;
        }
    }

    rnFileSize = 0;
    if( m_xSFA->exists( aRetFile ) )
        rnFileSize = m_xSFA->getSize( aRetFile );
    else
        aRetFile.clear();

    return aRetFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

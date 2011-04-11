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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"

#include <string.h>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "tvread.hxx"
#include <expat.h>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <comphelper/locale.hxx>

namespace treeview {


    class TVDom
    {
        friend class TVChildTarget;
        friend class TVRead;

    public:

        TVDom( TVDom* arent = 0 )
            : kind( other ),
              parent( arent ),
              childs( 0 )
        {
        }

        ~TVDom()
        {
            for( unsigned i = 0; i < childs.size(); ++i )
                delete childs[i];
        }


        TVDom* newChild()
        {
            childs.push_back( new TVDom( this ) );
            return childs.back();
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
        Kind getKind( ) const { return kind; }


        void setApplication( const char* appl )
        {
            application = rtl::OUString( (sal_Char*)(appl),
                                         strlen( appl ),
                                         RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const char* itle )
        {
            title += rtl::OUString( (sal_Char*)(itle),
                                    strlen( itle ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const XML_Char* itle,int len )
        {
            title += rtl::OUString( (sal_Char*)(itle),
                                    len,
                                    RTL_TEXTENCODING_UTF8 );
        }

        void setId( const char* d )
        {
            id = rtl::OUString( (sal_Char*)(d),
                                strlen( d ),
                                RTL_TEXTENCODING_UTF8 );
        }

        void setAnchor( const char* nchor )
        {
            anchor = rtl::OUString( (sal_Char*)(nchor),
                                    strlen( nchor ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        rtl::OUString getTargetURL()
        {
            if( ! targetURL.getLength() )
            {
                sal_Int32 len;
                for ( const TVDom* p = this;; p = p->parent )
                {
                    len = p->application.getLength();
                    if ( len != 0 )
                        break;
                }

                rtl::OUStringBuffer strBuff( 22 + len + id.getLength() );
                strBuff.appendAscii(
                                    "vnd.sun.star.help://"
                                    ).append(id);

                targetURL = strBuff.makeStringAndClear();
            }

            return targetURL;
        }

    private:

        Kind   kind;
        rtl::OUString  application;
        rtl::OUString  title;
        rtl::OUString  id;
        rtl::OUString  anchor;
        rtl::OUString  targetURL;

        TVDom *parent;
        std::vector< TVDom* > childs;
    };

}



using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::deployment;


ConfigData::ConfigData()
    : prodName(RTL_CONSTASCII_USTRINGPARAM("%PRODUCTNAME")),
      prodVersion(RTL_CONSTASCII_USTRINGPARAM("%PRODUCTVERSION")),
      vendName(RTL_CONSTASCII_USTRINGPARAM("%VENDORNAME")),
      vendVersion(RTL_CONSTASCII_USTRINGPARAM("%VENDORVERSION")),
      vendShort(RTL_CONSTASCII_USTRINGPARAM("%VENDORSHORT"))
{
}

void SAL_CALL ConfigData::replaceName( rtl::OUString& oustring ) const
{
    sal_Int32 idx = -1,k = 0,off;
    bool cap = false;
    rtl::OUStringBuffer aStrBuf( 0 );

    while( ( idx = oustring.indexOf( sal_Unicode('%'),++idx ) ) != -1 )
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




//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////


void SAL_CALL
TVBase::acquire(
    void )
    throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
TVBase::release(
              void )
  throw()
{
  OWeakObject::release();
}


Any SAL_CALL
TVBase::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XNameAccess*, this ),
                                     SAL_STATIC_CAST( XHierarchicalNameAccess*, this ),
                                     SAL_STATIC_CAST( XChangesNotifier*, this ),
                                     SAL_STATIC_CAST( XComponent*, this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


////////////////////////////////////////////////////////////////////////////////
//
// XTypeProvider methods.

XTYPEPROVIDER_IMPL_5( TVBase,
                         XTypeProvider,
                      XNameAccess,
                      XHierarchicalNameAccess,
                      XChangesNotifier,
                      XComponent );






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
        if( tvDom->anchor.getLength() )
            TargetURL += ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "#" )) +
                           tvDom->anchor );
    }
    else
        Children = new TVChildTarget( configData,tvDom );
}



TVRead::~TVRead()
{
}






// XNameAccess

Any SAL_CALL
TVRead::getByName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException )
{
    bool found( true );
    Any aAny;
    if( aName.compareToAscii( "Title" ) == 0 )
        aAny <<= Title;
    else if( aName.compareToAscii( "TargetURL" ) == 0 )
        aAny <<= TargetURL;
    else if( aName.compareToAscii( "Children" ) == 0 )
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




Sequence< rtl::OUString > SAL_CALL
TVRead::getElementNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( 3 );

    seq[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ));
    seq[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ));
    seq[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Children" ));

    return seq;
}



sal_Bool SAL_CALL
TVRead::hasByName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    if( aName.compareToAscii( "Title" ) == 0        ||
        aName.compareToAscii( "TargetURL" ) == 0    ||
        aName.compareToAscii( "Children" ) == 0 )
        return true;

    return false;
}


// XHierarchicalNameAccess

Any SAL_CALL
TVRead::getByHierarchicalName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

    if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1  &&
        name.copy( 0,idx ).compareToAscii( "Children" ) == 0 )
        return Children->getByHierarchicalName( name.copy( 1 + idx ) );

    return getByName( name );
}




sal_Bool SAL_CALL
TVRead::hasByHierarchicalName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

       if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1  &&
        name.copy( 0,idx ).compareToAscii( "Children" ) == 0 )
        return Children->hasByHierarchicalName( name.copy( 1 + idx ) );

    return hasByName( name );
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
    Elements.resize( tvDom->childs.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom->childs[i] );
}





TVChildTarget::TVChildTarget( const Reference< XMultiServiceFactory >& xMSF )
{
    ConfigData configData = init( xMSF );

    if( ! configData.locale.getLength()  ||
        ! configData.system.getLength() )
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

        XML_Parser parser = XML_ParserCreate( 0 );
        XML_SetElementHandler( parser,
                               start_handler,
                               end_handler );
        XML_SetCharacterDataHandler( parser,
                                     data_handler);
        XML_SetUserData( parser,&pTVDom ); // does not return this

        int parsed = XML_Parse( parser,s,int( len ),j==0 );
        (void)parsed;
        OSL_ENSURE( parsed, "TVChildTarget::TVChildTarget(): Tree file parsing failed" );

        XML_ParserFree( parser );
        delete[] s;
    }

    // now TVDom holds the relevant information

    Elements.resize( tvDom.childs.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom.childs[i] );
}


TVChildTarget::~TVChildTarget()
{
}



Any SAL_CALL
TVChildTarget::getByName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException )
{
    rtl::OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        throw NoSuchElementException();

    Any aAny;
    cppu::OWeakObject* p = Elements[idx].get();
    aAny <<= Reference< XInterface >( p );
    return aAny;
}




Sequence< rtl::OUString > SAL_CALL
TVChildTarget::getElementNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( Elements.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        seq[i] = rtl::OUString::valueOf( sal_Int32( 1+i ) );

    return seq;
}



sal_Bool SAL_CALL
TVChildTarget::hasByName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    rtl::OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        return false;

    return true;
}



// XHierarchicalNameAccess

Any SAL_CALL
TVChildTarget::getByHierarchicalName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

    if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString num( name.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;

        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            throw NoSuchElementException();

        return Elements[pref]->getByHierarchicalName( name.copy( 1 + idx ) );
    }
    else
        return getByName( name );
}



sal_Bool SAL_CALL
TVChildTarget::hasByHierarchicalName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

       if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString num( name.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;
        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            return false;

        return Elements[pref]->hasByHierarchicalName( name.copy( 1 + idx ) );
    }
    else
        return hasByName( name );
}






ConfigData TVChildTarget::init( const Reference< XMultiServiceFactory >& xSMgr )
{
    ConfigData configData;
    Reference< XMultiServiceFactory >  sProvider( getConfiguration(xSMgr) );

    /**********************************************************************/
    /*                       reading Office.Common                        */
    /**********************************************************************/

    Reference< XHierarchicalNameAccess > xHierAccess( getHierAccess( sProvider,
                                                                     "org.openoffice.Office.Common" ) );
    rtl::OUString system( getKey( xHierAccess,"Help/System" ) );
    sal_Bool showBasic( getBooleanKey(xHierAccess,"Help/ShowBasic") );
    rtl::OUString instPath( getKey( xHierAccess,"Path/Current/Help" ) );
    if( ! instPath.getLength() )
      // try to determine path from default
      instPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "$(instpath)/help" ));

    // replace anything like $(instpath);
    subst( xSMgr,instPath );

    /**********************************************************************/
    /*                       reading setup                                */
    /**********************************************************************/

    rtl::OUString productName;
    ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME) >>= productName;

    xHierAccess = getHierAccess( sProvider,
                                 "org.openoffice.Setup" );

    rtl::OUString setupversion( getKey( xHierAccess,"Product/ooSetupVersion" ) );
    rtl::OUString setupextension;

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
              xSMgr ->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider"))), uno::UNO_QUERY_THROW);

        uno::Sequence < uno::Any > lParams(1);
        beans::PropertyValue                       aParam ;
        aParam.Name    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath"));
        aParam.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Product"));
        lParams[0] = uno::makeAny(aParam);

        // open it
        uno::Reference< uno::XInterface > xCFG( xConfigProvider->createInstanceWithArguments(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                    lParams) );

        uno::Reference< container::XNameAccess > xDirectAccess(xCFG, uno::UNO_QUERY);
        uno::Any aRet = xDirectAccess->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupExtension")));

        aRet >>= setupextension;
    }
    catch ( uno::Exception& )
    {
    }

    rtl::OUString productVersion( setupversion +
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( " " )) +
                                  setupextension );
    rtl::OUString locale( getKey( xHierAccess,"L10N/ooLocale" ) );


    // Determine fileurl from url and locale
    rtl::OUString url;
    osl::FileBase::RC errFile = osl::FileBase::getFileURLFromSystemPath( instPath,url );
    if( errFile != osl::FileBase::E_None ) return configData;
    if( url.lastIndexOf( sal_Unicode( '/' ) ) != url.getLength() - 1 )
        url += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/" ));
    rtl::OUString ret;
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
        locale = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "en-US" ));
        ret = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
        }
    url = url + ret;

    // first of all, try do determine whether there are any *.tree files present

    // Start with extensions to set them at the end of the list
    TreeFileIterator aTreeIt( locale );
    rtl::OUString aTreeFile;
    sal_Int32 nFileSize;
    while( (aTreeFile = aTreeIt.nextTreeFile( nFileSize ) ).getLength() > 0 )
    {
        configData.vFileLen.push_back( nFileSize );
        configData.vFileURL.push_back( aTreeFile );
    }

    osl::Directory aDirectory( url );
    osl::FileStatus aFileStatus( FileStatusMask_FileName | FileStatusMask_FileSize | FileStatusMask_FileURL );
    if( osl::Directory::E_None == aDirectory.open() )
    {
        int idx_ = 0;
        rtl::OUString aFileUrl, aFileName;
        while( aDirectory.getNextItem( aDirItem ) == osl::FileBase::E_None &&
               aDirItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None &&
               aFileStatus.isValid( FileStatusMask_FileURL ) &&
               aFileStatus.isValid( FileStatusMask_FileName ) )
          {
            aFileUrl = aFileStatus.getFileURL();
            aFileName = aFileStatus.getFileName();
            idx_ = aFileName.lastIndexOf( sal_Unicode( '.' ) );
            if( idx_ == -1 )
              continue;

            const sal_Unicode* str = aFileName.getStr();

            if( aFileName.getLength() == idx_ + 5                   &&
                ( str[idx_ + 1] == 't' || str[idx_ + 1] == 'T' )    &&
                ( str[idx_ + 2] == 'r' || str[idx_ + 2] == 'R' )    &&
                ( str[idx_ + 3] == 'e' || str[idx_ + 3] == 'E' )    &&
                ( str[idx_ + 4] == 'e' || str[idx_ + 4] == 'E' ) )
              {
                OSL_ENSURE( aFileStatus.isValid( FileStatusMask_FileSize ),
                            "invalid file size" );

                rtl::OUString baseName = aFileName.copy(0,idx_).toAsciiLowerCase();
                if(! showBasic && baseName.compareToAscii("sbasic") == 0 )
                  continue;
                osl::File aFile( aFileUrl );
                if( osl::FileBase::E_None == aFile.open( osl_File_OpenFlag_Read ) )
                {
                    sal_uInt64 nSize;
                    aFile.getSize( nSize );
                    configData.vFileLen.push_back( aFileStatus.getFileSize() );
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
    configData.m_vReplacement[0] = productName;
    configData.m_vReplacement[1] = productVersion;
    // m_vReplacement[2...4] (vendorName/-Version/-Short) are empty strings

       configData.system = system;
    configData.locale = locale;
    configData.appendix =
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "?Language=" )) +
        configData.locale +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "&System=" )) +
        configData.system +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "&UseDB=no" )) ;

    return configData;
}









Reference< XMultiServiceFactory >
TVChildTarget::getConfiguration(const Reference< XMultiServiceFactory >& m_xSMgr) const
{
    Reference< XMultiServiceFactory > sProvider;
    if( m_xSMgr.is() )
    {
        try
        {
            rtl::OUString sProviderService =
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ));
            sProvider =
                Reference< XMultiServiceFactory >(
                    m_xSMgr->createInstance( sProviderService ),
                    UNO_QUERY );
        }
        catch( const com::sun::star::uno::Exception& )
        {
            OSL_ENSURE( sProvider.is(),"cant instantiate configuration" );
        }
    }

    return sProvider;
}



Reference< XHierarchicalNameAccess >
TVChildTarget::getHierAccess( const Reference< XMultiServiceFactory >& sProvider,
                              const char* file ) const
{
    Reference< XHierarchicalNameAccess > xHierAccess;

    if( sProvider.is() )
    {
        Sequence< Any > seq(1);
        rtl::OUString sReaderService =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" ));

        seq[0] <<= rtl::OUString::createFromAscii( file );

        try
        {
            xHierAccess =
                Reference< XHierarchicalNameAccess >
                ( sProvider->createInstanceWithArguments( sReaderService,seq ),
                  UNO_QUERY );
        }
        catch( const com::sun::star::uno::Exception& )
        {
        }
    }

    return xHierAccess;
}



rtl::OUString
TVChildTarget::getKey( const Reference< XHierarchicalNameAccess >& xHierAccess,
                       const char* key ) const
{
    rtl::OUString instPath;
    if( xHierAccess.is() )
    {
        Any aAny;
        try
        {
            aAny =
                xHierAccess->getByHierarchicalName( rtl::OUString::createFromAscii( key ) );
        }
        catch( const com::sun::star::container::NoSuchElementException& )
        {
        }
        aAny >>= instPath;
    }
    return instPath;
}


sal_Bool
TVChildTarget::getBooleanKey(const Reference<
                             XHierarchicalNameAccess >& xHierAccess,
                             const char* key) const
{
  sal_Bool ret = sal_False;
  if( xHierAccess.is() )
    {
      Any aAny;
      try
        {
          aAny =
            xHierAccess->getByHierarchicalName(
                                               rtl::OUString::createFromAscii(key));
        }
      catch( const com::sun::star::container::NoSuchElementException& )
        {
        }
      aAny >>= ret;
    }
  return ret;
}


void TVChildTarget::subst( const Reference< XMultiServiceFactory >& m_xSMgr,
                           rtl::OUString& instpath ) const
{
    Reference< XConfigManager >  xCfgMgr;
    if( m_xSMgr.is() )
    {
        try
        {
            xCfgMgr =
                Reference< XConfigManager >(
                    m_xSMgr->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" )) ),
                    UNO_QUERY );
        }
        catch( const com::sun::star::uno::Exception& )
        {
            OSL_ENSURE( xCfgMgr.is()," cant instantiate the special config manager " );
        }
    }

    OSL_ENSURE( xCfgMgr.is(), "specialconfigmanager not found\n" );

    if( xCfgMgr.is() )
        instpath = xCfgMgr->substituteVariables( instpath );
}


//===================================================================
// class ExtensionIteratorBase

static rtl::OUString aSlash(RTL_CONSTASCII_USTRINGPARAM("/"));
static rtl::OUString aHelpFilesBaseName(RTL_CONSTASCII_USTRINGPARAM("help"));
static rtl::OUString aHelpMediaType(RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.help"));

ExtensionIteratorBase::ExtensionIteratorBase( const rtl::OUString& aLanguage )
        : m_eState( USER_EXTENSIONS )
        , m_aLanguage( aLanguage )
{
    init();
}

void ExtensionIteratorBase::init()
{
    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XPropertySet > xProps( xFactory, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        xProps->getPropertyValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= m_xContext;
        OSL_ASSERT( m_xContext.is() );
    }
    if( !m_xContext.is() )
    {
        throw RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ExtensionIteratorBase::init(), no XComponentContext" )),
            Reference< XInterface >() );
    }

    Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );
    m_xSFA = Reference< ucb::XSimpleFileAccess >(
        xSMgr->createInstanceWithContext( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )),
        m_xContext ), UNO_QUERY_THROW );

    m_bUserPackagesLoaded = false;
    m_bSharedPackagesLoaded = false;
    m_bBundledPackagesLoaded = false;
    m_iUserPackage = 0;
    m_iSharedPackage = 0;
    m_iBundledPackage = 0;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetHelpPackageFromPackage
    ( Reference< deployment::XPackage > xPackage, Reference< deployment::XPackage >& o_xParentPackageBundle )
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
            rtl::OUString aMediaType = xPackageTypeInfo->getMediaType();
            if( aMediaType.equals( aHelpMediaType ) )
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
        rtl::OUString aMediaType = xPackageTypeInfo->getMediaType();
        if( aMediaType.equals( aHelpMediaType ) )
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
            thePackageManagerFactory::get( m_xContext )->getPackageManager( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")) );
        m_aUserPackagesSeq = xUserManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bUserPackagesLoaded = true;
    }

    if( m_iUserPackage == m_aUserPackagesSeq.getLength() )
    {
        m_eState = SHARED_EXTENSIONS;       // Later: SHARED_MODULE
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
            thePackageManagerFactory::get( m_xContext )->getPackageManager( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("shared")) );
        m_aSharedPackagesSeq = xSharedManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bSharedPackagesLoaded = true;
    }

    if( m_iSharedPackage == m_aSharedPackagesSeq.getLength() )
    {
        m_eState = BUNDLED_EXTENSIONS;
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
            thePackageManagerFactory::get( m_xContext )->getPackageManager( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bundled")) );
        m_aBundledPackagesSeq = xBundledManager->getDeployedPackages
            ( Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );

        m_bBundledPackagesLoaded = true;
    }

    if( m_iBundledPackage == m_aBundledPackagesSeq.getLength() )
    {
        m_eState = END_REACHED;
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
    bool bLetter = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
    return bLetter;
}

void ExtensionIteratorBase::implGetLanguageVectorFromPackage( ::std::vector< ::rtl::OUString > &rv,
    com::sun::star::uno::Reference< com::sun::star::deployment::XPackage > xPackage )
{
    rv.clear();
    rtl::OUString aExtensionPath = xPackage->getURL();
    Sequence< rtl::OUString > aEntrySeq = m_xSFA->getFolderContents( aExtensionPath, true );

    const rtl::OUString* pSeq = aEntrySeq.getConstArray();
    sal_Int32 nCount = aEntrySeq.getLength();
    for( sal_Int32 i = 0 ; i < nCount ; ++i )
    {
        rtl::OUString aEntry = pSeq[i];
        if( m_xSFA->isFolder( aEntry ) )
        {
            sal_Int32 nLastSlash = aEntry.lastIndexOf( '/' );
            if( nLastSlash != -1 )
            {
                rtl::OUString aPureEntry = aEntry.copy( nLastSlash + 1 );

                // Check language sceme
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


//===================================================================
// class TreeFileIterator

rtl::OUString TreeFileIterator::nextTreeFile( sal_Int32& rnFileSize )
{
    rtl::OUString aRetFile;

    while( !aRetFile.getLength() && m_eState != END_REACHED )
    {
        switch( m_eState )
        {
            case USER_EXTENSIONS:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

            case SHARED_EXTENSIONS:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }
            case BUNDLED_EXTENSIONS:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetTreeFileFromPackage( rnFileSize, xHelpPackage );
                break;
            }

        case END_REACHED:
                OSL_FAIL( "DataBaseIterator::nextTreeFile(): Invalid case END_REACHED" );
                break;
        }
    }

    return aRetFile;
}

rtl::OUString TreeFileIterator::expandURL( const rtl::OUString& aURL )
{
    static Reference< util::XMacroExpander > xMacroExpander;
    static Reference< uri::XUriReferenceFactory > xFac;

    osl::MutexGuard aGuard( m_aMutex );

    if( !xMacroExpander.is() || !xFac.is() )
    {
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY );

        xFac = Reference< uri::XUriReferenceFactory >(
            xSMgr->createInstanceWithContext( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.uri.UriReferenceFactory")), m_xContext ) , UNO_QUERY );
        if( !xFac.is() )
        {
            throw RuntimeException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Databases::expand(), could not instatiate UriReferenceFactory." )),
                Reference< XInterface >() );
        }

        xMacroExpander = Reference< util::XMacroExpander >(
            m_xContext->getValueByName(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander" )) ),
            UNO_QUERY_THROW );
     }

    rtl::OUString aRetURL = aURL;
    if( xMacroExpander.is() )
    {
        Reference< uri::XUriReference > uriRef;
        for (;;)
        {
            uriRef = Reference< uri::XUriReference >( xFac->parse( aRetURL ), UNO_QUERY );
            if ( uriRef.is() )
            {
                Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
                if( !sxUri.is() )
                    break;

                aRetURL = sxUri->expand( xMacroExpander );
            }
        }
     }
    return aRetURL;
}

rtl::OUString TreeFileIterator::implGetTreeFileFromPackage
    ( sal_Int32& rnFileSize, Reference< deployment::XPackage > xPackage )
{
    rtl::OUString aRetFile;
    rtl::OUString aLanguage = m_aLanguage;
    for( sal_Int32 iPass = 0 ; iPass < 2 ; ++iPass )
    {
        rtl::OUStringBuffer aStrBuf;
        aStrBuf.append( xPackage->getURL() );
        aStrBuf.append( aSlash );
        aStrBuf.append( aLanguage );
        aStrBuf.append( aSlash );
        aStrBuf.append( aHelpFilesBaseName );
        aStrBuf.appendAscii( ".tree" );

        aRetFile = expandURL( aStrBuf.makeStringAndClear() );
        if( iPass == 0 )
        {
            if( m_xSFA->exists( aRetFile ) )
                break;

            ::std::vector< ::rtl::OUString > av;
            implGetLanguageVectorFromPackage( av, xPackage );
            ::std::vector< ::rtl::OUString >::const_iterator pFound = av.end();
            try
            {
                pFound = ::comphelper::Locale::getFallback( av, m_aLanguage );
            }
            catch( ::comphelper::Locale::MalFormedLocaleException& )
            {}
            if( pFound != av.end() )
                aLanguage = *pFound;
        }
    }

    rnFileSize = 0;
    if( m_xSFA->exists( aRetFile ) )
        rnFileSize = m_xSFA->getSize( aRetFile );
    else
        aRetFile = rtl::OUString();

    return aRetFile;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

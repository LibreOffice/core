/*************************************************************************
 *
 *  $RCSfile: databases.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:09:36 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <berkeleydb/db_cxx.h>
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _URLPARAMETER_HXX_
#include <provider/urlparameter.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include "inputstream.hxx"
#include <algorithm>

using namespace chelp;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;


Databases::Databases( const rtl::OUString& instPath,
                      const rtl::OUString& productName,
                      const rtl::OUString& productVersion,
                      const rtl::OUString& vendorName,
                      const rtl::OUString& vendorVersion,
                      const rtl::OUString& vendorShort,
                      const rtl::OUString& styleSheet,
                      com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xSMgr )
    : m_xSMgr( xSMgr ),
      m_nErrorDocLength( 0 ),
      m_pErrorDoc( 0 ),
      m_nCustomCSSDocLength( 0 ),
      m_pCustomCSSDoc( 0 ),
      m_aCSS(styleSheet.toAsciiLowerCase()),
      prodName( rtl::OUString::createFromAscii( "%PRODUCTNAME" ) ),
      prodVersion( rtl::OUString::createFromAscii( "%PRODUCTVERSION" ) ),
      vendName( rtl::OUString::createFromAscii( "%VENDORNAME" ) ),
      vendVersion( rtl::OUString::createFromAscii( "%VENDORVERSION" ) ),
      vendShort( rtl::OUString::createFromAscii( "%VENDORSHORT" ) )
{
    m_vAdd[0] = 12;
    m_vAdd[1] = 15;
    m_vAdd[2] = 11;
    m_vAdd[3] = 14;
    m_vAdd[4] = 12;

    m_vReplacement[0] = productName;
    m_vReplacement[1] = productVersion;
    m_vReplacement[2] = vendorName;
    m_vReplacement[3] = vendorVersion;
    m_vReplacement[4] = vendorShort;

    setInstallPath( instPath );
}


Databases::~Databases()
{
    // release stylesheet

    delete[] m_pCustomCSSDoc;

    // release errorDocument

    delete[] m_pErrorDoc;

    // unload the databases

    {
        // DatabasesTable
        DatabasesTable::iterator it = m_aDatabases.begin();
        while( it != m_aDatabases.end() )
        {
            if( it->second )
                it->second->close( 0 );
            delete it->second;
            ++it;
        }
    }

    {
        //  ModInfoTable

        ModInfoTable::iterator it = m_aModInfo.begin();
        while( it != m_aModInfo.end() )
        {
            delete it->second;
            ++it;
        }
    }

    {
        // KeywordInfoTable

        KeywordInfoTable::iterator it = m_aKeywordInfo.begin();
        while( it != m_aKeywordInfo.end() )
        {
            delete it->second;
            ++it;
        }
    }

}



void Databases::replaceName( rtl::OUString& oustring ) const
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




rtl::OUString Databases::getInstallPathAsSystemPath()
{
    osl::MutexGuard aGuard( m_aMutex );

    if( ! m_aInstallDirectoryAsSystemPath.getLength() )
    {
        bool bla =
            osl::FileBase::E_None ==
            osl::FileBase::getSystemPathFromFileURL( m_aInstallDirectory,m_aInstallDirectoryAsSystemPath );
        VOS_ENSURE( bla,
                    "HelpProvider, no installpath" );
    }

    return m_aInstallDirectoryAsSystemPath;
}




rtl::OUString Databases::getInstallPathAsURL()
{
    osl::MutexGuard aGuard( m_aMutex );

      return m_aInstallDirectory;
}


rtl::OUString Databases::getInstallPathAsURLWithOutEncoding()
{
    osl::MutexGuard aGuard( m_aMutex );

    return m_aInstallDirectoryWithoutEncoding;
}


rtl::OUString Databases::getURLMode()
{
    return rtl::OUString::createFromAscii( "with-jars" );
}


std::vector< rtl::OUString > Databases::getModuleList( const rtl::OUString& Language )
{
    rtl::OUString  fileName,dirName = getInstallPathAsURL() + lang( Language );
    osl::Directory dirFile( dirName );

    osl::DirectoryItem aDirItem;
    osl::FileStatus    aStatus( FileStatusMask_FileName );

    std::vector< rtl::OUString > ret;
    sal_Int32 idx;

    if( osl::FileBase::E_None != dirFile.open() )
        return ret;

    while( dirFile.getNextItem( aDirItem ) == osl::FileBase::E_None &&
           aDirItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
    {
        if( ! aStatus.isValid( FileStatusMask_FileName ) )
            continue;

        fileName = aStatus.getFileName();

        // Check, whether fileName is of the form *.db with the exception of picture.db
        idx = fileName.lastIndexOf(  sal_Unicode( '.' ) );

        if( idx == -1 )
            continue;

        const sal_Unicode* str = fileName.getStr();

        if( fileName.getLength() == idx + 3                   &&
            ( str[idx + 1] == 'd' || str[idx + 1] == 'D' )    &&
            ( str[idx + 2] == 'b' || str[idx + 2] == 'B' )    &&
            ( fileName = fileName.copy(0,idx).toAsciiLowerCase() ).compareToAscii( "picture" ) != 0 )
            ret.push_back( fileName );
    }

    return ret;
}



StaticModuleInformation* Databases::getStaticInformationForModule( const rtl::OUString& Module,
                                                                   const rtl::OUString& Language )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + Module;

    std::pair< ModInfoTable::iterator,bool > aPair =
        m_aModInfo.insert( ModInfoTable::value_type( key,0 ) );

    ModInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        osl::File cfgFile( getInstallPathAsURL() +
                           key +
                           rtl::OUString::createFromAscii( ".cfg" ) );

        if( osl::FileBase::E_None != cfgFile.open( OpenFlag_Read ) )
            it->second = 0;
        else
        {
            sal_uInt32 pos = 0;
            sal_uInt64 nRead;
            sal_Char buffer[2048];
            sal_Unicode lineBuffer[1028];
            rtl::OUString fileContent;

            while( osl::FileBase::E_None == cfgFile.read( &buffer,2048,nRead ) && nRead )
                fileContent += rtl::OUString( buffer,sal_Int32( nRead ),RTL_TEXTENCODING_UTF8 );

            cfgFile.close();

            const sal_Unicode* str = fileContent.getStr();
            rtl::OUString current,lang,program,startid,title,heading,fulltext;
            rtl::OUString order = rtl::OUString::createFromAscii( "1" );

            for( sal_Int32 i = 0;i < fileContent.getLength();i++ )
            {
                sal_Unicode ch = str[ i ];
                if( ch == sal_Unicode( '\n' ) || ch == sal_Unicode( '\r' ) )
                {
                    if( pos )
                    {
                        current = rtl::OUString( lineBuffer,pos );

                        if( current.compareToAscii( "Title",5 ) == 0 )
                        {
                            title = current.copy( current.indexOf(sal_Unicode( '=' ) ) + 1 );
                        }
                        else if( current.compareToAscii( "Start",5 ) == 0 )
                        {
                            startid = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.compareToAscii( "Language",8 ) == 0 )
                        {
                            lang = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.compareToAscii( "Program",7 ) == 0 )
                        {
                            program = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.compareToAscii( "Heading",7 ) == 0 )
                        {
                            heading = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.compareToAscii( "FullText",8 ) == 0 )
                        {
                            fulltext = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.compareToAscii( "Order",5 ) == 0 )
                        {
                            order = current.copy( current.indexOf('=') + 1 );
                        }
                    }
                    pos = 0;
                }
                else
                    lineBuffer[ pos++ ] = ch;
            }
            replaceName( title );
            it->second = new StaticModuleInformation( title,
                                                      startid,
                                                      program,
                                                      heading,
                                                      fulltext,
                                                      order );
        }
    }

    return it->second;
}




rtl::OUString Databases::lang( const rtl::OUString& Language )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString ret;
    LangSetTable::iterator it = m_aLangSet.find( Language );

    if( it == m_aLangSet.end() )
    {
        sal_Int32 idx;
        osl::DirectoryItem aDirItem;

        if( osl::FileBase::E_None == osl::DirectoryItem::get( getInstallPathAsURL() + Language,aDirItem ) )
        {
            ret = Language;
            m_aLangSet[ Language ] = ret;
        }
        else if( ( ( idx = Language.indexOf( '-' ) ) != -1 ||
                   ( idx = Language.indexOf( '_' ) ) != -1 ) &&
                 osl::FileBase::E_None == osl::DirectoryItem::get( getInstallPathAsURL() + Language.copy( 0,idx ),
                                                                   aDirItem ) )
        {
            ret = Language.copy( 0,idx );
            m_aLangSet[ Language ] = ret;
        }
    }
    else
        ret = it->second;

    return ret;
}


rtl::OUString Databases::country( const rtl::OUString& Language )
{
    sal_Int32 idx;
    if( ( idx = Language.indexOf( '-' ) ) != -1 ||
        ( idx = Language.indexOf( '_' ) ) != -1 )
        return Language.copy( 1+idx );

    return rtl::OUString();
}



rtl::OUString Databases::variant( const rtl::OUString& System )
{
    if( System.compareToAscii( "WIN" ) == 0 ||
        System.compareToAscii( "MAC" ) )
        return System;
    else
        return rtl::OUString::createFromAscii( "POSIX" );
}



Db* Databases::getBerkeley( const rtl::OUString& Database,
                            const rtl::OUString& Language,
                            bool helpText )
{
    if( ! Database.getLength() || ! Language.getLength() )
        return 0;

    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString key =
        lang(Language) +
        rtl::OUString::createFromAscii( "/" ) +
        Database +
        ( helpText ? rtl::OUString::createFromAscii( ".ht" ) : rtl::OUString::createFromAscii( ".db" ) );

    std::pair< DatabasesTable::iterator,bool > aPair =
        m_aDatabases.insert( DatabasesTable::value_type( key,0 ) );

    DatabasesTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {

        Db* table = new Db( 0,DB_CXX_NO_EXCEPTIONS );

        rtl::OUString fileNameOU =
            getInstallPathAsSystemPath() +
            key;

        rtl::OString fileName( fileNameOU.getStr(),fileNameOU.getLength(),osl_getThreadTextEncoding() );

        if( table->open( fileName.getStr(),0,DB_BTREE,DB_RDONLY,0644 ) )
        {
            table->close( 0 );
            delete table;
            table = 0;
        }

        it->second = table;
    }

    return it->second;
}



Reference< XCollator >
Databases::getCollator( const rtl::OUString& Language,
                        const rtl::OUString& System )
{
    rtl::OUString key = Language;

    osl::MutexGuard aGuard( m_aMutex );

    CollatorTable::iterator it =
        m_aCollatorTable.insert( CollatorTable::value_type( key,0 ) ).first;

    if( ! it->second.is() )
    {
        it->second =
            Reference< XCollator > (
                m_xSMgr->createInstance( rtl::OUString::createFromAscii( "com.sun.star.i18n.Collator" ) ),
                UNO_QUERY );
        rtl::OUString langStr = lang(Language);
        rtl::OUString countryStr = country(Language);
        if( !countryStr.getLength() )
        {
            if( langStr.compareToAscii("de") == 0 )
                countryStr = rtl::OUString::createFromAscii("DE");
            else if( langStr.compareToAscii("en") == 0 )
                countryStr = rtl::OUString::createFromAscii("US");
            else if( langStr.compareToAscii("es") == 0 )
                countryStr = rtl::OUString::createFromAscii("ES");
            else if( langStr.compareToAscii("it") == 0 )
                countryStr = rtl::OUString::createFromAscii("IT");
            else if( langStr.compareToAscii("fr") == 0 )
                countryStr = rtl::OUString::createFromAscii("FR");
            else if( langStr.compareToAscii("sv") == 0 )
                countryStr = rtl::OUString::createFromAscii("SE");
            else if( langStr.compareToAscii("ja") == 0 )
                countryStr = rtl::OUString::createFromAscii("JP");
            else if( langStr.compareToAscii("ko") == 0 )
                countryStr = rtl::OUString::createFromAscii("KR");
        }
        it->second->loadDefaultCollator(  Locale( langStr,
                                                  countryStr,
                                                  rtl::OUString() ),
                                          0 );
    }

    return it->second;
}



namespace chelp {

    struct KeywordElementComparator
    {
        KeywordElementComparator( const Reference< XCollator >& xCollator )
            : m_xCollator( xCollator )
        { }

        bool operator()( const KeywordInfo::KeywordElement& la,
                         const KeywordInfo::KeywordElement& ra ) const
        {
            const rtl::OUString& l = la.key;
            const rtl::OUString& r = ra.key;

            bool ret;

            if( m_xCollator.is() )
            {
                sal_Int32 l1 = l.indexOf( sal_Unicode( ';' ) );
                sal_Int32 l3 = ( l1 == -1 ? l.getLength() : l1 );

                sal_Int32 r1 = r.indexOf( sal_Unicode( ';' ) );
                sal_Int32 r3 = ( r1 == -1 ? r.getLength() : r1 );

                sal_Int32 c1 = m_xCollator->compareSubstring( l,0,l3,r,0,r3 );

                if( c1 == +1 )
                    ret = false;
                else if( c1 == 0 )
                {
                    sal_Int32 l2 = l.getLength() - l1 - 1;
                    sal_Int32 r2 = r.getLength() - r1 - 1;
                    ret = ( m_xCollator->compareSubstring( l,1+l1,l2,r,1+r1,r2 ) < 0 );
                }
                else
                    ret = true;
            }
            else
                ret = bool( l < r );

            return ret;
        }

        Reference< XCollator > m_xCollator;
    }; // end struct KeywordElementComparator

}



KeywordInfo::KeywordElement::KeywordElement( Databases *pDatabases,
                                             Db* pDb,
                                             rtl::OUString& ky,
                                             rtl::OUString& data )
    : key( ky )
{
    pDatabases->replaceName( key );
    init( pDatabases,pDb,data );
}



void KeywordInfo::KeywordElement::init( Databases *pDatabases,Db* pDb,const rtl::OUString& ids )
{
    const sal_Unicode* idstr = ids.getStr();
    std::vector< rtl::OUString > id,anchor;
    int idx = -1,k;
    while( ( idx = ids.indexOf( ';',k = ++idx ) ) != -1 )
    {
        int h = ids.indexOf( sal_Unicode( '#' ),k );
        if( h < idx )
        {
            // found an anchor
            id.push_back( rtl::OUString( &idstr[k],h-k ) );
            anchor.push_back( rtl::OUString( &idstr[h+1],idx-h-1 ) );
        }
        else
        {
            id.push_back( rtl::OUString( &idstr[k],idx-k ) );
            anchor.push_back( rtl::OUString() );
        }
    }

    listId.realloc( id.size() );
    listAnchor.realloc( id.size() );
    listTitle.realloc( id.size() );
    for( sal_uInt32 i = 0; i < id.size(); ++i )
    {
        listId[i] = id[i];
        listAnchor[i] = anchor[i];

        rtl::OString idi( id[i].getStr(),id[i].getLength(),RTL_TEXTENCODING_UTF8 );
        Dbt key( static_cast< void* >( const_cast< sal_Char* >( idi.getStr() ) ),
                 idi.getLength() );
        Dbt data;
        if( pDb )
            pDb->get( 0,&key,&data,0 );

        DbtToStringConverter converter( static_cast< sal_Char* >( data.get_data() ),
                                        data.get_size() );

        rtl::OUString title = converter.getTitle();
        pDatabases->replaceName( title );
        listTitle[i] = title;
    }
}



KeywordInfo::KeywordInfo( const std::vector< KeywordElement >& aVec )
    : listKey( aVec.size() ),
      listId( aVec.size() ),
      listAnchor( aVec.size() ),
      listTitle( aVec.size() )
{
    for( unsigned int i = 0; i < aVec.size(); ++i )
    {
        listKey[i] = aVec[i].key;
        listId[i] = aVec[i].listId;
        listAnchor[i] = aVec[i].listAnchor;
        listTitle[i] = aVec[i].listTitle;
    }
}



KeywordInfo* Databases::getKeyword( const rtl::OUString& Database,
                                    const rtl::OUString& Language )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + Database;

    std::pair< KeywordInfoTable::iterator,bool > aPair =
        m_aKeywordInfo.insert( KeywordInfoTable::value_type( key,0 ) );

    KeywordInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        rtl::OUString fileNameOU =
            getInstallPathAsSystemPath() +
            key +
            rtl::OUString::createFromAscii( ".key" );

        rtl::OString fileName( fileNameOU.getStr(),
                               fileNameOU.getLength(),
                               osl_getThreadTextEncoding() );

        Db table(0,DB_CXX_NO_EXCEPTIONS);
        if( 0 == table.open( fileName.getStr(),0,DB_BTREE,DB_RDONLY,0644 ) )
        {
            std::vector<KeywordInfo::KeywordElement> aVector;
            Db* idmap = getBerkeley( Database,Language );

            bool first = true;

            Dbc* cursor = 0;
            table.cursor( 0,&cursor,0 );
            Dbt key,data;
            key.set_flags( DB_DBT_MALLOC ); // Initially the cursor must allocate the necessary memory
            data.set_flags( DB_DBT_MALLOC );
            while( cursor && DB_NOTFOUND != cursor->get( &key,&data,DB_NEXT ) )
            {
                rtl::OUString keyword( static_cast<sal_Char*>(key.get_data()),
                                       key.get_size(),
                                       RTL_TEXTENCODING_UTF8 );
                rtl::OUString doclist( static_cast<sal_Char*>(data.get_data()),
                                       data.get_size(),
                                       RTL_TEXTENCODING_UTF8 );

                aVector.push_back( KeywordInfo::KeywordElement( this,
                                                                idmap,
                                                                keyword,
                                                                doclist ) );
                if( first )
                {
                    key.set_flags( DB_DBT_REALLOC );
                    data.set_flags( DB_DBT_REALLOC );
                    first = false;
                }
            }

            if( cursor ) cursor->close();

            // sorting
            Reference< XCollator > xCollator = getCollator( Language,rtl::OUString());
            KeywordElementComparator aComparator( xCollator );
            std::sort(aVector.begin(),aVector.end(),aComparator);

            KeywordInfo* info = it->second = new KeywordInfo( aVector );
        }
        table.close( 0 );
    }

    return it->second;
}




Reference< XHierarchicalNameAccess > Databases::jarFile( const rtl::OUString& jar,
                                                         const rtl::OUString& Language )
{
    if( ! jar.getLength() ||
        ! Language.getLength() )
    {
        return Reference< XHierarchicalNameAccess >( 0 );
    }
    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + jar;

    osl::MutexGuard aGuard( m_aMutex );

    ZipFileTable::iterator it =
        m_aZipFileTable.insert( ZipFileTable::value_type( key,Reference< XHierarchicalNameAccess >(0) ) ).first;

    if( ! it->second.is() )
    {
        rtl::OUString zipFile;
        try
        {
            zipFile = getInstallPathAsURL() + key;
            Sequence< Any > aArguments( 1 );

            XInputStream_impl* p = new XInputStream_impl( zipFile );
            if( p->CtorSuccess() )
            {
                Reference< XInputStream > xInputStream( p );
                aArguments[ 0 ] <<= xInputStream;
            }
            else
            {
                delete p;
                aArguments[ 0 ] <<= zipFile;
            }

            Reference< XInterface > xIfc
                = m_xSMgr->createInstanceWithArguments(
                    rtl::OUString::createFromAscii(
                        "com.sun.star.packages.comp.ZipPackage" ),
                    aArguments );

            if ( xIfc.is() )
            {
                it->second = Reference< XHierarchicalNameAccess >( xIfc, UNO_QUERY );

                VOS_ENSURE( it->second.is(),
                            "ContentProvider::createPackage - "
                            "Got no hierarchical name access!" );

            }
        }
        catch ( RuntimeException & )
        {
        }
        catch ( Exception & )
        {
        }
    }

    return it->second;
}


void Databases::popupDocument( URLParameter* urlPar,char **buffer,int *byteCount )
{
    const char* pop1 =
        " <html>                                                                "
        " <head>                                                                "
        " <help:css-file-link xmlns:help=\"http://openoffice.org/2000/help\"/>  "
        " </head>                                                               "
        " <body>                                                                "
        " <help:popup-cut Id=\"";
    const sal_Int32 l1 = strlen( pop1 );

    const char* pop3 = "\" Eid=\"";
    const sal_Int32 l3 = strlen( pop3 );

    const char* pop5 =
        "\" xmlns:help=\"http://openoffice.org/2000/help\"></help:popup-cut>  "
        " </body>                                                             "
        " </html>";
    const sal_Int32 l5 = strlen( pop5 );
    sal_Int32 l2,l4;

    rtl::OUString val = urlPar->get_id();
    rtl::OString pop2O( val.getStr(),l2 = val.getLength(),RTL_TEXTENCODING_UTF8 );
    const char* pop2 = pop2O.getStr();

    val = urlPar->get_eid();
    rtl::OString pop4O( val.getStr(),l4 = val.getLength(),RTL_TEXTENCODING_UTF8 );
    const char* pop4 = pop4O.getStr();

    (*byteCount) = l1 + l2 + l3 + l4 + l5;

    *buffer = new char[ 1+*byteCount ];

    rtl_copyMemory( *buffer,pop1,l1 );
    rtl_copyMemory( *buffer+l1,pop2,l2 );
    rtl_copyMemory( *buffer+(l1+l2),pop3,l3 );
    rtl_copyMemory( *buffer+(l1+l2+l3),pop4,l4 );
    rtl_copyMemory( *buffer+(l1+l2+l3+l4),pop5,l5 );
    (*buffer)[*byteCount] = 0;
}


void Databases::errorDocument( const rtl::OUString& Language,
                               char** buffer,
                               int* byteCount )
{
    if( ! m_pErrorDoc )
    {
        rtl::OUString fileURL =
            getInstallPathAsURL()
            + lang( Language )
            + rtl::OUString::createFromAscii( "/err.html" );

        osl::DirectoryItem aDirItem;
        osl::File aFile( fileURL );
        osl::FileStatus aStatus( FileStatusMask_FileSize );

        if( osl::FileBase::E_None == osl::DirectoryItem::get( fileURL,aDirItem ) &&
            osl::FileBase::E_None == aFile.open( OpenFlag_Read )       &&
            osl::FileBase::E_None == aDirItem.getFileStatus( aStatus ) )
        {
            m_nErrorDocLength = int( aStatus.getFileSize() );
            m_pErrorDoc = new char[ 1 + m_nErrorDocLength ];
            m_pErrorDoc[ m_nErrorDocLength ] = 0;
            sal_uInt64 a = m_nErrorDocLength,b = m_nErrorDocLength;
            aFile.read( m_pErrorDoc,a,b );
            aFile.close();
        }
        else
        {
            // the error file does not exist
          const char* errorText =
            "<html><body>"
            "The requested document does not exist in the database !!"
            "</body></html>";

          m_nErrorDocLength = strlen( errorText );
          m_pErrorDoc = new char[ 1 + m_nErrorDocLength ];
          m_pErrorDoc[ m_nErrorDocLength ] = 0;
          rtl_copyMemory( m_pErrorDoc,errorText,m_nErrorDocLength );
        }

    }

    *byteCount = m_nErrorDocLength;
    *buffer = new char[ 1 + *byteCount ];
    (*buffer)[*byteCount] = 0;
    rtl_copyMemory( *buffer,m_pErrorDoc,m_nErrorDocLength );
}




void Databases::changeCSS(const rtl::OUString& newStyleSheet)
{
    m_aCSS = newStyleSheet.toAsciiLowerCase();
    delete[] m_pCustomCSSDoc, m_pCustomCSSDoc = 0,m_nCustomCSSDocLength = 0;
}



void Databases::cascadingStylesheet( const rtl::OUString& Language,
                                     char** buffer,
                                     int* byteCount )
{
    if( ! m_pCustomCSSDoc )
    {
        int retry = 2;
        bool error = true;
        rtl::OUString fileURL;

        while( error && retry )
        {
            if( retry == 2 )
                fileURL =
                    getInstallPathAsURL()  +
                    lang( Language )       +
                    rtl::OUString::createFromAscii( "/" ) +
                    m_aCSS +
                    rtl::OUString::createFromAscii( ".css" );
            else if( retry == 1 )
                fileURL =
                    getInstallPathAsURL()  +
                    m_aCSS +
                    rtl::OUString::createFromAscii( ".css" );

            osl::DirectoryItem aDirItem;
            osl::File aFile( fileURL );
            osl::FileStatus aStatus( FileStatusMask_FileSize );

            if( osl::FileBase::E_None == osl::DirectoryItem::get( fileURL,aDirItem ) &&
                osl::FileBase::E_None == aFile.open( OpenFlag_Read )                 &&
                osl::FileBase::E_None == aDirItem.getFileStatus( aStatus ) )
            {
                m_nCustomCSSDocLength = int( aStatus.getFileSize() );
                m_pCustomCSSDoc = new char[ 1 + m_nCustomCSSDocLength ];
                m_pCustomCSSDoc[ m_nCustomCSSDocLength ] = 0;
                sal_uInt64 a = m_nCustomCSSDocLength,b = m_nCustomCSSDocLength;
                aFile.read( m_pCustomCSSDoc,a,b );
                aFile.close();
                error = false;
            }

            --retry;
        }

        if( error )
        {
            m_nCustomCSSDocLength = 0;
            m_pCustomCSSDoc = new char[ 0 ];
        }
    }

    *byteCount = m_nCustomCSSDocLength;
    *buffer = new char[ 1 + *byteCount ];
    (*buffer)[*byteCount] = 0;
    rtl_copyMemory( *buffer,m_pCustomCSSDoc,m_nCustomCSSDocLength );

}



void Databases::setActiveText( const rtl::OUString& Module,
                               const rtl::OUString& Language,
                               const rtl::OUString& Id,
                               char** buffer,
                               int* byteCount )
{
    Db* db = getBerkeley( Module,Language,true );

    if( db )
    {
        rtl::OString id( Id.getStr(),Id.getLength(),RTL_TEXTENCODING_UTF8 );
        Dbt key( static_cast< void* >( const_cast< sal_Char* >( id.getStr() ) ),id.getLength() );
        Dbt data;
        db->get( 0,&key,&data,0 );
        int len = data.get_size();
        const sal_Char* ptr = static_cast<sal_Char*>( data.get_data() );

        // ensure existence of tmp after for
        rtl::OString tmp;
        for( int i = 0; i < len; ++i )
            if( ptr[i] == '%' )
            {
                // need to replace
                rtl::OUString temp = rtl::OUString( ptr,len,RTL_TEXTENCODING_UTF8 );
                replaceName( temp );
                tmp = rtl::OString( temp.getStr(),
                                    temp.getLength(),
                                    RTL_TEXTENCODING_UTF8 );
                len = tmp.getLength();
                ptr = tmp.getStr();
                break;
            }

        *byteCount = len;
        *buffer = new char[ 1 + len ];
        (*buffer)[len] = 0;
        rtl_copyMemory( *buffer,ptr,len );
    }
    else
    {
        *byteCount = 0;
        *buffer = new char[0];
    }
}


void Databases::setInstallPath( const rtl::OUString& aInstDir )
{
    osl::MutexGuard aGuard( m_aMutex );

    if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath( aInstDir,m_aInstallDirectory ) )
        ;

    if( m_aInstallDirectory.lastIndexOf( sal_Unicode( '/' ) ) != m_aInstallDirectory.getLength() - 1 )
        m_aInstallDirectory += rtl::OUString::createFromAscii( "/" );

    m_aInstallDirectoryWithoutEncoding = rtl::Uri::decode( m_aInstallDirectory,
                                                           rtl_UriDecodeWithCharset,
                                                           RTL_TEXTENCODING_UTF8 );
}

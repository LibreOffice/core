/*************************************************************************
 *
 *  $RCSfile: databases.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: abi $ $Date: 2001-08-21 13:26:25 $
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


using namespace chelp;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;


Databases::Databases( const rtl::OUString& instPath,
                      com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xSMgr )
    : m_xSMgr( xSMgr ),
      m_nErrorDocLength( 0 ),
      m_pErrorDoc( 0 ),
      m_nCustomCSSDocLength( 0 ),
      m_pCustomCSSDoc( 0 )
{
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

    ModInfoTable::iterator it =
        m_aModInfo.insert( ModInfoTable::value_type( key,0 ) ).first;

    if( ! it->second )
    {
        osl::File cfgFile( getInstallPathAsURL() +
                           key +
                           rtl::OUString::createFromAscii( ".cfg" ) );

        if( osl::FileBase::E_None != cfgFile.open( OpenFlag_Read ) )
            return 0;

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
                }
                pos = 0;
            }
            else
                lineBuffer[ pos++ ] = ch;
        }
        it->second = new StaticModuleInformation( title,startid,program,heading,fulltext );
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

    DatabasesTable::iterator it =
        m_aDatabases.insert( DatabasesTable::value_type( key,0 ) ).first;

    if( ! it->second )
    {
        Db* table = it->second = new Db( 0,0 );

        rtl::OUString fileNameOU =
            getInstallPathAsSystemPath() +
            key;

        rtl::OString fileName( fileNameOU.getStr(),fileNameOU.getLength(),RTL_TEXTENCODING_UTF8 );

        table->open( fileName.getStr(),0,DB_BTREE,DB_RDONLY,0644 );
        m_aDatabases[ key ] = table;
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

        it->second->loadDefaultCollator( Locale( lang( Language ),
                                                 country( Language ),
                                                 rtl::OUString() ),
                                         0 );
    }

    return it->second;
}




KeywordInfo::KeywordInfo()
    : pos( 0 ),
      listKey( 100 )
{
}



Sequence< rtl::OUString >& KeywordInfo::insertId( sal_Int32 index,rtl::OUString ids )
{
    std::vector< rtl::OUString > test;
    while( ids.getLength() )
    {
        sal_Int32 idx = ids.indexOf( ';' );
        test.push_back( ids.copy(0,idx) );
        ids = ids.copy( 1+idx );
    }

    listId[index].realloc( test.size() );
    for( sal_uInt32 i = 0; i < test.size(); ++i )
        listId[index][i] = test[i];

    listAnchor[index].realloc( test.size() );

    for( sal_Int32 k = 0; k < listId[index].getLength(); ++k )
    {
        if( listId[index][k].getLength() )
        {
            sal_Int32 idx = listId[index][k].indexOf( sal_Unicode( '#' ) );
            if( idx != -1 )
            {
                listAnchor[index][k] = listId[index][k].copy(1+idx).trim();
                listId[index][k] = listId[index][k].copy(0,idx).trim();
            }
        }
    }

    listTitle[index].realloc( test.size() );
    return listId[index];
}



KeywordInfo::Compare::Compare( const Reference< XCollator >& xCollator )
    : m_xCollator( xCollator )
{
}


int KeywordInfo::Compare::operator()( const rtl::OUString& l,const rtl::OUString& r )
{
    if( m_xCollator.is() )
    {
        sal_Int32 l1 = l.indexOf( sal_Unicode( ';' ) );
        sal_Int32 l2 = l.getLength() - l1 - 1;

        sal_Int32 r1 = r.indexOf( sal_Unicode( ';' ) );
        sal_Int32 r2 = r.getLength() - r1 - 1;

        sal_Int32 c1 = m_xCollator->compareSubstring( l,0,l1,r,0,r1 );

        if( c1 == +1 )
            return 0;
        else
        {
            if( c1 == 0 )
                return ( m_xCollator->compareSubstring( l,1+l1,l2,r,1+r1,r2 ) <= 0 ) ? 1 : 0;
            else
                return 1;
        }
//          return ( m_xCollator->compareString( l,r ) <= 0 ) ? 1 : 0;
    }
    else
        return ( l <= r ) ? 1 : 0;
}



void KeywordInfo::sort( std::vector< rtl::OUString >& listKey_,Compare& comp )
{
    std::sort( listKey_.begin(),listKey_.end(),comp );
    listKey.realloc( listKey_.size() );
    listId.realloc( listKey_.size() );
    listAnchor.realloc( listKey_.size() );
    listTitle.realloc( listKey_.size() );

    for( sal_uInt32 i = 0; i < listKey_.size(); ++i )
        listKey[i] = listKey_[i];
}



KeywordInfo* Databases::getKeyword( const rtl::OUString& Database,
                                    const rtl::OUString& Language )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + Database;

    KeywordInfoTable::iterator it =
        m_aKeywordInfo.insert( KeywordInfoTable::value_type( key,0 ) ).first;

    if( ! it->second )
    {
        std::vector< rtl::OUString > listKey_;
        std::hash_map< rtl::OUString,rtl::OUString,ha,eq > internalHash;

        rtl::OUString fileNameOU =
            getInstallPathAsSystemPath() +
            key +
            rtl::OUString::createFromAscii( ".key" );

        rtl::OString fileName( fileNameOU.getStr(),fileNameOU.getLength(),RTL_TEXTENCODING_UTF8 );

        Db table(0,0);
        table.open( fileName.getStr(),0,DB_BTREE,DB_RDONLY,0644 );

        Dbc* cursor = 0;
        table.cursor( 0,&cursor,0 );
        Dbt key,data;

        bool first = true;
        key.set_flags( DB_DBT_MALLOC );      // Initially the cursor must allocate the necessary memory
        data.set_flags( DB_DBT_MALLOC );
        KeywordInfo* info = it->second = new KeywordInfo();

        rtl::OUString keyStri;

        while( cursor && DB_NOTFOUND != cursor->get( &key,&data,DB_NEXT ) )
        {
            keyStri = rtl::OUString( static_cast<sal_Char*>(key.get_data()),
                                     key.get_size(),
                                     RTL_TEXTENCODING_UTF8 );
            info->insert( listKey_,keyStri );
            internalHash[ keyStri ] = rtl::OUString( static_cast<sal_Char*>(data.get_data()),
                                                     data.get_size(),
                                                     RTL_TEXTENCODING_UTF8 );

            if( first )
            {
                key.set_flags( DB_DBT_REALLOC );
                data.set_flags( DB_DBT_REALLOC );
                first = false;
            }
        }

        info->sort( listKey_,KeywordInfo::Compare( getCollator( Language,
                                                                rtl::OUString() ) ) );
        cursor->close();
        table.close( 0 );

        Sequence< rtl::OUString >& keywords = info->getKeywordList();
        Db *table2 = getBerkeley( Database,Language );
        for( sal_Int32 i = 0; i < keywords.getLength(); ++i )
        {
            Sequence< rtl::OUString >& id = info->insertId( i,internalHash[ keywords[i] ] );
            Sequence< rtl::OUString >& title = info->getTitleForIndex( i );

            for( sal_Int32 j = 0; j < id.getLength(); ++j )
            {
                rtl::OString idj( id[j].getStr(),id[j].getLength(),RTL_TEXTENCODING_UTF8 );
                Dbt key1( static_cast< void* >( const_cast< sal_Char* >( idj.getStr() ) ),
                          idj.getLength() );
                Dbt data1;
                if( table2 )
                    table2->get( 0,&key1,&data1,0 );

                DbtToStringConverter converter( static_cast< sal_Char* >( data1.get_data() ),
                                                data1.get_size() );

                title[j] = converter.getTitle();
            }
        }
    }

    return it->second;
}



//  Reference< XInputStream > Databases::getFromURL( const rtl::OUString& url )
//  {
//      osl::MutexGuard aGuard( m_aMutex );

//      Reference< XInputStream > xStream = m_aInputStreamTable[ url ];
//      if( xStream.is() )
//          m_aInputStreamTable[ url ] = Reference< XInputStream >( 0 );

//      return xStream;
//  }


//  void Databases::setFromURL( const rtl::OUString& url,const Reference< XInputStream >& xStream  )
//  {
//      osl::MutexGuard aGuard( m_aMutex );

//      m_aInputStreamTable[ url ] = xStream;
//  }



Reference< XHierarchicalNameAccess > Databases::jarFile( const rtl::OUString& jar,
                                                         const rtl::OUString& Language )
{
    if( ! jar.getLength() ||
        ! Language.getLength() )
        return Reference< XHierarchicalNameAccess >( 0 );

    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + jar;

    osl::MutexGuard aGuard( m_aMutex );

    ZipFileTable::iterator it =
        m_aZipFileTable.insert( ZipFileTable::value_type( key,Reference< XHierarchicalNameAccess >(0) ) ).first;

    if( ! it->second.is() )
    {
        try
        {
            rtl::OUString zipFile = getInstallPathAsURL() + key;
            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ] <<= zipFile;

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
                    rtl::OUString::createFromAscii( "/custom.css" );
            else if( retry == 1 )
                fileURL =
                    getInstallPathAsURL()  +
                    rtl::OUString::createFromAscii( "custom.css" );

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
        *byteCount = data.get_size();
        *buffer = new char[ 1 + *byteCount ];
        (*buffer)[*byteCount] = 0;
        rtl_copyMemory( *buffer,data.get_data(),*byteCount );
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
}

/*************************************************************************
 *
 *  $RCSfile: databases.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-29 15:14:49 $
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

#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _URLPARAMETER_HXX_
#include <provider/urlparameter.hxx>
#endif
#include <berkeleydb/db_cxx.h>

using namespace chelp;
using namespace com::sun::star::uno;

// The same for the jar files
//  private static final Hashtable _jarHash = new Hashtable();


//  public static final Hashtable _modInfo = new Hashtable();


osl::Mutex                  Databases::m_aMutex;
rtl::OUString               Databases::m_aInstallDirectory;              // Installation directory
rtl::OUString               Databases::m_aInstallDirectoryAsSystemPath;  // Installation directory
rtl::OUString               Databases::m_aInstallDirectoryAsURL;         // Installation directory
Databases::DatabasesTable   Databases::m_aDatabases;                     // Language and module dependent databases
Databases::LangSetTable     Databases::m_aLangSet;                       // Mapping to of lang-country to lang
Databases::ModInfoTable     Databases::m_aModInfo;                       // Module information
Databases::KeywordInfoTable Databases::m_aKeywordInfo;


void Databases::setInstallPath( const rtl::OUString& aInstDir )
{
    osl::MutexGuard aGuard( m_aMutex );

//      if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath( aInstDir,m_aInstallDirectory ) )
//          ;

    if( osl::FileBase::E_None != osl::FileBase::normalizePath( aInstDir,m_aInstallDirectory ) )
        ;

    if( m_aInstallDirectory.lastIndexOf( sal_Unicode( "/" ) ) != m_aInstallDirectory.getLength() - 1 )
        m_aInstallDirectory += rtl::OUString::createFromAscii( "/" );
}


rtl::OUString Databases::getInstallPath()
{
    osl::MutexGuard aGuard( m_aMutex );

    return m_aInstallDirectory;
}


rtl::OUString Databases::getInstallPathAsSystemPath()
{
    osl::MutexGuard aGuard( m_aMutex );

    if( ! m_aInstallDirectoryAsSystemPath.getLength() )
    {
//          bool bla =
//              osl::FileBase::E_None ==
//              osl::FileBase::getSystemPathFromFileURL( m_aInstallDirectory,m_aInstallDirectoryAsSystemPath );
        bool bla =
            osl::FileBase::E_None ==
            osl::FileBase::getSystemPathFromNormalizedPath( m_aInstallDirectory,m_aInstallDirectoryAsSystemPath );
        VOS_ENSURE( bla,
                    "HelpProvider, no installpath" );
    }

    return m_aInstallDirectoryAsSystemPath;
}


rtl::OUString Databases::getInstallPathAsURL()
{
    osl::MutexGuard aGuard( m_aMutex );

    if( ! m_aInstallDirectoryAsURL.getLength() )
    {
        bool bla =
            osl::FileBase::E_None ==
            osl::FileBase::getFileURLFromNormalizedPath( m_aInstallDirectory,m_aInstallDirectoryAsURL );
        VOS_ENSURE( bla,
                    "HelpProvider, no installpath" );
    }

    return m_aInstallDirectoryAsURL;
//      return m_aInstallDirectory;
}


rtl::OUString Databases::getURLMode()
{
    return rtl::OUString::createFromAscii( "with-jars" );
}


std::vector< rtl::OUString > Databases::getModuleList( const rtl::OUString& Language )
{
    rtl::OUString  fileName,dirName = getInstallPath() + lang( Language );
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
        osl::File cfgFile( getInstallPath() +
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
        if( ( idx = Language.indexOf( '-' ) ) != -1 ||
            ( idx = Language.indexOf( '_' ) ) != -1 )
            ret = Language.copy( 0,idx );
        else
            ret = Language;

        osl::DirectoryItem aDirItem;
        if( osl::FileBase::E_None == osl::DirectoryItem::get( getInstallPath() + ret,aDirItem ) )
            m_aLangSet[ Language ] = ret;
    }
    else
        ret = it->second;

    return ret;
}


Db* Databases::getBerkeley( const rtl::OUString& Database,
                            const rtl::OUString& Language )
{
    if( ! Database.getLength() || ! Language.getLength() )
        return 0;

    osl::MutexGuard aGuard( m_aMutex );

    rtl::OUString key = lang(Language) + rtl::OUString::createFromAscii( "/" ) + Database;

    DatabasesTable::iterator it =
        m_aDatabases.insert( DatabasesTable::value_type( key,0 ) ).first;

    if( ! it->second )
    {
        Db* table = it->second = new Db( 0,0 );

        rtl::OUString fileNameOU =
            getInstallPathAsSystemPath() +
            key +
            rtl::OUString::createFromAscii( ".db" );

        rtl::OString fileName( fileNameOU.getStr(),fileNameOU.getLength(),RTL_TEXTENCODING_UTF8 );

        table->open( fileName.getStr(),0,DB_BTREE,DB_RDONLY,0644 );
        m_aDatabases[ key ] = table;
    }

    return it->second;
}




//  public static synchronized JarFile getJarFileForLanguage( String Database,String Language )
//  {
//      if( Language == null || Database == null )
//          return null;

//      String key = lang(Language) + File.separator + Database;
//      JarFile jarFile = ( JarFile ) _jarHash.get( key );
//      if( jarFile == null )
//      {
//          try
//          {
//              File file = new File( _installDirectory + key );
//              if( file.exists() )
//              {
//                  jarFile = new JarFile( file );
//                  _jarHash.put( key,jarFile );
//              }
//              else
//                  throw new java.io.IOException();
//          }
//          catch( IOException e )
//          {
//              System.err.println( "Jarfile not found: " + Database + " " + Language );
//          }
//      }

//      return jarFile;
//  }


//  public static InputStream getCssSheet()
//  {
//      try
//      {
//          return new FileInputStream( _installDirectory + "custom.css" );
//      }
//      catch( FileNotFoundException e )
//      {
//          return null;
//      }
//  }


//  public static InputStream errorFile( String Language )
//  {
//      try
//      {
//          return new FileInputStream( _installDirectory + lang(Language) + File.separator + "err.html" );
//      }
//      catch( IOException e )
//      {
//          String errorFile =
//              " <html><body>         "+
//              "     The requested document does not exist in the database !!            "+
//              " </body></html> ";

//          return new ByteArrayInputStream( errorFile.getBytes() );
//      }
//  }



//  public static InputStream popupDocument( HelpURLParameter xPar )
//  {
//  //          String popupFile =
//  //              " <help:document xmlns:help=\"http://openoffice.org/2000/help\">         "+
//  //              " <help:body>" +
//  //              "  <help:popup-cut Id=\""+xPar.get_id()+"\" Eid=\""+xPar.get_eid()+"\"></help:popup-cut> " +
//  //          1   " </help:body></help:document> ";

//      String popupFile =
//          " <html>                                                                                 "+
//          " <head>                                                                                 "+
//          " <help:css-file-link xmlns:help=\"http://openoffice.org/2000/help\"/>                   "+
//          " </head>                                                                                "+
//          " <body>                                                                                 "+
//          " <help:popup-cut Id=\""+xPar.get_id()+"\" Eid=\""+xPar.get_eid()+"\" xmlns:help=\"http://openoffice.org/2000/help\"></help:popup-cut>  "+
//          " </body>                                                                                "+
//          " </html>                                                                                ";

//      System.out.println( popupFile );

//      return new ByteArrayInputStream( popupFile.getBytes() );
//  }


//  private static final Hashtable _keyword = new Hashtable();






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


struct comp
{
    int operator()( const rtl::OUString& l,const rtl::OUString& r )
    {
        if( l <= r )
            return 1;
        else
            return 0;
    }
};


void KeywordInfo::sort( std::vector< rtl::OUString >& listKey_ )
{
    std::sort( listKey_.begin(),listKey_.end(),comp() );
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

        info->sort( listKey_ );
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

#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif


using namespace chelp;


// The same for the jar files
//  private static final Hashtable _jarHash = new Hashtable();


//  public static final Hashtable _modInfo = new Hashtable();


osl::Mutex                  Databases::m_aMutex;
rtl::OUString               Databases::m_aInstallDirectory;  // Installation directory
Databases::DatabasesTable   Databases::m_aDatabases;         // Language and module dependent databases
Databases::LangSetTable     Databases::m_aLangSet;           // Mapping to of lang-country to lang
Databases::ModInfoTable     Databases::m_aModInfo;           // Module information


void Databases::setInstallPath( const rtl::OUString& aInstallDirectory )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_aInstallDirectory = aInstallDirectory;
}


rtl::OUString Databases::getInstallPath()
{
    osl::MutexGuard aGuard( m_aMutex );

    return rtl::OUString::createFromAscii( "//./e:/src630c/help/" );


    // return m_aInstallDirectory;
}


rtl::OUString Databases::getURLMode()
{
    return rtl::OUString::createFromAscii( "with-jars" );
    // return rtl::OUString::createFromAscii( "with-files" );
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
            ( fileName = fileName.copy(0,idx).toLowerCase() ).compareToAscii( "picture" ) != 0 )
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














//  public static synchronized Db getDatabaseForLanguage( String Database,String Language )
//  {
//      if( Database == null || Language == null )
//          return null;
//      String key = lang(Language) + File.separator + Database;       // For example de/swriter
//      Db table = ( Db ) _dbHash.get( key );
//      if( table == null )
//      {
//          try
//          {
//              table = new Db( null,0 );

//              String tablePath = _installDirectory + key + ".db";
//              table.open( tablePath,null,Db.DB_BTREE,Db.DB_RDONLY,0644 );
//              _dbHash.put( key,table );
//          }
//          catch( DbException e )
//          {
//              System.err.println( "Unsupported language in helpsystem: " + Language );
//              System.err.println( "module: " + Database );
//              System.err.println( e.getMessage() );
//          }
//          catch( FileNotFoundException e )
//          {
//              System.err.println( "Unsupported language in helpsystem: " + Language );
//              System.err.println( "module: " + Database );
//              System.err.println( e.getMessage() );
//          }
//      }

//      return table;
//  }


//  static Hashtable _helptextHash = new Hashtable();


//  public static synchronized Db getHelptextDbForLanguage( String Database,String Language )
//  {
//      String key = lang( Language ) + File.separator + Database;       // For example de/swriter
//      Db table = ( Db ) _helptextHash.get( key );
//      if( table == null )
//      {
//          try
//          {
//              table = new Db( null,0 );

//              String tablePath = _installDirectory + key + ".ht";
//              table.open( tablePath,null,Db.DB_BTREE,Db.DB_RDONLY,0644 );
//              _dbHash.put( key,table );
//          }
//          catch( DbException e )
//          {
//              System.err.println( "Unsupported language in helpsystem: " + Language );
//              System.err.println( "module: " + Database );
//              System.err.println( e.getMessage() );
//          }
//          catch( FileNotFoundException e )
//          {
//              System.err.println( "Unsupported language in helpsystem: " + Language );
//              System.err.println( "module: " + Database );
//              System.err.println( e.getMessage() );
//          }
//      }

//      return table;
//  }



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


//  public static final class KeywordInfo
//  {
//      int pos = 0;
//      String[] listKey = new String[100];
//      String[][] listId;
//      String[][] listAnchor;
//      String[][] listTitle;

//  public String [] getKeywordList()
//      {
//          return listKey;
//      }

//  public String[][] getIdList()
//      {
//          return listId;
//      }


//  public String[][] getAnchorList()
//      {
//          return listAnchor;
//      }

//  public String[][] getTitleList()
//      {
//          return listTitle;
//      }

//  private void realloc( int length )
//      {
//          String[] buff = listKey;
//          listKey = new String[ length ];
//          int count = ( listKey.length > buff.length ) ? buff.length : listKey.length;
//          for( int i = 0; i < count; ++i )
//              listKey[i] = buff[i];
//          buff = null;
//      }


//  public String[] getTitleForIndex( int i )
//      {
//          return listTitle[i];
//      }


//      void insert( String id )
//      {
//          if( pos == listKey.length )
//              realloc( pos + 100 );
//          listKey[ pos++ ] = id;
//      }


//  public String[] insertId( int index,String ids )
//      {
//          int pos = 0;
//          String[] test = new String[10];
//          while( ids != null && ids.length() != 0 )
//          {
//              int idx = ids.indexOf( ';' );
//              if( pos == test.length )
//              {
//                  String[] buff = test;
//                  test = new String[ pos+10 ];
//                  for( int i = 0; i < buff.length; ++i )
//                      test[i] = buff[i];
//                  buff = null;
//              }

//              test[pos++] = ids.substring(0,idx);
//              ids = ids.substring( 1+idx );
//          }

//          String[] buff = test;
//          test = new String[ pos ];
//          for( int i = 0; i < pos; ++i )
//              test[i] = buff[i];

//          listId[index] = test;
//          listAnchor[index] = new String[test.length];

//          for( int k = 0; k < listId[index].length; ++k )
//          {
//              if( listId[index][k] == null )
//              {
//                  listId[index][k] = "";
//                  listAnchor[index][k] = "";
//              }
//              else
//              {
//                  int idx = listId[index][k].indexOf('#');
//                  if( idx != -1 )
//                  {
//                      listAnchor[index][k] = listId[index][k].substring(1+idx).trim();
//                      listId[index][k] = listId[index][k].substring(0,idx).trim();
//                  }
//                  else
//                      listAnchor[index][k] = "";
//              }
//          }

//          listTitle[index] = new String[test.length];
//          return test;
//      }



//  public void sort()
//      {
//          realloc( pos );
//          Arrays.sort( listKey );
//          listId = new String[ listKey.length ][];
//          listAnchor = new String[ listKey.length ][];
//          listTitle = new String[ listKey.length ][];
//      }
//  }


//  public static synchronized KeywordInfo getKeyword( String Database, String Language )
//  {
//      String keyStr = lang(Language) + File.separator + Database;
//      KeywordInfo info = ( KeywordInfo ) _keyword.get( keyStr );

//      if( info == null )
//      {
//          try
//          {
//              HashMap internalHash = new HashMap();
//              String fileName = HelpDatabases.getInstallDirectory() + keyStr + ".key";
//              Db table = new Db( null,0 );
//              System.err.println( fileName );
//              table.open( fileName,null,Db.DB_BTREE,Db.DB_RDONLY,0644 );
//              Dbc cursor = table.cursor( null,0 );
//              StringDbt key = new StringDbt();
//              StringDbt data = new StringDbt();

//              boolean first = true;
//              key.set_flags( Db.DB_DBT_MALLOC );      // Initially the cursor must allocate the necessary memory
//              data.set_flags( Db.DB_DBT_MALLOC );
//              info = new KeywordInfo();

//              String keyStri;

//              while( Db.DB_NOTFOUND != cursor.get( key,data,Db.DB_NEXT ) )
//              {
//                  keyStri = key.getString();
//                  info.insert( keyStri );
//                  internalHash.put( keyStri,data.getString() );
//                  if( first )
//                  {
//                      key.set_flags( Db.DB_DBT_REALLOC );
//                      data.set_flags( Db.DB_DBT_REALLOC );
//                      first = false;
//                  }
//              }

//              info.sort();
//              cursor.close();
//              table.close( 0 );

//              String[] keywords = info.getKeywordList();
//              Db table2 = getDatabaseForLanguage( Database,Language );
//              for( int i = 0; i < keywords.length; ++i )
//              {
//                  String[] id = info.insertId( i,((String)internalHash.get( keywords[i])) );
//                  String[] title = info.getTitleForIndex( i );

//                  for( int j = 0; j < id.length; ++j )
//                  {
//                      StringDbt key1 = new StringDbt();
//                      key1.setString( id[j] );
//                      StringDbt data1 = new StringDbt();
//                      try
//                      {
//                          table2.get( null,key1,data1,0 );
//                          title[j] = data1.getTitle();
//                      }
//                      catch( Exception e )
//                      {
//                          e.printStackTrace();
//                          title[j] = "";
//                      }
//                  }
//              }

//              _keyword.put( keyStr,info );
//          }
//          catch( Exception e )
//          {
//              e.printStackTrace();
//              System.err.println( "any other exception in getKeyword: " + e.getMessage() );
//          }
//      }

//      return info;
//  }

//  }    // end class HelpDatabases


//  }

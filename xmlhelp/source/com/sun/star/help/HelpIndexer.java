/**
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */
package com.sun.star.help;


import java.io.*;
import java.net.URL;
import java.util.*;

import org.w3c.dom.*;
import com.sun.xml.tree.*;
import org.xml.sax.*;
import javax.xml.parsers.*;
import com.jclark.xsl.sax.*;

import com.sun.xmlsearch.util.*;
import com.sun.xmlsearch.xml.qe.*;
import com.sun.xmlsearch.xml.indexer.*;

import com.sleepycat.db.*;
import com.sun.star.help.HelpURLStreamHandlerFactory;

public class HelpIndexer {

    private HelpURLStreamHandlerFactory _urlHandler = null;
    private String _language = null, _module = null, _system = null;


    public HelpIndexer( HelpURLStreamHandlerFactory urlHandler,String language, String module, String system )
      {
           _urlHandler = urlHandler;
        _system = system;
          _language = language;
        _module = module;
      }


       private final class TagInfo
    {
        String _tag,_id;

          public TagInfo( String id,String tag )
           {
            _tag = tag;
             _id = id;
        }

        public String get_tag()
          {
            return _tag;
        }

         public String get_id()
          {
               return _id;
           }
    }


    private final class DocInfo
     {
          private String _url = null,_id = null;

        private ArrayList _helptags = new ArrayList();


          public void append( String id,String tag )
        {
             _helptags.add( new TagInfo( id,tag ) );
         }


          public String getURL()
           {
            return _url;
        }

        public void setURL( String url )
          {
            _url = url;
          }

        public void setId( String id )
          {
            _id = id;
          }

           public String getId()
        {
            return _id;
        }

        public ArrayList getAppendices()
        {
            return _helptags;
        }
      }


    Hashtable _hashDocInfo = new Hashtable();
    Hashtable _hashHelptext = new Hashtable();


     private void schnitzel()
      {
        // Determine the location of the database
        String installDirectory = HelpDatabases.getInstallDirectory();
        Db table = null;

          try
        {
            table = new Db( null,0 );
             table.set_error_stream( System.err );
            table.set_errpfx( "indexing" );


               // Create indexDirectory, if not existent
             String indexDirectory = installDirectory + _language + File.separator + _module + ".idx";
              File indexDir = new File( indexDirectory );
               if( indexDir.exists() && indexDir.isFile() )
                indexDir.delete();

            if( ! indexDir.exists() )
                indexDir.mkdir();

            indexDir = null;

             // Initialize the indexBuilder
            XmlIndexBuilder builder = new XmlIndexBuilder( indexDirectory );

             String[] translations = { "vnd.sun.star.help://", "#HLP#" };
            PrefixTranslator translator = PrefixTranslator.makePrefixTranslator( translations );
            builder.setPrefixTranslator( translator );

            builder.clearIndex();               // Build index from scratch
            builder.setTransformLocation( installDirectory + "..\\StyleSheets\\Indexing\\" );
            builder.init( "index" );


             // Determine and read the database
            String fileName = installDirectory
                              + _language
                            + File.separator
                            + _module
                               + ".db";

            table.open( fileName,null,Db.DB_BTREE,Db.DB_RDONLY,0644 );
            Dbc cursor = table.cursor( null,0 );
            StringDbt key = new StringDbt();
            StringDbt data = new StringDbt();

            boolean first = true;
              key.set_flags( Db.DB_DBT_MALLOC );      // Initially the cursor must allocate the necessary memory
             data.set_flags( Db.DB_DBT_MALLOC );
            while( Db.DB_NOTFOUND != cursor.get( key,data,Db.DB_NEXT ) )
            {
                try
                {
                       String keyStr = key.getString();
                    String dataStr = data.getFile();
                    String tagStr = data.getHash();

                    DocInfo info = ( DocInfo ) _hashDocInfo.get( dataStr );

                     if( info == null )
                     {
                        info = new DocInfo();
                        _hashDocInfo.put( dataStr,info );
                    }

                      if( ! tagStr.equals( "" ) )
                          info.append( keyStr,tagStr );
                    else
                     {
                        String url = "vnd.sun.star.help://" + _module + "/" + keyStr + "?Language=" + _language;
                        info.setURL( url );
                        info.setId( keyStr );
                      }
                }
                catch( Exception e )
                   {
                  }
                   if( first )
                  {
                    key.set_flags( Db.DB_DBT_REALLOC );
                     data.set_flags( Db.DB_DBT_REALLOC );
                     first = false;
                }
            }
            cursor.close();
               table.close( 0 );

            System.out.println( "Indexing..." );
              Enumeration enum = _hashDocInfo.elements();
            int cut = 0;
             while( enum.hasMoreElements() && cut < 10000000 )
             {
                  try
                {
                      DocInfo info = ( DocInfo ) enum.nextElement();
                     String url = info.getURL();
                    if( url == null )
                     {
                         System.out.println( "<----------------------------------->" );
                         System.out.println( "big error: found helptext without URL" );
                          System.out.println( "<----------------------------------->" );
                          continue;
                      }
                      cut++;

                       _urlHandler.setMode( null );
                      byte[] embResolved = getSourceDocument( url );
//                  InputSource in = new InputSource( new ByteArrayInputStream( embResolved ) );

                    ByteArrayInputStream inbyte = new ByteArrayInputStream(embResolved);
                    InputStreamReader inread;
                    try
                    {
                        inread = new InputStreamReader( inbyte,"UTF8" );
                    }
                    catch( UnsupportedEncodingException e )
                    {
                        inread = new InputStreamReader( inbyte );
                    }
                     InputSource in = new InputSource( inread );

                      in.setEncoding( "UTF8" );
                    Document docResolved = null;
                    try
                    {
                        docResolved = XmlDocument.createXmlDocument( in,false );
                    }
                    catch( Exception e )
                    {
                        if( docResolved == null )
                            System.err.println( "Nullpointer" );

                        System.err.println( e.getMessage() );
                    }

                     String id = info.getId();
                      if( id == null )
                           System.out.println( "Found tag without valid id" );
                    else
                        addKeywords( docResolved,info.getId() );

                      Object[] attrList = info.getAppendices().toArray();
                       for( int i = 0; i < attrList.length; ++i )
                    {
                        TagInfo tag = ( TagInfo ) attrList[i];
                         Node node = extractHelptext( docResolved,tag.get_tag() );
                           if( node != null )
                           {
                            String text = dump(node);
                            _hashHelptext.put( tag.get_id(),text );
                           }
                    }
                     _urlHandler.setMode( embResolved );

                    int idx = url.indexOf( '?' );
                    if( idx != -1 )
                         url = url.substring( 0,idx );
                    System.out.println( url );
                      builder.indexDocument( new URL( url ),"" );
                }
                catch( Exception e )
                {
                    System.err.println( e.getMessage() );
                }
             }

            try
            {
                dumpHelptext();
                _keywords.dump();
                builder.close();
            }
            catch( Exception e )
            {
                System.err.println( e.getMessage() );
            }
        }
         catch( DbRunRecoveryException e )
        {
            System.out.println( "Not able to create cursor: " + e.getMessage() );
             System.exit(1);
        }
        catch( DbException e )
        {
            System.out.println( "Error initializing database" );
             System.exit(1);
        }
        catch (FileNotFoundException fnfe)
        {
            System.err.println("HelpAccess: " + fnfe.getMessage() );
            System.exit(1);
        }
          catch( java.lang.Exception e )
        {
             System.out.println( "any other exception" + e.getMessage() );
         }
       }



    private final class NodeIterator
    {
        private final class StackElement
          {
            public boolean _isDone;
              public Node _node;

            StackElement( Node node )
              {
                _isDone = false;
                  _node = node;
            }
        }

        private Stack stack = new Stack();

        public NodeIterator( Node node )
          {
            stack.push( new StackElement(node) );
        }

         void change()
          {
               ((StackElement)(stack.peek()))._isDone = true;
              NodeList top = ((StackElement)(stack.peek()))._node.getChildNodes();
              for( int i = top.getLength()-1; i >= 0; --i )
                stack.push( new StackElement( top.item(i) ) );
           }

         public Node next()
          {
               if( stack.empty() ) return null;
            while( ! ((StackElement)(stack.peek()))._isDone ) change();
               return ((StackElement)stack.pop())._node;
        }
    }


    /**
     *  Given a dom of the document, the next Help:Helptext following Help:HelpID value="tag" is extracted
     */

    private Node extractHelptext( Node node,String tag )
    {
        boolean found = false;
        Node test;
        NodeIterator it = new NodeIterator( node );
          while( ( test=it.next() ) != null )
        {
            if( !found && test.getNodeName().equals("help:help-id" ) && ((Element)test).getAttribute("value").equals(tag) )
                   found = true;
               if( found && test.getNodeName().equals("help:help-text") )
                 return test;
          }
        return null;
    }


    private final class Keywords
    {
        private Hashtable _hash = new Hashtable();

         class Data
          {
               int pos = 0;
               String[] _idList = new String[5];

             void append( String id )
              {
                if( pos == _idList.length )
                {
                    String[] buff = _idList;
                    _idList = new String[ pos + 5 ];
                       for( int i = 0; i < buff.length; ++i )
                          _idList[i] = buff[i];
                     buff = null;
                }
                _idList[ pos++] = id;
               }


            int getLength()
            {
                return pos;
            }

            String getString()
            {
                String ret = new String();
                for( int i = 0; i < pos; ++i )
                 {
                      ret += ( _idList[i] + ";" );
                  }
                   return ret;
            }
           }   // end class data


        public void insert( String key, String id )
          {
            Data data = (Data)_hash.get(key);
              if( data == null )
            {
                 data = new Data();
                   _hash.put( key,data );
             }
              data.append( id );
        }


         void dump()
          {
            Enumeration enum = _hash.keys();
               int j = 0;
            String[] list = new String[ _hash.size() ];
               while( enum.hasMoreElements() )
              {
                  list[j++] = ( String ) enum.nextElement();
               }

            Db table;
              try
            {
                table = new Db( null,0 );

                String fileName = HelpDatabases.getInstallDirectory()
                                    + _language
                                  + File.separator
                                  + _module
                                     + ".key";

                table.open( fileName,null,Db.DB_BTREE,Db.DB_CREATE,0644 );

                for( int i = 0; i < list.length; ++i )
                {
                    Data data = ( Data ) _hash.get( list[i] );
                      StringDbt key = new StringDbt( list[i] );
                    StringDbt value = new StringDbt( data.getString() );
                     table.put( null,key,value,0);
                }
                 table.close( 0 );
            }
             catch( Exception e )
              {
                System.out.println( "error writing keydata" );
            }
           }

    }


     void dumpHelptext()
     {
        Enumeration enum = _hashHelptext.keys();
           int j = 0;
        String[] list = new String[ _hashHelptext.size() ];
           while( enum.hasMoreElements() )
          {
              list[j++] = ( String ) enum.nextElement();
        }

        Db table;
          try
        {
            table = new Db( null,0 );

            String fileName = HelpDatabases.getInstallDirectory()
                              + _language
                            + File.separator
                            + _module
                               + ".ht";

            table.open( fileName,null,Db.DB_BTREE,Db.DB_CREATE,0644 );

            for( int i = 0; i < list.length; ++i )
            {
                String data = ( String ) _hashHelptext.get( list[i] );
                  StringDbt key = new StringDbt( list[i] );
                StringDbt value = new StringDbt( data );
                table.put( null,key,value,0);
            }
            table.close( 0 );
        }
        catch( Exception e )
        {
            System.out.println( "error writing keydata" );
        }
    }




    Keywords _keywords = new Keywords();

    private void addKeywords( Node node,String id )
    {
          Node test;
          NodeIterator it = new NodeIterator( node );
        while( ( test=it.next() ) != null )
         {
              if( test.getNodeName().equals( "help:key-word" ) )
            {
                String keyword = (( Element ) test).getAttribute( "value" );
                String getJump = (( Element ) test).getAttribute( "tag" );
                if( ! keyword.equals("") && !id.equals("") )
                    _keywords.insert( keyword,id + "#" + getJump );
             }
          }
    }



    /**
     *  Returns a textual representation of
     *  the node
     */


    private String dump( Node node )
    {
        String app = new String();
        if( node.hasChildNodes() )
          {
            NodeList list = node.getChildNodes();
            for( int i = 0; i < list.getLength(); ++ i )
                 app += dump( list.item(i) );
        }
         if( node.getNodeType() == Node.ELEMENT_NODE )
          {
            /*
             String start = "<" + node.getNodeName();
               NamedNodeMap attr = node.getAttributes();
               for( int j = 0; j < attr.getLength(); ++j )
               {
                start += ( " " + ((Attr)attr.item(j)).getName()+"=\"" + ((Attr)attr.item(j)).getValue() + "\"");
            }
            start += ">";
              String end = "</" + node.getNodeName() + ">";

               return start + app + end;
            */
           }
        else if( node.getNodeType() == Node.TEXT_NODE )
        {
            return ((Text)node).toString();
        }
        return app;
    }



    // This is a configurable class, which capsulates the parser initialization stuff and all this things

       public static final class ParseStuff
    {
        private final XSLProcessor _processor;
         private final OutputMethodHandlerImpl _output;


        private ParseStuff()
        {
            _processor = new XSLProcessorImpl();

            // Determine the parser
             SAXParserFactory spf = SAXParserFactory.newInstance();
              spf.setValidating( false );
            try
            {
                _processor.setParser( spf.newSAXParser().getParser() );
            }
            catch( java.lang.Exception e )
            {
                System.err.println( "<!-- NO HELP AVAILABLE: no parser found -->" );
                 System.exit( 1 );
            }

            // Determine the OutputMethodHandler
            _output = new OutputMethodHandlerImpl( _processor );
            _processor.setOutputMethodHandler( _output );
        }

        // Loading from a URL
          public ParseStuff( URL url )
           {
            this();
            try
            {
                _processor.loadStylesheet( new InputSource( url.toExternalForm() ) );
            }
            catch( SAXException e )
            {
                System.err.println( "<!-- Syntactic error in stylesheet -->" );
                System.err.println( e.getMessage() );
                   System.exit( 1 );
            }
            catch( java.io.IOException e )
            {
                System.err.println( "<!-- Style sheet not found -->" );
                System.err.println( e.getMessage() );
                   System.exit( 1 );
            }
            System.out.println( "Using stylesheet: " + url.toExternalForm() );
        }


        // Loading from a file
          public ParseStuff( String styleSheet )
           {
            this();
            try
            {
                _processor.loadStylesheet( new InputSource( new FileInputStream( styleSheet ) ) );
            }
            catch( SAXException e )
            {
                System.err.println( "<!-- Syntactic error in stylesheet -->" );
                System.err.println( e.getMessage() );
                   System.exit( 1 );
            }
            catch( java.io.IOException e )
            {
                System.err.println( "<!-- Style sheet not found: -->" );
                   System.exit( 1 );
            }
            System.out.println( "Using stylesheet: " + styleSheet );
        }


          public byte[] parse( String url )
        {
            InputSource _in = new InputSource( url );
               _in.setEncoding( "UTF-8" );


            HelpOutputStream _out = new HelpOutputStream();
               try
            {
                  // OutputStreamDestination _dest = new OutputStreamDestination( _out );
                HelpProvider.ProviderDestination _dest = new HelpProvider.ProviderDestination( _out,"UTF-8" );
                  synchronized( this )
                 {
                    _output.setDestination( _dest );
                    _processor.parse( _in );
                    _out.flush();
                }
             }
            catch( java.io.IOException e )
            {
                System.err.println( "no file corresponding to URL exists: " + url );
                System.err.println( "no file corresponding to URL exists: " + e.getMessage() );
                // System.exit( 1 );
            }
            catch( SAXException e )
            {
                System.err.println( "ill formed xml document: " + e.getMessage() );
                System.err.println( "                    url: " + url );
                // System.exit( 1 );
            }
            catch( Exception e )
            {
                System.err.println( "any other exception" );
                System.err.println( e.getMessage() );
            }

               return _out.getBigBuffer();
         }


          public void setParameter( String key,Object value )
           {
            _processor.setParameter( key,value );
        }

    }   // end class ParseStuff



    /**
       *   Returns the embedding resolved document
     */

    ParseStuff _stuff = null;

    byte[] getSourceDocument( String url )
     {
          // Initialize
          if( _stuff == null )
        {
              String styleSheet = HelpDatabases.getInstallDirectory() + "..\\StyleSheets\\Indexing\\ResEmb.xsl";
            _stuff = new ParseStuff( styleSheet );

             // Setting the parameters
             _stuff.setParameter( "Language", _language );
             _stuff.setParameter( "Database", _module );
        }

        // and parse
        return _stuff.parse( url );
      }




       // Now has to be settled up
    public static void main( String[] args ) throws Exception
    {
         System.setProperty( "XMLSEARCH", HelpDatabases.getInstallDirectory() );
        if( args.length != 6 )
          {
            System.out.println( "Usage example: main -language de -module swriter -system WIN" );
              System.exit( 1 );
        }

         String language = null,module = null, system = null;

          for( int i = 0; i < 5; i+=2 )
           {
             if( args[i].trim().equals( "-language" ) )
                language = args[i+1];
               else if( args[i].trim().equals( "-module" ) )
                 module = args[i+1];
               else if( args[i].trim().equals( "-system" ) )
                 system = args[i+1];
        }

           if( language == null || module == null || system == null )
        {
            System.out.println( "Usage example: main -language de -module swriter -system WIN" );
              System.exit( 1 );
        }
        else
        {
              System.out.println( " Configuring for \"system\" = " + system);
              System.out.println( "                 \"module\" = " + module);
              System.out.println( "               \"language\" = " + language);
          }

        try
          {
            String urlmode = HelpDatabases.getURLMode();
              HelpURLStreamHandlerFactory urlHandler = new HelpURLStreamHandlerFactory( urlmode );
              URL.setURLStreamHandlerFactory( urlHandler );

            HelpIndexer helpIndexer = new HelpIndexer( urlHandler,language,module,system );

            // helpIndexer.indexDatabase();
             helpIndexer.schnitzel();
          }
        catch( Exception e )
        {
              e.printStackTrace();
        }
    }
}


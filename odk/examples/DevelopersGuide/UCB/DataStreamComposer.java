// base classes
import com.sun.star.ucb.*;
import com.sun.star.io.XInputStream;

/**
 * Setting (Storing) the Content Data Stream of a UCB Document Content.
 */
public class DataStreamComposer {

    /**
     * Member properties
     */
    private  Helper    m_helper;
    private  XContent  m_content;
    private  String    m_connectString = "";
    private  String    m_contenturl    = "";
    private  String    m_srcURL        = "";


    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -connect=socket,host=..., port=...
     *                          -url=..
     *                          -sourceDataConnect=... (optional).
     *                          -srcURL=...            (optional).
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public DataStreamComposer( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );
        String connect = getConnect();
        String url     = getContentURL();

        // Init
        m_helper       = new Helper( connect, url );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     * Write the document data stream of a document content.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@return boolean   Result
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     *@exception  java.lang.Exception
     */
    public boolean setDataStream()
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

         String sourceURL         = getSourceURL();
         return ( setDataStream( sourceURL ));
    }

    /**
     * Write the document data stream of a document content.
     *
     *@param  String    Source URL
     *@return boolean   Returns true if data stream successfully seted, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     *@exception  java.lang.Exception
     */
    public boolean setDataStream( String sourceURL )
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

         XInputStream stream;
         if ( sourceURL == null || sourceURL.equals("") )  {
            stream = new MyInputStream();
         } else {
            String connect = getConnect();
            String[] args =  new String[ 2 ];
            args[ 0 ] = "-connect=" + connect;
            args[ 1 ] = "-url=" + sourceURL;
            DataStreamRetriever access = new DataStreamRetriever( args );
            stream = access.getDataStream();
         }
         return ( setDataStream( stream ));
    }

    /**
     * Write the document data stream of a document content...
     *
     *@param  XInputStream   Stream
     *@return boolean        Returns true if data stream successfully seted, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean setDataStream( XInputStream stream )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        XInputStream data = stream;
        if ( data != null && m_content != null )  {

            // Fill argument structure...
            InsertCommandArgument arg = new InsertCommandArgument();
            arg.Data = data;
            arg.ReplaceExisting = true;

            // Execute command "insert".
            m_helper.executeCommand( m_content, "insert", arg );
            result = true;
        }
        return result;
    }

    /**
     * Get source URL.
     *
     *@return String    That contains the source URL
     */
    public String getSourceURL() {
        return m_srcURL;
    }

    /**
     *  Get connect URL.
     *
     *@return   String    That contains the connect URL
     */
    public String getContentURL() {
        return m_contenturl;
    }

    /**
     * Get source data connection.
     *
     *@return String    That contains the source data connection
     */
    public String getConnect() {
        return m_connectString;
    }

    /**
     * Parse arguments
     *
     *@param      String[]   Arguments
     *@exception  java.lang.Exception
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-connect=" )) {
                m_connectString = args[i].substring( 9 );
            } else if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-srcURL=" )) {
                m_srcURL = args[i].substring( 8 );
            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_connectString == null || m_connectString.equals( "" )) {
            m_connectString = "socket,host=localhost,port=8100";
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.createTargetDataFile();
        }

        if ( m_srcURL == null || m_srcURL.equals( "" )) {
            m_srcURL = Helper.getAbsoluteFileURL( "data/data.txt" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : DataStreamComposer -connect=socket,host=...,port=... -url=... -srcURL=..." );
        System.out.println(
            "Defaults: -connect=socket,host=localhost,port=8100 -url=<workdir>/data/data.txt -srcURL=<workdir>/data-<uniquepostfix>" );
        System.out.println(
            "\nExample : -url=file:///temp/my.txt -srcURL=file:///temp/src.txt " );
    }


    /**
     *  Create a new connection with the specific args to a running office and
     *  set the Content Data Stream of a UCB Document Content.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "DataStreamComposer - sets the data stream of a document resource." );
        System.out.println(
            " The data stream is obtained from another (the source) document " );
        System.out.println(
            " resource before." );
        System.out.println(
            "-----------------------------------------------------------------" );
        try {

            DataStreamComposer dataStream = new DataStreamComposer( args );
            String sourceURL         = dataStream.getSourceURL();
            boolean result = dataStream.setDataStream( sourceURL );
            if ( result ) {
                System.out.println(
                        "\nSetting data stream succeeded.\n   Source URL: " +
                        dataStream.getSourceURL() +
                        "\n   Target URL: " +
                        dataStream.getContentURL() );
            } else {
                System.out.println(
                        "\nSetting data stream failed. \n   Source URL: " +
                        dataStream.getSourceURL() +
                        "\n   Target URL: " +
                        dataStream.getContentURL() );
            }
        } catch ( com.sun.star.ucb.CommandAbortedException e ) {
            System.out.println( "Error: " + e );
        } catch ( com.sun.star.uno.Exception e ) {
            System.out.println( "Error: " + e );
        } catch ( java.lang.Exception e ) {
            System.out.println( "Error: " + e );
        }
        System.exit( 0 );
    }
}

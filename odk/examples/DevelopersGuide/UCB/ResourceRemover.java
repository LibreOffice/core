// base classes
import com.sun.star.ucb.*;

/**
 * Deleting a resource
 */
public class ResourceRemover {

    /**
     * Member properties
     */
    private  Helper   m_helper;
    private  XContent m_content;
    private  String   m_connectString = "";
    private  String   m_contenturl    = "";

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -connect=socket,host=..., port=...
     *                          -url=..
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public ResourceRemover( String args[] ) throws java.lang.Exception {

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
     *  Delete resource.
     *
     *@return     boolean   Returns true if resource successfully deleted, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean deleteResource()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        if ( m_content != null ) {

            /////////////////////////////////////////////////////////////////////
            // Destroy a resource physically...
            /////////////////////////////////////////////////////////////////////

            Boolean deletePhysically = new Boolean( true );

            // Execute command "delete".
            m_helper.executeCommand( m_content, "delete", deletePhysically );
            result = true;
        }
        return result;
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
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : ResourceRemover -connect=socket,host=...,port=... -url=..." );
        System.out.println(
            "Defaults: -connect=socket,host=localhost,port=8100 -url=<workdir>/data-<uniquepostfix>" );
        System.out.println(
            "\nExample  : -url=file:///temp/MyFile.txt \n" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  delete a resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {

        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "ResourceRemover - destroys a resource." );
        System.out.println(
            "-----------------------------------------------------------------" );

        try {
            ResourceRemover delete = new ResourceRemover( args );
            boolean result = delete.deleteResource();
            String url = delete.getContentURL();
            if ( result )  {
                System.out.println(
                        "Delete of resource " + url + " succeeded." );
            } else  {
                System.out.println(
                        "Delete of resource " + url + " failed." );
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

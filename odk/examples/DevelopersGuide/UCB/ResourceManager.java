// base classes
import com.sun.star.ucb.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Copying, Moving and Creating Links to a Resource
 */
public class ResourceManager {

    /**
     * Member properties
     */
    private  Helper      m_helper;
    private  XInterface  m_ucb;
    private  String      m_connectString;
    private  String      m_contenturl = "";
    private  String      m_srcURL = "";
    private  String      m_targetFolderURL = "";
    private  String      m_transOperation = "";

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -connect=socket,host=..., port=...
     *                          -url=..
     *                          -srcURL=...          (optional).
     *                          -targetFolderURL=... (optional).
     *                          -transOper=...       (optional).
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public ResourceManager( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );
        String connect = getConnect();
        String url     = getContentURL();

        // Init
        m_helper       = new Helper( connect, url );

        // Get xUCB
        m_ucb          = m_helper.getUCB();
    }

    /**
     *  Copy, move or create a link for a resource.
     *  This method requires the main and the optional arguments to be set in order to work.
     *  See Constructor.
     *
     *@return boolean  Returns true if resource successfully transfered, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean transferResource()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        String sourceURL       = getContentURL();      // URL of the source object
        String targetFolderURL = getTargetFolderURL(); // URL of the target folder
        String transOperation  = getTransOperation();
        return transferResource( sourceURL, targetFolderURL, transOperation );
    }

    /**
     *  Copy, move or create a link for a resource.
     *
     *@param  String   Source URL
     *@param  String   Target folder URL
     *@param  String   Transfering operation (copy, move, link)
     *@return boolean  Returns true if resource successfully transfered, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean transferResource(
            String sourceURL, String targetFolderURL, String transOperation )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        if ( m_ucb != null && sourceURL != null && !sourceURL.equals( "" ) &&
             targetFolderURL != null && !targetFolderURL.equals( "" ) &&
             transOperation != null && !transOperation.equals( "" ) &&
             ( transOperation.equals( "copy" ) || transOperation.equals( "move" ) ||
               transOperation.equals( "link" ))) {

            /////////////////////////////////////////////////////////////////////
            // Copy, move or create a link for a resource to another location...
            /////////////////////////////////////////////////////////////////////
            GlobalTransferCommandArgument arg = new GlobalTransferCommandArgument();
            if ( transOperation.equals( "copy" )) {
                arg.Operation = TransferCommandOperation.COPY;
            } else if ( transOperation.equals( "move" )) {
                arg.Operation = TransferCommandOperation.MOVE;
            } else if ( transOperation.equals( "link" )) {
                arg.Operation = TransferCommandOperation.LINK;
            }
            arg.SourceURL = sourceURL;
            arg.TargetURL = targetFolderURL;

            // object keeps it current name
            arg.NewTitle  = "";

            // fail, if object with same name exists in target folder
            arg.NameClash = NameClash.ERROR;

            // Let UCB execute the command "globalTransfer".
            m_helper.executeCommand( m_ucb, "globalTransfer", arg );
            result = true;
        }
        return result;
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
     *  Get connect URL.
     *
     *@return   String    That contains the connect URL
     */
    public String getContentURL() {
        return m_contenturl;
    }

    /**
     * Get trasfering Operation.
     *
     *@return String    That contains the trasfering Operation
     */
    public String getTransOperation() {
        return m_transOperation;
    }

    /**
     * Get target folder URL.
     *
     *@return String    That contains the target folder URL
     */
    public String getTargetFolderURL() {
        return m_targetFolderURL;
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
            } else if ( args[i].startsWith( "-targetFolderURL=" )) {
                m_targetFolderURL = args[i].substring( 17 );
            } else if ( args[i].startsWith( "-transOper=" )) {
                m_transOperation = args[i].substring( 11 );
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
            m_contenturl = Helper.getAbsoluteFileURL( "data/data.txt" );;
        }

        if ( m_targetFolderURL == null || m_targetFolderURL.equals( "" )) {
            m_targetFolderURL = Helper.getCurrentDirAsAbsoluteFileURL();
        }

        if ( m_transOperation == null || m_transOperation.equals( "" )) {
            m_transOperation = "copy";
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage: ResourceManager -connect=socket,host=...,port=... -transOper=... -url=... -targetFolderURL=..." );
        System.out.println(
            "Defaults: -connect=socket,host=localhost,port=8100 -url=<workdir>/data/data.txt> -targetFolderURL=<workdir> -transOper=copy");
        System.out.println(
            "\nExample : -transOper=copy -url=file:///temp/MyFile.txt -targetFolderURL=file:///test/" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  copy, move or create links a resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {

        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "ResourceManager - copies/moves a resource." );
        System.out.println(
            "-----------------------------------------------------------------" );

        try {
            ResourceManager transResource = new ResourceManager( args );
            String sourceURL       = transResource.getContentURL();
            String targetFolderURL = transResource.getTargetFolderURL();
            String transOperation  = transResource.getTransOperation();
            boolean result = transResource.transferResource(
                                sourceURL, targetFolderURL, transOperation );
            if ( result )
                System.out.println( "\nTransfering resource succeeded." );
            else
                System.out.println( "Transfering resource failed." );
            System.out.println( "   Source URL        : " + sourceURL );
            System.out.println( "   Target Folder URL : " + targetFolderURL );
            System.out.println( "   Transfer Operation: " + transOperation );


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

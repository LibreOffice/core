// base classes
import com.sun.star.ucb.*;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.beans.Property;
import com.sun.star.uno.UnoRuntime;
import java.util.*;

/**
 * Retrieve the Children of a UCB Folder Content
 */
public class ChildrenRetriever {

    /**
     * Member properties
     */
    private  Helper   m_helper;
    private  XContent m_content;
    private  String   m_connectString = "";
    private  String   m_contenturl    = "";
    private  Vector   m_propnames      = new Vector();

    /**
     * Constructor. Create a new connection with the specific args to a running office
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -connect=socket,host=..., port=...
     *                          -url=..
     *                          -propNames=... (optional).
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public ChildrenRetriever( String args[] ) throws java.lang.Exception {

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
     * Open a folder content, get properties values.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@return     Vector   Returns children properties values if values successfully retrieved,
     *                     null otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Vector getChildren()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        Vector properties = getProperties();
        return getChildren ( properties );
    }

    /**
     * Open a folder content, get properties values for the properties.
     *
     *@param  Vector   Properties
     *@return Vector   Returns children properties values if values successfully retrieved,
     *                 null otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Vector getChildren( Vector properties )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        Vector result = null;
        if ( m_content != null ) {
            int size = 0;
            if ( properties != null && !properties.isEmpty()) {
                size = properties.size();
            }
            // Fill info for the properties wanted.
            Property[] props = new Property[ size ];
            for ( int index = 0 ; index < size; index++ ) {

                // Define property sequence.
                Property prop = new Property();
                prop.Name = ( String )properties.get( index );
                prop.Handle = -1; // n/a
                props[ index ] = prop;
            }

            // Fill argument structure...
            OpenCommandArgument2 arg = new OpenCommandArgument2();
            arg.Mode = OpenMode.ALL; // FOLDER, DOCUMENTS -> simple filter
            arg.Priority = 32768;    // Final static for 32768
            arg.Properties = props;

            XDynamicResultSet set;

            // Execute command "open".
            set = ( XDynamicResultSet )UnoRuntime.queryInterface(
                XDynamicResultSet.class, m_helper.executeCommand( m_content, "open", arg ));
            XResultSet resultSet = ( XResultSet )set.getStaticResultSet();

            result = new Vector();

            /////////////////////////////////////////////////////////////////////
            // Iterate over children, access children and property values...
            /////////////////////////////////////////////////////////////////////

                // Move to begin.
            if ( resultSet.first() ) {
                XContentAccess contentAccess = ( XContentAccess )UnoRuntime.queryInterface(
                    XContentAccess.class, resultSet );
                XRow row = ( XRow )UnoRuntime.queryInterface( XRow.class, resultSet );

                do {
                    Vector propsValues = new Vector();

                    // Obtain URL of child.
                    String id = contentAccess.queryContentIdentifierString();
                    propsValues.add( id );
                    for ( int i = 1; i <= size ; i++)  {
                        Object propValue = row.getObject( i, null );
                        if ( !row.wasNull() && !(propValue instanceof com.sun.star.uno.Any )) {
                            propsValues.add( propValue );
                        } else {
                            propsValues.add( "[ Property not found ]" );
                        }
                    }
                    result.add( propsValues );
                } while ( resultSet.next() ); // next child
            }
        }
        return result;
    }

    /**
     *  Get connect URL.
     *
     *@return   String  That contains the connect URL
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
     * Get the properties.
     *
     *@return String    That contains the properties
     */
    public Vector getProperties() {
        return m_propnames;
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
            } else if ( args[i].startsWith( "-propNames=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 11 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propnames.add( tok.nextToken() );

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
            m_contenturl    = "file:///";
        }

        if ( m_propnames.size() == 0 ) {
            m_propnames.add( "Title" );
            m_propnames.add( "IsDocument" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : ChildrenRetriever -connect=socket,host=...,port=... -url=... -propNames=..." );
        System.out.println(
            "Defaults: -connect=socket,host=localhost,port=8100 -url=file:/// -propNames=Title,IsDocument" );
        System.out.println(
            "\nExample : -url=file:///temp/ -propNames=Title;IsFolder;IsDocument" );
    }

    /**
     *  Print all properties out contained in vector .
     *
     *@param   Vector
     */
    public void printLine( Vector props ) {
        int limit;
        while ( !props.isEmpty() )   {
            String print = "";
            int size  = props.size();
            for ( int i = 0; i < size; i++ ) {
                limit = 15;
                Object obj = props.get( i );
                if ( obj != null)  {
                    String prop = obj.toString();
                    int leng = prop.length();
                    if ( leng < limit ) {
                        for ( int l = leng; l < limit; l++) {
                            prop += " ";
                        }
                        print+= prop + "  ";
                        props.set( i, null );
                    } else {
                        String temp1 = prop.substring( 0, limit );
                        String temp2 = prop.substring( limit );
                        print+= temp1 + "  ";
                        props.set( i, temp2 );
                    }
                } else  {
                    for ( int l = 0; l < limit; l++) {
                        print += " ";
                    }
                    print+= "  ";
                }
            }
            System.out.println( print );
            boolean isEmpty = true;
            for ( int i = 0; i < size; i++ ) {
            Object obj = props.get( i );
            if( obj != null )
                isEmpty = false;
            }
            if( isEmpty )
                props.clear();
        }
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  access the children from a folder.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {

        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "ChildrenRetriever - obtains the children of a folder resource." );
        System.out.println(
            "-----------------------------------------------------------------" );

        try {
            ChildrenRetriever access = new ChildrenRetriever( args );

            // Get the properties Title and IsFolder for the children.
            Vector result = access.getChildren();

            String tempPrint = "\nChildren of resource " + access.getContentURL();
            int size = tempPrint.length();
            System.out.println( tempPrint );
            tempPrint = "";
            for( int i = 0; i < size; i++ ) {
                tempPrint += "-";
            }
            System.out.println( tempPrint );

            if ( result != null && !result.isEmpty() ) {

                Vector cont = new Vector();
                cont.add("URL:");
                Vector props = access.getProperties();
                size = props.size();
                for ( int i = 0; i < size; i++ ) {
                    Object obj = props.get( i );
                    String prop = obj.toString();
                    cont.add( prop + ":" );
                }
                access.printLine(cont);
                System.out.println( "\n" );
                for ( Enumeration e = result.elements(); e.hasMoreElements(); ) {
                    Vector propsV   = ( Vector )e.nextElement();
                    access.printLine( propsV );
                }
            }
        } catch ( com.sun.star.ucb.ResultSetException e ) {
            System.out.println( "Error: " + e );
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

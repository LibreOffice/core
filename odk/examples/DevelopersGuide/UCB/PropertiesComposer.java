// base classes
import java.util.Vector;
import java.util.StringTokenizer;
import com.sun.star.ucb.*;
import com.sun.star.beans.*;
import com.sun.star.uno.UnoRuntime;

/**
 * Setting Property Values of a UCB Content
 */
public class PropertiesComposer {

    /**
     * Member properties
     */
    private  Helper    m_helper;
    private  XContent  m_content;
    private  String    m_connectString = "";
    private  String    m_contenturl = "";
    private  Vector    m_propNames          = new Vector();
    private  Vector    m_propValues         = new Vector();

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -connect=socket,host=..., port=...
     *                          -url=..
     *                          -propNames=...  (optional).
     *                          -propValues=... (optional).
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public PropertiesComposer( String args[] ) throws java.lang.Exception {

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
     *  Set values of the properties.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@return Object[]  Returns null or instance object of com.sun.star.uno.Any
     *                  if values successfully seted, properties otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Object[] setProperties()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        Vector properties      = getProperties();
        Vector propertyValues  = getPropertyValues();
        return setProperties( properties, propertyValues );
    }

    /**
     *  Set values of the properties.
     *
     *@param  Vector    Properties
     *@param  Vector    Properties value
     *@return Object[]  Returns null or instance object of com.sun.star.uno.Any
     *                  if values successfully seted, properties otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Object[] setProperties( Vector properties, Vector propertiesValues )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        Object[] result = null;
        if ( m_content != null && !properties.isEmpty() &&
             !propertiesValues.isEmpty() &&
             properties.size() == propertiesValues.size() ) {

            /*
            ****     This code is for unregistered properties.     ****

            XPropertyContainer xPropContainer
                    = (XPropertyContainer)UnoRuntime.queryInterface(
                        XPropertyContainer.class, m_content );

            XPropertySetInfo xPropSetInfo = ( XPropertySetInfo )UnoRuntime.queryInterface(
                    XPropertySetInfo.class,
                    m_helper.executeCommand( m_content, "getPropertySetInfo", null ));
            */

            int size = properties.size();
            PropertyValue[] props = new PropertyValue[ size ];
            for ( int index = 0 ; index < size; index++ ) {
                String propName  = ( String )properties.get( index );
                Object propValue = propertiesValues.get( index );

                /*
                ****     This code is for unregistered properties.     ****

                if ( !xPropSetInfo.hasPropertyByName( propName )) {
                    xPropContainer.addProperty(
                        propName, PropertyAttribute.MAYBEVOID, propValue );
                }
                */

                // Define property sequence.
                PropertyValue prop = new PropertyValue();
                prop.Name = propName;
                prop.Handle = -1; // n/a
                prop.Value  = propValue;
                props[ index ] = prop;
            }

            // Execute command "setPropertiesValues".
            Object[] obj =
                ( Object[] )m_helper.executeCommand( m_content, "setPropertyValues", props );
            if ( obj.length == size )
                 result = obj;
        }
        return result;
    }

    /**
     *  Get properties names.
     *
     *@return   Vector    That contains the properties names
     */
    public Vector getProperties() {
        return m_propNames;
    }

    /**
     *  Get properties values.
     *
     *@return   Vector    That contains the properties values
     */
    public Vector getPropertyValues() {
        return m_propValues;
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
            } else if ( args[i].startsWith( "-propNames=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 11 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propNames.add( tok.nextToken() );

            } else if ( args[i].startsWith( "-propValues=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 12 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propValues.add( tok.nextToken() );

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

        if ( m_propNames.size() == 0 ) {
            m_propNames.add( "Title" );
        }

        if ( m_propValues.size() == 0 ) {
            m_propValues.add(
                "new-" + m_contenturl.substring(
                    m_contenturl.lastIndexOf( "/" ) + 1 ) );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : PropertiesComposer -connect=socket,host=...,port=... -url=... -propNames=... -propValues=..." );
        System.out.println(
            "Defaults: -connect=socket,host=localhost,port=8100 -url=<workdir>/data-<uniquepostfix> -propNames=Title -propValues=new-data-<uniquepostfix>" );
        System.out.println(
            "\nExample : -propNames=Title;Foo -propValues=MyRenamedFile.txt;bar" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  set properties of a resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "--------------------------------------------------------" );
        System.out.println(
            "PropertiesComposer - sets property values of a resource." );
        System.out.println(
            "--------------------------------------------------------" );

        try {

            PropertiesComposer setProp = new PropertiesComposer( args );
            Vector properties       = setProp.getProperties();
            Vector propertiesValues = setProp.getPropertyValues();
            Object[] result = setProp.setProperties( properties, propertiesValues );

            String tempPrint = "\nSetting properties of resource " + setProp.getContentURL();
            int size = tempPrint.length();
            System.out.println( tempPrint );
            tempPrint = "";
            for( int i = 0; i < size; i++ ) {
                tempPrint += "-";
            }
            System.out.println( tempPrint );
            if ( result != null )  {
                for ( int index = 0; index < result.length; index++  ) {
                    Object obj = result[ index ];
                    if( obj == null || obj instanceof com.sun.star.uno.Any )
                        System.out.println(
                            "Setting property " + properties.get( index ) + " succeeded." );
                    else
                        System.out.println(
                            "Setting property " + properties.get( index ) + " failed." );
                }
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


package org.openoffice.idesupport;

import java.io.File;
import java.net.ConnectException;
import java.util.Vector;

/**
 * LocalOffice represents a connection to the local office.
 *
 * This class allows to get access to some scripting framework
 * releated functionality of the locally running office. The
 * office has to be started with options appropriate for establishing
 * local connection.
 *
 * @author misha <misha@openoffice.org>
 */
public class LocalOffice
{
    /**
     * Creates an instance of the local office connection.
     *
     * @param parent is an application specific class loader.
     * @param officePath is a platform specific path string
     *   to the office distribution.
     * @param port is a communication port.
     */
    public static final LocalOffice create(
        ClassLoader parent, String officePath, int port)
    {
        Vector      path    = new Vector();
        path.addElement(officePath + "/program/classes/ridl.jar");
        path.addElement(officePath + "/program/classes/jurt.jar");
        path.addElement(officePath + "/program/classes/sandbox.jar");
        path.addElement(officePath + "/program/classes/unoil.jar");
        path.addElement(officePath + "/program/classes/juh.jar");
        path.addElement(System.getProperties().getProperty("netbeans.home") +
            File.separator + "modules" +
            File.separator + "ext" +
            File.separator + "localoffice.jar");
        ClassLoader appcl   = new DefaultScriptClassLoader(parent, path);
        Class       clazz   = null;
        LocalOffice office  = null;
        try {
            clazz   = appcl.loadClass(
                "org.openoffice.idesupport.localoffice.LocalOfficeImpl");
            office  = (LocalOffice)clazz.newInstance();
            office.connect(officePath, port);
        } catch (java.lang.Exception exp) {
            office  = null;
        }
        return office;
    }

    /**
     * Connects to the running office.
     *
     * @param officePath is a platform specific path string
     *   to the office distribution.
     * @param port is a communication port.
     */
    protected void connect(String officePath, int port)
        throws ConnectException
    {
    }

    /**
     * Closes the connection to the running office.
     */
    public void disconnect()
    {
    }

    /**
     * Refresh the script storage.
     *
     * @param uri is an identifier of storage has to be refreshed.
     */
    public void refreshStorage(String uri)
    {
    }
}

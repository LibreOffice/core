/*************************************************************************
 *
 *  $RCSfile: OfficeConnect.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:31:06 $
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

package OfficeDev.samples.DesktopEnvironment;

// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;

// exceptions
import com.sun.star.container.NoSuchElementException;
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

// interfaces
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;

// helper
import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;

// others
import java.lang.String;




// __________ Implementation __________

/**
 * support ONE singleton uno connection to an running office installation!
 * Can be used to open/use/close connection to uno environment of an already running office.
 * ctor isn't available from outside. You should call static function "getConnection()"
 * to open or use internal set connection which is created one times only.
 *
 * @author      Andreas Schl&uuml;ns
 * @created     7. Februar 2002
 * @modified    05.02.2002 12:10
 */
public class OfficeConnect
{
    // ____________________

    /**
     * At first call we create static connection object and open connection to already runing office - if we can.
     * Then - and for all further requests we return these static connection member.
     *
     * @param  sHost  host on which office runs
     * @param  sPort  port on which office can be found
     * @return        Description of the Returned Value
     */
    public static synchronized void createConnection(String sHost, String sPort)
    {
        if (maConnection == null)
            maConnection = new OfficeConnect(sHost, sPort);
    }

    // ____________________

    /**
     * close connection to remote office if it exist
     */
    public static synchronized void disconnect()
    {
        if(maConnection!=null)
        {
            mxServiceManager=null;
            maConnection=null;
        }
    }

    // ____________________

    /**
     * ctor
     * We try to open the connection in our ctor ... transparently for user.
     * After it was successfully you will find an internal set member m_xFactory wich
     * means remote uno service manager of connected office.
     * We made it private to support singleton pattern of these implementation.
     * see getConnection() for further informations
     *
     * @param  sHost  host on which office runs
     * @param  sPort  port on which office can be found
     */
    private OfficeConnect(String sHost, String sPort)
    {
        try
        {

            String sConnectString  = "uno:socket,host=";
                   sConnectString += sHost;
                   sConnectString += ",port=";
                   sConnectString += sPort;
                   sConnectString += ";urp;";                                 // enable oneway calls
//                   sConnectString += ";urp,Negotiate=0,ForceSynchronous=1;";    // disable oneway calls
                   sConnectString += "StarOffice.ServiceManager";

            com.sun.star.lang.XMultiServiceFactory xLocalServiceManager = com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();
            com.sun.star.bridge.XUnoUrlResolver xURLResolver = (com.sun.star.bridge.XUnoUrlResolver)UnoRuntime.queryInterface(
                    com.sun.star.bridge.XUnoUrlResolver.class,
                    xLocalServiceManager.createInstance("com.sun.star.bridge.UnoUrlResolver"));

            mxServiceManager = (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                    com.sun.star.lang.XMultiServiceFactory.class,
                    xURLResolver.resolve(sConnectString));
        }
        catch (com.sun.star.uno.RuntimeException exUNO)
        {
            System.out.println("connection failed" + exUNO);
            System.exit(0);
        }
        catch (com.sun.star.uno.Exception exRun)
        {
            System.out.println("connection failed" + exRun);
            System.exit(0);
        }
        catch (java.lang.Exception exJava)
        {
            System.out.println("connection failed" + exJava);
            System.exit(0);
        }
    }

    // ____________________

    /**
     * create uno components inside remote office process
     * After connection of these proccess to a running office we have access to remote service manager of it.
     * So we can use it to create all existing services. Use this method to create components by name and
     * get her interface. Casting of it to right target interface is part of your implementation.
     *
     * @param  aType              describe class type of created service
     *                            Returned object can be casted directly to this one.
     *                            Uno query was done by this method automaticly.
     * @param  sServiceSpecifier  name of service which should be created
     * @return                    Description of the Returned Value
     */
    public static synchronized Object createRemoteInstance(Class aType, String sServiceSpecifier)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                    aType,
                    mxServiceManager.createInstance(sServiceSpecifier));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.out.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * same as "createRemoteInstance()" but supports additional parameter for initializing created object
     *
     * @param  lArguments         optional arguments
     *                            They are used to initialize new created service.
     * @param  aType              Description of Parameter
     * @param  sServiceSpecifier  Description of Parameter
     * @return                    Description of the Returned Value
     */
    public static synchronized Object createRemoteInstanceWithArguments(Class aType, String sServiceSpecifier, Any[] lArguments)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                    aType,
                    mxServiceManager.createInstanceWithArguments(
                    sServiceSpecifier,
                    lArguments));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.out.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * returns rmote uno servie manager of singleton office instance
     */
    public static synchronized com.sun.star.lang.XMultiServiceFactory getSMGR()
    {
        return mxServiceManager;
    }

    // ____________________

    /**
     * member
     */
    private static OfficeConnect                           maConnection    ;    // singleton connection instance
    private static com.sun.star.lang.XMultiServiceFactory  mxServiceManager;    // reference to remote service manager of singleton connection object
}


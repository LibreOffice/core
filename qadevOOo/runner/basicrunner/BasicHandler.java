/*************************************************************************
 *
 *  $RCSfile: BasicHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:13:01 $
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
package basicrunner;


import com.sun.star.awt.XTextComponent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XContainer;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XSet;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XSortable;
import java.util.Hashtable;
import lib.TestParameters;

import share.LogWriter;


/**
 * This class is a java-part of BASIC-java interaction "driver"
 * It is used to call Star-Basic's function from java using
 * basic's part of "driver" where listeners are implemented.
 * The instance of the BasicHandler should be added to the MSF that will be
 * used for loading BASIC's part of "driver".<br>
 * After opening basic's document it creates an instance of the
 * HandlerContainer using BasicHandler. HandlerContainer is a UNO
 * XContainer and XNameContainer.
 * Only one instance of BasicHandler can be used at the moment.
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.lang.XSingleServiceFactory
 */
public class BasicHandler implements XServiceInfo, XSingleServiceFactory {
    /**
     * serviceName is the name of service that can be created in BASIC.
     */
    static final String serviceName =
                            "com.sun.star.jsuite.basicrunner.BasicHandler";

    /**
     * <code>container</code> is a SHARED variable (between BASIC and Java).
     * It is used for interacting.
     */
    static private HandlerContainer container = null;

    /**
     * Contains a writer to log an information about the interface testing, to
     * allows for tests to access it.
     */
    static private LogWriter log;

    /**
     * <code>oHandlerDoc</code> is a referrence to BASIC's document.
     */
    static private XComponent oHandlerDoc = null;

    /**
     * <code>xMSF</code> is a MultiServiceFactory currently used by
     * BasicHandler.
     */
    static private XMultiServiceFactory xMSF = null;

    /**
     * Interface being tested now.
     */
    static private BasicIfcTest TestedInterface = null;

    /**
     * Ab enhanced scheme of timeouts can be used with BASIC tests.
     * A small timeout can be used zo wait for changes in the test status.
     * <code>respFlag</code> is set to <code>true</code> when a BASIC test
     * writes any log information.
     */
    static private boolean respFlag = false;

    /**
     * <code>iBasicTimeout</code> is the amount of milliseconds that
     * the BasicHandler will wait for a response from tests
     * (finish to execute a method or add log information)
     * before it decides that SOffice is dead.
     */
    static private int iBasicTimeout = 10000;



    /**
     * Creates an instance of a HandlerContainer. This instance is used from
     * BASIC.
     * @param tParam The test parameters.
     */
    public BasicHandler(TestParameters tParam) {
        if (tParam.get("soapi.test.basic.debugFile") != null) {
            iBasicTimeout = 0; // Debug mode.
        }
        container = new HandlerContainer(this);
    }

    /**
     * Set the tested interface and a log writer.
     * @param ifc The test of an interface
     * @param log A log writer.
     */
    public void setTestedInterface(BasicIfcTest ifc, LogWriter log) {
        this.log = log;
        TestedInterface = ifc;
    }

    /**
     * Is called when BASIC signals that it has performed the test of a method.
     * @param methodName The name of the method.
     * @bResult The result of the test.
     */
    synchronized void methodTested(String methodName, boolean bResult) {
        respFlag = true;
        TestedInterface.methodTested(methodName, bResult);
        notify() ;
    }

    /**
     * Is called when BASIC sends a signal to write some log information.
     * @param info The string to write.
     */
    synchronized public void Log(String info) {
        respFlag = true;
        log.println(info);
        notify() ;
    }

    /**
     * Is called by BasicIfcTest to find out if this BasicHandler uses the
     * correct MultiServiceFactory.
     * @param xMSF The MultiServiceFactory
     * @see com.sun.star.lang.XMultiServiceFactory
     * @return True, if xMSF is equal to the MultiServiceFactory of this class.
     */
    public boolean isUptodate(XMultiServiceFactory xMSF) {
        return xMSF.equals(this.xMSF);
    }


    /**
     * Establishes a connection between BASIC and Java.
     * If required, hte BASIC part of the "driver" is loaded.
     * @param sBasicBridgeURL The URL of the basic bridge document
     *                                           (BasicBridge.sxw)
     * @param tParam The test parameters.
     * @param xMSF The MultiServiceFactory
     * @param log The log writer.
     * @see com.sun.star.lang.XMultiServiceFactory
     * @throws ConnectionSetupException Exception is thrown, if no connection could be made.
     */
    public synchronized void Connect(String sBasicBridgeURL,
                            TestParameters tParam, XMultiServiceFactory xMSF,
                            LogWriter log) throws ConnectionSetupException {
        this.log = log;
        try {
            this.xMSF = xMSF;
            Object oInterface = xMSF.createInstance(
                                                "com.sun.star.frame.Desktop");
            XDesktop oDesktop = (XDesktop) UnoRuntime.queryInterface(
                                                XDesktop.class, oInterface);
            XComponentLoader oCLoader = (XComponentLoader)
                                        UnoRuntime.queryInterface(
                                        XComponentLoader.class, oDesktop);

            // load BasicBridge with MarcoEceutionMode = Always-no warn
            //PropertyValue[] DocArgs = null;
            PropertyValue[] DocArgs = new PropertyValue[1];
            PropertyValue DocArg = new PropertyValue();
            DocArg.Name = "MacroExecutionMode";
            DocArg.Value = new Short(
                    com.sun.star.document.MacroExecMode.ALWAYS_EXECUTE_NO_WARN);
            DocArgs[0] = DocArg;

            // configure Office to allow to execute macos
            PropertyValue [] ProvArgs = new PropertyValue [1];
            PropertyValue Arg = new PropertyValue();
            Arg.Name = "nodepath";
            Arg.Value = "/org.openoffice.Office.Common/Security";
            ProvArgs[0] = Arg;

            Object oProvider = xMSF.createInstance(
                        "com.sun.star.configuration.ConfigurationProvider");

            XMultiServiceFactory oProviderMSF = (XMultiServiceFactory)
                                    UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class, oProvider);

            Object oSecure = oProviderMSF.createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess",
                        ProvArgs);

            XPropertySet oSecureProps = (XPropertySet)
                        UnoRuntime.queryInterface(XPropertySet.class, oSecure);

            Object oScripting = oSecureProps.getPropertyValue("Scripting");
            XPropertySet oScriptingSettings = (XPropertySet)
                    UnoRuntime.queryInterface(XPropertySet.class, oScripting);

            oScriptingSettings.setPropertyValue("Warning", Boolean.FALSE);
            oScriptingSettings.setPropertyValue("OfficeBasic", new Integer(2));

            XChangesBatch oSecureChange = (XChangesBatch)
                    UnoRuntime.queryInterface(XChangesBatch.class, oSecure);
            oSecureChange.commitChanges();

            // As we want to have some information about a debugFile
            // BEFORE connection is established
            // we pass the information about it in frame name.
            String sFrameName = (String)tParam.get(
                                                "soapi.test.basic.debugFile");
            if (sFrameName == null) sFrameName = "BasicRunner";

            oHandlerDoc = oCLoader.loadComponentFromURL(sBasicBridgeURL,
                                                    sFrameName, 40, DocArgs);

            do {
                respFlag = false ;
                wait(10000); // waiting for basic response for 10 seconds.
            } while (respFlag && !container.hasByName("BASIC_Done")) ;

            if (!container.hasByName("BASIC_Done")) {
                throw new ConnectionSetupException("Connection timed out.");
            }
        } catch (Exception e) {
            System.out.println("Exception: " + e.toString());
            throw new ConnectionSetupException();
        }

        log.println("Java-BASIC connection established!");
    }

    /**
     * Overloads perform(Strin fName, Object params) for convenience.
     * @return A proprty value as result.
     *
    public synchronized PropertyValue perform(String fName)
                                                        throws BasicException {
        return perform(fName, "");
    }
*/
    /**
     * Perform a test of a method.
     * @param fName The name of the method to test.
     * @param params The test parameters.
     * @return A proprty value as result of the test.
     * @throws BasicException The method could not be executed.
     */
    public synchronized PropertyValue perform(String fName, Object params)
                                                        throws BasicException {
        try {
            container.callBasicFunction(fName, params);

            do {
                respFlag = false;
                // waiting for basic response for iBasicTimeout milliseconds.
                wait(iBasicTimeout);
            } while(respFlag && !container.hasByName("BASIC_Done"));

        } catch (InterruptedException e) {
            System.out.println("The operation " + fName + " was interrupted.");
        } catch (com.sun.star.lang.DisposedException de) {
            System.out.println("## Office is disposed");
        }

        if (!container.hasByName("BASIC_Done")) {
            System.out.println("Operation timed out.");
        }

        Object res = container.getByName("BASIC_Done") ;
        container.removeByName("BASIC_Done");

        if (!(res instanceof PropertyValue)) {
            if (res == null) {
                System.out.println(
                            "BasicBridge returns null");
                throw new BasicException(
                            "BasicBridge returns null");
            } else {
                System.out.println(
                            "BasicBridge returns wrong type: " + res.getClass());
                throw new BasicException(
                            "BasicBridge returns wrong type: " + res.getClass());
            }
        }

        PropertyValue result = (PropertyValue) res ;

        if ((result.Value instanceof String) && (((String)result.Value)).startsWith("Exception")) {
            throw new BasicException((String)result.Value);
        }

        return result;
    }

    /**
     * Returns true, if name is a supported service of this class.
     * @param name The service name.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return serviceName.equals(name);
    }

    /**
     * Return all supported service names.
     * @return All supported services.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {serviceName};
    }

    /**
     * Get the implementation name.
     * @return Implementation name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }

    /**
     * Create an instance of HandlerContainer.
     * Arguments are not supported here, so they will be ignored.
     * @param args The arguments.
     * @return The instance.
     */
    public Object createInstanceWithArguments(Object[] args) {
        return container;
    }

    /**
     * Create an instance of HandlerContainer.
     * @return The instance.
     */
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    /**
     * Dispose the BASIC document.
     */
    public synchronized void dispose() {
        try {
            if (oHandlerDoc != null) {
                oHandlerDoc.dispose();
                wait(1000);
            }
        } catch (Exception e) {
            System.out.println("Exception: " + e.toString());
        }
    }
}


/**
 * This class handles the communication between Java and BASIC.
 * @see com.sun.star.container.XContainer
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.lang.XTypeProvider
 */
class HandlerContainer implements XContainer, XNameContainer, XTypeProvider{

    /** Container for parameters.
     **/
    Hashtable container = new Hashtable(20);
    /**
     * An array of listeners for container events.
     * @see com.sun.star.container.XContainerListener
     */
    static XContainerListener[] listener = null;

    /** The BasicHandler belonging to this handler. **/
    BasicHandler parent = null;

    /**
     * Constructor with the parent BasicHandler.
     * @param par The BasicHandler.
     */
    public HandlerContainer(BasicHandler par) {
        parent = par;
    }

    /**
     * Call a BASIC function, meaning a test method.
     * @param fName The method name.
     * @param args Arguments for the method.
     */
    public void callBasicFunction(String fName, Object args) {
        // BASIC's listener should be called ONLY in this case.
        if (container.containsKey(fName)) {
            container.remove(fName);
        }
        container.put(fName, args);
        if (listener != null) {
            ContainerEvent event = new ContainerEvent();
            event.Element = fName;
            for (int i=0; i<listener.length; i++){
                if (listener[i] != null) {
                    listener[i].elementInserted(event);
                }
            }
        }
    }

    /**
     * Insert an object into the container.
     * @param name The key for the object.
     * @param object The object to insert.
     * @throws IllegalArgumentException Throws this exception when trying to insert null.
     */
    public void insertByName(String name, Object object) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.ElementExistException, com.sun.star.lang.WrappedTargetException {

        // BASIC and Java can insert into the container.
        if (container.containsKey(name)) {
            container.remove(name);
        }
        container.put(name, object);

        PropertyValue result = null ;

        if (object instanceof PropertyValue) {
            result = (PropertyValue)object;
            if (name.equals("BASIC_Done")) {
                synchronized (parent) {
                    parent.notify();
                }
            } else if (name.equals("BASIC_MethodTested")) {
                parent.methodTested(result.Name,
                                ((Boolean)result.Value).booleanValue());
            }
        } else if (name.equals("BASIC_Log")) {
            parent.Log(object.toString());
        }
    }

    /**
     * Remove the object with this name from the container.
     * @param name The key.
     */
    public void removeByName(String name) {
        container.remove(name) ;
    }

    /**
     * Unsupported method.
     * @param name The name of the key.
     * @param value The value.
     * @throws WrappedTargetException Throws this exception when called falsely.
     */
    public void replaceByName(String name, Object value)
                                          throws WrappedTargetException {
        throw new WrappedTargetException("Unsupported");
    }

    /**
     * Has a value for this key.
     * @param name The name of a key.
     * @return True, if name exists as key in the container.
     */
    public boolean hasByName(String name) {
        return container.containsKey(name);
    }

    /**
     * Get an object by its key.
     * @param name The name of the key.
     * @return The object of this key.
     */
    public Object getByName(String name) {
        return container.get(name);
    }

    /**
     * Get all key names.
     * @return All names of keys.
     */
    public String[] getElementNames() {
        String[] res = new String[container.size()];
        return (String[])container.keySet().toArray(res);
    }

    /**
     * Is the xcontainer empty?
     * @return True, if the container has elements.
     */
    public boolean hasElements() {
        return !container.isEmpty();
    }

    /**
     * Get the type of this class.
     * @return The type of this class.
     */
    public Type getElementType() {
        try {
            return new Type(String.class);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Get the implementation id of this class.
     * @return A unique id for this class
     * @see com.sun.star.lang.XTypeProvider
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Get all types of this class.
     * @return All implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i) {
            types[i] = new Type(interfaces[i]);
        }
        return types;
    }

    /**
     * Add a listener
     * @param xListener The listener.
     */
    public void addContainerListener(XContainerListener xListener){
        int length = 0;
        if (listener != null)
            length = listener.length;

        XContainerListener[] mListener =
                        new XContainerListener[length+1];
        for (int i=0; i<length-1; i++) {
            mListener[i] = listener[i];
            // listener already added
            if (((Object)xListener).equals(listener[i]))
                return;
        }
        mListener[length] = xListener;
        listener = mListener;
    }

    /**
     * Remove a listener
     * @param xListener The listener.
     */
    public void removeContainerListener(XContainerListener xListener){
        if (listener != null && listener.length != 0) {
            int length = listener.length;
            XContainerListener[] mListener =
                                    new XContainerListener[length-1];
            boolean found = false;
            int j=0;
            for (int i=0; i<length-1; i++) {
                if (!((Object)xListener).equals(listener[j])) {
                    mListener[i] = listener[j];
                }
                else {
                    j++;
                    found = true;
                }
                j++;
            }
            if (!found) {
                if (((Object)xListener).equals(listener[length-1]))
                    listener = mListener;
            }
            else
                listener = mListener;

        }
    }
}

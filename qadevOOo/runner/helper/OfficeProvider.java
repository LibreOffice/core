/*************************************************************************
 *
 *  $RCSfile: OfficeProvider.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change:$Date: 2003-11-18 16:14:32 $
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
package helper;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.util.XCloseable;
import lib.TestParameters;


/**
 * This class will connect the office and start it if possible
 *
 */
public class OfficeProvider implements AppProvider {
    protected static boolean debug = false;

    /**
     * Dispose the office.
     * This method can only be used, if the office was connected in the first
     * place: getManager() was called first.
     */
    public boolean disposeManager(lib.TestParameters param) {
        XMultiServiceFactory msf = (XMultiServiceFactory) param.getMSF();

        if (msf == null) {
            return true;
        } else {
            XDesktop desk = null;

            try {
                desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                                                            msf.createInstance(
                                                                    "com.sun.star.frame.Desktop"));
            } catch (com.sun.star.uno.Exception ue) {
                return false;
            }

            msf = null;

            if (desk != null) {
                desk.terminate();

                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Method to get the ServiceManager of an Office
     */
    public Object getManager(lib.TestParameters param) {
        String additionalArgs = (String) param.get("AdditionalConnectionArguments");
        if ( additionalArgs == null) {
            additionalArgs = ";";
        } else {
            additionalArgs = ","+additionalArgs+";";
        }

        String cncstr = "uno:" + param.get("ConnectionString") +
                        ";urp"+additionalArgs+"StarOffice.ServiceManager";

        System.out.print("Connecting the Office");
        System.out.println(" with "+cncstr);

        debug = param.getBool("DebugIsActive");

        XMultiServiceFactory msf = connectOffice(cncstr);

        if (msf == null) {
            String exc = "";
            boolean isExecutable = false;
            boolean isAppKnown = cncstr.indexOf("host=localhost") > 0;
            isAppKnown &= !((String) param.get("AppExecutionCommand")).equals(
                                   "");

            if (isAppKnown) {
                if (debug) {
                    System.out.println(
                            "Local Connection trying to start the Office");
                }

                String cmd = (String) param.get("AppExecutionCommand");
                ProcessHandler ph = new ProcessHandler(cmd);
                isExecutable = ph.executeAsynchronously();

                if (isExecutable) {
                    param.put("AppProvider", ph);

                    OfficeWatcher ow = new OfficeWatcher(param);
                    param.put("Watcher", ow);
                    ow.start();
                    ow.ping();
                }

                int k = 0;

                while ((k < 11) && (msf == null)) {
                    try {
                        Thread.sleep(k * 1000);
                        msf = connect(cncstr);
                    } catch (com.sun.star.uno.Exception ue) {
                        exc = ue.getMessage();
                    } catch (java.lang.Exception je) {
                        exc = je.getMessage();
                    }

                    k++;
                }

                if (msf == null) {
                    System.out.println("Exception while connecting.\n" +
                                       exc);
                }
            } else {
                System.out.println("Could not connect an Office" +
                                   " and cannot start one.");
            }
        }

        return msf;
    }

    /**
     * Connect an Office
     */
    protected static XMultiServiceFactory connect(String connectStr)
                                           throws com.sun.star.uno.Exception,
                                                  com.sun.star.uno.RuntimeException,
                                                  com.sun.star.connection.NoConnectException,
                                                  Exception {
        // Get component context
        XComponentContext xcomponentcontext = com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
                                                      null);

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
        Object xUrlResolver = xLocalServiceManager.createInstanceWithContext(
                                      "com.sun.star.bridge.UnoUrlResolver",
                                      xcomponentcontext);
        XUnoUrlResolver urlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                                              XUnoUrlResolver.class,
                                              xUrlResolver);

        Object rInitialObject = urlResolver.resolve(connectStr);

        XMultiServiceFactory xMSF = null;

        if (rInitialObject != null) {
            if (debug) {
                System.out.println("resolved url");
            }

            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                           XMultiServiceFactory.class, rInitialObject);
        }

        return xMSF;
    }

    /**
     * Close an office.
     * @param param The test parameters.
     * @param closeIfPossible If true, close even if
     * it was running before the test
     */
    public boolean closeExistingOffice(lib.TestParameters param,
                                       boolean closeIfPossible) {
        debug = param.getBool("DebugIsActive");

        XMultiServiceFactory msf = (XMultiServiceFactory) param.getMSF();
        boolean alreadyConnected = (msf != null);

        if (alreadyConnected) {
            ProcessHandler ph = (ProcessHandler) param.get("AppProvider");

            if (ph != null) {
                disposeOffice(msf,param);


                // dispose watcher in case it's still running.
                //System.out.println("INFO: disposing the office and terminate the watcher process.");

                OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

                if ((ow != null) && ow.isAlive()) {
                    ow.finish = true;
                }

                return true;
            } else {
                if (closeIfPossible) {
                    return disposeOffice(msf, param);
                }
            }
        } else {
            String cncstr = "uno:" + param.get("ConnectionString") +
                            ";urp;StarOffice.ServiceManager";
            msf = connectOffice(cncstr);

            if (closeIfPossible) {
                return disposeOffice(msf, param);
            }
        }

        return true;
    }

    private XMultiServiceFactory connectOffice(String cncstr) {
        XMultiServiceFactory msf = null;
        String exc = "";

        try {
            msf = connect(cncstr);
        } catch (com.sun.star.uno.Exception ue) {
            exc = ue.getMessage();
        } catch (java.lang.Exception je) {
            exc = je.getMessage();
        }

        if (debug) {
            System.out.println("Could not connect an Office. " + exc);
        }

        return msf;
    }

    private boolean disposeOffice(XMultiServiceFactory msf, TestParameters param) {
        XDesktop desk = null;

        if (msf != null) {
            try {
                desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                                                            msf.createInstance(
                                                                    "com.sun.star.frame.Desktop"));
                msf = null;

                if (desk != null) {
                    boolean allClosed = closeAllWindows(desk);
                    if(!allClosed)
                        if(debug) System.out.println("Couldn't close all office windows!");
                    if(debug) System.out.println("Trying to terminate the desktop");
                    desk.terminate();
                    if(debug) System.out.println("Desktop terminated");
                    param.remove("AppProvider");
                    param.remove("ServiceFactory");

                    try {
                        Thread.sleep(5000);
                    } catch (java.lang.InterruptedException ie) {
                    }
                }
            } catch (com.sun.star.uno.Exception ue) {
                return false;
            }
        }

        return true;
    }

    protected boolean closeAllWindows(XDesktop desk) {
        XEnumerationAccess compEnumAccess = desk.getComponents();
        XEnumeration compEnum = compEnumAccess.createEnumeration();
        boolean res = true;
        try {
        while (compEnum.hasMoreElements()){
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, compEnum.nextElement());
            if(closer != null)
                closer.close(true);
        }
        } catch (com.sun.star.util.CloseVetoException cve) {
            res = false;
        } catch (com.sun.star.container.NoSuchElementException nsee) {
            res = false;
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res = false;
        }
        return res;
    }
}

/*************************************************************************
 *
 *  $RCSfile: ThreadRunner.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-03-17 16:25:45 $
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
package basicrunner.basichelper;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.util.XURLTransformer;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.lang.XMultiServiceFactory;
import util.utils;
import com.sun.star.lang.XComponent;
import lib.StatusException;
import lib.Status;
import util.SOfficeFactory;
import com.sun.star.ui.dialogs.XExecutableDialog;


public class ThreadRunner implements XServiceInfo, XSingleServiceFactory {
    static final String __serviceName = "basichelper.ThreadRunner";
    static ThreadRunnerImpl oThreadRunner = null;

    public ThreadRunner(XMultiServiceFactory xMSF) {
    oThreadRunner = new ThreadRunnerImpl(xMSF);
    }

    public Object createInstanceWithArguments(Object[] args) {
    return oThreadRunner;
    }

    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }

    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    public String getImplementationName() {
        return getClass().getName();
    }
}


class ThreadRunnerImpl implements XInitialization, XTypeProvider {
    Object oDoc = null;
    String actionType = null;
    String actionParm = null;
    XMultiServiceFactory oMSF = null;

    public ThreadRunnerImpl(XMultiServiceFactory MSF) {
    oMSF = MSF;
    }

    public void initialize(Object[] params) throws com.sun.star.uno.Exception {
    boolean parmsOK = false;

        if (!(params[0] instanceof String)) {
            throw new StatusException(Status.failed(
                "Wrong first parameter for ThreadRunner, allowed values:" +
                "'OpenToolkitDialog', 'OpenDialogFromFile', 'ExecuteDialog'"));
        }

        actionType = (String) params[0];

        if (actionType.equals("OpenToolkitDialog")) {
            if (params.length != 3 || !(params[2] instanceof Object) ||
                !(params[1] instanceof String) ) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'OpenToolkitDialog': " +
                    "String, Object"));
            }
            actionParm = (String)params[1];
            oDoc = (Object)params[2];
            ToolkitDialogThread aThread =
                new ToolkitDialogThread(oMSF, oDoc, actionParm);
            aThread.start();
        } else if (actionType.equals("OpenDialogFromFile")) {
            if (params.length != 2 || !(params[1] instanceof String) ) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'OpenDialogFromFile': " +
                    "String"));
            }
            actionParm = (String)params[1];
            DialogFromFileThread bThread =
                new DialogFromFileThread(oMSF, actionParm);
            bThread.start();
        } else if ( actionType.equals("ExecuteDialog")) {
            if (params.length != 2 || !(params[1] instanceof String)) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'ExecuteDialog': " +
                    "String"));
            }
            ExecuteDialogThread cThread =
                new ExecuteDialogThread(oMSF, (String)params[1]);
            cThread.start();
        } else {
            System.out.println("Error! ThreadRunnerImpl.initialize(): " +
            "Incorrect parameters!");
        }
    }

    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }
}


class ToolkitDialogThread extends Thread {
    Object oDoc = null;
    String url = null;
    XMultiServiceFactory msf = null;

    public ToolkitDialogThread(XMultiServiceFactory xMSF, Object doc, String sUrl) {
    oDoc = doc;
    url = sUrl;
    msf = xMSF;
    }

    public void run() {
    XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class, oDoc);
    XController xController = aModel.getCurrentController();
    try {
        XDispatchProvider xDispProv = (XDispatchProvider)
        UnoRuntime.queryInterface( XDispatchProvider.class, xController );
        XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
        UnoRuntime.queryInterface(XURLTransformer.class,
                      msf.createInstance("com.sun.star.util.URLTransformer"));
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = url;
        xParser.parseStrict(aParseURL);
        URL aURL = aParseURL[0];
        XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
        if( xDispatcher != null )
        xDispatcher.dispatch( aURL, null );
    } catch (com.sun.star.uno.Exception e) {
        System.out.println("Couldn't open dialog!!!");
        throw new StatusException( "Couldn't open dialog!!!", e );
    }
    }
}


class DialogFromFileThread extends Thread {
    String url = null;
    SOfficeFactory SOF = null;

    public DialogFromFileThread(XMultiServiceFactory xMSF, String sUrl) {
    url = sUrl;
    SOF = SOfficeFactory.getFactory(xMSF);
    }

    public void run() {
    try {
        String testUrl= utils.getFullTestURL(url);
        System.out.println("loading "+testUrl);
        XComponent xDoc = SOF.loadDocument(testUrl);
    } catch (com.sun.star.uno.Exception e) {
        System.out.println("Couldn't create document!!!");
        throw new StatusException( "Couldn't create document!!!", e );
    }
    }

}

class ExecuteDialogThread extends Thread {
    XMultiServiceFactory xMSF = null;
    String serviceName = null;

    public ExecuteDialogThread(XMultiServiceFactory xMSF, String serviceName) {
        this.xMSF = xMSF;
        this.serviceName = serviceName;
    }

    public void run() {
        Object dlg = null;
        try {
            dlg = xMSF.createInstance(serviceName);
        } catch(com.sun.star.uno.Exception e) {
            throw new StatusException(Status.failed("Couldn't create service"));
        }
        XExecutableDialog execDlg = (XExecutableDialog)UnoRuntime.queryInterface
            (XExecutableDialog.class, dlg);
        execDlg.execute();
    }
}
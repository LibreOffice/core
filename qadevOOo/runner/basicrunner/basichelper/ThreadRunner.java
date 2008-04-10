/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ThreadRunner.java,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package basicrunner.basichelper;

import com.sun.star.beans.PropertyValue;
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
    XMultiServiceFactory myMSF = null;

    public DialogFromFileThread(XMultiServiceFactory xMSF, String sUrl) {
    url = sUrl;
    SOF = SOfficeFactory.getFactory(xMSF);
        myMSF = xMSF;
    }

    public void run() {
    try {
            PropertyValue[] args = new PropertyValue[1];
            args[0] = new PropertyValue();
            args[0].Name = "InteractionHandler";
            args[0].Value = myMSF.createInstance(
                "com.sun.star.comp.uui.UUIInteractionHandler");

        String testUrl= utils.getFullTestURL(url);
        System.out.println("loading "+testUrl);
        XComponent xDoc = SOF.loadDocument(testUrl, args);
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



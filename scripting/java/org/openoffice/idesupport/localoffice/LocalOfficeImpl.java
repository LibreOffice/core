/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.idesupport.localoffice;

import java.net.ConnectException;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.beans.XPropertySet;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import org.openoffice.idesupport.LocalOffice;

/**
 * LocalOfficeImpl represents a connection to the local office.
 *
 * This class is an implementation of LocalOffice ane allows to
 * get access to some scripting framework releated functionality
 * of the locally running office. The office has to be started
 * with options appropriate for establishing local connection.
 */
public final class LocalOfficeImpl
    extends LocalOffice
{
    private final static String     STORAGE_MRG_SINGLETON =
        "/singletons/drafts.com.sun.star.script.framework.storage.theScriptStorageManager";

    private transient String                    mOfficePath;
    private transient XMultiComponentFactory    mComponentFactory;
    private transient XComponentContext         mComponentContext;
    private transient XMultiServiceFactory      mServiceFactory;
    /**
     * Constructor.
     */
    public LocalOfficeImpl()
    {
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
        mOfficePath    = officePath;
        try {
            bootstrap(port);
        } catch (java.lang.Exception ex) {
            throw new ConnectException(ex.getMessage());
        }
    }

    /**
     * Refresh the script storage.
     *
     * @param uri is an identifier of storage has to be refreshed.
     */
    public void refreshStorage(String uri)
    {
        try {
            Object  object = null;
            object      = mComponentContext.getValueByName(STORAGE_MRG_SINGLETON);
            XScriptStorageManager storageMgr;
            storageMgr  = (XScriptStorageManager)UnoRuntime.queryInterface(
                XScriptStorageManager.class, object);
            storageMgr.refreshScriptStorage(uri);
        } catch (java.lang.Exception ex) {
System.out.println("*** LocalOfficeImpl.refreshStorage: FAILED " + ex.getMessage());
System.out.println("*** LocalOfficeImpl.refreshStorage: FAILED " + ex.getClass().getName());
        }
System.out.println("*** LocalOfficeImpl.refreshStorage: DONE");
    }

    /**
     * Closes the connection to the running office.
     */
    public void disconnect()
    {
/*
        if(mComponentFactory != null) {
            XComponent  comp    = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, mComponentFactory);
            comp.dispose();
        }
*/
    }

    /**
     * Boot straps UNO.
     *
     * The office has to be started with following string:
     * "--accept=socket,host=localhost,port=<PORT>;urp;StarOffice.ServiceManager"
     *
     * @param port is a communication port.
     */
    private void bootstrap(int port)
        throws java.lang.Exception
    {
        Object          object;
        mComponentContext   = Bootstrap.createInitialComponentContext(null);
        XUnoUrlResolver urlresolver = UnoUrlResolver.create(mComponentContext);
        object              = urlresolver.resolve(
            "uno:socket,host=localhost,port=" +
            port +
            ";urp;StarOffice.ServiceManager");
        mComponentFactory   = (XMultiComponentFactory)UnoRuntime.queryInterface(
            XMultiComponentFactory.class, object);
        XPropertySet    factoryProps;
        factoryProps        = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, mComponentFactory);
        object              = factoryProps.getPropertyValue("DefaultContext");
        mComponentContext   = (XComponentContext)UnoRuntime.queryInterface(
            XComponentContext.class, object);
    }
}

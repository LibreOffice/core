/*************************************************************************
 *
 *  $RCSfile: LocalOfficeImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:54:33 $
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

package org.openoffice.idesupport.localoffice;

import java.net.ConnectException;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.beans.XPropertySet;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;

import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import org.openoffice.idesupport.LocalOffice;

/**
 * LocalOfficeImpl represents a connection to the local office.
 *
 * This class is an implementation of LocalOffice ane allows to
 * get access to some scripting framework releated functionality
 * of the locally running office. The office has to be started
 * with options appropriate for establishing local connection.
 *
 * @author misha <misha@openoffice.org>
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
     * "-accept=socket,host=localhost,port=<PORT>;urp;StarOffice.ServiceManager"
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

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

package ifc.scripting;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Collection;

import drafts.com.sun.star.script.framework.security.XScriptSecurity;
import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.frame.XModel;
import com.sun.star.container.XNameReplace;
import com.sun.star.util.XChangesBatch;
import com.sun.star.reflection.InvocationTargetException;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;
import util.SOfficeFactory;

public class _XScriptSecurity extends MultiMethodTest {

    public XScriptSecurity oObj = null;
    private XScriptStorageManager storageManager = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void after() throws StatusException {
    }

    public void _checkPermission() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_checkPermission");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runCheckPermissionTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        // set security to always without confirmation dialog and empty path
        // list so other tests can run without dialog popping up
        setSecurity(2, "false", "false", null);

        tRes.tested("checkPermission()", result);
    }

    private boolean runCheckPermissionTest(Parameters testdata) {
        // description of test
        String description = testdata.get("description");

        // document location
        String location = testdata.get("location");

        //security settings
        String runmacro = testdata.get("runmacro");
        String confirm = testdata.get("confirm");
        String warning = testdata.get("warning");
        String pathlist = testdata.get("pathlist");

        //do this test produce a dialog?
        String dialog = testdata.get("dialog");
        //is checkbox to be ticked?
        String checkBoxStr = testdata.get("checkbox");
        //name of button in dialog to press
        String buttonName = testdata.get("buttonName");

        //expected result
        String expected = testdata.get("expected");
        //do we need to check the pathlist?
        String checkpath = testdata.get("checkpath");

        String output = null;

        log.println(description);

        // get the officeBasic setting
        int officeBasic = 0;
        if( runmacro.equals("never") )
        {
            officeBasic = 0;
        }
        else if ( runmacro.equals("pathlist") )
        {
            officeBasic = 1;
        }
        else if ( runmacro.equals("always") )
        {
            officeBasic = 2;
        }

        // should pathlist include doc?
        String secureURLs = null;
        if( pathlist.equals("true") )
        {
            String uri = util.utils.getFullTestURL(location);
            secureURLs = uri.substring(0,  uri.lastIndexOf('/'));
        }

        if ( !setSecurity( officeBasic, confirm, warning, secureURLs ) )
        {
            log.println( "failed to set security" );
            return false;
        }

        if( dialog.equals( "true" ) )
        {
            // is the checkbox to be ticked?
            boolean checkBox = false;
            if( checkBoxStr.equals( "true" ) )
            {
                checkBox = true;
            }
            new SecurityDialogUtil( tParam.getMSF(), buttonName, checkBox ).start();
        }
        // need to set up dialog utils thread first
        int storageId = getStorageId(location);

        try {
            String uri = util.utils.getFullTestURL(location);
            oObj.checkPermission(uri, "execute" );
            output = "true";
        }
        catch (com.sun.star.security.AccessControlException ace) {
            log.println("Couldn't invoke script:" + ace);
            output = "com.sun.star.security.AccessControlException";
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't invoke script:" + iae);
            output = "com.sun.star.lang.IllegalArgumentException";
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Couldn't invoke script:" + re);
            output = "com.sun.star.uno.RuntimeException";
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
        {
            if( checkpath.equals("true") )
            {
                String setPath  = getPathList();
                String expectedPath = "empty";
                if( checkBoxStr.equals( "true" ) )
                {
                    String uri = util.utils.getFullTestURL(location);
                    expectedPath = uri.substring(0,  uri.lastIndexOf('/'));
                }
                log.println("pathlist: expected: " + expectedPath + ", output: " + setPath);
                if( setPath.equals( expectedPath ) )
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            return true;
        }
        else
            return false;
    }

    private String getPathList()
    {
        String result = "";
        try {
        Object oProv = tParam.getMSF().createInstance(
            "com.sun.star.configuration.ConfigurationProvider" );

        XMultiServiceFactory xProv = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, oProv);

        //the path to the security settings in the registry
        PropertyValue aPathArg = new PropertyValue();
        aPathArg.Name="nodepath";
        aPathArg.Value="org.openoffice.Office.Common/Security/Scripting";
        // we don't want to cache the write
        PropertyValue aModeArg = new PropertyValue();
        aModeArg.Name="lazywrite";
        aModeArg.Value=new Boolean(false);

        Object[]  aArgs = new Object[2];
        aArgs[0]=aPathArg;
        aArgs[1]=aModeArg;
        Object oConfigUpdate = xProv.createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess",
            aArgs );
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, oConfigUpdate );

        String[] paths = (String[])xPropertySet.getPropertyValue("SecureURL");
        if (paths == null || paths.length == 0)
            result = "empty";
        else
            result = paths[0];

        } catch (Exception e)
        {
            result = e.getClass().getName() + " getting list of secure URLs";
        }
        return result;
    }

    private boolean setSecurity( int officeBasic, String confirm,
                                String warning, String secureURLs )
    {
        boolean success=false;
        try {
        Object oProv = tParam.getMSF().createInstance(
            "com.sun.star.configuration.ConfigurationProvider" );

        XMultiServiceFactory xProv = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, oProv);

        //the path to the security settings in the registry
        PropertyValue aPathArg = new PropertyValue();
        aPathArg.Name="nodepath";
        aPathArg.Value="org.openoffice.Office.Common/Security/Scripting";
        // we don't want to cache the write
        PropertyValue aModeArg = new PropertyValue();
        aModeArg.Name="lazywrite";
        aModeArg.Value=new Boolean(false);

        Object[]  aArgs = new Object[2];
        aArgs[0]=aPathArg;
        aArgs[1]=aModeArg;
        Object oConfigUpdate = xProv.createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess",
            aArgs );
        XNameReplace xNameReplace = (XNameReplace)UnoRuntime.queryInterface(
                XNameReplace.class, oConfigUpdate );
        XChangesBatch xChangesBatch = (XChangesBatch)UnoRuntime.queryInterface(
                XChangesBatch.class, oConfigUpdate );

        Object[] aSecureURLs;
        if (secureURLs == null) {
            aSecureURLs = new Object[0];
        }
        else {
            aSecureURLs = new Object[1];
            aSecureURLs[0] = secureURLs;
        }
        log.println("setting SecureURL");
        xNameReplace.replaceByName( "SecureURL", aSecureURLs );

        log.println("setting OfficeBasic");
        xNameReplace.replaceByName( "OfficeBasic", new Integer(officeBasic) );

        Boolean bConfirm = null;
        if( ( confirm != null ) && ( confirm.equals("true") ) )
        {
            bConfirm = new Boolean( true );
        }
        else
        {
            bConfirm = new Boolean( false );
        }
        log.println("setting Confirmation");
        xNameReplace.replaceByName( "Confirmation", bConfirm );

        Boolean bWarning = null;
        if( ( warning != null ) && ( warning.equals("true") ) )
        {
            bWarning = new Boolean( true );
        }
        else
        {
            bWarning = new Boolean( false );
        }
        log.println("setting Warning");
        xNameReplace.replaceByName( "Warning", bWarning );

        // and now commit the changes
        xChangesBatch.commitChanges();
        success=true;
        } catch (Exception e) {
            log.println("Error updating security settings: " +
                e.getMessage() );
        }
        return success;
    }

    private int getStorageId(String location) {

        XSimpleFileAccess access = null;
        String uri = util.utils.getFullTestURL(location);

        if (storageManager == null) {
            try {
                XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, tParam.getMSF());

                XComponentContext xContext = (XComponentContext)
                    UnoRuntime.queryInterface(XComponentContext.class,
                    xProp.getPropertyValue("DefaultContext"));

                XInterface ifc = (XInterface)
                    xContext.getValueByName("/singletons/drafts.com.sun.star." +
                    "script.framework.storage.theScriptStorageManager");

                storageManager = (XScriptStorageManager)
                    UnoRuntime.queryInterface(XScriptStorageManager.class, ifc);
            }
            catch( Exception e ) {
                return -1;
            }
        }

        access = getXSimpleFileAccess();
        if (access == null)
            return -1;

        int id = storageManager.createScriptStorageWithURI(access, uri);

        return id;
    }

    private XSimpleFileAccess getXSimpleFileAccess() {
        XSimpleFileAccess access = null;

        try {
            Object fa = tParam.getMSF().createInstance(
                "com.sun.star.ucb.SimpleFileAccess");

            access = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class, fa);
        }
        catch (com.sun.star.uno.Exception e) {
            return null;
        }
        return access;
    }

    private XModel loadDocument(String name) {
        XModel model = null;
        SOfficeFactory factory = SOfficeFactory.getFactory(tParam.getMSF());

        String fullname = util.utils.getFullTestURL(name);

        try {
            Object obj = factory.loadDocument(fullname);
            model = (XModel) UnoRuntime.queryInterface(XModel.class, obj);
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            return null;
        }
        catch (Exception e) {
            return null;
        }

        try {
            Thread.sleep(5000);
        }
        catch (InterruptedException ie) {
        }

        return model;
    }
}

/*************************************************************************
 *
 *  $RCSfile: _XScriptSecurity.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-02-26 09:53:01 $
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

package ifc.script.framework;

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

        tRes.tested("checkPermission()", result);
    }

    private boolean runCheckPermissionTest(Parameters testdata) {
        String description = testdata.get("description");
        String location = testdata.get("location");
        String logicalname = testdata.get("logicalname");
        String runmacro = testdata.get("runmacro");
        String pathlist = testdata.get("pathlist");
        String expected = testdata.get("expected");
        String output = null;

        String officeBasic = null;
        if( runmacro.equals("never") )
        {
            officeBasic = "0";
        }
        else if ( runmacro.equals("pathlist") )
        {
            officeBasic = "1";
        }
        else if ( runmacro.equals("always") )
        {
            officeBasic = "2";
        }

        String secureURLs = null;
        if( pathlist.equals("true") )
        {
            String uri = util.utils.getFullTestURL(location);
            secureURLs = uri.substring(0,  uri.lastIndexOf('/'));
        }
        else if ( pathlist.equals("false") )
        {
            secureURLs = "";
        }
        if ( !setSecurity( officeBasic, secureURLs ) )
        {
            System.err.println( "failed to set security" );
            return false;
        }

        // need to set up dialog utils thread first
        int storageId = getStorageId(location);

        try {
            String uri = util.utils.getFullTestURL(location);
            if ( oObj.checkPermission(uri, "execute" ) )
            {
                output = "success";
            }
            else
            {
                output = "failure";
            }
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Couldn't invoke script:" + re);
            output = "com.sun.star.uno.RuntimeException";
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    private boolean setSecurity( String officeBasic, String secureURLs )
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
        Object[] aSecureURLs = new Object[1];
        aSecureURLs[0]=secureURLs;
        xNameReplace.replaceByName( "SecureURL", aSecureURLs );
        Object[] aOfficeBasic = new Object[1];
        aOfficeBasic[0]=officeBasic;
        xNameReplace.replaceByName( "OfficeBasic", aOfficeBasic );
        xChangesBatch.commitChanges();
        success=true;
        } catch (Exception e) {
            System.err.println("Error updating security settings: " +
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

/*************************************************************************
 *
 *  $RCSfile: _XScriptStorageManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-03-25 11:26:59 $
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

package ifc.script.framework.storage;

import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;
import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;

import java.util.Iterator;
import java.util.Collection;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

public class _XScriptStorageManager extends MultiMethodTest {

    public XScriptStorageManager oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _createScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_createScriptStorage");

        if (c == null) {
            tRes.tested("createScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String output = "";

            log.println(testdata.get("description"));

            XSimpleFileAccess access = getXSimpleFileAccess();

            if (access == null) {
                output = "Couldn't create XSimpleFileAccess";
            }
            else {
                try {
                    int id = oObj.createScriptStorage(access);
                    output = "success";
                }
                catch (com.sun.star.uno.RuntimeException re) {
                    log.println("Exception from createScriptStorage: " + re);
                    output = "com.sun.star.uno.RuntimeException";
                }
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("createScriptStorage()", result);
    }

    public void _createScriptStorageWithURI() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_createScriptStorageWithURI");

        if (c == null) {
            tRes.tested("createScriptStorageWithURI()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            String uri = util.utils.getFullTestURL(location);
            XSimpleFileAccess access = getXSimpleFileAccess();

            try {
                int id = oObj.createScriptStorageWithURI(access, uri);

                XInterface ifc = (XInterface)oObj.getScriptStorage(id);

                if (ifc == null)
                    output = "null";
                else {
                    Object info = UnoRuntime.queryInterface(
                        XScriptInfoAccess.class, ifc);

                    if (info == null)
                        output = "null";
                    else
                        output = "XScriptInfoAccess.class";
                }
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }

        tRes.tested("createScriptStorageWithURI()", result);
    }

    public void _getScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getScriptStorage");

        if (c == null) {
            tRes.tested("getScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            try {
                int storageid = getStorageId(location);

                XInterface ifc = (XInterface)oObj.getScriptStorage(storageid);

                if (ifc == null)
                    output = "null";
                else {
                    Object info = UnoRuntime.queryInterface(
                        XScriptInfoAccess.class, ifc);

                    if (info == null)
                        output = "null";
                    else
                        output = "XScriptInfoAccess.class";
                }
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("getScriptStorage()", result);
    }

    public void _refreshScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_refreshScriptStorage");

        if (c == null) {
            tRes.tested("refreshScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            try {
                String uri = util.utils.getFullTestURL(location);
                log.println("calling refreshScriptStorage with URI: " + uri);
                oObj.refreshScriptStorage(uri);
                output = "success";
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("refreshScriptStorage()", result);
    }

    private int getStorageId(String location) {

        if (location.equals("share"))
            return 0;

        if (location.equals("user"))
            return 1;

        String uri = util.utils.getFullTestURL(location);

        XSimpleFileAccess access = getXSimpleFileAccess();
        if (access == null)
            return -1;

        return oObj.createScriptStorageWithURI(access, uri);
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
}

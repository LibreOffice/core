/*************************************************************************
 *
 *  $RCSfile: _XScriptInfoAccess.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-03-25 17:55:44 $
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

import ifc.script.framework.ScriptingUtils;

import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.beans.XPropertySet;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

import java.util.Collection;
import java.util.Iterator;

public class _XScriptInfoAccess extends MultiMethodTest {

    public XScriptInfoAccess oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getScriptLogicalNames() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getScriptLogicalNames");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetScriptLogicalNamesTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getScriptLogicalNames()", result);
    }

    private boolean runGetScriptLogicalNamesTest(Parameters testdata) {
        String description = testdata.get("description");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

        // try {
            log.println("In _XScriptInfoAccess.getScriptLogicalNames()");
            String[] logicalNames = oObj.getScriptLogicalNames();

            if (logicalNames == null)
                output = "null";
            else if (logicalNames.length == 0)
                output = "empty";
            else {
                for (int i = 0; i < logicalNames.length; i++) {
                    if (logicalNames[i].equals(expected)) {
                        output = logicalNames[i];
                        break;
                    }
                }
            }
        // }
        // catch (com.sun.star.uno.Exception e) {
            // log.println("Caught UNO Exception :" + e);
            // output = "com.sun.star.uno.Exception";
        // }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    public void _getImplementations() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getImplementations");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetImplementationsTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getImplementations()", result);
    }

    private boolean runGetImplementationsTest(Parameters testdata) {
        String description = testdata.get("description");
        String logicalname = testdata.get("logicalname");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

    // performs a basic check to see if 1 match (XScriptInfo) is returned
    // the XScriptInfo object is tested more completely in _XScriptInfo
    // which is drive from ScriptInfo

        try {
            XScriptInfo[] impls = oObj.getImplementations(logicalname);

            // should only be one match
            if (impls == null)
                output = "null";
            else if (impls.length == 0)
                output = "empty";
            else
                output = impls[0].getLogicalName();
        }
        catch (com.sun.star.uno.Exception e) {
            log.println("Caught UNO Exception:" + e);
            output = "com.sun.star.uno.Exception";
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    public void _getAllImplementations() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getAllImplementations");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetAllImplementationsTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getAllImplementations()", result);
    }

    private boolean runGetAllImplementationsTest(Parameters testdata) {
        String description = testdata.get("description");
        String location = testdata.get("location");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

        Object obj = ScriptingUtils.getDefault().getScriptStorage(
            tParam.getMSF(), location);

        XScriptInfoAccess access = (XScriptInfoAccess)
            UnoRuntime.queryInterface(XScriptInfoAccess.class, obj);

        XScriptInfo[] impls = access.getAllImplementations();

        if (impls == null || impls.length == 0) {
            output = "empty";
        }
        else {
            for (int i = 0; i < impls.length - 1; i++)
                output += impls[i].getLogicalName() + ",";
            output += impls[impls.length - 1].getLogicalName();
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }
}

/*************************************************************************
 *
 *  $RCSfile: _XScriptInvocation.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2002-11-20 14:11:23 $
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

import drafts.com.sun.star.script.framework.XScriptInvocation;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XModel;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;
import util.SOfficeFactory;

public class _XScriptInvocation extends MultiMethodTest {

    public XScriptInvocation oObj = null;
    private XComponent oDoc = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
        SOfficeFactory factory = SOfficeFactory.getFactory(tParam.getMSF());

        String name = util.utils.getFullTestURL(
            "xscriptcontext_test_document.sxw");

        try {
            oDoc = factory.loadDocument(name);
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Exception while preloading document: " + iae);
        }
        catch (Exception e) {
            log.println("Exception while preloading document: " + e);
        }

        try {
            Thread.sleep(5000);
        }
        catch (InterruptedException ie) {
        }
    }

    public void after() throws StatusException {
        if (oDoc != null)
            oDoc.dispose();
    }

    public void _invoke() {
        boolean result = true;

        result &= testUserInvoke();
        result &= testUserInvokeWithContext();
        result &= testUserInvokeScriptInJar();
        // if (oDoc != null)
        //    result &= testDocumentInvoke();

        // result &= testInvokeExceptions();

        tRes.tested("invoke()", result);
    }

    private boolean testUserInvoke() {
        log.println("Try invoking a user level script");
        return testInvoke(ScriptingUtils.USER_LOGICAL_NAME, 1);
    }

    private boolean testUserInvokeWithContext() {
        log.println("Try invoking a user level script with an XScriptContext");

        XModel model = (XModel) UnoRuntime.queryInterface(XModel.class, oDoc);
        return testInvoke("script://xscriptcontext.jsuite.test", 1, model);
    }

    private boolean testUserInvokeScriptInJar() {
        log.println("Try invoking a user level script in a Jar file");

        return testInvoke("script://jarscript.jsuite.test", 1);
    }

    private boolean testDocumentInvoke() {
        log.println("Try invoking a user level script");

        String name = util.utils.getFullTestURL(
            ScriptingUtils.XSCRIPTCONTEXT_TEST_DOCUMENT);

        int storageId = ScriptingUtils.getDefault().getScriptStorageId(
            tParam.getMSF(), name);

        return testInvoke("script://xscriptcontext.jsuite.test", storageId);
    }

    private boolean testInvoke(String logicalName, int storageId) {
        return testInvoke(logicalName, storageId, null);
    }

    private boolean testInvoke(String logicalName, int storageId, XModel ctx) {
        HashMap map = new HashMap();
        map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(storageId));
        map.put("SCRIPTING_DOC_URI", "hahaha");
        if (ctx != null)
            map.put("SCRIPTING_DOC_REF", ctx);

        Parameters params = new Parameters(map);
        Object[] args = new Object[0];

        Object[][] result = new Object[1][0];
        result[0] = new Object[0];

        short[][] num = new short[1][0];
        num[0] = new short[0];

        try {
            oObj.invoke(logicalName, params, args, num, result);
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't invoke script:" + iae);
            return false;
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("Couldn't invoke script:" + cce);
            return false;
        }
        catch (com.sun.star.reflection.InvocationTargetException ite) {
            log.println("Couldn't invoke script:" + ite);
            return false;
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Couldn't invoke script:" + re);
            return false;
        }
        return true;
    }
}

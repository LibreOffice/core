/*************************************************************************
 *
 *  $RCSfile: _XScriptNameResolver.java,v $
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
import java.util.Iterator;

import drafts.com.sun.star.script.framework.XScriptNameResolver;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;
import com.sun.star.uno.XInterface;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

public class _XScriptNameResolver extends MultiMethodTest {

    public XScriptNameResolver oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _resolve() {
        boolean result = true;

        result &= testValidURIs();
        result &= testInvalidURIs();
        result &= testNonExistentURIs();

        tRes.tested("resolve()", result);
    }

    private boolean testValidURIs() {
        boolean result = true;

        log.println("Try to resolve a valid document script URI");

        String name = util.utils.getFullTestURL(
                ScriptingUtils.SCRIPT_IN_CLASSFILE_DOC_NAME);

        int storageId = ScriptingUtils.getDefault().getScriptStorageId(
            tParam.getMSF(), name);

        result &= testValidURI(ScriptingUtils.DOC_LOGICAL_NAME, storageId);

        log.println("Try to resolve a valid user script URI");
        result &= testValidURI(ScriptingUtils.USER_LOGICAL_NAME, 1);

        log.println("Try to resolve a valid share script URI");
        result &= testValidURI(ScriptingUtils.SHARE_LOGICAL_NAME, 0);

        return result;
    }

    private boolean testValidURI(String logicalName, int storageId) {
        HashMap map = new HashMap();
        map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(storageId));
        map.put("SCRIPTING_DOC_URI", "hahaha");

        Parameters params = new Parameters(map);
        Object[] args = new Object[] {params};

        try {
            XInterface ifc = (XInterface)
                oObj.resolve(logicalName, args);

            Integer resolvedId = (Integer)
                params.getPropertyValue("SCRIPTING_RESOLVED_STORAGE_ID");

            if (resolvedId == null) {
                log.println("SCRIPTING_RESOLVED_STORAGE_ID not in return args");
                return false;
            }

            if (resolvedId.intValue() != storageId) {
                log.println("Wrong SCRIPTING_RESOLVED_STORAGE_ID returned");
                return false;
            }

            if (ifc == null ||
                UnoRuntime.queryInterface(XScriptInfo.class, ifc) == null)
            {
                log.println("Can't get XScriptInfo from resolved interface");
                return false;
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't resolve URI:" + iae);
            return false;
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("Couldn't resolve URI:" + cce);
            return false;
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Couldn't resolve URI:" + re);
            return false;
        }
        return true;
    }

    private boolean testInvalidURIs() {
        log.println("Try an invalid URI, should throw IllegalArgumentException");
        try {
            HashMap map = new HashMap();
            map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(0));
            map.put("SCRIPTING_DOC_URI", "hahaha");

            Parameters params = new Parameters(map);
            Object[] args = new Object[] {params};

            XInterface ifc = (XInterface)
                oObj.resolve("scrpit://HighlightText.showForm", args);
            log.println("Should not have resolved invalid URI");
            return false;
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Correctly got exception:" + iae);
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("Got wrong exception" + cce);
            return false;
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Got wrong exception:" + re);
            return false;
        }
        return true;
    }

    private boolean testNonExistentURIs() {
        log.println("Try a valid but non-existent URI");
        try {
            HashMap map = new HashMap();
            map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(0));
            map.put("SCRIPTING_DOC_URI", "hahaha");

            Parameters params = new Parameters(map);
            Object[] args = new Object[] {params};

            XInterface ifc = (XInterface)
                oObj.resolve("script://Non.Existent", args);

            if (ifc != null &&
                UnoRuntime.queryInterface(XScriptInfo.class, ifc) != null)
            {
                log.println("Should not have resolved non-existent URI");
                return false;
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't resolve name:" + iae);
            return false;
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("Couldn't resolve name:" + cce);
            return false;
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Got wrong exception:" + re);
            return false;
        }
        return true;
    }
}

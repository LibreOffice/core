/*************************************************************************
 *
 *  $RCSfile: _XScriptStorageManager.java,v $
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

package ifc.script.framework.storage;

import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;
import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;

import ifc.script.framework.ScriptingUtils;

public class _XScriptStorageManager extends MultiMethodTest {

    public XScriptStorageManager oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _createScriptStorage() {
        boolean result = true;

        XSimpleFileAccess access =
            ScriptingUtils.getDefault().getXSimpleFileAccess(tParam.getMSF());

        if (access == null) {
            result = false;
        }
        else {
            try {
                int id = oObj.createScriptStorage(access);
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Exception from createScriptStorage: " + re);
                result = false;
            }
        }

        tRes.tested("createScriptStorage()", result);
    }

    public void _createScriptStorageWithURI() {
        boolean result = true;

        XSimpleFileAccess access =
            ScriptingUtils.getDefault().getXSimpleFileAccess(tParam.getMSF());

        String name = util.utils.getFullTestURL(
            ScriptingUtils.SCRIPT_IN_CLASSFILE_DOC_NAME);
        int id = oObj.createScriptStorageWithURI(access, name);

        XInterface ifc = (XInterface)oObj.getScriptStorage(id);
        XScriptInfoAccess info = (XScriptInfoAccess)
            UnoRuntime.queryInterface(XScriptInfoAccess.class, ifc);

        if (info == null) {
            log.println("Couldn't get XScriptInfoAccess:");
            tRes.tested("createScriptStorageWithURI()", false);
        }

        try {
            String[] names = info.getScriptLogicalNames();

            if (names == null || names.length == 0) {
                log.println("No logical names found");
                result = false;
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't get logical names:" + iae.getMessage());
            result = false;
        }

        tRes.tested("createScriptStorageWithURI()", result);
    }

    public void _getScriptStorage() {
        boolean result = true;

        log.println("Try getScriptStorage for share");
        try {
            XInterface ifc = (XInterface)oObj.getScriptStorage(0);

            if (ifc == null) {
                log.println("getScriptStorage returned null");
                result = false;
            }
            else {
                XScriptInfoAccess info = (XScriptInfoAccess)
                    UnoRuntime.queryInterface(XScriptInfoAccess.class, ifc);

                if (info == null) {
                    log.println("Couldn't get XScriptInfoAccess from storage");
                    result = false;
                }
            }
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Caught unexpected RuntimeException: " + re);
            result = false;
        }

        log.println("Try getScriptStorage for user");
        try {
            XInterface ifc = (XInterface)oObj.getScriptStorage(1);
            if (ifc == null) {
                log.println("getScriptStorage returned null");
                result = false;
            }
            else {
                XScriptInfoAccess info = (XScriptInfoAccess)
                    UnoRuntime.queryInterface(XScriptInfoAccess.class, ifc);

                if (info == null) {
                    log.println("Couldn't get XScriptInfoAccess from storage");
                    result = false;
                }
                try {
                    String[] names = info.getScriptLogicalNames();

                    if (names == null) {
                        log.println("No logical names found");
                        result = false;
                    }
                }
                catch (com.sun.star.lang.IllegalArgumentException iae) {
                    log.println("Error get logical names:" + iae.getMessage());
                    result = false;
                }
            }
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Caught unexpected RuntimeException: " + re);
            result = false;
        }

        tRes.tested("getScriptStorage()", result);
    }

    public void _refreshScriptStorage() {
        boolean result = true;

        log.println("Try to refresh a URI for non-existent script storage");
        try {
            oObj.refreshScriptStorage("file:///does/not/exist");
            result = false;
        }
        catch (com.sun.star.uno.RuntimeException re) {
        }

        log.println("Try to refresh a valid document URI");
        try {
            XSimpleFileAccess access =
                ScriptingUtils.getDefault().getXSimpleFileAccess(
                    tParam.getMSF());

            String name = util.utils.getFullTestURL(
                ScriptingUtils.SCRIPT_IN_CLASSFILE_DOC_NAME);

            int id = oObj.createScriptStorageWithURI(access, name);

            oObj.refreshScriptStorage(name);
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Caught unexpected RuntimeException: " + re);
            result = false;
        }

        tRes.tested("refreshScriptStorage()", result);
    }
}

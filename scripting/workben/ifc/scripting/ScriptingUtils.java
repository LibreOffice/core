/*************************************************************************
 *
 *  $RCSfile: ScriptingUtils.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2002-11-20 14:11:22 $
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

import java.io.File;
import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;

public class ScriptingUtils {
    private XScriptStorageManager storageManager;
    private static ScriptingUtils utils;

    public static final String USER_LOGICAL_NAME =
        "script://user.jsuite.test";
    public static final String SHARE_LOGICAL_NAME =
        "script://share.jsuite.test";
    public static final String DOC_LOGICAL_NAME =
        "script://doc.jsuite.testMethod";

    public static final String SCRIPT_IN_CLASSFILE_DOC_NAME =
        "script_in_class_file.sxw";

    public static final String SCRIPT_IN_JARFILE_DOC_NAME =
        "script_in_jar_file.sxw";

    public static final String DOC_WITH_ONE_SCRIPT =
        "doc_with_one_script.sxw";

    public static final String DOC_WITH_TWO_SCRIPTS =
        "doc_with_two_scripts.sxw";

    public static final String XSCRIPTCONTEXT_TEST_DOCUMENT =
        "xscriptcontext_test_document.sxw";

    private ScriptingUtils() {
    }

    public static ScriptingUtils getDefault() {
        if (utils == null) {
            synchronized (ScriptingUtils.class) {
                if (utils == null)
                    utils = new ScriptingUtils();
            }
        }
        return utils;
    }

    public static void cleanUserDir() {
    }

    public static void cleanShareDir() {
    }

    public int getScriptStorageId(XMultiServiceFactory xMSF, String uri) {

        XSimpleFileAccess access = null;

        if (storageManager == null) {
            try {
                XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xMSF);

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

        access = getXSimpleFileAccess(xMSF);
        if (access == null)
            return -1;

        int id = storageManager.createScriptStorageWithURI(access, uri);

        return id;
    }

    public XSimpleFileAccess getXSimpleFileAccess(XMultiServiceFactory xMSF) {
        XSimpleFileAccess access = null;

        try {
            Object fa =
                xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");

            access = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class, fa);
        }
        catch (com.sun.star.uno.Exception e) {
            return null;
        }
        return access;
    }
}

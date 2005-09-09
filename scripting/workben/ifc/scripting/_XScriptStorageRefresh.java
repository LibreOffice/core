/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XScriptStorageRefresh.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:43:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.script.framework.storage;

import drafts.com.sun.star.script.framework.storage.XScriptStorageRefresh;

import java.util.Collection;
import java.util.Iterator;

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

public class _XScriptStorageRefresh extends MultiMethodTest {

    public XScriptStorageRefresh oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _refresh() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_refresh");

        if (c == null) {
            tRes.tested("refresh()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String output = "";

            log.println(testdata.get("description"));

            try {
                oObj.refresh();
                output = "success";
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("refresh()", result);
    }
}

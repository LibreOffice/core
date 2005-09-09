/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SvxUnoNumberingRules.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:37:22 $
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
package mod._svx;

import com.sun.star.beans.Property;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DrawTools;


public class SvxUnoNumberingRules extends TestCase {
    XComponent xDrawDoc;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a drawdoc");
        xDrawDoc = DrawTools.createDrawDoc(
                           (XMultiServiceFactory) tParam.getMSF());
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XMultiServiceFactory docMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                              XMultiServiceFactory.class,
                                              xDrawDoc);
        XInterface oObj = null;

        try {
            oObj = (XInterface) docMSF.createInstance(
                           "com.sun.star.text.NumberingRules");
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("Implementationname: " + util.utils.getImplName(oObj));

        Object NewRules=null;

        try{
        XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, oObj);
        NewRules = xIA.getByIndex(2);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get new Rules for XIndexReplace");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get new Rules for XIndexReplace");
        }

        tEnv.addObjRelation("INSTANCE1",NewRules);

        return tEnv;
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");
        util.DesktopTools.closeDoc(xDrawDoc);
    }
}
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LocalSingleStratum.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:53:01 $
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
package mod._cfgmgr2;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.util.XURLTransformer;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;


public class LocalSingleStratum extends TestCase {
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        String AdminURL = "";

        try {
            Object[] args = new Object[1];
            XStringSubstitution sts = createStringSubstitution(
                                              (XMultiServiceFactory) tParam.getMSF());
            String userURL = sts.getSubstituteVariableValue("$(user)");
            args[0] = userURL + "/registry";
            AdminURL = sts.getSubstituteVariableValue("$(inst)") +
                       "/share/registry";
            log.println("Using: " + args[0]);
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstanceWithArguments(
                           "com.sun.star.comp.configuration.backend.LocalSingleStratum",
                           args);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("Implementation name: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("NoAdmin", AdminURL);

        return tEnv;
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;

        try {
            xPathSubst = xMSF.createInstance(
                                 "com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        if (xPathSubst != null) {
            return (XStringSubstitution) UnoRuntime.queryInterface(
                           XStringSubstitution.class, xPathSubst);
        } else {
            return null;
        }
    }
}
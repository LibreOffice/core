/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoControlContainerModel.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:35:38 $
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
package mod._toolkit;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.utils;


public class UnoControlContainerModel extends TestCase {
    public TestEnvironment createTestEnvironment(TestParameters param,
                                                 PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) param.getMSF()).createInstance(
                           "com.sun.star.awt.UnoControlContainerModel");
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create " +
                                      "UnoControlContainerModel", e);
        }

        log.println(
                "creating a new environment for UnoControlContainer object");

        TestEnvironment tEnv = new TestEnvironment(oObj);
        tEnv.addObjRelation("OBJNAME",
                            "stardiv.vcl.controlmodel.ControlContainer");
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    }
}
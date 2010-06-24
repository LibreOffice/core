/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._dbaccess;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sdb.ErrorMessageDialog</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 *  <li> <code>com::sun::star::sdb::ErrorMessageDialog</code></li>
 *  <li> <code>com::sun::star::ui::dialogs::XExecutableDialog</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.sdb.ErrorMessageDialog
 * @see com.sun.star.ui.dialogs.XExecutableDialog
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XInitialization
 * @see ifc.sdb._ErrorMessageDialog
 * @see ifc.ui.dialogs._XExecutableDialog
 * @see ifc.beans._XPropertySet
 */
public class OSQLMessageDialog extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     *
     * Creates an instance of the service
     * <code>com.sun.star.sdb.ErrorMessageDialog</code>.
     *     Object relations created :
     * <ul>
     *  <li> <code>'ERR1', 'ERR2'</code> for
     *      {@link ifc.sdb._ErrorMessageDialog} </li>
     * </ul>
     */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        Object err1 = null;
        Object err2 = null;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                            "com.sun.star.sdb.ErrorMessageDialog" );

            err1 = new com.sun.star.sdbc.SQLException("err1");
            err2 = new com.sun.star.sdbc.SQLException("err2");
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

        oObj = (XInterface) oInterface;

        // create XWindow for Object relations...
        com.sun.star.awt.XToolkit xToolkit = null;

        try{
            xToolkit = (com.sun.star.awt.XToolkit)UnoRuntime.queryInterface(
                        com.sun.star.awt.XToolkit.class,
            ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.awt.Toolkit") );
        } catch (com.sun.star.uno.Exception e){
            log.println("could not create instacne of 'com.sun.star.awt.Toolkit'" );
            throw new StatusException("could not create instacne of 'com.sun.star.awt.Toolkit'", e) ;
        }

        // Describe the properties of the container window.
        com.sun.star.awt.WindowDescriptor aDescriptor =
        new com.sun.star.awt.WindowDescriptor();

        aDescriptor.Type              = com.sun.star.awt.WindowClass.TOP ;
        aDescriptor.WindowServiceName = "window" ;
        aDescriptor.ParentIndex       = -1;
        aDescriptor.Parent            = null;
        aDescriptor.Bounds            = new com.sun.star.awt.Rectangle(0,0,0,0);

        aDescriptor.WindowAttributes  =
        com.sun.star.awt.WindowAttribute.BORDER    |
        com.sun.star.awt.WindowAttribute.MOVEABLE  |
        com.sun.star.awt.WindowAttribute.SIZEABLE  |
        com.sun.star.awt.WindowAttribute.CLOSEABLE ;

        com.sun.star.awt.XWindowPeer xPeer = null;
        try{

            xPeer = xToolkit.createWindow(aDescriptor) ;

        } catch (com.sun.star.lang.IllegalArgumentException e){
            log.println("could not create window" );
            throw new StatusException("could not create window", e) ;
        }

        com.sun.star.awt.XWindow xWindow = (com.sun.star.awt.XWindow)UnoRuntime.queryInterface (
        com.sun.star.awt.XWindow .class, xPeer);

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println("add ObjectRelations err1 and err2 for 'ErrorMessageDialog'");
        tEnv.addObjRelation("ERR1",err1);
        tEnv.addObjRelation("ERR2",err2);
        tEnv.addObjRelation("ERR_XWindow", xWindow);

        return tEnv;
    } // finish method getTestEnvironment

} // finish class OSQLMessageDialog


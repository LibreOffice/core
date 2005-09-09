/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXModule.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:48:57 $
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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.GlobalSettings</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::view::XPrintSettingsSupplier</code></li>
 *  <li> <code>com::sun::star::view::XViewSettingsSupplier</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.view.XPrintSettingsSupplier
 * @see com.sun.star.view.XViewSettingsSupplier
 * @see ifc.view._XPrintSettingsSupplier
 * @see ifc.view._XViewSettingsSupplier
 */
public class SwXModule extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.GlobalSettings</code>.
    */
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.text.GlobalSettings" );
        } catch ( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException("Service wasn't created",
                new NullPointerException()) ;
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

} // finish class SwXModule


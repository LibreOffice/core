/*************************************************************************
 *
 *  $RCSfile: SpellChecker.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-10-06 13:33:05 $
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

package mod._lnn;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.linguistic2.XLinguServiceManager;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.linguistic2.SpellChecker</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::linguistic2::XSupportedLocales</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::linguistic2::XSpellChecker</code></li>
 *  <li> <code>com::sun::star::lang::XServiceDisplayName</code></li>
 *  <li> <code>com::sun::star::linguistic2::XLinguServiceEventBroadcaster</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.linguistic2.XSupportedLocales
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.linguistic2.XSpellChecker
 * @see com.sun.star.lang.XServiceDisplayName
 * @see com.sun.star.linguistic2.XLinguServiceEventBroadcaster
 * @see ifc.lang._XInitialization
 * @see ifc.lang._XComponent
 * @see ifc.linguistic2._XSupportedLocales
 * @see ifc.lang._XServiceInfo
 * @see ifc.linguistic2._XSpellChecker
 * @see ifc.lang._XServiceDisplayName
 * @see ifc.linguistic2._XLinguServiceEventBroadcaster
 */
public class SpellChecker extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.linguistic2.SpellChecker</code>.
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;
        Object LinguServiceManager = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.lingu2.SpellChecker");
            LinguServiceManager = xMSF.createInstance
                ("com.sun.star.linguistic2.LinguServiceManager");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        XLinguServiceManager xLinguServiceManager = (XLinguServiceManager) UnoRuntime.queryInterface(XLinguServiceManager.class, LinguServiceManager);
        tEnv.addObjRelation("AlternativeChecker",xLinguServiceManager.getSpellChecker());

        return tEnv;
    }

}    // finish class TestCase


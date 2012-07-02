/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._fop;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.fop.FolderPicker</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ui::XExecutableDialog</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::ui::XFolderPickerNotifier</code></li>
*  <li> <code>com::sun::star::ui::XFolderPicker</code></li>
*  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.ui.XExecutableDialog
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.ui.XFolderPickerNotifier
* @see com.sun.star.ui.XFolderPicker
* @see com.sun.star.lang.XTypeProvider
* @see com.sun.star.ui.FolderPicker
* @see ifc.ui.dialogs._XExecutableDialog
* @see ifc.lang._XComponent
* @see ifc.lang._XServiceInfo
* @see ifc.ui._XFolderPickerNotifier
* @see ifc.ui.dialogs._XFolderPicker
* @see ifc.lang._XTypeProvider
*/
public class FolderPicker extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.fop.FolderPicker</code>. <p>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();


        try {
            oInterface = xMSF.createInstance
                ( "com.sun.star.ui.dialogs.FolderPicker" );

            // adding one child container
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class BreakIterator


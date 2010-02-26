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

package mod._fps;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ui.FilePicker</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::ui::XFilePicker</code></li>
*  <li> <code>com::sun::star::lang::XInitialization</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
*  <li> <code>com::sun::star::ui::XExecutableDialog</code></li>
*  <li> <code>com::sun::star::ui::XFilePickerControlAccess</code></li>
*  <li> <code>com::sun::star::ui::XFilterManager</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::ui::XFilePickerNotifier</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.ui.XFilePicker
* @see com.sun.star.lang.XInitialization
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.lang.XTypeProvider
* @see com.sun.star.ui.XExecutableDialog
* @see com.sun.star.ui.XFilePickerControlAccess
* @see com.sun.star.ui.XFilterManager
* @see com.sun.star.lang.XComponent
* @see com.sun.star.ui.XFilePickerNotifier
* @see com.sun.star.ui.XFilePicker
* @see ifc.ui._XFilePicker
* @see ifc.lang._XInitialization
* @see ifc.lang._XServiceInfo
* @see ifc.lang._XTypeProvider
* @see ifc.ui._XExecutableDialog
* @see ifc.ui._XFilePickerControlAccess
* @see ifc.ui._XFilterManager
* @see ifc.lang._XComponent
* @see ifc.ui._XFilePickerNotifier
*/
public class FilePicker extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.ui.FilePicker</code>. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'XInitialization.args'</code> for
    *      {@link ifc.lang._XInitialization} : argument
    *    for creating dialog from teamplate. </li>
    *  <li> <code>'XFilePickerControlAccess.ControlID'</code> for
    *      {@link ifc.ui._XFilePickerControlAccess} : here
    *    it is identifier of check box.</li>
    *  <li> <code>'XFilePickerControlAccess.ControlValue'</code> for
    *      {@link ifc.ui._XFilePickerControlAccess} :
    *      The value for control to set (since the initial value
    *      of the control is empty) .</li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();

        Object[] iniArgs = new Object[] { new Short
            (com.sun.star.ui.dialogs.TemplateDescription.FILEOPEN_READONLY_VERSION)};
        short ctrlId =
            com.sun.star.ui.dialogs.ExtendedFilePickerElementIds.CHECKBOX_READONLY;
        try {
            oInterface = xMSF.createInstance
                ( "com.sun.star.ui.dialogs.FilePicker" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XInitialization
        tEnv.addObjRelation("XInitialization.args", iniArgs) ;

        // adding relation for XFilePickerControlAccess
        tEnv.addObjRelation("XFilePickerControlAccess.ControlID",
            new Short(ctrlId)) ;
        tEnv.addObjRelation("XFilePickerControlAccess.ControlValue",
            Boolean.FALSE) ;

        return tEnv;
    } // finish method getTestEnvironment

}


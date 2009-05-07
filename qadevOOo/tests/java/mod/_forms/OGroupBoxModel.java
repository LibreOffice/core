/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OGroupBoxModel.java,v $
 * $Revision: 1.6 $
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
package mod._forms;

import java.io.PrintWriter;

import lib.TestEnvironment;
import lib.TestParameters;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.form.component.GroupBox</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::awt::UnoControlGroupBoxModel</code></li>
 *  <li> <code>com::sun::star::io::XPersistObject</code></li>
 *  <li> <code>com::sun::star::container::XChild</code></li>
 *  <li> <code>com::sun::star::form::FormControlModel</code></li>
 *  <li> <code>com::sun::star::form::FormComponent</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyState</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 * </ul>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.awt.UnoControlGroupBoxModel
 * @see com.sun.star.io.XPersistObject
 * @see com.sun.star.container.XChild
 * @see com.sun.star.form.FormControlModel
 * @see com.sun.star.form.FormComponent
 * @see com.sun.star.beans.XPropertyAccess
 * @see com.sun.star.beans.XPropertyContainer
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.beans.XMultiPropertySet
 * @see com.sun.star.lang.XComponent
 * @see ifc.awt._UnoControlGroupBoxModel
 * @see ifc.io._XPersistObject
 * @see ifc.container._XChild
 * @see ifc.form._FormControlModel
 * @see ifc.form._FormComponent
 * @see ifc.beans._XPropertySet
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XPropertyState
 * @see ifc.container._XNamed
 * @see ifc.beans._XMultiPropertySet
 * @see ifc.lang._XComponent
 */
public class OGroupBoxModel extends GenericModelTest {

    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    <super.m_ChangePropertyName</CODE> = "Text";
     *    super.m_kindOfControl</CODE>="GroupBox";
     *    super.m_ObjectName</CODE> = "GroupBox";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */

    protected void initialize(TestParameters tParam, PrintWriter log) {

        super.initialize(tParam, log);

        super.m_kindOfControl="GroupBox";

        super.m_ObjectName = "stardiv.one.form.component.GroupBox";

        super.m_LCShape_Type = "FixedText";

        super.m_ChangePropertyName = "Text";
    }
    /**
     * calls <CODE>cleanup()</CODE> from it's super class
     * @param tParam the test parameter
     * @param log the log writer
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        super.cleanup(tParam, log);
    }


    /**
     * calls <CODE>createTestEnvironment()</CODE> from it's super class
     * @param Param the test parameter
     * @param log the log writer
     * @return lib.TestEnvironment
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {
        return super.createTestEnvironment(Param, log);
    }

} // finish class OGroupBoxModel

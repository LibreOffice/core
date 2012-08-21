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
package mod._forms;

import java.io.PrintWriter;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.form.component.ImageButton</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
*  <li> <code>com::sun::star::awt::UnoControlImageControlModel</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::form::XImageProducerSupplier</code></li>
*  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::form::component::ImageButton</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.container.XChild
* @see com.sun.star.awt.UnoControlImageControlModel
* @see com.sun.star.form
* @see com.sun.star.form.FormComponent
* @see com.sun.star.form.XImageProducerSupplier
* @see com.sun.star.beans.XPropertyAccess
* @see com.sun.star.beans.XPropertyContainer
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.form.component.ImageButton
* @see com.sun.star.container.XNamed
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.lang.XComponent
* @see ifc.io._XPersistObject
* @see ifc.container._XChild
* @see ifc.awt._UnoControlImageControlModel
* @see ifc.form._FormControlModel
* @see ifc.form._FormComponent
* @see ifc.form._XImageProducerSupplier
* @see ifc.beans._XPropertySet
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XPropertyState
* @see ifc.form.component._ImageButton
* @see ifc.container._XNamed
* @see ifc.beans._XMultiPropertySet
* @see ifc.lang._XComponent
*/

public class OImageButtonModel extends GenericModelTest {

    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    super.m_kindOfControl="ImageButton";
     *    super.m_ObjectName = "stardiv.one.form.component.ImageButton";
     *    super.m_LCShape_Type = "ImageButton";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */

    protected void initialize(TestParameters tParam, PrintWriter log) {

        super.initialize(tParam, log);

        super.m_kindOfControl="ImageButton";

        super.m_ObjectName = "stardiv.one.form.component.ImageButton";

        super.m_LCShape_Type = "ImageButton";

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

} // finish class OImageButtonModel

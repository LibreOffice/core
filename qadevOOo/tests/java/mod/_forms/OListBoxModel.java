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

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XBoundComponent;
import com.sun.star.form.XLoadable;


import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import ifc.form._XUpdateBroadcaster.UpdateChecker;
import java.io.PrintWriter;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.component.DatabaseListBox</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::awt::UnoControlListBoxModel</code></li>
*  <li> <code>com::sun::star::form::XReset</code></li>
*  <li> <code>com::sun::star::form::XBoundComponent</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::form::component::ListBox</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::form::XUpdateBroadcaster</code></li>
*  <li> <code>com::sun::star::form::DataAwareControlModel</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XEventListener</code></li>
*  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::form::XLoadListener</code></li>
*  <li> <code>com::sun::star::form::component::DatabaseListBox</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.awt.UnoControlListBoxModel
* @see com.sun.star.form.XReset
* @see com.sun.star.form.XBoundComponent
* @see com.sun.star.form.FormComponent
* @see com.sun.star.form.component.ListBox
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.form.XUpdateBroadcaster
* @see com.sun.star.form.DataAwareControlModel
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.form
* @see com.sun.star.container.XNamed
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XEventListener
* @see com.sun.star.beans.XPropertyAccess
* @see com.sun.star.beans.XPropertyContainer
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.form.XLoadListener
* @see com.sun.star.form.component.DatabaseListBox
* @see com.sun.star.container.XChild
* @see ifc.io._XPersistObject
* @see ifc.awt._UnoControlListBoxModel
* @see ifc.form._XReset
* @see ifc.form._XBoundComponent
* @see ifc.form._FormComponent
* @see ifc.form.component._ListBox
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XMultiPropertySet
* @see ifc.form._XUpdateBroadcaster
* @see ifc.form._DataAwareControlModel
* @see ifc.beans._XPropertyState
* @see ifc.form._FormControlModel
* @see ifc.container._XNamed
* @see ifc.lang._XComponent
* @see ifc.lang._XEventListener
* @see ifc.beans._XPropertySet
* @see ifc.form._XLoadListener
* @see ifc.form.component._DatabaseListBox
* @see ifc.container._XChild
*/
public class OListBoxModel extends GenericModelTest {
    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    super.m_ChangePropertyName = "Date";
     *    super.m_kindOfControl="DateField";
     *    super.m_ObjectName = "stardiv.one.form.component.DateField";
     *    NamedValue DataField = new NamedValue();
     *    DataField.Name = "DataField";
     *    DataField.Value = DBTools.TST_DATE_F;
     *    super.m_propertiesToSet.add(DataField);
     *
     *    NamedValue ListSource = new NamedValue();
     *    ListSource.Name = "ListSource";
     *    ListSource.Value = new String[] {
     *           "OListBoxModel1", "OListBoxModel2", "OListBoxModel3"};
     *    super.m_propertiesToSet.add(ListSource);
     *    super.m_LCShape_Type = "FixedText";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {

        super.initialize(tParam, log);

        super.m_ChangePropertyName = "SelectedItems";

        super.m_kindOfControl="ListBox";

        super.m_ObjectName = "stardiv.one.form.component.ListBox";

        NamedValue DataField = new NamedValue();
        DataField.Name = "DataField";
        DataField.Value = DBTools.TST_STRING_F;
        super.m_propertiesToSet.add(DataField);

        NamedValue ListSource = new NamedValue();
        ListSource.Name = "ListSource";
        ListSource.Value = new String[] {
                "OListBoxModel1", "OListBoxModel2", "OListBoxModel3"};
        super.m_propertiesToSet.add(ListSource);

        super.m_LCShape_Type = "FixedText";

    }

    /**
     * calls <CODE>createTestEnvironment()</CODE> from its super class
     * This test uses not the generic implementation of <CODE>checker()</CODE> of its
     * super class. This tests uses its own implementation of <CODE>checker()</CODE>
     * to test <CODE>com::sun::star::form::XUpdateBroadcaster</CODE>
     * @param Param the test parameter
     * @param log the log writer
     * @return lib.TestEnvironment
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) throws Exception {
        TestEnvironment tEnv = super.createTestEnvironment(Param, log);

        tEnv.addObjRelation("XUpdateBroadcaster.Checker",
                            new Checker(m_XFormLoader, m_XPS, m_XCtrl, m_ChangePropertyName));
        return tEnv;
    }

    private static class Checker implements UpdateChecker {
            private short lastItem = (short) 0;
            private XLoadable formLoaderF = null;
            private XPropertySet ps = null;
            private XInterface ctrl = null;
            private String ChangePropertyName = null;

            public Checker(XLoadable xl, XPropertySet ps, XInterface ctrl, String ChangePropertyName) {
                formLoaderF = xl;
                this.ps = ps;
                this.ctrl = ctrl;
                this.ChangePropertyName=ChangePropertyName;
            }

            public void update() throws com.sun.star.uno.Exception {
                if (!formLoaderF.isLoaded()) {
                    formLoaderF.load();
                }
                lastItem = (short) (1 - lastItem);
                ps.setPropertyValue(ChangePropertyName, new short[] { lastItem });
            }

            public void commit() throws SQLException {
                XBoundComponent bound = UnoRuntime.queryInterface(
                                                XBoundComponent.class, ctrl);
                XResultSetUpdate update = UnoRuntime.queryInterface(
                                                  XResultSetUpdate.class,
                                                  formLoaderF);

                bound.commit();
                update.updateRow();
            }

            public boolean wasCommited() throws com.sun.star.uno.Exception {
                formLoaderF.reload();

                short[] getS = (short[]) ps.getPropertyValue(ChangePropertyName);

                return (getS.length > 0) && (lastItem == getS[0]);
            }
        }

} // finish class OListBoxModel

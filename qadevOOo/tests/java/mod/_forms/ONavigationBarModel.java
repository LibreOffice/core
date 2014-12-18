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

public class ONavigationBarModel extends GenericModelTest {

    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    super.m_kindOfControl="NavigationToolBar";
     *    super.m_ObjectName = "om.sun.star.form.component.NavigationToolBar";
     *    super.m_LCShape_Type = "NavigationToolBar";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */

    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) {

        super.initialize(tParam, log);

        super.m_kindOfControl="NavigationToolBar";

        super.m_ObjectName = "com.sun.star.form.component.NavigationToolBar";

        super.m_LCShape_Type = "NavigationToolBar";

    }

    /**
     * calls <CODE>createTestEnvironment()</CODE> from it's super class
     * @param Param the test parameter
     * @param log the log writer
     * @return lib.TestEnvironment
     */
    @Override
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {
        return super.createTestEnvironment(Param, log);
    }

}    // finish class ONavigationBarModel


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

package ifc.form.component;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.form.component.DateField</code>
* service properties :
* <ul>
*  <li><code> DefaultDate</code></li>
* </ul>
* @see com.sun.star.form.component.DateField
*/
public class _DateField extends MultiPropertyTest {

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _DefaultDate() {
        testProperty("DefaultDate", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? Integer.valueOf(20010507) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }
}


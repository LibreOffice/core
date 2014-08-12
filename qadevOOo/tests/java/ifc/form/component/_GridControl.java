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
* Testing <code>com.sun.star.form.component.GridControl</code>
* service properties :
* <ul>
*  <li><code> Border</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> FontDescriptor</code></li>
*  <li><code> RowHeight</code></li>
*  <li><code> Tabstop</code></li>
*  <li><code> TextColor</code></li>
* </ul>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.form.component.GridControl
*/
public class _GridControl extends MultiPropertyTest {

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _RowHeight() {
        testProperty("RowHeight", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? Integer.valueOf(11) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? Boolean.TRUE :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? Integer.valueOf(342) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _BorderColor() {
        testProperty("BorderColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? Integer.valueOf(342) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }
}  // finish class _GridControl



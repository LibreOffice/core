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

package ifc.awt;

import lib.MultiPropertyTest;

/*
* Testing <code>com.sun.star.awt.UnoControlButtonModel</code>
* service properties :
* <ul>
*  <li><code> BackgroundColor</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> Border</code></li>
*  <li><code> DefaultControl</code></li>
*  <li><code> Printable</code></li>
*  <li><code> Text</code></li>
*  <li><code> HelpText</code></li>
*  <li><code> HelpURL</code></li>
* </ul>
* After test completion object environment has to be recreated.
* @see com.sun.star.awt.UnoControlContainerModel
*/
public class _UnoControlContainerModel extends MultiPropertyTest {

    /**
    * Redefined method returns value, that differs from property value.
    */
    protected PropertyTester ColorTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (util.ValueComparer.equalValue(oldValue, new Integer(17)))
                return new Integer(25);
            else
                return new Integer(17);
        }
    };

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _BackgroundColor() {
        log.println("Testing with custom Property tester") ;
        testProperty("BackgroundColor", ColorTester) ;
    }

}


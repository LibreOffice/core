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

package ifc.presentation;

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.presentation.XPresentation;

/**
* Testing <code>com.sun.star.presentation.Presentation</code>
* service properties :
* <ul>
*  <li><code> AllowAnimations</code></li>
*  <li><code> CustomShow</code></li>
*  <li><code> FirstPage</code></li>
*  <li><code> IsAlwaysOnTop</code></li>
*  <li><code> IsAutomatic</code></li>
*  <li><code> IsEndless</code></li>
*  <li><code> IsFullScreen</code></li>
*  <li><code> IsLivePresentation</code></li>
*  <li><code> IsMouseVisible</code></li>
*  <li><code> Pause</code></li>
*  <li><code> StartWithNavigator</code></li>
*  <li><code> UsePen</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Presentation'</code> (of type <code>XPresentation</code>):
*   for test of property <code>IsLivePresentation</code>
*   presentation start needed </li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.presentation.Presentation
* @see com.sun.star.presentation.XPresentation
*/
public class _Presentation extends MultiPropertyTest {

    public void _IsLivePresentation() {
        XPresentation aPresentation = (XPresentation)
            tEnv.getObjRelation("Presentation");
        if (aPresentation == null) throw new StatusException(Status.failed
            ("Relation 'Presentation' not found"));

        aPresentation.start();
        testProperty("IsLivePresentation");
    }

    protected PropertyTester CustomShowTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if ( ((String)oldValue).equals("SecondPresentation") ) {
                return new String("FirstPresentation");
            } else {
                return new String("SecondPresentation");
            }
        }
    };

    public void _CustomShow() {
        testProperty("CustomShow", CustomShowTester);
    }
}  // finish class _Presentation



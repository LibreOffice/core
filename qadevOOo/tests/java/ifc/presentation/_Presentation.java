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
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if ( ((String)oldValue).equals("SecondPresentation") ) {
                return "FirstPresentation";
            } else {
                return "SecondPresentation";
            }
        }
    };

    public void _CustomShow() {
        testProperty("CustomShow", CustomShowTester);
    }
}  // finish class _Presentation



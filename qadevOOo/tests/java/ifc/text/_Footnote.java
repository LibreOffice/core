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

package ifc.text;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.text.Footnote</code>
* service properties :
* <ul>
*  <li><code> ReferenceId</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.Footnote
*/
public class _Footnote extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _ReferenceId() {
        final Short val1 = Short.valueOf( (short) 1);
        final Short val2 = Short.valueOf( (short) 2);

        log.println("Testing with custom Property tester");
        testProperty("ReferenceId", val1, val2);
    }

} //finish class _Footnote


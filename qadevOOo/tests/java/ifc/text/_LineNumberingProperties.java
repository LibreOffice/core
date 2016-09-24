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
* Testing <code>com.sun.star.text.LineNumberingProperties</code>
* service properties :
* <ul>
*  <li><code> IsOn</code></li>
*  <li><code> CharStyleName</code></li>
*  <li><code> CountEmptyLines</code></li>
*  <li><code> CountLinesInFrames</code></li>
*  <li><code> Distance</code></li>
*  <li><code> Interval</code></li>
*  <li><code> SeparatorText</code></li>
*  <li><code> SeparatorInterval</code></li>
*  <li><code> NumberPosition</code></li>
*  <li><code> NumberingType</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.LineNumberingProperties
*/
public class _LineNumberingProperties extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _CharStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("CharStyleName", "Endnote Symbol", "Endnote anchor") ;
    }

}



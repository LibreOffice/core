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
* Testing <code>com.sun.star.text.FootnoteSettings</code>
* service properties :
* <ul>
*  <li><code> CharStyleName</code></li>
*  <li><code> NumberingType</code></li>
*  <li><code> PageStyleName</code></li>
*  <li><code> ParaStyleName</code></li>
*  <li><code> Prefix</code></li>
*  <li><code> StartAt</code></li>
*  <li><code> Suffix</code></li>
*  <li><code> BeginNotice</code></li>
*  <li><code> EndNotice</code></li>
*  <li><code> FootnoteCounting</code></li>
*  <li><code> PositionEndOfDoc</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.FootnoteSettings
*/
public class _FootnoteSettings extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _CharStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("CharStyleName", "Endnote Symbol", "Endnote anchor") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _PageStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("PageStyleName", "Standard", "Endnote") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _ParaStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("ParaStyleName", "Standard", "Endnote") ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _NumberingType() {
        Short val1 = Short.valueOf(com.sun.star.text.FootnoteNumbering.PER_DOCUMENT);
        Short val2 = Short.valueOf(com.sun.star.text.FootnoteNumbering.PER_PAGE);
        log.println("Testing with custom Property tester") ;
        testProperty("NumberingType", val1, val2) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _FootnoteCounting() {
        Short val1 = Short.valueOf( (short) 1 );
        Short val2 = Short.valueOf( (short) 2 );
        log.println("Testing with custom Property tester") ;
        testProperty("FootnoteCounting", val1, val2) ;
    }

}



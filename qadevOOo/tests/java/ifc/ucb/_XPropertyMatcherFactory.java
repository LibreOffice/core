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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.RuleOperator;
import com.sun.star.ucb.RuleTerm;
import com.sun.star.ucb.SearchCriterium;
import com.sun.star.ucb.XPropertyMatcher;
import com.sun.star.ucb.XPropertyMatcherFactory;

/**
* Testing <code>com.sun.star.ucb.XPropertyMatcherFactory</code>
* interface methods :
* <ul>
*  <li><code> createPropertyMatcher()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XPropertyMatcherFactory
*/
public class _XPropertyMatcherFactory extends MultiMethodTest {

    public static XPropertyMatcherFactory oObj = null;

    /**
    * Tries to create <code>XPropertyMatcher</code> implementation. <p>
    * Has <b>OK</b> status if not null alue returned.
    */
    public void _createPropertyMatcher() {
        RuleTerm term = new RuleTerm() ;

        term.Property = "ContentType" ;
        term.Operand = "vnd.sun.star.fsys" ;
        term.Operator = RuleOperator.CONTAINS ;

        SearchCriterium crit = new SearchCriterium(new RuleTerm[] {term}) ;

        XPropertyMatcher matcher = oObj.createPropertyMatcher
            (new SearchCriterium[] {crit}) ;

        tRes.tested("createPropertyMatcher()", matcher != null) ;
    }
}



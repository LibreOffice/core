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
* Test is <b> NOT </b> multithread compilant. <p>
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



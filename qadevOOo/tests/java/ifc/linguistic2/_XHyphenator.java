/*************************************************************************
 *
 *  $RCSfile: _XHyphenator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:46:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XHyphenatedWord;
import com.sun.star.linguistic2.XHyphenator;
import com.sun.star.linguistic2.XPossibleHyphens;

/**
* Testing <code>com.sun.star.linguistic2.XHyphenator</code>
* interface methods:
* <ul>
*   <li><code>hyphenate()</code></li>
*   <li><code>queryAlternativeSpelling()</code></li>
*   <li><code>createPossibleHyphens()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XHyphenator
*/
public class _XHyphenator extends MultiMethodTest {

    public XHyphenator oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _hyphenate() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
            XHyphenatedWord result = oObj.hyphenate(
                    "wacker",new Locale("de","DE",""),(short)3,Props);
            res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'hyphenate'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("hyphenate()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _queryAlternativeSpelling() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
                XHyphenatedWord result = oObj.queryAlternativeSpelling(
                    "wacker",new Locale("de","DE",""),(short)2,Props);
                res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'queryAlternativeSpelling'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("queryAlternativeSpelling()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _createPossibleHyphens() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
            XPossibleHyphens result = oObj.createPossibleHyphens(
                    "wacker",new Locale("de","DE",""),Props);
            res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'createPossibleHyphens'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("createPossibleHyphens()",res);
    }

}  // \u0422\u044B finish class XHyphenator



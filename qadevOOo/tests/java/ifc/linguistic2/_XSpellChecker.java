/*************************************************************************
 *
 *  $RCSfile: _XSpellChecker.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:47:33 $
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
import com.sun.star.linguistic2.XSpellAlternatives;
import com.sun.star.linguistic2.XSpellChecker;

/**
* Testing <code>com.sun.star.linguistic2.XSpellChecker</code>
* interface methods:
* <ul>
*   <li><code>isValid()</code></li>
*   <li><code>spell()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XSpellChecker
*/
public class _XSpellChecker extends MultiMethodTest {

    public XSpellChecker oObj = null;

    /**
    * Test calls the method for a correctly spelled word and
    * for a uncorrectly spelled word and checks returned values. <p>
    * Has <b> OK </b> status if returned value is equal to true in first case,
    * if returned value is equal to false in second case and no exceptions
    * were thrown. <p>
    */
    public void _isValid() {
        boolean res = true;
        try {
            PropertyValue[] empty = new PropertyValue[0] ;
            res &= oObj.isValid("Sun", new Locale("en","US",""), empty);
            res &= !oObj.isValid("Summersun", new Locale("en","US","") ,empty);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'isValid'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("isValid()",res);
    }

    /**
    * Test calls the method for a uncorrectly spelled word
    * and checks returned values. <p>
    * Has <b> OK </b> status if at least one spell alternative exists
    * and no exceptions were thrown. <p>
    */
    public void _spell() {
        boolean res = true;
        try {
            PropertyValue[] empty = new PropertyValue[0] ;
            XSpellAlternatives alt = oObj.spell(
                            "Summersun",new Locale("en","US",""),empty);
            String alternative = alt.getAlternatives()[0];
            res = (alternative != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'spell'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("spell()",res);
    }

}  // finish class MTest



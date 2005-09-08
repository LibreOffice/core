/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XThesaurus.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:19:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XMeaning;
import com.sun.star.linguistic2.XThesaurus;

/**
* Testing <code>com.sun.star.linguistic2.XThesaurus</code>
* interface methods:
* <ul>
*   <li><code>queryMeanings()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XThesaurus
*/
public class _XThesaurus extends MultiMethodTest {

    public XThesaurus oObj = null;

    /**
    * Test calls the method for one of supported language and checks
    * returned value. <p>
    * Has <b> OK </b> status if returned array is not empty
    * and no exceptions were thrown. <p>
    */
    public void _queryMeanings() {
        boolean res = true;
        try {
            XMeaning[] mean = oObj.queryMeanings(
                "survive",new Locale("en","US",""), new PropertyValue[0]);
            res = (mean.length > 0);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'queryMeanings'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("queryMeanings()",res);
    }


}  // finish class



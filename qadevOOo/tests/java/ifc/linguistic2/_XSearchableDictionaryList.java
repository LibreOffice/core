/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSearchableDictionaryList.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:19:06 $
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

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XDictionaryEntry;
import com.sun.star.linguistic2.XSearchableDictionaryList;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.linguistic2.XSearchableDictionaryList</code>
* interface methods:
* <ul>
*   <li><code>queryDictionaryEntry()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XSearchableDictionaryList
*/
public class _XSearchableDictionaryList extends MultiMethodTest {

    public XSearchableDictionaryList oObj = null;

    /**
    * Test calls the method for negative dictionary and for positive dictionary
    * and checks returned values. <p>
    * Has <b> OK </b> status if returned values are not null. <p>
    */
    public void _queryDictionaryEntry() {
        XDictionaryEntry aNegativEntry = oObj.queryDictionaryEntry("Negativ",
                                    new Locale("en","US","WIN"),false,true);
        if (aNegativEntry == null) {
            log.println("'queryDictionary' didn't work for negative dictionaries");
        } else {
            log.println("'queryDictionary' works for negative dictionaries");
        }
        XDictionaryEntry aPositivEntry = oObj.queryDictionaryEntry("Positiv",
                                    new Locale("en","US","WIN"),true,true);
        if (aPositivEntry == null) {
            log.println("'queryDictionary' didn't work for positive dictionaries");
        } else {
            log.println("'queryDictionary' works for positive dictionaries");
        }
        tRes.tested("queryDictionaryEntry()",(aNegativEntry != null) &&
                                                    (aPositivEntry != null) );
    }

}  // finish class _XSearchableDictionaryList



/*************************************************************************
 *
 *  $RCSfile: _XSearchableDictionaryList.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:23:08 $
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



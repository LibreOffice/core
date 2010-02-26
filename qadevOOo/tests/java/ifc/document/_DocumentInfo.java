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

package ifc.document;

import lib.MultiPropertyTest;

import com.sun.star.util.DateTime;

/**
* Testing <code>com.sun.star.document.DocumentInfo</code>
* service properties : <p>
* <ul>
*   <li> <code> Author          </code> </li>
*   <li> <code> AutoloadEnabled </code> </li>
*   <li> <code> AutoloadSecs    </code> </li>
*   <li> <code> AutoloadURL     </code> </li>
*   <li> <code> BlindCopiesTo   </code> </li>
*   <li> <code> CopyTo          </code> </li>
*   <li> <code> CreationDate    </code> </li>
*   <li> <code> DefaultTarget   </code> </li>
*   <li> <code> Description     </code> </li>
*   <li> <code> InReplyTo       </code> </li>
*   <li> <code> IsEncrypted     </code> </li>
*   <li> <code> Keywords        </code> </li>
*   <li> <code> MIMEType        </code> </li>
*   <li> <code> ModifiedBy      </code> </li>
*   <li> <code> ModifyDate      </code> </li>
*   <li> <code> Newsgroups      </code> </li>
*   <li> <code> Original        </code> </li>
*   <li> <code> PrintDate       </code> </li>
*   <li> <code> PrintedBy       </code> </li>
*   <li> <code> Priority        </code> </li>
*   <li> <code> Recipient       </code> </li>
*   <li> <code> References      </code> </li>
*   <li> <code> ReplyTo         </code> </li>
*   <li> <code> Template        </code> </li>
*   <li> <code> TemplateDate    </code> </li>
*   <li> <code> Theme           </code> </li>
*   <li> <code> Title           </code> </li>
* </ul>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.document.DocumentInfo
*/
public class _DocumentInfo extends MultiPropertyTest {

    /**
     * Overrides compare method. Can compare <code>DateTime</code>
     * structures.
     */
    protected boolean compare(Object ob1, Object ob2) {
        if (ob1 instanceof DateTime && ob2 instanceof DateTime) {
            DateTime dt1 = (DateTime)ob1;
            DateTime dt2 = (DateTime)ob2;

            return dt1.Year == dt2.Year
                && dt1.Month == dt2.Month
                && dt1.Day == dt2.Day
                && dt1.Hours == dt2.Hours
                && dt1.Minutes == dt2.Minutes
                && dt1.Seconds == dt2.Seconds
                && dt1.HundredthSeconds == dt2.HundredthSeconds;
        } else {
            return super.compare(ob1, ob2);
        }
    }

    /**
     * Prints DateTime.
     */
    protected String toString(Object obj) {
        if (obj instanceof DateTime) {
            DateTime dt = (DateTime)obj;

            return dt.Year + ":" + dt.Month + ":" + dt.Day
                    + ":" + dt.Hours + ":" + dt.Minutes
                    + ":" + dt.Seconds + ":" + dt.HundredthSeconds;
        } else {
            return super.toString(obj);
        }
    }

    /**
     * Tester for properties with <code>DateTime</code> type which
     * creates new structure if old property value was null.
     */
    class DatePropertyTester extends PropertyTester {
        protected Object getNewValue(String propName,
                Object oldValue) {
            if (oldValue == null || util.utils.isVoid(oldValue)) {
                DateTime dt = new DateTime();

                dt.Year = 2000;
                dt.Month = 10;
                dt.Day = 18;
                dt.Hours = 19;
                dt.Minutes = 41;

                return dt;
            } else {
                return super.getNewValue(propName, oldValue);
            }
        }
    }

    /**
     * Can be void.
     */
    public void _PrintDate() {
        testProperty("PrintDate", new DatePropertyTester());
    }

    /**
     * Can be void.
     */
    public void _TemplateDate() {
        testProperty("TemplateDate", new DatePropertyTester());
    }

    /**
     * Can be void.
     */
    public void _ModifyDate() {
        testProperty("ModifyDate", new DatePropertyTester());
    }
}


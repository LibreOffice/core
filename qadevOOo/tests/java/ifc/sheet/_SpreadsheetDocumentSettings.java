/*************************************************************************
 *
 *  $RCSfile: _SpreadsheetDocumentSettings.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:57:25 $
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
package ifc.sheet;

import lib.MultiPropertyTest;

import com.sun.star.i18n.ForbiddenCharacters;
import com.sun.star.i18n.XForbiddenCharacters;
import com.sun.star.lang.Locale;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;


/**
* Testing <code>com.sun.star.sheet.SpreadsheetDocumentSettings</code>
* service properties :
* <ul>
*  <li><code> IsIterationEnabled</code></li>
*  <li><code> IterationCount</code></li>
*  <li><code> IterationEpsilon</code></li>
*  <li><code> StandardDecimals</code></li>
*  <li><code> NullDate</code></li>
*  <li><code> DefaultTabStop</code></li>
*  <li><code> IgnoreCase</code></li>
*  <li><code> CalcAsShown</code></li>
*  <li><code> MatchWholeCell</code></li>
*  <li><code> SpellOnline</code></li>
*  <li><code> LookUpLabels</code></li>
*  <li><code> RegularExpressions</code></li>
*  <li><code> ForbiddenCharacters</code></li>
*  <li><code> HasDrawPages</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.SpreadsheetDocumentSettings
*/
public class _SpreadsheetDocumentSettings extends MultiPropertyTest {
    /**
     *This class is destined to custom test of property <code>NullDate</code>.
     */
    protected PropertyTester DateTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            Date date = (Date) oldValue;
            Date newDate = new Date((short) (date.Day - 1), date.Month,
                                    date.Year);

            return newDate;
        }
    };

    /**
     *This class is destined to custom test of property <code>ForbiddenCharacters</code>.
     */
    protected PropertyTester ChrTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            return new ForbiddenChrTest();
        }

        protected boolean compare(Object obj1, Object obj2) {
            Locale loc = new Locale("ru", "RU", "");
            XForbiddenCharacters fc1 = (XForbiddenCharacters) UnoRuntime.queryInterface(
                                               XForbiddenCharacters.class,
                                               obj1);
            XForbiddenCharacters fc2 = (XForbiddenCharacters) UnoRuntime.queryInterface(
                                               XForbiddenCharacters.class,
                                               obj2);
            boolean has1 = fc1.hasForbiddenCharacters(loc);
            boolean has2 = fc2.hasForbiddenCharacters(loc);

            return has1 == has2;
        }
    };

    /**
     * Test property <code>NullDate</code> using custom <code>PropertyTest</code>.
     */
    public void _NullDate() {
        testProperty("NullDate", DateTester);
    }

    /**
     * Test property <code>ForbiddenCharacters</code> using custom <code>PropertyTest</code>.
     */
    public void _ForbiddenCharacters() {
        testProperty("ForbiddenCharacters", ChrTester);
    }

    /**
     * Class implements interface <code>XForbiddenCharacters</code>.
     * It's destined to custom test of property <code>ForbiddenCharacters</code>.
     * Feature of the class that it supports forbidden characters
     * for russian locale.
     * @see com.sun.star.i18n.XForbiddenCharacters
     */
    protected class ForbiddenChrTest implements XForbiddenCharacters {
        protected Locale locale = new Locale("ru", "RU", "");
        protected ForbiddenCharacters chrs = new ForbiddenCharacters("q", "w");

        public ForbiddenCharacters getForbiddenCharacters(Locale rLocale)
            throws com.sun.star.container.NoSuchElementException {
            if (rLocale.Country.equals(locale.Country) &&
                    rLocale.Language.equals(locale.Language) &&
                    rLocale.Variant.equals(locale.Variant)) {
                return chrs;
            }

            throw new com.sun.star.container.NoSuchElementException();
        }

        public void setForbiddenCharacters(Locale rLocale,
                                           ForbiddenCharacters rForbiddenCharacters) {
        }

        public void removeForbiddenCharacters(Locale rLocale) {
        }

        public boolean hasForbiddenCharacters(Locale rLocale) {
            if (rLocale.Country.equals(locale.Country) &&
                    rLocale.Language.equals(locale.Language) &&
                    rLocale.Variant.equals(locale.Variant)) {
                return true;
            }

            return false;
        }
    }
}
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
*  <li><code> Wildcards</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.SpreadsheetDocumentSettings
*/
public class _SpreadsheetDocumentSettings extends MultiPropertyTest {
    /**
     *This class is destined to custom test of property <code>NullDate</code>.
     */
    protected PropertyTester DateTester = new PropertyTester() {
        @Override
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
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            return new ForbiddenChrTest();
        }

        @Override
        protected boolean compare(Object obj1, Object obj2) {
            Locale loc = new Locale("ru", "RU", "");
            XForbiddenCharacters fc1 = UnoRuntime.queryInterface(
                                               XForbiddenCharacters.class,
                                               obj1);
            XForbiddenCharacters fc2 = UnoRuntime.queryInterface(
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
    protected static class ForbiddenChrTest implements XForbiddenCharacters {
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
            return rLocale.Country.equals(locale.Country) &&
                    rLocale.Language.equals(locale.Language) &&
                    rLocale.Variant.equals(locale.Variant);
        }
    }
}

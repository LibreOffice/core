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
package ifc.i18n;

import com.sun.star.i18n.TransliterationModules;
import com.sun.star.i18n.XExtendedTransliteration;
import com.sun.star.lang.Locale;
import lib.MultiMethodTest;

/**
 *
 */
public class _XExtendedTransliteration extends MultiMethodTest {
    public XExtendedTransliteration oObj = null;
//    private Locale loc = new Locale("ja", "JP", "") ;
    private Locale loc = new Locale("en", "US", "") ;

    public void before() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);
    }

    /**
     * Check lowercase - uppercase conversion of chars
     */
    public void _transliterateChar2Char() {
        boolean result = true;
        char in = 'a';
        char out = ' ';
        try {
            out = oObj.transliterateChar2Char(in) ;
            result &= out == 'A';
            in = '$'; // should not be changed
            out = oObj.transliterateChar2Char(in) ;
            result &= out == '$';
        }
        catch(com.sun.star.i18n.MultipleCharsOutputException e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }
        tRes.tested("transliterateChar2Char()", result);
    }

    /**
     * Check lowercase - uppercase conversion of char to string
     */
    public void _transliterateChar2String() {
        boolean result = true;
        char in = 'a';
        String out = null;
        out = oObj.transliterateChar2String('a') ;
        result &= out.equals("A");
        in = '$'; // should not be changed
        out = oObj.transliterateChar2String(in) ;
        result &= out.equals("$");
        tRes.tested("transliterateChar2String()", result);
    }

    /**
     * Check lowercase - uppercase conversion of strings
     */
    public void _transliterateString2String() {
        boolean result = true;
        String in = "aAbBcC";
        String out = null;
        out = oObj.transliterateString2String(in, 0, 6) ;
        result &= out.equals("AABBCC");
        in = "$"; // should not be changed
        out = oObj.transliterateString2String(in, 0, 1) ;
        result &= out.equals("$");
        tRes.tested("transliterateString2String()", result);
    }
}

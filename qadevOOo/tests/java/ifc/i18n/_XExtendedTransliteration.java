/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XExtendedTransliteration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:08:52 $
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

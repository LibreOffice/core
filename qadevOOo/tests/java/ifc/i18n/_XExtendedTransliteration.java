/*************************************************************************
 *
 *  $RCSfile: _XExtendedTransliteration.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-07-23 10:45:36 $
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

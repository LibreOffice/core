/*************************************************************************
 *
 *  $RCSfile: _XTransliteration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-07-23 10:45:51 $
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

import lib.MultiMethodTest;

import com.sun.star.i18n.TransliterationModules;
import com.sun.star.i18n.TransliterationModulesNew;
import com.sun.star.i18n.TransliterationType;
import com.sun.star.i18n.XTransliteration;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XTransliteration</code>
* interface methods :
* <ul>
*  <li><code> getName()</code></li>
*  <li><code> getType()</code></li>
*  <li><code> loadModule()</code></li>
*  <li><code> loadModuleNew()</code></li>
*  <li><code> loadModuleByImplName()</code></li>
*  <li><code> loadModulesByImplNames()</code></li>
*  <li><code> getAvailableModules()</code></li>
*  <li><code> transliterate()</code></li>
*  <li><code> folding()</code></li>
*  <li><code> equals()</code></li>
*  <li><code> transliterateRange()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XTransliteration
*/
public class _XTransliteration extends MultiMethodTest {

    public XTransliteration oObj = null;
    private String[] mod = null ;
    private Locale loc = new Locale("en", "EN", "") ;

    /**
    * Gets all available transliteration modules. <p>
    * Has <b>OK</b> status if array returned has at least
    * one module name.
    */
    public void _getAvailableModules() {
        mod = oObj.getAvailableModules(loc, TransliterationType.ONE_TO_ONE);

        if (mod != null) {
            log.println("Available modules :") ;
            for (int i = 0; i < mod.length; i++) {
                log.println("  '" + mod[i] + "'") ;
            }
        } else {
            log.println("!!! NULL returned !!!") ;
        }

        tRes.tested("getAvailableModules()", mod != null && mod.length > 0) ;
    }

    /**
    * Calls the method for load IGNORE_CASE module and checks the name returned
    * by the method <code>getName</code>. <p>
    * Has <b>OK</b> status if the method <code>getName</code> returns the
    * string "case ignore (generic)".
    */
    public void _loadModule() {
        log.println("Load module IGNORE_CASE");
        oObj.loadModule(TransliterationModules.IGNORE_CASE, loc);

        String name = oObj.getName();
        boolean res = name.equals("case ignore (generic)");
        log.println("getName return: " + name);

        tRes.tested("loadModule()", res );
    }

    /**
     * Loads <code>LOWERCASE_UPPERCASE</code> module and checks the current
     * name of object. <p>
     *
     * Has <b>OK</b> status if the name of the object is equals to
     * 'lower_to_upper(generic)'
     */
    public void _loadModuleNew() {
        boolean result = true ;

        oObj.loadModuleNew(
            new TransliterationModulesNew[]
            {TransliterationModulesNew.LOWERCASE_UPPERCASE}, loc);

        String name = oObj.getName();
        result = name.equals("lower_to_upper(generic)");
        log.println("getName return: " + name);

        tRes.tested("loadModuleNew()", result);
    }

    /**
    * Calls the method for load LOWERCASE_UPPERCASE module and
    * checks the name returned by the method <code>getName</code>. <p>
    * Has <b>OK</b> status if the method <code>getName</code> returns the
    * string "lower_to_upper(generic)".
    */
    public void _loadModuleByImplName() {
        log.println("Load module LOWERCASE_UPPERCASE");
        oObj.loadModuleByImplName("LOWERCASE_UPPERCASE", loc);

        String name = oObj.getName();
        boolean res = name.equals("lower_to_upper(generic)");
        log.println("getName return: " + name);

        tRes.tested("loadModuleByImplName()", res);
    }

    /**
    * Calls the method for load UPPERCASE_LOWERCASE module and
    * checks the name returned by the method <code>getName</code>. <p>
    * Has <b>OK</b> status if the method <code>getName</code> returns the
    * string "upper_to_lower(generic)".
    */
    public void _loadModulesByImplNames() {
        log.println("Load module UPPERCASE_LOWERCASE");
        oObj.loadModulesByImplNames(new String[]{"UPPERCASE_LOWERCASE"}, loc);

        String name = oObj.getName();
        boolean res = name.equals("upper_to_lower(generic)");
        log.println("getName return: " + name);

        tRes.tested("loadModulesByImplNames()", res);
    }

    /**
     * Loads <code>LOWERCASE_UPPERCASE</code> module and checks current type.
     * <p>Has <b>OK</b> status if the type is <code>ONE_TO_ONE</code>
     */
    public void _getType() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);
        boolean result = oObj.getType() == TransliterationType.ONE_TO_ONE;
        tRes.tested("getType()", result);
    }

    /**
    * Loads UPPERCASE_LOWERCASE module and
    * checks the name returned by the method <code>getName</code>. <p>
    *
    * Has <b>OK</b> status if the method <code>getName</code> returns the
    * string "upper_to_lower(generic)".
    */
    public void _getName() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);

        String name = oObj.getName();
        boolean res = name.equals("lower_to_upper(generic)");
        log.println("getName return: " + name);

        tRes.tested("getName()", res);
    }

    /**
    * First loads <code>LOWERCASE_UPPERCASE</code> module.
    * Then tries to transliterate (make uppercase) a substring. <p>
    * Has <b>OK</b> status if all chars were made uppercase,
    * and array returned has size as substring length, and its
    * elements are positions of substring characters in the source
    * string.
    */
    public void _transliterate() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);

        int[][] offs = new int[1][] ;

        String out = oObj.transliterate("AaBbCc", 1, 4, offs) ;

        boolean result = "ABBC".equals(out) && offs[0].length == 4 &&
            offs[0][0] == 1 &&
            offs[0][1] == 2 &&
            offs[0][2] == 3 &&
            offs[0][3] == 4 ;

        tRes.tested("transliterate()", result) ;
    }


    /**
    * First loads <code>LOWERCASE_UPPERCASE</code> module.
    * Tries to transliterate a range of two characters. <p>
    * Has <b>OK</b> status if the appropriate String array
    * returned (not null, length = 4, with two ranges
    * (a, i), (A, I) in any order).
    */
    public void _transliterateRange() {
        oObj.loadModule(TransliterationModules.IGNORE_CASE, loc);

        String[] out = oObj.transliterateRange("a", "i") ;

        log.println("transliterateRange return:");
        for(int i = 0; i < out.length; i++) {
            log.println(out[i]);
        }

        boolean bOK = out != null &&
            out.length == 4 &&
            ("A".equals(out[0]) && "I".equals(out[1]) &&
            "a".equals(out[2]) && "i".equals(out[3])) ||
            ("a".equals(out[0]) && "i".equals(out[1]) &&
            "A".equals(out[2]) && "I".equals(out[3])) ;

        if (!bOK) {
            log.println("Unexpected range returned :");
            for (int i = 0; i < out.length; i++) {
                log.print("'" + out[i] +"', ");
            }
            log.println();
        }

        tRes.tested("transliterateRange()", bOK);
    }

    /**
    * This method is used internally by <code>equals</code>
    * method so it indirectly tested in this method. <p>
    * Always has <b>OK</b> status.
    */
    public void _folding() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);

        int[][] offs = new int[1][] ;

        String out = oObj.folding("AaBbCc", 1, 4, offs) ;

        boolean result = "ABBC".equals(out) && offs[0].length == 4 &&
            offs[0][0] == 1 &&
            offs[0][1] == 2 &&
            offs[0][2] == 3 &&
            offs[0][3] == 4 ;


        tRes.tested("folding()", result) ;
    }


    /**
    * First loads <code>LOWERCASE_UPPERCASE</code> module.
    * Tries to compare two equal substrings. <p>
    * Has <b>OK</b> status if the method returned <code>true</code>.
    */
    public void _equals() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);

        int[] match1 = new int[1],
              match2 = new int[1] ;

        boolean res = oObj.equals("aAbBcC", 1, 3, match1, "aAbBcC", 1,
            3, match2) ;

        log.println("Returned : " + res + " Match1 = " + match1[0] +
            " Match2 = " + match2[0]) ;

        tRes.tested("equals()", res) ;
    }

    /**
     * Test performed for sets of equal substrings, not equal
     * substrings, and with out of bounds offset and length
     * parameters.<p>
     *
     * Has <b>OK</b> status if comparings of equal substrings
     * always return 0, if comparisons of none equal returns
     * proper value according to lexicographical order and if
     * comparisons with invalid parameters return none 0 value.
     */
    public void _compareSubstring() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);
        boolean result = true ;

        // substrings below must be equal
        result &= testSubstring("", 0, 0, "", 0, 0, 0) ;
        result &= testSubstring("aa", 1, 0, "", 0, 0, 0) ;
        result &= testSubstring("aa", 1, 0, "aa", 2, 0, 0) ;
        result &= testSubstring("a", 0, 1, "a", 0, 1, 0) ;
        result &= testSubstring("ab", 0, 2, "ab", 0, 2, 0) ;
        result &= testSubstring("abc", 1, 2, "abc", 1, 2, 0) ;
        result &= testSubstring("abcdef", 0, 3, "123abc", 3, 3, 0) ;
        result &= testSubstring("abcdef", 1, 1, "123abc", 4, 1, 0) ;

        // substrings below must NOT be equal
        result &= testSubstring("a", 0, 1, "a", 0, 0, 1) ;
        result &= testSubstring("aaa", 1, 1, "", 0, 0, 1) ;
        result &= testSubstring("bbb", 2, 1, "aaa", 2, 1, 1) ;
        result &= testSubstring("abc", 0, 3, "abc", 0, 2, 1) ;
        result &= testSubstring("bbc", 1, 2, "bbc", 0, 2, 1) ;

        // testing with wrong offsets and lengths

        tRes.tested("compareSubstring()", result) ;
    }

    /**
     * Performs tesing of two substrings. Also testing of opposite
     * substrings order performed.
     * @return <code>true</code> if substrings are equal and retruned
     * value is 0 for both orders,
     * if substrings are different and expected value
     * returned for direct order and opposite value returned for
     * opposite order.
     */
    private boolean testSubstring(String str1, int p1, int len1,
        String str2, int p2, int len2, int expRes) {

        boolean ret = true ;

        int res = -666 ;
        try {
            res = oObj.compareSubstring(str1, p1, len1, str2, p2, len2);
        } catch (java.lang.NullPointerException e) {
            log.println("Exception while method calling occurs :" + e);
        }

        if (res != expRes) {
            log.print("Comparing FAILED; return: " + res + ", expected: " +
                expRes + " ");
            ret = false ;
        } else {
            log.print("Comparing OK : ");
        }
        log.println("('" + str1 + "', " + p1 + ", " + len1 + ", '" +
            str2 + "', " + p2 + ", " + len2 + ")");

        res = -666 ;
        try {
            res = oObj.compareSubstring(str2, p2, len2, str1, p1, len1);
        } catch (java.lang.NullPointerException e) {
            log.println("Exception while method calling occurs :" + e);
        }

        if (res != -expRes) {
            log.print("Comparing FAILED; return: " + res + ", expected: " +
                -expRes  + " ");
            ret = false ;
        } else {
            log.print("Comparing OK :");
        }
        log.println("('" + str2 + "', " + p2 + ", " + len2 + ", '" +
            str1 + "', " + p1 + ", " + len1 + ")");

        return ret ;
    }

    /**
     * Test performed for sets of equal strings and not equal
     * strings.<p>
     *
     * Has <b>OK</b> status if comparings of equal strings
     * always return 0 and if comparisons of none equal returns
     * proper value according to lexicographical order .
     */
    public void _compareString() {
        oObj.loadModule(TransliterationModules.LOWERCASE_UPPERCASE, loc);
        boolean result = true ;

        result &= testString("", "", 0) ;
        result &= testString("a", "", 1) ;
        result &= testString("a", "a", 0) ;
        result &= testString("A", "a", 0) ;
        result &= testString("b", "a", 1) ;
        result &= testString("\n", "\n", 0) ;
        result &= testString("\n", "\t", 1) ;
        result &= testString("aaa", "aaa", 0) ;
        result &= testString("aaA", "aaa", 0) ;
        result &= testString("aaa", "aa", 1) ;
        result &= testString("ab", "aaa", 1) ;
        result &= testString("aba", "aa", 1) ;
        result &= testString("aaa\t\na", "aaa\t\na", 0) ;
        result &= testString("aaa\t\nb", "aaa\t\na", 1) ;

        tRes.tested("compareString()", result) ;
    }

    /**
     * Performs tesing of two strings. If the expected value is not 0
     * (i.e. strings are not equal), then also testing of opposite
     * strings order performed.
     * @return <code>true</code> if strings are equal and retruned
     * value is 0, if strings are different and expected value
     * returned for direct order and opposite value returned for
     * opposite order.
     */
    protected boolean testString(String str1, String str2, int expRes) {
        if (expRes == 0) return testString(str1, str2, expRes, false) ;
        return testString(str1, str2, expRes, true) ;
    }

    private boolean testString(String str1, String str2, int expRes,
        boolean testReverse) {

        boolean ret = true ;

        int res = -666 ;
        try {
            res = oObj.compareString(str1, str2);
        } catch (java.lang.NullPointerException e) {
            log.println("Exception while method calling occurs :" + e);
        }

        if (res == expRes) {
            log.println("Comparing of '" + str1 + "' and '" + str2 + "' OK" );
        } else {
            log.println("Comparing of '" + str1 + "' and '" + str2 +
                "' FAILED; return: " + res + ", expected: " + expRes);
            ret = false ;
        }

        if (!testReverse) return ret ;

        res = -666 ;
        try {
            res = oObj.compareString(str2, str1);
        } catch (java.lang.NullPointerException e) {
            log.println("Exception while method calling occurs :" + e);
        }

        if (res == -expRes) {
            log.println("Comparing of '" + str2 + "' and '" + str1 + "' OK" );
        } else {
            log.println("Comparing of '" + str2 + "' and '" + str1 +
                "' FAILED; return: " + res + ", expected: " + -expRes);
            ret = false ;
        }

        return ret ;
    }
}


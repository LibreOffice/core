/*************************************************************************
 *
 *  $RCSfile: _XCollator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:41:12 $
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

import java.text.Collator;

import lib.MultiMethodTest;

import com.sun.star.i18n.CollatorOptions;
import com.sun.star.i18n.XCollator;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XCollator</code>
* interface methods :
* <ul>
*  <li><code> compareSubstring()</code></li>
*  <li><code> compareString()</code></li>
*  <li><code> loadDefaultCollator()</code></li>
*  <li><code> loadCollatorAlgorithm()</code></li>
*  <li><code> listCollatorAlgorithms()</code></li>
*  <li><code> loadCollatorAlgorithmWithEndUserOption()</code></li>
*  <li><code> listCollatorOptions()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XCollator
*/
public class _XCollator extends MultiMethodTest {
    public XCollator oObj = null;
    private String[] alg = null ;
    private int[] opt = null ;
    Locale loc = new Locale("en", "EN", "");

    /**
    * Just retrieves a list of algorithms. <p>
    * Has <b>OK</b> status if non-zero length array returned.
    */
    public void _listCollatorAlgorithms() {
        alg = oObj.listCollatorAlgorithms(loc) ;
        log.println("Collator algorithms :");
        if (alg != null) {
            for (int i = 0; i < alg.length; i++) {
                log.println("  '" + alg[i] + "'") ;
            }
            tRes.tested("listCollatorAlgorithms()", alg.length > 0) ;
        } else {
            tRes.tested("listCollatorAlgorithms()", false) ;
        }
    }

    /**
    * Just gets a list of options for some collator. <p>
    * Has <b>OK</b> status if not null value returned.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> listCollatorAlgorithms </code> : to have some
    *    algorithm name. </li>
    * </ul>
    */
    public void _listCollatorOptions() {
        requiredMethod("listCollatorAlgorithms()") ;
        opt = oObj.listCollatorOptions(alg[0]) ;
        log.println("Collator '" + alg[0] + "' options :");
        if (opt != null) {
            for (int i = 0; i < opt.length; i++) {
                log.println("  " + opt[i]) ;
            }
            tRes.tested("listCollatorOptions()", true) ;
        } else {
            tRes.tested("listCollatorOptions()", false) ;
        }
    }

    /**
    * Calls the method with no options and with options(IGNORE_CASE),
    * compares strings.<p>
    * Has <b>OK</b> status if compareString() returned correct values.
    */
    public void _loadDefaultCollator() {
        oObj.loadDefaultCollator(loc, 0);
        boolean res = oObj.compareString("A", "a") != 0;
        oObj.loadDefaultCollator(loc,
            CollatorOptions.CollatorOptions_IGNORE_CASE);
        res &= oObj.compareString("a", "A") == 0;
        tRes.tested("loadDefaultCollator()", res) ;
    }

    /**
    * Calls the method with no options and with options(IGNORE_CASE),
    * compares strings.<p>
    * Has <b>OK</b> status if compareString() returned correct values.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> listCollatorAlgorithms </code> : to have some
    *    algorithm name. </li>
    * </ul>
    */
    public void _loadCollatorAlgorithm() {
        requiredMethod("listCollatorAlgorithms()");
        oObj.loadCollatorAlgorithm(alg[0], loc,
            CollatorOptions.CollatorOptions_IGNORE_CASE);
        boolean res = oObj.compareString("A", "a") == 0;
        oObj.loadCollatorAlgorithm(alg[0], loc, 0);
        res &= oObj.compareString("a", "A") != 0;
        tRes.tested("loadCollatorAlgorithm()", res);
    }

    /**
    * Calls the method with no options and with options(IGNORE_CASE),
    * compares strings.<p>
    * Has <b>OK</b> status if compareString() returned correct values.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> listCollatorAlgorithms </code> : to have some
    *    algorithm name. </li>
    * </ul>
    */
    public void _loadCollatorAlgorithmWithEndUserOption() {
        requiredMethod("listCollatorAlgorithms()");
        oObj.loadCollatorAlgorithmWithEndUserOption(alg[0], loc,
            new int[] {0});
        boolean res = oObj.compareString("A", "a") != 0;
        oObj.loadCollatorAlgorithmWithEndUserOption(alg[0], loc,
            new int[] {CollatorOptions.CollatorOptions_IGNORE_CASE});
        res = oObj.compareString("A", "a") == 0;
        tRes.tested("loadCollatorAlgorithmWithEndUserOption()", res);
    }

    /**
    * Test is performed for locales : en, ru, ja, zh, ko.
    * Default collator is loaded for each locale. Then collation
    * is performed for different combination of symbols from range of
    * this locale.<p>
    * Has <b>OK</b> status if comparing of different strings
    * returns not 0 value, then comparing in the opposite
    * order returns value with opposite sign, and comparing
    * of two equal strings returns 0. The such comparing is performed
    * for one character strings.
    */
    public void _compareSubstring() {
        boolean result = true ;
        char[] chars = new char[2] ;
        Collator col = null ;

        log.println(" #### Testing English locale ####") ;
        oObj.loadDefaultCollator(loc, 0) ;
        col = Collator.getInstance(new java.util.Locale("en", "EN")) ;
        for (char ch = 0x0020; ch < 0x007F; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareSubstring(chars, col) ;
        }

        log.println(" #### Testing Russian locale ####") ;
        oObj.loadDefaultCollator(
            new com.sun.star.lang.Locale("ru", "RU", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ru", "RU")) ;
        for (char ch = 0x0410; ch < 0x0450; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareSubstring(chars, col) ;
        }

        log.println(" #### Testing Japan locale ####") ;
        oObj.loadDefaultCollator(
            new com.sun.star.lang.Locale("ja", "JP", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ja", "JP")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareSubstring(chars, col) ;
        }

        log.println(" #### Testing China locale ####") ;
        oObj.loadDefaultCollator(new Locale("zh", "CN", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("zh", "CN")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareSubstring(chars, col) ;
        }

        log.println(" #### Testing Korean locale ####") ;
        oObj.loadDefaultCollator(new Locale("ko", "KR", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ko", "KR")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareSubstring(chars, col) ;
        }

        tRes.tested("compareSubstring()", result) ;
    }

    /**
    * Test is performed for locales : en, ru, ja, zh, ko.
    * Default collator is loaded for each locale. Then collation
    * is performed for different combination of symbols from range of
    * this locale.<p>
    * Has <b>OK</b> status if comparing of different strings
    * returns not 0 value, then comparing in the opposite
    * order returns value with opposite sign, and comparing
    * of two equal strings returns 0. The such comparing is performed
    * for one character strings.
    */
    public void _compareString() {
        boolean result = true ;
        char[] chars = new char[2] ;
        Collator col = null ;
        log.println(" #### Testing English locale ####") ;
        oObj.loadDefaultCollator(
            new com.sun.star.lang.Locale("en", "EN", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("en", "EN")) ;
        for (char ch = 0x0020; ch < 0x007F; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareString(chars, col) ;
        }

        log.println(" #### Testing Russian locale ####") ;
        oObj.loadDefaultCollator(
            new com.sun.star.lang.Locale("ru", "RU", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ru", "RU")) ;
        for (char ch = 0x0410; ch < 0x0450; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareString(chars, col) ;
        }

        log.println(" #### Testing Japan locale ####") ;
        oObj.loadDefaultCollator(
            new com.sun.star.lang.Locale("ja", "JP", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ja", "JP")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareString(chars, col) ;
        }

        log.println(" #### Testing China locale ####") ;
        oObj.loadDefaultCollator(new Locale("zh", "CN", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("zh", "CN")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareString(chars, col) ;
        }

        log.println(" #### Testing Korean locale ####") ;
        oObj.loadDefaultCollator(new Locale("ko", "KR", ""), 0) ;
        col = Collator.getInstance(new java.util.Locale("ko", "KR")) ;
        for (char ch = 0x4E00; ch < 0x4EFD; ch ++) {
            chars[0] = ch ; chars[1] = (char) (ch + 1) ;
            result &= testCompareString(chars, col) ;
        }

        tRes.tested("compareString()", result) ;
    }


    /**
    * Testing compareString() method. At first method is testing single chars
    * comparing, then strings comparing.
    * @param locChar sequence of at list two characters of a given locale
    * to be used in comparing.
    * @param col Collator for a given locale
    * @return true if:
    * <ol>
    *  <li> if comparing of two identical characters returns zero</li>
    *  <li> if comparing of two different characters returns non zero</li>
    *  <li> if comparing of two identical strings, composed of given chars
    *  returns zero</li>
    *  <li> if comparing of two different strings, composed of given chars
    *  returns non zero</li>
    * </ol>
    */
    public boolean testCompareString(char[] locChar, Collator col) {
        boolean result = true;
        int res;
        String msg = "";

        String char0 = "_"+new String(new char[] {locChar[0]});
        String char1 = "_"+new String(new char[] {locChar[1]});
        res = oObj.compareString(char0 , char0) ;
        if (res != 0) {
            msg += "  Testing collation of single equal characters ("
                + toUnicode(char0) + ") ... FAILED\n" ;
        }
        result &= res == 0 ;
        res = oObj.compareString(char0, char1) ;
        if (res == 0) {
            msg += "  Testing collation of single different" +
                " characters (" + toUnicode(char0+char1) +
                ") ... FAILED (0 returned)\n" ;
            msg += "  Java collator returned " +
                col.compare(char0, char1) + "\n" ;
            result = false ;
        } else { // opposite order - sum of results must be 0
            res += oObj.compareString(char1, char0) ;
            if (res != 0) {
                msg += "  Testing collation of single different" +
                    " characters (" + toUnicode(char0+char1) +
                    ") ... FAILED\n" ;
            }
            result &= res == 0 ;
        }

        String str1 = new String(new char[] {locChar[0], locChar[0],
            locChar[1], locChar[1], locChar[1]}) ;
        String str2 = new String(new char[] {locChar[0], locChar[0],
            locChar[0], locChar[1], locChar[1]}) ;

        res = oObj.compareString(str1 , str1) ;
        if (res != 0) {
            msg += "  Testing collation of equal strings (" +
                toUnicode(str1) + ") ... FAILED\n" ;
        }
        result &= res == 0 ;
        res = oObj.compareString(str1, str2) ;
        if (res == 0) {
            msg += "  Testing collation of different strings ((" +
                toUnicode(str1) + "),(" + toUnicode(str2) +
                ")) ... FAILED (0 returned)\n" ;
            msg += "  Java collator returned " +
                col.compare(str1, str2) + "\n" ;
            result = false ;
        } else { // opposite order - sum of results must be
            res += oObj.compareString(str2, str1) ;
            if (res != 0) {
                msg += "  Testing collation of different strings ((" +
                    toUnicode(str1) + "),(" + toUnicode(str2) +
                    ")) ... FAILED\n" ;
            }
            result &= res == 0 ;
        }

        if (!result) {
            log.println(msg) ;
        }
        return result ;
    }


    /**
    * Testing compareSubstring() method. Method is testing substrings comparing.
    * @param locChar sequence of at list two characters of a given locale
    * to be used in comparing.
    * @param col Collator for a given locale
    * @return true if:
    * <ol>
    *  <li> if comparing of two identical substrings of strings, composed
    *  of given chars returns zero</li>
    *  <li> if comparing of two different substrings of strings, composed
    *  of given chars returns non zero</li>
    * </ol>
    */
    public boolean testCompareSubstring(char[] locChar, Collator col) {
        boolean result = true ;
        int res ;
        String msg = "" ;

        String str1 = new String(new char[] {locChar[0], locChar[0],
            locChar[1], locChar[1], locChar[1]}) ;
        String str2 = new String(new char[] {locChar[0], locChar[0],
            locChar[0], locChar[1], locChar[1]}) ;

        res = oObj.compareSubstring(str1, 1, 2 , str2, 2, 2) ;
        if (res != 0) {
            msg += "  Testing collation of equal substrings (" +
                toUnicode(str1) + ") ... FAILED\n" ;
        }
        result &= res == 0 ;
        res = oObj.compareSubstring(str1, 1, 2, str2, 1, 2) ;
        if (res == 0) {
            msg += "  Testing collation of different strings ((" +
                toUnicode(str1.substring(1, 3)) + "),(" +
                toUnicode(str2.substring(1, 3))
                + ")) ... FAILED (0 returned)\n" ;
            msg += "  Java collator returned " + col.compare
                (str1.substring(1, 3), str2.substring(1, 3)) + "\n" ;
            result = false ;
        } else { // opposite order - sum of results must be
            res += oObj.compareSubstring(str2, 1, 2, str1, 1, 2) ;
            if (res != 0) {
                msg += "  Testing collation of different strings ((" +
                    toUnicode(str1) + "),(" + toUnicode(str2) +
                    ")) ... FAILED\n" ;
            }
            result &= res == 0 ;
        }

        if (!result) {
            log.println(msg) ;
        }
        return result ;
    }


    /**
    * Transforms string to unicode hex codes.
    * @param str String to be transformed
    */
    public String toUnicode(String str) {
        char[] chars = str.toCharArray() ;
        String res = "" ;
        for (int i = 0; i < chars.length; i++) {
            if (i != 0) res += "," ;
            res += Integer.toHexString(chars[i]) ;
        }
        return res ;
    }

}


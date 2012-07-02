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

package ifc.i18n;

import lib.MultiMethodTest;

import com.sun.star.i18n.KParseTokens;
import com.sun.star.i18n.KParseType;
import com.sun.star.i18n.ParseResult;
import com.sun.star.i18n.XCharacterClassification;
import com.sun.star.lang.Locale;

/**
 * Testing <code>com.sun.star.i18n.XCharacterClassification</code>
 * interface methods:
 * <ul>
 *  <li><code> toUpper() </code></li>
 *  <li><code> toLower() </code></li>
 *  <li><code> toTitle() </code></li>
 *  <li><code> getType() </code></li>
 *  <li><code> getCharacterType() </code></li>
 *  <li><code> getStringType() </code></li>
 *  <li><code> getCharacterDirection() </code></li>
 *  <li><code> getScript() </code></li>
 *  <li><code> parseAnyToken() </code></li>
 *  <li><code> parsePredefinedToken() </code></li>
 * </ul><p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.i18n.XCharacterClassification
 */
public class _XCharacterClassification extends MultiMethodTest {
    public XCharacterClassification oObj = null;
    public String[] languages = new String[]{"de","en","es","fr","ja","ko","zh"};
    public String[] countries = new String[]{"DE","US","ES","FR","JP","KR","CN"};

    public String[] charstyles_java = new String[] {"UNASSIGNED","UPPERCASE_LETTER",
        "LOWERCASE_LETTER","TITLECASE_LETTER","MODIFIER_LETTER","OTHER_LETTER",
        "NON_SPACING_MARK","ENCLOSING_MARK","COMBINING_SPACING_MARK",
        "DECIMAL_DIGIT_NUMBER","LETTER_NUMBER","OTHER_NUMBER","SPACE_SEPARATOR",
        "LINE_SEPARATOR","PARAGRAPH_SEPARATOR","CONTROL","FORMAT","none17",
        "PRIVATE_USE","none19","DASH_PUNCTUATION","START_PUNCTUATION","END_PUNCTUATION",
        "CONNECTOR_PUNCTUATION","OTHER_PUNCTUATION","MATH_SYMBOL","CURRENCY_SYMBOL",
        "MODIFIER_SYMBOL","OTHER_SYMBOL"};

    public String[] charstyles_office = new String[] {"UNASSIGNED","UPPERCASE_LETTER",
        "LOWERCASE_LETTER","TITLECASE_LETTER","MODIFIER_LETTER","OTHER_LETTER",
        "NON_SPACING_MARK","ENCLOSING_MARK","COMBINING_SPACING_MARK",
        "DECIMAL_DIGIT_NUMBER","LETTER_NUMBER","OTHER_NUMBER","SPACE_SEPARATOR",
        "LINE_SEPARATOR","PARAGRAPH_SEPARATOR","CONTROL","FORMAT","PRIVATE_USE",
        "OTHER_PUNCTUATION","DASH_PUNCTUATION","START_PUNCTUATION","END_PUNCTUATION",
        "CONNECTOR_PUNCTUATION",
        "OTHER_PUNCTUATION","MATH_SYMBOL","CURRENCY_SYMBOL","MODIFIER_SYMBOL",
        "OTHER_SYMBOL","INITIAL_PUNCTUATION","FINAL_PUNCTUATION","GENERAL_TYPES_COUNT"};

    public String[] unicode_script = new String[] {"U_BASIC_LATIN","U_LATIN_1_SUPPLEMENT",
        "U_LATIN_EXTENDED_A","U_LATIN_EXTENDED_B","U_IPA_EXTENSIONS","U_SPACING_MODIFIER_LETTERS",
        "U_COMBINING_DIACRITICAL_MARKS","U_GREEK","U_CYRILLIC","U_ARMENIAN","U_HEBREW",
        "U_ARABIC","U_SYRIAC","U_THAANA","U_DEVANAGARI","U_BENGALI","U_GURMUKHI",
        "U_GUJARATI","U_ORIYA","U_TAMIL","U_TELUGU","U_KANNADA","U_MALAYALAM",
        "U_SINHALA","U_THAI","U_LAO","U_TIBETAN","U_MYANMAR","U_GEORGIAN",
        "U_HANGUL_JAMO","U_ETHIOPIC","U_CHEROKEE","U_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS",
        "U_OGHAM","U_RUNIC","U_KHMER","U_MONGOLIAN","U_LATIN_EXTENDED_ADDITIONAL",
        "U_GREEK_EXTENDED","U_GENERAL_PUNCTUATION","U_SUPERSCRIPTS_AND_SUBSCRIPTS",
        "U_CURRENCY_SYMBOLS","U_COMBINING_MARKS_FOR_SYMBOLS","U_LETTERLIKE_SYMBOLS",
        "U_NUMBER_FORMS","U_ARROWS","U_MATHEMATICAL_OPERATORS","U_MISCELLANEOUS_TECHNICAL",
        "U_CONTROL_PICTURES","U_OPTICAL_CHARACTER_RECOGNITION","U_ENCLOSED_ALPHANUMERICS",
        "U_BOX_DRAWING","U_BLOCK_ELEMENTS","U_GEOMETRIC_SHAPES","U_MISCELLANEOUS_SYMBOLS",
        "U_DINGBATS","U_BRAILLE_PATTERNS","U_CJK_RADICALS_SUPPLEMENT","U_KANGXI_RADICALS",
        "U_IDEOGRAPHIC_DESCRIPTION_CHARACTERS","U_CJK_SYMBOLS_AND_PUNCTUATION",
        "U_HIRAGANA","U_KATAKANA","U_BOPOMOFO","U_HANGUL_COMPATIBILITY_JAMO","U_KANBUN",
        "U_BOPOMOFO_EXTENDED","U_ENCLOSED_CJK_LETTERS_AND_MONTHS","U_CJK_COMPATIBILITY",
        "U_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A","U_CJK_UNIFIED_IDEOGRAPHS","U_YI_SYLLABLES",
        "U_YI_RADICALS","U_HANGUL_SYLLABLES","U_HIGH_SURROGATES","U_HIGH_PRIVATE_USE_SURROGATES",
        "U_LOW_SURROGATES","U_PRIVATE_USE_AREA","U_CJK_COMPATIBILITY_IDEOGRAPHS",
        "U_ALPHABETIC_PRESENTATION_FORMS","U_ARABIC_PRESENTATION_FORMS_A","U_COMBINING_HALF_MARKS",
        "U_CJK_COMPATIBILITY_FORMS","U_SMALL_FORM_VARIANTS","U_ARABIC_PRESENTATION_FORMS_B",
        "U_SPECIALS","U_HALFWIDTH_AND_FULLWIDTH_FORMS","U_CHAR_SCRIPT_COUNT","U_NO_SCRIPT"};

    /**
    * Test calls the method for different locales. Then each result is compared
    * with a string, converted to a upper case using
    * <code>java.lang.String</code> method <code>toUpperCase()</code>.<p>
    * Has <b> OK </b> status if string, returned by the method is equal to
    * a string that is returned by String.toUpperCase() for all locales.
    */
    public void _toUpper() {
        boolean res = true;
        char[] characters = new char[]{586,65,97,498,721,4588,772,8413,3404};
        String toCheck = new String(characters);
        String get = "";
        String exp = "";

        for (int i=0;i<7;i++) {
            get = oObj.toUpper(toCheck, 0, toCheck.length(), getLocale(i));
            exp = toCheck.toUpperCase(
                new java.util.Locale(languages[i], countries[i]));
            res &= get.equals(exp);
            if (!res) {
                log.println("FAILED for: language=" + languages[i] +
                    " ; country=" + countries[i]);
                log.println("Expected: " + exp);
                log.println("Gained : " + get);
            }
        }
        tRes.tested("toUpper()", res);
    }

    /**
    * Test calls the method for different locales. Then each result is compared
    * with a string, converted to a lower case using
    * <code>java.lang.String</code> method <code>toLowerCase()</code>.<p>
    * Has <b> OK </b> status if string, returned by the method is equal to
    * a string that is returned by String.toLowerCase() for all locales.
    */
    public void _toLower() {
        boolean res = true;
        char[] characters = new char[]{586,65,97,498,721,4588,772,8413,3404};
        String toCheck = new String(characters);
        String get = "";
        String exp = "";

        for (int i=0;i<7;i++) {
            get = oObj.toLower(toCheck,0,toCheck.length(),getLocale(i));
            exp = toCheck.toLowerCase(
                new java.util.Locale(languages[i],countries[i]));
            res &= get.equals(exp);
            if (!res) {
                log.println("FAILED for: language=" + languages[i]
                    + " ; country=" + countries[i]);
                log.println("Expected: " + exp);
                log.println("Gained : " + get);
            }
        }
        tRes.tested("toLower()", res);
    }

    /**
    * Test calls the method for different locales. Then each result is compared
    * with a string, converted to a title case using
    * <code>java.lang.Character</code> method <code>toTitleCase()</code>.<p>
    * Has <b> OK </b> status if string, returned by the method is equal to
    * a string that was converted using Character.toTitleCase() for all locales.
    */
    public void _toTitle() {
        boolean res = true;
        String toCheck = new String(new char[]{8112});
        String get = "";
        String exp = "";

        for (int i=0;i<7;i++) {
            get = oObj.toTitle(toCheck, 0, 1, getLocale(i));
            exp = new String(
                new char[]{Character.toTitleCase(toCheck.toCharArray()[0])});
            res &= get.equals(exp);
            if (!res) {
                log.println("FAILED for: language=" + languages[i]
                    + " ; country=" + countries[i]);
                log.println("Expected: " + exp);
                log.println("Gained : " + get);
            }
        }
        tRes.tested("toTitle()", res);
    }

    /**
    * At first we define <code>int[]</code> and <code>char[]</code> arrays of
    * unicode symbol numbers, arranged as sequences, where symbols are sorted
    * by type, so the character of <code>i<sup><small>th</small></sup></code>
    * type is located on <code>i<sup><small>th</small></sup></code> position.<p>
    * Has <b> OK </b> status if for all 30 types the method returns value, that
    * is equal to an element number.<p>
    * @see com.sun.star.i18n.CharType
    */
    public void _getType() {
        boolean res = true;
        char[] characters = new char[]{586,65,97,498,721,4588,772,8413,3404,
            48,8544,179,32,8232,8233,144,8204,57344,56320,173,40,41,95,3852,247,
            3647,901,3896,171,187};
        int[] charsInt = new int[]{586,65,97,498,721,4588,772,8413,3404,48,
            8544,179,32,8232,8233,144,8204,57344,56320,173,40,41,95,3852,247,
            3647,901,3896,171,187};
        String toCheck = new String(characters);

        for (int i=0;i<characters.length;i++) {
            int get = oObj.getType(toCheck, i);
            res &= (charstyles_office[get] == charstyles_office[i]);
            if (!res) {
                log.println("Code :" + Integer.toHexString(charsInt[i]));
                log.println("Gained: " + charstyles_office[get]);
                log.println("Expected : " + charstyles_office[i]);
            }
        }
        tRes.tested("getType()", res);
    }

    /**
    * After defining string to be checked and array of expected types, test
    * calls the method for each character of a string and for all locales.<p>
    * Has <b> OK </b> status if the method returns type, expected for a given
    * character and locale.
    */
    public void _getCharacterType() {
        boolean res = true;
        String toCheck = "Ab0)";
        int[] expected = new int[]{226,228,97,32};

        for (int i=0;i<toCheck.length();i++) {
            for (int j=1;j<7;j++) {
                int get = oObj.getCharacterType(toCheck, i, getLocale(j));
                res &= (get == expected[i]);
                if (!res) {
                    log.println("FAILED for: language=" + languages[j] +
                        " ; country=" + countries[j]);
                    log.println("Sysmbol :" + toCheck.toCharArray()[i]);
                    log.println("Gained: " + get);
                    log.println("Expected : " + expected[i]);
                }
            }
        }
        tRes.tested("getCharacterType()", res);
    }

    /**
    * After defining array of strings to be checked and array of expected types,
    * test calls the method for each string of an array and for all locales.<p>
    * Has <b> OK </b> status if the method returns type, expected for a given
    * string and locale.
    */
    public void _getStringType() {
        boolean res = true;
        String[] toCheck = new String[]{"01234","AAAAA","bbbbb","AA()bb"};
        int[] exp = new int[]{97,226,228,230};

        for (int j=0;j<toCheck.length;j++) {
            for (int i=0;i<7;i++) {
                int get = oObj.getStringType(toCheck[j], 0,
                    toCheck[j].length(), getLocale(i));
                res &= (get == exp[j]);
                if (!res) {
                    log.println("FAILED for: language=" + languages[i] +
                        " ; country=" + countries[i]);
                    log.println("Expected: " + exp[j]);
                    log.println("Gained : " + get);
                }
            }
        }
        tRes.tested("getStringType()", res);
    }

    /**
    * After string to be checked is initialized (all symbols are sorted
    * by direction, so the character of <code>i<sup><small>th</small></sup></code>
    * direction is located on <code>i<sup><small>th</small></sup></code>
    * position), test calls the method for every character of that string. <p>
    * Has <b> OK </b> status if the method returns direction, that's equal to
    * a symbol position in the string.
    */
    public void _getCharacterDirection() {
        boolean res = true;
        String toCheck = new String(new char[]{65,1470,48,47,35,1632,44,10,
                                9,12,33,8234,8237,1563,8235,8238,8236,768,1});
        for (short i=0;i<19;i++) {
            short get = oObj.getCharacterDirection(toCheck, i);
            res &= (get == i);
            if (!res) {
                log.println("Code :" + toCheck.toCharArray()[i]);
                log.println("Gained: " + get);
                log.println("Expected: " + i);
            }
        }
        tRes.tested("getCharacterDirection()", res);
    }

    /**
    * At first we define <code>int[]</code> and <code>char[]</code> arrays of
    * unicode symbol numbers, arranged as sequences, where symbols are sorted
    * by type, so the character of <code>i<sup><small>th</small></sup></code>
    * type is located on <code>i<sup><small>th</small></sup></code> position.<p>
    * Has <b> OK </b> status if for each character method returns value, that
    * is equal to a number where element is located in array. Also method has
    * <b> OK </b> status for symbol with code 55296, because it doesn't work
    * since it hasn't the right neighborhood.<p>
    * @see "http://ppewww.ph.gla.ac.uk/~flavell/unicode/unidata.html"
    */
    public void _getScript() {
        boolean res = true;
        char[] characters = new char[]{65,128,256,384,592,750,773,924,1030,1331,1448,
            1569,1792,1936,2313,2465,2570,2707,2822,2972,3079,3240,3337,3464,3590,
            3745,3906,4097,4274,4357,4621,5040,5200,5776,5806,6030,6155,7683,7943,
            8202,8319,8352,8413,8452,8545,8616,8715,8965,9217,9281,9336,9474,9608,9719,
            9734,9999,10247,11911,12034,12274,12294,12358,12456,12552,12605,12688,12727,
            12806,13065,13312,19968,40964,42152,44032,55296,56192,56320,57344,63744,
            64257,64370,65056,65073,65131,65146,65532,65288};
        int[] charsInt = new int[]{65,128,256,384,592,750,773,924,1030,1331,1448,
            1569,1792,1936,2313,2465,2570,2707,2822,2972,3079,3240,3337,3464,3590,
            3745,3906,4097,4274,4357,4621,5040,5200,5776,5806,6030,6155,7683,7943,
            8202,8319,8352,8413,8452,8545,8616,8715,8965,9217,9281,9336,9474,9608,9719,
            9734,9999,10247,11911,12034,12274,12294,12358,12456,12552,12605,12688,12727,
            12806,13065,13312,19968,40964,42152,44032,55296,56192,56320,57344,63744,
            64257,64370,65056,65073,65131,65146,65532,65288};
        String toCheck = new String(characters);

        for (int i=0;i<characters.length;i++) {
            int get = oObj.getScript(toCheck, i);
            res &= (get == i);
            //The HIGH_SURROGATE 55296 doesn't work since it hasn't the right
            //neighborhood
            if (toCheck.substring(i, i + 1).hashCode() == 55296) res = true;
            if (!res) {
                log.println("-- " + toCheck.substring(i, i + 1).hashCode());
                log.println("Code: " + Integer.toHexString(charsInt[i]));
                log.println("Gained: " + unicode_script[get]);
                log.println("Expected: " + unicode_script[i]);
            }
        }
        tRes.tested("getScript()", res);
    }

    /**
    * After defining a string to be parsed and parse conditions (flags), test
    * calls the method for different locales three times with different parameters,
    * checking result after every call.  <p>
    * Has <b> OK </b> status if the method returns right results all three
    * times.
    */
    public void _parseAnyToken() {
        int nStartFlags = KParseTokens.ANY_ALPHA | KParseTokens.ASC_UNDERSCORE;
        int nContFlags = KParseTokens.ANY_ALNUM | KParseTokens.ASC_UNDERSCORE
                        | KParseTokens.ASC_DOT;
        String toCheck = " 18 i18n ^";
        ParseResult pRes = null;
        boolean res = true;

        for (int i=0;i<7;i++) {
            pRes = oObj.parseAnyToken(toCheck, 1, getLocale(i),
                nStartFlags, "", nContFlags, "");
            res = ( (pRes.CharLen==2)
                 && (pRes.TokenType==32)
                 && (pRes.Value==18.0) );
            pRes = oObj.parseAnyToken(toCheck, 4, getLocale(i),
                nStartFlags, "", nContFlags, "");
            res &= ( (pRes.CharLen==4)
                  && (pRes.TokenType==4)
                  && (pRes.Value==0.0) );
            pRes = oObj.parseAnyToken(toCheck, 9, getLocale(i),
                nStartFlags, "", nContFlags, "");
            res &= ( (pRes.CharLen==1)
                  && (pRes.TokenType==1)
                  && (pRes.Value==0.0) );
        }
        tRes.tested("parseAnyToken()", res);
    }

    /**
    * After defining a string to be parsed and parse conditions (flags), test
    * calls the method for different locales two times with different parameters,
    * checking result after every call. <p>
    * Has <b> OK </b> status if the method returns right results.
    */
    public void _parsePredefinedToken() {
        int nStartFlags = KParseTokens.ANY_ALPHA | KParseTokens.ASC_UNDERSCORE;
        int nContFlags = nStartFlags;
        String toCheck = " 18 int";
        ParseResult pRes = null;
        boolean res = true;

        for (int i=0;i<7;i++) {
            pRes = oObj.parsePredefinedToken(KParseType.IDENTNAME, toCheck,
                1, getLocale(i), nStartFlags, "", nContFlags, "");
            res = (pRes.CharLen==0);
            pRes = oObj.parsePredefinedToken(KParseType.IDENTNAME, toCheck,
                4, getLocale(i), nStartFlags, "", nContFlags, "");
            res &= ( (pRes.CharLen==3)
                  && (pRes.TokenType==4)
                  && (pRes.Value==0.0) );
        }
        tRes.tested("parsePredefinedToken()", res);
    }


    /**
    * Method returns locale for a given language and country.
    * @param k index of needed locale.
    */
    private Locale getLocale(int k) {
        return new Locale(languages[k],countries[k],"");
    }


} // end XCharacterClassification


/*************************************************************************
 *
 *  $RCSfile: _XExtendedIndexEntrySupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-07-23 10:45:23 $
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

import com.sun.star.i18n.CollatorOptions;
import com.sun.star.i18n.XExtendedIndexEntrySupplier;
import com.sun.star.lang.Locale;

import java.util.HashMap;
import java.util.Vector;

import lib.MultiMethodTest;


public class _XExtendedIndexEntrySupplier extends MultiMethodTest {
    public XExtendedIndexEntrySupplier oObj;
    protected Locale[] locales = null;
    protected HashMap algorithms = new HashMap();

    public void _compareIndexEntry() {
        requiredMethod("getIndexKey()");
        Locale locale = new Locale("zh", "CN", "");
        String val1 = new String(new char[]{UnicodeStringPair.getUnicodeValue(0), UnicodeStringPair.getUnicodeValue(1)});
        String val2 = new String(new char[]{UnicodeStringPair.getUnicodeValue(1), UnicodeStringPair.getUnicodeValue(0)});
        short result1 = oObj.compareIndexEntry(val1, "", locale, val1, "", locale);
        short result2 = oObj.compareIndexEntry(val1, "", locale, val2, "", locale);
        short result3 = oObj.compareIndexEntry(val2, "", locale, val1, "", locale);

        tRes.tested("compareIndexEntry()", result1 == 0 && result2 + result3 == 0);
    }

    /*
     * gets the list of all algorithms for each listed language
     * is OK if everyone of the returned lists are filled
     */
    public void _getAlgorithmList() {
        requiredMethod("getLocaleList()");

        boolean result = true;
        boolean locResult = false;

        for (int i = 0; i < locales.length; i++) {
            String[] algNames = oObj.getAlgorithmList(locales[i]);
            algorithms.put(new Integer(i), algNames);

            locResult = algNames != null && algNames.length > 0;
            System.out.println("Locale " + i + ": " + locales[i].Country+","+locales[i].Language);

            for (int j=0; j<algNames.length; j++) {
                System.out.println("\tAlgorithm " + j + ": " + algNames[j]);
            }

            if (!locResult) {
                log.println("No Algorithm found for " + locales[i].Country +
                            "," + locales[i].Language);
            }

            result &= locResult;
        }

        tRes.tested("getAlgorithmList()", result);
    }

    public void _getIndexKey() {
        requiredMethod("loadAlgorithm()");
        char[] characters = new char[] { 19968 };
        String getIndexFor = new String(characters);
        for (int i = 0; i < locales.length; i++) {
            log.println("Language: " + locales[i].Language);

            for (int j = 0; j < algorithms.size(); j++) {
                String[] algs = (String[])algorithms.get(new Integer(j));
                for (int k=0;k<algs.length;k++) {
                log.println("\t Algorythm :" +
                            algs[k]);
                oObj.loadAlgorithm(locales[i], algs[k], CollatorOptions.CollatorOptions_IGNORE_CASE);
                log.println("\t\t Get: " +
                            oObj.getIndexKey(getIndexFor, "", locales[i]));
                }
            }
        }
        tRes.tested("getIndexKey()", true);
    }

    /*
     * gets a list of all locales, is OK if this list isn't empty
     */
    public void _getLocaleList() {
        locales = oObj.getLocaleList();
        tRes.tested("getLocaleList()", locales.length > 0);
    }

    /*
     * gets one phonetic canidate for the chinese local
     * is ok if 'yi' is returned as expected.
     */
    public void _getPhoneticCandidate() {
        requiredMethod("getLocaleList()");

        boolean res = true;

        Locale loc = new Locale("zh", "CN", "");

        for (int i = 0;i<UnicodeStringPair.getValCount();i++) {

            char[] c = new char[]{UnicodeStringPair.getUnicodeValue(i)};

            String getting = oObj.getPhoneticCandidate(new String(c), loc);

            boolean locResult = getting.equals(UnicodeStringPair.getExpectedPhoneticString(i));

            if (!locResult) {
                log.println("Char: "+ c[0] + " (" + (int)c[0] + ")");
                log.println("Expected " + UnicodeStringPair.getExpectedPhoneticString(i));
                log.println("Getting " + getting);
            }

            res &= locResult;
       }
        tRes.tested("getPhoneticCandidate()", res);
    }

    /*
     * loads all algorithms available in all language.
     * Is OK if no exception occurs and the method returns
     * true for each valid algorithm and false otherwise
     */
    public void _loadAlgorithm() {
        requiredMethod("getAlgorithmList()");

        boolean res = true;

        for (int i = 0; i < algorithms.size(); i++) {
            String[] names = (String[]) algorithms.get(new Integer(i));
            log.println("loading algorithms for " + locales[i].Country +
                        "," + locales[i].Language);

            for (int j = 0; j < names.length; j++) {
                log.println("\t Loading " + names[j]);

                boolean localres = oObj.loadAlgorithm(locales[i], names[j],
                                                      CollatorOptions.CollatorOptions_IGNORE_CASE);

                if (!localres) {
                    log.println("\t ... didn't work - FAILED");
                } else {
                    log.println("\t ... worked - OK");
                }

                res &= localres;
            }

/*            log.println("\tTrying to load 'dummy' algorithm");

            boolean localres = !oObj.loadAlgorithm(locales[i], "dummy",
                                                   CollatorOptions.CollatorOptions_IGNORE_WIDTH);

            if (!localres) {
                log.println("\t ... didn't work as expected - FAILED");
            } else {
                log.println("\t ... worked - OK");
            }

            res &= localres;*/
        }

        tRes.tested("loadAlgorithm()", res);
    }

    /*
     * checks the method usePhoneticEntry(). Only the languages ja, ko and zh
     * should return true. Has OK state if exactly this is the case.
     */
    public void _usePhoneticEntry() {
        requiredMethod("getLocaleList()");

        boolean res = true;

        for (int i = 0; i < locales.length; i++) {
            boolean expected = false;

            if (locales[i].Language.equals("ja") ||
                    locales[i].Language.equals("ko") ||
                    locales[i].Language.equals("zh")) {
                expected = true;
            }

            boolean locResult = oObj.usePhoneticEntry(locales[i]) == expected;

            if (!locResult) {
                log.println("Failure for language " + locales[i].Language);
                log.println("Expected " + expected);
                log.println("Getting " + oObj.usePhoneticEntry(locales[i]));
            }

            res &= locResult;
        }

        tRes.tested("usePhoneticEntry()", res);
    }

    /**
     * Helper class to handle the phonetic equivalence of unicode characters
     * This class delivers an amount oif unicode characters and the equivalent phonetics
     * for the "getPhoneticCandidate" test. Euivalents are only usable for zh,CN locale.
     */
    public static class UnicodeStringPair {
        final static int valCount = 78;
        static String[] sStringEquivalence = null;
        static char[] iUnicodeEquivalence = null;

        static {
            sStringEquivalence = new String[valCount];
            iUnicodeEquivalence = new char[valCount];
            fillValues();
        }

        public static int getValCount() {
            return valCount;
        }

        public static String getExpectedPhoneticString(int index) {
            if (index >= valCount) return null;
            return sStringEquivalence[index];
        }

        public static char getUnicodeValue(int index) {
            if (index > valCount) return 0;
            return iUnicodeEquivalence[index];
        }

        private static void fillValues() {
            iUnicodeEquivalence[0] = 20049; sStringEquivalence[0] = "zhong";
            iUnicodeEquivalence[1] = 19968; sStringEquivalence[1] = "yi";
            iUnicodeEquivalence[2] = 19969; sStringEquivalence[2] = "ding";
            iUnicodeEquivalence[3] = 19970; sStringEquivalence[3] = "kao";
            iUnicodeEquivalence[4] = 19971; sStringEquivalence[4] = "qi";
            iUnicodeEquivalence[5] = 19972; sStringEquivalence[5] = "shang";
            iUnicodeEquivalence[6] = 19973; sStringEquivalence[6] = "xia";
            iUnicodeEquivalence[7] = 19975; sStringEquivalence[7] = "wan";
            iUnicodeEquivalence[8] = 19976; sStringEquivalence[8] = "zhang";
            iUnicodeEquivalence[9] = 19977; sStringEquivalence[9] = "san";
            iUnicodeEquivalence[10] = 19978; sStringEquivalence[10] = "shang";
            iUnicodeEquivalence[11] = 19979; sStringEquivalence[11] = "xia";
            iUnicodeEquivalence[12] = 19980; sStringEquivalence[12] = "ji";
            iUnicodeEquivalence[13] = 19981; sStringEquivalence[13] = "bu";
            iUnicodeEquivalence[14] = 19982; sStringEquivalence[14] = "yu";
            iUnicodeEquivalence[15] = 19983; sStringEquivalence[15] = "mian";
            iUnicodeEquivalence[16] = 19984; sStringEquivalence[16] = "gai";
            iUnicodeEquivalence[17] = 19985; sStringEquivalence[17] = "chou";
            iUnicodeEquivalence[18] = 19986; sStringEquivalence[18] = "chou";
            iUnicodeEquivalence[19] = 19987; sStringEquivalence[19] = "zhuan";
            iUnicodeEquivalence[20] = 19988; sStringEquivalence[20] = "qie";
            iUnicodeEquivalence[21] = 19989; sStringEquivalence[21] = "pi";
            iUnicodeEquivalence[22] = 19990; sStringEquivalence[22] = "shi";
            iUnicodeEquivalence[23] = 19991; sStringEquivalence[23] = "shi";
            iUnicodeEquivalence[24] = 19992; sStringEquivalence[24] = "qiu";
            iUnicodeEquivalence[25] = 19993; sStringEquivalence[25] = "bing";
            iUnicodeEquivalence[26] = 19994; sStringEquivalence[26] = "ye";
            iUnicodeEquivalence[27] = 19995; sStringEquivalence[27] = "cong";
            iUnicodeEquivalence[28] = 19996; sStringEquivalence[28] = "dong";
            iUnicodeEquivalence[29] = 19997; sStringEquivalence[29] = "si";
            iUnicodeEquivalence[30] = 19998; sStringEquivalence[30] = "cheng";
            iUnicodeEquivalence[31] = 19999; sStringEquivalence[31] = "diu";
            iUnicodeEquivalence[32] = 20000; sStringEquivalence[32] = "qiu";
            iUnicodeEquivalence[33] = 20001; sStringEquivalence[33] = "liang";
            iUnicodeEquivalence[34] = 20002; sStringEquivalence[34] = "diu";
            iUnicodeEquivalence[35] = 20003; sStringEquivalence[35] = "you";
            iUnicodeEquivalence[36] = 20004; sStringEquivalence[36] = "liang";
            iUnicodeEquivalence[37] = 20005; sStringEquivalence[37] = "yan";
            iUnicodeEquivalence[38] = 20006; sStringEquivalence[38] = "bing";
            iUnicodeEquivalence[39] = 20007; sStringEquivalence[39] = "sang";
            iUnicodeEquivalence[40] = 20008; sStringEquivalence[40] = "shu";
            iUnicodeEquivalence[41] = 20009; sStringEquivalence[41] = "jiu";
            iUnicodeEquivalence[42] = 20010; sStringEquivalence[42] = "ge";
            iUnicodeEquivalence[43] = 20011; sStringEquivalence[43] = "ya";
            iUnicodeEquivalence[44] = 20012; sStringEquivalence[44] = "qiang";
            iUnicodeEquivalence[45] = 20013; sStringEquivalence[45] = "zhong";
            iUnicodeEquivalence[46] = 20014; sStringEquivalence[46] = "ji";
            iUnicodeEquivalence[47] = 20015; sStringEquivalence[47] = "jie";
            iUnicodeEquivalence[48] = 20016; sStringEquivalence[48] = "feng";
            iUnicodeEquivalence[49] = 20017; sStringEquivalence[49] = "guan";
            iUnicodeEquivalence[50] = 20018; sStringEquivalence[50] = "chuan";
            iUnicodeEquivalence[51] = 20019; sStringEquivalence[51] = "chan";
            iUnicodeEquivalence[52] = 20020; sStringEquivalence[52] = "lin";
            iUnicodeEquivalence[53] = 20021; sStringEquivalence[53] = "zhuo";
            iUnicodeEquivalence[54] = 20022; sStringEquivalence[54] = "zhu";
            iUnicodeEquivalence[55] = 20024; sStringEquivalence[55] = "wan";
            iUnicodeEquivalence[56] = 20025; sStringEquivalence[56] = "dan";
            iUnicodeEquivalence[57] = 20026; sStringEquivalence[57] = "wei";
            iUnicodeEquivalence[58] = 20027; sStringEquivalence[58] = "zhu";
            iUnicodeEquivalence[59] = 20028; sStringEquivalence[59] = "jing";
            iUnicodeEquivalence[60] = 20029; sStringEquivalence[60] = "li";
            iUnicodeEquivalence[61] = 20030; sStringEquivalence[61] = "ju";
            iUnicodeEquivalence[62] = 20031; sStringEquivalence[62] = "pie";
            iUnicodeEquivalence[63] = 20032; sStringEquivalence[63] = "fu";
            iUnicodeEquivalence[64] = 20033; sStringEquivalence[64] = "yi";
            iUnicodeEquivalence[65] = 20034; sStringEquivalence[65] = "yi";
            iUnicodeEquivalence[66] = 20035; sStringEquivalence[66] = "nai";
            iUnicodeEquivalence[67] = 20037; sStringEquivalence[67] = "jiu";
            iUnicodeEquivalence[68] = 20038; sStringEquivalence[68] = "jiu";
            iUnicodeEquivalence[69] = 20039; sStringEquivalence[69] = "tuo";
            iUnicodeEquivalence[70] = 20040; sStringEquivalence[70] = "me";
            iUnicodeEquivalence[71] = 20041; sStringEquivalence[71] = "yi";
            iUnicodeEquivalence[72] = 20043; sStringEquivalence[72] = "zhi";
            iUnicodeEquivalence[73] = 20044; sStringEquivalence[73] = "wu";
            iUnicodeEquivalence[74] = 20045; sStringEquivalence[74] = "zha";
            iUnicodeEquivalence[75] = 20046; sStringEquivalence[75] = "hu";
            iUnicodeEquivalence[76] = 20047; sStringEquivalence[76] = "fa";
            iUnicodeEquivalence[77] = 20048; sStringEquivalence[77] = "le";
        }
    }
}

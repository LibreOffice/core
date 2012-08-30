/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package testlib.gui;

import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.UIMap.*;

import java.lang.reflect.Array;
import java.util.StringTokenizer;
import java.util.logging.Logger;

public class CalcUtil {

    private static Logger LOG = Logger.getLogger(CalcUtil.class.getName());

    /**
     * Select a range.
     *
     * @param range
     *            e.g. Sheet1.A10, Sheet1.A11:B30, A11, A30:B45
     */
    public static void selectRange(String range) {
        SC_InputBar_Position.click(0.5, 0.5);
        SC_InputBar_Position.setText(range);
        typeKeys("<enter>");
        sleep(1);
    }

    /**
     * Get the input at the given cell. If the cell is a formula, return the
     * formula rather than the result
     *
     * @param cell
     * @return
     */
    public static String getCellInput(String cell) {
        if (cell != null)
            selectRange(cell);
        String caption = SC_InputBar_Input.getCaption();
        // Fix: When formulaEdit's caption is blank, the hook will return the
        // document window's caption
        // So if it's document window caption, return ""
        return caption.contains("[12479]") ? "" : caption;
    }

    /**
     * Get the text at the given cell. If the cell is a formula, return the
     * result rather than the formula
     *
     * @param cell
     * @return
     */
    public static String getCellText(String cell) {
        app.setClipboard("$$$$");
        selectRange(cell);
        typeKeys("<$copy>");
        String content = app.getClipboard();
        if (content.endsWith("\r\n"))
            content = content.substring(0, content.length() - 2);
        else if (content.endsWith("\n"))
            content = content.substring(0, content.length() - 1);

        return content;
    }

    /**
     * convert the format of column number to integer e.g. A -> 1 AA -> 27 AMJ
     * -> 1024
     *
     * @param no
     * @return
     */
    public static int toIntColumnNo(String no) {
        int len = no.length();
        int ret = 0;
        for (int i = 0; i < len; i++) {
            char c = no.charAt(len - i - 1);
            ret += Math.pow(26, i) * (c - 'A' + 1);
        }

        return ret;
    }

    /**
     * Convert the format of column number to char
     *
     * @param no
     * @return
     */
    public static String toCharColumnNo(int no) {
        String ret = "";
        int f = 0;
        do {
            f = (no - 1) / 26;
            int s = (no - 1) % 26;
            ret = (char) ('A' + s) + ret;
            no = f;
        } while (f != 0);
        return ret;
    }

    /**
     * Parse location string into integer values
     *
     * @param loc
     *            e.g. A1
     * @return
     */
    public static int[] parseLocation(String loc) {
        int i = 0;
        for (; i < loc.length(); i++) {
            char c = loc.charAt(i);
            if (c >= '0' && c <= '9')
                break;
        }

        String col = loc.substring(0, i);
        String row = loc.substring(i);
        int sC = toIntColumnNo(col);
        int sR = Integer.parseInt(row);
        return new int[] { sC, sR };
    }

    /**
     * Parse range string into integer values
     *
     * @param range
     *            e.g. A3:F9
     * @return
     */
    public static int[] parseRange(String range) {
        int dotIndex = range.indexOf(".");
        if (dotIndex != -1) {
            range = range.substring(dotIndex + 1);
        }

        String[] locs = range.split(":");
        int[] ret = new int[4];
        int[] start = parseLocation(locs[0]);
        ret[0] = start[0];
        ret[1] = start[1];
        if (locs.length == 1) {
            ret[2] = start[0];
            ret[3] = start[1];
        } else {
            int[] end = parseLocation(locs[1]);
            ret[2] = end[0];
            ret[3] = end[1];
        }

        return ret;
    }

    /**
     * Get the text at the given cells. If the cell is a formula, return the
     * result rather than the formula. Note:
     *
     * @param range
     *            e.g. A3:D9
     * @return
     */
    public static String[][] getCellTexts(String range) {
        selectRange(range);
        int[] intRange = parseRange(range);
        int rowCount = intRange[3] - intRange[1] + 1;
        int colCount = intRange[2] - intRange[0] + 1;
        String[][] texts = new String[rowCount][colCount];

        app.setClipboard("$$$$");
        typeKeys("<$copy>");
        sleep(1);
        String text = app.getClipboard();
        StringTokenizer tokenizer = new StringTokenizer(text, "\"\t\n", true);
        int state = 0;
        String cellContent = "";
        int r = 0, c = 0;
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            switch (state) {
            case 0:
                if ("\"".equals(token)) {
                    state = 1;
                } else if ("\t".equals(token)) {
                    texts[r][c] = cellContent;
                    cellContent = "";
                    c++;
                } else if ("\n".equals(token)) {
                    if (cellContent.endsWith("\r"))
                        cellContent = cellContent.substring(0, cellContent.length() - 1);
                    texts[r][c] = cellContent;
                    cellContent = "";
                    c = 0;
                    r++;
                } else {
                    cellContent += token;
                }
                break;
            case 1:
                if ("\"".equals(token)) {
                    state = 0;
                } else {
                    cellContent += token;
                }
                break;
            }
        }

        LOG.info("Text of range [" + range + "]:\n" + arrayToString(texts));
        return texts;
    }

    private static String arrayToString(Object array) {
        if (array == null)
            return "null";
        if (!array.getClass().isArray())
            return array.toString();

        int len = Array.getLength(array);
        String ret = "{";
        for (int i = 0; i < len; i++) {
            Object el = Array.get(array, i);
            if (el == null) {
                ret += "null";
            } else if (el.getClass().isArray()) {
                ret += arrayToString(el);
            } else {
                ret += "\"" + el + "\"";
            }

            if (i + 1 != len)
                ret += ",";

        }
        ret += "}";
        return ret;
    }
}
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


package testlib.uno;

import java.util.Random;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.uno.UnoRuntime;

import testlib.uno.CellInfo;


/**
 * Utilities for UNO automation testing
 *
 */

public class TestUtil {

    private static int colLimit = 1024;
    private static int rowLimit = 1048576;
    private static Random random = new Random();

    public TestUtil() {

    }

    /**
     * Generate a random cell index
     * @return cellIndex    column: cellIndex[0]  row: cellIndex[1]
     * @throws Exception
     */
    public static CellInfo randCell() throws Exception {
        CellInfo cInfo = new CellInfo();

        cInfo.setCol(random.nextInt(colLimit));
        cInfo.setRow(random.nextInt(rowLimit));

        return cInfo;
    }

    /**
     * Generate a random cell index, in the limited range
     * @param colTop  The max column limit
     * @param rowTop  The max row limit
     * @return
     * @throws Exception
     */
    public static CellInfo randCell(int colTop, int rowTop) throws Exception {
        CellInfo cInfo = new CellInfo();

        cInfo.setCol(random.nextInt(colTop));
        cInfo.setRow(random.nextInt(rowTop));

        return cInfo;
    }

    /**
     * Generate a font size number in limited range
     * @param max  The font size in Excel2003 is [1,409]
     * @return
     * @throws Exception
     */
    public static double randFontSize(int max) throws Exception {
        double basic = random.nextInt(max * 2);
        double size = 1;
        if (basic < 2) {
            size = 1;
        }
        else {
            size = basic / 2;
        }

        return size;
    }

    /**
     * Generate a series of font size number
     * @param listSize
     * @param max
     * @return
     * @throws Exception
     */
    public static double[] randFontSizeList(int listSize, int max) throws Exception {
        double[] sizeList = new double[listSize];
        for (int i =0; i < listSize; i++) {
            sizeList[i] = randFontSize(max);
        }
        return sizeList;
    }

    /**
     * Generate a random decimal RGB color number
     * @return
     * @throws Exception
     */
    public static int randColor() throws Exception {
        int r = random.nextInt(256);
        int g = random.nextInt(256);
        int b = random.nextInt(256);

        return r * 65536 + g * 256 + b;
    }

    /**
     * Generate a random decimal RGB color number in limited color space
     * @param rMax  The R value limit, get a value in [0, rMax]
     * @param gMax  The G value limit, get a value in [0, gMax]
     * @param bMax  The B value limit, get a value in [0, bMax]
     * @return
     * @throws Exception
     */
    public static int randColor(int rMax, int gMax, int bMax) throws Exception {
        int r = random.nextInt(rMax + 1) % 256;
        int g = random.nextInt(gMax + 1) % 256;
        int b = random.nextInt(bMax + 1) % 256;

        return r * 65536 + g * 256 + b;
    }

    /**
     * Generate a series of decimal RGB color number
     * @param size Set the quantity of random color value generated into the array
     * @return
     * @throws Exception
     */
    public static int[] randColorList(int size) throws Exception {
        int[] colorList = new int[size];
        for (int i = 0; i < size; i++) {
            colorList[i] = randColor();
        }

        return colorList;
    }

    /**
     * Add "=" before a string
     * @param expression
     * @return
     */
    public static String toFormula(String expression) {
        return "=" + expression;
    }

    /**
     * Use specific operator to connect a series of number
     * @param number
     * @param operator
     * @return
     */
    public static String connectByOperator(double[] number, String operator) throws Exception{
        StringBuffer buffer = new StringBuffer();

        for (int i = 0; i < number.length; i++) {
            buffer.append(number[i]);
            if (i < number.length - 1) {
                buffer.append(operator);
            }
        }
        return buffer.toString();
    }

    /**
     * Print the properties list of specific object to console
     * @param obj   The instance of the object of which the property list you want to get. e.g. instance of XCell.
     * @throws Exception
     */
    public static void printPropertiesList(Object obj) throws Exception {
        // Get the property set of specific object
        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, obj);
        XPropertySetInfo xPropertySetInfo = xPropertySet.getPropertySetInfo();

        // Get all properties info
        Property[] aProps = xPropertySetInfo.getProperties();

        for (int i = 0; i < aProps.length; i++) {
            // Print name and type of each property
            System.out.print("[" + (i + 1) + "]: Name=\"" + aProps[i].Name + "\" " + aProps[i].Type.toString() + " (");

            // Get flag. pay attention to the READONLY properties
            short nAttribs = aProps[i].Attributes;
            if ((nAttribs & PropertyAttribute.MAYBEVOID) != 0)
                System.out.print("MAYBEVOID|");
            if ((nAttribs & PropertyAttribute.BOUND) != 0)
                System.out.print("BOUND|");
            if ((nAttribs & PropertyAttribute.CONSTRAINED) != 0)
                System.out.print("CONSTRAINED|");
            if ((nAttribs & PropertyAttribute.READONLY) != 0)
                System.out.print("READONLY|");
            if ((nAttribs & PropertyAttribute.TRANSIENT) != 0)
                System.out.print("TRANSIENT|");
            if ((nAttribs & PropertyAttribute.MAYBEAMBIGUOUS ) != 0)
                System.out.print("MAYBEAMBIGUOUS|");
            if ((nAttribs & PropertyAttribute.MAYBEDEFAULT) != 0)
                System.out.print("MAYBEDEFAULT|");
            if ((nAttribs & PropertyAttribute.REMOVEABLE) != 0)
                System.out.print("REMOVEABLE|");

            System.out.println(")");
        }

    }

}

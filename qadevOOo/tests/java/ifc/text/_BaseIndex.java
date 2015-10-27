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
package ifc.text;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexReplace;
import com.sun.star.text.XTextColumns;
import com.sun.star.uno.UnoRuntime;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.text.BaseIndex</code>
* service properties :
* <ul>
*  <li><code> Title</code></li>
*  <li><code> IsProtected</code></li>
*  <li><code> ParaStyleHeading</code></li>
*  <li><code> ParaStyleLevel1</code></li>
*  <li><code> ParaStyleLevel2</code></li>
*  <li><code> ParaStyleLevel3</code></li>
*  <li><code> ParaStyleLevel4</code></li>
*  <li><code> ParaStyleLevel5</code></li>
*  <li><code> ParaStyleLevel6</code></li>
*  <li><code> ParaStyleLevel7</code></li>
*  <li><code> ParaStyleLevel8</code></li>
*  <li><code> ParaStyleLevel9</code></li>
*  <li><code> ParaStyleLevel10</code></li>
*  <li><code> ParaStyleSeparator</code></li>
*  <li><code> TextColumns</code></li>
*  <li><code> BackGraphicURL</code></li>
*  <li><code> BackGraphicFilter</code></li>
*  <li><code> BackGraphicLocation</code></li>
*  <li><code> BackTransparent</code></li>
*  <li><code> LevelFormat</code></li>
*  <li><code> CreateFromChapter</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.BaseIndex
*/
public class _BaseIndex extends MultiPropertyTest {
    /**
    * Redefined method returns object, that contains changed property value.
    */
    protected PropertyTester CustomTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            XTextColumns TC = UnoRuntime.queryInterface(
                                      XTextColumns.class, oldValue);
            TC.setColumnCount((short) (TC.getColumnCount() + (short) 1));

            return TC;
        }
    };

    /**
    * New value must be defined for this property.
    */
    public void _TextColumns() {
        log.println(
                "Testing property 'TextColumns' with custom property tester");
        testProperty("TextColumns", CustomTester);
    }

    /**
     * The value of this property is a collection of document index
     * level formats. This property is tested in the following manner :
     * the property value (a collection) is obtained, the first element
     * of this collection is replaced with new non-empty array
     * (<code>PropertyValue[][]</code>) with some properties set.
     * After that the collection is set back as property value. <p>
     *
     * Comparing of set and got <code>PropertyValue</code> arrays
     * is difficult because values can be changed after setting
     * by service miplementation. <p>
     *
     * Has <b>OK</b> status if the collection again gotten, has a
     * new first element (i.e. lengths of the old array and the array
     * get are different or their contents differ).
     */
    public void _LevelFormat() {
        log.println(
                "Testing property 'LevelFormat' with custom property tester");
        testProperty("LevelFormat",
                     new PropertyTester() {
            PropertyValue[][] newVal = null;
            PropertyValue[][] oldVal = null;

            @Override
            protected Object getNewValue(String propName, Object oldValue) {
                XIndexReplace indProp = UnoRuntime.queryInterface(
                                                XIndexReplace.class, oldValue);

                try {
                    oldVal = (PropertyValue[][]) indProp.getByIndex(0);

                    log.println("Get:");
                    printLevelFormatProperty(oldValue);

                    newVal = new PropertyValue[1][2];

                    for (int i = 0; i < newVal[0].length; i++) {
                        newVal[0][i] = new PropertyValue();
                    }

                    newVal[0][1].Name = "TokenType";
                    newVal[0][1].Value = "TokenEntryText";
                    newVal[0][0].Name = "Text";
                    newVal[0][0].Value = "BaseIndex";

                    indProp.replaceByIndex(0, newVal);
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    log.println("Exception occurred while testing LevelFormat");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Exception occurred while testing LevelFormat");
                    e.printStackTrace(log);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Exception occurred while testing LevelFormat");
                    e.printStackTrace(log);
                }

                return indProp;
            }

            @Override
            protected void checkResult(String propName, Object oldValue,
                                       Object newValue, Object resValue,
                                       Exception exception)
                                throws Exception {

                super.checkResult(propName, oldValue, newValue, resValue, exception);

                PropertyValue[][] res = (PropertyValue[][]) UnoRuntime.queryInterface(
                                                                     XIndexAccess.class,
                                                                     resValue).getByIndex(0);

                log.println("Result:");
                printLevelFormatProperty(resValue);

                boolean result = (res.length != oldVal.length) ||
                                 !util.ValueComparer.equalValue(res, oldVal);

                tRes.tested(propName, result);
            }
        });
    }

    /**
     * Outputs full description of 'LevelFormat' property
     * value into <code>log</code>.
     */
    private void printLevelFormatProperty(Object value) {
        XIndexReplace indProp = UnoRuntime.queryInterface(
                                        XIndexReplace.class, value);
        PropertyValue[][] val = null;

        try {
            log.println(" \u0421ollection has " + indProp.getCount() +
                        " elements : ");

            for (int i = 0; i < indProp.getCount(); i++) {
                val = (PropertyValue[][]) indProp.getByIndex(i);

                log.println("  " + i + ": has " + val.length + " levels :");

                for (int j = 0; j < val.length; j++) {
                    log.println("    " + j + " level :");

                    for (int k = 0; k < val[j].length; k++) {
                        log.println("      " + val[j][k].Name + "=" +
                                    val[j][k].Value);
                    }
                }
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred while printing LevelFormat");
            e.printStackTrace(log);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while printing LevelFormat");
            e.printStackTrace(log);
        }
    }
} // finish class _NumberingRules

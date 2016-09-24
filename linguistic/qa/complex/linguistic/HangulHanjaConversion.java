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
package complex.linguistic;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.i18n.TextConversionOption;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.linguistic2.ConversionDictionaryType;
import com.sun.star.linguistic2.ConversionDirection;
import com.sun.star.linguistic2.XConversionDictionary;
import com.sun.star.linguistic2.XConversionDictionaryList;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;

import com.sun.star.uno.UnoRuntime;



import util.DesktopTools;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

public class HangulHanjaConversion {
    XMultiServiceFactory xMSF = null;
    boolean disposed = false;
    Locale aLocale = new Locale("ko", "KR", "");
    short dictType = ConversionDictionaryType.HANGUL_HANJA;

    @Before public void before() {
        xMSF = getMSF();
    }

    @Test public void ConversionDictionaryList() {
        Object ConversionDictionaryList = null;

        try {
            ConversionDictionaryList = xMSF.createInstance(
                                               "com.sun.star.linguistic2.ConversionDictionaryList");
        } catch (com.sun.star.uno.Exception e) {
            fail("Couldn't create ConversionDictionaryList");
        }

        if (ConversionDictionaryList == null) {
            fail("Couldn't create ConversionDictionaryList");
        }

        boolean bList = checkXConversionDictionaryList(
                                ConversionDictionaryList);
        assertTrue("XConversionDictionaryList doesn't work as expected", bList);
    }

    private boolean checkXConversionDictionaryList(Object list) {
        boolean res = true;
        XConversionDictionaryList xCList = UnoRuntime.queryInterface(XConversionDictionaryList.class, list);
        XConversionDictionary xDict = null;

        try {
            xDict = xCList.addNewDictionary("addNewDictionary", aLocale,
                                            dictType);
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            fail("Couldn't add Dictionary");
        } catch (com.sun.star.container.ElementExistException e) {
            res = false;
            fail("Couldn't add Dictionary");
        }

        try {
            xCList.addNewDictionary("addNewDictionary", aLocale, dictType);
            res = false;
            fail("wrong exception while adding Dictionary again");
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            fail("wrong exception while adding Dictionary again");
        } catch (com.sun.star.container.ElementExistException e) {
        }

        boolean localRes = checkNameContainer(xCList.getDictionaryContainer());
        res &= localRes;
        assertTrue("getDictionaryContainer didn't work as expected", localRes);

        String FileToLoad = TestDocument.getUrl("hangulhanja.sxc");

XComponent xDoc = DesktopTools.loadDoc(xMSF, FileToLoad,
                                               new PropertyValue[] {  });
        XSpreadsheet xSheet = getSheet(xDoc);
        boolean done = false;
        int counter = 0;
        String wordToCheck = "";
        String expectedConversion = "";

        while (!done) {
            String[] HangulHanja = getLeftAndRight(counter, xSheet);
            done = HangulHanja[0].equals("");
            counter++;

            if (!done) {
                try {
                    xDict.addEntry(HangulHanja[0], HangulHanja[1]);
                    wordToCheck += HangulHanja[0];
                    expectedConversion += HangulHanja[1];
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace();
                    res = false;
                    fail("Exception while checking adding entry");
                } catch (com.sun.star.container.ElementExistException e) {
                    //ignored
                }
            }
        }

        try {
            xDict.addEntry(wordToCheck, expectedConversion);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace();
            res = false;
            fail("Exception while checking adding entry");
        } catch (com.sun.star.container.ElementExistException e) {
            //ignored
        }

        localRes = xCList.queryMaxCharCount(aLocale, dictType,
                                            ConversionDirection.FROM_LEFT) == 42;
        res &= localRes;
        assertTrue("queryMaxCharCount returned the wrong value", localRes);

        String[] conversion = null;

        try {
            conversion = xCList.queryConversions(wordToCheck, 0,
                                                 wordToCheck.length(), aLocale,
                                                 dictType,
                                                 ConversionDirection.FROM_LEFT,
                                                 TextConversionOption.NONE);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            res = false;
            fail("Exception while calling queryConversions");
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            fail("Exception while calling queryConversions");
        }

        localRes = conversion[0].equals(expectedConversion);
        res &= localRes;
        assertTrue("queryConversions didn't work as expected", localRes);

        try {
            xCList.getDictionaryContainer().removeByName("addNewDictionary");
        } catch (com.sun.star.container.NoSuchElementException e) {
            res = false;
            fail("exception while removing Dictionary again");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            res = false;
            fail("exception while removing Dictionary again");
        }

        localRes = !xCList.getDictionaryContainer()
                          .hasByName("addNewDictionary");
        res &= localRes;
        assertTrue("Dictionary hasn't been removed properly", localRes);

        XComponent dicList = UnoRuntime.queryInterface(XComponent.class, xCList);
        XEventListener listen = new EventListener();
        dicList.addEventListener(listen);
        dicList.dispose();
        assertTrue("dispose didn't work", disposed);
        dicList.removeEventListener(listen);

        DesktopTools.closeDoc(xDoc);

        return res;
    }

    private boolean checkNameContainer(XNameContainer xNC) {
        boolean res = true;

        try {
            res &= xNC.hasByName("addNewDictionary");

            XConversionDictionary myCD = new ConversionDictionary();
            xNC.insertByName("insertByName", myCD);
            res &= xNC.hasByName("insertByName");
            xNC.removeByName("insertByName");
            res &= !(xNC.hasByName("insertByName"));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            res = false;
            e.printStackTrace();
        } catch (com.sun.star.container.NoSuchElementException e) {
            res = false;
            e.printStackTrace();
        } catch (com.sun.star.container.ElementExistException e) {
            res = false;
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            res = false;
            e.printStackTrace();
        }

        return res;
    }

    private XSpreadsheet getSheet(XComponent xDoc) {
        XSpreadsheetDocument xSheetDoc = UnoRuntime.queryInterface(XSpreadsheetDocument.class, xDoc);
        XSpreadsheet xSheet = null;

        try {
            xSheet = UnoRuntime.queryInterface(XSpreadsheet.class, xSheetDoc.getSheets().getByName(xSheetDoc.getSheets().getElementNames()[0]));
        } catch (com.sun.star.container.NoSuchElementException e) {
            System.out.println("Couldn't get sheet");
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            System.out.println("Couldn't get sheet");
            e.printStackTrace();
        }

        return xSheet;
    }

    private String[] getLeftAndRight(int counter, XSpreadsheet xSpreadsheet) {
        String[] re = new String[] {
            getCell(0, counter, xSpreadsheet).getFormula().trim(),
            getCell(1, counter, xSpreadsheet).getFormula().trim() };

        return re;
    }

    private XCell getCell(int x, int y, XSpreadsheet xSpreadsheet) {
        XCell re = null;

        try {
            re = xSpreadsheet.getCellByPosition(x, y);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Couldn't get word");
            e.printStackTrace();
        }

        return re;
    }

    private class ConversionDictionary implements XConversionDictionary {
        boolean active = false;

        public void addEntry(String str, String str1)
                      throws IllegalArgumentException, ElementExistException {
        }

        public void clear() {
        }

        public String[] getConversionEntries(ConversionDirection conversionDirection) {
            return new String[] { "getConversionEntries" };
        }

        public short getConversionType() {
            return ConversionDictionaryType.HANGUL_HANJA;
        }

        public String[] getConversions(String str, int param, int param2,
                                       ConversionDirection conversionDirection,
                                       int param4)
                                throws IllegalArgumentException {
            return new String[] { "getConversion" };
        }

        public com.sun.star.lang.Locale getLocale() {
            return new Locale("de", "DE", "");
        }

        public short getMaxCharCount(ConversionDirection conversionDirection) {
            return (short) 2;
        }

        public String getName() {
            return "insertByName";
        }

        public boolean isActive() {
            return active;
        }

        public void removeEntry(String str, String str1)
                         throws NoSuchElementException {
        }

        public void setActive(boolean param) {
            active = param;
        }
    }

    private class EventListener implements XEventListener {
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposed = true;
        }
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}

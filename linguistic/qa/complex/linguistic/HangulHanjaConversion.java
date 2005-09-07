/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HangulHanjaConversion.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:47:35 $
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
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XWordCursor;
import com.sun.star.uno.UnoRuntime;

import complexlib.ComplexTestCase;

import java.io.PrintWriter;

import util.DesktopTools;


public class HangulHanjaConversion extends ComplexTestCase {
    XMultiServiceFactory xMSF = null;
    boolean disposed = false;
    Locale aLocale = new Locale("ko", "KR", "");
    short dictType = ConversionDictionaryType.HANGUL_HANJA;

    public String[] getTestMethodNames() {
        return new String[] { "ConversionDictionaryList" };
    }

    public void before() {
        xMSF = (XMultiServiceFactory) param.getMSF();
    }

    public void ConversionDictionaryList() {
        Object ConversionDictionaryList = null;

        try {
            ConversionDictionaryList = xMSF.createInstance(
                                               "com.sun.star.linguistic2.ConversionDictionaryList");
        } catch (com.sun.star.uno.Exception e) {
            assure("Couldn't create ConversionDictionaryList", false);
        }

        if (ConversionDictionaryList == null) {
            assure("Couldn't create ConversionDictionaryList", false);
        }

        boolean bList = checkXConversionDictionaryList(
                                ConversionDictionaryList);
        assure("XConversionDictionaryList doesnt work as expected", bList);
    }

    private boolean checkXConversionDictionaryList(Object list) {
        boolean res = true;
        XConversionDictionaryList xCList = (XConversionDictionaryList) UnoRuntime.queryInterface(
                                                   XConversionDictionaryList.class,
                                                   list);
        XConversionDictionary xDict = null;

        try {
            xDict = xCList.addNewDictionary("addNewDictionary", aLocale,
                                            dictType);
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            assure("Couldn't add Dictionary", false);
        } catch (com.sun.star.container.ElementExistException e) {
            res = false;
            assure("Couldn't add Dictionary", false);
        }

        try {
            xCList.addNewDictionary("addNewDictionary", aLocale, dictType);
            res = false;
            assure("wrong exception while adding Dictionary again", false);
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            assure("wrong exception while adding Dictionary again", false);
        } catch (com.sun.star.container.ElementExistException e) {
        }

        boolean localRes = checkNameContainer(xCList.getDictionaryContainer());
        res &= localRes;
        assure("getDictionaryContainer didn't work as expected", localRes);

        String FileToLoad = util.utils.getFullTestURL("hangulhanja.sxc");
        XComponent xDoc = DesktopTools.loadDoc(xMSF, FileToLoad,
                                               new PropertyValue[] {  });
        XSpreadsheet xSheet = getSheet(xDoc);
        boolean done = false;
        int counter = 0;
        int numberOfWords = 0;
        String wordToCheck = "";
        String expectedConversion = "";

        while (!done) {
            String[] HangulHanja = getLeftAndRight(counter, xSheet);
            done = (HangulHanja[0].equals(""));
            counter++;

            if (!done) {
                numberOfWords++;

                try {
                    xDict.addEntry(HangulHanja[0], HangulHanja[1]);
                    wordToCheck += HangulHanja[0];
                    expectedConversion += HangulHanja[1];
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    e.printStackTrace();
                    res = false;
                    assure("Exception while checking adding entry", false);
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
            assure("Exception while checking adding entry", false);
        } catch (com.sun.star.container.ElementExistException e) {
            //ignored
        }

        localRes = xCList.queryMaxCharCount(aLocale, dictType,
                                            ConversionDirection.FROM_LEFT) == 42;
        res &= localRes;
        assure("queryMaxCharCount returned the wrong value", localRes);

        String[] conversion = null;

        try {
            conversion = xCList.queryConversions(wordToCheck, 0,
                                                 wordToCheck.length(), aLocale,
                                                 dictType,
                                                 ConversionDirection.FROM_LEFT,
                                                 TextConversionOption.NONE);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            res = false;
            assure("Exception while calling queryConversions", false);
        } catch (com.sun.star.lang.NoSupportException e) {
            res = false;
            assure("Exception while calling queryConversions", false);
        }

        localRes = conversion[0].equals(expectedConversion);
        res &= localRes;
        assure("queryConversions didn't work as expected", localRes);

        try {
            xCList.getDictionaryContainer().removeByName("addNewDictionary");
        } catch (com.sun.star.container.NoSuchElementException e) {
            res = false;
            assure("exception while removing Dictionary again", false);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            res = false;
            assure("exception while removing Dictionary again", false);
        }

        localRes = !xCList.getDictionaryContainer()
                          .hasByName("addNewDictionary");
        res &= localRes;
        assure("Dictionary hasn't been removed properly", localRes);

        XComponent dicList = (XComponent) UnoRuntime.queryInterface(
                                     XComponent.class, xCList);
        XEventListener listen = new EventListener();
        dicList.addEventListener(listen);
        dicList.dispose();
        assure("dispose didn't work", disposed);
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
        XSpreadsheetDocument xSheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                                                 XSpreadsheetDocument.class,
                                                 xDoc);
        XSpreadsheet xSheet = null;

        try {
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class,
                             xSheetDoc.getSheets()
                                      .getByName(xSheetDoc.getSheets()
                                                          .getElementNames()[0]));
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Couldn't get sheet");
            e.printStackTrace((PrintWriter) log);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get sheet");
            e.printStackTrace((PrintWriter) log);
        }

        return xSheet;
    }

    private String[] getLeftAndRight(int counter, XSpreadsheet xSpreadsheet) {
        String[] re = new String[2];
        re[0] = getCell(0, counter, xSpreadsheet).getFormula().trim();
        re[1] = getCell(1, counter, xSpreadsheet).getFormula().trim();

        return re;
    }

    private XCell getCell(int x, int y, XSpreadsheet xSpreadsheet) {
        XCell re = null;

        try {
            re = xSpreadsheet.getCellByPosition(x, y);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get word");
            e.printStackTrace((PrintWriter) log);
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
}
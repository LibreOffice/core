/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.writer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNamed;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import java.math.BigInteger;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

class BookmarkHashes {
    public BigInteger m_nSetupHash;
    public BigInteger m_nInsertRandomHash;
    public BigInteger m_nDeleteRandomHash;
    public BigInteger m_nLinebreakHash;
    public BigInteger m_nOdfReloadHash;
    public BigInteger m_nMsWordReloadHash;

    public void assertExpectation(BookmarkHashes aExpectation) {
        assertEquals(aExpectation.m_nSetupHash, m_nSetupHash);
        assertEquals(aExpectation.m_nInsertRandomHash, m_nInsertRandomHash);
        assertEquals(aExpectation.m_nDeleteRandomHash, m_nDeleteRandomHash);
        assertEquals(aExpectation.m_nLinebreakHash, m_nLinebreakHash);
        assertEquals(aExpectation.m_nOdfReloadHash, m_nOdfReloadHash);
        assertEquals(aExpectation.m_nMsWordReloadHash, m_nMsWordReloadHash);
    }

    static public java.math.BigInteger getBookmarksHash(XTextDocument xDoc)
        throws com.sun.star.uno.Exception, java.security.NoSuchAlgorithmException
    {
        StringBuffer buffer = new StringBuffer("");
        XBookmarksSupplier xBookmarksSupplier = (XBookmarksSupplier)UnoRuntime.queryInterface(
            XBookmarksSupplier.class,
            xDoc);
        XNameAccess xBookmarks = xBookmarksSupplier.getBookmarks();
        for(String sBookmarkname : xBookmarks.getElementNames()) {
            Object xBookmark = xBookmarks.getByName(sBookmarkname);
            XTextContent xBookmarkAsContent = (XTextContent)UnoRuntime.queryInterface(
                XTextContent.class,
                xBookmark);
            buffer.append(sBookmarkname);
            buffer.append(":");
            buffer.append(xBookmarkAsContent.getAnchor().getString());
            buffer.append(";");
        }
        java.security.MessageDigest sha1 = java.security.MessageDigest.getInstance("SHA-1");
        sha1.reset();
        sha1.update(buffer.toString().getBytes());
        return new java.math.BigInteger(sha1.digest());
    }
}

public class CheckBookmarks {
    private XMultiServiceFactory m_xMsf = null;
    private XTextDocument m_xDoc = null;
    private XTextDocument m_xOdfReloadedDoc = null;
    private XTextDocument m_xMsWordReloadedDoc = null;
    private final BookmarkHashes actualHashes = new BookmarkHashes();

    private BookmarkHashes getDEV300m41Expectations() {
        BookmarkHashes result = new BookmarkHashes();
        result.m_nSetupHash = new BigInteger("-4b0706744e8452fe1ae9d5e1c28cf70fb6194795",16);
        result.m_nInsertRandomHash = new BigInteger("25aa0fad3f4881832dcdfe658ec2efa8a1a02bc5",16);
        result.m_nDeleteRandomHash = new BigInteger("-3ec87e810b46d734677c351ad893bbbf9ea10f55",16);
        result.m_nLinebreakHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        result.m_nOdfReloadHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        result.m_nMsWordReloadHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        return result;
    }

    @Test public void checkBookmarks()
        throws com.sun.star.uno.Exception,
            com.sun.star.io.IOException,
            java.security.NoSuchAlgorithmException
    {
        actualHashes.assertExpectation(getDEV300m41Expectations());
    }

    @Before public void setUpDocuments() throws Exception {
        m_xMsf = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
        m_xDoc = util.WriterTools.createTextDoc(m_xMsf);
        setupBookmarks();
        actualHashes.m_nSetupHash = BookmarkHashes.getBookmarksHash(m_xDoc);
        insertRandomParts(200177);
        actualHashes.m_nInsertRandomHash = BookmarkHashes.getBookmarksHash(m_xDoc);
        deleteRandomParts(4711);
        actualHashes.m_nDeleteRandomHash = BookmarkHashes.getBookmarksHash(m_xDoc);
        insertLinebreaks(007);
        actualHashes.m_nLinebreakHash = BookmarkHashes.getBookmarksHash(m_xDoc);
        m_xOdfReloadedDoc = reloadFrom("StarOffice XML (Writer)", "odf");
        actualHashes.m_nOdfReloadHash = BookmarkHashes.getBookmarksHash(m_xOdfReloadedDoc);
        m_xMsWordReloadedDoc = reloadFrom("MS Word 97", "doc");
        actualHashes.m_nMsWordReloadHash = BookmarkHashes.getBookmarksHash(m_xMsWordReloadedDoc);
    }

    @After public void tearDownDocuments() {
        util.DesktopTools.closeDoc(m_xDoc);
        util.DesktopTools.closeDoc(m_xOdfReloadedDoc);
        util.DesktopTools.closeDoc(m_xMsWordReloadedDoc);
    }

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

    private void setupBookmarks()
        throws com.sun.star.uno.Exception
    {
        XText xText = m_xDoc.getText();
        XSimpleText xSimpleText = (XSimpleText)UnoRuntime.queryInterface(
            XSimpleText.class,
            xText);
        for(int nPara=0; nPara<10; ++nPara) {
            for(int nBookmark=0; nBookmark<100; ++nBookmark){
                insertBookmark(
                    xText.createTextCursor(),
                    "P" + nPara + "word" + nBookmark,
                    "P" + nPara + "word" + nBookmark);
                XTextCursor xWordCrsr = xText.createTextCursor();
                xWordCrsr.setString(" ");
            }
            XTextCursor xParaCrsr = xText.createTextCursor();
            XTextRange xParaCrsrAsRange = (XTextRange)UnoRuntime.queryInterface(
                XTextRange.class,
                xParaCrsr);
            xText.insertControlCharacter(xParaCrsrAsRange, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
        }
    }

    private void insertRandomParts(long seed)
        throws com.sun.star.uno.Exception
    {
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCrsr = m_xDoc.getText().createTextCursor();
        for(int i=0; i<600; i++) {
            xCrsr.goRight((short)rnd.nextInt(100), false);
            xCrsr.setString(Long.toString(rnd.nextLong()));
        }
    }

    private void deleteRandomParts(long seed)
        throws com.sun.star.uno.Exception
    {
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCrsr = m_xDoc.getText().createTextCursor();
        for(int i=0; i<600; i++) {
            xCrsr.goRight((short)rnd.nextInt(100), false);
            xCrsr.goRight((short)rnd.nextInt(20), true);
            xCrsr.setString("");
        }
    }

    private void insertLinebreaks(long seed)
        throws com.sun.star.uno.Exception
    {
        XText xText = m_xDoc.getText();
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCrsr = m_xDoc.getText().createTextCursor();
        for(int i=0; i<30; i++) {
            xCrsr.goRight((short)rnd.nextInt(300), false);
            XTextRange xCrsrAsRange = (XTextRange)UnoRuntime.queryInterface(
                XTextRange.class,
                xCrsr);
            xText.insertControlCharacter(xCrsrAsRange, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
        }
    }

    private void insertBookmark(XTextCursor crsr, String name, String content)
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xDocFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            m_xDoc);

        Object xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark");
        XTextContent xBookmarkAsTextContent = (XTextContent)UnoRuntime.queryInterface(
            XTextContent.class,
            xBookmark);
        crsr.setString(content);
        XNamed xBookmarkAsNamed = (XNamed)UnoRuntime.queryInterface(
            XNamed.class,
            xBookmark);
        xBookmarkAsNamed.setName(name);
        m_xDoc.getText().insertTextContent(crsr, xBookmarkAsTextContent, true);
    }

    private XTextDocument reloadFrom(String sFilter, String sExtension)
        throws com.sun.star.io.IOException
    {
        String sFileUrl = util.utils.getOfficeTemp(m_xMsf) + "/Bookmarktest." + sExtension;
        try {
            PropertyValue[] aStoreProperties = new PropertyValue[2];
            aStoreProperties[0] = new PropertyValue();
            aStoreProperties[1] = new PropertyValue();
            aStoreProperties[0].Name = "Override";
            aStoreProperties[0].Value = true;
            aStoreProperties[1].Name = "FilterName";
            aStoreProperties[1].Value = sFilter;
            XStorable xStorable = (XStorable)UnoRuntime.queryInterface(
                XStorable.class,
                m_xDoc);
            xStorable.storeToURL(sFileUrl, aStoreProperties);
            return util.WriterTools.loadTextDoc(m_xMsf, sFileUrl);
        } finally {
            if(util.utils.fileExists(m_xMsf, sFileUrl))
                util.utils.deleteFile(m_xMsf, sFileUrl);
        }
    }
}

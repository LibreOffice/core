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

package complex.writer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNamed;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
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
    private BigInteger m_nMsWordReloadHash;

    public void assertExpectation(BookmarkHashes aExpectation) {
        assertEquals(aExpectation.m_nSetupHash, m_nSetupHash);
        assertEquals(aExpectation.m_nInsertRandomHash, m_nInsertRandomHash);
        assertEquals(aExpectation.m_nDeleteRandomHash, m_nDeleteRandomHash);
        assertEquals(aExpectation.m_nLinebreakHash, m_nLinebreakHash);
        assertEquals(aExpectation.m_nOdfReloadHash, m_nOdfReloadHash);
        assertEquals(aExpectation.m_nMsWordReloadHash, m_nMsWordReloadHash);
    }

    public static java.math.BigInteger getBookmarksHash(XTextDocument xDoc)
        throws com.sun.star.uno.Exception, java.security.NoSuchAlgorithmException
    {
        StringBuffer buffer = new StringBuffer("");
        XBookmarksSupplier xBookmarksSupplier = UnoRuntime.queryInterface(
            XBookmarksSupplier.class,
            xDoc);
        XNameAccess xBookmarks = xBookmarksSupplier.getBookmarks();
        for(String sBookmarkname : xBookmarks.getElementNames()) {
            Object xBookmark = xBookmarks.getByName(sBookmarkname);
            XTextContent xBookmarkAsContent = UnoRuntime.queryInterface(
                XTextContent.class,
                xBookmark);
            buffer.append(sBookmarkname);
            buffer.append(':');
            buffer.append(xBookmarkAsContent.getAnchor().getString().replaceAll("\r\n", "\n"));
            buffer.append(';');
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
    private final BookmarkHashes actualHashes = new BookmarkHashes();

    private BookmarkHashes get20111110Expectations() {
        BookmarkHashes result = new BookmarkHashes();
        result.m_nSetupHash = new BigInteger("-4b0706744e8452fe1ae9d5e1c28cf70fb6194795",16);
        result.m_nInsertRandomHash = new BigInteger("25aa0fad3f4881832dcdfe658ec2efa8a1a02bc5",16);
        result.m_nDeleteRandomHash = new BigInteger("-3ec87e810b46d734677c351ad893bbbf9ea10f55",16);
        result.m_nLinebreakHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        result.m_nOdfReloadHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        // MsWord Hash is unstable over different systems
	// result.m_nMsWordReloadHash = new BigInteger("3ae08c284ea0d6e738cb43c0a8105e718a633550",16);
        return result;
    }

    @Test public void checkBookmarks()
    {
        actualHashes.assertExpectation(get20111110Expectations());
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
        insertLinebreaks(7);
        actualHashes.m_nLinebreakHash = BookmarkHashes.getBookmarksHash(m_xDoc);
        m_xOdfReloadedDoc = reloadFrom("writer8", "odf");
        actualHashes.m_nOdfReloadHash = BookmarkHashes.getBookmarksHash(m_xOdfReloadedDoc);
        //m_xMsWordReloadedDoc = reloadFrom("MS Word 97", "doc");
        //actualHashes.m_nMsWordReloadHash = BookmarkHashes.getBookmarksHash(m_xMsWordReloadedDoc);
    }

    @After public void tearDownDocuments() {
        util.DesktopTools.closeDoc(m_xDoc);
        util.DesktopTools.closeDoc(m_xOdfReloadedDoc);
        //util.DesktopTools.closeDoc(m_xMsWordReloadedDoc);
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
        UnoRuntime.queryInterface(XSimpleText.class, xText);
        for(int nPara=0; nPara<10; ++nPara) {
            for(int nBookmark=0; nBookmark<100; ++nBookmark){
                insertBookmark(
                    xText.createTextCursor(),
                    "P" + nPara + "word" + nBookmark,
                    "P" + nPara + "word" + nBookmark);
                XTextCursor xWordCursor = xText.createTextCursor();
                xWordCursor.setString(" ");
            }
            XTextCursor xParaCursor = xText.createTextCursor();
            XTextRange xParaCursorAsRange = UnoRuntime.queryInterface(
                XTextRange.class,
                xParaCursor);
            xText.insertControlCharacter(xParaCursorAsRange, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
        }
    }

    private void insertRandomParts(long seed)
    {
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCursor = m_xDoc.getText().createTextCursor();
        for(int i=0; i<600; i++) {
            xCursor.goRight((short)rnd.nextInt(100), false);
            xCursor.setString(Long.toString(rnd.nextLong()));
        }
    }

    private void deleteRandomParts(long seed)
    {
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCursor = m_xDoc.getText().createTextCursor();
        for(int i=0; i<600; i++) {
            xCursor.goRight((short)rnd.nextInt(100), false);
            xCursor.goRight((short)rnd.nextInt(20), true);
            xCursor.setString("");
        }
    }

    private void insertLinebreaks(long seed)
    {
        XText xText = m_xDoc.getText();
        java.util.Random rnd = new java.util.Random(seed);
        XTextCursor xCursor = m_xDoc.getText().createTextCursor();
        for(int i=0; i<30; i++) {
            xCursor.goRight((short)rnd.nextInt(300), false);
            XTextRange xCursorAsRange = UnoRuntime.queryInterface(
                XTextRange.class,
                xCursor);
            xText.insertControlCharacter(xCursorAsRange, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false);
        }
    }

    private void insertBookmark(XTextCursor crsr, String name, String content)
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xDocFactory = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            m_xDoc);

        Object xBookmark = xDocFactory.createInstance("com.sun.star.text.Bookmark");
        XTextContent xBookmarkAsTextContent = UnoRuntime.queryInterface(
            XTextContent.class,
            xBookmark);
        crsr.setString(content);
        XNamed xBookmarkAsNamed = UnoRuntime.queryInterface(
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
            XStorable xStorable = UnoRuntime.queryInterface(
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

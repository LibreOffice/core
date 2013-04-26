/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package complex.writer;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XRefreshable;
import com.sun.star.util.XRefreshListener;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XDocumentIndex;
import com.sun.star.text.XParagraphCursor;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import org.openoffice.test.OfficeConnection;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;


public class CheckIndex
{
    private static final OfficeConnection connection = new OfficeConnection();

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private XMultiServiceFactory m_xMSF = null;
    private XComponentContext m_xContext = null;
    private XTextDocument m_xDoc = null;

    @Before public void before() throws Exception
    {
        m_xMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
        m_xContext = connection.getComponentContext();
        assertNotNull("could not get component context.", m_xContext);
        m_xDoc = util.WriterTools.createTextDoc(m_xMSF);
    }

    @After public void after()
    {
        util.DesktopTools.closeDoc(m_xDoc);
    }

    class RefreshListener implements XRefreshListener
    {
        public boolean m_bDisposed = false;
        public boolean m_bRefreshed = false;
        public void disposing(EventObject event)
        {
            m_bDisposed = true;
        }
        public void refreshed(EventObject event)
        {
            m_bRefreshed = true;
        }
        public void assertRefreshed()
        {
            assertTrue(m_bRefreshed);
            m_bRefreshed = false;
        }
    }

    @Test
    public void test_refresh() throws Exception
    {
        XMultiServiceFactory xDocFactory =
            UnoRuntime.queryInterface(XMultiServiceFactory.class, m_xDoc);
        Object xIndex =
            xDocFactory.createInstance("com.sun.star.text.ContentIndex");

        XText xBodyText = m_xDoc.getText();
        XParagraphCursor xCursor = UnoRuntime.queryInterface(
                XParagraphCursor.class, xBodyText.createTextCursor());
        XPropertySet xCursorSet =
            UnoRuntime.queryInterface(XPropertySet.class, xCursor);
        XTextContent xIndexContent =
            UnoRuntime.queryInterface(XTextContent.class, xIndex);
        XPropertySet xIndexSet =
            UnoRuntime.queryInterface(XPropertySet.class, xIndex);
        xIndexSet.setPropertyValue("CreateFromOutline", true);
        xBodyText.insertTextContent(xCursor, xIndexContent, true);

        XRefreshable xRefreshable =
            UnoRuntime.queryInterface(XRefreshable.class, xIndex);

        // test that refresh calls listener
        RefreshListener listener = new RefreshListener();
        xRefreshable.addRefreshListener(listener);
        assertFalse(listener.m_bRefreshed);
        xRefreshable.refresh();
        listener.assertRefreshed();

        // insert some heading
        xCursor.gotoEnd(false);
        xBodyText.insertControlCharacter(xCursor,
                ControlCharacter.PARAGRAPH_BREAK, false);
        xCursor.gotoEnd(false);
        xCursor.setString("a heading");
        xCursor.gotoStartOfParagraph(true);
        xCursorSet.setPropertyValue("ParaStyleName", "Heading 1");

        xRefreshable.refresh();
        listener.assertRefreshed();
        // hope text is in last paragraph...
        xCursor.gotoRange(xIndexContent.getAnchor().getEnd(), false);
        xCursor.gotoStartOfParagraph(true);
        String text = xCursor.getString();
        assertTrue(text.contains("a heading"));

        // insert some more headings
        xCursor.gotoEnd(false);
        xBodyText.insertControlCharacter(xCursor,
                ControlCharacter.PARAGRAPH_BREAK, false);
        xCursor.gotoEnd(false);
        xCursor.setString("yet another heading");
        xCursor.gotoStartOfParagraph(true);
        xCursorSet.setPropertyValue("ParaStyleName", "Heading 1");

        // try again with update
        XDocumentIndex xIndexIndex =
            UnoRuntime.queryInterface(XDocumentIndex.class, xIndex);
        xIndexIndex.update();
        listener.assertRefreshed();
        xCursor.gotoRange(xIndexContent.getAnchor().getEnd(), false);
        xCursor.gotoStartOfParagraph(true);
        text = xCursor.getString();
        assertTrue(text.contains("yet another heading"));

        // dispose must call the listener
        assertFalse(listener.m_bDisposed);
        xIndexIndex.dispose();
        assertTrue(listener.m_bDisposed);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

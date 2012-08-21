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

package testcase.uno.sw.bookmark;

import static org.junit.Assert.assertArrayEquals;
import static testlib.uno.SWUtil.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;


public class CheckBookmarks {
    private static final UnoApp app = new UnoApp();

    private XTextDocument document = null;

    private String[] initBookmarkNames= new String[]{"bookmark1", "bookmark2", "bookmark3"};

    private String[] initBookmarkContents= new String[]{"bookmark1 content", "bookmark2 content", "bookmark3 content!!!!!!!"};

    @Before
    public void setUp() throws Exception {
        app.start();
        document = UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
        XText xText = document.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.setString("Contents");

        for (int i = 0; i < initBookmarkNames.length; i++) {
            xTextCursor.gotoEnd(false);
            XTextRange xTextRange = UnoRuntime.queryInterface(XTextRange.class, xTextCursor);
            xText.insertControlCharacter(xTextRange, ControlCharacter.PARAGRAPH_BREAK, false);
            xTextCursor.gotoEnd(false);
            xTextCursor.setString(initBookmarkContents[i]);
            insertBookmark(document, xTextCursor, initBookmarkNames[i]);
        }
    }

    @After
    public void tearDown() {
        app.closeDocument(document);
    }

    @AfterClass
    public static void tearDownConnection() throws Exception {
        app.close();
    }

    private static String[] getBookmarkContents(XNameAccess xBookmarks) throws Exception {
        String[] bookmarkNames = xBookmarks.getElementNames();
        String[] bookmarkContents = new String[bookmarkNames.length];
        for (int i = 0; i < bookmarkNames.length; i++) {
            Object xBookmark = xBookmarks.getByName(bookmarkNames[i]);
            XTextContent xBookmarkAsContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xBookmark);
            bookmarkContents[i] = xBookmarkAsContent.getAnchor().getString();
        }

        return bookmarkContents;
    }

    @Test
    public void createBookmark() throws Exception {
        XNameAccess xBookmarks = UnoRuntime.queryInterface(XBookmarksSupplier.class, document).getBookmarks();
        assertArrayEquals("Bookmark name list:", initBookmarkNames, xBookmarks.getElementNames());
        assertArrayEquals("Bookmark content list:", initBookmarkContents, getBookmarkContents(xBookmarks));
    }

    @Test
    public void updateBookmarkContent() throws Exception {
        String[] expectedBookmarkNames= new String[]{"bookmark1", "bookmark2", "bookmark3"};
        String[] expectedBookmarkContents= new String[]{"bookmark1 content", "bookmark2 content", "bookmark3 cont"};
        // Delete some content
        XText xText = document.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(false);
        xTextCursor.goLeft((short)10, true);
        xTextCursor.setString("new");

        // Let's see the bookmarks
        XNameAccess xBookmarks = UnoRuntime.queryInterface(XBookmarksSupplier.class, document).getBookmarks();
        assertArrayEquals("Bookmark name list after updating some content:", expectedBookmarkNames, xBookmarks.getElementNames());
        assertArrayEquals("Bookmark content list after updating some content:", expectedBookmarkContents, getBookmarkContents(xBookmarks));
    }

    @Test
    public void removeBookmark() throws Exception {
        String[] expectedBookmarkNames= new String[]{"bookmark2", "bookmark3"};
        String[] expectedBookmarkContents= new String[]{"tent", "bookmark3 content!!!!!!!"};
        // Delete some content
        XText xText = document.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.goRight((short)40, true);
        xTextCursor.setString("");

        // Let's see the bookmarks
        XNameAccess xBookmarks = UnoRuntime.queryInterface(XBookmarksSupplier.class, document).getBookmarks();
        assertArrayEquals("Bookmark name list after deleting some content:", expectedBookmarkNames, xBookmarks.getElementNames());
        assertArrayEquals("Bookmark content list after deleting some content:", expectedBookmarkContents, getBookmarkContents(xBookmarks));
    }

}

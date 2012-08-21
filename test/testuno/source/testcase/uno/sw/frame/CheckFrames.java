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

package testcase.uno.sw.frame;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFrame;
import com.sun.star.text.XTextFramesSupplier;
import com.sun.star.uno.UnoRuntime;

public class CheckFrames {

    private static final UnoApp app = new UnoApp();

    private XTextDocument document = null;

    @Test(expected = NoSuchElementException.class)
    public void testLoadTextFrame() throws Exception {
        document = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.prepareData("testcase/uno/sw/CheckFlies.odt")));
        XTextFramesSupplier xTFS = UnoRuntime.queryInterface(XTextFramesSupplier.class, document);
        String[] expectedNames = { "Frame1", "Frame2" };
        String[] expectedContents = { "PageBoundFrame", "ParaBoundFrame" };
        XNameAccess xTextFrames = xTFS.getTextFrames();
        assertArrayEquals("Text frame names", expectedNames, xTextFrames.getElementNames());
        assertTrue("Has text frame named Frame1", xTextFrames.hasByName(expectedNames[0]));
        assertFalse("Has nonexisting text frame.", xTextFrames.hasByName("Nonexisting text frame"));

        XIndexAccess xTextFramesIdx = UnoRuntime.queryInterface(XIndexAccess.class, xTextFrames);
        assertEquals("Text frame count", expectedNames.length, xTextFramesIdx.getCount());
        String[] contents = new String[expectedNames.length];
        for (int i = 0; i < xTextFramesIdx.getCount(); i++) {
            Object obj = xTextFramesIdx.getByIndex(i);
            XTextFrame frame = UnoRuntime.queryInterface(XTextFrame.class, obj);
            contents[i] = frame.getText().getString();
        }
        assertArrayEquals("Text frame contents", expectedContents, contents);
        xTextFrames.getByName("Nonexisting Textframe");
    }

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() {
        app.closeDocument(document);
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException, Exception {
        app.close();
    }

}

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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import java.util.Collection;
import java.util.ArrayList;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

public class CheckFlies {
    @Test public void checkFlies()
        throws com.sun.star.uno.Exception
    {
        com.sun.star.text.XTextFramesSupplier xTFS = UnoRuntime.queryInterface(
            com.sun.star.text.XTextFramesSupplier.class,
            document);
        checkTextFrames(xTFS);
        com.sun.star.text.XTextGraphicObjectsSupplier xTGOS = UnoRuntime.queryInterface(
            com.sun.star.text.XTextGraphicObjectsSupplier.class,
            document);
        checkGraphicFrames(xTGOS);
        com.sun.star.text.XTextEmbeddedObjectsSupplier xTEOS = UnoRuntime.queryInterface(
            com.sun.star.text.XTextEmbeddedObjectsSupplier.class,
            document);
        checkEmbeddedFrames(xTEOS);
    }

    private void checkEmbeddedFrames(com.sun.star.text.XTextEmbeddedObjectsSupplier xTGOS)
        throws com.sun.star.uno.Exception
    {
        Collection<String> vExpectedEmbeddedFrames = new ArrayList<String>();
        vExpectedEmbeddedFrames.add("Object1");
        int nEmbeddedFrames = vExpectedEmbeddedFrames.size();
        com.sun.star.container.XNameAccess xEmbeddedFrames = xTGOS.getEmbeddedObjects();
        for(String sFrameName : xEmbeddedFrames.getElementNames())
        {
            assertTrue(
                "Unexpected frame name",
                vExpectedEmbeddedFrames.remove(sFrameName));
            xEmbeddedFrames.getByName(sFrameName);
            assertTrue(
                "Could not find embedded frame by name.",
                xEmbeddedFrames.hasByName(sFrameName));
        }
        assertTrue(
            "Missing expected embedded frames.",
            vExpectedEmbeddedFrames.isEmpty());
        try
        {
            xEmbeddedFrames.getByName("Nonexisting embedded frame");
            fail("Got nonexisting embedded frame");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        assertFalse(
            "Has nonexisting embedded frame",
            xEmbeddedFrames.hasByName("Nonexisting embedded frame"));

        com.sun.star.container.XIndexAccess xEmbeddedFramesIdx = UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xEmbeddedFrames);
        assertEquals(
            "Unexpected number of embedded frames reported.", nEmbeddedFrames,
            xEmbeddedFramesIdx.getCount());
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xEmbeddedFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            xEmbeddedFramesIdx.getByIndex(nCurrentFrameIdx);
        }
    }

    private void checkGraphicFrames(com.sun.star.text.XTextGraphicObjectsSupplier xTGOS)
        throws com.sun.star.uno.Exception
    {
        Collection<String> vExpectedGraphicFrames = new ArrayList<String>();
        vExpectedGraphicFrames.add("graphics1");
        int nGraphicFrames = vExpectedGraphicFrames.size();
        com.sun.star.container.XNameAccess xGraphicFrames = xTGOS.getGraphicObjects();
        for(String sFrameName : xGraphicFrames.getElementNames())
        {
            assertTrue(
                "Unexpected frame name",
                vExpectedGraphicFrames.remove(sFrameName));
            xGraphicFrames.getByName(sFrameName);
            assertTrue(
                "Could not find graphics frame by name.",
                xGraphicFrames.hasByName(sFrameName));
        }
        assertTrue(
            "Missing expected graphics frames.",
            vExpectedGraphicFrames.isEmpty());
        try
        {
            xGraphicFrames.getByName("Nonexisting graphics frame");
            fail("Got nonexisting graphics frame");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        assertFalse(
            "Has nonexisting graphics frame",
            xGraphicFrames.hasByName("Nonexisting graphics frame"));

        com.sun.star.container.XIndexAccess xGraphicFramesIdx = UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xGraphicFrames);
        assertEquals(
            "Unexpected number of graphics frames reported.", nGraphicFrames,
            xGraphicFramesIdx.getCount());
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xGraphicFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            xGraphicFramesIdx.getByIndex(nCurrentFrameIdx);
        }
    }

    private void checkTextFrames(com.sun.star.text.XTextFramesSupplier xTFS)
        throws com.sun.star.uno.Exception
    {
        Collection<String> vExpectedTextFrames = new ArrayList<String>();
        vExpectedTextFrames.add("Frame1");
        vExpectedTextFrames.add("Frame2");

        int nTextFrames = vExpectedTextFrames.size();
        com.sun.star.container.XNameAccess xTextFrames = xTFS.getTextFrames();
        for(String sFrameName : xTextFrames.getElementNames())
        {
            assertTrue(
                "Unexpected frame name",
                vExpectedTextFrames.remove(sFrameName));
            xTextFrames.getByName(sFrameName);
            assertTrue(
                "Could not find text frame by name.",
                xTextFrames.hasByName(sFrameName));
        }
        assertTrue(
            "Missing expected text frames.", vExpectedTextFrames.isEmpty());
        try
        {
            xTextFrames.getByName("Nonexisting Textframe");
            fail("Got nonexisting text frame.");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        assertFalse(
            "Has nonexisting text frame.",
            xTextFrames.hasByName("Nonexisting text frame"));

        com.sun.star.container.XIndexAccess xTextFramesIdx = UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xTextFrames);
        assertEquals(
            "Unexpected number of text frames reported.", nTextFrames,
            xTextFramesIdx.getCount());
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xTextFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            xTextFramesIdx.getByIndex(nCurrentFrameIdx);
        }
    }

    @Before public void setUpDocument() throws com.sun.star.uno.Exception {
        document = util.WriterTools.loadTextDoc(
            UnoRuntime.queryInterface(
                XMultiServiceFactory.class,
                connection.getComponentContext().getServiceManager()),
            TestDocument.getUrl("CheckFlies.odt"));
    }

    @After public void tearDownDocument() {
        util.DesktopTools.closeDoc(document);
    }

    private XTextDocument document = null;

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}

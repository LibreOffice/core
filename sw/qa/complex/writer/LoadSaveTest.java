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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.Pair;
import com.sun.star.frame.XStorable;
import com.sun.star.document.DocumentEvent;
import com.sun.star.document.XDocumentEventBroadcaster;
import com.sun.star.document.XDocumentEventListener;
import org.openoffice.test.OfficeConnection;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

import java.util.List;
import java.util.ArrayList;
import java.io.File;

/**
 * a small program to load documents from one directory (recursively)
 * and store them in another, implemented as a complex test.
 */
public class LoadSaveTest
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
    private XDocumentEventBroadcaster m_xGEB = null;
    private String m_TmpDir = null;

    private String m_fileURL = "file://";
    // these should be parameters or something?
    private String m_SourceDir = "FIXME";
    private String m_TargetDir = "/tmp/out";

    @Before public void before() throws Exception
    {
        m_xContext = connection.getComponentContext();
        assertNotNull("could not get component context.", m_xContext);
        m_xMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, m_xContext.getServiceManager());
        Object oGEB = m_xMSF.createInstance(
                "com.sun.star.frame.GlobalEventBroadcaster");
        m_xGEB = UnoRuntime.queryInterface(XDocumentEventBroadcaster.class, oGEB);
        assertNotNull("could not get global event broadcaster.", m_xGEB);
        m_TmpDir = util.utils.getOfficeTemp/*Dir*/(m_xMSF);
        System.out.println("tempdir: " + m_TmpDir);
        System.out.println("sourcedir: " + m_SourceDir);
        System.out.println("targetdir: " + m_TargetDir);
    }

    @Test public void testLoadStore() throws Exception
    {
        Pair<List<String>, List<String>> dirsFiles =
            getDirAndFileNames(m_SourceDir);
        makeDirs(m_TargetDir, dirsFiles.First);
        for (String fileName : dirsFiles.Second)
        {
            try {
                testDoc(fileName);
            } catch (Exception e) {
                report(e);
            }
        }
    }

    public void testDoc(String fileName) throws Exception
    {
        XComponent xDoc = null;
        EventListener xListener = new EventListener();
        try {
            m_xGEB.addDocumentEventListener(xListener);

            System.out.println("Loading document: " + fileName + " ...");

            PropertyValue[] loadProps = new PropertyValue[1];
            loadProps[0] = new PropertyValue();
            loadProps[0].Name = "ReadOnly";
            loadProps[0].Value = new Boolean(true);

            String sourceFile = m_fileURL + m_SourceDir + fileName;

            xDoc = util.DesktopTools.loadDoc(m_xMSF, sourceFile, loadProps);

            System.out.println("... done");

            {
                // apparently OnLayoutFinished is not sent for every doc???
                // 10 seconds is evidently not enough for large documents
                int time = 0;
                while (!xListener.IsLayoutFinished() && (time < 30000)) {
                    Thread.sleep(100);
                    time += 100;
                }
                if (time >= 30000) {
                    System.out.println("timeout: no OnLayoutFinished received");
                }
            }

            System.out.println("Storing document: " + fileName + " ...");

            XStorable xStor = UnoRuntime.queryInterface(
                        XStorable.class, xDoc);

            String targetFile = m_fileURL + m_TargetDir + fileName;

            xStor.storeToURL(targetFile, new PropertyValue[0]);

            System.out.println("... done");

        } finally {
            if (xDoc != null) {
                util.DesktopTools.closeDoc(xDoc);
            }
            if (xListener != null) {
                m_xGEB.removeDocumentEventListener(xListener);
            }
        }
    }

    class EventListener implements XDocumentEventListener
    {
        boolean m_isLayoutFinished = false;
        boolean IsLayoutFinished() { return m_isLayoutFinished; }
        public void documentEventOccured(DocumentEvent Event)
        {
//            System.out.println("event: " + Event.EventName);
            if ("OnLayoutFinished".equals(Event.EventName))
            {
                // we only have one doc at any time, so no need to check
                m_isLayoutFinished = true;
//                System.out.println("received OnLayoutFinished");
            }
        }
        public void disposing(EventObject Event) { }
    }

    void report2(Exception e)
    {
        if (e instanceof WrappedTargetException)
        {
            System.out.println("Cause:");
            Exception cause = (Exception)
                (((WrappedTargetException)e).TargetException);
            System.out.println(cause.toString());
            report2(cause);
        } else if (e instanceof WrappedTargetRuntimeException) {
            System.out.println("Cause:");
            Exception cause = (Exception)
                (((WrappedTargetRuntimeException)e).TargetException);
            System.out.println(cause.toString());
            report2(cause);
        }
    }

    void report(Exception e) {
        System.out.println("Exception occurred:");
        System.out.println(e.toString());
        e.printStackTrace(System.err);
        report2(e);
//        failed();
    }

    Pair<List<String>, List<String>> getDirAndFileNames(String dir)
    {
        List<String> dirs = new ArrayList<String>();
        List<String> files = new ArrayList<String>();
        File root = new File(dir);
        getDirAndFileNames(root, "", dirs, files);
        return new Pair<List<String>, List<String>>(dirs, files);
    }

    void getDirAndFileNames(File file, String relPath,
            List<String> dirs, List<String> files)
    {
        assertTrue("does not exist: " + relPath, file.exists());
        if (file.isDirectory()) {
            dirs.add(relPath);
            File[] subfiles = file.listFiles();
            for (File subfile : subfiles)
            {
                String subfileName =
                    relPath + File.separator + subfile.getName();
                getDirAndFileNames(subfile, subfileName, dirs, files);
            }
        }
        else if (file.isFile()) {
            if (file.getName().endsWith(".odt")) {
                files.add(relPath);
            }
        }
    }

    void makeDirs(String target, List<String> dirs) throws Exception
    {
        for (String dir : dirs) {
            File f = new File(target + dir);
            if (!f.exists()) {
                if (!f.mkdir()) {
                    throw new Exception("cannot mkdir: " + target + dir);
                }
            }
        }
    }
}


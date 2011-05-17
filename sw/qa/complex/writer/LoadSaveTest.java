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

import complexlib.ComplexTestCase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.Pair;
import com.sun.star.util.XCloseable;
import com.sun.star.frame.XStorable;
import com.sun.star.document.DocumentEvent;
import com.sun.star.document.XDocumentEventBroadcaster;
import com.sun.star.document.XDocumentEventListener;
import com.sun.star.text.XTextDocument;

import java.util.List;
import java.util.ArrayList;
import java.io.File;

/**
 * a small program to load documents from one directory (recursively)
 * and store them in another, implemented as a complex test.
 */
public class LoadSaveTest extends ComplexTestCase
{
    private XMultiServiceFactory m_xMSF = null;
    private XComponentContext m_xContext = null;
    private XDocumentEventBroadcaster m_xGEB = null;
    private String m_TmpDir = null;

    private String m_fileURL = "file://";
    // these should be parameters or something?
    private String m_SourceDir = "FIXME";
    private String m_TargetDir = "/tmp/out";

    public String[] getTestMethodNames() {
        return new String[] { "testLoadStore" };
    }

    public void before() throws Exception
    {
        m_xMSF = (XMultiServiceFactory) param.getMSF();
        XPropertySet xPropertySet = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, m_xMSF);
        Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
        m_xContext = (XComponentContext)
            UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
        assure("could not get component context.", m_xContext != null);
        Object oGEB = m_xMSF.createInstance(
                "com.sun.star.frame.GlobalEventBroadcaster");
        m_xGEB = (XDocumentEventBroadcaster)
            UnoRuntime.queryInterface(XDocumentEventBroadcaster.class, oGEB);
        assure("could not get global event broadcaster.", m_xGEB != null);
        m_TmpDir = util.utils.getOfficeTemp/*Dir*/(m_xMSF);
        log.println("tempdir: " + m_TmpDir);
        log.println("sourcedir: " + m_SourceDir);
        log.println("targetdir: " + m_TargetDir);
    }

    /*
    public void after()
    {
    }
    */

    public void testLoadStore() throws Exception
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

            log.println("Loading document: " + fileName + " ...");

            PropertyValue[] loadProps = new PropertyValue[1];
            loadProps[0] = new PropertyValue();
            loadProps[0].Name = "ReadOnly";
            loadProps[0].Value = new Boolean(true);

            String sourceFile = m_fileURL + m_SourceDir + fileName;

            xDoc = util.DesktopTools.loadDoc(m_xMSF, sourceFile, loadProps);

            log.println("... done");

            {
                // apparently OnLayoutFinished is not sent for every doc???
                // 10 seconds is evidently not enough for large documents
                int time = 0;
                while (!xListener.IsLayoutFinished() && (time < 30000)) {
                    Thread.sleep(100);
                    time += 100;
                }
                if (time >= 30000) {
                    log.println("timeout: no OnLayoutFinished received!");
                }
            }

            log.println("Storing document: " + fileName + " ...");

            XStorable xStor = (XStorable) UnoRuntime.queryInterface(
                        XStorable.class, xDoc);

            String targetFile = m_fileURL + m_TargetDir + fileName;

            xStor.storeToURL(targetFile, new PropertyValue[0]);

            log.println("... done");

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
//            log.println("event: " + Event.EventName);
            if ("OnLayoutFinished".equals(Event.EventName))
            {
                // we only have one doc at any time, so no need to check
                m_isLayoutFinished = true;
//                log.println("received OnLayoutFinished");
            }
        }
        public void disposing(EventObject Event) { }
    }

    void report2(Exception e)
    {
        if (e instanceof WrappedTargetException)
        {
            log.println("Cause:");
            Exception cause = (Exception)
                (((WrappedTargetException)e).TargetException);
            log.println(cause.toString());
            report2(cause);
        } else if (e instanceof WrappedTargetRuntimeException) {
            log.println("Cause:");
            Exception cause = (Exception)
                (((WrappedTargetRuntimeException)e).TargetException);
            log.println(cause.toString());
            report2(cause);
        }
    }

    void report(Exception e) {
        log.println("Exception occurred:");
        log.println(e.toString());
        e.printStackTrace((java.io.PrintWriter) log);
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
        assure("does not exist: " + relPath, file.exists());
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


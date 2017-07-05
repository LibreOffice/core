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

package complex.sfx2;


import complex.sfx2.tools.TestDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XInitialization;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.Locale;
import com.sun.star.lang.EventObject;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;
import com.sun.star.util.Duration;
import com.sun.star.util.XModifyListener;
import com.sun.star.util.XModifyBroadcaster;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.IllegalTypeException;

import com.sun.star.document.XDocumentProperties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * Test case for the service com.sun.star.document.DocumentProperties.
 * Currently, this service is implemented in
 * sfx2/source/doc/SfxDocumentMetaData.cxx.
 *
 */
public class DocumentProperties
{
    @After public void cleanup() {
                // nothing to do
    }

    // for testing modifications
    private class Listener implements XModifyListener {
        private boolean m_Called;

        Listener() {
            m_Called = false;
        }

        private boolean reset() {
            boolean oldCalled = m_Called;
            m_Called = false;
            return oldCalled;
        }

        public void modified(EventObject e) {
            m_Called = true;
        }

        public void disposing(EventObject e) {
        }
    }

    @Test public void check() throws Exception
    {
        XMultiServiceFactory xMSF = getMSF();
        assertNotNull("could not create MultiServiceFactory.", xMSF);
        XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
        XComponentContext xContext = UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
        assertNotNull("could not get component context.", xContext);

        // TODO: Path to temp
        String temp = util.utils.getOfficeTemp/*Dir*/(xMSF);
        System.out.println("tempdir: " + temp);

        PropertyValue[] noArgs = { };
        PropertyValue mimetype = new PropertyValue();
        mimetype.Name = "MediaType";
        mimetype.Value = "application/vnd.oasis.opendocument.text";
        PropertyValue[] mimeArgs = { mimetype };
        PropertyValue cfile = new PropertyValue();
        cfile.Name = "URL";
        cfile.Value = temp + "EMPTY.odt";
        PropertyValue[] mimeEmptyArgs = { mimetype, cfile };

        System.out.println("Creating service DocumentProperties...");

        Object oDP =
            xMSF.createInstance("com.sun.star.document.DocumentProperties");
        XDocumentProperties xDP = UnoRuntime.queryInterface(XDocumentProperties.class, oDP);

        System.out.println("...done");


        System.out.println("Checking initialize ...");

        XDocumentProperties xDP2 = UnoRuntime.queryInterface(XDocumentProperties.class, xMSF.createInstance("com.sun.star.document.DocumentProperties"));
        XInitialization xInit = UnoRuntime.queryInterface(XInitialization.class, xDP2);
        xInit.initialize(new Object[] { });

        System.out.println("...done");

        System.out.println("Checking storing default-initialized meta data ...");

        xDP2.storeToMedium("", mimeEmptyArgs);

        System.out.println("...done");

        System.out.println("Checking loading default-initialized meta data ...");

        xDP2.loadFromMedium("", mimeEmptyArgs);
        assertEquals("Author", "", xDP2.getAuthor());

        System.out.println("...done");

        System.out.println("(Not) Checking preservation of custom meta data ...");

        xDP2.loadFromMedium(TestDocument.getUrl("CUSTOM.odt"),
            noArgs);
        assertEquals("Author", "", xDP2.getAuthor());
        xDP2.storeToMedium(temp + "CUSTOM.odt", mimeArgs);

        //FIXME: now what? comparing for binary equality seems useless
        // we could unzip the written file and grep for the custom stuff
        // but would that work on windows...

        System.out.println("...done");

        System.out.println("Checking loading from test document...");

        String file = TestDocument.getUrl("TEST.odt");
        xDP.loadFromMedium(file, noArgs);
/*            XInputStream xStream =
                new StreamSimulator("./testdocuments/TEST.odt", true, param);
            Object oSF =
                xMSF.createInstance("com.sun.star.embed.StorageFactory");
            XSingleServiceFactory xSF = (XSingleServiceFactory)
                UnoRuntime.queryInterface(XSingleServiceFactory.class, oSF);
            Object oStor = xSF.createInstanceWithArguments(
                new Object[] { xStream });
            XStorage xStor = (XStorage) UnoRuntime.queryInterface(
                XStorage.class, oStor);
            xDP.loadFromStorage(xStor);*/

        System.out.println("...done");

        System.out.println("Checking meta-data import...");

        assertEquals("Author", "John Smith Jr.", xDP.getAuthor());
        assertEquals(
            "Generator",
            "StarOffice/8$Solaris_x86 OpenOffice.org_project/680m232$Build-9227",
            xDP.getGenerator());
        assertEquals("CreationDate", 2007, xDP.getCreationDate().Year);
        assertEquals("Title", "Urgent Memo", xDP.getTitle());
        assertEquals("Subject", "Important Message", xDP.getSubject());
        assertEquals(
            "Description",
            "Modern internal company memorandum in full-blocked style",
            xDP.getDescription());
        assertEquals(
            "ModifiedBy", "John Smith Jr.", xDP.getModifiedBy());
        assertEquals(
            "ModificationDate", 10, xDP.getModificationDate().Month);
        assertEquals(
            "PrintedBy", "John Smith Jr.", xDP.getPrintedBy());
        assertEquals("PrintDate", 29, xDP.getPrintDate().Day);
        assertEquals("TemplateName", "Modern Memo", xDP.getTemplateName());
        assertTrue("TemplateURL",
            xDP.getTemplateURL().endsWith("memmodern.ott"));
        assertEquals("TemplateDate", 17, xDP.getTemplateDate().Hours);
        assertTrue(
            "AutoloadURL", xDP.getAutoloadURL().endsWith("/TEST.odt"));
        assertEquals("AutoloadSecs", 0, xDP.getAutoloadSecs());
        assertEquals("DefaultTarget", "_blank", xDP.getDefaultTarget());
        assertEquals("EditingCycles", 3, xDP.getEditingCycles());
        assertEquals("EditingDuration", 320, xDP.getEditingDuration());

        String[] kws = xDP.getKeywords();
        assertTrue("Keywords", fromArray(kws).containsAll(
                fromArray(new Object[] { "Asia", "Memo", "Rice" })));

        NamedValue[] ds = xDP.getDocumentStatistics();
        assertTrue("DocumentStatistics:WordCount", containsNV(ds,
                    new NamedValue("WordCount", Integer.valueOf(23))));
        assertTrue("DocumentStatistics:PageCount", containsNV(ds,
                    new NamedValue("PageCount", Integer.valueOf(1))));

        XPropertyContainer udpc = xDP.getUserDefinedProperties();
        XPropertySet udps = UnoRuntime.queryInterface( XPropertySet.class, udpc );
        assertEquals(
            "UserDefined 1", "This is an important note",
            udps.getPropertyValue("Hinweis"));
        assertEquals(
            "UserDefined 2", "May contain traces of nuts",
            udps.getPropertyValue("Warning"));

        System.out.println("...done");

        System.out.println("Checking meta-data updates...");

        String str;
        DateTime dt = new DateTime();
        Locale l = new Locale();
        int i;

        str = "me";
        xDP.setAuthor(str);
        assertEquals("setAuthor", str, xDP.getAuthor());
        str = "the computa";
        xDP.setGenerator(str);
        assertEquals("setGenerator", str, xDP.getGenerator());
        dt.Year = 2038;
        dt.Month = 1;
        dt.Day = 1;
        xDP.setCreationDate(dt);
        assertEquals(
            "setCreationDate", dt.Year, xDP.getCreationDate().Year);
        str = "The title";
        xDP.setTitle(str);
        assertEquals("setTitle", str, xDP.getTitle());
        str = "A degenerate subject";
        xDP.setSubject(str);
        assertEquals("setSubject", str, xDP.getSubject());
        str = "This description is not important";
        xDP.setDescription(str);
        assertEquals("setDescription", str, xDP.getDescription());
        l.Language = "en";
        l.Country = "GB";
        xDP.setLanguage(l);
        Locale l2 = xDP.getLanguage();
        assertEquals("setLanguage Lang", l.Language, l2.Language);
        assertEquals("setLanguage Cty", l.Country, l2.Country);
        str = "myself";
        xDP.setModifiedBy(str);
        assertEquals("setModifiedBy", str, xDP.getModifiedBy());
        dt.Year = 2042;
        xDP.setModificationDate(dt);
        assertEquals(
            "setModificationDate", dt.Year, xDP.getModificationDate().Year);
        str = "i did not do it";
        xDP.setPrintedBy(str);
        assertEquals("setPrintedBy", str, xDP.getPrintedBy());
        dt.Year = 2024;
        xDP.setPrintDate(dt);
        assertEquals("setPrintDate", dt.Year, xDP.getPrintDate().Year);
        str = "blah";
        xDP.setTemplateName(str);
        assertEquals("setTemplateName", str, xDP.getTemplateName());
        str = "gopher://some-hole-in-the-ground/";
        xDP.setTemplateURL(str);
        assertEquals("setTemplateURL", str, xDP.getTemplateURL());
        dt.Year = 2043;
        xDP.setTemplateDate(dt);
        assertEquals(
            "setTemplateDate", dt.Year, xDP.getTemplateDate().Year);
        str = "http://nowhere/";
        xDP.setAutoloadURL(str);
        assertEquals("setAutoloadURL", str, xDP.getAutoloadURL());
        i = 3661; // this might not work (due to conversion via double...)
        xDP.setAutoloadSecs(i);
        assertEquals("setAutoloadSecs", i, xDP.getAutoloadSecs());
        str = "_blank";
        xDP.setDefaultTarget(str);
        assertEquals("setDefaultTarget", str, xDP.getDefaultTarget());
        i = 42;
        xDP.setEditingCycles((short) i);
        assertEquals("setEditingCycles", i, xDP.getEditingCycles());
        i = 84;
        xDP.setEditingDuration(i);
        assertEquals("setEditingDuration", i, xDP.getEditingDuration());
        str = "";

        String[] kws2 = new String[] {
            "keywordly", "keywordlike", "keywordalicious" };
        xDP.setKeywords(kws2);
        kws = xDP.getKeywords();
        assertTrue("setKeywords", fromArray(kws).containsAll(fromArray(kws2)));

        NamedValue[] ds2 = new NamedValue[] {
                new NamedValue("SyllableCount", Integer.valueOf(9)),
                new NamedValue("FrameCount", Integer.valueOf(2)),
                new NamedValue("SentenceCount", Integer.valueOf(7)) };
        xDP.setDocumentStatistics(ds2);
        ds = xDP.getDocumentStatistics();
        assertTrue("setDocumentStatistics:SyllableCount", containsNV(ds,
                    new NamedValue("SyllableCount", Integer.valueOf(9))));
        assertTrue("setDocumentStatistics:FrameCount", containsNV(ds,
                    new NamedValue("FrameCount", Integer.valueOf(2))));
        assertTrue("setDocumentStatistics:SentenceCount", containsNV(ds,
                    new NamedValue("SentenceCount", Integer.valueOf(7))));

        System.out.println("...done");

        System.out.println("Checking user-defined meta-data updates...");

        // actually, this tests the PropertyBag service
        // but maybe the DocumentProperties service will be implemented
        // differently some day...
        boolean b = true;
        double d = 3.1415;
        // note that Time is only supported for backward compatibility!
        Time t = new Time();
        t.Hours = 1;
        t.Minutes = 16;
        Date date = new Date();
        date.Year = 2071;
        date.Month = 2;
        date.Day = 3;
        dt.Year = 2065;
        Duration dur = new Duration();
        dur.Negative = true;
        dur.Years = 1001;
        dur.Months = 999;
        dur.Days = 888;
        dur.Hours = 777;
        dur.Minutes = 666;
        dur.Seconds = 555;
        dur.NanoSeconds = 444444444;

        udpc.addProperty("Frobnicate", PropertyAttribute.REMOVABLE, b);
        udpc.addProperty("FrobDuration", PropertyAttribute.REMOVABLE, dur);
        udpc.addProperty("FrobDuration2", PropertyAttribute.REMOVABLE, t);
        udpc.addProperty("FrobEndDate", PropertyAttribute.REMOVABLE, date);
        udpc.addProperty("FrobStartTime", PropertyAttribute.REMOVABLE, dt);
        udpc.addProperty("Pi", PropertyAttribute.REMOVABLE, new Double(d));
        udpc.addProperty("Foo", PropertyAttribute.REMOVABLE, "bar");
        udpc.addProperty("Removed", PropertyAttribute.REMOVABLE, "bar");
        // #i94175#: empty property name is valid ODF 1.1
        udpc.addProperty("", PropertyAttribute.REMOVABLE, "eeeeek");
        try {
            udpc.removeProperty("Info 1");
            udpc.removeProperty("Removed");
        } catch (UnknownPropertyException e) {
            fail("removeProperty failed");
        }

        try {
            udpc.addProperty("Forbidden", PropertyAttribute.REMOVABLE,
                new String[] { "foo", "bar" });
            fail("inserting value of non-supported type did not fail");
        } catch (IllegalTypeException e) {
            // ignore
        }

        assertEquals(
            "UserDefined bool", b, udps.getPropertyValue("Frobnicate"));
        assertTrue("UserDefined duration", eqDuration(dur, (Duration)
                udps.getPropertyValue("FrobDuration")));
        assertTrue("UserDefined time", eqTime(t, (Time)
                udps.getPropertyValue("FrobDuration2")));
        assertTrue("UserDefined date", eqDate(date, (Date)
                udps.getPropertyValue("FrobEndDate")));
        assertTrue("UserDefined datetime", eqDateTime(dt, (DateTime)
                udps.getPropertyValue("FrobStartTime")));
        assertEquals("UserDefined float", d, udps.getPropertyValue("Pi"));
        assertEquals(
            "UserDefined string", "bar", udps.getPropertyValue("Foo"));
        assertEquals(
            "UserDefined empty name", "eeeeek", udps.getPropertyValue(""));

        try {
            udps.getPropertyValue("Removed");
            fail("UserDefined remove didn't");
        } catch (UnknownPropertyException e) {
            // ok
        }

        System.out.println("...done");

        System.out.println("Checking storing meta-data to file...");

        xDP.storeToMedium(temp + "TEST.odt", mimeArgs);

        System.out.println("...done");

        System.out.println("Checking loading meta-data from stored file...");

        xDP.loadFromMedium(temp + "TEST.odt", noArgs);

        System.out.println("...done");

        System.out.println("Checking user-defined meta-data from stored file...");

        udpc = xDP.getUserDefinedProperties();
        udps = UnoRuntime.queryInterface( XPropertySet.class, udpc );

        assertEquals(
            "UserDefined bool", b, udps.getPropertyValue("Frobnicate"));
        assertTrue("UserDefined duration", eqDuration(dur, (Duration)
                udps.getPropertyValue("FrobDuration")));
        // this is now a Duration!
        Duration t_dur = new Duration(false, (short)0, (short)0, (short)0,
                t.Hours, t.Minutes, t.Seconds,
                t.NanoSeconds);
        assertTrue("UserDefined time", eqDuration(t_dur, (Duration)
                udps.getPropertyValue("FrobDuration2")));
        assertTrue("UserDefined date", eqDate(date, (Date)
                udps.getPropertyValue("FrobEndDate")));
        assertTrue("UserDefined datetime", eqDateTime(dt, (DateTime)
                udps.getPropertyValue("FrobStartTime")));
        assertEquals("UserDefined float", d, udps.getPropertyValue("Pi"));
        assertEquals(
            "UserDefined string", "bar", udps.getPropertyValue("Foo"));

        try {
            udps.getPropertyValue("Removed");
            fail("UserDefined remove didn't");
        } catch (UnknownPropertyException e) {
            // ok
        }

        System.out.println("...done");

        System.out.println("Checking notification listener interface...");

        Listener listener = new Listener();
        XModifyBroadcaster xMB = UnoRuntime.queryInterface( XModifyBroadcaster.class, xDP );
        xMB.addModifyListener(listener);
        xDP.setAuthor("not me");
        assertTrue("Listener Author", listener.reset());
        udpc.addProperty("Listener", PropertyAttribute.REMOVABLE, "foo");
        assertTrue("Listener UserDefined Add", listener.reset());
        udps.setPropertyValue("Listener", "bar");
        assertTrue("Listener UserDefined Set", listener.reset());
        udpc.removeProperty("Listener");
        assertTrue("Listener UserDefined Remove", listener.reset());
        xMB.removeModifyListener(listener);
        udpc.addProperty("Listener2", PropertyAttribute.REMOVABLE, "foo");
        assertTrue("Removed Listener UserDefined Add", !listener.reset());

        System.out.println("...done");
    }

    // grrr...
    boolean eqDateTime(DateTime a, DateTime b) {
        return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day
            && a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.NanoSeconds == b.NanoSeconds;
    }

    boolean eqDate(Date a, Date b) {
        return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day;
    }

    boolean eqTime(Time a, Time b) {
        return a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.NanoSeconds == b.NanoSeconds;
    }

    boolean eqDuration(Duration a, Duration b) {
        return a.Years == b.Years && a.Months == b.Months && a.Days == b.Days
            && a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.NanoSeconds == b.NanoSeconds
            && a.Negative == b.Negative;
    }

    java.util.Collection<Object> fromArray(Object[] os) {
        java.util.Collection<Object> ret = new java.util.HashSet<Object>();
        for (int i = 0; i < os.length; ++i) {
            ret.add(os[i]);
        }
        return ret;
    }

    // bah, structs do not have proper equals(), and uno.Type is not comparable
    public static boolean containsNV (NamedValue[] nvs, NamedValue nv) {
        for (int i = 0; i < nvs.length; ++i) {
            if (nvs[i].Name.equals(nv.Name) && nvs[i].Value.equals(nv.Value)) {
                return true;
            }
        }
        return false;
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface( XMultiServiceFactory.class, connection.getComponentContext().getServiceManager() );
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "starting class: " + DocumentProperties.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "finishing class: " + DocumentProperties.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}


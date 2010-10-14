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
 * @author mst
 */
public class DocumentProperties
{
    @After public void cleanup() {
                // nothing to do
    }

    // for testing modifications
    class Listener implements XModifyListener {
        private boolean m_Called;

        Listener() {
            m_Called = false;
        }

        public boolean reset() {
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

    @Test public void check() {
        try {
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
//                new Any("application/vnd.oasis.opendocument.text")) };
            PropertyValue cfile = new PropertyValue();
            cfile.Name = "URL";
            cfile.Value = temp + "EMPTY.odt";
            PropertyValue[] mimeEmptyArgs = { mimetype, cfile };

            System.out.println("Creating service DocumentProperties...");

            Object oDP =
//                xMSF.createInstanceWithContext(
//                    "com.sun.star.document.DocumentProperties", xContext);
                xMSF.createInstance("com.sun.star.document.DocumentProperties");
            XDocumentProperties xDP = UnoRuntime.queryInterface(XDocumentProperties.class, oDP);

            System.out.println("...done");


            System.out.println("Checking initialize ...");

            XDocumentProperties xDP2 = UnoRuntime.queryInterface(XDocumentProperties.class, xMSF.createInstance("com.sun.star.document.DocumentProperties"));
            XInitialization xInit = UnoRuntime.queryInterface(XInitialization.class, xDP2);
            xInit.initialize(new Object[] { });

            System.out.println("...done");

            System.out.println("Checking storing default-initialized meta data ...");

//            xDP2.storeToMedium(temp + "EMPTY.odt", mimeArgs);
            xDP2.storeToMedium("", mimeEmptyArgs);

            System.out.println("...done");

            System.out.println("Checking loading default-initialized meta data ...");

//            xDP2.loadFromMedium(temp + "EMPTY.odt", noArgs);
            xDP2.loadFromMedium("", mimeEmptyArgs);
            assertTrue ("Author", "".equals(xDP2.getAuthor()));

            System.out.println("...done");

            System.out.println("(Not) Checking preservation of custom meta data ...");

            xDP2.loadFromMedium(TestDocument.getUrl("CUSTOM.odt"),
                noArgs);
            assertTrue ("Author", "".equals(xDP2.getAuthor()));
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

            assertTrue("Author", "Karl-Heinz Mustermann".equals(xDP.getAuthor()));
            assertTrue("Generator",
            "StarOffice/8$Solaris_x86 OpenOffice.org_project/680m232$Build-9227"
                .equals(xDP.getGenerator()));
            assertTrue("CreationDate", 2007 == xDP.getCreationDate().Year);
            assertTrue("Title", "Urgent Memo".equals(xDP.getTitle()));
            assertTrue("Subject", "Wichtige Mitteilung".equals(xDP.getSubject()));
            assertTrue("Description",
                "Modern internal company memorandum in full-blocked style"
                .equals(xDP.getDescription()));
//            assertTrue("Language", "".equals(xDP.getLanguage()));
            assertTrue("ModifiedBy",
                "Karl-Heinz Mustermann".equals(xDP.getModifiedBy()));
            assertTrue("ModificationDate", 10 == xDP.getModificationDate().Month);
            assertTrue("PrintedBy",
                "Karl-Heinz Mustermann".equals(xDP.getPrintedBy()));
            assertTrue("PrintDate", 29 == xDP.getPrintDate().Day);
            assertTrue("TemplateName",
                "Modern Memo".equals(xDP.getTemplateName()));
            assertTrue("TemplateURL",
                xDP.getTemplateURL().endsWith("memmodern.ott"));
            assertTrue("TemplateDate", 17 == xDP.getTemplateDate().Hours);
            assertTrue("AutoloadURL", "../TEST.odt".equals(xDP.getAutoloadURL()));
            assertTrue("AutoloadSecs", 0 == xDP.getAutoloadSecs());
            assertTrue("DefaultTarget", "_blank".equals(xDP.getDefaultTarget()));
            assertTrue("EditingCycles", 3 == xDP.getEditingCycles());
            assertTrue("EditingDuration", 320 == xDP.getEditingDuration());

            String[] kws = xDP.getKeywords();
            assertTrue("Keywords", fromArray(kws).containsAll(
                    fromArray(new Object[] { "Asien", "Memo", "Reis" })));

            NamedValue[] ds = xDP.getDocumentStatistics();
/*            for (int i = 0; i < ds.length; ++i) {
                System.out.println("nv: " + ds[i].Name + " " + ds[i].Value);
            }
            NamedValue nv1 = new NamedValue("WordCount", new Integer(23));
            NamedValue nv2 = new NamedValue("WordCount", new Integer(23));
            System.out.println("eq: " + nv1.equals(nv2)); // grrr, this is false...
*/
            assertTrue("DocumentStatistics:WordCount", containsNV(ds,
                        new NamedValue("WordCount", new Integer(23))));
            assertTrue("DocumentStatistics:PageCount", containsNV(ds,
                        new NamedValue("PageCount", new Integer(1))));

            XPropertyContainer udpc = xDP.getUserDefinedProperties();
            XPropertySet udps = UnoRuntime.queryInterface( XPropertySet.class, udpc );
            assertTrue("UserDefined 1", "Dies ist ein wichtiger Hinweis"
                    .equals(udps.getPropertyValue("Hinweis")));
            assertTrue("UserDefined 2", ("Kann Spuren von N"
                + new String(new byte[] { (byte) 0xc3, (byte) 0xbc }, "UTF-8")
                + "ssen enthalten")
                    .equals(udps.getPropertyValue("Warnung")));

            System.out.println("...done");

            System.out.println("Checking meta-data updates...");

            String str;
            DateTime dt = new DateTime();
            Locale l = new Locale();
            int i;

            str = "me";
            xDP.setAuthor(str);
            assertTrue("setAuthor", str.equals(xDP.getAuthor()));
            str = "the computa";
            xDP.setGenerator(str);
            assertTrue("setGenerator", str.equals(xDP.getGenerator()));
            dt.Year = 2038;
            dt.Month = 1;
            dt.Day = 1;
            xDP.setCreationDate(dt);
            assertTrue("setCreationDate", dt.Year == xDP.getCreationDate().Year);
            str = "El t'itulo";
            xDP.setTitle(str);
            assertTrue("setTitle", str.equals(xDP.getTitle()));
            str = "Ein verkommenes Subjekt";
            xDP.setSubject(str);
            assertTrue("setSubject", str.equals(xDP.getSubject()));
            str = "Este descripci'on no es importante";
            xDP.setDescription(str);
            assertTrue("setDescription", str.equals(xDP.getDescription()));
            l.Language = "en";
            l.Country = "GB";
            xDP.setLanguage(l);
            Locale l2 = xDP.getLanguage();
            assertTrue("setLanguage Lang", l.Language.equals(l2.Language));
            assertTrue("setLanguage Cty", l.Country.equals(l2.Country));
            str = "myself";
            xDP.setModifiedBy(str);
            assertTrue("setModifiedBy", str.equals(xDP.getModifiedBy()));
            dt.Year = 2042;
            xDP.setModificationDate(dt);
            assertTrue("setModificationDate",
                dt.Year == xDP.getModificationDate().Year);
            str = "i didnt do it";
            xDP.setPrintedBy(str);
            assertTrue("setPrintedBy", str.equals(xDP.getPrintedBy()));
            dt.Year = 2024;
            xDP.setPrintDate(dt);
            assertTrue("setPrintDate", dt.Year == xDP.getPrintDate().Year);
            str = "blah";
            xDP.setTemplateName(str);
            assertTrue("setTemplateName", str.equals(xDP.getTemplateName()));
            str = "gopher://some-hole-in-the-ground/";
            xDP.setTemplateURL(str);
            assertTrue("setTemplateURL", str.equals(xDP.getTemplateURL()));
            dt.Year = 2043;
            xDP.setTemplateDate(dt);
            assertTrue("setTemplateDate", dt.Year == xDP.getTemplateDate().Year);
            str = "http://nowhere/";
            xDP.setAutoloadURL(str);
            assertTrue("setAutoloadURL", str.equals(xDP.getAutoloadURL()));
            i = 3661; // this might not work (due to conversion via double...)
            xDP.setAutoloadSecs(i);
//            System.out.println("set: " + i + " get: " + xDP.getAutoloadSecs());
            assertTrue("setAutoloadSecs", i == xDP.getAutoloadSecs());
            str = "_blank";
            xDP.setDefaultTarget(str);
            assertTrue("setDefaultTarget", str.equals(xDP.getDefaultTarget()));
            i = 42;
            xDP.setEditingCycles((short) i);
            assertTrue("setEditingCycles", i == xDP.getEditingCycles());
            i = 84;
            xDP.setEditingDuration(i);
            assertTrue("setEditingDuration", i == xDP.getEditingDuration());
            str = "";

            String[] kws2 = new String[] {
                "keywordly", "keywordlike", "keywordalicious" };
            xDP.setKeywords(kws2);
            kws = xDP.getKeywords();
            assertTrue("setKeywords", fromArray(kws).containsAll(fromArray(kws2)));

            NamedValue[] ds2 = new NamedValue[] {
                    new NamedValue("SyllableCount", new Integer(9)),
                    new NamedValue("FrameCount", new Integer(2)),
                    new NamedValue("SentenceCount", new Integer(7)) };
            xDP.setDocumentStatistics(ds2);
            ds = xDP.getDocumentStatistics();
            assertTrue("setDocumentStatistics:SyllableCount", containsNV(ds,
                        new NamedValue("SyllableCount", new Integer(9))));
            assertTrue("setDocumentStatistics:FrameCount", containsNV(ds,
                        new NamedValue("FrameCount", new Integer(2))));
            assertTrue("setDocumentStatistics:SentenceCount", containsNV(ds,
                        new NamedValue("SentenceCount", new Integer(7))));

            System.out.println("...done");

            System.out.println("Checking user-defined meta-data updates...");

            // actually, this tests the PropertyBag service
            // but maybe the DocumentProperties service will be implemented
            // differently some day...
            boolean b = true;
            double d = 3.1415;
            // note that Time is only supported for backward compatibilty!
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
            dur.MilliSeconds = 444;

            udpc.addProperty("Frobnicate", PropertyAttribute.REMOVEABLE, b);
            udpc.addProperty("FrobDuration", PropertyAttribute.REMOVEABLE, dur);
            udpc.addProperty("FrobDuration2", PropertyAttribute.REMOVEABLE, t);
            udpc.addProperty("FrobEndDate", PropertyAttribute.REMOVEABLE, date);
            udpc.addProperty("FrobStartTime", PropertyAttribute.REMOVEABLE, dt);
            udpc.addProperty("Pi", PropertyAttribute.REMOVEABLE, new Double(d));
            udpc.addProperty("Foo", PropertyAttribute.REMOVEABLE, "bar");
            udpc.addProperty("Removed", PropertyAttribute.REMOVEABLE, "bar");
            // #i94175#: empty property name is valid ODF 1.1
            udpc.addProperty("", PropertyAttribute.REMOVEABLE, "eeeeek");
            try {
                udpc.removeProperty("Info 1");
                udpc.removeProperty("Removed");
            } catch (UnknownPropertyException e) {
                fail("removeProperty failed");
            }

            try {
                udpc.addProperty("Forbidden", PropertyAttribute.REMOVEABLE,
                    new String[] { "foo", "bar" });
                fail("inserting value of non-supported type did not fail");
            } catch (IllegalTypeException e) {
                // ignore
            }

            assertTrue("UserDefined bool", new Boolean(b).equals(
                    udps.getPropertyValue("Frobnicate")));
            assertTrue("UserDefined duration", eqDuration(dur, (Duration)
                    udps.getPropertyValue("FrobDuration")));
            assertTrue("UserDefined time", eqTime(t, (Time)
                    udps.getPropertyValue("FrobDuration2")));
            assertTrue("UserDefined date", eqDate(date, (Date)
                    udps.getPropertyValue("FrobEndDate")));
            assertTrue("UserDefined datetime", eqDateTime(dt, (DateTime)
                    udps.getPropertyValue("FrobStartTime")));
            assertTrue("UserDefined float", new Double(d).equals(
                    udps.getPropertyValue("Pi")));
            assertTrue("UserDefined string", "bar".equals(
                    udps.getPropertyValue("Foo")));
            assertTrue("UserDefined empty name", "eeeeek".equals(
                    udps.getPropertyValue("")));

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

            assertTrue("UserDefined bool", new Boolean(b).equals(
                    udps.getPropertyValue("Frobnicate")));
            assertTrue("UserDefined duration", eqDuration(dur, (Duration)
                    udps.getPropertyValue("FrobDuration")));
            // this is now a Duration!
            Duration t_dur = new Duration(false, (short)0, (short)0, (short)0,
                    t.Hours, t.Minutes, t.Seconds,
                    (short)(10 * t.HundredthSeconds));
            assertTrue("UserDefined time", eqDuration(t_dur, (Duration)
                    udps.getPropertyValue("FrobDuration2")));
            assertTrue("UserDefined date", eqDate(date, (Date)
                    udps.getPropertyValue("FrobEndDate")));
            assertTrue("UserDefined datetime", eqDateTime(dt, (DateTime)
                    udps.getPropertyValue("FrobStartTime")));
            assertTrue("UserDefined float", new Double(d).equals(
                    udps.getPropertyValue("Pi")));
            assertTrue("UserDefined string", "bar".equals(
                    udps.getPropertyValue("Foo")));

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
            udpc.addProperty("Listener", PropertyAttribute.REMOVEABLE, "foo");
            assertTrue("Listener UserDefined Add", listener.reset());
            udps.setPropertyValue("Listener", "bar");
            assertTrue("Listener UserDefined Set", listener.reset());
            udpc.removeProperty("Listener");
            assertTrue("Listener UserDefined Remove", listener.reset());
            xMB.removeModifyListener(listener);
            udpc.addProperty("Listener2", PropertyAttribute.REMOVEABLE, "foo");
            assertTrue("Removed Listener UserDefined Add", !listener.reset());

            System.out.println("...done");

        } catch (Exception e) {
            report(e);
        }
    }

    // grrr...
    boolean eqDateTime(DateTime a, DateTime b) {
        return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day
            && a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.HundredthSeconds == b.HundredthSeconds;
    }

    boolean eqDate(Date a, Date b) {
        return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day;
    }

    boolean eqTime(Time a, Time b) {
        return a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.HundredthSeconds == b.HundredthSeconds;
    }

    boolean eqDuration(Duration a, Duration b) {
        return a.Years == b.Years && a.Months == b.Months && a.Days == b.Days
            && a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.MilliSeconds == b.MilliSeconds
            && a.Negative == b.Negative;
    }

    java.util.Collection fromArray(Object[] os) {
        java.util.Collection ret = new java.util.HashSet();
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

    public void report(Exception e) {
        System.out.println("Exception occurred:");
        e.printStackTrace();
        fail();
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


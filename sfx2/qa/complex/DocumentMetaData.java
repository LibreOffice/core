/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentMetaData.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:59:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package complex.framework;

import complexlib.ComplexTestCase;
import helper.StreamSimulator;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.Any;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.Locale;
import com.sun.star.lang.EventObject;
import com.sun.star.util.Time;
import com.sun.star.util.DateTime;
import com.sun.star.util.XModifyListener;
import com.sun.star.util.XModifyBroadcaster;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.IllegalTypeException;
import com.sun.star.embed.XStorage;
import com.sun.star.io.XInputStream;
import com.sun.star.document.XDocumentProperties;


/**
 * Test case for the service com.sun.star.document.DocumentProperties.
 * Currently, this service is implemented in
 * sfx2/source/doc/SfxDocumentMetaData.cxx.
 *
 * @author mst
 */
public class DocumentMetaData extends ComplexTestCase
{
    public String[] getTestMethodNames () {
        return new String[] { "check", "cleanup" };
    }

    public void cleanup() {
                // nothing to do
    }

    // for testing modifications
    class Listener implements XModifyListener {
        private boolean m_Called;

        public Listener() {
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

    public void check() {
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory) param.getMSF();
            assure("could not create MultiServiceFactory.", xMSF != null);
            XPropertySet xPropertySet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, xMSF);
            Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
            XComponentContext xContext = (XComponentContext)
                UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
            assure("could not get component context.", xContext != null);

            String temp = util.utils.getOfficeTemp/*Dir*/(xMSF);
            log.println("tempdir: " + temp);

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

            log.println("Creating service DocumentProperties...");

            Object oDP =
//                xMSF.createInstanceWithContext(
//                    "com.sun.star.document.DocumentProperties", xContext);
                xMSF.createInstance("com.sun.star.document.DocumentProperties");
            XDocumentProperties xDP = (XDocumentProperties)
                UnoRuntime.queryInterface(XDocumentProperties.class, oDP);

            log.println("...done");


            log.println("Checking initialize ...");

            XDocumentProperties xDP2 = (XDocumentProperties)
                UnoRuntime.queryInterface(XDocumentProperties.class,
                    xMSF.createInstance(
                        "com.sun.star.document.DocumentProperties"));
            XInitialization xInit = (XInitialization)
                UnoRuntime.queryInterface(XInitialization.class, xDP2);
            xInit.initialize(new Object[] { });

            log.println("...done");

            log.println("Checking storing default-initialized meta data ...");

//            xDP2.storeToMedium(temp + "EMPTY.odt", mimeArgs);
            xDP2.storeToMedium("", mimeEmptyArgs);

            log.println("...done");

            log.println("Checking loading default-initialized meta data ...");

//            xDP2.loadFromMedium(temp + "EMPTY.odt", noArgs);
            xDP2.loadFromMedium("", mimeEmptyArgs);
            assure ("Author", "".equals(xDP2.getAuthor()));

            log.println("...done");

            log.println("(Not) Checking preservation of custom meta data ...");

            xDP2.loadFromMedium(util.utils.getFullTestURL("CUSTOM.odt"),
                noArgs);
            assure ("Author", "".equals(xDP2.getAuthor()));
            xDP2.storeToMedium(temp + "CUSTOM.odt", mimeArgs);

            //FIXME: now what? comparing for binary equality seems useless
            // we could unzip the written file and grep for the custom stuff
            // but would that work on windows...

            log.println("...done");

            log.println("Checking loading from test document...");

            String file = util.utils.getFullTestURL("TEST.odt");
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

            log.println("...done");

            log.println("Checking meta-data import...");

            assure ("Author", "Karl-Heinz Mustermann".equals(xDP.getAuthor()));
            assure ("Generator",
            "StarOffice/8$Solaris_x86 OpenOffice.org_project/680m232$Build-9227"
                .equals(xDP.getGenerator()));
            assure ("CreationDate", 2007 == xDP.getCreationDate().Year);
            assure ("Title", "Urgent Memo".equals(xDP.getTitle()));
            assure ("Subject", "Wichtige Mitteilung".equals(xDP.getSubject()));
            assure ("Description",
                "Modern internal company memorandum in full-blocked style"
                .equals(xDP.getDescription()));
//            assure ("Language", "".equals(xDP.getLanguage()));
            assure ("ModifiedBy",
                "Karl-Heinz Mustermann".equals(xDP.getModifiedBy()));
            assure ("ModificationDate", 10 == xDP.getModificationDate().Month);
            assure ("PrintedBy",
                "Karl-Heinz Mustermann".equals(xDP.getPrintedBy()));
            assure ("PrintDate", 29 == xDP.getPrintDate().Day);
            assure ("TemplateName",
                "Modern Memo".equals(xDP.getTemplateName()));
            assure ("TemplateURL",
                xDP.getTemplateURL().endsWith("memmodern.ott"));
            assure ("TemplateDate", 17 == xDP.getTemplateDate().Hours);
            assure ("AutoloadURL", "../TEST.odt".equals(xDP.getAutoloadURL()));
            assure ("AutoloadSecs", 0 == xDP.getAutoloadSecs());
            assure ("DefaultTarget", "_blank".equals(xDP.getDefaultTarget()));
            assure ("EditingCycles", 3 == xDP.getEditingCycles());
            assure ("EditingDuration", 320 == xDP.getEditingDuration());

            String[] kws = xDP.getKeywords();
            assure ("Keywords", fromArray(kws).containsAll(
                    fromArray(new Object[] { "Asien", "Memo", "Reis" })));

            NamedValue[] ds = xDP.getDocumentStatistics();
/*            for (int i = 0; i < ds.length; ++i) {
                log.println("nv: " + ds[i].Name + " " + ds[i].Value);
            }
            NamedValue nv1 = new NamedValue("WordCount", new Integer(23));
            NamedValue nv2 = new NamedValue("WordCount", new Integer(23));
            log.println("eq: " + nv1.equals(nv2)); // grrr, this is false...
*/
            assure ("DocumentStatistics:WordCount", containsNV(ds,
                        new NamedValue("WordCount", new Integer(23))));
            assure ("DocumentStatistics:PageCount", containsNV(ds,
                        new NamedValue("PageCount", new Integer(1))));

            XPropertyContainer udpc = xDP.getUserDefinedProperties();
            XPropertySet udps = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, udpc);
            assure ("UserDefined 1", "Dies ist ein wichtiger Hinweis"
                    .equals(udps.getPropertyValue("Hinweis")));
            assure ("UserDefined 2", ("Kann Spuren von N"
                + new String(new byte[] { (byte) 0xc3, (byte) 0xbc }, "UTF-8")
                + "ssen enthalten")
                    .equals(udps.getPropertyValue("Warnung")));

            log.println("...done");

            log.println("Checking meta-data updates...");

            String str;
            DateTime dt = new DateTime();
            Locale l = new Locale();
            int i;

            str = "me";
            xDP.setAuthor(str);
            assure ("setAuthor", str.equals(xDP.getAuthor()));
            str = "the computa";
            xDP.setGenerator(str);
            assure ("setGenerator", str.equals(xDP.getGenerator()));
            dt.Year = 2038;
            dt.Month = 1;
            dt.Day = 1;
            xDP.setCreationDate(dt);
            assure ("setCreationDate", dt.Year == xDP.getCreationDate().Year);
            str = "El t'itulo";
            xDP.setTitle(str);
            assure ("setTitle", str.equals(xDP.getTitle()));
            str = "Ein verkommenes Subjekt";
            xDP.setSubject(str);
            assure ("setSubject", str.equals(xDP.getSubject()));
            str = "Este descripci'on no es importante";
            xDP.setDescription(str);
            assure ("setDescription", str.equals(xDP.getDescription()));
            l.Language = "en";
            l.Country = "GB";
            xDP.setLanguage(l);
            Locale l2 = xDP.getLanguage();
            assure ("setLanguage Lang", l.Language.equals(l2.Language));
            assure ("setLanguage Cty", l.Country.equals(l2.Country));
            str = "myself";
            xDP.setModifiedBy(str);
            assure ("setModifiedBy", str.equals(xDP.getModifiedBy()));
            dt.Year = 2042;
            xDP.setModificationDate(dt);
            assure ("setModificationDate",
                dt.Year == xDP.getModificationDate().Year);
            str = "i didnt do it";
            xDP.setPrintedBy(str);
            assure ("setPrintedBy", str.equals(xDP.getPrintedBy()));
            dt.Year = 2024;
            xDP.setPrintDate(dt);
            assure ("setPrintDate", dt.Year == xDP.getPrintDate().Year);
            str = "blah";
            xDP.setTemplateName(str);
            assure ("setTemplateName", str.equals(xDP.getTemplateName()));
            str = "gopher://some-hole-in-the-ground/";
            xDP.setTemplateURL(str);
            assure ("setTemplateURL", str.equals(xDP.getTemplateURL()));
            dt.Year = 2043;
            xDP.setTemplateDate(dt);
            assure ("setTemplateDate", dt.Year == xDP.getTemplateDate().Year);
            str = "http://nowhere/";
            xDP.setAutoloadURL(str);
            assure ("setAutoloadURL", str.equals(xDP.getAutoloadURL()));
            i = 3661; // this might not work (due to conversion via double...)
            xDP.setAutoloadSecs(i);
//            log.println("set: " + i + " get: " + xDP.getAutoloadSecs());
            assure ("setAutoloadSecs", i == xDP.getAutoloadSecs());
            str = "_blank";
            xDP.setDefaultTarget(str);
            assure ("setDefaultTarget", str.equals(xDP.getDefaultTarget()));
            i = 42;
            xDP.setEditingCycles((short) i);
            assure ("setEditingCycles", i == xDP.getEditingCycles());
            i = 84;
            xDP.setEditingDuration(i);
            assure ("setEditingDuration", i == xDP.getEditingDuration());
            str = "";

            String[] kws2 = new String[] {
                "keywordly", "keywordlike", "keywordalicious" };
            xDP.setKeywords(kws2);
            kws = xDP.getKeywords();
            assure ("setKeywords", fromArray(kws).containsAll(fromArray(kws2)));

            NamedValue[] ds2 = new NamedValue[] {
                    new NamedValue("SyllableCount", new Integer(9)),
                    new NamedValue("FrameCount", new Integer(2)),
                    new NamedValue("SentenceCount", new Integer(7)) };
            xDP.setDocumentStatistics(ds2);
            ds = xDP.getDocumentStatistics();
            assure ("setDocumentStatistics:SyllableCount", containsNV(ds,
                        new NamedValue("SyllableCount", new Integer(9))));
            assure ("setDocumentStatistics:FrameCount", containsNV(ds,
                        new NamedValue("FrameCount", new Integer(2))));
            assure ("setDocumentStatistics:SentenceCount", containsNV(ds,
                        new NamedValue("SentenceCount", new Integer(7))));

            log.println("...done");

            log.println("Checking user-defined meta-data updates...");

            // actually, this tests the PropertyBag service
            // but maybe the DocumentProperties service will be implemented
            // differently some day...
            boolean b = true;
            double d = 3.1415;
            Time t = new Time();
            t.Hours = 1;
            t.Minutes = 16;
            dt.Year = 2065;

            udpc.addProperty("Frobnicate", PropertyAttribute.REMOVEABLE,
                new Boolean(b));
            udpc.addProperty("FrobDuration", PropertyAttribute.REMOVEABLE, t);
            udpc.addProperty("FrobStartTime", PropertyAttribute.REMOVEABLE, dt);
            udpc.addProperty("Pi", PropertyAttribute.REMOVEABLE, new Double(d));
            udpc.addProperty("Foo", PropertyAttribute.REMOVEABLE, "bar");
            udpc.addProperty("Removed", PropertyAttribute.REMOVEABLE, "bar");
            try {
                udpc.removeProperty("Info 1");
                udpc.removeProperty("Removed");
            } catch (UnknownPropertyException e) {
                assure("removeProperty failed", false);
            }

            try {
                udpc.addProperty("Forbidden", PropertyAttribute.REMOVEABLE,
                    new String[] { "foo", "bar" });
                assure("inserting value of non-supported type did not fail",
                    false);
            } catch (IllegalTypeException e) {
                // ignore
            }

            assure ("UserDefined bool", new Boolean(b).equals(
                    udps.getPropertyValue("Frobnicate")));
            assure ("UserDefined time", eqTime(t, (Time)
                    udps.getPropertyValue("FrobDuration")));
            assure ("UserDefined date", eqDateTime(dt, (DateTime)
                    udps.getPropertyValue("FrobStartTime")));
            assure ("UserDefined float", new Double(d).equals(
                    udps.getPropertyValue("Pi")));
            assure ("UserDefined string", "bar".equals(
                    udps.getPropertyValue("Foo")));

            try {
                udps.getPropertyValue("Removed");
                assure("UserDefined remove didn't", false);
            } catch (UnknownPropertyException e) {
                // ok
            }

            log.println("...done");

            log.println("Checking storing meta-data to file...");

            xDP.storeToMedium(temp + "TEST.odt", mimeArgs);

            log.println("...done");

            log.println("Checking loading meta-data from stored file...");

            xDP.loadFromMedium(temp + "TEST.odt", noArgs);

            log.println("...done");

            log.println("Checking user-defined meta-data from stored file...");

            udpc = xDP.getUserDefinedProperties();
            udps = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, udpc);

            assure ("UserDefined bool", new Boolean(b).equals(
                    udps.getPropertyValue("Frobnicate")));
            assure ("UserDefined time", eqTime(t, (Time)
                    udps.getPropertyValue("FrobDuration")));
            assure ("UserDefined date", eqDateTime(dt, (DateTime)
                    udps.getPropertyValue("FrobStartTime")));
            assure ("UserDefined float", new Double(d).equals(
                    udps.getPropertyValue("Pi")));
            assure ("UserDefined string", "bar".equals(
                    udps.getPropertyValue("Foo")));

            try {
                udps.getPropertyValue("Removed");
                assure("UserDefined remove didn't", false);
            } catch (UnknownPropertyException e) {
                // ok
            }

            log.println("...done");

            log.println("Checking notification listener interface...");

            Listener listener = new Listener();
            XModifyBroadcaster xMB = (XModifyBroadcaster)
                UnoRuntime.queryInterface(XModifyBroadcaster.class, xDP);
            xMB.addModifyListener(listener);
            xDP.setAuthor("not me");
            assure ("Listener Author", listener.reset());
            udpc.addProperty("Listener", PropertyAttribute.REMOVEABLE, "foo");
            assure ("Listener UserDefined Add", listener.reset());
            udps.setPropertyValue("Listener", "bar");
            assure ("Listener UserDefined Set", listener.reset());
            udpc.removeProperty("Listener");
            assure ("Listener UserDefined Remove", listener.reset());
            xMB.removeModifyListener(listener);
            udpc.addProperty("Listener2", PropertyAttribute.REMOVEABLE, "foo");
            assure ("Removed Listener UserDefined Add", !listener.reset());

            log.println("...done");

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

    boolean eqTime(Time a, Time b) {
        return a.Hours == b.Hours && a.Minutes == b.Minutes
            && a.Seconds == b.Seconds
            && a.HundredthSeconds == b.HundredthSeconds;
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
        log.println("Exception occurred:");
        e.printStackTrace((java.io.PrintWriter) log);
        failed();
    }
}


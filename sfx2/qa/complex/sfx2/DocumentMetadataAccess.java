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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import helper.StreamSimulator;

import lib.TestParameters;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import com.sun.star.beans.Pair;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.StringPair;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.rdf.BlankNode;
import com.sun.star.rdf.FileFormat;
import com.sun.star.rdf.Literal;
import com.sun.star.rdf.Statement;
import com.sun.star.rdf.URI;
import com.sun.star.rdf.URIs;
import com.sun.star.rdf.XBlankNode;
import com.sun.star.rdf.XDocumentMetadataAccess;
import com.sun.star.rdf.XDocumentRepository;
import com.sun.star.rdf.XLiteral;
import com.sun.star.rdf.XMetadatable;
import com.sun.star.rdf.XNamedGraph;
import com.sun.star.rdf.XNode;
import com.sun.star.rdf.XQuerySelectResult;
import com.sun.star.rdf.XRepository;
import com.sun.star.rdf.XRepositorySupplier;
import com.sun.star.rdf.XURI;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;
import complex.sfx2.tools.TestDocument;

/**
 * Test case for interface com.sun.star.rdf.XDocumentMetadataAccess
 * Currently, this service is implemented in
 * sfx2/source/doc/DocumentMetadataAccess.cxx
 *
 * Actually, this is not a service, so we need to create a document and
 * go from there...
 *
 */
public class DocumentMetadataAccess
{
    XMultiServiceFactory xMSF;
    XComponentContext xContext;
    String tempDir;

    String nsRDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    String nsRDFS = "http://www.w3.org/2000/01/rdf-schema#";
    String nsPkg="http://docs.oasis-open.org/opendocument/meta/package/common#";
    String nsODF ="http://docs.oasis-open.org/opendocument/meta/package/odf#";

    XURI foo;
    XURI bar;
    XURI baz;

    static XURI rdf_type;
    static XURI rdfs_label;
    static XURI pkg_Document;
    static XURI pkg_hasPart;
    static XURI pkg_MetadataFile;
    static XURI odf_ContentFile;
    static XURI odf_StylesFile;
    static XURI odf_Element;
    static XBlankNode blank1;
    static XBlankNode blank2;
    static XBlankNode blank3;
    static XBlankNode blank4;
    static String manifestPath = "manifest.rdf";
    static String contentPath = "content.xml";
    static String stylesPath = "styles.xml";
    static String fooPath = "foo.rdf";
    static String fooBarPath = "meta/foo/bar.rdf";

    XRepository xRep;
    XRepositorySupplier xRS;
    XDocumentMetadataAccess xDMA;

    /**
     * The test parameters
     */
    private static TestParameters param = null;

    @Before public void before() throws Exception
    {
        xMSF = getMSF();
        param = new TestParameters();
        param.put("ServiceFactory", xMSF);  // important for param.getMSF()

        assertNotNull("could not create MultiServiceFactory.", xMSF);
        XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
        xContext = UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
        assertNotNull("could not get component context.", xContext);

        tempDir = util.utils.getOfficeTemp/*Dir*/(xMSF);
        System.out.println("tempdir: " + tempDir);

        foo = URI.create(xContext, "uri:foo");
        assertNotNull("foo", foo);
        bar = URI.create(xContext, "uri:bar");
        assertNotNull("bar", bar);
        baz = URI.create(xContext, "uri:baz");
        assertNotNull("baz", baz);

        blank1 = BlankNode.create(xContext, "_:1");
        assertNotNull("blank1", blank1);
        blank2 = BlankNode.create(xContext, "_:2");
        assertNotNull("blank2", blank2);
        blank3 = BlankNode.create(xContext, "_:3");
        assertNotNull("blank3", blank3);
        blank4 = BlankNode.create(xContext, "_:4");
        assertNotNull("blank4", blank4);
        rdf_type = URI.createKnown(xContext, URIs.RDF_TYPE);
        assertNotNull("rdf_type", rdf_type);
        rdfs_label = URI.createKnown(xContext, URIs.RDFS_LABEL);
        assertNotNull("rdfs_label", rdfs_label);
        pkg_Document = URI.createKnown(xContext, URIs.PKG_DOCUMENT);
        assertNotNull("pkg_Document", pkg_Document);
        pkg_hasPart = URI.createKnown(xContext, URIs.PKG_HASPART);
        assertNotNull("pkg_hasPart", pkg_hasPart);
        pkg_MetadataFile = URI.createKnown(xContext, URIs.PKG_METADATAFILE);
        assertNotNull("pkg_MetadataFile", pkg_MetadataFile);
        odf_ContentFile = URI.createKnown(xContext, URIs.ODF_CONTENTFILE);
        assertNotNull("odf_ContentFile", odf_ContentFile);
        odf_StylesFile = URI.createKnown(xContext, URIs.ODF_STYLESFILE);
        assertNotNull("odf_StylesFile", odf_StylesFile);
        odf_Element = URI.createKnown(xContext, URIs.ODF_ELEMENT);
        assertNotNull("odf_Element", odf_Element);
    }

    @After public void after()
    {
        xRep = null;
        xRS  = null;
        xDMA = null;
    }

    @Test public void check() throws Exception
    {
        XComponent xComp = null;
        XComponent xComp2 = null;
        try {
            XEnumeration xStmtsEnum;
            XNamedGraph xManifest;

            System.out.println("Creating document with Repository...");

            // we cannot create a XDMA directly, we must create
            // a document and get it from there :(
            // create document
            PropertyValue[] loadProps = new PropertyValue[1];
            loadProps[0] = new PropertyValue();
            loadProps[0].Name = "Hidden";
            loadProps[0].Value = true;
            xComp = util.DesktopTools.openNewDoc(xMSF, "swriter", loadProps);
            XTextDocument xText = UnoRuntime.queryInterface(XTextDocument.class, xComp);

            XRepositorySupplier xRepoSupplier = UnoRuntime.queryInterface(XRepositorySupplier.class, xComp);
            assertNotNull("xRS null", xRepoSupplier);
            XDocumentMetadataAccess xDocMDAccess = UnoRuntime.queryInterface(XDocumentMetadataAccess.class, xRepoSupplier);
            assertNotNull("xDMA null", xDocMDAccess);
            xRep = xRepoSupplier.getRDFRepository();
            assertNotNull("xRep null", xRep);

            System.out.println("...done");

            System.out.println("Checking that new repository is initialized...");

            XURI xBaseURI = xDocMDAccess;
            String baseURI = xBaseURI.getStringValue();
            assertNotNull("new: baseURI", xBaseURI );
            assertTrue("new: baseURI", !xBaseURI.getStringValue().equals(""));

            assertTrue("new: # graphs", 1 == xRep.getGraphNames().length);
            XURI manifest = URI.createNS(xContext, xBaseURI.getStringValue(),
                manifestPath);
            xManifest = xRep.getGraph(manifest);
            assertTrue("new: manifest graph", null != xManifest);

            Statement[] manifestStmts = getManifestStmts(xBaseURI);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assertTrue("new: manifest graph", eq(xStmtsEnum, manifestStmts));

            System.out.println("...done");

            System.out.println("Checking some invalid args...");

            String content = "behold, for I am the content.";
            new TestRange(content);

            try {
                xDocMDAccess.getElementByURI(null);
                fail("getElementByURI: null allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.getMetadataGraphsWithType(null);
                fail("getMetadataGraphsWithType: null URI allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("", new XURI[0]);
                fail("addMetadataFile: empty filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("/foo", new XURI[0]);
                fail("addMetadataFile: absolute filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("fo\"o", new XURI[0]);
                fail("addMetadataFile: invalid filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("../foo", new XURI[0]);
                fail("addMetadataFile: filename with .. allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("foo/../../bar", new XURI[0]);
                fail("addMetadataFile: filename with nest .. allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("foo/././bar", new XURI[0]);
                fail("addMetadataFile: filename with nest . allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("content.xml", new XURI[0]);
                fail("addMetadataFile: content.xml allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("styles.xml", new XURI[0]);
                fail("addMetadataFile: styles.xml allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("meta.xml", new XURI[0]);
                fail("addMetadataFile: meta.xml allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addMetadataFile("settings.xml", new XURI[0]);
                fail("addMetadataFile: settings.xml allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.importMetadataFile(FileFormat.RDF_XML, null, "foo",
                    foo, new XURI[0]);
                fail("importMetadataFile: null stream allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }

            final String sEmptyRDF = TestDocument.getUrl("empty.rdf");
            try {
                XInputStream xFooIn = new StreamSimulator(sEmptyRDF, true, param);
                xDocMDAccess.importMetadataFile(FileFormat.RDF_XML, xFooIn, "",
                    foo, new XURI[0]);
                fail("importMetadataFile: empty filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(sEmptyRDF, true, param);
                xDocMDAccess.importMetadataFile(FileFormat.RDF_XML, xFooIn, "meta.xml",
                    foo, new XURI[0]);
                fail("importMetadataFile: meta.xml filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(sEmptyRDF, true, param);
                xDocMDAccess.importMetadataFile(FileFormat.RDF_XML,
                    xFooIn, "foo", null, new XURI[0]);
                fail("importMetadataFile: null base URI allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(sEmptyRDF, true, param);
                xDocMDAccess.importMetadataFile(FileFormat.RDF_XML,
                    xFooIn, "foo", rdf_type, new XURI[0]);
                fail("importMetadataFile: non-absolute base URI allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.removeMetadataFile(null);
                fail("removeMetadataFile: null URI allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addContentOrStylesFile("");
                fail("addContentOrStylesFile: empty filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addContentOrStylesFile("/content.xml");
                fail("addContentOrStylesFile: absolute filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.addContentOrStylesFile("foo.rdf");
                fail("addContentOrStylesFile: invalid filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.removeContentOrStylesFile("");
                fail("removeContentOrStylesFile: empty filename allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.loadMetadataFromStorage(null, foo, null);
                fail("loadMetadataFromStorage: null storage allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.storeMetadataToStorage(null/*, base*/);
                fail("storeMetadataToStorage: null storage allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.loadMetadataFromMedium(new PropertyValue[0]);
                fail("loadMetadataFromMedium: empty medium allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDocMDAccess.storeMetadataToMedium(new PropertyValue[0]);
                fail("storeMetadataToMedium: empty medium allowed");
            } catch (IllegalArgumentException e) {
                // ignore
            }

            System.out.println("...done");

            System.out.println("Checking file addition/removal...");

            xDocMDAccess.removeContentOrStylesFile(contentPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("removeContentOrStylesFile (content)",
                eq(xStmtsEnum, new Statement[] {
                        manifestStmts[0], manifestStmts[2], manifestStmts[4]
                    }));

            xDocMDAccess.addContentOrStylesFile(contentPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("addContentOrStylesFile (content)",
                eq(xStmtsEnum, manifestStmts));

            xDocMDAccess.removeContentOrStylesFile(stylesPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("removeContentOrStylesFile (styles)",
                eq(xStmtsEnum, new Statement[] {
                        manifestStmts[0], manifestStmts[1], manifestStmts[3]
                    }));

            xDocMDAccess.addContentOrStylesFile(stylesPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("addContentOrStylesFile (styles)",
                eq(xStmtsEnum, manifestStmts));

            XURI xFoo = URI.createNS(xContext, xBaseURI.getStringValue(),
                fooPath);
            Statement xM_BaseHaspartFoo =
                new Statement(xBaseURI, pkg_hasPart, xFoo, manifest);
            Statement xM_FooTypeMetadata =
                new Statement(xFoo, rdf_type, pkg_MetadataFile, manifest);
            Statement xM_FooTypeBar =
                new Statement(xFoo, rdf_type, bar, manifest);
            xDocMDAccess.addMetadataFile(fooPath, new XURI[] { bar });
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("addMetadataFile",
                eq(xStmtsEnum, merge(manifestStmts, new Statement[] {
                        xM_BaseHaspartFoo, xM_FooTypeMetadata, xM_FooTypeBar
                    })));

            XURI[] graphsBar = xDocMDAccess.getMetadataGraphsWithType(bar);
            assertTrue("getMetadataGraphsWithType",
                graphsBar.length == 1 && eq(graphsBar[0], xFoo));


            xDocMDAccess.removeMetadataFile(xFoo);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assertTrue("removeMetadataFile",
                eq(xStmtsEnum, manifestStmts));

            System.out.println("...done");

            System.out.println("Checking mapping...");

            XEnumerationAccess xTextEnum = UnoRuntime.queryInterface(XEnumerationAccess.class, xText.getText());
            Object o = xTextEnum.createEnumeration().nextElement();
            XMetadatable xMeta1 = UnoRuntime.queryInterface(XMetadatable.class, o);

            XMetadatable xMeta;
            xMeta = xDocMDAccess.getElementByURI(xMeta1);
            assertTrue("getElementByURI: null", null != xMeta);
            String XmlId = xMeta.getMetadataReference().Second;
            String XmlId1 = xMeta1.getMetadataReference().Second;
            assertTrue("getElementByURI: no xml id", !XmlId.equals(""));
            assertTrue("getElementByURI: different xml id", XmlId.equals(XmlId1));

            System.out.println("...done");

            System.out.println("Checking storing and loading...");

            XURI xFoobar = URI.createNS(xContext, xBaseURI.getStringValue(),
                fooBarPath);
            Statement[] metadataStmts = getMetadataFileStmts(xBaseURI,
                fooBarPath);
            xDocMDAccess.addMetadataFile(fooBarPath, new XURI[0]);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assertTrue("addMetadataFile",
                eq(xStmtsEnum, merge(manifestStmts, metadataStmts )));

            Statement xFoobar_FooBarFoo =
                new Statement(foo, bar, foo, xFoobar);
            xRep.getGraph(xFoobar).addStatement(foo, bar, foo);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assertTrue("addStatement",
                eq(xStmtsEnum, merge(manifestStmts, merge(metadataStmts,
                    new Statement[] { xFoobar_FooBarFoo }))));

            PropertyValue noMDNoContentFile = new PropertyValue();
            noMDNoContentFile.Name = "URL";
            noMDNoContentFile.Value = TestDocument.getUrl("CUSTOM.odt");
            PropertyValue noMDFile = new PropertyValue();
            noMDFile.Name = "URL";
            noMDFile.Value = TestDocument.getUrl("TEST.odt");
            PropertyValue file = new PropertyValue();
            file.Name = "URL";
            file.Value = tempDir + "TESTDMA.odt";
            PropertyValue mimetype = new PropertyValue();
            mimetype.Name = "MediaType";
            mimetype.Value = "application/vnd.oasis.opendocument.text";
            PropertyValue[] argsEmptyNoContent = { mimetype, noMDNoContentFile};
            PropertyValue[] argsEmpty = { mimetype, noMDFile };
            PropertyValue[] args = { mimetype, file };

            xStmtsEnum = xRep.getStatements(null, null, null);
            XURI[] graphs = xRep.getGraphNames();

            xDocMDAccess.storeMetadataToMedium(args);

            // this should re-init
            xDocMDAccess.loadMetadataFromMedium(argsEmptyNoContent);
            xRep = xRepoSupplier.getRDFRepository();
            assertTrue("xRep null", null != xRep);
            assertTrue("baseURI still tdoc?",
                !baseURI.equals(xDocMDAccess.getStringValue()));
            Statement[] manifestStmts2 = getManifestStmts(xDocMDAccess);
            xStmtsEnum = xRep.getStatements(null, null, null);
            // there is no content or styles file in here, so we have just
            // the package stmt
            assertTrue("loadMetadataFromMedium (no metadata, no content)",
                eq(xStmtsEnum, new Statement[] { manifestStmts2[0] }));

            // this should re-init
            xDocMDAccess.loadMetadataFromMedium(argsEmpty);
            xRep = xRepoSupplier.getRDFRepository();
            assertTrue("xRep null", null != xRep);
            assertTrue("baseURI still tdoc?",
                !baseURI.equals(xDocMDAccess.getStringValue()));
            Statement[] manifestStmts3 = getManifestStmts(xDocMDAccess);

            xStmtsEnum = xRep.getStatements(null, null, null);
            assertTrue("loadMetadataFromMedium (no metadata)",
                eq(xStmtsEnum, manifestStmts3));

            xDocMDAccess.loadMetadataFromMedium(args);
            xRep = xRepoSupplier.getRDFRepository();
            assertTrue("xRep null", null != xRep);
            Statement[] manifestStmts4 = getManifestStmts(xDocMDAccess);
            Statement[] metadataStmts4 = getMetadataFileStmts(xDocMDAccess,
                fooBarPath);

            xStmtsEnum = xRep.getStatements(null, null, null);
            assertTrue("some graph(s) not reloaded",
                graphs.length == xRep.getGraphNames().length);

            XURI xFoobar4 = URI.createNS(xContext, xDocMDAccess.getStringValue(),
                fooBarPath);
            Statement xFoobar_FooBarFoo4 =
                new Statement(foo, bar, foo, xFoobar4);
            assertTrue("loadMetadataFromMedium (re-load)",
                eq(xStmtsEnum, merge(manifestStmts4, merge(metadataStmts4,
                        new Statement[] { xFoobar_FooBarFoo4 }))));

            System.out.println("...done");

            System.out.println("Checking storing and loading via model...");

            String f = tempDir + "TESTPARA.odt";

            XStorable xStor = UnoRuntime.queryInterface(XStorable.class, xRepoSupplier);

            xStor.storeToURL(f, new PropertyValue[0]);

            xComp2 = util.DesktopTools.loadDoc(xMSF, f, loadProps);

            XDocumentMetadataAccess xDMA2 = UnoRuntime.queryInterface(XDocumentMetadataAccess.class, xComp2);
            assertTrue("xDMA2 null", null != xDMA2);

            XRepositorySupplier xRS2 = UnoRuntime.queryInterface(XRepositorySupplier.class, xComp2);
            assertTrue("xRS2 null", null != xRS2);

            XRepository xRep2 = xRS2.getRDFRepository();
            assertTrue("xRep2 null", null != xRep2);

            Statement[] manifestStmts5 = getManifestStmts(xDMA2);
            Statement[] metadataStmts5 = getMetadataFileStmts(xDMA2,
                fooBarPath);
            XURI xFoobar5 = URI.createNS(xContext, xDMA2.getStringValue(),
                fooBarPath);
            Statement xFoobar_FooBarFoo5 =
                new Statement(foo, bar, foo, xFoobar5);
            xStmtsEnum = xRep.getStatements(null, null, null);
            XEnumeration xStmtsEnum2 = xRep2.getStatements(null, null, null);
            assertTrue("load: repository differs",
                eq(xStmtsEnum2, merge(manifestStmts5, merge(metadataStmts5,
                        new Statement[] { xFoobar_FooBarFoo5 }))));

            System.out.println("...done");

        } finally {
            close(xComp);
            close(xComp2);
        }
    }

    @Test public void checkRDFa() throws Exception
    {
        XComponent xComp = null;
        try {
            final String file = TestDocument.getUrl("TESTRDFA.odt");
            xComp = loadRDFa(file);
            if (xComp != null)
            {
                final String sNewFile = tempDir + "TESTRDFA.odt";
                storeRDFa(xComp, sNewFile);
                close(xComp);

                xComp = loadRDFa(sNewFile);
            }
        } finally {
            close(xComp);
        }
    }

    @Test
    public void checkTdf123293() throws Exception
    {
        XComponent xComp = null;
        try {
            xComp = util.DesktopTools.loadDocUsingStream(xMSF, TestDocument.getPath("TESTRDFA.odt"));

            // Metadata was discarded when loading from stream, make sure it's there now
            XRepositorySupplier xRepoSupplier = UnoRuntime.queryInterface(XRepositorySupplier.class, xComp);
            assertNotNull("No metadata loaded", xRepoSupplier);
        } finally {
            close(xComp);
        }
    }

    private void storeRDFa(XComponent xComp, String file) throws com.sun.star.io.IOException
    {
        System.out.println("Storing test document...");

        XStorable xStor = UnoRuntime.queryInterface(XStorable.class, xComp);

        xStor.storeToURL(file, new PropertyValue[0]);

        System.out.println("...done");
    }

    private XComponent loadRDFa(String file) throws Exception
    {
        XComponent xComp = null;

        System.out.println("Loading test document...");

        PropertyValue[] loadProps = new PropertyValue[1];
        loadProps[0] = new PropertyValue();
        loadProps[0].Name = "Hidden";
        loadProps[0].Value = true;



        xComp = util.DesktopTools.loadDoc(xMSF, file, loadProps);

        XRepositorySupplier xRepoSupplier = UnoRuntime.queryInterface(XRepositorySupplier.class, xComp);
        assertTrue("xRS null", null != xRepoSupplier);

        XDocumentRepository xDocRepository = UnoRuntime.queryInterface(XDocumentRepository.class, xRepoSupplier.getRDFRepository());
        assertTrue("xRep null", null != xDocRepository);

        XTextDocument xTextDoc = UnoRuntime.queryInterface(XTextDocument.class, xComp);

        XText xText = xTextDoc.getText();

        XEnumerationAccess xEA = UnoRuntime.queryInterface(XEnumerationAccess.class, xText);
        XEnumeration xEnum = xEA.createEnumeration();

        System.out.println("...done");

        System.out.println("Checking RDFa in loaded test document...");

        XMetadatable xPara;
        Pair<Statement[], Boolean> result;

        Statement x_FooBarLit1 = new Statement(foo, bar, mkLit("1"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 1",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit1
                }));

        Statement x_FooBarLit2 = new Statement(foo, bar, mkLit("2"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 2",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit2
                }));

        Statement x_BlankBarLit3 =
            new Statement(blank1, bar, mkLit("3"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 3",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_BlankBarLit3
                }));
        XBlankNode b3 = UnoRuntime.queryInterface(XBlankNode.class, result.First[0].Subject);

        Statement x_BlankBarLit4 =
            new Statement(blank2, bar, mkLit("4"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 4",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_BlankBarLit4
                }));
        XBlankNode b4 = UnoRuntime.queryInterface(XBlankNode.class, result.First[0].Subject);

        Statement x_BlankBarLit5 =
            new Statement(blank1, bar, mkLit("5"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 5",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_BlankBarLit5
                }));
        XBlankNode b5 = UnoRuntime.queryInterface(XBlankNode.class, result.First[0].Subject);

        assertTrue("RDFa: 3 != 4",
            !b3.getStringValue().equals(b4.getStringValue()));
        assertTrue("RDFa: 3 == 5",
             b3.getStringValue().equals(b5.getStringValue()));

        Statement x_FooBarLit6 = new Statement(foo, bar, mkLit("6"), null);
        Statement x_FooBazLit6 = new Statement(foo, baz, mkLit("6"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 6",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit6, x_FooBazLit6
                }));

        Statement x_FooBarLit7 = new Statement(foo, bar, mkLit("7"), null);
        Statement x_FooBazLit7 = new Statement(foo, baz, mkLit("7"), null);
        Statement x_FooFooLit7 = new Statement(foo, foo, mkLit("7"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 7",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit7, x_FooBazLit7, x_FooFooLit7
                }));

        XNode lit = mkLit("a fooish bar");
        XNode lit_type= mkLit("a fooish bar", bar);
        Statement x_FooBarLit = new Statement(foo, bar, lit, null);
        Statement x_FooBarLittype = new Statement(foo, bar, lit_type, null);

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 8",
            result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit
                }));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 9",
            result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit
                }));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 10",
            result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLittype
                }));

        Statement x_FooBarLit11
            = new Statement(foo, bar, mkLit("11", bar), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 11",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit11
                }));

        XURI xFile = URI.createNS(xContext, file, "/" + contentPath);
        Statement x_FileBarLit12 =
            new Statement(xFile, bar, mkLit("12"), null);
          xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 12",
            !result.Second &&
            eq(result.First, new Statement[] {
                    x_FileBarLit12
                }));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 13",
            result.Second &&
            eq(result.First, new Statement[] {
                    x_FooBarLit
                }));

        new Statement(foo, rdfs_label, mkLit("14"), null);
        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 14",
            result.Second &&
            eq(result.First, new Statement[] {
                    /* x_FooLabelLit14 */ x_FooBarLit
                }));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 15", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 16", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 17", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 18", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 19", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(
            XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 20", eq(result.First, new Statement[] { } ));

        xPara = UnoRuntime.queryInterface(
            XMetadatable.class, xEnum.nextElement());
        result = xDocRepository.getStatementRDFa(xPara);
        assertTrue("RDFa: 21", eq(result.First, new Statement[] { } ));

        System.out.println("...done");

        return xComp;
    }


// utilities -------------------------------------------------------------

    static void close(XComponent i_comp)
    {
        try {
            XCloseable xClos = UnoRuntime.queryInterface(XCloseable.class, i_comp);
            if (xClos != null)
            {
                xClos.close(true);
            }
        } catch (Exception e) {
        }
    }

    XLiteral mkLit(String i_content)
    {
        return Literal.create(xContext, i_content);
    }

    XLiteral mkLit(String i_content, XURI i_uri)
    {
        return Literal.createWithType(xContext, i_content, i_uri);
    }

    static Statement[] merge(Statement[] i_A1, Statement[] i_A2)
    {
        // bah, java sucks...
        Statement[] ret = new Statement[i_A1.length + i_A2.length];
        for (int i = 0; i < i_A1.length; ++i) {
            ret[i] = i_A1[i];
        }
        for (int i = 0; i < i_A2.length; ++i) {
            ret[i+i_A1.length] = i_A2[i];
        }
        return ret;
    }

    public static String toS(XNode n) {
        if (null == n)
        {
            return "< null >";
        }
        return n.getStringValue();
    }

    static boolean isBlank(XNode i_node)
    {
        XBlankNode blank = UnoRuntime.queryInterface(XBlankNode.class, i_node);
        return blank != null;
    }


    static Statement[] toSeq(XEnumeration i_Enum) throws Exception
    {
        java.util.Collection<Statement> c = new java.util.ArrayList<Statement>();
        while (i_Enum.hasMoreElements()) {
            Statement s = (Statement) i_Enum.nextElement();
            c.add(s);
        }
        // java sucks
        Object[] arr = c.toArray();
        Statement[] ret = new Statement[arr.length];
        for (int i = 0; i < arr.length; ++i) {
            ret[i] = (Statement) arr[i];
        }
        return ret;
    }

    static XNode[][] toSeqs(XEnumeration i_Enum) throws Exception
    {
        java.util.Collection<XNode[]> c = new java.util.ArrayList<XNode[]>();
        while (i_Enum.hasMoreElements()) {
            XNode[] s = (XNode[]) i_Enum.nextElement();
            c.add(s);
        }
        Object[] arr = c.toArray();
        XNode[][] ret = new XNode[arr.length][];
        for (int i = 0; i < arr.length; ++i) {
            ret[i] = (XNode[]) arr[i];
        }
        return ret;
    }

    private static class BindingComp implements java.util.Comparator<XNode[]>
    {
        public int compare(XNode[] left, XNode[] right)
        {
            if (left.length != right.length)
            {
                throw new RuntimeException();
            }
            for (int i = 0; i < left.length; ++i) {
                int eq = (left[i].getStringValue().compareTo(
                            right[i].getStringValue()));
                if (eq != 0)
                {
                    return eq;
                }
            }
            return 0;
        }
    }

    private static class StmtComp implements java.util.Comparator<Statement>
    {
        public int compare(Statement left, Statement right)
        {
            int eq;
            if ((eq = cmp(left.Graph,     right.Graph    )) != 0) return eq;
            if ((eq = cmp(left.Subject,   right.Subject  )) != 0) return eq;
            if ((eq = cmp(left.Predicate, right.Predicate)) != 0) return eq;
            if ((eq = cmp(left.Object,    right.Object   )) != 0) return eq;
            return 0;
        }

        private int cmp(XNode i_Left, XNode i_Right)
        {
            if (isBlank(i_Left)) {
                return isBlank(i_Right) ? 0 : 1;
            } else {
                if (isBlank(i_Right)) {
                    return -1;
                } else {
                    return toS(i_Left).compareTo(toS(i_Right));
                }
            }
        }
    }

    static boolean eq(Statement i_Left, Statement i_Right)
    {
        XURI lG = i_Left.Graph;
        XURI rG = i_Right.Graph;
        if (!eq(lG, rG)) {
            System.out.println("Graphs differ: " + toS(lG) + " != " + toS(rG));
            return false;
        }
        if (!eq(i_Left.Subject, i_Right.Subject)) {
            System.out.println("Subjects differ: " +
                i_Left.Subject.getStringValue() + " != " +
                i_Right.Subject.getStringValue());
            return false;
        }
        if (!eq(i_Left.Predicate, i_Right.Predicate)) {
            System.out.println("Predicates differ: " +
                i_Left.Predicate.getStringValue() + " != " +
                i_Right.Predicate.getStringValue());
            return false;
        }
        if (!eq(i_Left.Object, i_Right.Object)) {
            System.out.println("Objects differ: " +
                i_Left.Object.getStringValue() + " != " +
                i_Right.Object.getStringValue());
            return false;
        }
        return true;
    }

    static boolean eq(Statement[] i_Result, Statement[] i_Expected)
    {
        if (i_Result.length != i_Expected.length) {
            System.out.println("eq: different lengths: " + i_Result.length + " " +
                i_Expected.length);
            return false;
        }
        Statement[] expected = i_Expected.clone();
        java.util.Arrays.sort(i_Result, new StmtComp());
        java.util.Arrays.sort(expected, new StmtComp());
        for (int i = 0; i < expected.length; ++i)
        {
            // This is better for debug!
            final Statement a = i_Result[i];
            final Statement b = expected[i];
            final boolean cond = eq(a, b);
            if (!cond) return false;
        }
        return true;
    }

    static boolean eq(XEnumeration i_Enum, Statement[] i_Expected)
        throws Exception
    {
        Statement[] current = toSeq(i_Enum);
        return eq(current, i_Expected);
    }

    static boolean eq(XNode i_Left, XNode i_Right)
    {
        if (i_Left == null) {
            return (i_Right == null);
        } else {
            return (i_Right != null) &&
                (i_Left.getStringValue().equals(i_Right.getStringValue())
                // FIXME: hack: blank nodes considered equal
                || (isBlank(i_Left) && isBlank(i_Right)));
        }
    }

    static boolean eq(XQuerySelectResult i_Result,
            String[] i_Vars, XNode[][] i_Bindings) throws Exception
    {
        String[] vars =  i_Result.getBindingNames();
        XEnumeration iter = i_Result;
        XNode[][] bindings = toSeqs(iter);
        if (vars.length != i_Vars.length) {
            System.out.println("var lengths differ");
            return false;
        }
        if (bindings.length != i_Bindings.length) {
            System.out.println("binding lengths differ: " + i_Bindings.length +
                " vs " + bindings.length );
            return false;
        }
        java.util.Arrays.sort(bindings, new BindingComp());
        java.util.Arrays.sort(i_Bindings, new BindingComp());
        for (int i = 0; i < i_Bindings.length; ++i) {
            if (i_Bindings[i].length != i_Vars.length) {
                System.out.println("TEST ERROR!");
                throw new Exception();
            }
            if (bindings[i].length != i_Vars.length) {
                System.out.println("binding length and var length differ");
                return false;
            }
            for (int j = 0; j < i_Vars.length; ++j) {
                if (!eq(bindings[i][j], i_Bindings[i][j])) {
                    System.out.println("bindings differ: " +
                        toS(bindings[i][j]) + " != " + toS(i_Bindings[i][j]));
                    return false;
                }
            }
        }
        for (int i = 0; i < i_Vars.length; ++i) {
            if (!vars[i].equals(i_Vars[i])) {
                System.out.println("variable names differ: " +
                    vars[i] + " != " + i_Vars[i]);
                return false;
            }
        }
        return true;
    }

    static boolean eq(StringPair i_Left, StringPair i_Right)
    {
        return ((i_Left.First).equals(i_Right.First)) &&
            ((i_Left.Second).equals(i_Right.Second));
    }

    static String mkNamespace(String i_prefix, String i_namespace)
    {
        return "PREFIX " + i_prefix + ": <" + i_namespace + ">\n";
    }

    static String mkNss()
    {
        String namespaces = mkNamespace("rdf",
            "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
        namespaces += mkNamespace("pkg",
            "http://docs.oasis-open.org/opendocument/meta/package/common#");
        namespaces += mkNamespace("odf",
            "http://docs.oasis-open.org/opendocument/meta/package/odf#");
        return namespaces;
    }

    Statement[] getManifestStmts(XURI xBaseURI) throws Exception
    {
        XURI xManifest = URI.createNS(xContext, xBaseURI.getStringValue(),
            manifestPath);
        XURI xContent = URI.createNS(xContext, xBaseURI.getStringValue(),
            contentPath);
        XURI xStyles  = URI.createNS(xContext, xBaseURI.getStringValue(),
            stylesPath);
        Statement xM_BaseTypeDoc =
            new Statement(xBaseURI, rdf_type, pkg_Document, xManifest);
        Statement xM_BaseHaspartContent =
            new Statement(xBaseURI, pkg_hasPart, xContent, xManifest);
        Statement xM_BaseHaspartStyles =
            new Statement(xBaseURI, pkg_hasPart, xStyles, xManifest);
        Statement xM_ContentTypeContent =
            new Statement(xContent, rdf_type, odf_ContentFile, xManifest);
        Statement xM_StylesTypeStyles =
            new Statement(xStyles, rdf_type, odf_StylesFile, xManifest);
        return new Statement[] {
                xM_BaseTypeDoc, xM_BaseHaspartContent, xM_BaseHaspartStyles,
                xM_ContentTypeContent, xM_StylesTypeStyles
            };
    }

    Statement[] getMetadataFileStmts(XURI xBaseURI, String Path)
        throws Exception
    {
        XURI xManifest = URI.createNS(xContext, xBaseURI.getStringValue(),
            manifestPath);
        XURI xGraph = URI.createNS(xContext, xBaseURI.getStringValue(), Path);
        Statement xM_BaseHaspartGraph =
            new Statement(xBaseURI, pkg_hasPart, xGraph, xManifest);
        Statement xM_GraphTypeMetadata =
            new Statement(xGraph, rdf_type, pkg_MetadataFile, xManifest);
        return new Statement[] { xM_BaseHaspartGraph, xM_GraphTypeMetadata };
    }

    class TestRange implements XTextRange, XMetadatable, XServiceInfo
    {
        String m_Stream;
        String m_XmlId;
        String m_Text;
        TestRange(String i_Str) { m_Text = i_Str; }

        public String getStringValue() { return ""; }
        public String getNamespace() { return ""; }
        public String getLocalName() { return ""; }

        public StringPair getMetadataReference()
            {
                return new StringPair(m_Stream, m_XmlId);
            }
        public void setMetadataReference(StringPair i_Ref)
            throws IllegalArgumentException
            {
                m_Stream = i_Ref.First;
                m_XmlId = i_Ref.Second;
            }
        public void ensureMetadataReference()
            {
                m_Stream = "content.xml";
                m_XmlId = "42";
            }

        public String getImplementationName() { return null; }
        public String[] getSupportedServiceNames() { return null; }
        public boolean supportsService(String i_Svc)
            {
                return i_Svc.equals("com.sun.star.text.Paragraph");
            }

        public XText getText() { return null; }
        public XTextRange getStart() { return null; }
        public XTextRange getEnd() { return null; }
        public String getString() { return m_Text; }
        public void setString(String i_Str) { m_Text = i_Str; }
    }



    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "starting class: " + DocumentMetadataAccess.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "finishing class: " + DocumentMetadataAccess.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}


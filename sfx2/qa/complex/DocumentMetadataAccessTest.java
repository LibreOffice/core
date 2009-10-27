/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocumentMetadataAccessTest.java,v $
 *
 * $Revision: 1.1.2.9 $
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
 ************************************************************************/

package complex.framework;

import complexlib.ComplexTestCase;
import helper.StreamSimulator;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.Any;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.StringPair;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.util.XCloseable;
import com.sun.star.frame.XStorable;
import com.sun.star.frame.XLoadable;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
import com.sun.star.rdf.*;

/**
 * Test case for interface com.sun.star.rdf.XDocumentMetadataAccess
 * Currently, this service is implemented in
 * sfx2/source/doc/DocumentMetadataAccess.cxx
 *
 * Actually, this is not a service, so we need to create a document and
 * go from there...
 *
 * @author mst
 */
public class DocumentMetadataAccessTest extends ComplexTestCase
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

    public String[] getTestMethodNames ()
    {
        return new String[] { "check", "checkRDFa" };
    }

    public void before()
    {
        try {

            xMSF = (XMultiServiceFactory) param.getMSF();
            assure("could not create MultiServiceFactory.", xMSF != null);
            XPropertySet xPropertySet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, xMSF);
            Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
            xContext = (XComponentContext)
                UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
            assure("could not get component context.", xContext != null);

            tempDir = util.utils.getOfficeTemp/*Dir*/(xMSF);
            log.println("tempdir: " + tempDir);

            foo = URI.create(xContext, "uri:foo");
            assure("foo", null != foo);
            bar = URI.create(xContext, "uri:bar");
            assure("bar", null != bar);
            baz = URI.create(xContext, "uri:baz");
            assure("baz", null != baz);

            blank1 = BlankNode.create(xContext, "_:1");
            assure("blank1", null != blank1);
            blank2 = BlankNode.create(xContext, "_:2");
            assure("blank2", null != blank2);
            blank3 = BlankNode.create(xContext, "_:3");
            assure("blank3", null != blank3);
            blank4 = BlankNode.create(xContext, "_:4");
            assure("blank4", null != blank4);
            rdf_type = URI.createKnown(xContext, URIs.RDF_TYPE);
            assure("rdf_type", null != rdf_type);
            rdfs_label = URI.createKnown(xContext, URIs.RDFS_LABEL);
            assure("rdfs_label", null != rdfs_label);
            pkg_Document = URI.createKnown(xContext, URIs.PKG_DOCUMENT);
            assure("pkg_Document", null != pkg_Document);
            pkg_hasPart = URI.createKnown(xContext, URIs.PKG_HASPART);
            assure("pkg_hasPart", null != pkg_hasPart);
            pkg_MetadataFile = URI.createKnown(xContext, URIs.PKG_METADATAFILE);
            assure("pkg_MetadataFile", null != pkg_MetadataFile);
            odf_ContentFile = URI.createKnown(xContext, URIs.ODF_CONTENTFILE);
            assure("odf_ContentFile", null != odf_ContentFile);
            odf_StylesFile = URI.createKnown(xContext, URIs.ODF_STYLESFILE);
            assure("odf_StylesFile", null != odf_StylesFile);
            odf_Element = URI.createKnown(xContext, URIs.ODF_ELEMENT);
            assure("odf_Element", null != odf_Element);

        } catch (Exception e) {
            report(e);
        }
    }

    public void after()
    {
        xRep = null;
        xRS  = null;
        xDMA = null;
    }

    public void check()
    {
        XComponent xComp = null;
        XComponent xComp2 = null;
        try {
            XEnumeration xStmtsEnum;
            XNamedGraph xManifest;

            log.println("Creating document with Repository...");

            // we cannot create a XDMA directly, we must create
            // a document and get it from there :(
            // create document
            PropertyValue[] loadProps = new PropertyValue[1];
            loadProps[0] = new PropertyValue();
            loadProps[0].Name = "Hidden";
            loadProps[0].Value = new Boolean(true);
            xComp = util.DesktopTools.openNewDoc(xMSF, "swriter", loadProps);
            XTextDocument xText = (XTextDocument) UnoRuntime.queryInterface(
                        XTextDocument.class, xComp);

            XRepositorySupplier xRS = (XRepositorySupplier)
                UnoRuntime.queryInterface(XRepositorySupplier.class, xComp);
            assure("xRS null", null != xRS);
            XDocumentMetadataAccess xDMA = (XDocumentMetadataAccess)
                UnoRuntime.queryInterface(XDocumentMetadataAccess.class, xRS);
            assure("xDMA null", null != xDMA);
            xRep = xRS.getRDFRepository();
            assure("xRep null", null != xRep);

            log.println("...done");

            log.println("Checking that new repository is initialized...");

            XURI xBaseURI = (XURI) xDMA;
            String baseURI = xBaseURI.getStringValue();
            assure("new: baseURI",
                null != xBaseURI && !xBaseURI.getStringValue().equals(""));

            assure("new: # graphs", 1 == xRep.getGraphNames().length);
            XURI manifest = URI.createNS(xContext, xBaseURI.getStringValue(),
                manifestPath);
            xManifest = xRep.getGraph(manifest);
            assure("new: manifest graph", null != xManifest);

            Statement[] manifestStmts = getManifestStmts(xBaseURI);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assure("new: manifest graph", eq(xStmtsEnum, manifestStmts));

            log.println("...done");

            log.println("Checking some invalid args...");

            String content = "behold, for i am the content.";
            XTextRange xTR = new TestRange(content);
            XMetadatable xM = (XMetadatable) xTR;

            try {
                xDMA.getElementByURI(null);
                assure("getElementByURI: null allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.getMetadataGraphsWithType(null);
                assure("getMetadataGraphsWithType: null URI allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("", new XURI[0]);
                assure("addMetadataFile: empty filename allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("/foo", new XURI[0]);
                assure("addMetadataFile: absolute filename allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("fo\"o", new XURI[0]);
                assure("addMetadataFile: invalid filename allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("../foo", new XURI[0]);
                assure("addMetadataFile: filename with .. allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("foo/../../bar", new XURI[0]);
                assure("addMetadataFile: filename with nest .. allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("foo/././bar", new XURI[0]);
                assure("addMetadataFile: filename with nest . allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("content.xml", new XURI[0]);
                assure("addMetadataFile: content.xml allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("styles.xml", new XURI[0]);
                assure("addMetadataFile: styles.xml allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("meta.xml", new XURI[0]);
                assure("addMetadataFile: meta.xml allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addMetadataFile("settings.xml", new XURI[0]);
                assure("addMetadataFile: settings.xml allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.importMetadataFile(FileFormat.RDF_XML, null, "foo",
                    foo, new XURI[0]);
                assure("importMetadataFile: null stream allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(tempDir + "empty.rdf", true, param);
                xDMA.importMetadataFile(FileFormat.RDF_XML, xFooIn, "",
                    foo, new XURI[0]);
                assure("importMetadataFile: empty filename allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(tempDir + "empty.rdf", true, param);
                xDMA.importMetadataFile(FileFormat.RDF_XML, xFooIn, "meta.xml",
                    foo, new XURI[0]);
                assure("importMetadataFile: meta.xml filename allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(tempDir + "empty.rdf", true, param);
                xDMA.importMetadataFile(FileFormat.RDF_XML,
                    xFooIn, "foo", null, new XURI[0]);
                assure("importMetadataFile: null base URI allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                XInputStream xFooIn =
                    new StreamSimulator(tempDir + "empty.rdf", true, param);
                xDMA.importMetadataFile(FileFormat.RDF_XML,
                    xFooIn, "foo", rdf_type, new XURI[0]);
                assure("importMetadataFile: non-absolute base URI allowed",
                    false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.removeMetadataFile(null);
                assure("removeMetadataFile: null URI allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addContentOrStylesFile("");
                assure("addContentOrStylesFile: empty filename allowed",
                    false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addContentOrStylesFile("/content.xml");
                assure("addContentOrStylesFile: absolute filename allowed",
                    false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.addContentOrStylesFile("foo.rdf");
                assure("addContentOrStylesFile: invalid filename allowed",
                    false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.removeContentOrStylesFile("");
                assure("removeContentOrStylesFile: empty filename allowed",
                    false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.loadMetadataFromStorage(null, foo, null);
                assure("loadMetadataFromStorage: null storage allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.storeMetadataToStorage(null/*, base*/);
                assure("storeMetadataToStorage: null storage allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.loadMetadataFromMedium(new PropertyValue[0]);
                assure("loadMetadataFromMedium: empty medium allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }
            try {
                xDMA.storeMetadataToMedium(new PropertyValue[0]);
                assure("storeMetadataToMedium: empty medium allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }

            log.println("...done");

            log.println("Checking file addition/removal...");

            xDMA.removeContentOrStylesFile(contentPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("removeContentOrStylesFile (content)",
                eq(xStmtsEnum, new Statement[] {
                        manifestStmts[0], manifestStmts[2], manifestStmts[4]
                    }));

            xDMA.addContentOrStylesFile(contentPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("addContentOrStylesFile (content)",
                eq(xStmtsEnum, manifestStmts));

            xDMA.removeContentOrStylesFile(stylesPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("removeContentOrStylesFile (styles)",
                eq(xStmtsEnum, new Statement[] {
                        manifestStmts[0], manifestStmts[1], manifestStmts[3]
                    }));

            xDMA.addContentOrStylesFile(stylesPath);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("addContentOrStylesFile (styles)",
                eq(xStmtsEnum, manifestStmts));

            XURI xFoo = URI.createNS(xContext, xBaseURI.getStringValue(),
                fooPath);
            Statement xM_BaseHaspartFoo =
                new Statement(xBaseURI, pkg_hasPart, xFoo, manifest);
            Statement xM_FooTypeMetadata =
                new Statement(xFoo, rdf_type, pkg_MetadataFile, manifest);
            Statement xM_FooTypeBar =
                new Statement(xFoo, rdf_type, bar, manifest);
            xDMA.addMetadataFile(fooPath, new XURI[] { bar });
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("addMetadataFile",
                eq(xStmtsEnum, merge(manifestStmts, new Statement[] {
                        xM_BaseHaspartFoo, xM_FooTypeMetadata, xM_FooTypeBar
                    })));

            XURI[] graphsBar = xDMA.getMetadataGraphsWithType(bar);
            assure("getMetadataGraphsWithType",
                graphsBar.length == 1 && eq(graphsBar[0], xFoo));


            xDMA.removeMetadataFile(xFoo);
            xStmtsEnum = xManifest.getStatements(null, null, null);
            assure("removeMetadataFile",
                eq(xStmtsEnum, manifestStmts));

            log.println("...done");

            log.println("Checking mapping...");

            XEnumerationAccess xTextEnum = (XEnumerationAccess)
                UnoRuntime.queryInterface(XEnumerationAccess.class,
                    xText.getText());
            Object o = xTextEnum.createEnumeration().nextElement();
            XMetadatable xMeta1 = (XMetadatable) UnoRuntime.queryInterface(
                        XMetadatable.class, o);

            XURI uri;
            XMetadatable xMeta;
            xMeta = xDMA.getElementByURI(xMeta1);
            assure("getElementByURI: null", null != xMeta);
            String XmlId = xMeta.getMetadataReference().Second;
            String XmlId1 = xMeta1.getMetadataReference().Second;
            assure("getElementByURI: no xml id", !XmlId.equals(""));
            assure("getElementByURI: different xml id", XmlId.equals(XmlId1));

            log.println("...done");

            log.println("Checking storing and loading...");

            XURI xFoobar = URI.createNS(xContext, xBaseURI.getStringValue(),
                fooBarPath);
            Statement[] metadataStmts = getMetadataFileStmts(xBaseURI,
                fooBarPath);
            xDMA.addMetadataFile(fooBarPath, new XURI[0]);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assure("addMetadataFile",
                eq(xStmtsEnum, merge(manifestStmts, metadataStmts )));

            Statement xFoobar_FooBarFoo =
                new Statement(foo, bar, foo, xFoobar);
            xRep.getGraph(xFoobar).addStatement(foo, bar, foo);
            xStmtsEnum = xRep.getStatements(null, null, null);
            assure("addStatement",
                eq(xStmtsEnum, merge(manifestStmts, merge(metadataStmts,
                    new Statement[] { xFoobar_FooBarFoo }))));

            PropertyValue noMDNoContentFile = new PropertyValue();
            noMDNoContentFile.Name = "URL";
            noMDNoContentFile.Value = util.utils.getFullTestURL("CUSTOM.odt");
            PropertyValue noMDFile = new PropertyValue();
            noMDFile.Name = "URL";
            noMDFile.Value = util.utils.getFullTestURL("TEST.odt");
            PropertyValue file = new PropertyValue();
            file.Name = "URL";
            file.Value = tempDir + "TESTDMA.odt";
            /*
            PropertyValue baseURL = new PropertyValue();
            baseURL.Name = "DocumentBaseURL";
            baseURL.Value = tempDir + "TMP.odt";
            */
            PropertyValue mimetype = new PropertyValue();
            mimetype.Name = "MediaType";
            mimetype.Value = "application/vnd.oasis.opendocument.text";
            PropertyValue[] argsEmptyNoContent = { mimetype, noMDNoContentFile};
            PropertyValue[] argsEmpty = { mimetype, noMDFile };
            PropertyValue[] args = { mimetype, file };

            xStmtsEnum = xRep.getStatements(null, null, null);
            XURI[] graphs = xRep.getGraphNames();

            xDMA.storeMetadataToMedium(args);

            // this should re-init
            xDMA.loadMetadataFromMedium(argsEmptyNoContent);
            xRep = xRS.getRDFRepository();
            assure("xRep null", null != xRep);
            assure("baseURI still tdoc?",
                !baseURI.equals(xDMA.getStringValue()));
            Statement[] manifestStmts2 = getManifestStmts((XURI) xDMA);
            xStmtsEnum = xRep.getStatements(null, null, null);
            // there is no content or styles file in here, so we have just
            // the package stmt
            assure("loadMetadataFromMedium (no metadata, no content)",
                eq(xStmtsEnum, new Statement[] { manifestStmts2[0] }));

            // this should re-init
            xDMA.loadMetadataFromMedium(argsEmpty);
            xRep = xRS.getRDFRepository();
            assure("xRep null", null != xRep);
            assure("baseURI still tdoc?",
                !baseURI.equals(xDMA.getStringValue()));
            Statement[] manifestStmts3 = getManifestStmts((XURI) xDMA);

            xStmtsEnum = xRep.getStatements(null, null, null);
            assure("loadMetadataFromMedium (no metadata)",
                eq(xStmtsEnum, manifestStmts3));

            xDMA.loadMetadataFromMedium(args);
            xRep = xRS.getRDFRepository();
            assure("xRep null", null != xRep);
            Statement[] manifestStmts4 = getManifestStmts((XURI) xDMA);
            Statement[] metadataStmts4 = getMetadataFileStmts((XURI) xDMA,
                fooBarPath);

            xStmtsEnum = xRep.getStatements(null, null, null);
            assure("some graph(s) not reloaded",
                graphs.length == xRep.getGraphNames().length);

            XURI xFoobar4 = URI.createNS(xContext, xDMA.getStringValue(),
                fooBarPath);
            Statement xFoobar_FooBarFoo4 =
                new Statement(foo, bar, foo, xFoobar4);
            assure("loadMetadataFromMedium (re-load)",
                eq(xStmtsEnum, merge(manifestStmts4, merge(metadataStmts4,
                        new Statement[] { xFoobar_FooBarFoo4 }))));

            log.println("...done");

            log.println("Checking storing and loading via model...");

            String f = tempDir + "TESTPARA.odt";

            XStorable xStor = (XStorable) UnoRuntime.queryInterface(
                        XStorable.class, xRS);

            xStor.storeToURL(f, new PropertyValue[0]);

            xComp2 = util.DesktopTools.loadDoc(xMSF, f, loadProps);

            XDocumentMetadataAccess xDMA2 = (XDocumentMetadataAccess)
                UnoRuntime.queryInterface(XDocumentMetadataAccess.class,
                xComp2);
            assure("xDMA2 null", null != xDMA2);

            XRepositorySupplier xRS2 = (XRepositorySupplier)
                UnoRuntime.queryInterface(XRepositorySupplier.class, xComp2);
            assure("xRS2 null", null != xRS2);

            XRepository xRep2 = xRS2.getRDFRepository();
            assure("xRep2 null", null != xRep2);

            Statement[] manifestStmts5 = getManifestStmts((XURI) xDMA2);
            Statement[] metadataStmts5 = getMetadataFileStmts((XURI) xDMA2,
                fooBarPath);
            XURI xFoobar5 = URI.createNS(xContext, xDMA2.getStringValue(),
                fooBarPath);
            Statement xFoobar_FooBarFoo5 =
                new Statement(foo, bar, foo, xFoobar5);
            xStmtsEnum = xRep.getStatements(null, null, null);
            XEnumeration xStmtsEnum2 = xRep2.getStatements(null, null, null);
            assure("load: repository differs",
                eq(xStmtsEnum2, merge(manifestStmts5, merge(metadataStmts5,
                        new Statement[] { xFoobar_FooBarFoo5 }))));

            log.println("...done");

        } catch (Exception e) {
            report(e);
        } finally {
            close(xComp);
            close(xComp2);
        }
    }

    public void checkRDFa()
    {
        XComponent xComp = null;
        String file;
        try {
            file = util.utils.getFullTestURL("TESTRDFA.odt");
            xComp = loadRDFa(file);
            if (xComp != null)
            {
                file = tempDir + "TESTRDFA.odt";
                storeRDFa(xComp, file);
                close(xComp);
                xComp = loadRDFa(file);
            }
        } finally {
            close(xComp);
        }
    }

    public void storeRDFa(XComponent xComp, String file)
    {
        try {

            log.println("Storing test document...");

            XStorable xStor = (XStorable) UnoRuntime.queryInterface(
                        XStorable.class, xComp);

            xStor.storeToURL(file, new PropertyValue[0]);

            log.println("...done");

        } catch (Exception e) {
            report(e);
        }
    }

    public XComponent loadRDFa(String file)
    {
        XComponent xComp = null;
        try {

            log.println("Loading test document...");

            PropertyValue[] loadProps = new PropertyValue[1];
            loadProps[0] = new PropertyValue();
            loadProps[0].Name = "Hidden";
            loadProps[0].Value = new Boolean(true);



            xComp = util.DesktopTools.loadDoc(xMSF, file, loadProps);

            XRepositorySupplier xRS = (XRepositorySupplier)
                UnoRuntime.queryInterface(XRepositorySupplier.class, xComp);
            assure("xRS null", null != xRS);

            XDocumentRepository xRep = (XDocumentRepository)
                UnoRuntime.queryInterface(XDocumentRepository.class,
                    xRS.getRDFRepository());
            assure("xRep null", null != xRep);

            XTextDocument xTextDoc = (XTextDocument)
                UnoRuntime.queryInterface(XTextDocument.class, xComp);

            XText xText = xTextDoc.getText();

            XEnumerationAccess xEA = (XEnumerationAccess)
                UnoRuntime.queryInterface(XEnumerationAccess.class, xText);
            XEnumeration xEnum = xEA.createEnumeration();

            log.println("...done");

            log.println("Checking RDFa in loaded test document...");

            XMetadatable xPara;
            Statement[] stmts;

            Statement x_FooBarLit1 = new Statement(foo, bar, mkLit("1"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 1",
                eq(stmts, new Statement[] {
                        x_FooBarLit1
                    }));

            Statement x_FooBarLit2 = new Statement(foo, bar, mkLit("2"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 2",
                eq(stmts, new Statement[] {
                        x_FooBarLit2
                    }));

            Statement x_BlankBarLit3 =
                new Statement(blank1, bar, mkLit("3"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 3",
                eq(stmts, new Statement[] {
                        x_BlankBarLit3
                    }));
            XBlankNode b3 = (XBlankNode) UnoRuntime.queryInterface(
                XBlankNode.class, stmts[0].Subject);

            Statement x_BlankBarLit4 =
                new Statement(blank2, bar, mkLit("4"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 4",
                eq(stmts, new Statement[] {
                        x_BlankBarLit4
                    }));
            XBlankNode b4 = (XBlankNode) UnoRuntime.queryInterface(
                XBlankNode.class, stmts[0].Subject);

            Statement x_BlankBarLit5 =
                new Statement(blank1, bar, mkLit("5"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 5",
                eq(stmts, new Statement[] {
                        x_BlankBarLit5
                    }));
            XBlankNode b5 = (XBlankNode) UnoRuntime.queryInterface(
                XBlankNode.class, stmts[0].Subject);

            assure("RDFa: 3 != 4",
                !b3.getStringValue().equals(b4.getStringValue()));
            assure("RDFa: 3 == 5",
                 b3.getStringValue().equals(b5.getStringValue()));

            Statement x_FooBarLit6 = new Statement(foo, bar, mkLit("6"), null);
            Statement x_FooBazLit6 = new Statement(foo, baz, mkLit("6"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 6",
                eq(stmts, new Statement[] {
                        x_FooBarLit6, x_FooBazLit6
                    }));

            Statement x_FooBarLit7 = new Statement(foo, bar, mkLit("7"), null);
            Statement x_FooBazLit7 = new Statement(foo, baz, mkLit("7"), null);
            Statement x_FooFooLit7 = new Statement(foo, foo, mkLit("7"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 7",
                eq(stmts, new Statement[] {
                        x_FooBarLit7, x_FooBazLit7, x_FooFooLit7
                    }));

            XNode lit = mkLit("a fooish bar");
            XNode lit_type= mkLit("a fooish bar", bar);
            Statement x_FooBarLit = new Statement(foo, bar, lit, null);
            Statement x_FooBarLittype = new Statement(foo, bar, lit_type, null);

            Statement x_FooLabelLit8 =
                new Statement(foo, rdfs_label, mkLit("8"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 8",
                eq(stmts, new Statement[] {
                        x_FooBarLit, x_FooLabelLit8
                    }));

            Statement x_FooLabelLit9 =
                new Statement(foo, rdfs_label, mkLit("9"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 9",
                eq(stmts, new Statement[] {
                        x_FooBarLit, x_FooLabelLit9
                    }));

            Statement x_FooLabelLit10 =
                new Statement(foo, rdfs_label, mkLit("10"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 10",
                eq(stmts, new Statement[] {
                        x_FooBarLittype, x_FooLabelLit10
                    }));

            Statement x_FooBarLit11
                = new Statement(foo, bar, mkLit("11", bar), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 11",
                eq(stmts, new Statement[] {
                        x_FooBarLit11
                    }));

            XURI xFile = URI.createNS(xContext, file, "/" + contentPath);
            Statement x_FileBarLit12 =
                new Statement(xFile, bar, mkLit("12"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 12",
                eq(stmts, new Statement[] {
                        x_FileBarLit12
                    }));

            Statement x_FooLabelLit13 =
                new Statement(foo, rdfs_label, mkLit("13"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 13",
                eq(stmts, new Statement[] {
                        x_FooBarLit, x_FooLabelLit13
                    }));

            Statement x_FooLabelLit14 =
                new Statement(foo, rdfs_label, mkLit("14"), null);
            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 14",
                eq(stmts, new Statement[] {
                        x_FooBarLit, x_FooLabelLit14
                    }));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 15", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 16", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 17", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 18", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 19", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 20", eq(stmts, new Statement[] { } ));

            xPara = (XMetadatable) UnoRuntime.queryInterface(
                XMetadatable.class, xEnum.nextElement());
            stmts = xRep.getStatementRDFa(xPara);
            assure("RDFa: 21", eq(stmts, new Statement[] { } ));

            log.println("...done");

        } catch (Exception e) {
            report(e);
            close(xComp);
        }
        return xComp;
    }


// utilities -------------------------------------------------------------

    public void report2(Exception e)
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

    public void report(Exception e) {
        log.println("Exception occurred:");
        e.printStackTrace((java.io.PrintWriter) log);
        report2(e);
        failed();
    }

    static void close(XComponent i_comp)
    {
        try {
            XCloseable xClos = (XCloseable) UnoRuntime.queryInterface(
                        XCloseable.class, i_comp);
            if (xClos != null) xClos.close(true);
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
        if (null == n) return "< null >";
        return n.getStringValue();
    }

    static boolean isBlank(XNode i_node)
    {
        XBlankNode blank = (XBlankNode) UnoRuntime.queryInterface(
                    XBlankNode.class, i_node);
        return blank != null;
    }

/*
    static class Statement implements XStatement
    {
        XResource m_Subject;
        XResource m_Predicate;
        XNode m_Object;
        XURI m_Graph;

        Statement(XResource i_Subject, XResource i_Predicate, XNode i_Object,
            XURI i_Graph)
        {
            m_Subject = i_Subject;
            m_Predicate = i_Predicate;
            m_Object = i_Object;
            m_Graph = i_Graph;
        }

        public XResource getSubject() { return m_Subject; }
        public XResource getPredicate() { return m_Predicate; }
        public XNode getObject() { return m_Object; }
        public XURI getGraph() { return m_Graph; }
    }
*/

    static Statement[] toSeq(XEnumeration i_Enum) throws Exception
    {
        java.util.Collection c = new java.util.Vector();
        while (i_Enum.hasMoreElements()) {
            Statement s = (Statement) i_Enum.nextElement();
//log.println("toSeq: " + s.getSubject().getStringValue() + " " + s.getPredicate().getStringValue() + " " + s.getObject().getStringValue() + ".");
            c.add(s);
        }
//        return (Statement[]) c.toArray();
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
        java.util.Collection c = new java.util.Vector();
        while (i_Enum.hasMoreElements()) {
            XNode[] s = (XNode[]) i_Enum.nextElement();
            c.add(s);
        }
//        return (XNode[][]) c.toArray();
        Object[] arr = c.toArray();
        XNode[][] ret = new XNode[arr.length][];
        for (int i = 0; i < arr.length; ++i) {
            ret[i] = (XNode[]) arr[i];
        }
        return ret;
    }

    static class BindingComp implements java.util.Comparator
    {
        public int compare(Object i_Left, Object i_Right)
        {
            XNode[] left = (XNode[]) i_Left;
            XNode[] right = (XNode[]) i_Right;
            if (left.length != right.length) throw new RuntimeException();
            for (int i = 0; i < left.length; ++i) {
                int eq = (left[i].getStringValue().compareTo(
                            right[i].getStringValue()));
                if (eq != 0) return eq;
            }
            return 0;
        }
    }

    static class StmtComp implements java.util.Comparator
    {
        public int compare(Object i_Left, Object i_Right)
        {
            int eq;
            Statement left = (Statement) i_Left;
            Statement right = (Statement) i_Right;
            if ((eq = cmp(left.Graph,     right.Graph    )) != 0) return eq;
            if ((eq = cmp(left.Subject,   right.Subject  )) != 0) return eq;
            if ((eq = cmp(left.Predicate, right.Predicate)) != 0) return eq;
            if ((eq = cmp(left.Object,    right.Object   )) != 0) return eq;
            return 0;
        }

        public int cmp(XNode i_Left, XNode i_Right)
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
            log.println("Graphs differ: " + toS(lG) + " != " + toS(rG));
            return false;
        }
        if (!eq(i_Left.Subject, i_Right.Subject)) {
            log.println("Subjects differ: " +
                i_Left.Subject.getStringValue() + " != " +
                i_Right.Subject.getStringValue());
            return false;
        }
        if (!eq(i_Left.Predicate, i_Right.Predicate)) {
            log.println("Predicates differ: " +
                i_Left.Predicate.getStringValue() + " != " +
                i_Right.Predicate.getStringValue());
            return false;
        }
        if (!eq(i_Left.Object, i_Right.Object)) {
            log.println("Objects differ: " +
                i_Left.Object.getStringValue() + " != " +
                i_Right.Object.getStringValue());
            return false;
        }
        return true;
    }

    static boolean eq(Statement[] i_Result, Statement[] i_Expected)
    {
        if (i_Result.length != i_Expected.length) {
            log.println("eq: different lengths: " + i_Result.length + " " +
                i_Expected.length);
            return false;
        }
        Statement[] expected = (Statement[])
            java.util.Arrays.asList(i_Expected).toArray();
        java.util.Arrays.sort(i_Result, new StmtComp());
        java.util.Arrays.sort(expected, new StmtComp());
        for (int i = 0; i < expected.length; ++i) {
            if (!eq(i_Result[i], expected[i])) return false;
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
        String[] vars = (String[]) i_Result.getBindingNames();
        XEnumeration iter = (XEnumeration) i_Result;
        XNode[][] bindings = toSeqs(iter);
        if (vars.length != i_Vars.length) {
            log.println("var lengths differ");
            return false;
        }
        if (bindings.length != i_Bindings.length) {
            log.println("binding lengths differ: " + i_Bindings.length +
                " vs " + bindings.length );
            return false;
        }
        java.util.Arrays.sort(bindings, new BindingComp());
        java.util.Arrays.sort(i_Bindings, new BindingComp());
        for (int i = 0; i < i_Bindings.length; ++i) {
            if (i_Bindings[i].length != i_Vars.length) {
                log.println("TEST ERROR!");
                throw new Exception();
            }
            if (bindings[i].length != i_Vars.length) {
                log.println("binding length and var length differ");
                return false;
            }
            for (int j = 0; j < i_Vars.length; ++j) {
                if (!eq(bindings[i][j], i_Bindings[i][j])) {
                    log.println("bindings differ: " +
                        toS(bindings[i][j]) + " != " + toS(i_Bindings[i][j]));
                    return false;
                }
            }
        }
        for (int i = 0; i < i_Vars.length; ++i) {
            if (!vars[i].equals(i_Vars[i])) {
                log.println("variable names differ: " +
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
            { return new StringPair(m_Stream, m_XmlId); }
        public void setMetadataReference(StringPair i_Ref)
            throws IllegalArgumentException
            { m_Stream = (String)i_Ref.First; m_XmlId = (String)i_Ref.Second; }
        public void ensureMetadataReference()
            { m_Stream = "content.xml"; m_XmlId = "42"; }

        public String getImplementationName() { return null; }
        public String[] getSupportedServiceNames() { return null; }
        public boolean supportsService(String i_Svc)
            { return i_Svc.equals("com.sun.star.text.Paragraph"); }

        public XText getText() { return null; }
        public XTextRange getStart() { return null; }
        public XTextRange getEnd() { return null; }
        public String getString() { return m_Text; }
        public void setString(String i_Str) { m_Text = i_Str; }
    }
}


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

package complex.unoxml;

import complexlib.ComplexTestCase;
import helper.StreamSimulator;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.Any;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.Pair;
import com.sun.star.beans.StringPair;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
import com.sun.star.rdf.*;

/**
 * Test case for service com.sun.star.rdf.Repository
 * Currently, this service is implemented in
 * unoxml/source/rdf/librdf_repository.cxx
 * 
 * @author mst
 */
public class RDFRepositoryTest extends ComplexTestCase
{
    XComponentContext xContext;
    String tempDir;

    XDocumentRepository xRep;
    XURI foo;
    XURI bar;
    XURI baz;
    XURI uint;
    XURI rdfslabel;
    XURI manifest;
    XURI uuid;
    XURI base;
    XBlankNode blank;
    XLiteral lit;
    XLiteral litlang;
    XLiteral littype;
    String rdfs = "http://www.w3.org/2000/01/rdf-schema#";

    public String[] getTestMethodNames ()
    {
        return new String[] { "check", "checkSPARQL", "checkRDFa" };
    }

    public void before()
    {
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory) param.getMSF();
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
            uint = URI.create(xContext, "uri:int");
            assure("uint", null != uint);
            blank = BlankNode.create(xContext, "_:uno");
            assure("blank", null != blank);
            lit = Literal.create(xContext, "i am the literal");
            assure("lit", null != lit);
            litlang = Literal.createWithLanguage(xContext,
                "i am the literal", "en");
            assure("litlang", null != litlang);
            littype = Literal.createWithType(xContext, "42", uint);
            assure("littype", null != littype);

            rdfslabel = URI.create(xContext, rdfs + "label");
            assure("rdfslabel", null != rdfslabel);
            manifest = URI.create(xContext, "manifest:manifest"); //FIXME
            assure("manifest", null != manifest);
            uuid = URI.create(xContext,
                "urn:uuid:224ab023-77b8-4396-a75a-8cecd85b81e3");
            assure("uuid", null != uuid);
            base = URI.create(xContext, "base-uri:"); //FIXME
            assure("base", null != base);
        } catch (Exception e) {
            report(e);
        }
    }

    public void after()
    {
        xRep = null;
    }

    public void check()
    {
        try {

            log.println("Creating service Repository...");

            //FIXME: ?
//            xRep = Repository.create(xContext);
            xRep = (XDocumentRepository) UnoRuntime.queryInterface(
                XDocumentRepository.class, Repository.create(xContext));
            assure("null", null != xRep);

            log.println("...done");

            log.println("Checking that new repository is really empty...");
            assure("empty: graphs", 0 == xRep.getGraphNames().length);

            XEnumeration stmts;
            stmts = xRep.getStatements(null, null, null);
            assure("empty: stmts", !stmts.hasMoreElements());

            log.println("...done");

            log.println("Checking graph creation...");

            XNamedGraph xFooGraph = xRep.createGraph(foo);
            assure("foo graph", null != xFooGraph);

            try {
                xRep.createGraph(foo);
                assure("creating duplicate graph was allowed", false);
            } catch (ElementExistException e) {
                // ignore
            }

            try {
                xRep.createGraph(null);
                assure("invalid graph name was allowed", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }

            XURI[] names = xRep.getGraphNames();
            assure("no foo graph in getGraphNames",
                1 == names.length && eq(names[0], foo));
            assure("no foo graph", null != xRep.getGraph(foo));

            stmts = xFooGraph.getStatements(null, null, null);
            assure("stmts in foo graph", !stmts.hasMoreElements());

            XOutputStream xFooOut =
                new StreamSimulator(tempDir + "empty.rdf", false, param);
            xRep.exportGraph(FileFormat.RDF_XML, xFooOut, foo, base);
            xFooOut.closeOutput();

            XInputStream xFooIn =
                new StreamSimulator(tempDir + "empty.rdf", true, param);
            xRep.importGraph(FileFormat.RDF_XML, xFooIn, bar, base);
            assure("no bar graph", null != xRep.getGraph(bar));

            log.println("...done");

            log.println("Checking graph manipulation...");

            XEnumeration xFooEnum;

            Statement xFoo_FooBarBaz = new Statement(foo, bar, baz, foo);
            xFooGraph.addStatement(foo, bar, baz);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(foo,bar,baz)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz }));

            Statement xFoo_FooBarBlank = new Statement(foo, bar, blank, foo);
            xFooGraph.addStatement(foo, bar, blank);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(foo,bar,blank)",
                eq(xFooEnum,
                    new Statement[] { xFoo_FooBarBaz, xFoo_FooBarBlank }));
            xFooEnum = xRep.getStatements(null, null, null);
            assure("addStatement(foo,bar,blank) (global)",
                eq(xFooEnum,
                    new Statement[] { xFoo_FooBarBaz, xFoo_FooBarBlank }));

            Statement xFoo_BazBarLit = new Statement(baz, bar, lit, foo);
            xFooGraph.addStatement(baz, bar, lit);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(baz,bar,lit)",
                eq(xFooEnum, new Statement[] {
                    xFoo_FooBarBaz, xFoo_FooBarBlank, xFoo_BazBarLit }));
            xFooEnum = xFooGraph.getStatements(baz, bar, null);
            assure("addStatement(baz,bar,lit) (baz,bar)",
                eq(xFooEnum, new Statement[] { xFoo_BazBarLit }));

            Statement xFoo_BazBarLitlang =
                new Statement(baz, bar, litlang, foo);
            xFooGraph.addStatement(baz, bar, litlang);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(baz,bar,litlang)",
                eq(xFooEnum, new Statement[] {
                    xFoo_FooBarBaz, xFoo_FooBarBlank, xFoo_BazBarLit,
                    xFoo_BazBarLitlang }));
            xFooEnum = xFooGraph.getStatements(null, null, baz);
            assure("addStatement(baz,bar,litlang) (baz)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz }));

            Statement xFoo_BazBarLittype =
                new Statement(baz, bar, littype, foo);
            xFooGraph.addStatement(baz, bar, littype);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(baz,bar,littype)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz, xFoo_FooBarBlank,
                    xFoo_BazBarLit, xFoo_BazBarLitlang, xFoo_BazBarLittype }));

            xFooGraph.removeStatements(baz, bar, litlang);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("removeStatement(baz,bar,litlang)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz, xFoo_FooBarBlank,
                    xFoo_BazBarLit, xFoo_BazBarLittype }));

            xFooGraph.removeStatements(foo, bar, null);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("removeStatement(foo,bar,null)",
                eq(xFooEnum, new Statement[] {
                    xFoo_BazBarLit, xFoo_BazBarLittype }));

            xFooGraph.addStatement(foo, bar, baz);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(foo,bar,baz) (re-add)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz,
                    xFoo_BazBarLit, xFoo_BazBarLittype }));

            xFooGraph.addStatement(foo, bar, baz);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(foo,bar,baz) (duplicate)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz,
                    xFoo_BazBarLit, xFoo_BazBarLittype }));

            xFooGraph.addStatement(xFooGraph, bar, baz);
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("addStatement(foo,bar,baz) (triplicate, as graph)",
                eq(xFooEnum, new Statement[] { xFoo_FooBarBaz,
                     xFoo_BazBarLit, xFoo_BazBarLittype }));

            log.println("...done");

            log.println("Checking graph import/export...");

            xFooOut = new StreamSimulator(tempDir + "foo.rdf", false, param);
            xRep.exportGraph(FileFormat.RDF_XML, xFooOut, foo, base);
            xFooOut.closeOutput();

            xFooIn = new StreamSimulator(tempDir + "foo.rdf", true, param);
            try {
                xRep.importGraph(FileFormat.RDF_XML, xFooIn, bar, base);
                assure("importing existing graph did not fail", false);
            } catch (ElementExistException e) {
                // ignore
            }

            xFooIn = new StreamSimulator(tempDir + "foo.rdf", true, param);
            xRep.importGraph(FileFormat.RDF_XML, xFooIn, baz, base);
            XNamedGraph xBazGraph =  xRep.getGraph(baz);
            assure("no baz graph", null != xBazGraph);

            Statement xBaz_FooBarBaz = new Statement(foo, bar, baz, baz);
            Statement xBaz_BazBarLit = new Statement(baz, bar, lit, baz);
            Statement xBaz_BazBarLittype =
                new Statement(baz, bar, littype, baz);
            XEnumeration xBazEnum = xBazGraph.getStatements(null, null, null);
            assure("importing exported graph",
                eq(xBazEnum, new Statement[] {
                     xBaz_FooBarBaz, xBaz_BazBarLit, xBaz_BazBarLittype }));

            log.println("...done");

            log.println("Checking graph deletion...");

            xFooGraph.clear();
            xFooEnum = xFooGraph.getStatements(null, null, null);
            assure("clear", eq(xFooEnum, new Statement[] { }));

            xRep.destroyGraph(baz);
            assure("baz graph zombie", null == xRep.getGraph(baz));

            try {
                xBazGraph.clear();
                assure("deleted graph not invalid (clear)", false);
            } catch (NoSuchElementException e) {
                // ignore
            }

            try {
                xBazGraph.addStatement(foo, foo, foo);
                assure("deleted graph not invalid (add)", false);
            } catch (NoSuchElementException e) {
                // ignore
            }

            try {
                xBazGraph.removeStatements(null, null, null);
                assure("deleted graph not invalid (remove)", false);
            } catch (NoSuchElementException e) {
                // ignore
            }

            try {
                xBazGraph.getStatements(null, null, null);
                assure("deleted graph not invalid (get)", false);
            } catch (NoSuchElementException e) {
                // ignore
            }

            log.println("...done");

        } catch (Exception e) {
            report(e);
        }
    }

    public void checkSPARQL()
    {
        try {

            log.println("Checking SPARQL queries...");

            XInputStream xIn = new StreamSimulator(
                util.utils.getFullTestDocName("example.rdf"), true, param);
            xRep.importGraph(FileFormat.RDF_XML, xIn, manifest, base);

            String query;
            query = "SELECT ?p WHERE { ?p rdf:type pkg:Package . }";
            XQuerySelectResult result = xRep.querySelect(mkNss() + query);
            assure("query: package-id\n" + query,
                eq(result, new String[] { "p" },
                    new XNode[][] { { uuid } }));

            query = "SELECT ?part ?path FROM <" + manifest +
               "> WHERE { ?pkg rdf:type pkg:Package . ?pkg pkg:hasPart ?part ."
               + " ?part pkg:path ?path . ?part rdf:type odf:ContentFile. }";
            result = xRep.querySelect(mkNss() + query);
            assure("query: contentfile",
                eq(result, new String[] { "part", "path" },
                    new XNode[][] { { BlankNode.create(xContext, "whatever"),
                            Literal.create(xContext, "content.xml") } }));

            query = "SELECT ?pkg ?path FROM <" + toS(manifest) +  "> WHERE { "
                + "?pkg rdf:type pkg:Package . ?pkg pkg:hasPart ?part . "
                + "?part pkg:path ?path . ?part rdf:type odf:ContentFile. }";
            result = xRep.querySelect(mkNss() + query);
            assure("query: contentfile\n" + query,
                eq(result, new String[] { "pkg", "path" },
                    new XNode[][] { { uuid ,
                            Literal.create(xContext, "content.xml") } }));

            query = "SELECT ?part ?path FROM <" + toS(manifest) + "> WHERE { "
                + "?pkg rdf:type pkg:Package . ?pkg pkg:hasPart ?part . "
                + "?part pkg:path ?path . ?part rdf:type odf:StylesFile. }";
            result = xRep.querySelect(mkNss() + query);
            assure("query: stylesfile\n" + query,
                eq(result, new String[] { "part", "path" },
                    new XNode[][] { }));

            query = "SELECT ?part ?path FROM <" + toS(manifest) + "> WHERE { "
                + "?pkg rdf:type pkg:Package . ?pkg pkg:hasPart ?part . "
                + "?part pkg:path ?path . ?part rdf:type odf:MetadataFile. }";
            result = xRep.querySelect(mkNss() + query);
            assure("query: metadatafile\n" + query,
                eq(result, new String[] { "part", "path" },
                    new XNode[][] { {
                        URI.create(xContext, "http://hospital-employee/doctor"),
                        Literal.create(xContext,
                            "meta/hospital/doctor.rdf") } }));

//FIXME redland BUG
            String uri = "uri:example-element-2";
            query = "SELECT ?path ?idref FROM <" + toS(manifest) +  "> WHERE { "
//                + "<" + toS(uuid) + "> rdf:type pkg:Package ; "
//                                 + " pkg:hasPart ?part . "
                + "<" + toS(uuid) + "> pkg:hasPart ?part . "
                + "?part pkg:path ?path ; "
                     + " rdf:type ?type ; "
                     + " pkg:hasPart <" + uri + "> . "
//                + "<" + uri + "> rdf:type odf:Element ; "
                + "<" + uri + "> "
                      + " pkg:idref ?idref . "
                + " FILTER (?type = odf:ContentFile || ?type = odf:StylesFile)"
                + " }";
//log.println(query);
            result = xRep.querySelect(mkNss() + query);
            assure("query: example-element-2\n" + query,
                eq(result, new String[] { "path", "idref" },
                    new XNode[][] { {
                        Literal.create(xContext, "content.xml"),
                        Literal.create(xContext, "ID_B") } }));

            // CONSTRUCT result triples have no graph!
            Statement x_PkgFooLit = new Statement(uuid, foo, lit, null);
            query = "CONSTRUCT { ?pkg <" + toS(foo) + "> \"" +
                lit.getStringValue() + "\" } FROM <" + toS(manifest) +
                "> WHERE { ?pkg rdf:type pkg:Package . } ";
            XEnumeration xResultEnum = xRep.queryConstruct(mkNss() + query);
            assure("query: construct\n" + query,
                eq(xResultEnum, new Statement[] { x_PkgFooLit }));

            query = "ASK { ?pkg rdf:type pkg:Package . }";
            boolean bResult = xRep.queryAsk(mkNss() + query);
            assure("query: ask\n" + query, bResult);

            log.println("...done");

        } catch (Exception e) {
            report(e);
        }
    }

    public void checkRDFa()
    {
        try {
            log.println("Checking RDFa gunk...");

            String content = "behold, for i am the content.";
            XTextRange xTR = new TestRange(content);
            XMetadatable xM = (XMetadatable) xTR;

            Pair<Statement[], Boolean> result =
                xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: not empty (initial)",
                0 == result.First.length);

            try {
                xRep.setStatementRDFa(foo, new XURI[] {}, xM, "", null);
                assure("RDFa: set: no predicate", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }

            try {
                xRep.setStatementRDFa(foo, new XURI[] {bar}, null, "", null);
                assure("RDFa: set: null", false);
            } catch (IllegalArgumentException e) {
                // ignore
            }

            XLiteral trlit = Literal.create(xContext, content);
            Statement x_FooBarTRLit = new Statement(foo, bar, trlit, null);
            xRep.setStatementRDFa(foo, new XURI[] { bar }, xM, "", null);

            result = xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: without content",
                !result.Second && (1 == result.First.length)
                && eq((Statement)result.First[0], x_FooBarTRLit));

            //FIXME: do this?
            xTR.setString(lit.getStringValue());
/*
            Statement xFooBarLit = new Statement(foo, bar, lit, null);
            result = xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: change",
                eq((Statement)result.First, xFooBarLit) && null == result.Second);
*/

            Statement x_FooBarLittype = new Statement(foo, bar, littype, null);
            xRep.setStatementRDFa(foo, new XURI[] { bar }, xM, "42", uint);

            result = xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: with content",
                result.Second &&
                (1 == result.First.length) &&
                eq((Statement)result.First[0], x_FooBarLittype));

            //FIXME: do this?
            xTR.setString(content);
/*
            Statement xFooLabelTRLit = new Statement(foo, rdfslabel, trlit, null);
            result = xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: change (label)",
                eq((Statement)result.First, xFooBarLittype) &&
                eq((Statement)result.Second, xFooLabelTRLit));
*/

            xRep.removeStatementRDFa((XMetadatable)xTR);

            result = xRep.getStatementRDFa((XMetadatable)xTR);
            assure("RDFa: get: not empty (removed)",
                0 == result.First.length);

            xRep.setStatementRDFa(foo, new XURI[] { foo, bar, baz }, xM,
                "", null);

            Statement x_FooFooTRLit = new Statement(foo, foo, trlit, null);
            Statement x_FooBazTRLit = new Statement(foo, baz, trlit, null);
            result = xRep.getStatementRDFa((XMetadatable) xTR);
            assure("RDFa: get: without content (multiple predicates, reinsert)",
                !result.Second &&
                eq(result.First, new Statement[] {
                     x_FooFooTRLit, x_FooBarTRLit, x_FooBazTRLit }));

            xRep.removeStatementRDFa((XMetadatable)xTR);

            result = xRep.getStatementRDFa((XMetadatable) xTR);
            assure("RDFa: get: not empty (re-removed)",
                0 == result.First.length);

            log.println("...done");

        } catch (Exception e) {
            report(e);
        }
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


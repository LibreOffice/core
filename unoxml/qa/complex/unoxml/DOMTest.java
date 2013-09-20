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

package complex.unoxml;

import lib.TestParameters;
import helper.StreamSimulator;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.StringPair;
import com.sun.star.io.XInputStream;
import com.sun.star.io.SequenceInputStream;
import com.sun.star.xml.dom.*;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XSAXSerializable;
import com.sun.star.xml.sax.SAXException;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.xml.sax.XLocator;
import static com.sun.star.xml.dom.DOMExceptionType.*;
import static com.sun.star.xml.dom.NodeType.*;
import com.sun.star.xml.xpath.*;
import static com.sun.star.xml.xpath.XPathObjectType.*;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * Test for com.sun.star.xml.dom.*, com.sun.star.xml.xpath.*
 */
public class DOMTest
{
    private static final OfficeConnection connection = new OfficeConnection();

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    XComponentContext m_xContext;
    XMultiServiceFactory m_xMSF;
    TestParameters m_params;

    @Before public void before() throws Exception
    {
        final XMultiServiceFactory xMSF = UnoRuntime.queryInterface(
                XMultiServiceFactory.class,
                connection.getComponentContext().getServiceManager());
        assertNotNull("could not create MultiServiceFactory.", xMSF);
        m_params = new TestParameters();
        m_params.put("ServiceFactory", xMSF);
        XPropertySet xPropertySet =
            UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        m_xContext = UnoRuntime.queryInterface(XComponentContext.class,
                xPropertySet.getPropertyValue("DefaultContext"));
        assertNotNull("could not get component context.", m_xContext);
        m_xMSF = xMSF;
    }

    @Test public void testXSAXDocumentBuilder() throws Exception
    {
        XSAXDocumentBuilder xSAXBuilder =
            UnoRuntime.queryInterface(XSAXDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.SAXDocumentBuilder"));
        //FIXME TODO
    }

    @Test
    public void testXDocumentBuilder() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));

        XDOMImplementation xDomImpl = xBuilder.getDOMImplementation();
//FIXME fails        assertNotNull("getDOMImplementation", xDomImpl);

        xBuilder.isNamespaceAware();
        xBuilder.isValidating();

        {
            XDocument xDoc = xBuilder.newDocument();
            assertNotNull("newDocument", xDoc);
        }

        try {
            xBuilder.parse(null);
            fail("XDocumentBuilder.parse(null)");
        } catch (Exception e) { /* expected */ }
        {
            XInputStream xIn = new StreamSimulator(
                    TestDocument.getUrl("example.rdf"), true, m_params);
            XDocument xDoc = xBuilder.parse(xIn);
            assertNotNull("XDocumentBuilder.parse", xDoc);
        }
        try {
            xBuilder.parseURI("");
            fail("XDocumentBuilder.parseURI(\"\")");
        } catch (Exception e) { /* expected */ }
        {
            XDocument xDoc =
                xBuilder.parseURI(TestDocument.getUrl("example.rdf"));
            assertNotNull("XDocumentBuilder.parseURI", xDoc);
        }

        xBuilder.setEntityResolver(null);
        /* FIXME TODO
        XEntityResolver xER;
        xBuilder.setEntityResolver(xER);
        */

        xBuilder.setErrorHandler(null);
        /* FIXME TODO
        XErrorHandler xEH;
        xBuilder.setErrorHandler(xEH);
        */
    }

    @Test public void testXDocument() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        /* FIXME
        try {
            xDoc.createAttribute("&");
            fail("XDocument.createAttribute");
        } catch (DOMException e) {
            assertTrue("XDocument.createAttribute",
                    INVALID_CHARACTER_ERR == e.Code);
        }*/
        {
            XAttr xAttr = xDoc.createAttribute("foo");
            assertNotNull("XDocument.createAttribute", xAttr);
            assertEquals("XDocument.createAttribute",
                    "foo", xAttr.getNodeName());
        }

        String ns = "http://example.com/";
        /* FIXME
        try {
            xDoc.createAttributeNS(ns, "&");
            fail("XDocument.createAttributeNS");
        } catch (DOMException e) {
            assertTrue("XDocument.createAttributeNS",
                    INVALID_CHARACTER_ERR == e.Code);
        }
        */
        {
            XAttr xAttr = xDoc.createAttributeNS(ns, "e:foo");
            assertNotNull("XDocument.createAttributeNS", xAttr);
            assertEquals("XDocument.createAttributeNS", "foo",
                    xAttr.getNodeName());
        }

        XCDATASection xCDS = xDoc.createCDATASection("foo");
        assertNotNull("XDocument.createCDATASection", xCDS);

        XComment xComment = xDoc.createComment("foo");
        assertNotNull("XDocument.createComment", xComment);

        XDocumentFragment xDF = xDoc.createDocumentFragment();
        assertNotNull("XDocument.createDocumentFragment", xDF);

        /* FIXME
        try {
            xDoc.createElement("&");
            fail("XDocument.createElement(\"&\")");
        } catch (DOMException e) {
            assertTrue("XDocument.createElement(\"&\")",
                    INVALID_CHARACTER_ERR == e.Code);
        }
        */
        XElement xElemFoo = xDoc.createElement("foo");
        assertNotNull("XDocument.createElement(\"foo\")", xElemFoo);
        assertEquals("XDocument.createElement(\"foo\")",
                "foo", xElemFoo.getNodeName());

        /* FIXME
        try {
            xDoc.createElementNS(ns, "&");
            fail("XDocument.createElementNS(\"&\")");
        } catch (DOMException e) {
            assertTrue("XDocument.createElementNS(\"&\")",
                    INVALID_CHARACTER_ERR == e.Code);
        }
        */
        XElement xElemFooNs = xDoc.createElementNS(ns, "foo");
        assertNotNull("XDocument.createElementNS(\"foo\")", xElemFooNs);
        assertEquals("XDocument.createElementNS(\"foo\")",
                "foo", xElemFooNs.getNodeName());

        XEntityReference xER = xDoc.createEntityReference("foo");
        assertNotNull("XDocument.createEntityReference", xER);

        XProcessingInstruction xPI =
            xDoc.createProcessingInstruction("foo", "bar");
        assertNotNull("XDocument.createProcessingInstruction", xPI);

        XText xText = xDoc.createTextNode("foo");
        assertNotNull("XDocument.createTextNode", xText);

        XDocumentType xDT = xDoc.getDoctype();
        assertNull("XDocument.getDoctype", xDT);

        {
            XElement xDE = xDoc.getDocumentElement();
            assertNull("XDocument.getDocumentElement", xDE);
        }
        {
            XElement xById = xDoc.getElementById("foo");
            assertNull("XDocument.getDocumentElement", xById);
        }

        {
            XNodeList xNodeList = xDoc.getElementsByTagName("foo");
            assertNotNull("XDocument.getElementsByTagName", xNodeList);
            assertTrue("XDocument.getElementsByTagName",
                    0 == xNodeList.getLength());
        }

        {
            XNodeList xNodeList = xDoc.getElementsByTagNameNS(ns, "foo");
            assertNotNull("XDocument.getElementsByTagNameNS", xNodeList);
            assertTrue("XDocument.getElementsByTagNameNS",
                    0 == xNodeList.getLength());
        }

        XDOMImplementation xDOMImpl = xDoc.getImplementation();
        assertNotNull("XDocument.getImplementation", xDOMImpl);

        {
            XNode xRet = xElemFooNs.appendChild(xElemFoo);
            assertEquals("XElement.appendChild(xElemFoo)", xElemFoo, xRet);
        }
        {
            XNode xRet = xDoc.appendChild(xElemFooNs);
            assertTrue("XDocument.appendChild(xElemFooNs)",
                    xElemFooNs.equals(xRet));
        }

        XElement xDE = xDoc.getDocumentElement();
        assertNotNull("XDocument.getDocumentElement", xDE);
        assertEquals("XDocument.getDocumentElement", xElemFooNs, xDE);

        {
            XNodeList xNodeList = xDoc.getElementsByTagName("foo");
            assertNotNull("XDocument.getElementsByTagName", xNodeList);
            assertTrue("XDocument.getElementsByTagName",
                    2 == xNodeList.getLength());
            assertEquals("XDocument.getElementsByTagNameNS",
                    xElemFooNs, xNodeList.item(0));
            assertEquals("XDocument.getElementsByTagName",
                    xElemFoo, xNodeList.item(1));
        }

        {
            XNodeList xNodeList = xDoc.getElementsByTagNameNS(ns, "foo");
            assertNotNull("XDocument.getElementsByTagNameNS", xNodeList);
            assertTrue("XDocument.getElementsByTagNameNS",
                    1 == xNodeList.getLength());
            assertEquals("XDocument.getElementsByTagNameNS",
                    xElemFooNs, xNodeList.item(0));
        }

        xElemFoo.setAttributeNS("http://www.w3.org/XML/1998/namespace",
                "xml:id", "bar");

        XElement xById = xDoc.getElementById("bar");
        assertNotNull("XDocument.getDocumentElement", xById);
        assertEquals("XDocument.getDocumentElement", xElemFoo, xById);

        try {
            xDoc.importNode(null, false);
            fail("XDocument.importNode(null)");
        } catch (Exception e) { /* expected */ }
        {
            XNode xImported = xDoc.importNode(xElemFoo, false);
            assertNotNull("XDocument.importNode()", xImported);
            assertEquals("XDocument.importNode()", xElemFoo, xImported);
        }
        {
            MockAttr xMockAttrBar = new MockAttr("bar", "blah");
            MockAttr xMockAttrBaz = new MockAttr("baz", "quux");
            MockElement xMockElemFoo = new MockElement("foo",
                    new MockAttr[] { xMockAttrBar, xMockAttrBaz });
            MockElement xMockElemBar = new MockElement("bar",
                    new MockAttr[] { });
            MockElement xMockElemRoot =
                new MockElement("root", new MockAttr[] { });
            MockDoc xMockDoc = new MockDoc();
            xMockDoc.init(new MockNode[] { xMockElemRoot });
            xMockElemRoot.init(xMockDoc, xMockDoc, null, null,
                    new MockNode[] { xMockElemFoo, xMockElemBar });
            xMockElemFoo.init(xMockDoc, xMockElemRoot, null, xMockElemBar,
                    new MockNode[] { });
            xMockElemBar.init(xMockDoc, xMockElemRoot, xMockElemFoo, null,
                    new MockNode[] { });

            {
                XNode xImported = xDoc.importNode(xMockElemRoot, false);
                assertNotNull("XDocument.importNode(false)", xImported);
                XElement xE =
                    UnoRuntime.queryInterface(XElement.class, xImported);
                assertNotNull("XDocument.importNode(false)", xE);
                assertEquals("XDocument.importNode(false)",
                        "root", xE.getLocalName());
                assertFalse("XDocument.importNode(false)", xE.hasAttributes());
                assertFalse("XDocument.importNode(false)", xE.hasChildNodes());
            }

            {
                XNode xImported = xDoc.importNode(xMockElemRoot, true);
                assertNotNull("XDocument.importNode(true)", xImported);
                XElement xImpRoot =
                    UnoRuntime.queryInterface(XElement.class, xImported);
                assertNotNull("XDocument.importNode(true)", xImpRoot);
                assertEquals("XDocument.importNode(true)",
                        "root", xImpRoot.getLocalName());
                assertFalse("XDocument.importNode(true)",
                        xImpRoot.hasAttributes());
                assertTrue("XDocument.importNode(true)",
                        xImpRoot.hasChildNodes());
                assertEquals("XDocument.importNode(true)",
                        "root", xImpRoot.getNodeName());

                XNode xImpFooN = xImpRoot.getFirstChild();
                assertNotNull("XDocument.importNode(true)", xImpFooN);
                XElement xImpFoo =
                    UnoRuntime.queryInterface(XElement.class, xImpFooN);
                assertNotNull("XDocument.importNode(true)", xImpFoo);
                assertTrue("XDocument.importNode(true)",
                        xImpFoo.hasAttributes());
                assertFalse("XDocument.importNode(true)",
                        xImpFoo.hasChildNodes());
                assertEquals("XDocument.importNode(true)",
                        "foo", xImpFoo.getNodeName());
                assertEquals("XDocument.importNode(true)",
                        "blah", xImpFoo.getAttribute("bar"));
                assertEquals("XDocument.importNode(true)",
                        "quux", xImpFoo.getAttribute("baz"));
                XNode xImpBarN = xImpFooN.getNextSibling();
                assertNotNull("XDocument.importNode(true)", xImpBarN);
                XElement xImpBar =
                    UnoRuntime.queryInterface(XElement.class, xImpBarN);
                assertNotNull("XDocument.importNode(true)", xImpBar);
                assertFalse("XDocument.importNode(true)",
                        xImpBar.hasAttributes());
                assertFalse("XDocument.importNode(true)",
                        xImpBar.hasChildNodes());
                assertEquals("XDocument.importNode(true)",
                        "bar", xImpBar.getNodeName());
                assertNull("XDocument.importNode(true)",
                        xImpBar.getNextSibling());
            }
        }

        // XNode ////////////////////////////////////////////////////

        {
            XNode xDocCloneN = xDoc.cloneNode(false);
            assertNotNull("XDocument.cloneNode(false)", xDocCloneN);
            XDocument xDocClone =
                UnoRuntime.queryInterface(XDocument.class, xDocCloneN);
            assertNotNull("XDocument.cloneNode(false)", xDocClone);
            assertFalse("XDocument.cloneNode(false)",
                    xDocClone.hasChildNodes());
            assertNull("XDocument.cloneNode(false)", xDocClone.getFirstChild());
            assertNull("XDocument.cloneNode(false)",
                    xDocClone.getDocumentElement());
        }
        {
            XNode xDocCloneN = xDoc.cloneNode(true);
            assertNotNull("XDocument.cloneNode(true)", xDocCloneN);
            XDocument xDocClone =
                UnoRuntime.queryInterface(XDocument.class, xDocCloneN);
            assertNotNull("XDocument.cloneNode(true)", xDocClone);
            assertTrue("XDocument.cloneNode(true)", xDocClone.hasChildNodes());
            assertNotNull("XDocument.cloneNode(true)",
                    xDocClone.getFirstChild());
            XElement xE = xDocClone.getDocumentElement();
            assertNotNull("XDocument.cloneNode(true)", xE);
            assertFalse("XDocument.cloneNode(true)", xElemFooNs.equals(xE));
            assertEquals("XDocument.cloneNode(true)", "foo", xE.getLocalName());
            assertEquals("XDocument.cloneNode(true)", ns, xE.getNamespaceURI());
        }

        assertNull("XDocument.getAttributes()", xDoc.getAttributes());

        {
            XNodeList xChildren = xDoc.getChildNodes();
            assertTrue("XDocument.getChildNodes()", 1 == xChildren.getLength());
            assertEquals("XDocument.getChildNodes()",
                    xElemFooNs, xChildren.item(0));

            XNode xFirst = xDoc.getFirstChild();
            assertEquals("XDocument.getFirstChild()", xElemFooNs, xFirst);
            XNode xLast = xDoc.getLastChild();
            assertEquals("XDocument.getLastChild()", xElemFooNs, xLast);
        }

        assertEquals("XDocument.getLocalName()", "", xDoc.getLocalName());

        assertEquals("XDocument.getNamespaceURI()", "", xDoc.getNamespaceURI());

        assertNull("XDocument.getNextSibling()", xDoc.getNextSibling());

        assertEquals("XDocument.getNodeName()",
                "#document", xDoc.getNodeName());

        assertTrue("XDocument.getNodeType()",
                DOCUMENT_NODE == xDoc.getNodeType());

        assertEquals("XDocument.getNodeValue()", "", xDoc.getNodeValue());

        assertEquals("XDocument.getOwnerDocument()",
                xDoc, xDoc.getOwnerDocument());

        assertNull("XDocument.getParentNode()", xDoc.getParentNode());

        assertEquals("XDocument.getPrefix()", "", xDoc.getPrefix());

        assertNull("XDocument.getPreviousSibling()", xDoc.getPreviousSibling());

        assertFalse("XDocument.hasAttributes()", xDoc.hasAttributes());

        assertTrue("XDocument.hasChildNodes()", xDoc.hasChildNodes());

        assertFalse("XDocument.isSupported()",
                xDoc.isSupported("frobnication", "v99.33.0.0.0.1"));

        xDoc.normalize();

        try {
            xDoc.setNodeValue("42");
            fail("XDocument.setNodeValue()");
        } catch (DOMException e) {
            assertTrue("XDocument.setNodeValue()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xDoc.setPrefix("foo");
            fail("XDocument.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XDocument.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xDoc.appendChild(null);
            fail("XDocument.appendChild(null)");
        } catch (Exception e) { /* expected */ }


        try {
            xDoc.insertBefore(null, xText);
            fail("XDocument.insertBefore(null,)");
        } catch (Exception e) { /* expected */ }
        try {
            xDoc.insertBefore(xText, null);
            fail("XDocument.insertBefore(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDoc.insertBefore(xText, xText);
            fail("XDocument.insertBefore(x, x)");
        } catch (DOMException e) {
            assertTrue("XDocument.insertBefore(x, x)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        {
            XNode xRet = xDoc.insertBefore(xComment, xElemFooNs);
            assertEquals("XDocument.insertBefore(xComment, xElemFooNs)",
                    xRet, xElemFooNs); // why does this return the old node?
            assertEquals("XDocument.insertBefore(xComment, xElemFooNs)",
                    xComment, xDoc.getFirstChild());
            assertEquals("XDocument.insertBefore(xComment, xElemFooNs)",
                    xDoc, xComment.getParentNode());
            assertEquals("XDocument.insertBefore(xCommnet, xElemFooNs)",
                    xElemFooNs, xDoc.getLastChild());
        }

        try {
            xDoc.replaceChild(null, xText);
            fail("XDocument.replaceChild(null, )");
        } catch (Exception e) { /* expected */ }
        try {
            xDoc.replaceChild(xText, null);
            fail("XDocument.replaceChild(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDoc.replaceChild(xElemFoo, xElemFoo); // not child
            fail("XDocument.replaceChild(xElemFoo, xElemFoo)");
        } catch (DOMException e) {
            assertTrue("XDocument.replaceChild(xElemFoo, xElemFoo)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        try {
            xDoc.replaceChild(xElemFooNs, xElemFooNs); // child
            assertFalse("XDocument.replaceChild(xElemFooNs, xElemFooNs)",
                    false);
        } catch (DOMException e) {
            assertTrue("XDocument.replaceChild(xElemFooNs, xElemFooNs)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        XNode xReplaced = xDoc.replaceChild(xPI, xComment);
        assertEquals("XDocument.replaceChild(xPI, xComment)",
                xReplaced, xComment);
        assertEquals("XDocument.replaceChild(xPI, xComment)",
                xPI, xDoc.getFirstChild());
        assertEquals("XDocument.replaceChild(xPI, xComment)",
                xElemFooNs, xDoc.getLastChild());

        try {
            xDoc.removeChild(null);
            fail("XDocument.removeChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDoc.removeChild(xElemFoo);
            fail("XDocument.removeChild()");
        } catch (DOMException e) {
            assertTrue("XDocument.removeChild()",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xRemoved = xDoc.removeChild(xPI);
        assertEquals("XDocument.removeChild(xPI)", xRemoved, xPI);
        assertTrue("XDocument.removeChild(xPI)", xDoc.hasChildNodes());
        assertEquals("XDocument.removeChild(xPI)",
                xElemFooNs, xDoc.getFirstChild());
        assertEquals("XDocument.removeChild(xPI)",
                xElemFooNs, xDoc.getLastChild());
    }

    @Test public void testXDocumentFragment() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XDocumentFragment xDF = xDoc.createDocumentFragment();
        assertNotNull("XDocument.createDocumentFragment", xDF);

        XElement xElemFoo = xDoc.createElement("foo");
        assertNotNull("XDocument.createElement", xElemFoo);

        xDF.appendChild(xElemFoo);

        // XNode ////////////////////////////////////////////////////

        XText xText = xDoc.createTextNode("foo");
        XComment xComment = xDoc.createComment("foo");

        {
            XNode xDFCloneN = xDF.cloneNode(false);
            assertNotNull("XDocumentFragment.cloneNode(false)", xDFCloneN);
            XDocumentFragment xDFClone =
                UnoRuntime.queryInterface(XDocumentFragment.class, xDFCloneN);
            assertNotNull("XDocumentFragment.cloneNode(false)", xDFClone);
            assertFalse("XDocumentFragment.cloneNode(false)",
                    xDFClone.hasChildNodes());
            assertNull("XDocumentFragment.cloneNode(false)",
                    xDFClone.getFirstChild());
        }
        {
            XNode xDFCloneN = xDF.cloneNode(true);
            assertNotNull("XDocumentFragment.cloneNode(true)", xDFCloneN);
            XDocumentFragment xDFClone =
                UnoRuntime.queryInterface(XDocumentFragment.class, xDFCloneN);
            assertNotNull("XDocumentFragment.cloneNode(true)", xDFClone);
            assertTrue("XDocumentFragment.cloneNode(true)",
                    xDFClone.hasChildNodes());
            XNode xChild = xDFClone.getFirstChild();
            assertNotNull("XDocumentFragment.cloneNode(true)", xChild);
            XElement xE = UnoRuntime.queryInterface(XElement.class, xChild);
            assertFalse("XDocumentFragment.cloneNode(true)",
                    xElemFoo.equals(xE));
            assertEquals("XDocumentFragment.cloneNode(true)",
                    "foo", xE.getLocalName());
        }

        assertNull("XDocumentFragment.getAttributes()", xDF.getAttributes());

        {
            XNodeList xChildren = xDF.getChildNodes();
            assertTrue("XDocumentFragment.getChildNodes()",
                    1 == xChildren.getLength());
            assertEquals("XDocumentFragment.getChildNodes()",
                    xElemFoo, xChildren.item(0));

            XNode xFirst = xDF.getFirstChild();
            assertEquals("XDocumentFragment.getFirstChild()",
                    xElemFoo, xFirst);
            XNode xLast = xDF.getLastChild();
            assertEquals("XDocumentFragment.getLastChild()", xElemFoo, xLast);
        }

        assertEquals("XDocumentFragment.getLocalName()",
                "", xDF.getLocalName());

        assertEquals("XDocumentFragment.getNamespaceURI()",
                "", xDF.getNamespaceURI());

        assertNull("XDocumentFragment.getNextSibling()", xDF.getNextSibling());

        assertEquals("XDocumentFragment.getNodeName()",
                "#document-fragment", xDF.getNodeName());

        assertTrue("XDocumentFragment.getNodeType()",
                DOCUMENT_FRAGMENT_NODE == xDF.getNodeType());

        assertEquals("XDocumentFragment.getNodeValue()",
                "", xDF.getNodeValue());

        assertEquals("XDocumentFragment.getOwnerDocument()",
                xDoc, xDF.getOwnerDocument());

        assertNull("XDocumentFragment.getParentNode()", xDF.getParentNode());

        assertEquals("XDocumentFragment.getPrefix()", "", xDF.getPrefix());

        assertNull("XDocumentFragment.getPreviousSibling()",
                xDF.getPreviousSibling());

        assertFalse("XDocumentFragment.hasAttributes()", xDF.hasAttributes());

        assertTrue("XDocumentFragment.hasChildNodes()", xDF.hasChildNodes());

        assertFalse("XDocumentFragment.isSupported()",
                xDF.isSupported("frobnication", "v99.33.0.0.0.1"));

        xDF.normalize();

        try {
            xDF.setNodeValue("42");
            fail("XDocumentFragment.setNodeValue()");
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.setNodeValue()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xDF.setPrefix("foo");
            fail("XDocumentFragment.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xDF.appendChild(null);
            fail("XDocumentFragment.appendChild(null)");
        } catch (Exception e) { /* expected */ }


        try {
            xDF.insertBefore(null, xText);
            fail("XDocumentFragment.insertBefore(null,)");
        } catch (Exception e) { /* expected */ }
        try {
            xDF.insertBefore(xText, null);
            fail("XDocumentFragment.insertBefore(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDF.insertBefore(xText, xText);
            fail("XDocumentFragment.insertBefore(x, x)");
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.insertBefore(x, x)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        {
            XNode xRet = xDF.insertBefore(xComment, xElemFoo);
            assertEquals("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                    xRet, xElemFoo); // why does this return the old node?
            assertEquals("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                    xComment, xDF.getFirstChild());
            assertEquals("XDocumentFragment.insertBefore(xComment, xElemFoo)",
                    xDF, xComment.getParentNode());
            assertEquals("XDocumentFragment.insertBefore(xCommnet, xElemFoo)",
                    xElemFoo, xDF.getLastChild());
        }

        try {
            xDF.replaceChild(null, xText);
            fail("XDocumentFragment.replaceChild(null, )");
        } catch (Exception e) { /* expected */ }
        try {
            xDF.replaceChild(xText, null);
            fail("XDocumentFragment.replaceChild(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDF.replaceChild(xElemFoo, xElemFoo); // not child
            fail("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)");
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        try {
            xDF.replaceChild(xElemFoo, xElemFoo); // child
            assertFalse("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)",
                    false);
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.replaceChild(xElemFoo, xElemFoo)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        XNode xReplaced = xDF.replaceChild(xText, xComment);
        assertEquals("XDocumentFragment.replaceChild(xText, xComment)",
                xReplaced, xComment);
        assertEquals("XDocumentFragment.replaceChild(xText, xComment)",
                xText, xDF.getFirstChild());
        assertEquals("XDocumentFragment.replaceChild(xText, xComment)",
                xElemFoo, xDF.getLastChild());

        try {
            xDF.removeChild(null);
            fail("XDocumentFragment.removeChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xDF.removeChild(xComment);
            fail("XDocumentFragment.removeChild()");
        } catch (DOMException e) {
            assertTrue("XDocumentFragment.removeChild()",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xRemoved = xDF.removeChild(xText);
        assertEquals("XDocumentFragment.removeChild(xText)", xRemoved, xText);
        assertTrue("XDocumentFragment.removeChild(xText)", xDF.hasChildNodes());
        assertEquals("XDocumentFragment.removeChild(xText)",
                xElemFoo, xDF.getFirstChild());
        assertEquals("XDocumentFragment.removeChild(xText)",
                xElemFoo, xDF.getLastChild());
    }

    @Test public void testXElement() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        String ns = "http://example.com/";

        XElement xElemFoo = xDoc.createElement("foo");
        assertNotNull("XDocument.createElement(\"foo\")", xElemFoo);

        XElement xElemFooNs = xDoc.createElementNS(ns, "e:foo");
        assertNotNull("XDocument.createElementNs(\"foo\")", xElemFooNs);

        assertEquals("XElement.getTagName", "foo", xElemFoo.getTagName());

        {
            XNodeList xNodeList = xElemFoo.getElementsByTagName("bar");
            assertNotNull("XElement.getElementsByTagName", xNodeList);
            assertTrue("XElement.getElementsByTagName",
                    0 == xNodeList.getLength());
        }

        {
            XNodeList xNodeList = xElemFoo.getElementsByTagNameNS(ns, "bar");
            assertNotNull("XElement.getElementsByTagNameNS", xNodeList);
            assertTrue("XElement.getElementsByTagNameNS",
                    0 == xNodeList.getLength());
        }

        xElemFoo.appendChild(xElemFooNs);

        {
            XNodeList xNodeList = xElemFoo.getElementsByTagName("foo");
            assertNotNull("XElement.getElementsByTagName", xNodeList);
            assertTrue("XElement.getElementsByTagName",
                    2 == xNodeList.getLength());
            assertEquals("XElement.getElementsByTagName",
                    xElemFoo, xNodeList.item(0));
            assertEquals("XElement.getElementsByTagName",
                    xElemFooNs, xNodeList.item(1));
        }
        {
            XNodeList xNodeList = xElemFoo.getElementsByTagNameNS(ns, "foo");
            assertNotNull("XElement.getElementsByTagNameNS", xNodeList);
            assertTrue("XElement.getElementsByTagNameNS",
                    1 == xNodeList.getLength());
            assertEquals("XElement.getElementsByTagNameNS",
                    xElemFooNs, xNodeList.item(0));
        }

        {
            String ret = xElemFoo.getAttribute("foo");
            assertEquals("XElement.getAttribute", "", ret);
        }
        {
            String ret = xElemFoo.getAttributeNS(ns, "foo");
            assertEquals("XElement.getAttributeNS", "", ret);
        }
        {
            XNode xAttr = xElemFoo.getAttributeNode("foo");
            assertNull("XElement.getAttributeNode", xAttr);
        }
        {
            XNode xAttr = xElemFoo.getAttributeNodeNS(ns, "foo");
            assertNull("XElement.getAttributeNodeNS", xAttr);
        }
        assertFalse("XElement.hasAttribute", xElemFoo.hasAttribute("foo"));
        assertFalse("XElement.hasAttributeNS",
                xElemFoo.hasAttributeNS(ns, "foo"));

        // surprisingly this does not throw?
        xElemFoo.removeAttribute("foo");
        xElemFoo.removeAttributeNS(ns, "foo");

        XAttr xAttr = xDoc.createAttribute("foo");
        XAttr xAttrNs = xDoc.createAttributeNS(ns, "foo");

        try {
            xElemFoo.removeAttributeNode(null);
            fail("XElement.removeAttributeNode(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xElemFoo.removeAttributeNode(xAttr);
            fail("XElement.removeAttributeNode(xAttr)");
        } catch (DOMException e) {
            assertTrue("XElement.removeAttributeNode(xAttr)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        /* FIXME
        try {
            xElemFoo.setAttribute("&", "foo");
            fail("XElement.setAttribute(\"&\")");
        } catch (DOMException e) {
            assertTrue("XElement.setAttribute(\"&\")",
                    INVALID_CHARACTER_ERR == e.Code);
        }
        try {
            xElemFoo.setAttributeNS(ns, "&", "foo");
            fail("XElement.setAttributeNS(\"&\")");
        } catch (DOMException e) {
            assertTrue("XElement.setAttributeNS(\"&\")",
                    INVALID_CHARACTER_ERR == e.Code);
        }
        */

        XAttr xAttrSet = xElemFoo.setAttributeNode(xAttr);
        assertEquals("XElement.setAttributeNode(xAttr)",
                xAttrSet, xElemFoo.getAttributeNode("foo"));
        assertEquals("XElement.setAttributeNode(xAttr)",
                xElemFoo, xAttrSet.getOwnerElement());
        try {
            xElemFooNs.setAttributeNode(xAttrSet);
            fail("XElement.setAttributeNode(xAttrSet)");
        } catch (DOMException e) {
            assertTrue("XElement.setAttributeNode(xAttrSet)",
                    INUSE_ATTRIBUTE_ERR == e.Code);
        }

        XAttr xAttrNsSet = xElemFooNs.setAttributeNodeNS(xAttrNs);
        assertEquals("XElement.setAttributeNodeNS(xAttr)",
                xAttrNsSet, xElemFooNs.getAttributeNodeNS(ns, "foo"));
        assertEquals("XElement.setAttributeNodeNS(xAttrNs)",
                xElemFooNs, xAttrNsSet.getOwnerElement());
        try {
            xElemFooNs.setAttributeNodeNS(xAttrNsSet);
            fail("XElement.setAttributeNodeNS(xAttrNsSet)");
        } catch (DOMException e) {
            assertTrue("XElement.setAttributeNodeNS(xAttrNsSet)",
                    INUSE_ATTRIBUTE_ERR == e.Code);
        }

        XAttr xAttrRemoved = xElemFoo.removeAttributeNode(xAttrSet);
        assertNotNull("XElement.removeAttributeNode(xAttrSet)", xAttrRemoved);
        assertEquals("XElement.removeAttributeNode(xAttrSet)",
                "foo", xAttrRemoved.getName());
        assertNull("XElement.removeAttributeNode(xAttrSet)",
                xAttrRemoved.getOwnerElement());

        XAttr xAttrNsRemoved = xElemFooNs.removeAttributeNode(xAttrNsSet);
        assertNotNull("XElement.removeAttributeNode(xAttrNsSet)",
                xAttrNsRemoved);
        assertEquals("XElement.removeAttributeNode(xAttrNsSet)",
                "foo", xAttrNsRemoved.getName());
        assertNull("XElement.removeAttributeNode(xAttrNsSet)",
                xAttrNsRemoved.getOwnerElement());


        xElemFoo.setAttribute("foo", "bar");
        assertEquals("XElement.setAttribute()",
                "bar", xElemFoo.getAttribute("foo"));

        xElemFooNs.setAttributeNS(ns, "foo", "bar");
        assertEquals("XElement.setAttributeNS()",
                "bar", xElemFooNs.getAttributeNS(ns, "foo"));

        xElemFoo.removeAttribute("foo");
        assertNull("XElement.removeAttribute",
                xElemFoo.getAttributeNode("foo"));

        xElemFooNs.removeAttributeNS(ns, "foo");
        assertNull("XElement.removeAttributeNS",
                xElemFooNs.getAttributeNodeNS(ns, "foo"));

        // XNode ////////////////////////////////////////////////////

        XText xText = xDoc.createTextNode("foo");
        XComment xComment = xDoc.createComment("foo");

        {
            XNamedNodeMap xAttrMap = xElemFoo.getAttributes();
            assertNotNull("XElement.getAttributes", xAttrMap);
            assertTrue("XElement.getAttributes", 0 == xAttrMap.getLength());
            assertFalse("XElement.hasAttributes()", xElemFoo.hasAttributes());
        }

        xElemFooNs.setAttribute("foo", "bar");
        xElemFoo.setAttributeNS(ns, "foo", "bar");

        {
            XNamedNodeMap xAttrMap = xElemFoo.getAttributes();
            assertNotNull("XElement.getAttributes", xAttrMap);
            assertTrue("XElement.getAttributes", 1 == xAttrMap.getLength());
            XNode xAttr_ = xAttrMap.getNamedItemNS(ns, "foo");
            assertNotNull("XElement.getAttributes", xAttr_);
        }
        {
            XNamedNodeMap xAttrMap = xElemFooNs.getAttributes();
            assertNotNull("XElement.getAttributes", xAttrMap);
            assertTrue("XElement.getAttributes", 1 == xAttrMap.getLength());
            XNode xAttr_ = xAttrMap.getNamedItem("foo");
            assertNotNull("XElement.getAttributes", xAttr_);
        }

        {
            XNode xElemFooCloneN = xElemFoo.cloneNode(false);
            assertNotNull("XElement.cloneNode(false)", xElemFooCloneN);
            XElement xElemFooClone =
                UnoRuntime.queryInterface(XElement.class, xElemFooCloneN);
            assertNotNull("XElement.cloneNode(false)", xElemFooClone);
            assertFalse("XElement.cloneNode(false)",
                    xElemFooClone.hasChildNodes());
            assertNull("XElement.cloneNode(false)",
                    xElemFooClone.getFirstChild());
        }
        {
            XNode xElemFooCloneN = xElemFoo.cloneNode(true);
            assertNotNull("XElement.cloneNode(true)", xElemFooCloneN);
            XElement xElemFooClone =
                UnoRuntime.queryInterface(XElement.class, xElemFooCloneN);
            assertNotNull("XElement.cloneNode(true)", xElemFooClone);
            assertTrue("XElement.cloneNode(true)",
                    xElemFooClone.hasChildNodes());
            assertTrue("XElement.cloneNode(true)",
                    xElemFooClone.hasAttributeNS(ns, "foo"));
            XNode xChild = xElemFooClone.getFirstChild();
            assertNotNull("XElement.cloneNode(true)", xChild);
            XElement xElemFooNsClone =
                UnoRuntime.queryInterface(XElement.class, xChild);
            assertNotNull("XElement.cloneNode(true)", xElemFooNsClone);
            assertEquals("XElement.cloneNode(true)", "foo",
                    xElemFooNsClone.getLocalName());
            assertEquals("XElement.cloneNode(true)", ns,
                    xElemFooNsClone.getNamespaceURI());
            assertTrue("XElement.cloneNode(true)",
                    xElemFooNsClone.hasAttribute("foo"));
        }

        {
            XNodeList xChildren = xElemFoo.getChildNodes();
            assertTrue("XElement.getChildNodes()", 1 == xChildren.getLength());
            assertEquals("XElement.getChildNodes()",
                    xElemFooNs, xChildren.item(0));

            XNode xFirst = xElemFoo.getFirstChild();
            assertEquals("XDocument.getFirstChild()", xElemFooNs, xFirst);
            XNode xLast = xElemFoo.getLastChild();
            assertEquals("XDocument.getLastChild()", xElemFooNs, xLast);
        }

        assertEquals("XElement.getLocalName()", "foo", xElemFoo.getLocalName());
        assertEquals("XElement.getLocalName()", "foo",
                xElemFooNs.getLocalName());

        assertEquals("XElement.getNamespaceURI()", "",
                xElemFoo.getNamespaceURI());
        assertEquals("XElement.getNamespaceURI()", ns,
                xElemFooNs.getNamespaceURI());

        assertNull("XElement.getNextSibling()", xElemFoo.getNextSibling());

        assertEquals("XElement.getNodeName()", "foo", xElemFoo.getNodeName());
        assertEquals("XElement.getNodeName()", "foo",
                xElemFooNs.getNodeName());

        assertTrue("XElement.getNodeType()",
                ELEMENT_NODE == xElemFoo.getNodeType());

        assertEquals("XElement.getNodeValue()", "", xElemFoo.getNodeValue());

        assertEquals("XElement.getOwnerDocument()",
                xDoc, xElemFoo.getOwnerDocument());

        assertNull("XElement.getParentNode()", xElemFoo.getParentNode());
        assertEquals("XElement.getParentNode()",
                xElemFoo, xElemFooNs.getParentNode());

        assertEquals("XElement.getPrefix()", "", xElemFoo.getPrefix());
        assertEquals("XElement.getPrefix()", "e", xElemFooNs.getPrefix());

        assertNull("XElement.getPreviousSibling()",
                xElemFoo.getPreviousSibling());

        assertTrue("XElement.hasAttributes()", xElemFoo.hasAttributes());

        assertTrue("XElement.hasChildNodes()", xElemFoo.hasChildNodes());
        assertFalse("XElement.hasChildNodes()", xElemFooNs.hasChildNodes());

        assertFalse("XElement.isSupported()",
                xElemFoo.isSupported("frobnication", "v99.33.0.0.0.1"));

        xElemFoo.normalize();

        try {
            xElemFoo.setNodeValue("42");
            fail("XElement.setNodeValue()");
        } catch (DOMException e) {
            assertTrue("XElement.setNodeValue()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        xElemFooNs.setPrefix("f");
        assertEquals("XElement.getPrefix()", "f", xElemFooNs.getPrefix());

        try {
            xElemFoo.appendChild(null);
            fail("XElement.appendChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xElemFoo.insertBefore(null, xText);
            fail("XElemFoo.insertBefore(null,)");
        } catch (Exception e) { /* expected */ }
        try {
            xElemFoo.insertBefore(xText, null);
            fail("XElemFoo.insertBefore(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xElemFoo.insertBefore(xText, xText);
            fail("XElement.insertBefore(x, x)");
        } catch (DOMException e) {
            assertTrue("XDocument.insertBefore(x, x)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        {
            XNode xRet = xElemFoo.insertBefore(xText, xElemFooNs);
            assertEquals("XElement.insertBefore(xText, xElemFooNs)",
                    xRet, xElemFooNs); // why does this return the old node?
            assertEquals("XElement.insertBefore(xText, xElemFooNs)",
                    xText, xElemFoo.getFirstChild());
            assertEquals("XElement.insertBefore(xText, xElemFooNs)",
                    xElemFoo, xText.getParentNode());
            assertEquals("XElement.insertBefore(xText, xElemFooNs)",
                    xElemFooNs, xElemFoo.getLastChild());
        }

        try {
            xElemFoo.replaceChild(null, xText);
            fail("XElement.replaceChild(null, )");
        } catch (Exception e) { /* expected */ }
        try {
            xElemFoo.replaceChild(xText, null);
            fail("XElement.replaceChild(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xElemFoo.replaceChild(xElemFoo, xElemFoo); // not child
            fail("XElement.replaceChild(xElemFoo, xElemFoo)");
        } catch (DOMException e) {
            assertTrue("XElement.replaceChild(xElemFoo, xElemFoo)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        try {
            xElemFoo.replaceChild(xElemFooNs, xElemFooNs); // child
            assertFalse("XElement.replaceChild(xElemFooNs, xElemFooNs)",
                    false);
        } catch (DOMException e) {
            assertTrue("XElement.replaceChild(xElemFooNs, xElemFooNs)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        XNode xReplaced = xElemFoo.replaceChild(xComment, xText);
        assertEquals("XElement.replaceChild(xComment, xText)",
                xReplaced, xText);
        assertEquals("XElement.replaceChild(xComment, xText)",
                xComment, xElemFoo.getFirstChild());
        assertEquals("XElement.replaceChild(xComment, xText)",
                xElemFooNs, xElemFoo.getLastChild());

        try {
            xElemFoo.removeChild(null);
            fail("XElement.removeChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xElemFoo.removeChild(xElemFoo);
            fail("XElement.removeChild()");
        } catch (DOMException e) {
            assertTrue("XElement.removeChild()",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xRemoved = xElemFoo.removeChild(xComment);
        assertEquals("XElement.removeChild(xComment)", xRemoved, xComment);
        assertTrue("XElement.removeChild(xComment)", xElemFoo.hasChildNodes());
        assertEquals("XElement.removeChild(xComment)",
                xElemFooNs, xElemFoo.getFirstChild());
        assertEquals("XElement.removeChild(xComment)",
                xElemFooNs, xElemFoo.getLastChild());
    }

    @Test public void testXAttr() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        String ns = "http://example.com/";

        XAttr xAttr = xDoc.createAttribute("foo");
        assertNotNull("XDocument.createAttribute", xAttr);

        XAttr xAttrNs = xDoc.createAttributeNS(ns, "e:foo");
        assertNotNull("XDocument.createAttribute", xAttr);

        assertTrue("XAttr.getSpecified", xAttr.getSpecified());

        assertEquals("XAttr.getName()", "foo", xAttr.getName());

        assertNull("XAttr.getOwnerElement()", xAttr.getOwnerElement());

        XElement xElemFoo = xDoc.createElement("foo");
        XNode xInserted = xElemFoo.appendChild(xAttr);
        XAttr xAttrIns =
            UnoRuntime.queryInterface(XAttr.class, xInserted);
        assertNotNull(xAttrIns);
        assertEquals("XAttr.getOwnerElement()",
                xElemFoo, xAttrIns.getOwnerElement());

        assertEquals("XAttr.getValue()", "", xAttr.getValue());

        xAttr.setValue("bar");
        assertEquals("XAttr.setValue()", "bar", xAttr.getValue());

        // XNode ////////////////////////////////////////////////////

        {
            XNode xAttrCloneN = xAttr.cloneNode(false);
            assertNotNull("XAttr.cloneNode(false)", xAttrCloneN);
            XAttr xAttrClone =
                UnoRuntime.queryInterface(XAttr.class, xAttrCloneN);
            assertNotNull("XAttr.cloneNode(false)", xAttrClone);
            // actually the children are copied even if bDeep=false
            // does that make sense for attributes?
            /*
            assertFalse("XAttr.cloneNode(false)", xAttrClone.hasChildNodes());
            assertNull("XAttr.cloneNode(false)", xAttrClone.getFirstChild());
            */
            assertTrue("XAttr.cloneNode(true)", xAttrClone.hasChildNodes());
            XNode xChild = xAttrClone.getFirstChild();
            assertNotNull("XAttr.cloneNode(true)", xChild);
            XText xText = UnoRuntime.queryInterface(XText.class, xChild);
            assertNotNull("XAttr.cloneNode(true)", xText);
            assertEquals("XAttr.cloneNode(true)", "bar", xText.getNodeValue());
        }
        {
            XNode xAttrCloneN = xAttr.cloneNode(true);
            assertNotNull("XAttr.cloneNode(true)", xAttrCloneN);
            XAttr xAttrClone =
                UnoRuntime.queryInterface(XAttr.class, xAttrCloneN);
            assertNotNull("XAttr.cloneNode(true)", xAttrClone);
            assertTrue("XAttr.cloneNode(true)", xAttrClone.hasChildNodes());
            XNode xChild = xAttrClone.getFirstChild();
            assertNotNull("XAttr.cloneNode(true)", xChild);
            XText xText = UnoRuntime.queryInterface(XText.class, xChild);
            assertNotNull("XAttr.cloneNode(true)", xText);
            assertEquals("XAttr.cloneNode(true)", "bar", xText.getNodeValue());
        }

        assertNull("XAttr.getAttributes()", xAttr.getAttributes());

        {
            XNodeList xChildren = xAttr.getChildNodes();
            assertTrue("XAttr.getChildNodes()", 1 == xChildren.getLength());
            XNode xChild = xChildren.item(0);
            assertNotNull("XAttr.getChildNodes()", xChild);
            XText xText = UnoRuntime.queryInterface(XText.class, xChild);
            assertNotNull("XAttr.getChildNodes()", xText);

            XNode xFirst = xAttr.getFirstChild();
            assertEquals("XAttr.getFirstChild()", xText, xFirst);
            XNode xLast = xAttr.getLastChild();
            assertEquals("XAttr.getLastChild()", xText, xLast);
        }

        assertEquals("XAttr.getLocalName()", "foo", xAttr.getLocalName());
        assertEquals("XAttr.getLocalName()", "foo", xAttrNs.getLocalName());

        assertEquals("XAttr.getNamespaceURI()", "", xAttr.getNamespaceURI());
        assertEquals("XAttr.getNamespaceURI()", ns, xAttrNs.getNamespaceURI());

        assertNull("XAttr.getNextSibling()", xAttr.getNextSibling());

        assertEquals("XAttr.getNodeName()", "foo", xAttr.getNodeName());
        assertEquals("XAttr.getNodeName()", "foo", xAttrNs.getNodeName());

        assertTrue("XAttr.getNodeType()",
                ATTRIBUTE_NODE == xAttr.getNodeType());

        assertEquals("XAttr.getNodeValue()", "bar", xAttr.getNodeValue());
        assertEquals("XAttr.getNodeValue()", "", xAttrNs.getNodeValue());

        assertEquals("XAttr.getOwnerDocument()",
                xDoc, xDoc.getOwnerDocument());

        assertNull("XAttr.getParentNode()", xAttr.getParentNode());

        assertEquals("XAttr.getPrefix()", "", xAttr.getPrefix());
        assertEquals("XAttr.getPrefix()", "e", xAttrNs.getPrefix());

        assertNull("XAttr.getPreviousSibling()", xAttr.getPreviousSibling());

        assertFalse("XAttr.hasAttributes()", xAttr.hasAttributes());

        assertTrue("XAttr.hasChildNodes()", xAttr.hasChildNodes());

        assertFalse("XAttr.isSupported()",
                xAttr.isSupported("frobnication", "v99.33.0.0.0.1"));

        xAttr.normalize();

        xAttr.setNodeValue("42");
        assertEquals("XAttr.setNodeValue()", "42", xAttr.getNodeValue());

        xAttrNs.setPrefix("f");
        assertEquals("XAttr.setPrefix()", "f", xAttrNs.getPrefix());

        XText xText = xDoc.createTextNode("baz");
        XText xTextNew = xDoc.createTextNode("quux");

        try {
            xAttr.appendChild(null);
            fail("XAttr.appendChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xAttr.insertBefore(null, xText);
            fail("XAttr.insertBefore(null,)");
        } catch (Exception e) { /* expected */ }
        try {
            xAttr.insertBefore(xText, null);
            fail("XAttr.insertBefore(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xAttr.insertBefore(xText, xText);
            fail("XAttr.insertBefore(x, x)");
        } catch (DOMException e) {
            assertTrue("XAttr.insertBefore(x, x)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xChild = xAttr.getFirstChild();
        assertNotNull(xChild);

        {
            XNode xRet = xAttr.insertBefore(xText, xChild);
            assertEquals("XAttr.insertBefore(xText, xChild)",
                    xRet, xChild); // why does this return the old node?
            assertEquals("XAttr.insertBefore(xText, xChild)",
                    xText, xAttr.getFirstChild());
            assertEquals("XAttr.insertBefore(xText, xChild)",
                    xAttr, xText.getParentNode());
            assertEquals("XAttr.insertBefore(xText, xChild)",
                    xChild, xAttr.getLastChild());
        }

        try {
            xAttr.replaceChild(null, xText);
            fail("XAttr.replaceChild(null, )");
        } catch (Exception e) { /* expected */ }
        try {
            xAttr.replaceChild(xText, null);
            fail("XAttr.replaceChild(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xAttr.replaceChild(xAttrNs, xAttrNs); // not child
            fail("XAttr.replaceChild(xAttrNs, xAttrNs)");
        } catch (DOMException e) {
            assertTrue("XAttr.replaceChild(xAttrNs, xAttrNs)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        try {
            xAttr.replaceChild(xChild, xChild); // child
            assertFalse("XAttr.replaceChild(xChild, xChild)",
                    false);
        } catch (DOMException e) {
            assertTrue("XAttr.replaceChild(xChild, xChild)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        XNode xReplaced = xAttr.replaceChild(xTextNew, xChild);
        assertEquals("XAttr.replaceChild(xTextNew, xChild)", xChild, xReplaced);
        assertEquals("XAttr.replaceChild(xTextNew, xChild)",
                xText, xAttr.getFirstChild());
        assertEquals("XAttr.replaceChild(xTextNew, xChild)",
                xTextNew, xAttr.getLastChild());

        try {
            xAttr.removeChild(null);
            fail("XAttr.removeChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xAttr.removeChild(xAttrNs);
            fail("XAttr.removeChild()");
        } catch (DOMException e) {
            assertTrue("XAttr.removeChild()", HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xRemoved = xAttr.removeChild(xTextNew);
        assertEquals("XAttr.removeChild(xText)", xRemoved, xTextNew);
        assertTrue("XAttr.removeChild(xText)", xAttr.hasChildNodes());
        assertEquals("XAttr.removeChild(xText)",
                xText, xAttr.getFirstChild());
        assertEquals("XAttr.removeChild(xText)",
                xText, xAttr.getLastChild());
    }

    @Test public void testXText() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XText xText = xDoc.createTextNode("foobar");
        assertNotNull(xText);

        assertEquals("XText.getData", "foobar", xText.getData());
        assertEquals("XText.getLength", 6, xText.getLength());

        /* FIXME
        try {
            xText.splitText(9999);
            fail("XText.splitText(9999)");
        } catch (DOMException e) {
            assertTrue("XText.splitText(9999)", INDEX_SIZE_ERR == e.Code);
        }

        {
            XText xTextBar = xText.splitText(2);
            assertNotNull("XText.splitText", xTextBar);
            assertEquals("XText.splitText", "foo", xText.getData());
            assertEquals("XText.splitText", "bar", xTextBar.getData());
        }
        */
        xText.setData("foo");

        xText.appendData("baz");
        assertEquals("XText.appendData", "foobaz", xText.getData());

        try {
            xText.deleteData(999,999);
            fail("XText.deleteData(999,999)");
        } catch (DOMException e) {
            assertTrue("XText.deleteData(999,999)", INDEX_SIZE_ERR == e.Code);
        }
        xText.deleteData(0, 3);
        assertEquals("XText.deleteData", "baz", xText.getData());

        try {
            xText.insertData(999,"blah");
            fail("XText.insertData(999,\"blah\")");
        } catch (DOMException e) {
            assertTrue("XText.insertData(999,\"blah\")",
                INDEX_SIZE_ERR == e.Code);
        }
        xText.insertData(1, "arb");
        assertEquals("XText.insertData", "barbaz", xText.getData());

        try {
            xText.replaceData(999,999,"x");
            fail("XText.replaceData(999,999,\"x\")");
        } catch (DOMException e) {
            assertTrue("XText.replaceData(999,999,\"x\")",
                    INDEX_SIZE_ERR == e.Code);
        }
        xText.replaceData(3, 3, "foo");
        assertEquals("XText.replaceData", "barfoo", xText.getData());

        xText.setData("quux");
        assertEquals("XText.setData", "quux", xText.getData());

        try {
            xText.subStringData(999,999);
            fail("XText.subStringData(999,999)");
        } catch (DOMException e) {
            assertTrue("XText.subStringData(999,999)",
                INDEX_SIZE_ERR == e.Code);
        }
        assertEquals("XText.subStringData", "x", xText.subStringData(3, 1));

        // XNode ////////////////////////////////////////////////////

        {
            XNode xTextCloneN = xText.cloneNode(false);
            assertNotNull("XText.cloneNode(false)", xTextCloneN);
            XText xTextClone =
                UnoRuntime.queryInterface(XText.class, xTextCloneN);
            assertNotNull("XText.cloneNode(false)", xTextClone);
            assertFalse("XText.cloneNode(false)",
                    xTextClone.hasChildNodes());
        }
        {
            XNode xTextCloneN = xText.cloneNode(true);
            assertNotNull("XText.cloneNode(true)", xTextCloneN);
            XText xTextClone =
                UnoRuntime.queryInterface(XText.class, xTextCloneN);
            assertNotNull("XText.cloneNode(true)", xTextClone);
            assertFalse("XText.cloneNode(true)", xTextClone.hasChildNodes());
        }

        assertNull("XText.getAttributes()", xText.getAttributes());

        {
            XNodeList xChildren = xText.getChildNodes();
            assertTrue("XText.getChildNodes()", 0 == xChildren.getLength());
        }

        assertEquals("XText.getLocalName()", "", xText.getLocalName());

        assertEquals("XText.getNamespaceURI()", "", xText.getNamespaceURI());

        assertNull("XText.getNextSibling()", xText.getNextSibling());

        assertEquals("XText.getNodeName()", "#text", xText.getNodeName());

        assertTrue("XText.getNodeType()",
                TEXT_NODE == xText.getNodeType());

        assertEquals("XText.getNodeValue()", "quux", xText.getNodeValue());

        assertEquals("XText.getOwnerDocument()",
                xDoc, xText.getOwnerDocument());

        assertNull("XText.getParentNode()", xText.getParentNode());

        assertEquals("XText.getPrefix()", "", xText.getPrefix());

        assertNull("XText.getPreviousSibling()", xText.getPreviousSibling());

        assertFalse("XText.hasAttributes()", xText.hasAttributes());

        assertFalse("XText.hasChildNodes()", xText.hasChildNodes());

        assertFalse("XText.isSupported()",
                xText.isSupported("frobnication", "v99.33.0.0.0.1"));

        xText.normalize();

        xText.setNodeValue("42");
        assertEquals("XText.setNodeValue()", "42", xText.getNodeValue());

        try {
            xText.setPrefix("foo");
            fail("XText.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XText.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        XText xText2 = xDoc.createTextNode("foobar");
        XText xText3 = xDoc.createTextNode("foobar");

        try {
            xText.appendChild(null);
            fail("XText.appendChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xText.appendChild(xText2);
            fail("XText.appendChild(xText2)");
        } catch (DOMException e) {
            assertTrue("XText.appendChild(xText2)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        try {
            xText.insertBefore(xText2, xText3);
            fail("XText.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xText.replaceChild(xText2, xText3);
            fail("XText.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xText.removeChild(null);
            fail("XText.removeChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xText.removeChild(xText2);
            fail("XText.removeChild");
        } catch (DOMException e) {
            assertTrue("XText.removeChild", HIERARCHY_REQUEST_ERR == e.Code);
        }
    }

    @Test public void testXCDataSection() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XCDATASection xCDS = xDoc.createCDATASection("foobar");
        assertNotNull(xCDS);

        assertEquals("XCDATASection.getData", "foobar", xCDS.getData());
        assertEquals("XCDATASection.getLength", 6, xCDS.getLength());

        /* FIXME
        try {
            xCDS.splitText(9999);
            fail("XCDATASection.splitText(9999)");
        } catch (DOMException e) {
            assertTrue("XCDATASection.splitText(9999)",
                INDEX_SIZE_ERR == e.Code);
        }

        {
            XCDATASection xCDSBar = xCDS.splitText(2);
            assertNotNull("XCDATASection.splitText", xCDSBar);
            assertEquals("XCDATASection.splitText", "foo", xCDS.getData());
            assertEquals("XCDATASection.splitText", "bar", xCDSBar.getData());
        }
        */
        xCDS.setData("foo");

        xCDS.appendData("baz");
        assertEquals("XCDATASection.appendData", "foobaz", xCDS.getData());

        try {
            xCDS.deleteData(999,999);
            fail("XCDATASection.deleteData(999,999)");
        } catch (DOMException e) {
            assertTrue("XCDATASection.deleteData(999,999)",
                    INDEX_SIZE_ERR == e.Code);
        }
        xCDS.deleteData(0, 3);
        assertEquals("XCDATASection.deleteData", "baz", xCDS.getData());

        try {
            xCDS.insertData(999,"blah");
            fail("XCDATASection.insertData(999,\"blah\")");
        } catch (DOMException e) {
            assertTrue("XCDATASection.insertData(999,\"blah\")",
                INDEX_SIZE_ERR == e.Code);
        }
        xCDS.insertData(1, "arb");
        assertEquals("XCDATASection.insertData", "barbaz", xCDS.getData());

        try {
            xCDS.replaceData(999,999,"x");
            fail("XCDATASection.replaceData(999,999,\"x\")");
        } catch (DOMException e) {
            assertTrue("XCDATASection.replaceData(999,999,\"x\")",
                    INDEX_SIZE_ERR == e.Code);
        }
        xCDS.replaceData(3, 3, "foo");
        assertEquals("XCDATASection.replaceData", "barfoo", xCDS.getData());

        xCDS.setData("quux");
        assertEquals("XCDATASection.setData", "quux", xCDS.getData());

        try {
            xCDS.subStringData(999,999);
            fail("XCDATASection.subStringData(999,999)");
        } catch (DOMException e) {
            assertTrue("XCDATASection.subStringData(999,999)",
                INDEX_SIZE_ERR == e.Code);
        }
        assertEquals("XCDATASection.subStringData", "x",
                xCDS.subStringData(3, 1));

        // XNode ////////////////////////////////////////////////////

        {
            XNode xCDSCloneN = xCDS.cloneNode(false);
            assertNotNull("XCDATASection.cloneNode(false)", xCDSCloneN);
            XCDATASection xCDSClone =
                UnoRuntime.queryInterface(XCDATASection.class, xCDSCloneN);
            assertNotNull("XCDATASection.cloneNode(false)", xCDSClone);
            assertFalse("XCDATASection.cloneNode(false)",
                    xCDSClone.hasChildNodes());
        }
        {
            XNode xCDSCloneN = xCDS.cloneNode(true);
            assertNotNull("XCDATASection.cloneNode(true)", xCDSCloneN);
            XCDATASection xCDSClone =
                UnoRuntime.queryInterface(XCDATASection.class, xCDSCloneN);
            assertNotNull("XCDATASection.cloneNode(true)", xCDSClone);
            assertFalse("XCDATASection.cloneNode(true)",
                    xCDSClone.hasChildNodes());
        }

        assertNull("XCDATASection.getAttributes()", xCDS.getAttributes());

        {
            XNodeList xChildren = xCDS.getChildNodes();
            assertTrue("XCDATASection.getChildNodes()",
                    0 == xChildren.getLength());
        }

        assertEquals("XCDATASection.getLocalName()", "", xCDS.getLocalName());

        assertEquals("XCDATASection.getNamespaceURI()", "",
                xCDS.getNamespaceURI());

        assertNull("XCDATASection.getNextSibling()", xCDS.getNextSibling());

        assertEquals("XCDATASection.getNodeName()", "#cdata-section",
                xCDS.getNodeName());

        assertTrue("XCDATASection.getNodeType()",
                CDATA_SECTION_NODE == xCDS.getNodeType());

        assertEquals("XCDATASection.getNodeValue()", "quux",
                xCDS.getNodeValue());

        assertEquals("XCDATASection.getOwnerDocument()",
                xDoc, xCDS.getOwnerDocument());

        assertNull("XCDATASection.getParentNode()", xCDS.getParentNode());

        assertEquals("XCDATASection.getPrefix()", "", xCDS.getPrefix());

        assertNull("XCDATASection.getPreviousSibling()",
                xCDS.getPreviousSibling());

        assertFalse("XCDATASection.hasAttributes()", xCDS.hasAttributes());

        assertFalse("XCDATASection.hasChildNodes()", xCDS.hasChildNodes());

        assertFalse("XCDATASection.isSupported()",
                xCDS.isSupported("frobnication", "v99.33.0.0.0.1"));

        xCDS.normalize();

        xCDS.setNodeValue("42");
        assertEquals("XCDATASection.setNodeValue()", "42", xCDS.getNodeValue());

        try {
            xCDS.setPrefix("foo");
            fail("XCDATASection.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XCDATASection.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        XCDATASection xCDS2 = xDoc.createCDATASection("foobar");
        XCDATASection xCDS3 = xDoc.createCDATASection("foobar");

        try {
            xCDS.appendChild(null);
            fail("XCDATASection.appendChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xCDS.appendChild(xCDS2);
            fail("XCDATASection.appendChild(xCDS2)");
        } catch (DOMException e) {
            assertTrue("XCDATASection.appendChild(xCDS2)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        try {
            xCDS.insertBefore(xCDS2, xCDS3);
            fail("XCDATASection.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xCDS.replaceChild(xCDS2, xCDS3);
            fail("XCDATASection.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xCDS.removeChild(null);
            fail("XCDATASection.removeChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xCDS.removeChild(xCDS2);
            fail("XCDATASection.removeChild");
        } catch (DOMException e) {
            assertTrue("XCDATASection.removeChild",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

    }

    @Test public void testXComment() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XComment xComment = xDoc.createComment("foo");
        assertNotNull(xComment);

        assertEquals("XComment.getData", "foo", xComment.getData());
        assertEquals("XComment.getLength", 3, xComment.getLength());

        xComment.appendData("baz");
        assertEquals("XComment.appendData", "foobaz", xComment.getData());

        try {
            xComment.deleteData(999,999);
            fail("XComment.deleteData(999,999)");
        } catch (DOMException e) {
            assertTrue("XComment.deleteData(999,999)",
                    INDEX_SIZE_ERR == e.Code);
        }
        xComment.deleteData(0, 3);
        assertEquals("XComment.deleteData", "baz", xComment.getData());

        try {
            xComment.insertData(999,"blah");
            fail("XComment.insertData(999,\"blah\")");
        } catch (DOMException e) {
            assertTrue("XComment.insertData(999,\"blah\")",
                INDEX_SIZE_ERR == e.Code);
        }
        xComment.insertData(1, "arb");
        assertEquals("XComment.insertData", "barbaz", xComment.getData());

        try {
            xComment.replaceData(999,999,"x");
            fail("XComment.replaceData(999,999,\"x\")");
        } catch (DOMException e) {
            assertTrue("XComment.replaceData(999,999,\"x\")",
                    INDEX_SIZE_ERR == e.Code);
        }
        xComment.replaceData(3, 3, "foo");
        assertEquals("XComment.replaceData", "barfoo", xComment.getData());

        xComment.setData("quux");
        assertEquals("XComment.setData", "quux", xComment.getData());

        try {
            xComment.subStringData(999,999);
            fail("XComment.subStringData(999,999)");
        } catch (DOMException e) {
            assertTrue("XComment.subStringData(999,999)",
                INDEX_SIZE_ERR == e.Code);
        }
        assertEquals("XComment.subStringData", "x",
                xComment.subStringData(3, 1));

        // XNode ////////////////////////////////////////////////////

        {
            XNode xCommentCloneN = xComment.cloneNode(false);
            assertNotNull("XComment.cloneNode(false)", xCommentCloneN);
            XComment xCommentClone =
                UnoRuntime.queryInterface(XComment.class, xCommentCloneN);
            assertNotNull("XComment.cloneNode(false)", xCommentClone);
            assertFalse("XComment.cloneNode(false)",
                    xCommentClone.hasChildNodes());
        }
        {
            XNode xCommentCloneN = xComment.cloneNode(true);
            assertNotNull("XComment.cloneNode(true)", xCommentCloneN);
            XComment xCommentClone =
                UnoRuntime.queryInterface(XComment.class, xCommentCloneN);
            assertNotNull("XComment.cloneNode(true)", xCommentClone);
            assertFalse("XComment.cloneNode(true)",
                    xCommentClone.hasChildNodes());
        }

        assertNull("XComment.getAttributes()", xComment.getAttributes());

        {
            XNodeList xChildren = xComment.getChildNodes();
            assertTrue("XComment.getChildNodes()", 0 == xChildren.getLength());
        }

        assertEquals("XComment.getLocalName()", "", xComment.getLocalName());

        assertEquals("XComment.getNamespaceURI()", "",
                xComment.getNamespaceURI());

        assertNull("XComment.getNextSibling()", xComment.getNextSibling());

        assertEquals("XComment.getNodeName()", "#comment",
                xComment.getNodeName());

        assertTrue("XComment.getNodeType()",
                COMMENT_NODE == xComment.getNodeType());

        assertEquals("XComment.getNodeValue()", "quux",
                xComment.getNodeValue());

        assertEquals("XComment.getOwnerDocument()",
                xDoc, xComment.getOwnerDocument());

        assertNull("XComment.getParentNode()", xComment.getParentNode());

        assertEquals("XComment.getPrefix()", "", xComment.getPrefix());

        assertNull("XComment.getPreviousSibling()",
                xComment.getPreviousSibling());

        assertFalse("XComment.hasAttributes()", xComment.hasAttributes());

        assertFalse("XComment.hasChildNodes()", xComment.hasChildNodes());

        assertFalse("XComment.isSupported()",
                xComment.isSupported("frobnication", "v99.33.0.0.0.1"));

        xComment.normalize();

        xComment.setNodeValue("42");
        assertEquals("XComment.setNodeValue()", "42", xComment.getNodeValue());

        try {
            xComment.setPrefix("foo");
            fail("XComment.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XComment.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        XComment xComment2 = xDoc.createComment("foobar");
        XComment xComment3 = xDoc.createComment("foobar");

        try {
            xComment.appendChild(null);
            fail("XComment.appendChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xComment.appendChild(xComment2);
            fail("XComment.appendChild(xComment2)");
        } catch (DOMException e) {
            assertTrue("XComment.appendChild(xComment2)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        try {
            xComment.insertBefore(xComment2, xComment3);
            fail("XComment.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xComment.replaceChild(xComment2, xComment3);
            fail("XComment.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xComment.removeChild(null);
            fail("XComment.removeChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xComment.removeChild(xComment2);
            fail("XComment.removeChild");
        } catch (DOMException e) {
            assertTrue("XComment.removeChild", HIERARCHY_REQUEST_ERR == e.Code);
        }
    }

    @Test public void testXEntityReference() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XEntityReference xER = xDoc.createEntityReference("foobar");
        assertNotNull(xER);

        XEntityReference xERChild = xDoc.createEntityReference("baz");
        assertNotNull(xERChild);

        xER.appendChild(xERChild);

        // XNode ////////////////////////////////////////////////////

        XText xText = xDoc.createTextNode("foo");
        XComment xComment = xDoc.createComment("foo");

        {
            XNode xERCloneN = xER.cloneNode(false);
            assertNotNull("XEntityReference.cloneNode(false)", xERCloneN);
            XEntityReference xERClone =
                UnoRuntime.queryInterface(XEntityReference.class, xERCloneN);
            assertNotNull("XEntityReference.cloneNode(false)", xERClone);
            assertFalse("XEntityReference.cloneNode(false)",
                    xERClone.hasChildNodes());
            assertNull("XEntityReference.cloneNode(false)",
                    xERClone.getFirstChild());
        }
        {
            XNode xERCloneN = xER.cloneNode(true);
            assertNotNull("XEntityReference.cloneNode(true)", xERCloneN);
            XEntityReference xERClone =
                UnoRuntime.queryInterface(XEntityReference.class, xERCloneN);
            assertNotNull("XEntityReference.cloneNode(true)", xERClone);
            /* FIXME this is actually in libxml2: children are not copied
            assertTrue("XEntityReference.cloneNode(true)",
                    xERClone.hasChildNodes());
            XNode xChild = xERClone.getFirstChild();
            assertNotNull("XEntityReference.cloneNode(true)", xChild);
            XEntityReference xChildER =
                UnoRuntime.queryInterface(XEntityReference.class, xChild);
            assertNotNull("XEntityReference.cloneNode(true)", xChildER);
            assertFalse("XEntityReference.cloneNode(true)",
                    xChildER.equals(xERChild));
            assertEquals("XEntityReference.cloneNode(true)",
                    "baz", xChildER.getLocalName());
            */
        }

        assertNull("XEntityReference.getAttributes()", xER.getAttributes());

        {
            XNodeList xChildren = xER.getChildNodes();
            assertTrue("XEntityReference.getChildNodes()",
                    1 == xChildren.getLength());
            assertEquals("XEntityReference.getChildNodes()",
                    xERChild, xChildren.item(0));

            XNode xFirst = xER.getFirstChild();
            assertEquals("XEntityReference.getFirstChild()",
                    xERChild, xFirst);
            XNode xLast = xER.getLastChild();
            assertEquals("XEntityReference.getLastChild()", xERChild, xLast);
        }

        assertEquals("XEntityReference.getLocalName()", "", xER.getLocalName());

        assertEquals("XEntityReference.getNamespaceURI()", "",
                xER.getNamespaceURI());

        assertNull("XEntityReference.getNextSibling()", xER.getNextSibling());

        assertEquals("XEntityReference.getNodeName()",
                "foobar", xER.getNodeName());

        assertTrue("XEntityReference.getNodeType()",
                ENTITY_REFERENCE_NODE == xER.getNodeType());

        assertEquals("XEntityReference.getNodeValue()", "", xER.getNodeValue());

        assertEquals("XEntityReference.getOwnerDocument()",
                xDoc, xER.getOwnerDocument());

        assertNull("XEntityReference.getParentNode()", xER.getParentNode());

        assertEquals("XEntityReference.getPrefix()", "", xER.getPrefix());

        assertNull("XEntityReference.getPreviousSibling()",
                xER.getPreviousSibling());

        assertFalse("XEntityReference.hasAttributes()", xER.hasAttributes());

        assertTrue("XEntityReference.hasChildNodes()", xER.hasChildNodes());

        assertFalse("XEntityReference.isSupported()",
                xER.isSupported("frobnication", "v99.33.0.0.0.1"));

        xER.normalize();

        try {
            xER.setNodeValue("42");
            fail("XEntityReference.setNodeValue()");
        } catch (DOMException e) {
            assertTrue("XEntityReference.setNodeValue()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xER.setPrefix("foo");
            fail("XEntityReference.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XEntityReference.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        try {
            xER.appendChild(null);
            fail("XEntityReference.appendChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xER.insertBefore(null, xText);
            fail("XEntityReference.insertBefore(null,)");
        } catch (Exception e) { /* expected */ }
        try {
            xER.insertBefore(xText, null);
            fail("XEntityReference.insertBefore(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xER.insertBefore(xText, xText);
            fail("XEntityReference.insertBefore(x, x)");
        } catch (DOMException e) {
            assertTrue("XEntityReference.insertBefore(x, x)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        {
            XNode xRet = xER.insertBefore(xComment, xERChild);
            assertEquals("XEntityReference.insertBefore(xComment, xERChild)",
                    xRet, xERChild); // why does this return the old node?
            assertEquals("XEntityReference.insertBefore(xComment, xERChild)",
                    xComment, xER.getFirstChild());
            assertEquals("XEntityReference.insertBefore(xComment, xERChild)",
                    xER, xComment.getParentNode());
            assertEquals("XEntityReference.insertBefore(xCommnet, xERChild)",
                    xERChild, xER.getLastChild());
        }

        try {
            xER.replaceChild(null, xText);
            fail("XEntityReference.replaceChild(null, )");
        } catch (Exception e) { /* expected */ }
        try {
            xER.replaceChild(xText, null);
            fail("XEntityReference.replaceChild(, null)");
        } catch (Exception e) { /* expected */ }
        try {
            xER.replaceChild(xText, xText); // not child
            fail("XEntityReference.replaceChild(xElemFoo, xElemFoo)");
        } catch (DOMException e) {
            assertTrue("XEntityReference.replaceChild(xElemFoo, xElemFoo)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        try {
            xER.replaceChild(xERChild, xERChild); // child
            assertFalse("XEntityReference.replaceChild(xERChild, xERChild)",
                    false);
        } catch (DOMException e) {
            assertTrue("XEntityReference.replaceChild(xERChild, xERChild)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
        XNode xReplaced = xER.replaceChild(xText, xComment);
        assertEquals("XEntityReference.replaceChild(xText, xComment)",
                xReplaced, xComment);
        assertEquals("XEntityReference.replaceChild(xText, xComment)",
                xText, xER.getFirstChild());
        assertEquals("XEntityReference.replaceChild(xText, xComment)",
                xERChild, xER.getLastChild());

        try {
            xER.removeChild(null);
            fail("XEntityReference.removeChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xER.removeChild(xER);
            fail("XEntityReference.removeChild()");
        } catch (DOMException e) {
            assertTrue("XEntityReference.removeChild()",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        XNode xRemoved = xER.removeChild(xText);
        assertEquals("XEntityReference.removeChild(xText)", xRemoved, xText);
        assertTrue("XEntityReference.removeChild(xText)", xER.hasChildNodes());
        assertEquals("XEntityReference.removeChild(xText)",
                xERChild, xER.getFirstChild());
        assertEquals("XEntityReference.removeChild(xText)",
                xERChild, xER.getLastChild());
    }

    @Test public void testXProcessingInstruction() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XProcessingInstruction xPI =
            xDoc.createProcessingInstruction("foo", "bar");
        assertNotNull(xPI);

        assertEquals("XProcessingInstruction.getTarget",
                "foo", xPI.getTarget());

        assertEquals("XProcessingInstruction.getData", "bar", xPI.getData());

        xPI.setData("baz");
        assertEquals("XProcessingInstruction.setData", "baz", xPI.getData());

        // XNode ////////////////////////////////////////////////////

        {
            XNode xPICloneN = xPI.cloneNode(false);
            assertNotNull("XProcessingInstruction.cloneNode(false)", xPICloneN);
            XProcessingInstruction xPIClone = UnoRuntime.queryInterface(
                    XProcessingInstruction.class, xPICloneN);
            assertNotNull("XProcessingInstruction.cloneNode(false)", xPIClone);
            assertFalse("XProcessingInstruction.cloneNode(false)",
                    xPIClone.hasChildNodes());
        }
        {
            XNode xPICloneN = xPI.cloneNode(true);
            assertNotNull("XProcessingInstruction.cloneNode(true)", xPICloneN);
            XProcessingInstruction xPIClone = UnoRuntime.queryInterface(
                    XProcessingInstruction.class, xPICloneN);
            assertNotNull("XProcessingInstruction.cloneNode(true)", xPIClone);
            assertFalse("XProcessingInstruction.cloneNode(true)",
                    xPIClone.hasChildNodes());
        }

        assertNull("XProcessingInstruction.getAttributes()",
                xPI.getAttributes());

        {
            XNodeList xChildren = xPI.getChildNodes();
            assertTrue("XProcessingInstruction.getChildNodes()",
                    0 == xChildren.getLength());
        }

        assertEquals("XProcessingInstruction.getLocalName()",
                "", xPI.getLocalName());

        assertEquals("XProcessingInstruction.getNamespaceURI()",
                "", xPI.getNamespaceURI());

        assertNull("XProcessingInstruction.getNextSibling()",
                xPI.getNextSibling());

        assertEquals("XProcessingInstruction.getNodeName()",
                "foo", xPI.getNodeName());

        assertTrue("XProcessingInstruction.getNodeType()",
                PROCESSING_INSTRUCTION_NODE == xPI.getNodeType());

        assertEquals("XProcessingInstruction.getNodeValue()",
                "baz", xPI.getNodeValue());

        assertEquals("XProcessingInstruction.getOwnerDocument()",
                xDoc, xPI.getOwnerDocument());

        assertNull("XProcessingInstruction.getParentNode()",
                xPI.getParentNode());

        assertEquals("XProcessingInstruction.getPrefix()", "", xPI.getPrefix());

        assertNull("XProcessingInstruction.getPreviousSibling()",
                xPI.getPreviousSibling());

        assertFalse("XProcessingInstruction.hasAttributes()",
                xPI.hasAttributes());

        assertFalse("XProcessingInstruction.hasChildNodes()",
                xPI.hasChildNodes());

        assertFalse("XProcessingInstruction.isSupported()",
                xPI.isSupported("frobnication", "v99.33.0.0.0.1"));

        xPI.normalize();

        xPI.setNodeValue("42");
        assertEquals("XProcessingInstruction.setNodeValue()",
                "42", xPI.getNodeValue());

        try {
            xPI.setPrefix("foo");
            fail("XProcessingInstruction.setPrefix()");
        } catch (DOMException e) {
            assertTrue("XProcessingInstruction.setPrefix()",
                    NO_MODIFICATION_ALLOWED_ERR == e.Code);
        }

        XText xText2 = xDoc.createTextNode("foobar");
        XText xText3 = xDoc.createTextNode("foobar");

        try {
            xPI.appendChild(null);
            fail("XProcessingInstruction.appendChild(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xPI.appendChild(xText2);
            fail("XProcessingInstruction.appendChild(xText2)");
        } catch (DOMException e) {
            assertTrue("XProcessingInstruction.appendChild(xText2)",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }

        try {
            xPI.insertBefore(xText2, xText3);
            fail("XProcessingInstruction.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xPI.replaceChild(xText2, xText3);
            fail("XProcessingInstruction.insertBefore");
        } catch (Exception e) { /* expected */ }

        try {
            xPI.removeChild(null);
            fail("XProcessingInstruction.removeChild(null)");
        } catch (Exception e) { /* expected */ }

        try {
            xPI.removeChild(xText2);
            fail("XProcessingInstruction.removeChild");
        } catch (DOMException e) {
            assertTrue("XProcessingInstruction.removeChild",
                    HIERARCHY_REQUEST_ERR == e.Code);
        }
    }

    /*
    @Test public void testXEntity() throws Exception
    {
        XEntity xEntity = FIXME  how to get at this shy creature?
    }
    */

    /*
    @Test public void testXNotation() throws Exception
    {
        XNotation xNotation = FIXME how to create?
    }
    */

    /*
    @Test public void testXDocumentType() throws Exception
    {
        XDocumentType xDT = FIXME how to create?
    }
    */

    @Test public void testXNodeList_ChildList() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XElement xRoot = xDoc.createElement("root");
        XElement xFoo = xDoc.createElement("foo");
        XElement xBar = xDoc.createElement("bar");
        XElement xBaz = xDoc.createElement("baz");

        xDoc.appendChild(xRoot);

        XNodeList xChildList = xRoot.getChildNodes();
        assertNotNull(xChildList);
        assertSame("ChildList.getLength()", 0, xChildList.getLength());

        try {
            xChildList.item(4);
        } catch (Exception e) { /* expected */ }

        xRoot.appendChild(xFoo);
        assertSame("ChildList.getLength()", 1, xChildList.getLength());
        assertEquals("ChildList.item", xFoo, xChildList.item(0));

        xRoot.appendChild(xBar);
        assertSame("ChildList.getLength()", 2, xChildList.getLength());
        assertEquals("ChildList.item", xFoo, xChildList.item(0));
        assertEquals("ChildList.item", xBar, xChildList.item(1));

        xRoot.appendChild(xBaz);
        assertSame("ChildList.getLength()", 3, xChildList.getLength());
        assertEquals("ChildList.item", xFoo, xChildList.item(0));
        assertEquals("ChildList.item", xBar, xChildList.item(1));
        assertEquals("ChildList.item", xBaz, xChildList.item(2));

        xRoot.removeChild(xBar);
        assertSame("ChildList.getLength()", 2, xChildList.getLength());
        assertEquals("ChildList.item", xFoo, xChildList.item(0));
        assertEquals("ChildList.item", xBaz, xChildList.item(1));
    }

    @Test public void testXNodeList_ElementList() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        XElement xRoot = xDoc.createElement("root");
        XElement xBar = xDoc.createElement("bar");
        XElement xFoo1 = xDoc.createElement("foo");
        XElement xFoo2 = xDoc.createElement("foo");
        XElement xFoo3 = xDoc.createElement("foo");

        xDoc.appendChild(xRoot);

        XNodeList xElementList = xRoot.getElementsByTagName("foo");
        assertNotNull(xElementList);
        assertSame("ElementList.getLength()", 0, xElementList.getLength());

        try {
            xElementList.item(4);
        } catch (Exception e) { /* expected */ }

        xRoot.appendChild(xFoo1);
        assertSame("ElementList.getLength()", 1, xElementList.getLength());
        assertEquals("ElementList.item", xFoo1, xElementList.item(0));

        xFoo1.appendChild(xBar);
        assertSame("ElementList.getLength()", 1, xElementList.getLength());
        assertEquals("ElementList.item", xFoo1, xElementList.item(0));

        xRoot.appendChild(xFoo3);
        assertSame("ElementList.getLength()", 2, xElementList.getLength());
        assertEquals("ElementList.item", xFoo1, xElementList.item(0));
        assertEquals("ElementList.item", xFoo3, xElementList.item(1));

        xBar.appendChild(xFoo2);
        assertSame("ElementList.getLength()", 3, xElementList.getLength());
        assertEquals("ElementList.item", xFoo1, xElementList.item(0));
        assertEquals("ElementList.item", xFoo2, xElementList.item(1));
        assertEquals("ElementList.item", xFoo3, xElementList.item(2));

        xRoot.removeChild(xFoo1);
        assertSame("ElementList.getLength()", 1, xElementList.getLength());
        assertEquals("ElementList.item", xFoo3, xElementList.item(0));
    }

    @Test public void testXNamedNodeMap_AttributesMap() throws Exception
    {
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));
        XDocument xDoc = xBuilder.newDocument();

        String ns = "http://example.com/";

        XElement xElem = xDoc.createElement("foo");

        XNamedNodeMap xAttributes = xElem.getAttributes();
        assertNotNull(xAttributes);
        assertSame("AttributesMap.getLength()", 0, xAttributes.getLength());

        try {
            xAttributes.item(4);
        } catch (Exception e) { /* expected */ }

        xElem.setAttribute("bar", "42");
        XAttr xAttrBar = xElem.getAttributeNode("bar");
        assertSame("AttributesMap.getLength()", 1, xAttributes.getLength());
        assertEquals("AttributesMap.item", xAttrBar, xAttributes.item(0));
        assertEquals("AttributesMap.getNamedItem",
                xAttrBar, xAttributes.getNamedItem("bar"));

        xElem.setAttributeNS(ns, "n:bar", "43");
        XAttr xAttrBarNs = xElem.getAttributeNodeNS(ns, "bar");
        assertSame("AttributesMap.getLength()", 2, xAttributes.getLength());
        assertEquals("AttributesMap.item", xAttrBar, xAttributes.item(0));
        assertEquals("AttributesMap.item", xAttrBarNs, xAttributes.item(1));
        assertEquals("AttributesMap.getNamedItem",
                xAttrBar, xAttributes.getNamedItem("bar"));
        assertEquals("AttributesMap.getNamedItemNS",
                xAttrBarNs, xAttributes.getNamedItemNS(ns, "bar"));

        XNode xAttrBarNsRem = xAttributes.removeNamedItemNS(ns, "bar");
        assertSame("AttributesMap.getLength()", 1, xAttributes.getLength());
        assertEquals("AttributesMap.removeNamedItemNS",
                xAttrBar, xAttributes.item(0));
        assertEquals("AttributesMap.removeNamedItemNS",
                xAttrBar, xAttributes.getNamedItem("bar"));
        assertNull("AttributesMap.removeNamedItemNS",
                xAttrBarNsRem.getParentNode());

        XNode xAttrBarRem = xAttributes.removeNamedItem("bar");
        assertSame("AttributesMap.getLength()", 0, xAttributes.getLength());
        assertNull("AttributesMap.removeNamedItem",
                xAttrBarRem.getParentNode());

        XNode xAttrBarSetN = xAttributes.setNamedItem(xAttrBarRem);
        assertNotNull("AttributesMap.setNamedItem", xAttrBarSetN);
        XAttr xAttrBarSet =
            UnoRuntime.queryInterface(XAttr.class, xAttrBarSetN);
        assertNotNull("AttributesMap.setNamedItem", xAttrBarSet);
        assertEquals("AttributesMap.setNamedItem",
                xAttrBarSet, xAttributes.getNamedItem("bar"));

        XNode xAttrBarNsSetN = xAttributes.setNamedItemNS(xAttrBarNsRem);
        assertNotNull("AttributesMap.setNamedItemNS", xAttrBarNsSetN);
        XAttr xAttrBarNsSet =
            UnoRuntime.queryInterface(XAttr.class, xAttrBarNsSetN);
        assertNotNull("AttributesMap.setNamedItemNS", xAttrBarNsSet);
        assertEquals("AttributesMap.setNamedItemNS",
                xAttrBarNsSet, xAttributes.getNamedItemNS(ns, "bar"));
        assertSame("AttributesMap.getLength()", 2, xAttributes.getLength());
    }

    /*
    @Test public void testXNamedNodeMap_EntitiesMap() throws Exception
    {
        XNamedNodeMap xEntities = FIXME
    }
    */

    /*
    @Test public void testXNamedNodeMap_NotationsMap() throws Exception
    {
        XNamedNodeMap xNotations = FIXME
    }
    */

    @Test public void testXXPathAPI() throws Exception
    {
        XXPathAPI xXPathAPI =
            UnoRuntime.queryInterface(XXPathAPI.class,
            m_xMSF.createInstance("com.sun.star.xml.xpath.XPathAPI"));
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));

        String ns = "http://example.com/";

        XDocument xDoc = xBuilder.newDocument();

        XElement xRoot = xDoc.createElement("root");

        XElement xFoo1 = xDoc.createElement("foo");
        XElement xFoo2 = xDoc.createElement("foo");
        XElement xFooNs = xDoc.createElementNS(ns, "ns:foo");
        XElement xBar = xDoc.createElement("bar");

        xDoc.appendChild(xRoot);
        xRoot.appendChild(xFoo1);
        xFoo1.appendChild(xBar);
        xBar.appendChild(xFoo2);
        xRoot.appendChild(xFooNs);

        try {
            xXPathAPI.eval(xRoot, "~/-$+&#_");
            fail("XXPathAPI.eval");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.evalNS(xRoot, "~/-$+&#_", xRoot);
            fail("XXPathAPI.evalNS");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.selectNodeList(xRoot, "~/-$+&#_");
            fail("XXPathAPI.selectNodeList");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.selectNodeListNS(xRoot, "~/-$+&#_", xRoot);
            fail("XXPathAPI.selectNodeListNS");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.selectSingleNode(xRoot, "~/-$+&#_");
            fail("XXPathAPI.selectSingleNode");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.selectSingleNodeNS(xRoot, "~/-$+&#_", xRoot);
            fail("XXPathAPI.selectSingleNodeNS");
        } catch (XPathException e) { /* expected */ }
        try {
            xXPathAPI.eval(null, "child::foo");
            fail("XXPathAPI.eval(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xXPathAPI.evalNS(null, "child::foo", xRoot);
            fail("XXPathAPI.evalNS(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xXPathAPI.selectNodeList(null, "child::foo");
            fail("XXPathAPI.selectNodeList(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xXPathAPI.selectNodeListNS(null, "child::foo", xRoot);
            fail("XXPathAPI.selectNodeListNS(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xXPathAPI.selectSingleNode(null, "child::foo");
            fail("XXPathAPI.selectSingleNode(null)");
        } catch (Exception e) { /* expected */ }
        try {
            xXPathAPI.selectSingleNodeNS(null, "child::foo", xRoot);
            fail("XXPathAPI.selectSingleNodeNS(null)");
        } catch (Exception e) { /* expected */ }

        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "count(child::foo)");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathAPI.eval",
                    XPATH_NUMBER, xResult.getObjectType());
            assertEquals("XXPathAPI.eval", 1, xResult.getLong());
        }
        {
            XXPathObject xResult =
                xXPathAPI.evalNS(xRoot, "count(//ns:foo)", xFooNs);
            assertNotNull("XXPathAPI.evalNS", xResult);
            assertEquals("XXPathAPI.evalNS",
                    XPATH_NUMBER, xResult.getObjectType());
            assertEquals("XXPathAPI.evalNS", 1, xResult.getLong());
        }
        {
            XNodeList xResult = xXPathAPI.selectNodeList(xRoot, "child::foo");
            assertNotNull("XXPathAPI.selectNodeList", xResult);
            assertEquals("XXPathAPI.selectNodeList", 1, xResult.getLength());
            assertEquals("XXPathAPI.selectNodeList", xFoo1, xResult.item(0));
        }
        {
            XNodeList xResult =
                xXPathAPI.selectNodeListNS(xRoot, ".//ns:foo", xFooNs);
            assertNotNull("XXPathAPI.selectNodeListNS", xResult);
            assertEquals("XXPathAPI.selectNodeListNS", 1, xResult.getLength());
            assertEquals("XXPathAPI.selectNodeListNS", xFooNs, xResult.item(0));
        }
        {
            XNode xResult = xXPathAPI.selectSingleNode(xBar, "child::foo");
            assertNotNull("XXPathAPI.selectSingleNode", xResult);
            assertEquals("XXPathAPI.selectSingleNode", xFoo2, xResult);
        }
        {
            XNode xResult =
                xXPathAPI.selectSingleNodeNS(xFoo2, "//ns:foo", xFooNs);
            assertNotNull("XXPathAPI.selectSingleNodeNS", xResult);
            assertEquals("XXPathAPI.selectSingleNodeNS", xFooNs, xResult);
        }

        try {
            XNode xResult = xXPathAPI.selectSingleNode(xDoc, "//pre:foo");
            fail("XXPathAPI.selectSingleNode");
        } catch (XPathException e) { /* expected */ }
        xXPathAPI.registerNS("pre", ns);
        {
            XNode xResult = xXPathAPI.selectSingleNode(xDoc, "//pre:foo");
            assertNotNull("XXPathAPI.registerNS", xResult);
            assertEquals("XXPathAPI.registerNS", xFooNs, xResult);
        }

        xXPathAPI.unregisterNS("pre", ns);
        try {
            XNode xResult = xXPathAPI.selectSingleNode(xDoc, "//pre:foo");
            fail("XXPathAPI.unregisterNS");
        } catch (XPathException e) { /* expected */ }

        /* FIXME
        registerExtension("");
        registerExtensionInstance(xExtension);
        */
    }

    @Test public void testXXPathObject() throws Exception
    {
        XXPathAPI xXPathAPI =
            UnoRuntime.queryInterface(XXPathAPI.class,
            m_xMSF.createInstance("com.sun.star.xml.xpath.XPathAPI"));
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));

        String ns = "http://example.com/";

        XDocument xDoc = xBuilder.newDocument();

        XElement xRoot = xDoc.createElement("root");

        XElement xFoo1 = xDoc.createElement("foo");
        XElement xFoo2 = xDoc.createElement("foo");
        XElement xFooNs = xDoc.createElementNS(ns, "ns:foo");
        XElement xBar = xDoc.createElement("bar");

        xDoc.appendChild(xRoot);
        xRoot.appendChild(xFoo1);
        xFoo1.appendChild(xBar);
        xBar.appendChild(xFoo2);
        xRoot.appendChild(xFooNs);

        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "count(//foo)");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_NUMBER, xResult.getObjectType());
            assertEquals("XXPathObject.getByte", 2, xResult.getByte());
            assertEquals("XXPathObject.getShort", 2, xResult.getShort());
            assertEquals("XXPathObject.getLong", 2, xResult.getLong());
            assertEquals("XXPathObject.getHyper", 2, xResult.getHyper());
            assertEquals("XXPathObject.getFloat", 2.0, xResult.getFloat(), 0.0);
            assertEquals("XXPathObject.getDouble",
                    2.0, xResult.getDouble(), 0.0);
            assertEquals("XXPathObject.getString", "2", xResult.getString());
        }
        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "count(//foo) = 2");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_BOOLEAN, xResult.getObjectType());
            assertEquals("XXPathObject.getBoolean", true, xResult.getBoolean());
            assertEquals("XXPathObject.getString", "true", xResult.getString());
        }
        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "count(//foo) = 2");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_BOOLEAN, xResult.getObjectType());
            assertEquals("XXPathObject.getBoolean", true, xResult.getBoolean());
            assertEquals("XXPathObject.getString", "true", xResult.getString());
        }
        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "local-name(foo)");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_STRING, xResult.getObjectType());
            assertEquals("XXPathObject.getString", "foo", xResult.getString());
        }
        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "//foo");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_NODESET, xResult.getObjectType());
            assertNotNull("XXPathObject.getNodeList", xResult.getNodeList());
        }
    }

    @Test public void testXNodeList_NodeList() throws Exception
    {
        XXPathAPI xXPathAPI =
            UnoRuntime.queryInterface(XXPathAPI.class,
            m_xMSF.createInstance("com.sun.star.xml.xpath.XPathAPI"));
        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));

        String ns = "http://example.com/";

        XDocument xDoc = xBuilder.newDocument();

        XElement xRoot = xDoc.createElement("root");

        XElement xFoo1 = xDoc.createElement("foo");
        XElement xFoo2 = xDoc.createElement("foo");
        XElement xFooNs = xDoc.createElementNS(ns, "ns:foo");
        XElement xBar = xDoc.createElement("bar");

        xDoc.appendChild(xRoot);
        xRoot.appendChild(xFoo1);
        xFoo1.appendChild(xBar);
        xBar.appendChild(xFoo2);
        xRoot.appendChild(xFooNs);

        {
            XXPathObject xResult = xXPathAPI.eval(xRoot, "//foo");
            assertNotNull("XXPathAPI.eval", xResult);
            assertEquals("XXPathObject.getObjectType",
                    XPATH_NODESET, xResult.getObjectType());
            XNodeList xNodeList = xResult.getNodeList();
            assertNotNull("XXPathObject.getNodeList", xNodeList);
            assertEquals("NodeList.getLength", 2, xNodeList.getLength());
            assertEquals("NodeList.item", xFoo1, xNodeList.item(0));
            assertEquals("NodeList.item", xFoo2, xNodeList.item(1));
        }
    }

    @Test public void testXSAXSerialize() throws Exception
    {
        String file =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
            "<office:document-content " +
            "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" " +
            "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " +
            "xmlns=\"\" " +
            "office:version=\"1.0\">" +
            "<office:scripts/>" +
            "<xlink:test/>" +
            "<office:automatic-styles teststyle=\"test\"/>" +
            "<moretest/>" +
            "some text \uC3B6\uC3A4\uC3BC" +
            "</office:document-content>";

        XDocumentBuilder xBuilder =
            UnoRuntime.queryInterface(XDocumentBuilder.class,
            m_xMSF.createInstance("com.sun.star.xml.dom.DocumentBuilder"));

        XInputStream xIn =
            SequenceInputStream.createStreamFromSequence(m_xContext, file.getBytes("UTF-8"));

        XDocument xDoc =
            xBuilder.parse(xIn);

        XDocumentHandler xHandler =
            UnoRuntime.queryInterface(XDocumentHandler.class, new DummyDocumentHandler());

        XSAXSerializable serializable =
            UnoRuntime.queryInterface(XSAXSerializable.class, xDoc);

        serializable.serialize(xHandler, new StringPair[0]);
    }

    class DummyDocumentHandler implements XDocumentHandler
    {
        public void startDocument() throws SAXException {}
        public void endDocument() throws SAXException {}
        public void startElement(String s, XAttributeList a) throws SAXException {}
        public void endElement(String s) throws SAXException {}
        public void characters(String s) throws SAXException {}
        public void ignorableWhitespace(String s) throws SAXException {}
        public void processingInstruction(String s1, String s2) throws SAXException {}
        public void setDocumentLocator(XLocator l) throws SAXException {}
    }

    // just for importNode...
    abstract class MockNode implements XNode
    {
        MockDoc  m_document;
        MockNode m_parent;
        MockNode m_prev;
        MockNode m_next;
        MockNode[] m_children;
        String m_localname;

//        MockNode() { ; }
        void init(MockDoc doc, MockNode parent, MockNode prev, MockNode next,
                MockNode[] children)
        {
            m_document = doc;
            m_parent = parent; m_prev = prev; m_next = next;
            m_children = children;
        }

        public XNode appendChild(XNode c) throws DOMException {
            fail("MockNode.appendChild called?");
            return null;
        }
        public XNode cloneNode(boolean b) {
            fail("MockNode.cloneNode called?");
            return null;
        }
        public XNamedNodeMap getAttributes() {
            fail("MockNode.getAttributes not implemented");
            return null;
        }
        public XNodeList getChildNodes() {
            fail("MockNode.getChildList not implemented");
            return null;
        }
        public XNode getFirstChild() {
            return (m_children.length != 0) ? m_children[0] : null;
        }
        public XNode getLastChild() {
            return (m_children.length != 0)
                ? m_children[m_children.length-1] : null;
        }
        public String getLocalName() { return m_localname; }
        public String getNamespaceURI() { return ""; }
        public XNode getNextSibling() { return m_next; }
        public String getNodeName() { return m_localname; }
//        NodeType getNodeType() { return m_type; }
        public String getNodeValue() throws DOMException { return ""; }
        public XDocument getOwnerDocument() { return m_document; }
        public XNode getParentNode() { return m_parent; }
        public String getPrefix() { return ""; }
        public XNode getPreviousSibling() { return m_prev; }
        public boolean hasAttributes() { return false; }
        public boolean hasChildNodes() { return m_children.length != 0; }
        public XNode insertBefore(XNode c, XNode r)  throws DOMException {
            fail("MockNode.insertBefore called?");
            return null;
        }
        public boolean isSupported(String a, String b) { return false; }
        public void normalize() {
            fail("MockNode.normalize called?");
        }
        public XNode removeChild(XNode c) throws DOMException {
            fail("MockNode.removeChild called?");
            return null;
        }
        public XNode replaceChild(XNode c, XNode o) throws DOMException {
            fail("MockNode.replaceChild called?");
            return null;
        }
        public void setNodeValue(String v) throws DOMException {
            fail("MockNode.setNodeValue called?");
        }
        public void setPrefix(String p) throws DOMException {
            fail("MockNode.setPrefix called?");
        }
    }
    class MockDoc extends MockNode implements XDocument
    {
//        MockDoc() { }
        void init(MockNode[] children) {
            super.init(this, null, null, null, children);
        }

        public NodeType getNodeType() { return DOCUMENT_NODE; }

        public XAttr createAttribute(String n) throws DOMException {
            fail("MockNode.createAttribute called?");
            return null;
        }
        public XAttr createAttributeNS(String n, String q) throws DOMException {
            fail("MockNode.createAttributeNS called?");
            return null;
        }
        public XCDATASection createCDATASection(String s) throws DOMException {
            fail("MockNode.createCDATASection called?");
            return null;
        }
        public XComment createComment(String s) {
            fail("MockNode.createCDATASection called?");
            return null;
        }
        public XDocumentFragment createDocumentFragment() {
            fail("MockNode.createDocumentFragment called?");
            return null;
        }
        public XElement createElement(String n) {
            fail("MockNode.createElement called?");
            return null;
        }
        public XElement createElementNS(String n, String q) {
            fail("MockNode.createElementNS called?");
            return null;
        }
        public XEntityReference createEntityReference(String n)
                throws DOMException {
            fail("MockNode.createEntityReference called?");
            return null;
        }
        public XProcessingInstruction createProcessingInstruction(String t,
                String d) throws DOMException {
            fail("MockNode.createEntityReference called?");
            return null;
        }
        public XText createTextNode(String d) {
            fail("MockNode.createTextNode called?");
            return null;
        }
        public XDocumentType getDoctype() {
            fail("MockNode.getDoctype called?");
            return null;
        }
        public XElement getDocumentElement() {
            fail("MockNode.getDocumentElement called?");
            return null;
        }
        public XElement getElementById(String id) {
            fail("MockNode.getElementById called?");
            return null;
        }
        public XNodeList getElementsByTagName(String n) {
            fail("MockNode.getElementsByTagName called?");
            return null;
        }
        public XNodeList getElementsByTagNameNS(String n, String q) {
            fail("MockNode.getElementsByTagNameNS called?");
            return null;
        }
        public XDOMImplementation getImplementation() {
            fail("MockNode.getImplementation called?");
            return null;
        }
        public XNode importNode(XNode i, boolean b) throws DOMException {
            fail("MockNode.importNode called?");
            return null;
        }
    }
    class MockNodeMap implements XNamedNodeMap
    {
        MockAttr[] m_attributes;

        MockNodeMap(MockAttr[] attrs) { m_attributes = attrs; }

        public int getLength() { return m_attributes.length; }
        public XNode getNamedItem(String name) {
            fail("MockNodeMap.getNamedItem not implemented");
            return null;
        }
        public XNode getNamedItemNS(String n, String l) {
            fail("MockNodeMap.getNamedItemNS not implemented");
            return null;
        }
        public XNode item(int index) {
            return m_attributes[index];
        }
        public XNode removeNamedItem(String n) throws DOMException {
            fail("MockNodeMap.removeNamedItem called?");
            return null;
        }
        public XNode removeNamedItemNS(String n, String l) throws DOMException {
            fail("MockNodeMap.removeNamedItemNS called?");
            return null;
        }
        public XNode setNamedItem(XNode n) throws DOMException {
            fail("MockNodeMap.setNamedItem called?");
            return null;
        }
        public XNode setNamedItemNS(XNode n) throws DOMException {
            fail("MockNodeMap.setNamedItemNS called?");
            return null;
        }
    }
    class MockElement extends MockNode implements XElement
    {
        MockAttr[] m_attributes;

        MockElement(String name, MockAttr[] attrs) {
            m_localname = name; m_attributes = attrs;
        }

        public NodeType getNodeType() { return ELEMENT_NODE; }
        public XNamedNodeMap getAttributes() {
            return new MockNodeMap(m_attributes);
        }
        public boolean hasAttributes() { return m_attributes.length != 0; }

        public String getAttribute(String n) {
            fail("MockNode.getAttribute not implemented");
            return null;
        }
        public XAttr getAttributeNode(String n) {
            fail("MockNode.getAttributeNode not implemented");
            return null;
        }
        public XAttr getAttributeNodeNS(String n, String l) {
            fail("MockNode.getAttributeNodeNS not implemented");
            return null;
        }
        public String getAttributeNS(String n, String q) {
            fail("MockNode.getAttributeNS not implemented");
            return null;
        }
        public XNodeList getElementsByTagName(String n) {
            fail("MockNode.getElementsByTagName called?");
            return null;
        }
        public XNodeList getElementsByTagNameNS(String n, String l) {
            fail("MockNode.getElementsByTagNameNS called?");
            return null;
        }
        public String getTagName() {
            return getLocalName();
        }
        public boolean hasAttribute(String n) {
            fail("MockNode.hasAttribute not implemented");
            return false;
        }
        public boolean hasAttributeNS(String n, String l) {
            fail("MockNode.hasAttributeNS not implemented");
            return false;
        }
        public void removeAttribute(String n) throws DOMException {
            fail("MockNode.removeAttribute called?");
        }
        public XAttr removeAttributeNode(XAttr o) throws DOMException {
            fail("MockNode.removeAttributeNode called?");
            return null;
        }
        public void removeAttributeNS(String n, String l) throws DOMException {
            fail("MockNode.removeAttributeNS called?");
        }
        public void setAttribute(String n, String v) throws DOMException {
            fail("MockNode.setAttribute called?");
        }
        public XAttr setAttributeNode(XAttr n) throws DOMException {
            fail("MockNode.setAttributeNode called?");
            return null;
        }
        public XAttr setAttributeNodeNS(XAttr n) throws DOMException {
            fail("MockNode.setAttributeNodeNS called?");
            return null;
        }
        public void setAttributeNS(String n, String q, String v)
                throws DOMException {
            fail("MockNode.setAttributeNS called?");
        }
    }
    class MockAttr extends MockNode implements XAttr
    {
        String m_value;

        MockAttr(String name, String value) {
            m_localname = name; m_value = value;
        }

        public NodeType getNodeType() { return ATTRIBUTE_NODE; }

        public String getName() { return m_localname; }
        public XElement getOwnerElement() { return (XElement) m_parent; }
        public boolean getSpecified() { return true; }
        public String getValue() { return m_value; }
        public void setValue(String v) {
            fail("MockNode.setValue called?");
        }
    }
}


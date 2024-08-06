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

package org.openoffice.xmerge.converter.xml.xslt;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.*;

import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.dom.DOMDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

// Imported TraX classes
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.TransformerException;
import javax.xml.transform.URIResolver;
import javax.xml.transform.Source;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import javax.xml.XMLConstants;

/**
 * Xslt implementation of {@code org.openoffice.xmerge.DocumentSerializer}
 * for the {@link org.openoffice.xmerge.converter.xml.xslt.PluginFactoryImpl
 * PluginFactoryImpl}.
 *
 * <p>The {@code serialize} method transforms the DOM document from the given
 * {@code Document} object by means of a supplied Xsl Stylesheet.</p>
 */

public final class DocumentSerializerImpl
    implements DocumentSerializer,OfficeConstants,URIResolver {

    /** SXW {@code Document} object that this converter processes. */
    private final GenericOfficeDocument sxwDoc;

    private final PluginFactoryImpl pluginFactory;

    private static DocumentBuilderFactory makeFactory() {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

        try {
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        try {
            factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        } catch (ParserConfigurationException e) {
            Debug.log(Debug.ERROR, "Exception when calling setFeature: ", e);
        }

        return factory;
    }

    /**
     * Constructor.
     *
     * @param  pf   A {@code PluginFactoryImpl}.
     * @param  doc  A SXW {@code Document} to be converted.
     */
    public DocumentSerializerImpl(PluginFactoryImpl pf,Document doc) {
        pluginFactory=pf;
        sxwDoc = (GenericOfficeDocument) doc;
    }

    /**
     * Method to convert a {@code Document} with an xsl stylesheet.
     *
     * <p>It creates a {@code Document} object, which is then transformed with the
     * Xslt processor. A {@code ConvertData} object is constructed and returned.</p>
     *
     * @return  A {@code ConvertData} object.
     *
     * @throws  ConvertException  If any I/O error occurs.
     * @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws ConvertException, IOException {
        String docName = sxwDoc.getName();
        org.w3c.dom.Document domDoc = sxwDoc.getContentDOM();
        org.w3c.dom.Document metaDoc = sxwDoc.getMetaDOM();
        org.w3c.dom.Document styleDoc = sxwDoc.getStyleDOM();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ConvertData cd = new ConvertData();
        Node offnode = domDoc.getDocumentElement();
        if (!(offnode.getNodeName()).equals("office:document")) {
            try {
                DocumentBuilderFactory builderFactory = makeFactory();
                DocumentBuilder builder = builderFactory.newDocumentBuilder();
                DOMImplementation domImpl = builder.getDOMImplementation();
                DocumentType docType = domImpl.createDocumentType(
                        "office:document",
                        "-//OpenOffice.org//DTD OfficeDocument 1.0//EN", null);
                org.w3c.dom.Document newDoc = domImpl.createDocument(
                        "http://openoffice.org/2000/office", "office:document",
                        docType);

                Element rootElement = newDoc.getDocumentElement();
                rootElement.setAttribute("xmlns:office",
                        "http://openoffice.org/2000/office");
                rootElement.setAttribute("xmlns:style",
                        "http://openoffice.org/2000/style");
                rootElement.setAttribute("xmlns:text",
                        "http://openoffice.org/2000/text");
                rootElement.setAttribute("xmlns:table",
                        "http://openoffice.org/2000/table");

                rootElement.setAttribute("xmlns:draw",
                        "http://openoffice.org/2000/drawing");
                rootElement.setAttribute("xmlns:fo",
                        "http://www.w3.org/1999/XSL/Format");
                rootElement.setAttribute("xmlns:xlink",
                        "http://www.w3.org/1999/xlink");
                rootElement.setAttribute("xmlns:dc",
                        "http://purl.org/dc/elements/1.1/");
                rootElement.setAttribute("xmlns:meta",
                        "http://openoffice.org/2000/meta");
                rootElement.setAttribute("xmlns:number",
                        "http://openoffice.org/2000/datastyle");
                rootElement.setAttribute("xmlns:svg",
                        "http://www.w3.org/2000/svg");
                rootElement.setAttribute("xmlns:chart",
                        "http://openoffice.org/2000/chart");
                rootElement.setAttribute("xmlns:dr3d",
                        "http://openoffice.org/2000/dr3d");
                rootElement.setAttribute("xmlns:math",
                        "http://www.w3.org/1998/Math/MathML");
                rootElement.setAttribute("xmlns:form",
                        "http://openoffice.org/2000/form");
                rootElement.setAttribute("xmlns:script",
                        "http://openoffice.org/2000/script");
                rootElement.setAttribute("xmlns:config",
                        "http://openoffice.org/2001/config");
                rootElement.setAttribute("office:class", "text");
                rootElement.setAttribute("office:version", "1.0");

                NodeList nodeList;
                Node tmpNode;
                Node rootNode = rootElement;
                if (metaDoc != null) {
                    nodeList = metaDoc.getElementsByTagName(TAG_OFFICE_META);
                    if (nodeList.getLength() > 0) {
                        tmpNode = newDoc.importNode(nodeList.item(0), true);
                        rootNode.appendChild(tmpNode);
                    }
                }
                if (styleDoc != null) {
                    nodeList = styleDoc.getElementsByTagName(TAG_OFFICE_STYLES);
                    if (nodeList.getLength() > 0) {
                        tmpNode = newDoc.importNode(nodeList.item(0), true);
                        rootNode.appendChild(tmpNode);
                    }
                }
                nodeList = domDoc
                        .getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
                if (nodeList.getLength() > 0) {
                    tmpNode = newDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }
                nodeList = domDoc.getElementsByTagName(TAG_OFFICE_BODY);
                if (nodeList.getLength() > 0) {
                    tmpNode = newDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }
                domDoc = newDoc;
            } catch (Exception e) {
                System.out
                        .println("\nAn Exception occurred with Xslt Serializer"
                                + e);
            }

        }

        try {
            baos = transform(domDoc);
        } catch (Exception e) {
            System.out.println("\n Error with Xslt\n");
        }

        DOMDocument resultDomDoc = (DOMDocument) pluginFactory
                .createDeviceDocument(docName,
                        new ByteArrayInputStream(baos.toByteArray()));
        cd.addDocument(resultDomDoc);
        return cd;
    }

    public Source resolve(String href, String base)
            throws TransformerException {
        if (href != null) {
            if (href.equals("javax.xml.transform.dom.DOMSource") || href.length() == 0) {
                return null;
            }
            try {
                ConverterInfo ci = pluginFactory.getConverterInfo();
                String newhRef = "jar:" + ci.getJarName() + "!/" + href;
                StreamSource sheetFile = new StreamSource(newhRef);
                return sheetFile;
            } catch (Exception e) {
                System.out.println("\nException in Xslt Resolver " + e);
                return null;
            }
        } else {
            return null;
        }
    }

    /**
     * This method performs the xsl transformation on the supplied
     * {@code Document} and returns a {@code DOMResult} object.
     *
     * <p>Xslt transformation code.</p>
     *
     * @return A {@code ByteArrayOutputStream} object containing the result
     *         of the Xslt transformation.
     */
    private ByteArrayOutputStream transform(org.w3c.dom.Document domDoc) {
        ConverterInfo ci = pluginFactory.getConverterInfo();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {

            DocumentBuilderFactory dFactory = makeFactory();
            dFactory.setNamespaceAware(true);

            DocumentBuilder dBuilder = dFactory.newDocumentBuilder();
            String teststr = ci.getXsltSerial();

            teststr = teststr.substring(0, 6);
            org.w3c.dom.Document xslDoc = null;
            if ((teststr.equals("http:/")) || (teststr.equals("file:/"))
                    || (teststr.equals("jar://"))) {
                System.out.println(ci.getXsltSerial());
                xslDoc = dBuilder.parse(ci.getXsltSerial());

            } else {
                xslDoc = dBuilder.parse(
                        "jar:" + ci.getJarName() + "!/" + ci.getXsltSerial());
            }

            DOMSource xslDomSource = new DOMSource(xslDoc);
            DOMSource xmlDomSource = new DOMSource(domDoc);

            //call the transformer using the XSL, Source and Result.
            TransformerFactory tFactory = TransformerFactory.newInstance();
            tFactory.setURIResolver(this);
            Transformer transformer = tFactory.newTransformer(xslDomSource);

            transformer.transform(xmlDomSource, new StreamResult(baos));
        } catch (Exception e) {
            System.out.println("An error occurred in the transformation : " + e);
        }
        return baos;
    }
}

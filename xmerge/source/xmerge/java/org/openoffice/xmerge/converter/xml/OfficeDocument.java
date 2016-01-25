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

package org.openoffice.xmerge.converter.xml;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.InputStreamReader;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.Document;
import org.w3c.dom.DOMImplementation;
import org.w3c.dom.DocumentType;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.w3c.dom.NamedNodeMap;
import org.xml.sax.SAXException;

import javax.xml.transform.*;
import javax.xml.transform.dom.*;
import javax.xml.transform.stream.*;

import org.openoffice.xmerge.util.Debug;

/**
 *  An implementation of {@code Document} for StarOffice documents.
 */
public abstract class OfficeDocument
    implements org.openoffice.xmerge.Document, OfficeConstants {

    /** Factory for {@code DocumentBuilder} objects. */
    private static DocumentBuilderFactory factory =
       DocumentBuilderFactory.newInstance();

    /** DOM {@code Document} of content.xml. */
    private Document contentDoc = null;

   /** DOM {@code Document} of meta.xml. */
    private Document metaDoc = null;

   /** DOM {@code Document} of settings.xml. */
    private Document settingsDoc = null;

    /** DOM {@code Document} of content.xml. */
    private Document styleDoc = null;

    /** DOM {@code Document} of @code META-INF/manifest.xml. */
    private Document manifestDoc = null;

    private String documentName = null;
    private String fileName = null;

    /**
     * {@code OfficeZip} object to store zip contents from read
     * {@code InputStream}.
     *
     * <p>Note that this member will still be null if it was initialized using
     * a template file instead of reading from a StarOffice zipped XML file.</p>
     */
    private OfficeZip zip = null;

    /** Collection to keep track of the embedded objects in the document. */
    private Map<String, EmbeddedObject> embeddedObjects = null;

    /**
     * Default constructor.
     *
     * @param  name  {@code Document} name.
     */
    public OfficeDocument(String name) {
        this(name, true, false);
    }

    /**
     * Constructor with arguments to set {@code namespaceAware} and
     * {@code validating} flags.
     *
     * @param  name            {@code Document} name (may or may not contain
     *                         extension).
     * @param  namespaceAware  Value for {@code namespaceAware} flag.
     * @param  validating      Value for {@code validating} flag.
     */
    public OfficeDocument(String name, boolean namespaceAware, boolean validating) {
        factory.setValidating(validating);
        factory.setNamespaceAware(namespaceAware);
        this.documentName = trimDocumentName(name);
        this.fileName = documentName + getFileExtension();
    }

    /**
     * Removes the file extension from the {@code Document} name.
     *
     * @param   name  Full {@code Document} name with extension.
     *
     * @return  Name of {@code Document} without the extension.
     */
    private String trimDocumentName(String name) {
        String temp = name.toLowerCase();
        String ext = getFileExtension();

        if (temp.endsWith(ext)) {
            // strip the extension
            int nlen = name.length();
            int endIndex = nlen - ext.length();
            name = name.substring(0,endIndex);
        }

        return name;
    }

    /**
     * Return a DOM {@code Document} object of the content.xml file.
     *
     * <p>Note that a content DOM is not created when the constructor is called.
     * So, either the {@code read} method or the {@code initContentDOM} method
     * will need to be called ahead on this object before calling this method.</p>
     *
     * @return  DOM {@code Document} object.
     */
    public Document getContentDOM() {

        return contentDoc;
    }

    /**
     * Return a DOM {@code Document} object of the meta.xml file.
     *
     * <p>Note that a content DOM is not created when the constructor is called.
     * So, either the {@code read} method or the {@code initContentDOM} method
     * will need to be called ahead on this object before calling this method.</p>
     *
     * @return  DOM <code>Document</code> object.
     */
    public Document getMetaDOM() {

        return metaDoc;
    }

    /**
     * Return a DOM {@code Document} object of the settings.xml file.
     *
     * <p>Note that a content DOM is not created when the constructor is called.
     * So, either the {@code read} method or the {@code initContentDOM} method
     * will need to be called ahead on this object before calling this method.</p>
     *
     * @return  DOM {@code Document} object.
     */
    public Document getSettingsDOM() {

        return settingsDoc;
    }

    /**
     * Sets the content tree of the document.
     *
     * @param   newDom  {@code Node} containing the new content tree.
     */
    public void setContentDOM( Node newDom) {
        contentDoc = (Document)newDom;
    }

    /**
     * Sets the meta tree of the document.
     *
     * @param   newDom  {@code Node} containing the new meta tree.
     */
    public void setMetaDOM (Node newDom) {
        metaDoc = (Document)newDom;
    }

    /**
     * Sets the settings tree of the document.
     *
     * @param   newDom  {@code Node} containing the new settings tree.
     */
    public void setSettingsDOM (Node newDom) {
        settingsDoc = (Document)newDom;
    }

    /**
     * Sets the style tree of the document.
     *
     * @param   newDom  {@code Node} containing the new style tree.
     */
    public void setStyleDOM (Node newDom) {
        styleDoc = (Document)newDom;
    }

    /**
     * Return a DOM {@code Document} object of the style.xml file.
     *
     * <p>Note that this may return {@code null} if there is no style DOM.</p>
     * <p>Note that a style DOM is not created when the constructor is called.
     * Depending on the {@code InputStream}, a {@code read} method may or may
     * not build a style DOM.  When creating a new style DOM, call the
     * {@code initStyleDOM} method first.</p>
     *
     * @return  DOM {@code Document} object.
     */
    public Document getStyleDOM() {

        return styleDoc;
    }

    /**
     * Return the name of the {@code Document}.
     *
     * @return  The name of {@code Document}.
     */
    public String getName() {

        return documentName;
    }

    /**
     * Return the file name of the {@code Document}, possibly with the standard
     * extension.
     *
     * @return  The file name of {@code Document}.
     */
    public String getFileName() {

        return fileName;
    }

    /**
     * Returns the file extension for this type of {@code Document}.
     *
     * @return  The file extension of {@code Document}.
     */
    protected abstract String getFileExtension();

    /**
     * Returns all the embedded objects (graphics, formulae, etc.) present in
     * this document.
     *
     * @return An {@code Iterator} of {@code EmbeddedObject} objects.
     */
    private Iterator<EmbeddedObject> getEmbeddedObjects() {

        if (embeddedObjects == null && manifestDoc != null) {
            embeddedObjects = new HashMap<String, EmbeddedObject>();

            // Need to read the manifest file and construct a list of objects
            NodeList nl = manifestDoc.getElementsByTagName(TAG_MANIFEST_FILE);

            // Don't create the HashMap if there are no embedded objects
            int len = nl.getLength();
            for (int i = 0; i < len; i++) {
                Node n = nl.item(i);

                NamedNodeMap attrs = n.getAttributes();

                String type = attrs.getNamedItem(ATTRIBUTE_MANIFEST_FILE_TYPE).getNodeValue();
                String path = attrs.getNamedItem(ATTRIBUTE_MANIFEST_FILE_PATH).getNodeValue();

                /*
                 * According to OpenOffice.org XML File Format document (ver. 1)
                 * there are only two types of embedded object:
                 *
                 *      Objects with an XML representation.
                 *      Objects without an XML representation.
                 *
                 * The former are represented by one or more XML files.
                 * The latter are in binary form.
                 */
                if (type.startsWith("application/vnd.sun.xml"))
                {
                    if (path.equals("/")) {
                        // Exclude the main document entries
                        continue;
                    }
                    // Take off the trailing '/'
                    String name = path.substring(0, path.length() - 1);
                    embeddedObjects.put(name, new EmbeddedXMLObject(name, type, zip));
                }
                else if (type.equals("text/xml")) {
                    // XML entries are either embedded StarOffice doc entries or main
                    // document entries
                    continue;
                }
                else { // FIX (HJ): allows empty MIME type
                    embeddedObjects.put(path, new EmbeddedBinaryObject(path, type, zip));
                }
            }
        }

        return embeddedObjects.values().iterator();
    }

    /**
     * Read the Office {@code Document} from the given {@code InputStream}.
     *
     * @param  is  Office document {@code InputStream}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream is) throws IOException {

        Debug.log(Debug.INFO, "reading Office file");
        DocumentBuilder builder = null;

        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

        // read in Office zip file format
        zip = new OfficeZip();
        zip.read(is);

        // grab the content.xml and
        // parse it into contentDoc.
        byte contentBytes[] = zip.getContentXMLBytes();
        if (contentBytes == null) {
            throw new OfficeDocumentException("Entry content.xml not found in file");
        }
        try {
            contentDoc = parse(builder, contentBytes);
        } catch (SAXException ex) {
            throw new OfficeDocumentException(ex);
        }

        // if style.xml exists, grab the style.xml
        // parse it into styleDoc.
        byte styleBytes[] = zip.getStyleXMLBytes();
        if (styleBytes != null) {
            try {
                styleDoc = parse(builder, styleBytes);
            } catch (SAXException ex) {
                throw new OfficeDocumentException(ex);
            }
        }

        byte metaBytes[] = zip.getMetaXMLBytes();
        if (metaBytes != null) {
            try {
                metaDoc = parse(builder, metaBytes);
            } catch (SAXException ex) {
                throw new OfficeDocumentException(ex);
            }
        }

        byte settingsBytes[] = zip.getSettingsXMLBytes();
        if (settingsBytes != null) {
            try {
                settingsDoc = parse(builder, settingsBytes);

            } catch (SAXException ex) {
                throw new OfficeDocumentException(ex);
            }
        }

        // Read in the META-INF/manifest.xml file
        byte manifestBytes[] = zip.getManifestXMLBytes();
        if (manifestBytes != null) {
            try {
                manifestDoc = parse(builder, manifestBytes);
            } catch (SAXException ex) {
                throw new OfficeDocumentException(ex);
            }
        }
    }

    /**
     * Read the Office {@code Document} from the given {@code InputStream}.
     *
     * @param   is    Office document {@code InputStream}.
     * @param   isZip {@code boolean} Identifies whether a file is zipped or not.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream is, boolean isZip) throws IOException {

        Debug.log(Debug.INFO, "reading Office file");

        DocumentBuilder builder = null;

        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

        if (isZip) {
            read(is);
        } else {
            try {
                Reader r = secondHack(is);
                InputSource ins = new InputSource(r);
                org.w3c.dom.Document newDoc = builder.parse(ins);
                Element rootElement = newDoc.getDocumentElement();

                NodeList nodeList;
                Node tmpNode;
                Node rootNode = rootElement;

                /* content */
                contentDoc = createDOM(TAG_OFFICE_DOCUMENT_CONTENT);
                rootElement = contentDoc.getDocumentElement();
                rootNode = rootElement;

                // FIX (HJ): Include office:font-decls in content DOM
                nodeList = newDoc
                        .getElementsByTagName(TAG_OFFICE_FONT_DECLS);
                if (nodeList.getLength() > 0) {
                    tmpNode = contentDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                nodeList = newDoc
                        .getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
                if (nodeList.getLength() > 0) {
                    tmpNode = contentDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                nodeList = newDoc.getElementsByTagName(TAG_OFFICE_BODY);
                if (nodeList.getLength() > 0) {
                    tmpNode = contentDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                /* Styles */
                styleDoc = createDOM(TAG_OFFICE_DOCUMENT_STYLES);
                rootElement = styleDoc.getDocumentElement();
                rootNode = rootElement;

                // FIX (HJ): Include office:font-decls in styles DOM
                nodeList = newDoc
                        .getElementsByTagName(TAG_OFFICE_FONT_DECLS);
                if (nodeList.getLength() > 0) {
                    tmpNode = styleDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                nodeList = newDoc.getElementsByTagName(TAG_OFFICE_STYLES);
                if (nodeList.getLength() > 0) {
                    tmpNode = styleDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                // FIX (HJ): Include office:automatic-styles in styles DOM
                nodeList = newDoc
                        .getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
                if (nodeList.getLength() > 0) {
                    tmpNode = styleDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                // FIX (HJ): Include office:master-styles in styles DOM
                nodeList = newDoc
                        .getElementsByTagName(TAG_OFFICE_MASTER_STYLES);
                if (nodeList.getLength() > 0) {
                    tmpNode = styleDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }

                /* Settings */
                settingsDoc = createDOM(TAG_OFFICE_DOCUMENT_SETTINGS);
                rootElement = settingsDoc.getDocumentElement();
                rootNode = rootElement;
                nodeList = newDoc.getElementsByTagName(TAG_OFFICE_SETTINGS);
                if (nodeList.getLength() > 0) {
                    tmpNode = settingsDoc
                            .importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }
                /* Meta */
                metaDoc = createDOM(TAG_OFFICE_DOCUMENT_META);
                rootElement = metaDoc.getDocumentElement();
                rootNode = rootElement;
                nodeList = newDoc.getElementsByTagName(TAG_OFFICE_META);
                if (nodeList.getLength() > 0) {
                    tmpNode = metaDoc.importNode(nodeList.item(0), true);
                    rootNode.appendChild(tmpNode);
                }
            } catch (SAXException ex) {
                throw new OfficeDocumentException(ex);
            }
        }

    }

    /**
     * Parse given {@code byte} array into a DOM {@code Document} object using
     * the {@code DocumentBuilder} object.
     *
     * @param   builder  {@code DocumentBuilder} object for parsing.
     * @param   bytes    {@code byte} array for parsing.
     *
     * @return  Resulting DOM {@code Document} object.
     *
     * @throws  SAXException  If any parsing error occurs.
     */
    static Document parse(DocumentBuilder builder, byte bytes[])
        throws SAXException, IOException {

        Document doc = null;

        ByteArrayInputStream is = new ByteArrayInputStream(bytes);

        // TODO:  replace hack with a more appropriate fix.

        Reader r = hack(is);
        InputSource ins = new InputSource(r);
        doc = builder.parse(ins);

        return doc;
    }

    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    protected abstract String getDocumentMimeType();

    /**
     * Write out Office ZIP file format.
     *
     * @param   os  XML {@code OutputStream}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        if (zip == null) {
            zip = new OfficeZip();
        }

        initManifestDOM();

        Element domEntry;
        Element manifestRoot = manifestDoc.getDocumentElement();

        // The EmbeddedObjects come first.
        Iterator<EmbeddedObject> embObjs = getEmbeddedObjects();
        while (embObjs.hasNext()) {
            EmbeddedObject obj = embObjs.next();
            obj.writeManifestData(manifestDoc);

            obj.write(zip);
        }

        // Add in the entry for the Pictures directory.  Always present.
        domEntry = manifestDoc.createElement(TAG_MANIFEST_FILE);
        domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "Pictures/");
        domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, "");
        manifestRoot.appendChild(domEntry);

        // Write content to the Zip file and then write any of the optional
        // data, if it exists.
        zip.setContentXMLBytes(docToBytes(contentDoc));

        domEntry = manifestDoc.createElement(TAG_MANIFEST_FILE);
        domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "content.xml");
        domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, "text/xml");

        manifestRoot.appendChild(domEntry);

        if (styleDoc != null) {
            zip.setStyleXMLBytes(docToBytes(styleDoc));

            domEntry = manifestDoc.createElement(TAG_MANIFEST_FILE);
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "styles.xml");
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, "text/xml");
            manifestRoot.appendChild(domEntry);
        }

        if (metaDoc != null) {
            zip.setMetaXMLBytes(docToBytes(metaDoc));

            domEntry = manifestDoc.createElement(TAG_MANIFEST_FILE);
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "meta.xml");
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, "text/xml");
            manifestRoot.appendChild(domEntry);
        }

        if (settingsDoc != null) {
            zip.setSettingsXMLBytes(docToBytes(settingsDoc));

            domEntry = manifestDoc.createElement(TAG_MANIFEST_FILE);
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "settings.xml");
            domEntry.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, "text/xml");
            manifestRoot.appendChild(domEntry);
        }

        zip.setManifestXMLBytes(docToBytes(manifestDoc));

        zip.write(os);
    }

    /**
     * Write out Office ZIP file format.
     *
     * @param   os    XML {@code OutputStream}.
     * @param   isZip {@code boolean}
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os, boolean isZip) throws IOException {

        // Create an OfficeZip object if one does not exist.
        if (isZip){
            write(os);
        } else {
        try {
        DocumentBuilderFactory builderFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder= builderFactory.newDocumentBuilder();
        DOMImplementation domImpl = builder.getDOMImplementation();
        domImpl.createDocumentType("office:document","-//OpenOffice.org//DTD OfficeDocument 1.0//EN",null);
        org.w3c.dom.Document newDoc = domImpl.createDocument("http://openoffice.org/2000/office","office:document",null);

        Element rootElement=newDoc.getDocumentElement();
        rootElement.setAttribute("xmlns:office","http://openoffice.org/2000/office");
        rootElement.setAttribute("xmlns:style","http://openoffice.org/2000/style" );
        rootElement.setAttribute("xmlns:text","http://openoffice.org/2000/text");
        rootElement.setAttribute("xmlns:table","http://openoffice.org/2000/table");

        rootElement.setAttribute("xmlns:draw","http://openoffice.org/2000/drawing");
        rootElement.setAttribute("xmlns:fo","http://www.w3.org/1999/XSL/Format" );
        rootElement.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink" );
        rootElement.setAttribute("xmlns:dc","http://purl.org/dc/elements/1.1/" );
        rootElement.setAttribute("xmlns:meta","http://openoffice.org/2000/meta" );
        rootElement.setAttribute("xmlns:number","http://openoffice.org/2000/datastyle" );
        rootElement.setAttribute("xmlns:svg","http://www.w3.org/2000/svg" );
        rootElement.setAttribute("xmlns:chart","http://openoffice.org/2000/chart" );
        rootElement.setAttribute("xmlns:dr3d","http://openoffice.org/2000/dr3d" );
        rootElement.setAttribute("xmlns:math","http://www.w3.org/1998/Math/MathML" );
        rootElement.setAttribute("xmlns:form","http://openoffice.org/2000/form" );
        rootElement.setAttribute("xmlns:script","http://openoffice.org/2000/script" );
        rootElement.setAttribute("xmlns:config","http://openoffice.org/2001/config" );
        // #i41033# OASIS format needs the "office:class" set.
        if(getDocumentMimeType().equals(SXC_MIME_TYPE))
            rootElement.setAttribute("office:class","spreadsheet" );
        else if(getDocumentMimeType().equals(SXW_MIME_TYPE))
            rootElement.setAttribute("office:class","text" );
        rootElement.setAttribute("office:version","1.0");

        NodeList nodeList;
        Node tmpNode;
        Node rootNode = rootElement;
        if (metaDoc !=null) {
            nodeList= metaDoc.getElementsByTagName(TAG_OFFICE_META);
            if (nodeList.getLength()>0) {
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
            }
        } if (styleDoc !=null) {
            nodeList= styleDoc.getElementsByTagName(TAG_OFFICE_STYLES);
            if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
            }
        } if (settingsDoc !=null) {
            nodeList= settingsDoc.getElementsByTagName(TAG_OFFICE_SETTINGS);
            if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
            }
        } if (contentDoc !=null) {
            nodeList= contentDoc.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
            if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
            }
            nodeList= contentDoc.getElementsByTagName(TAG_OFFICE_BODY);
            if (nodeList.getLength()>0){
            tmpNode = newDoc.importNode(nodeList.item(0),true);
            rootNode.appendChild(tmpNode);
            }
        }

        byte contentBytes[] = docToBytes(newDoc);
        os.write(contentBytes);
        } catch(Exception exc){
            System.out.println("\nException in OfficeDocument.write():" +exc);
        }
        }
    }


    /**
     * Write out a {@code org.w3c.dom.Document} object into a {@code byte}
     * array.
     *
     * <p>TODO: remove dependency on {@code com.sun.xml.tree.XmlDocument}
     * package!</p>
     *
     * @param   doc  DOM {@code Document} object.
     *
     * @return  {@code byte} array of DOM {@code Document} object.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    static byte[] docToBytes(Document doc)
        throws IOException {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        java.lang.reflect.Constructor<?> con;
        java.lang.reflect.Method meth;

        String domImpl = doc.getClass().getName();

        /*
         * We may have multiple XML parsers in the Classpath.
         * Depending on which one is first, the actual type of
         * doc may vary.  Need a way to find out which API is being
         * used and use an appropriate serialization method.
         */

        try {
            // First of all try for JAXP 1.0
            if (domImpl.equals("com.sun.xml.tree.XmlDocument")) {

                Debug.log(Debug.INFO, "Using JAXP");

                Class<?> jaxpDoc = Class.forName("com.sun.xml.tree.XmlDocument");

                // The method is in the XMLDocument class itself, not a helper
                meth = jaxpDoc.getMethod("write",
                            new Class[] { Class.forName("java.io.OutputStream") } );

                meth.invoke(doc, new Object [] { baos } );
            } else if (domImpl.equals("org.apache.crimson.tree.XmlDocument")) {
                Debug.log(Debug.INFO, "Using Crimson");

                Class<?> crimsonDoc = Class.forName("org.apache.crimson.tree.XmlDocument");
                // The method is in the XMLDocument class itself, not a helper
                meth = crimsonDoc.getMethod("write",
                            new Class[] { Class.forName("java.io.OutputStream") } );

                meth.invoke(doc, new Object [] { baos } );
            } else if (domImpl.equals("org.apache.xerces.dom.DocumentImpl")
                    || domImpl.equals("org.apache.xerces.dom.DeferredDocumentImpl")) {

                Debug.log(Debug.INFO, "Using Xerces");

                // Try for Xerces
                Class<?> xercesSer =
                        Class.forName("org.apache.xml.serialize.XMLSerializer");

                // Get the OutputStream constructor
                // May want to use the OutputFormat parameter at some stage too
                con = xercesSer.getConstructor(new Class []
                        { Class.forName("java.io.OutputStream"),
                          Class.forName("org.apache.xml.serialize.OutputFormat") } );

                // Get the serialize method
                meth = xercesSer.getMethod("serialize",
                            new Class [] { Class.forName("org.w3c.dom.Document") } );

                // Get an instance
                Object serializer = con.newInstance(new Object [] { baos, null } );

                // Now call serialize to write the document
                meth.invoke(serializer, new Object [] { doc } );
            } else if (domImpl.equals("gnu.xml.dom.DomDocument")) {
                Debug.log(Debug.INFO, "Using GNU");

                Class<?> gnuSer = Class.forName("gnu.xml.dom.ls.DomLSSerializer");

                // Get the serialize method
                meth = gnuSer.getMethod("serialize",
                            new Class [] { Class.forName("org.w3c.dom.Node"),
                            Class.forName("java.io.OutputStream") } );

                // Get an instance
                Object serializer = gnuSer.newInstance();

                // Now call serialize to write the document
                meth.invoke(serializer, new Object [] { doc, baos } );
            } else {
                try {
                    DOMSource domSource = new DOMSource(doc);
                    StringWriter writer = new StringWriter();
                    StreamResult result = new StreamResult(writer);
                    TransformerFactory tf = TransformerFactory.newInstance();
                    Transformer transformer = tf.newTransformer();
                    transformer.transform(domSource, result);
                    return writer.toString().getBytes();
                } catch (Exception e) {
                    // We don't have another parser
                    IOException newEx = new IOException("No appropriate API (JAXP/Xerces) to serialize XML document: " + domImpl);
                    newEx.initCause(e);
                    throw newEx;
                }
            }
        }
        catch (Exception e) {
            // We may get some other errors, but the bottom line is that
            // the steps being executed no longer work
            IOException newEx = new IOException(e.getMessage());
            newEx.initCause(e);
            throw newEx;
        }

        byte bytes[] = baos.toByteArray();

        return bytes;
    }

    /**
     * Initializes a new DOM {@code Document} with the content containing
     * minimum OpenOffice XML tags.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public final void initContentDOM() throws IOException {

        contentDoc = createDOM(TAG_OFFICE_DOCUMENT_CONTENT);

        // this is a work-around for a bug in Office6.0 - not really
        // needed but StarCalc 6.0 will crash without this tag.
        Element root = contentDoc.getDocumentElement();

        Element child = contentDoc.createElement(TAG_OFFICE_FONT_DECLS);
        root.appendChild(child);

        child = contentDoc.createElement(TAG_OFFICE_AUTOMATIC_STYLES);
        root.appendChild(child);

        child = contentDoc.createElement(TAG_OFFICE_BODY);
        root.appendChild(child);
    }

    /**
     * Initializes a new DOM {@code Document} with the content containing
     * minimum OpenOffice XML tags.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public final void initSettingsDOM() throws IOException {

        settingsDoc = createSettingsDOM(TAG_OFFICE_DOCUMENT_SETTINGS);

        // this is a work-around for a bug in Office6.0 - not really
        // needed but StarCalc 6.0 will crash without this tag.
        Element root = settingsDoc.getDocumentElement();

        Element child = settingsDoc.createElement(TAG_OFFICE_SETTINGS);
        root.appendChild(child);
    }

    /**
     * Initializes a new DOM Document with styles containing minimum OpenOffice
     * XML tags.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public final void initStyleDOM() throws IOException {

        styleDoc = createDOM(TAG_OFFICE_DOCUMENT_STYLES);
    }

    /**
     * Creates a new DOM {@code Document} containing minimum OpenOffice XML tags.
     *
     * <p>This method uses the subclass {@code getOfficeClassAttribute} method
     * to get the attribute for <i>office:class</i>.</p>
     *
     * @param   rootName  root name of {@code Document}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    private final Document createSettingsDOM(String rootName) throws IOException {

        Document doc = null;

        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            doc = builder.newDocument();
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

        Element root = doc.createElement(rootName);
        doc.appendChild(root);

        root.setAttribute("xmlns:office", "http://openoffice.org/2000/office");
        root.setAttribute("xmlns:xlink", "http://openoffice.org/1999/xlink");
        root.setAttribute("xmlns:config", "http://openoffice.org/2001/config");
        root.setAttribute("office:version", "1.0");

        return doc;
    }

    /**
     * Creates a new DOM {@code Document} containing minimum OpenOffice XML tags.
     *
     * <p>This method uses the subclass {@code getOfficeClassAttribute} method
     * to get the attribute for <i>office:class</i>.</p>
     *
     * @param   rootName  root name of <code>Document</code>.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    private final Document createDOM(String rootName) throws IOException {

        Document doc = null;

        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            doc = builder.newDocument();
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

        Element root = doc.createElement(rootName);
        doc.appendChild(root);

        root.setAttribute("xmlns:office", "http://openoffice.org/2000/office");
        root.setAttribute("xmlns:style", "http://openoffice.org/2000/style");
        root.setAttribute("xmlns:text", "http://openoffice.org/2000/text");
        root.setAttribute("xmlns:table", "http://openoffice.org/2000/table");
        root.setAttribute("xmlns:draw", "http://openoffice.org/2000/drawing");
        root.setAttribute("xmlns:fo", "http://www.w3.org/1999/XSL/Format");
        root.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
        root.setAttribute("xmlns:number", "http://openoffice.org/2000/datastyle");
        root.setAttribute("xmlns:svg", "http://www.w3.org/2000/svg");
        root.setAttribute("xmlns:chart", "http://openoffice.org/2000/chart");
        root.setAttribute("xmlns:dr3d", "http://openoffice.org/2000/dr3d");
        root.setAttribute("xmlns:math", "http://www.w3.org/1998/Math/MathML");
        root.setAttribute("xmlns:form", "http://openoffice.org/2000/form");
        root.setAttribute("xmlns:script", "http://openoffice.org/2000/script");
        root.setAttribute("office:class", getOfficeClassAttribute());
        root.setAttribute("office:version", "1.0");

        return doc;
    }

    /**
     *  Return the <i>office:class</i> attribute value.
     *
     *  @return  The attribute value.
     */
    protected abstract String getOfficeClassAttribute();

    /**
     * Hacked code to filter {@literal <!DOCTYPE>} tag before sending stream to
     * parser.
     *
     * <p>This hacked code needs to be changed later on.</p>
     *
     * <p>Issue: using current jaxp1.0 parser, there is no way to turn off
     * processing of dtds.  Current set of dtds have bugs, processing them will
     * throw exceptions.</p>
     *
     * <p>This is a simple hack that assumes the whole {@literal <!DOCTYPE>} tag
     * are all in the same line.  This is sufficient for current StarOffice 6.0
     * generated XML files.  Since this hack really needs to go away, I don't
     * want to spend too much time in making it a perfect hack.</p>
     *
     * FIX (HJ): Removed requirement for DOCTYPE to be in one line
     * FIX (HJ): No longer removes newlines
     *
     * @param   is  {@code InputStream} to be filtered.
     *
     * @return  Reader value without the {@literal <!DOCTYPE>} tag.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    private static Reader hack(InputStream is) throws IOException {

        BufferedReader br = new BufferedReader(new InputStreamReader(is, "UTF-8"));
        StringBuffer buffer = new StringBuffer();

        String str;
        while ((str = br.readLine()) != null) {

            int sIndex = str.indexOf("<!DOCTYPE");
            if (sIndex > -1) {

                buffer.append(str.substring(0, sIndex));

                int eIndex = str.indexOf('>', sIndex + 8 );
                if (eIndex > -1) {

                    buffer.append(str.substring(eIndex + 1, str.length()));
                    // FIX (HJ): Preserve the newline
                    buffer.append("\n");

                } else {

                    // FIX (HJ): More than one line. Search for '>' in following lines
                    boolean bOK = false;
                    while ((str = br.readLine())!=null) {
                        eIndex = str.indexOf('>');
                        if (eIndex>-1) {
                            buffer.append(str.substring(eIndex+1));
                            // FIX (HJ): Preserve the newline
                            buffer.append("\n");
                            bOK = true;
                            break;
                        }
                    }

                    if (!bOK) { throw new IOException("Invalid XML"); }
                }

            } else {

                buffer.append(str);
                // FIX (HJ): Preserve the newline
                buffer.append("\n");
            }
        }

        StringReader r = new StringReader(buffer.toString());
        return r;
    }

    /**
     * Transform the {@code InputStream} to a Reader Stream.
     *
     * <p>This hacked code needs to be changed later on.</p>
     *
     * <p>Issue: the new oasis input file stream means that the old input stream
     * fails. see #i33702# </p>
     *
     * @param   is  {@code InputStream} to be filtered.
     *
     * @return  Reader value of the {@code InputStream()}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    private static Reader secondHack(InputStream is) throws IOException {

        BufferedReader br = new BufferedReader(new InputStreamReader(is, "UTF-8"));
        char[] charArray = new char[4096];
        StringBuffer sBuf = new StringBuffer();
        int n;
        while ((n=br.read(charArray, 0, charArray.length)) > 0) {
            sBuf.append(charArray, 0, n);
        }

        // ensure there is no trailing garbage after the end of the stream.
        int sIndex = sBuf.lastIndexOf("</office:document>");
        sBuf.delete(sIndex, sBuf.length());
        sBuf.append("</office:document>");
        StringReader r = new StringReader(sBuf.toString());
        return r;
    }

    /**
     * Method to create the initial entries in the manifest.xml file stored
     * in an SX? file.
     */
    private void initManifestDOM() throws IOException {

        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            DOMImplementation domImpl = builder.getDOMImplementation();

            DocumentType docType = domImpl.createDocumentType(TAG_MANIFEST_ROOT,
                                        "-//OpenOffice.org//DTD Manifest 1.0//EN",
                                        "Manifest.dtd");
            manifestDoc = domImpl.createDocument("manifest", TAG_MANIFEST_ROOT, docType);
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

        // Add the <manifest:manifest> entry
        Element manifestRoot = manifestDoc.getDocumentElement();

        manifestRoot.setAttribute("xmlns:manifest", "http://openoffice.org/2001/manifest");

        Element docRoot = manifestDoc.createElement(TAG_MANIFEST_FILE);

        docRoot.setAttribute(ATTRIBUTE_MANIFEST_FILE_PATH, "/");
        docRoot.setAttribute(ATTRIBUTE_MANIFEST_FILE_TYPE, getDocumentMimeType());

        manifestRoot.appendChild(docRoot);
    }
}

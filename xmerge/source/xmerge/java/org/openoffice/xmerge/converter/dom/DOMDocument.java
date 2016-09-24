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

package org.openoffice.xmerge.converter.dom;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringWriter;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;

import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import org.openoffice.xmerge.util.Debug;

/**
 * An implementation of {@code Document} for StarOffice documents.
 */
public class DOMDocument
    implements org.openoffice.xmerge.Document {

    /** Factory for {@code DocumentBuilder} objects. */
    private static DocumentBuilderFactory factory =
       DocumentBuilderFactory.newInstance();

    /** DOM {@code Document} of content.xml. */
    private Document contentDoc = null;

    private String documentName = null;
    private String fileName = null;
    private String fileExt = null;

    /**
     * Default constructor.
     *
     * @param  name  {@code Document} name.
     * @param  ext   {@code Document} extension.
     */
    public DOMDocument(String name,String ext) {
        this(name,ext,true, false);
    }

    /**
     * Returns the file extension of the {@code Document} represented.
     *
     * @return  file extension of the {@code Document}.
     */
    private String getFileExtension() {
        return fileExt;
    }

    /**
     * Constructor with arguments to set {@code namespaceAware} and
     * {@code validating} flags.
     *
     * @param  name            {@code Document} name (may or may not contain
     *                         extension).
     * @param  ext             {@code Document} extension.
     * @param  namespaceAware  Value for {@code namespaceAware} flag.
     * @param  validating      Value for {@code validating} flag.
     */
    private DOMDocument(String name, String ext,boolean namespaceAware,
            boolean validating) {

        factory.setValidating(validating);
        factory.setNamespaceAware(namespaceAware);
        this.fileExt = ext;
        this.documentName = trimDocumentName(name);
        this.fileName = documentName + getFileExtension();
    }

    /**
     * Removes the file extension from the {@code Document} name.
     *
     * @param  name  Full {@code Document} name with extension.
     *
     * @return Name of {@code Document} without the extension.
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
     * Return a DOM {@code Document} object of the document content file.
     *
     * <p>Note that a content DOM is not created when the constructor is called.
     * So, either the {@code read} method or the {@code initContentDOM} method
     * will need to be called ahead on this object before calling this method.
     * </p>
     *
     * @return  DOM {@code Document} object.
     */
    public Document getContentDOM() {

        return contentDoc;
    }

    /**
     * Sets the Content of the {@code Document} to the contents of the supplied
     * {@code Node} list.
     *
     * @param  newDom  DOM {@code Document} object.
     */
    public void setContentDOM( Node newDom) {
    contentDoc=(Document)newDom;
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
     * Read the Office {@code Document} from the specified {@code InputStream}.
     *
     * @param   is  Office document {@code InputStream}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream is) throws IOException {
     Debug.log(Debug.INFO, "reading file");
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            contentDoc = builder.parse(is);
        } catch (ParserConfigurationException ex) {
            System.out.println("Error:"+ ex);
        } catch (SAXException ex) {
            System.out.println("Error:"+ ex);
        }
    }

    /**
     * Write out content to the supplied {@code OutputStream}.
     *
     * @param   os  XML {@code OutputStream}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {

        // set bytes for writing to output stream
        byte contentBytes[] = docToBytes(contentDoc);

        os.write(contentBytes);
    }

    /**
     * Write out a {@code org.w3c.dom.Document} object into a {@code byte} array.
     *
     * <p>TODO: remove dependency on {@code com.sun.xml.tree.XmlDocument} package!
     * </p>
     *
     * @param   doc  DOM {@code Document} object.
     *
     * @return  {@code byte} array of DOM {@code Document} object.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    private byte[] docToBytes(Document doc)
        throws IOException {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        java.lang.reflect.Constructor<?> con;
        java.lang.reflect.Method meth;

        String domImpl = doc.getClass().getName();

        System.err.println("type b " + domImpl);

        /*
         * We may have multiple XML parsers in the Classpath.
         * Depending on which one is first, the actual type of
         * doc may vary.  Need a way to find out which API is being
         * used and use an appropriate serialization method.
         */
        try {
            // First of all try for JAXP 1.0
            if (domImpl.equals("com.sun.xml.tree.XmlDocument")) {
                System.out.println("Using JAXP");
                Class<?> jaxpDoc = Class.forName("com.sun.xml.tree.XmlDocument");

                // The method is in the XMLDocument class itself, not a helper
                meth = jaxpDoc.getMethod("write",
                        new Class[]{Class.forName("java.io.OutputStream")});

                meth.invoke(doc, new Object[]{baos});
            } else if (domImpl.equals("org.apache.crimson.tree.XmlDocument")) {
                System.out.println("Using Crimson");
                Class<?> crimsonDoc = Class.forName("org.apache.crimson.tree.XmlDocument");
                // The method is in the XMLDocument class itself, not a helper
                meth = crimsonDoc.getMethod("write",
                        new Class[]{Class.forName("java.io.OutputStream")});

                meth.invoke(doc, new Object[]{baos});
            } else if (domImpl.equals("org.apache.xerces.dom.DocumentImpl")
                    || domImpl.equals("org.apache.xerces.dom.DeferredDocumentImpl")) {
                System.out.println("Using Xerces");
                // Try for Xerces
                Class<?> xercesSer
                        = Class.forName("org.apache.xml.serialize.XMLSerializer");

                // Get the OutputStream constructor
                // May want to use the OutputFormat parameter at some stage too
                con = xercesSer.getConstructor(new Class[]{Class.forName("java.io.OutputStream"),
                    Class.forName("org.apache.xml.serialize.OutputFormat")});

                // Get the serialize method
                meth = xercesSer.getMethod("serialize",
                        new Class[]{Class.forName("org.w3c.dom.Document")});

                // Get an instance
                Object serializer = con.newInstance(new Object[]{baos, null});

                // Now call serialize to write the document
                meth.invoke(serializer, new Object[]{doc});
            } else if (domImpl.equals("gnu.xml.dom.DomDocument")) {
                System.out.println("Using GNU");

                Class<?> gnuSer = Class.forName("gnu.xml.dom.ls.DomLSSerializer");

                // Get the serialize method
                meth = gnuSer.getMethod("serialize",
                        new Class[]{Class.forName("org.w3c.dom.Node"),
                            Class.forName("java.io.OutputStream")});

                // Get an instance
                Object serializer = gnuSer.newInstance();

                // Now call serialize to write the document
                meth.invoke(serializer, new Object[]{doc, baos});
            } else {
                // We don't have another parser
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
                    IOException ex2 = new IOException("No appropriate API (JAXP/Xerces) to serialize XML document: " + domImpl);
                    ex2.initCause(e);
                    throw ex2;
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
}

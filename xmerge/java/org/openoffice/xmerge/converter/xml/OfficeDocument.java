/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.InputStreamReader;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
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
import org.xml.sax.SAXParseException;

import org.openoffice.xmerge.util.Resources;
import org.openoffice.xmerge.util.Debug;

/**
 *  An implementation of <code>Document</code> for
 *  StarOffice documents.
 */
public abstract class OfficeDocument
    implements org.openoffice.xmerge.Document,
               OfficeConstants {

    /** Factory for <code>DocumentBuilder</code> objects. */
    private static DocumentBuilderFactory factory =
       DocumentBuilderFactory.newInstance();

    /** DOM <code>Document</code> of content.xml. */
    private Document contentDoc = null;

   /** DOM <code>Document</code> of meta.xml. */
    private Document metaDoc = null;

   /** DOM <code>Document</code> of settings.xml. */
    private Document settingsDoc = null;

    /** DOM <code>Document</code> of content.xml. */
    private Document styleDoc = null;

    /** DOM <code>Docuemtn</code> of META-INF/manifest.xml. */
    private Document manifestDoc = null;

    private String documentName = null;
    private String fileName = null;

    /** Resources object. */
    private Resources res = null;

    /**
     *  <code>OfficeZip</code> object to store zip contents from
     *  read <code>InputStream</code>.  Note that this member
     *  will still be null if it was initialized using a template
     *  file instead of reading from a StarOffice zipped
     *  XML file.
     */
    private OfficeZip zip = null;

    /** Collection to keep track of the embedded objects in the document. */
    private Map embeddedObjects = null;

    /**
     *  Default constructor.
     *
     *  @param  name  <code>Document</code> name.
     */
    public OfficeDocument(String name)
    {
        this(name, true, false);
    }


    /**
     *  Constructor with arguments to set <code>namespaceAware</code>
     *  and <code>validating</code> flags.
     *
     *  @param  name            <code>Document</code> name (may or may not
     *                          contain extension).
     *  @param  namespaceAware  Value for <code>namespaceAware</code> flag.
     *  @param  validating      Value for <code>validating</code> flag.
     */
    public OfficeDocument(String name, boolean namespaceAware, boolean validating) {

        res = Resources.getInstance();
        factory.setValidating(validating);
        factory.setNamespaceAware(namespaceAware);
        this.documentName = trimDocumentName(name);
        this.fileName = documentName + getFileExtension();
    }


    /**
     *  Removes the file extension from the <code>Document</code>
     *  name.
     *
     *  @param  name  Full <code>Document</code> name with extension.
     *
     *  @return  Name of <code>Document</code> without the extension.
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
     *  Return a DOM <code>Document</code> object of the content.xml
     *  file.  Note that a content DOM is not created when the constructor
     *  is called.  So, either the <code>read</code> method or the
     *  <code>initContentDOM</code> method will need to be called ahead
     *  on this object before calling this method.
     *
     *  @return  DOM <code>Document</code> object.
     */
    public Document getContentDOM() {

        return contentDoc;
    }

 /**
     *  Return a DOM <code>Document</code> object of the meta.xml
     *  file.  Note that a content DOM is not created when the constructor
     *  is called.  So, either the <code>read</code> method or the
     *  <code>initContentDOM</code> method will need to be called ahead
     *  on this object before calling this method.
     *
     *  @return  DOM <code>Document</code> object.
     */
    public Document getMetaDOM() {

        return metaDoc;
    }


 /**
     *  Return a DOM <code>Document</code> object of the settings.xml
     *  file.  Note that a content DOM is not created when the constructor
     *  is called.  So, either the <code>read</code> method or the
     *  <code>initContentDOM</code> method will need to be called ahead
     *  on this object before calling this method.
     *
     *  @return  DOM <code>Document</code> object.
     */
    public Document getSettingsDOM() {

        return settingsDoc;
    }


    /**
     * Sets the content tree of the document.
     *
     * @param   newDom  <code>Node</code> containing the new content tree.
     */
    public void setContentDOM( Node newDom) {
        contentDoc = (Document)newDom;
    }


    /**
     * Sets the meta tree of the document.
     *
     * @param   newDom  <code>Node</code> containing the new meta tree.
     */
    public void setMetaDOM (Node newDom) {
        metaDoc = (Document)newDom;
    }


    /**
     * Sets the settings tree of the document.
     *
     * @param   newDom  <code>Node</code> containing the new settings tree.
     */
    public void setSettingsDOM (Node newDom) {
        settingsDoc = (Document)newDom;
    }


    /**
     * Sets the style tree of the document.
     *
     * @param   newDom  <code>Node</code> containing the new style tree.
     */
    public void setStyleDOM (Node newDom) {
        styleDoc = (Document)newDom;
    }


    /**
     *  Return a DOM <code>Document</code> object of the style.xml file.
     *  Note that this may return null if there is no style DOM.
     *  Note that a style DOM is not created when the constructor
     *  is called.  Depending on the <code>InputStream</code>, a
     *  <code>read</code> method may or may not build a style DOM.  When
     *  creating a new style DOM, call the <code>initStyleDOM</code> method
     *  first.
     *
     *  @return  DOM <code>Document</code> object.
     */
    public Document getStyleDOM() {

        return styleDoc;
    }


    /**
     *  Return the name of the <code>Document</code>.
     *
     *  @return  The name of <code>Document</code>.
     */
    public String getName() {

        return documentName;
    }


    /**
     *  Return the file name of the <code>Document</code>, possibly
     *  with the standard extension.
     *
     *  @return  The file name of <code>Document</code>.
     */
    public String getFileName() {

        return fileName;
    }


    /**
     *  Returns the file extension for this type of
     *  <code>Document</code>.
     *
     *  @return  The file extension of <code>Document</code>.
     */
    protected abstract String getFileExtension();


    /**
     * Returns all the embedded objects (graphics, formulae, etc.) present in
     * this document.
     *
     * @return An <code>Iterator</code> of <code>EmbeddedObject</code> objects.
     */
    public Iterator getEmbeddedObjects() {

        if (embeddedObjects == null && manifestDoc != null) {
            embeddedObjects = new HashMap();

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
                else if (!type.equals("")) {
                    embeddedObjects.put(path, new EmbeddedBinaryObject(path, type, zip));
                }
            }
        }

        return embeddedObjects.values().iterator();
    }

    /**
     * Returns the embedded object corresponding to the name provided.
     * The name should be stripped of any preceding path characters, such as
     * '/', '.' or '#'.
     *
     * @param   name    The name of the embedded object to retrieve.
     *
     * @return  An <code>EmbeddedObject</code> instance representing the named
     *          object.
     */
    public EmbeddedObject getEmbeddedObject(String name) {
        if (name == null) {
            return null;
        }

        if (embeddedObjects == null) {
            getEmbeddedObjects();
        }

        if (embeddedObjects.containsKey(name)) {
            return (EmbeddedObject)embeddedObjects.get(name);
        }
        else {
            return null;
        }
    }


    /**
     * Adds a new embedded object to the document.
     *
     * @param   embObj  An instance of <code>EmbeddedObject</code>.
     */
    public void addEmbeddedObject(EmbeddedObject embObj) {
        if (embObj == null) {
            return;
        }

        if (embeddedObjects == null) {
            embeddedObjects = new HashMap();
        }

        embeddedObjects.put(embObj.getName(), embObj);
    }


    /**
     *  Read the Office <code>Document</code> from the given
     *  <code>InputStream</code>.
     *
     *  @param  is  Office document <code>InputStream</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
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
     *  Read the Office <code>Document</code> from the given
     *  <code>InputStream</code>.
     *
     *  @param  is  Office document <code>InputStream</code>.
     *  @param  isZip <code>boolean</code> Identifies whether
     *                 a file is zipped or not
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream is, boolean isZip) throws IOException {

        Debug.log(Debug.INFO, "reading Office file");

        DocumentBuilder builder = null;

        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException ex) {
            throw new OfficeDocumentException(ex);
        }

    if (isZip)
    {
            read(is);
    }
    else{
        try{
        //System.out.println("\nParsing Input stream, validating?: "+builder.isValidating());
        contentDoc=  builder.parse((InputStream)is);
        }
        catch (SAXException ex) {
        throw new OfficeDocumentException(ex);
        }
    }

    }



    /**
     *  Parse given <code>byte</code> array into a DOM
     *  <code>Document</code> object using the
     *  <code>DocumentBuilder</code> object.
     *
     *  @param  builder  <code>DocumentBuilder</code> object for parsing.
     *  @param  bytes    <code>byte</code> array for parsing.
     *
     *  @return  Resulting DOM <code>Document</code> object.
     *
     *  @throws  SAXException  If any parsing error occurs.
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
     *  Write out Office ZIP file format.
     *
     *  @param  os  XML <code>OutputStream</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        if (zip == null) {
            zip = new OfficeZip();
        }

        initManifestDOM();

        Element domEntry;
        Element manifestRoot = manifestDoc.getDocumentElement();

        // The EmbeddedObjects come first.
        Iterator embObjs = getEmbeddedObjects();
        while (embObjs.hasNext()) {
            EmbeddedObject obj = (EmbeddedObject)embObjs.next();
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
     *  Write out Office ZIP file format.
     *
     *  @param  os  XML <code>OutputStream</code>.
     *  @param  isZip <code>boolean</code>
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os, boolean isZip) throws IOException {

        // Create an OfficeZip object if one does not exist.
        if (isZip){
            write(os);
    }
    else{
        byte contentBytes[] = docToBytes(contentDoc);
        os.write(contentBytes);
        if (styleDoc != null) {

        byte styleBytes[] = docToBytes(styleDoc);
        os.write(styleBytes);
        }
    }
    }


    /**
     *  <p>Write out a <code>org.w3c.dom.Document</code> object into a
     *  <code>byte</code> array.</p>
     *
     *  <p>TODO: remove dependency on com.sun.xml.tree.XmlDocument
     *  package!</p>
     *
     *  @param  Document  DOM <code>Document</code> object.
     *
     *  @return  <code>byte</code> array of DOM <code>Document</code>
     *           object.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    static byte[] docToBytes(Document doc)
        throws IOException {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        java.lang.reflect.Constructor con;
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

                Class jaxpDoc = Class.forName("com.sun.xml.tree.XmlDocument");

                // The method is in the XMLDocument class itself, not a helper
                meth = jaxpDoc.getMethod("write",
                            new Class[] { Class.forName("java.io.OutputStream") } );

                meth.invoke(doc, new Object [] { baos } );
            }
         else if (domImpl.equals("org.apache.crimson.tree.XmlDocument"))
        {
                Debug.log(Debug.INFO, "Using Crimson");

         Class crimsonDoc = Class.forName("org.apache.crimson.tree.XmlDocument");
         // The method is in the XMLDocument class itself, not a helper
                meth = crimsonDoc.getMethod("write",
                            new Class[] { Class.forName("java.io.OutputStream") } );

                meth.invoke(doc, new Object [] { baos } );
        }
            else if (domImpl.equals("org.apache.xerces.dom.DocumentImpl")
            || domImpl.equals("org.apache.xerces.dom.DeferredDocumentImpl")) {

                Debug.log(Debug.INFO, "Using Xerces");

                // Try for Xerces
                Class xercesSer =
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
            }
            else {
                // We don't have another parser
                throw new IOException("No appropriate API (JAXP/Xerces) to serialize XML document: " + domImpl);
            }
        }
        catch (ClassNotFoundException cnfe) {
            throw new IOException(cnfe.toString());
        }
        catch (Exception e) {
            // We may get some other errors, but the bottom line is that
            // the steps being executed no longer work
            throw new IOException(e.toString());
        }

        byte bytes[] = baos.toByteArray();

        return bytes;
    }


    /**
     *  Initializes a new DOM <code>Document</code> with the content
     *  containing minimum OpenOffice XML tags.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public final void initContentDOM() throws IOException {

        contentDoc = createDOM(TAG_OFFICE_DOCUMENT_CONTENT);

        // this is a work-around for a bug in Office6.0 - not really
        // needed but StarCalc 6.0 will crash without this tag.
        Element root = contentDoc.getDocumentElement();

        Element child = contentDoc.createElement(TAG_OFFICE_AUTOMATIC_STYLES);
        root.appendChild(child);

        child = contentDoc.createElement(TAG_OFFICE_BODY);
        root.appendChild(child);
    }

    /**
     *  Initializes a new DOM <code>Document</code> with the content
     *  containing minimum OpenOffice XML tags.
     *
     *  @throws  IOException  If any I/O error occurs.
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
     *  Initializes a new DOM Document with styles
     *  containing minimum OpenOffice XML tags.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public final void initStyleDOM() throws IOException {

        styleDoc = createDOM(TAG_OFFICE_DOCUMENT_STYLES);
    }

    /**
     *  <p>Creates a new DOM <code>Document</code> containing minimum
     *  OpenOffice XML tags.</p>
     *
     *  <p>This method uses the subclass
     *  <code>getOfficeClassAttribute</code> method to get the
     *  attribute for <i>office:class</i>.</p>
     *
     *  @param  rootName  root name of <code>Document</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private final Document createSettingsDOM(String rootName) throws IOException {

        Document doc = null;

        try {

            DocumentBuilder builder = factory.newDocumentBuilder();
            doc = builder.newDocument();

        } catch (ParserConfigurationException ex) {

            throw new OfficeDocumentException(ex);

        }

        Element root = (Element) doc.createElement(rootName);
        doc.appendChild(root);

        root.setAttribute("xmlns:office", "http://openoffice.org/2000/office");
        root.setAttribute("xmlns:xlink", "http://openoffice.org/1999/xlink");
        root.setAttribute("xmlns:config", "http://openoffice.org/2001/config");
        root.setAttribute("office:version", "1.0");

        return doc;
    }


    /**
     *  <p>Creates a new DOM <code>Document</code> containing minimum
     *  OpenOffice XML tags.</p>
     *
     *  <p>This method uses the subclass
     *  <code>getOfficeClassAttribute</code> method to get the
     *  attribute for <i>office:class</i>.</p>
     *
     *  @param  rootName  root name of <code>Document</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private final Document createDOM(String rootName) throws IOException {

        Document doc = null;

        try {

            DocumentBuilder builder = factory.newDocumentBuilder();
            doc = builder.newDocument();

        } catch (ParserConfigurationException ex) {

            throw new OfficeDocumentException(ex);

        }

        Element root = (Element) doc.createElement(rootName);
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
     *  <p>Hacked code to filter <!DOCTYPE> tag before
     *  sending stream to parser.</p>
     *
     *  <p>This hacked code needs to be changed later on.</p>
     *
     *  <p>Issue: using current jaxp1.0 parser, there is no way
     *  to turn off processing of dtds.  Current set of dtds
     *  have bugs, processing them will throw exceptions.</p>
     *
     *  <p>This is a simple hack that assumes the whole <!DOCTYPE>
     *  tag are all in the same line.  This is sufficient for
     *  current StarOffice 6.0 generated XML files.  Since this
     *  hack really needs to go away, I don't want to spend
     *  too much time in making it a perfect hack.</p>
     *
     *  @param  is  <code>InputStream</code> to be filtered.
     *
     *  @return  Reader value without the <!DOCTYPE> tag.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    private static Reader hack(InputStream is) throws IOException {

        BufferedReader br = new BufferedReader(new InputStreamReader(is, "UTF-8"));
        StringBuffer buffer = new StringBuffer(is.available());

        String str = null;

        while ((str = br.readLine()) != null) {

            int sIndex = str.indexOf("<!DOCTYPE");

            if (sIndex > -1) {

                buffer.append(str.substring(0, sIndex));

                int eIndex = str.indexOf('>', sIndex + 8 );

                if (eIndex > -1) {

                    buffer.append(str.substring(eIndex + 1, str.length()));

                } else {

                    throw new IOException("Invalid XML");
                }

            } else {

                buffer.append(str);
            }
        }

        StringReader r = new StringReader(buffer.toString());
        return r;
    }


    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    protected abstract String getDocumentMimeType();


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


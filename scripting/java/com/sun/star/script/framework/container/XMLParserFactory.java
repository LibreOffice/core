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

package com.sun.star.script.framework.container;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.lang.reflect.Method;

import javax.xml.parsers.*;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

public class XMLParserFactory {

    private static XMLParser parser = null;
    private static String officedtdurl = null;

    private XMLParserFactory() {}

    public static XMLParser getParser() {
        if (parser == null) {
            synchronized (XMLParserFactory.class) {
                if (parser == null)
                    parser = new DefaultParser();
            }
        }
        return parser;
    }

    public static void setParser(XMLParser p) {
        parser = p;
    }

    public static void setOfficeDTDURL(String url) {
        officedtdurl = url;
    }

    private static class DefaultParser implements XMLParser {

        private DocumentBuilderFactory factory;

        public DefaultParser() {
            factory = DocumentBuilderFactory.newInstance();
        }

        public Document parse(InputStream inputStream) throws IOException {
            Document result = null;
            InputSource is = null;

            try {
                DocumentBuilder builder = factory.newDocumentBuilder();

                is = new InputSource(inputStream);

                if (officedtdurl != null) {
                    is.setSystemId(officedtdurl);
                }

                result = builder.parse(is);
            }
            catch (SAXParseException spe) {
                throw new IOException(spe.getMessage());
            }
            catch (SAXException se) {
                throw new IOException(se.getMessage());
            }
            catch (ParserConfigurationException pce) {
                throw new IOException(pce.getMessage());
            }
            return result;
        }

        public void write(Document doc, OutputStream out) throws IOException {
            Class clazz = doc.getClass();
            String name = clazz.getName();

            // depending on the class of the Document object use introspection
            // to invoke the appropriate methods for writing the XML
            // this code is based on the code used by the NetBeans
            // class XMLUtilImpl in the openide module
            try {
                if (name.equals("com.sun.xml.tree.XmlDocument") ||
                    name.equals("org.apache.crimson.tree.XmlDocument")) {

                    // these DOM implementations are self writing
                    Method write;
                    write = clazz.getDeclaredMethod("write",
                        new Class[] {OutputStream.class});
                    write.invoke(doc, new Object[] {out});
                }
                else {
                    // try xerces serialize package using introspection
                    ClassLoader cl = this.getClass().getClassLoader();

                    Class serializerClass = null;
                    Class formatterClass = null;

                    try {
                        serializerClass = Class.forName(
                            "org.apache.xml.serialize.XMLSerializer", true, cl);
                        formatterClass = Class.forName(
                            "org.apache.xml.serialize.OutputFormat", true, cl);
                    } catch (ClassNotFoundException cnfe) {
                        String prefix = "com.sun.org.apache.xml.internal.";

                        serializerClass = Class.forName(
                            prefix +  "serialize.XMLSerializer" , true, cl);
                        formatterClass = Class.forName(
                            prefix + "serialize.OutputFormat", true, cl);
                    }

                    Object serializerObject = serializerClass.newInstance();
                    Object formatterObject = formatterClass.newInstance();

                    // improve output readability using the OutputFormat class
                    Method method = null;
                    method = formatterClass.getMethod("setMethod",
                        new Class[] {String.class});
                    method.invoke(formatterObject, new Object[] {"xml"});
                    method = formatterClass.getMethod("setIndenting",
                        new Class[] {Boolean.TYPE});
                    method.invoke(formatterObject, new Object[] {Boolean.TRUE});

                    // now set up an instance of XMLSerializer with our
                    // OutputStream and serialize our Document
                    method = serializerClass.getMethod("setOutputByteStream",
                        new Class[] {OutputStream.class});
                    method.invoke(serializerObject, new Object[] {out});
                    method = serializerClass.getMethod("setOutputFormat",
                        new Class[] {formatterClass});
                    method.invoke(serializerObject,
                        new Object[] {formatterObject});

                    method = serializerClass.getMethod("asDOMSerializer",
                        new Class[0]);
                    Object impl = method.invoke(serializerObject,
                        new Object[0]);

                    method = impl.getClass().getMethod("serialize",
                        new Class[] {Document.class});
                    method.invoke(impl, new Object[] {doc});
                }
            } catch (NoSuchMethodException ex) {
                throw new IOException(ex.getMessage());
            } catch (ClassNotFoundException ex) {
                throw new IOException(ex.getMessage());
            } catch (Exception ex) {
                throw new IOException(ex.getMessage());
            }
        }
    }
}

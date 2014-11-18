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

package com.sun.star.script.framework.container;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.lang.reflect.Method;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

public class XMLParserFactory {

    private static XMLParser parser = null;
    private static String officedtdurl = null;

    private XMLParserFactory() {}

    public static synchronized XMLParser getParser() {
        if (parser == null)
            parser = new DefaultParser();

        return parser;
    }

    public static void setOfficeDTDURL(String url) {
        officedtdurl = url;
    }

    private static class DefaultParser implements XMLParser {

        private final DocumentBuilderFactory factory;

        public DefaultParser() {
            factory = DocumentBuilderFactory.newInstance();
        }

        public Document parse(InputStream inputStream) throws IOException {

            Document result = null;

            try {
                DocumentBuilder builder = factory.newDocumentBuilder();
                InputSource is = new InputSource(inputStream);

                if (officedtdurl != null) {
                    is.setSystemId(officedtdurl);
                }

                result = builder.parse(is);
            } catch (SAXParseException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            } catch (SAXException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            } catch (ParserConfigurationException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            }

            return result;
        }

        public void write(Document doc, OutputStream out) throws IOException {

            Class<?> clazz = doc.getClass();
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
                } else {
                    // try xerces serialize package using introspection
                    ClassLoader cl = this.getClass().getClassLoader();

                    Class<?> serializerClass = null;
                    Class<?> formatterClass = null;

                    try {

                        serializerClass =
                            Class.forName("org.apache.xml.serialize.XMLSerializer",
                                          true, cl);

                        formatterClass =
                            Class.forName("org.apache.xml.serialize.OutputFormat",
                                          true, cl);

                    } catch (ClassNotFoundException cnfe) {
                        String prefix = "com.sun.org.apache.xml.internal.";

                        serializerClass =
                            Class.forName(prefix +  "serialize.XMLSerializer",
                                          true, cl);

                        formatterClass =
                            Class.forName(prefix + "serialize.OutputFormat",
                                          true, cl);
                    }

                    Object serializerObject = serializerClass.newInstance();
                    Object formatterObject = formatterClass.newInstance();

                    // improve output readability using the OutputFormat class
                    Method method =
                        formatterClass.getMethod("setMethod",
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

                    method.invoke(serializerObject, new Object[] {formatterObject});

                    method = serializerClass.getMethod("asDOMSerializer", new Class[0]);

                    Object impl = method.invoke(serializerObject, new Object[0]);

                    method = impl.getClass().getMethod("serialize",
                                                       new Class[] {Document.class});

                    method.invoke(impl, new Object[] {doc});
                }
            } catch (NoSuchMethodException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            } catch (ClassNotFoundException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            } catch (Exception ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            }
        }
    }
}
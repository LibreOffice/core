/*************************************************************************
 *
 *  $RCSfile: XMLParserFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-15 15:56:56 $
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

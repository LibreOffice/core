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

import com.sun.star.script.framework.log.LogUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.XMLConstants;

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

            try {
                factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            } catch (ParserConfigurationException e) {
                LogUtils.DEBUG(LogUtils.getTrace(e));
            }

            try {
                factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            } catch (ParserConfigurationException e) {
                LogUtils.DEBUG(LogUtils.getTrace(e));
            }

            try {
                factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            } catch (ParserConfigurationException e) {
                LogUtils.DEBUG(LogUtils.getTrace(e));
            }

            try {
                factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
            } catch (ParserConfigurationException e) {
                LogUtils.DEBUG(LogUtils.getTrace(e));
            }
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
            try {
                TransformerFactory transformerFactory = TransformerFactory.newInstance();
                transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
                transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_STYLESHEET, "");
                transformerFactory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
                transformerFactory.newTransformer().transform(
                    new DOMSource(doc), new StreamResult(out));
            } catch (TransformerException ex1) {
                IOException ex2 = new IOException();
                ex2.initCause(ex1);
                throw ex2;
            }
        }
    }
}

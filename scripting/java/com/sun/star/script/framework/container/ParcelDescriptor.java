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

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.ByteArrayInputStream;

import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Iterator;

// import javax.xml.parsers.DocumentBuilderFactory;
// import javax.xml.parsers.DocumentBuilder;
// import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.*;

public class ParcelDescriptor {

    // File name to be used for parcel descriptor files
    public static final String
        PARCEL_DESCRIPTOR_NAME = "parcel-descriptor.xml";

    // Collection of all ParcelDescriptor created for files
    private static final Map<File,ParcelDescriptor> PARCEL_DESCRIPTOR_MAP = new HashMap<File,ParcelDescriptor>(5);

    // This is the default contents of a parcel descriptor to be used when
    // creating empty descriptors
    private static final byte[] EMPTY_DOCUMENT =
        ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
         "<parcel xmlns:parcel=\"scripting.dtd\" language=\"Java\">\n" +
         "</parcel>").getBytes();

    private File file = null;
    private Document document = null;
    private String language = null;
    private Map<String,String> languagedepprops = new Hashtable<String,String>(3);

    public static synchronized void removeParcelDescriptor(File parent) {
        File path = new File(parent, PARCEL_DESCRIPTOR_NAME);
        PARCEL_DESCRIPTOR_MAP.remove(path);
    }

    public static synchronized void renameParcelDescriptor(File oldFile, File newFile) {
        File oldPath = new File(oldFile, PARCEL_DESCRIPTOR_NAME);
        ParcelDescriptor pd = PARCEL_DESCRIPTOR_MAP.get(oldPath);
        if(pd != null) {
            PARCEL_DESCRIPTOR_MAP.remove(oldPath);
            File newPath = new File(newFile, PARCEL_DESCRIPTOR_NAME);
            pd.file = newPath;
            PARCEL_DESCRIPTOR_MAP.put(newPath, pd);
        }
    }

    // returns the ParcelDescriptor in the corresponding directory
    // returns null if no ParcelDescriptor is found in the directory
    public static synchronized ParcelDescriptor
        getParcelDescriptor(File parent) {

        File path = new File(parent, PARCEL_DESCRIPTOR_NAME);
        ParcelDescriptor pd = PARCEL_DESCRIPTOR_MAP.get(path);

        if (pd == null && path.exists()) {
            try {
                pd = new ParcelDescriptor(path);
            }
            catch (IOException ioe) {
                return null;
            }
            PARCEL_DESCRIPTOR_MAP.put(path, pd);
        }
        return pd;
    }

    // returns a ParcelDescriptor for the corresponding directory
    // if no ParcelDescriptor exists, one is created
    public static synchronized ParcelDescriptor
        createParcelDescriptor(File parent) throws IOException {

        ParcelDescriptor pd = getParcelDescriptor(parent);

        if (pd == null) {
            if (parent == null || !parent.exists() || !parent.isDirectory()) {
                throw new IOException("Cannot create Parcel Descriptor");
            }

            File path = new File(parent, PARCEL_DESCRIPTOR_NAME);
            pd = new ParcelDescriptor(path);
            PARCEL_DESCRIPTOR_MAP.put(path, pd);
        }
        return pd;
    }

    public ParcelDescriptor() throws IOException {
        ByteArrayInputStream bis = null;
        try {
            bis = new ByteArrayInputStream(EMPTY_DOCUMENT);
            this.document = XMLParserFactory.getParser().parse(bis);
        }
        finally {
            if (bis != null)
                bis.close();
        }
    }

    public ParcelDescriptor(Document document) {
        this.document = document;
        initLanguageProperties();
    }

    public ParcelDescriptor(InputStream is) throws IOException {
        this(XMLParserFactory.getParser().parse(is));
    }

    public ParcelDescriptor(File file) throws IOException {
        this(file, "Java");
    }

    public ParcelDescriptor(File file, String language) throws IOException {
        this.file = file;

        if (file.exists()) {
            FileInputStream fis = null;
            try {
                fis = new FileInputStream(file);
                this.document = XMLParserFactory.getParser().parse(fis);
            }
            finally {
                if (fis != null)
                    fis.close();
            }
        }
        else {
            ByteArrayInputStream bis = null;
            try {
                bis = new ByteArrayInputStream(EMPTY_DOCUMENT);
                this.document = XMLParserFactory.getParser().parse(bis);
            }
            finally {
                if (bis != null)
                    bis.close();
            }
            setLanguage(language);
        }
        initLanguageProperties();
    }

    public void write(File file) throws IOException {
        FileOutputStream fos = new FileOutputStream(file);
        XMLParserFactory.getParser().write(document, fos);
        fos.close();
    }

    public void write() throws IOException {
        if (file == null)
            throw new FileNotFoundException("No file specified");

        write(file);
    }

    public void write(OutputStream out) throws IOException {
        XMLParserFactory.getParser().write(document, out);
    }

    public Document getDocument() {
        return document;
    }

    public String getLanguage() {
        if (language == null) {
            if (document != null) {
                Element e = document.getDocumentElement();
                language = e.getAttribute("language");
            }
        }
        return language;
    }

    public void setLanguage(String language) {
        this.language = language;

        if (document != null) {
            try {
                Element e = document.getDocumentElement();
                e.setAttribute("language", language);
            }
            catch (DOMException de) {
            }
        }
    }

    public ScriptEntry[] getScriptEntries() {
        ArrayList<ScriptEntry> scripts = new ArrayList<ScriptEntry>();
        NodeList scriptNodes;
        int len;

        if (document == null ||
            (scriptNodes = document.getElementsByTagName("script")) == null ||
            (len = scriptNodes.getLength()) == 0)
            return new ScriptEntry[0];

        for (int i = 0; i < len; i++) {
            String language, languagename, logicalname, description = "";
            Map<String,String> langProps = new HashMap<String,String>();
            NodeList nl;
            Element tmp;

            Element scriptElement = (Element)scriptNodes.item(i);
            language = scriptElement.getAttribute("language");

            nl = scriptElement.getElementsByTagName("logicalname");
            if (nl == null)
                logicalname = "";
            else {
                tmp = (Element)nl.item(0);
                logicalname = tmp.getAttribute("value");
            }

            // get the text of the description element
            nl = scriptElement.getElementsByTagName("locale");
            if (nl != null)
            {
                nl = nl.item(0).getChildNodes();
                if (nl != null)
                {
                    for (int j = 0 ; j < nl.getLength(); j++)
                    {
                        if (nl.item(j).getNodeName().equals("description"))
                        {
                            CharacterData cd =
                                (CharacterData)nl.item(j).getFirstChild();
                            description = cd.getData().trim();
                        }
                    }
                }
            }

            nl = scriptElement.getElementsByTagName("functionname");
            if (nl == null) {
                languagename = "";
            } else {
                tmp = (Element)nl.item(0);
                languagename = tmp.getAttribute("value");
            }
            nl = scriptElement.getElementsByTagName("languagedepprops");
            if ( nl != null && nl.getLength() > 0 )
            {

                NodeList props = ((Element)nl.item(0)).getElementsByTagName("prop");
                if ( props != null )
                {
                    for ( int j=0; j < props.getLength(); j++ )
                    {
                        tmp = (Element)props.item(j);
                        String key = tmp.getAttribute("name");
                        String val = tmp.getAttribute("value");
                        langProps.put( key,val );
                    }
                }
            }
            ScriptEntry entry = new ScriptEntry(language, languagename, logicalname, "",langProps, description);
            scripts.add(entry);
        }
        return scripts.toArray(new ScriptEntry[scripts.size()]);
    }

    public void setScriptEntries(ScriptEntry[] scripts) {
        clearEntries();
        for (int i = 0; i < scripts.length; i++)
            addScriptEntry(scripts[i]);
    }

    public void setScriptEntries(Enumeration<ScriptEntry> scripts) {
        clearEntries();
        while (scripts.hasMoreElements())
            addScriptEntry(scripts.nextElement());
    }

    public String getLanguageProperty(String name) {
        return languagedepprops.get(name);
    }

    public void setLanguageProperty(String name, String value) {
        languagedepprops.put(name, value);
        setScriptEntries(getScriptEntries());
    }

    private void initLanguageProperties() {
        NodeList nl = document.getElementsByTagName("languagedepprops");
        int len;

        if (nl != null && (len = nl.getLength()) != 0) {

            for (int i = 0; i < len; i++) {
                Element e = (Element)nl.item(i);
                NodeList nl2 = e.getElementsByTagName("prop");
                int len2;

                if (nl2 != null && (len2 = nl2.getLength()) != 0) {
                    for (int j = 0; j < len2; j++) {
                        Element e2 = (Element)nl2.item(j);

                        String name = e2.getAttribute("name");
                        String value = e2.getAttribute("value");

                        if (getLanguageProperty(name) == null) {
                            languagedepprops.put(name, value);
                        }
                    }
                }
            }
        }
    }

    private void clearEntries() {
        NodeList scriptNodes;
        Element main = document.getDocumentElement();
        int len;

        if ((scriptNodes = document.getElementsByTagName("script")) != null &&
            (len = scriptNodes.getLength()) != 0)
        {
            for (int i = len - 1; i >= 0; i--) {
                try {
                    main.removeChild(scriptNodes.item(i));
                }
                catch (DOMException de) {
                    // ignore
                }
            }
        }
    }

    public void removeScriptEntry(ScriptEntry script) {
        NodeList scriptNodes;
        Element main = document.getDocumentElement();
        int len;

        if ((scriptNodes = document.getElementsByTagName("script")) != null &&
            (len = scriptNodes.getLength()) != 0)
        {
            for (int i = len - 1; i >= 0; i--) {
                try {
                    Element scriptElement = (Element)scriptNodes.item(i);
                    String languagename = "";

                    NodeList nl =
                        scriptElement.getElementsByTagName("functionname");
                    if (nl == null) {
                        continue;
                    } else {
                        Element tmp = (Element)nl.item(0);
                        languagename = tmp.getAttribute("value");
                    }

                    if (languagename.equals(script.getLanguageName())) {
                        main.removeChild(scriptElement);
                    }
                }
                catch (DOMException de) {
                    // ignore
                }
            }
        }
    }

    public void addScriptEntry(ScriptEntry script) {
        Element main = document.getDocumentElement();
        Element root, item, tempitem;

        root = document.createElement("script");
        root.setAttribute("language", script.getLanguage());

        item = document.createElement("locale");
        item.setAttribute("lang", "en");
        tempitem = document.createElement("displayname");
        tempitem.setAttribute("value", script.getLogicalName());
        item.appendChild(tempitem);

        tempitem = document.createElement("description");
        String description = script.getDescription();
        if (description == null || description.equals(""))
        {
            description = script.getLogicalName();
        }
        tempitem.appendChild(document.createTextNode(description));
        item.appendChild(tempitem);

        root.appendChild(item);

        item = document.createElement("logicalname");
        item.setAttribute("value", script.getLogicalName());
        root.appendChild(item);

        item = document.createElement("functionname");
        item.setAttribute("value", script.getLanguageName());
        root.appendChild(item);

        if (languagedepprops != null && languagedepprops.size() != 0) {
            String key;
            item = document.createElement("languagedepprops");

            Iterator<String> iter = languagedepprops.keySet().iterator();
            while (iter.hasNext()) {
                tempitem = document.createElement("prop");
                key = iter.next();
                tempitem.setAttribute("name", key);
                tempitem.setAttribute("value", languagedepprops.get(key));
                item.appendChild(tempitem);
            }
            root.appendChild(item);
        }

        //add to the Top Element
        main.appendChild(root);
    }
}

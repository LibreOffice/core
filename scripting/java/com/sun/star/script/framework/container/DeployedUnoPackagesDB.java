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

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.ByteArrayInputStream;

import java.util.ArrayList;

import org.w3c.dom.*;

public class DeployedUnoPackagesDB {

    // This is the default contents of a parcel descriptor to be used when
    // creating empty descriptors
    private static final byte[] EMPTY_DOCUMENT =
        ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
         "<unopackages xmlns:unopackages=\"unopackages.dtd\">\n" +
         "</unopackages>").getBytes();

    private Document document = null;

    public DeployedUnoPackagesDB() throws IOException {
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

    public DeployedUnoPackagesDB(Document document) {
        this.document = document;
    }

    public DeployedUnoPackagesDB(InputStream is) throws IOException {
        this(XMLParserFactory.getParser().parse(is));
    }

    public String[] getDeployedPackages( String language )
    {
        ArrayList<String> packageUrls = new ArrayList<String>(4);
        Element main = document.getDocumentElement();
        Element root = null;
        int len = 0;
        NodeList langNodes = null;

        if ((langNodes = main.getElementsByTagName("language")) != null &&
            (len = langNodes.getLength()) != 0)
        {
            for ( int i=0; i<len; i++ )
            {
                Element e = (Element)langNodes.item( i );
                if ( e.getAttribute("value").equals(language) )
                {
                    root = e;
                    break;
                }
            }
        }
        if ( root != null )
        {
            len = 0;
            NodeList packages = null;
            if ((packages = root.getElementsByTagName("package")) != null &&
                (len = packages.getLength()) != 0)
            {

                for ( int i=0; i<len; i++ )
                {

                    Element e = (Element)packages.item( i );
                    packageUrls.add( e.getAttribute("value") );
                }
            }
        }
        if ( !packageUrls.isEmpty() )
        {
            return packageUrls.toArray( new String[packageUrls.size()] );
        }
        return new String[0];
    }

    public void write(OutputStream out) throws IOException {
        XMLParserFactory.getParser().write(document, out);
    }

    public Document getDocument() {
        return document;
    }


    private void clearEntries() {
        NodeList langNodes;
        Element main = document.getDocumentElement();
        int len;

        if ((langNodes = document.getElementsByTagName("language")) != null &&
            (len = langNodes.getLength()) != 0)
        {
            for (int i = len - 1; i >= 0; i--) {
                try {
                    main.removeChild(langNodes.item(i));
                }
                catch (DOMException de) {
                    // ignore
                }
            }
        }
    }

    public boolean removePackage( String language, String url )
    {
        Element main = document.getDocumentElement();
        Element langNode = null;
        int len = 0;
        NodeList langNodes = null;
        boolean result = false;
        if ((langNodes = main.getElementsByTagName("language")) != null &&
            (len = langNodes.getLength()) != 0)
        {
            for ( int i=0; i<len; i++ )
            {
                Element e = (Element)langNodes.item( i );
                if ( e.getAttribute("value").equals(language) )
                {
                    langNode = e;
                    break;
                }
            }
        }
        if ( langNode != null )
        {
            len = 0;
            NodeList packages = null;
            if ((packages = langNode.getElementsByTagName("package")) != null &&
                (len = packages.getLength()) != 0)
            {
                for ( int i=0; i<len; i++ )
                {

                    Element e = (Element)packages.item( i );
                    String value =  e.getAttribute("value");

                    if ( value.equals(url) )
                    {
                        langNode.removeChild( e );
                        result = true;
                        break;
                    }
                }
            }
        }
        return result;
    }

    public void addPackage(String language, String url ) {
        Element main = document.getDocumentElement();
        Element langNode = null;
        Element pkgNode = null;

        int len = 0;
        NodeList langNodes = null;

        if ((langNodes = document.getElementsByTagName("language")) != null &&
            (len = langNodes.getLength()) != 0)
        {
            for ( int i=0; i<len; i++ )
            {
                Element e = (Element)langNodes.item( i );
                if ( e.getAttribute("value").equals(language) )
                {
                    langNode = e;
                    break;
                }
            }
        }
        if ( langNode == null )
        {
            langNode = document.createElement("language");
            langNode.setAttribute( "value", language );
        }
        pkgNode = document.createElement("package");
        pkgNode.setAttribute( "value", url );

        langNode.appendChild(pkgNode);
        //add to the Top Element
        main.appendChild(langNode);
    }
}

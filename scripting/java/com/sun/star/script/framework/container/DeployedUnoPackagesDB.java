/*************************************************************************
 *
 *  $RCSfile: DeployedUnoPackagesDB.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:57:30 $
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

public class DeployedUnoPackagesDB {

    // File name to be used for parcel descriptor files
    private static final String
        PARCEL_DESCRIPTOR_NAME = "unopkg-desc.xml";

    // This is the default contents of a parcel descriptor to be used when
    // creating empty descriptors
    private static final byte[] EMPTY_DOCUMENT =
        ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
         "<unopackages xmlns:unopackages=\"unopackages.dtd\">\n" +
         "</unopackages>").getBytes();

    private File file = null;
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
        ArrayList packageUrls = new ArrayList(4);
        Element main = document.getDocumentElement();
        Element root = null;
        Element item;
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
            return (String[])packageUrls.toArray( new String[0] );
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

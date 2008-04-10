/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLPackageDescription.java,v $
 * $Revision: 1.3 $
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

package org.openoffice.setup.SetupData;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Util.FileExtensionFilter;
import java.io.File;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Stack;
import java.util.Vector;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.DefaultHandler;
public class XMLPackageDescription {

    /**
     * fill the package description tree by handling the SAXParser events
     */
    private class PackageDescriptionHandler extends DefaultHandler {

        private XMLPackageDescription root;
        private Stack stack;

        public PackageDescriptionHandler(XMLPackageDescription base) {
            root  = base;
            stack = new Stack();
        }

        private PackageDescriptionHandler() {
            /* forbidden */
        }

        public XMLPackageDescription getDescription() {
            return root;
        }

        /* implement the DefaultHandler interface */

        public void characters(char[] ch, int start, int length) {
            XMLPackageDescription entity = (XMLPackageDescription) stack.peek();
            entity.value = entity.value == null ? new String(ch, start, length)
                                                : entity.value + new String(ch, start, length);
        }
        public void startDocument() {
            stack.push(root);
        }
        public void endDocument() {
            stack.pop();
        }
        public void startElement(String uri, String localName, String qName, Attributes attributes) {
            XMLPackageDescription parent = (XMLPackageDescription) stack.peek();
            XMLPackageDescription entity = new XMLPackageDescription();

            entity.key = qName;
            for (int i = 0; i < attributes.getLength(); i++) {
                entity.attributes.put(attributes.getQName(i), attributes.getValue(i));
            }

            parent.add(entity);
            stack.push(entity);
        }
        public void endElement(String uri, String localName, String qName) {
            stack.pop();
        }
        public void error(SAXParseException e) {
            System.err.println("Parse Error:" + e);
        }
        public void processingInstruction(String target, String data) {
            /* ignore */
        }
        public void skippedEntity(String name) {
            /* ignore */
        }
        public void warning(SAXParseException e) {
            System.err.println("Warning:" + e);
        }
    }

    /**
     * general storage for XML elements
     */

    private String key;             /* XML element name       */
    private String value;           /* XML element characters */
    private Hashtable attributes;   /* XML element attributes */
    private Vector children;        /* children are of type XMLPackageDescription */

    protected XMLPackageDescription() {
        key        = "";
        value      = null;
        attributes = new Hashtable();
        children   = new Vector();
    }

    private void add(XMLPackageDescription p) {
        children.add(p);
    }

    /**
     * helper routines to find content information
     */
    protected String getKey() {
        return key;
    }
    protected String getAttribute(String key) {
        return (String) attributes.get(key);
    }
    protected String getValue() {
        return value;
    }
    protected XMLPackageDescription getElement(String key) {
        return getElement(key, null, null);
    }
    protected XMLPackageDescription getElement(String key, String attrKey, String attrValue) {
        for (Enumeration e = children.elements(); e.hasMoreElements();) {
            XMLPackageDescription child = (XMLPackageDescription) e.nextElement();
            if (child.key.equals(key)) {
                if (attrKey == null) {
                    return child;
                } else if (attrValue.equals(child.getAttribute(attrKey))) {
                    return child;
                }
            }
        }
        return null;
    }

    /**
     * find a PackageDescription of type package that has the given name,
     * recurses into the children
     */
    private XMLPackageDescription findPackage(String name) {
        String self = (String) attributes.get("name");

        if ((self != null) && self.equals(name))
            return this;

        XMLPackageDescription found = null;
        for (Enumeration e = children.elements(); e.hasMoreElements();) {
            XMLPackageDescription child = (XMLPackageDescription) e.nextElement();
            if (child.getAttribute("parent") != null) {
                found = child.findPackage(name);
                if (found != null) {
                    break;
                }
            }
        }
        return found;
    }

    /**
     * adjust the tree so that all children have a matching parent and not just
     * the ones they got by reading files in random order
     */
    private void adjust(XMLPackageDescription root) {
        String self = (String) attributes.get("name");

        for (int i = children.size() - 1; i >= 0; --i) {
            XMLPackageDescription child = (XMLPackageDescription) children.elementAt(i);
            String childParentName = child.getAttribute("parent");
            if (childParentName != null) {

                child.adjust(root);

                if ((childParentName != null) && (childParentName.length() > 0) && (! childParentName.equals(self))) {
                    XMLPackageDescription newParent = root.findPackage(childParentName);
                    if (newParent != null) {
                        newParent.add(child);
                        children.remove(i);
                    }
                }
            }
        }
    }

    protected void read() {
        PackageDescriptionHandler handler = new PackageDescriptionHandler(this);

        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser parser = factory.newSAXParser();

            InstallData data = InstallData.getInstance();
            File xpdRoot = data.getInfoRoot("xpd");

            if ( xpdRoot != null ) {
                File[] file = xpdRoot.listFiles(new FileExtensionFilter("xpd"));

                if (file != null) {
                    for (int i = 0; i < file.length; i++) {
                        parser.parse(file[i], handler);
                    }
                } else {
                    key   = "";
                    value = "no package file found";
                }
            }
            else {
                System.err.println("Did not find xpd directory");
            }
        } catch (ParserConfigurationException ex) {
            ex.printStackTrace();
        } catch (IOException ex) {
            ex.printStackTrace();
        } catch (SAXException ex) {
            ex.printStackTrace();
        }

        adjust(this);
    }

    /* provide an iterator through the children */
    protected class Elements implements Enumeration {

        Enumeration e;

        protected Elements() {
            e = children.elements();
        }
        public boolean hasMoreElements() {
            return e.hasMoreElements();
        }
        public Object nextElement() {
            return e.nextElement();
        }
    }

    protected Enumeration elements() {
        return new Elements();
    }


    // FIXME: remove it, dump() is for testing only
    public void dump() {
        dump("");
    }

    // FIXME: remove it, dump(String) is for testing only
    public void dump(String indent) {
        final String space = "    ";
        if (key != null) {
            System.out.print(indent + "<" + key);
        }
        for (Enumeration e = attributes.keys() ; e.hasMoreElements() ;) {
            String key   = (String) e.nextElement();
            String value = (String) attributes.get(key);
            System.out.print(" " + key + "=\"" + value + "\"");
        }
        if (key != null) {
            System.out.print(">");
        }

        if ((value != null) && (value.length() > 0)) {
            String trimmedValue = value.trim();
            if (trimmedValue.length() > 60) {
                trimmedValue = trimmedValue.substring(0, 57) + "...";
            }
            if (trimmedValue.length() > 0) {
                System.out.print(trimmedValue);
            }
        }

        for (Enumeration e = children.elements() ; e.hasMoreElements() ;) {
            XMLPackageDescription current = (XMLPackageDescription) e.nextElement();
            System.out.println();
            current.dump(indent + space);
        }
        if (children.size() > 0) {
            System.out.println();
            System.out.print(indent);
        }
        if (key != null) {
           System.out.print("</" + key + ">");
        }
    }
}

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

package org.openoffice.xmerge.converter.xml;

import java.lang.reflect.Constructor;
import java.util.ArrayList;

import org.openoffice.xmerge.util.Debug;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;


/**
 *  A <code>StyleCatalog</code> holds a collection of <code>Style</code>
 *  objects.  It is intended for use when parsing or building a DOM
 *  document.
 *
 *  Each entry in the <code>StyleCatalog</code> represents a
 *  <code>Style</code>, and is an object which is a subclass of
 *  <code>Style</code>.
 *
 *  @author David Proulx
 *  @see <a href="Style.html">Style</a>
 */
public class StyleCatalog {

    private ArrayList<Style> styles;  // The actual styles

    /**
     *  Constructor
     *
     *  @param  initialEntries  Expected number of entries to set
     *                          for efficiency purposes.
     */
    public StyleCatalog(int initialEntries) {
        styles = new ArrayList<Style>(initialEntries);
    }


    /**
     *  <p>Parse the <code>Document</code> starting from <code>node</code>
     *  and working downward, and add all styles found, so long as their
     *  family name is listed in <code>families</code>.  For each
     *  family name in <code>families</code> there must be a corresponding
     *  element in <code>classes</code>, which specifies the class type
     *  to use for that family.  All of these classes must be
     *  subclasses of <code>Style</code>.  There can be multiple
     *  classes specified for a particular family.</p>
     *
     *  <p>If <code>defaultClass</code> is non-null, then all styles that
     *  are found will be added.  Any <code>Style</code> whose family is
     *  not listed in <code>families</code> will be added using defaultClass,
     *  which, of course, must be a subclass of <code>Style</code>.
     *  If <code>alwaysCreateDefault</code> is true, then a class
     *  of type <code>defaultClass</code> will always be created,
     *  regardless of whether there was also a match in
     *  <code>families</code>.</p>
     *
     *  <p>DJP Todo: make it recursive so that <code>node</code> can be
     *  higher up in the <code>Document</code> tree.</p>
     *
     *  @param  node                 The node to be searched for
     *                               <code>Style</code> objects.
     *  @param  families             An array of <code>Style</code> families
     *                               to add.
     *  @param  classes              An array of class types corresponding
     *                               to the families array.
     *  @param  defaultClass         All <code>Style</code> objects that are
     *                               found are added to this class.
     *  @param  alwaysCreateDefault  A class of type <code>defaultClass</code>
     *                               will always be created, regardless of
     *                               whether there is a match in the
     *                               families array.
     */
    public void add(Node node, String families[], Class<?> classes[],
    Class<?> defaultClass, boolean alwaysCreateDefault) {

    if (node == null)
            return;

        if (families == null)
            families = new String[0];
        if (classes == null)
            classes = new Class[0];
        if (node.hasChildNodes()) {
            NodeList children = node.getChildNodes();
            int len = children.getLength();

            for (int i = 0; i < len; i++) {
                boolean found = false;
                Node child = children.item(i);
                String name = child.getNodeName();
                if (name.equals("style:default-style") || name.equals("style:style")) {
                    String familyName = getFamilyName(child);
                    if (familyName == null) {
                        Debug.log(Debug.ERROR, "familyName is null!");
                        continue;
                    }

                    for (int j = 0; j < families.length; j++) {
                        if (families[j].equals(familyName)) {
                            callConstructor(classes[j], child);
                            found = true;
                        }
                    }
                    if ((!found || alwaysCreateDefault) && (defaultClass != null))
                        callConstructor(defaultClass, child);
                }
            }
        }
    }


    /**
     *  Call the constructor of class <code>cls</code> with parameters
     *  <code>node</code>, and add the resulting <code>Style</code> to
     *  the catalog.
     *
     *  @param  cls   The class whose constructor will be called.
     *  @param  node  The constructed class will be added to this node.
     */
    private void callConstructor(Class<?> cls, Node node) {
        Class<?> params[] = new Class[2];
        params[0] = Node.class;
        params[1] = this.getClass();
        try {
            Constructor<?> c = cls.getConstructor(params);
            Object p[] = new Object[2];
            p[0] = node;
            p[1] = this;
            styles.add((Style) c.newInstance(p));
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Exception when calling constructor", e);
        }
    }


    /**
     *  Add a <code>Style</code> to the catalog.
     *
     *  @param  s  The <code>Style</code> to add.
     */
    public void add(Style s) {
        styles.add(s);
    }


    /**
     *  Return the first <code>Style</code> matching the specified names.
     *
     *  @param  name        Name to match, null is considered
     *                      <i>always match</i>.
     *  @param  family      Family to match, null is considered
     *                      <i>always match</i>.
     *  @param  parent      Parent to match, null is considered
     *                      <i>always match</i>.
     *  @param  styleClass  styleClass to match, null is considered
     *                      <i>always match</i>.
     *
     *  @return  <code>Style</code> value if all parameters match,
     *           null otherwise
     */
    public Style lookup(String name, String family, String parent,
                        Class<?> styleClass) {
        int nStyles = styles.size();
        for (int i = 0; i < nStyles; i++) {
            Style s = styles.get(i);
            if ((name != null) && (s.getName() != null)
            && (!s.getName().equals(name)))
                continue;
            if ((family != null) && (s.getFamily() != null)
            && (!s.getFamily().equals(family)))
                continue;
            if ((parent != null) && (s.getParent() != null)
            && (!s.getParent().equals(parent)))
                continue;
            if ((styleClass != null) && (s.getClass() != styleClass))
                continue;
            if (s.getName() == null) continue;  // DJP: workaround for "null name" problem
            return s;
        }
        return null;  // none found
    }


    /**
     *  Given a <code>Style</code> <code>s<code> return all
     *  <code>Style</code> objects that match.
     *
     *  @param  s  <code>Style</code> to match.
     *
     *  @return  An array of <code>Style</code> objects that match, an
     *           empty array if none match.
     */
    public Style[] getMatching(Style s) {

        // Run through and count the matching styles so we know how big of
        // an array to allocate.
        int matchCount = 0;
        int nStyles = styles.size();
        for (int j = 0; j < nStyles; j++) {
            Style p = styles.get(j).getResolved();
            if (p.isSubset(s)) matchCount++;
        }

        // Now allocate the array, and run through again, populating it.
        Style[] matchArray = new Style[matchCount];
        matchCount = 0;
        for (int j = 0; j < nStyles; j++) {
            Style p = styles.get(j).getResolved();
            if (p.isSubset(s)) matchArray[matchCount++] = p;
        }
        return matchArray;
    }


    /**
     *  Given a <code>Style</code> <code>s</code>, return the
     *  <code>style</code> that is the closest match.  Not currently
     *  implemented.
     *
     *  @param  s  <code>Style</code> to match.
     *
     *  @return  The <code>Style</code> that most closely matches.
     */
    public Style getBestMatch(Style s) {
        // DJP: is this needed?
        // DJP ToDo: implement this
        return null;
    }


    /**
     *  <p>Create a <code>Node</code> named <code>name</code> in
     *  <code>Document</code> <code>parentDoc</code>, and write the
     *  entire <code>StyleCatalog</code> to it.</p>
     *
     *  <p>Note that the resulting node is returned, but is not connected
     *  into the document.  Placing the output node in the document is
     *  left to the caller.</p>
     *
     *  @param  parentDoc  The <code>Document</code> to add the
     *                     <code>Node</code>.
     *  @param  name       The name of the <code>Node</code> to add.
     *
     *  @return  The <code>Element</code> that was created.
     */
    public Element writeNode(org.w3c.dom.Document parentDoc, String name) {
        Element rootNode = parentDoc.createElement(name);

        int len = styles.size();
        for (int j = 0; j < len; j++) {
            Style s = styles.get(j);

            Element styleNode = parentDoc.createElement("style:style");

            if (s.getName() != null)
                styleNode.setAttribute("style:name", s.getName());
            if (s.getParent() != null)
                styleNode.setAttribute("style:parent-style-name", s.getParent());
            if (s.getFamily() != null)
                styleNode.setAttribute("style:family", s.getFamily());

            Element propertiesNode = (Element) s.createNode(parentDoc, "style:properties");
            // DJP: only add node if has children OR attributes
            if (propertiesNode != null)
                styleNode.appendChild(propertiesNode);

            rootNode.appendChild(styleNode);
        }

        return rootNode;
    }


    /**
     *  Dump the <code>Style</code> table in Comma Separated Value (CSV)
     *  format
     *
     *  @param  para  If true, dump in paragraph <code>Style</code>,
     *                otherwise dump in text style.
     */
    public void dumpCSV(boolean para) {
        if (!para) {
            TextStyle.dumpHdr();
            int nStyles = styles.size();
            for (int i = 0; i < nStyles; i++) {
                Style s = styles.get(i);
                if (s.getClass().equals(TextStyle.class))
                    ((TextStyle)s).dumpCSV();
            }
        } else {
            ParaStyle.dumpHdr();
            int nStyles = styles.size();
            for (int i = 0; i < nStyles; i++) {
                Style s = styles.get(i);
                if (s.getClass().equals(ParaStyle.class))
                    ((ParaStyle)s).dumpCSV();
            }
        }

    }


    /**
     *  Find the family attribute of a <code>Style</code> <code>Node</code>.
     *
     *  @param  node  The <code>Node</code> to check.
     *
     *  @return  The family attribute, or null if one does not
     *           exist.
     */
    private String getFamilyName(Node node) {
        NamedNodeMap attributes = node.getAttributes();
        if (attributes != null) {
            int len = attributes.getLength();
            for (int i = 0; i < len; i++) {
                Node attr = attributes.item(i);
                if (attr.getNodeName().equals("style:family")) {
                    return attr.getNodeValue();
                }
            }
        }
        return null;
    }
}


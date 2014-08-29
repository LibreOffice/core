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

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

/**
 * An object of class {@code Style} represents a <i>style</i> in an OpenOffice
 * document.
 *
 * <p>In practice subclasses of this {@code Style}, such as {@code TextStyle},
 * {@code ParaStyle} are used.</p>
 *
 * @see  <a href="TextStyle.html">TextStyle</a>
 * @see  <a href="ParaStyle.html">ParaStyle</a>
 */
public class Style {

    /** Name of the {@code Style}. */
    protected String name = null;
    /** Family of the {@code Style}. */
    protected String family = null;
    /** Parent of the {@code Style}. */
    protected String parent = null;

    /**
     * A reference to the {@code StyleCatalog} to be used for looking up ancestor
     * {@code Style} objects.
     */
    protected StyleCatalog sc;

    /**
     * Constructor for use when going from DOM to client device format.
     *
     * @param  node  A <i>style:style</i> or <i>style:default-style</i>
     *               {@code Node} from the document being parsed. No checking of
     *               {@code Node} is done, so if it is not of the proper type
     *               the results will be unpredictable.
     * @param  sc    The {@code StyleCatalog}, which is used for looking up
     *               ancestor {@code Style} objects.
     */
    public Style(Node node, StyleCatalog sc) {

        this.sc = sc;

        // Run through the attributes of this node, saving
        // the ones we're interested in.
        if (node.getNodeName().equals("style:default-style"))
            name = "DEFAULT_STYLE";
        NamedNodeMap attrNodes = node.getAttributes();
        if (attrNodes != null) {
            int len = attrNodes.getLength();
            for (int i = 0; i < len; i++) {
                Node attr = attrNodes.item(i);
                if (attr.getNodeName().equals("style:family"))
                    family = attr.getNodeValue();
                else if (attr.getNodeName().equals("style:name")) {
                    name = attr.getNodeValue();
                } else if (attr.getNodeName().equals("style:parent-style-name"))
                    parent = attr.getNodeValue();

            }
        }
    }

    /**
     * Constructor for use when going from client device format to DOM.
     *
     * @param  name    Name of the {@code Style}.  Can be {@code null}.
     * @param  family  Family of the {@code Style} {@literal -} usually
     *                 <i>paragraph</i>, <i>text</i>, etc.  Can be {@code null}.
     * @param  parent  Name of the parent {@code Style}, or {@code null} if none.
     * @param  sc      The {@code StyleCatalog}, which is used for looking up
     *                 ancestor {@code Style} objects.
     */
    public Style(String name, String family, String parent, StyleCatalog sc) {
        this.sc = sc;
        this.name = name;
        this.family = family;
        this.parent = parent;
    }

    /**
     * Set the {@code StyleCatalog} to be used when looking up the {@code Style}
     * parent.
     *
     * @param  sc  The {@code StyleCatalog}, which is used for looking up
     *             ancestor {@code Style} objects.
     */
    public void setCatalog(StyleCatalog sc) {
        this.sc = sc;
    }

    /**
     * Returns the name of this {@code Style}.
     *
     * @return  The name of this {@code Style}.
     */
    public String getName() {
        return name;
    }

    /**
     * Sets the name of this {@code Style}.
     *
     * @param  newName  The new name of this {@code Style}.
     */
    public void setName(String newName) {
        name = newName;
    }

    /**
     * Return the family of this {@code Style}.
     *
     * @return  The family of this {@code Style}.
     */
    public String getFamily() {
        return family;
    }

    /**
     * Return the name of the parent of this {@code Style}.
     *
     * @return  The parent of this {@code Style}.
     */
    public String getParent() {
        return parent;
    }

    /**
     * Return a {@code Style} object corresponding to this one, but with all of
     * the inherited information from parent {@code Style} objects filled in.
     *
     * <p>The object returned will be a new object, not a reference to this
     * object, even if it does not need any information added.</p>
     *
     * @return  A resolved {@code Style} object in which to look up ancestors.
     */
    public Style getResolved() {
        return new Style(name, family, parent, sc);
    }

    /**
     * Write a {@code Node} in {@code parentDoc} representing this {@code Style}.
     *
     * <p>Note that the {@code Node} is returned unconnected.</p>
     *
     * @param  parentDoc  Document to which new {@code Node} will belong.
     * @param  name       Name to use for new {@code Node}.
     */
    public Node createNode(org.w3c.dom.Document parentDoc, String name) {
        // DJP: write this!  Should call writeAttributes()
        return null;
    }

    /**
     * Return {@code true} if {@code Style} is a subset of this one.
     *
     * <p>Note that this will return true even if {@code Style} is less specific
     * than this {@code Style}, so long as it does not contradict this
     * {@code Style} in any way.</p>
     *
     * <p>This always returns true since only subclasses of {@code Style}
     * contain any actual {@code Style} information.</p>
     *
     * @param   style  The {@code Style} to check.
     *
     * @return  {@code true} if the {@code Style} is a subset, {@code false}
     *          otherwise.
     */
    public boolean isSubset(Style style) {
        return true;
    }
}
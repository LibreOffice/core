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

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.util.Debug;


abstract class conversionAlgorithm {
    abstract int I(String val);
}

/**
 *  This algorithm expects only values in millimeters, e.g. "20.3mm".
 */
class horizSize extends conversionAlgorithm {
    int I(String value) {
        if (value.endsWith("mm")) {
            float size = (float)0.0;
            String num = value.substring(0, value.length() - 2);
            try {
                size = Float.parseFloat(num);
            } catch (Exception e) {
                Debug.log(Debug.ERROR, "Error parsing " + value, e);
            }
            size *= 100;
            return (int)size;
        } else {
            Debug.log(Debug.ERROR, "Unexpected value (" + value
            + ") in horizSize.I()");
            return 0;
        }
    }
}


/*
 *  This algorithm does line height - can be either millimeters or
 *  a percentage.
 */
class lineHeight extends conversionAlgorithm {
    int I(String value) {
        if (value.endsWith("mm")) {
            float size = (float)0.0;
            String num = value.substring(0, value.length() - 2);
            try {
                size = Float.parseFloat(num);
            } catch (Exception e) {
                Debug.log(Debug.ERROR, "Error parsing " + value, e);
            }
            size *= 100;
            return (int)size;
        } else if (value.endsWith("%")) {
            float size = (float)0.0;
            String num = value.substring(0, value.length() - 1);
            try {
                size = Float.parseFloat(num);
            } catch (Exception e) {
                Debug.log(Debug.ERROR, "Error parsing " + value, e);
            }
            int retval = (int) size;
            retval |= ParaStyle.LH_PCT;
            return retval;
        }
        return 0;
    }
}


/**
 *  This class converts alignment values.
 */
class alignment extends conversionAlgorithm {
    int I(String value) {
        if (value.equals("end"))
            return ParaStyle.ALIGN_RIGHT;
        if (value.equals("right"))
            return ParaStyle.ALIGN_RIGHT;
        if (value.equals("center"))
            return ParaStyle.ALIGN_CENTER;
        if (value.equals("justify"))
            return ParaStyle.ALIGN_JUST;
        if (value.equals("justified"))
            return ParaStyle.ALIGN_JUST;
        if (value.equals("start"))
            return ParaStyle.ALIGN_LEFT;
        if (value.equals("left"))
            return ParaStyle.ALIGN_LEFT;
        Debug.log(Debug.ERROR, "Unknown string ("
        + value + ") in alignment.I()");
        return ParaStyle.ALIGN_LEFT;
    }
}


/**
 *  <p>This class represents a paragraph <code>Style</code>.</p>
 *
 *  <p><table border="1" cellpadding="1"><tr><td>
 *  Attribute        </td><td>Value
 *  </td></tr><tr><td>
 *  MARGIN_LEFT      </td><td>mm * 100
 *  </td></tr><tr><td>
 *  MARGIN_RIGHT     </td><td>mm * 100
 *  </td></tr><tr><td>
 *  MARGIN_TOP       </td><td>mm * 100 (space on top of paragraph)
 *  </td></tr><tr><td>
 *  MARGIN_BOTTOM    </td><td>mm * 100
 *  </td></tr><tr><td>
 *  TEXT_INDENT      </td><td>mm * 100 (first line indent)
 *  </td></tr><tr><td>
 *  LINE_HEIGHT      </td><td>mm * 100, unless or'ed with LH_PCT, in which
 *                   case it is a percentage (e.g. 200% for double spacing)
 *                   Can also be or'ed with LH_ATLEAST.  Value is stored
 *                   in bits indicated by LH_VALUEMASK.
 *  </td></tr><tr><td>
 *  TEXT_ALIGN       </td><td>ALIGN_RIGHT, ALIGN_CENTER, ALIGN_JUST, ALIGN_LEFT
 *  </td></tr></table></p>
 */
public class ParaStyle extends Style implements Cloneable {

    /**  The left margin property. */
    public static final int MARGIN_LEFT      = 0;
    /**  The right margin property. */
    public static final int MARGIN_RIGHT     = 1;
    /**  The top margin property. */
    public static final int MARGIN_TOP       = 2;
    /**  The bottom margin property. */
    public static final int MARGIN_BOTTOM    = 3;
    /**  Indent left property. */
    public static final int TEXT_INDENT      = 4;
    /**  Indent right property. */
    public static final int LINE_HEIGHT      = 5;
    /**  Align text property. */
    public static final int TEXT_ALIGN       = 6;
    // This must always be one more than highest property
    /**  Total number of properties. */
    protected static final int NR_PROPERTIES    = 7;

    /**
     *  Array of flags indicating which attributes are set for this
     *  paragraph <code>Style</code>.
     */
    protected boolean isSet[] = new boolean[NR_PROPERTIES];
    /**  Array of attribute values for this paragraph <code>tyle</code>. */
    protected int  value[] = new int[NR_PROPERTIES];
    /**  Array of attribute names for this paragraph <code>Style</code>. */
    protected String attrName[] = {
        "fo:margin-left",
        "fo:margin-right",
        "fo:margin-top",
        "fo:margin-bottom",
        "fo:text-indent",
        "fo:line-height",
        "fo:text-align"
    };

    /**  Array of attribute structures for this paragraph <code>Style</code>. */
    protected Class<?> algor[] = {
        horizSize.class,
        horizSize.class,
        horizSize.class,
        horizSize.class,
        horizSize.class,
        lineHeight.class,
        alignment.class
    };

    /**  Align right. */
    public static final int ALIGN_RIGHT   = 1;
    /**  Align center. */
    public static final int ALIGN_CENTER  = 2;
    /**  Align justified. */
    public static final int ALIGN_JUST    = 3;
    /**  Align left. */
    public static final int ALIGN_LEFT    = 4;

    /**  Line height percentage.  */
    public static final int LH_PCT        = 0x40000000;
    /**  Line height minimum value.  */
    public static final int LH_ATLEAST    = 0x20000000;
    /**  Line height mask.  */
    public static final int LH_VALUEMASK  = 0x00FFFFFF;

    /**  Ignored tags. */
    private static String[] ignored = {
        "style:font-name", "fo:font-size", "fo:font-weight", "fo:color",
        "fo:language", "fo:country", "style:font-name-asian",
        "style:font-size-asian", "style:language-asian",
        "style:country-asian", "style:font-name-complex",
        "style:font-size-complex", "style:language-complex",
        "style:country-complex", "style:text-autospace", "style:punctuation-wrap",
        "style:line-break", "fo:keep-with-next", "fo:font-style",
        "text:number-lines", "text:line-number"
    };


    /**
     *  Constructor for use when going from DOM to client device format.
     *
     *  @param  node  A <i>style:style</i> <code>Node</code> which, which
     *                is assumed to have <i>family</i> attribute of
     *                <i>paragraph</i>.
     *  @param  sc    The <code>StyleCatalog</code>, which is used for
     *                looking up ancestor <code>Style</code> objects.
     */
    public ParaStyle(Node node, StyleCatalog sc) {

        super(node, sc);

        // Look for children.  Only ones we care about are "style:properties"
        // nodes.  If any are found, recursively traverse them, passing
        // along the style element to add properties to.
        //
        if (node.hasChildNodes()) {
            NodeList children = node.getChildNodes();
            int len = children.getLength();
            for (int i = 0; i < len; i++) {
                Node child = children.item(i);
                String name = child.getNodeName();
                if (name.equals("style:properties")) {
                    NamedNodeMap childAttrNodes = child.getAttributes();
                    if (childAttrNodes != null) {
                        int nChildAttrNodes = childAttrNodes.getLength();
                        for (int j = 0; j < nChildAttrNodes; j++) {
                            Node attr = childAttrNodes.item(j);
                            setAttribute(attr.getNodeName(), attr.getNodeValue());
                        }
                    }
                }
            }
        }
    }


    /**
     *  Constructor for use when going from client device format to DOM.
     *
     *  @param  name       Name of the <code>Style</code>.  Can be null.
     *  @param  familyName Family of the <code>Style</code> - usually
     *                     <i>paragraph</i>, <i>text</i>, etc.  Can be null.
     *  @param  parentName Name of the parent <code>Style</code>, or null
     *                     if none.
     *  @param  attribs    Array of attributes to set.
     *  @param  values     Array of values to set.
     *  @param  sc         The <code>StyleCatalog</code>, which is used for
     *                     looking up ancestor <code>Style</code> objects.
     */
    public ParaStyle(String name, String familyName, String parentName,
    String attribs[], String values[], StyleCatalog sc) {
        super(name, familyName, parentName, sc);
        if (attribs != null)
            for (int i = 0; i < attribs.length; i++)
                setAttribute(attribs[i], values[i]);
    }


    /**
     *  Alternate constructor for use when going from client device
     *  format to DOM.
     *
     *  @param  name       Name of the <code>Style</code>.  Can be null.
     *  @param  familyName Family of the <code>Style</code> - usually
     *                     <i>paragraph</i>, <i>text</i>, etc.  Can be null.
     *  @param  parentName Name of the parent <code>Style</code>, or
     *                     null if none.
     *  @param  attribs    Array of attributes indices to set.
     *  @param  values     Array of values to set.
     *  @param  lookup     The <code>StyleCatalog</code>, which is used for
     *                     looking up ancestor <code>Style</code> objects.
     */
    public ParaStyle(String name, String familyName, String parentName,
    int attribs[], String values[], StyleCatalog lookup) {
        super(name, familyName, parentName, lookup);
        if (attribs != null)
            for (int i = 0; i < attribs.length; i++)
                setAttribute(attribs[i], values[i]);
    }


    /**
     *  This code checks whether an attribute is one that we
     *  intentionally ignore.
     *
     *  @param  attribute  The attribute to check.
     *
     *  @return  true if attribute can be ignored, false otherwise.
     */
    private boolean isIgnored(String attribute) {
        for (int i = 0; i < ignored.length; i++) {
            if (ignored[i].equals(attribute))
                return true;
        }
        return false;
    }


    /**
     *  Set an attribute for this paragraph <code>Style</code>.
     *
     *  @param  attr   The attribute to set.
     *  @param  value  The attribute value to set.
     */
    public void setAttribute(String attr, String value) {
        for (int i = 0; i < NR_PROPERTIES; i++) {
            if (attr.equals(attrName[i])) {
                setAttribute(i, value);
                return;
            }
        }
        if (!isIgnored(attr))
            Debug.log(Debug.INFO, "ParaStyle Unhandled: " + attr + "=" + value);
    }


    /**
     *  Check whether an attribute is set in this <code>Style</code>.
     *
     *  @param  attrIndex  The attribute index to check.
     *
     *  @return  true if the attribute at specified index is set,
     *           false otherwise.
     */
    public boolean isAttributeSet(int attrIndex) {
        return isSet[attrIndex];
    }


    /**
     *  Get the value of an integer attribute.
     *
     *  @param  attrIndex  Index of the attribute.
     *
     *  @return  Value of the attribute, 0 if not set.
     */
    public int getAttribute(int attrIndex) {
        if (isSet[attrIndex])
            return  value[attrIndex];
        else return 0;
    }


    /**
     *  Set an attribute for this paragraph <code>Style</code>.
     *
     *  @param  attr   The attribute index to set.
     *  @param  value  The attribute value to set.
     */
    public void setAttribute(int attr, String value) {
        isSet[attr] = true;
        try {
            this.value[attr] = (((conversionAlgorithm)algor[attr].newInstance())).I(value);
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Instantiation error", e);
        }
    }


    /**
     *  Return the <code>Style</code> in use.
     *
     *  @return  The fully-resolved copy of the <code>Style</code> in use.
     */
    public Style getResolved() {
        ParaStyle resolved = null;
        try {
            resolved = (ParaStyle)this.clone();
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Can't clone", e);
        }

        // Look up the parent style.  (If there is no style catalog
        // specified, we can't do any lookups).
        ParaStyle parentStyle = null;
        if (sc != null) {
            if (parent != null) {
                parentStyle = (ParaStyle)sc.lookup(parent, family, null,
                              this.getClass());
                if (parentStyle == null)
                    Debug.log(Debug.ERROR, "parent style lookup of "
                              + parent + " failed!");
                else
                    parentStyle = (ParaStyle)parentStyle.getResolved();
            } else if (!name.equals("DEFAULT_STYLE")) {
                parentStyle = (ParaStyle)sc.lookup("DEFAULT_STYLE", null, null,
                                                   this.getClass());
            }
        }

        // If we found a parent, for any attributes which we don't have
        // set, try to get the values from the parent.
        if (parentStyle != null) {
            parentStyle = (ParaStyle)parentStyle.getResolved();
            for (int i = 0; i < NR_PROPERTIES; i++) {
                if (!isSet[i] && parentStyle.isSet[i]) {
                    resolved.isSet[i] = true;
                    resolved.value[i] = parentStyle.value[i];
                }
            }
        }
        return resolved;
    }


    /**
     *  Private function to return the value as an element in
     *  a Comma Separated Value (CSV) format.
     *
     *  @param  value  The value to format.
     *
     *  @return  The formatted value.
     */
    private static String toCSV(String value) {
        if (value != null)
            return "\"" + value + "\",";
        else
            return "\"\",";
    }


    /**
     *  Private function to return the value as a last element in
     *  a Comma Separated Value (CSV) format.
     *
     *  @param  value  The value to format.
     *
     *  @return  The formatted value.
     */
    private static String toLastCSV(String value) {
        if (value != null)
            return "\"" + value + "\"";
        else
            return "\"\"";
    }


    /**
     *  Print a Comma Separated Value (CSV) header line for the
     *  spreadsheet dump.
     */
    public static void dumpHdr() {
        System.out.println(toCSV("Name") + toCSV("Family") + toCSV("parent")
        + toCSV("left mgn") + toCSV("right mgn")
        + toCSV("top mgn") + toCSV("bottom mgn") + toCSV("txt indent")
        + toCSV("line height") + toLastCSV("txt align"));
    }


    /**
     *  Dump this <code>Style</code> as a Comma Separated Value (CSV)
     *  line.
     */
    public void dumpCSV() {
        String attributes = "";
        for (int index = 0; index <= 6; index++) {
            if (isSet[index]) {
                attributes += toCSV("" + value[index]);
            }
            else
                attributes += toCSV(null);  // unspecified
        }
        System.out.println(toCSV(name) + toCSV(family) + toCSV(parent)
        + attributes + toLastCSV(null));
    }


    /**
     *  Create the <code>Node</code> with the specified elements.
     *
     *  @param parentDoc  Parent <code>Document</code> of the
     *                    <code>Node</code> to create.
     *  @param name       Name of the <code>Node</code>.
     *
     *  @return  The created <code>Node</code>.
     */
    public Node createNode(org.w3c.dom.Document parentDoc, String name) {
        Element node = parentDoc.createElement(name);
        writeAttributes(node);
        return node;
    }


    /**
     *  Return true if <code>style</code> is a subset of the
     *  <code>Style</code>.
     *
     *  @param  style  <code>Style</code> to check.
     *
     *  @return  true if <code>style</code> is a subset, false
     *           otherwise.
     */
    public boolean isSubset(Style style) {

        if (!super.isSubset(style))
            return false;
        if (!this.getClass().isAssignableFrom(style.getClass()))
            return false;
        ParaStyle ps = (ParaStyle)style;

        for (int i = 0; i < NR_PROPERTIES; i++) {
            if (ps.isSet[i]) {
                if (i < NR_PROPERTIES - 1) {
                    // Compare the actual values.  We allow a margin of error
                    // here because the conversion loses precision.
                    int diff;
                    if (value[i] > ps.value[i])
                        diff = value[i] - ps.value[i];
                    else
                        diff = ps.value[i] - value[i];
                    if (diff > 32)
                        return false;
                } else {
                    if (i == TEXT_ALIGN)
                        if ((value[i] == 0) && (ps.value[i] == 4))
                            continue;
                    if (value[i] != ps.value[i])
                        return false;
                }
            }
        }
        return true;
    }


    /**
     *  Add <code>Style</code> attributes to the given
     *  <code>Node</code>.  This may involve writing child
     *  <code>Node</code> objects as well.
     *
     *  @param  node  The <code>Node</code> to add <code>Style</code>
     *                attributes.
     */
    public void writeAttributes(Element node) {
        for (int i = 0; i <= TEXT_INDENT; i++) {
            if (isSet[i]) {
                double temp = value[i] / 100.0;
                String stringVal = (new Double(temp)).toString() + "mm";
                node.setAttribute(attrName[i], stringVal);
            }
        }

        if (isSet[LINE_HEIGHT]) {
            String stringVal;
            if ((value[LINE_HEIGHT] & LH_PCT) != 0)
                stringVal = (new Integer(value[LINE_HEIGHT] & LH_VALUEMASK)).toString() + "%";
            else {
                double temp = (value[LINE_HEIGHT] & LH_VALUEMASK) / 100.0;
                stringVal = (new Double(temp)).toString() + "mm";
            }
            node.setAttribute(attrName[LINE_HEIGHT], stringVal);
        }

        if (isSet[TEXT_ALIGN]) {
            String val;
            switch (value[TEXT_ALIGN]) {
                case ALIGN_RIGHT:  val = "end"; break;
                case ALIGN_CENTER: val = "center"; break;
                case ALIGN_JUST:   val = "justify"; break;
                case ALIGN_LEFT:   val = "left"; break;
                default:           val = "unknown"; break;
            }
            node.setAttribute(attrName[TEXT_ALIGN], val);
        }
    }
}


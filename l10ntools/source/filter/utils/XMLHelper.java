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

package com.sun.star.filter.config.tools.utils;

//_______________________________________________

import java.lang.*;
import java.util.*;
import java.io.*;

//_______________________________________________

/**
 *  It provides some constant values and some static helper routines
 *  which are neccessary to work with a xml file - especialy
 *  the filter configuration.
 *
 *
 */
public class XMLHelper
{


    /** its a possible value of the xml attribute "oor:type" and identify an integer type. */
    public static final java.lang.String XMLTYPE_INTEGER = "xs:int";

    /** its a possible value of the xml attribute "oor:type" and identify an boolean type. */
    public static final java.lang.String XMLTYPE_BOOLEAN = "xs:boolean";

    /** its a possible value of the xml attribute "oor:type" and identify an string type. */
    public static final java.lang.String XMLTYPE_STRING = "xs:string";

    /** its a possible value of the xml attribute "oor:type" and identify an string list type. */
    public static final java.lang.String XMLTYPE_STRINGLIST = "oor:string-list";

    /** its a xml attribute, which specify a property name. */
    public static final java.lang.String XMLATTRIB_OOR_NAME = "oor:name";

    /** its a xml attribute, which specify a property type. */
    public static final java.lang.String XMLATTRIB_OOR_TYPE = "oor:type";

    /** its a xml attribute, which specify a list separator. */
    public static final java.lang.String XMLATTRIB_OOR_SEPARATOR = "oor:separator";

    /** its a xml attribute, which specify a localized value. */
    public static final java.lang.String XMLATTRIB_OOR_LOCALIZED = "oor:localized";

    /** its a xml attribute, which specify a merge operation for cfg layering. */
    public static final java.lang.String XMLATTRIB_OOR_OP = "oor:op";

    /** can be used as value for XMLATTRIB_OOR_OP. */
    public static final java.lang.String XMLATTRIB_OP_REPLACE = "replace";

    /** its a xml attribute, which specify a locale value. */
    public static final java.lang.String XMLATTRIB_XML_LANG = "xml:lang";

    /** its the tag name of a <value ...> entry. */
    public static final java.lang.String XMLTAG_VALUE = "value";

    /** its the tag name of a <prop ...> entry. */
    public static final java.lang.String XMLTAG_PROP = "prop";

    /** its the tag name of a <node ...> entry. */
    public static final java.lang.String XMLTAG_NODE = "node";

    //___________________________________________
    // private const

    /** a static list of all possible separators, which can be used for configuration type string-list. */
    private static final java.lang.String[] DELIMS = {" ", ",", ";", ".", ":", "-", "_", "#", "'", "+", "*", "~", "=", "?"};

    /** index of the default separator inside list DELIMS.
     *  Its neccessary to know such default separator; because it can
     *  be supressed as xml attribute of the corresponding value tag. */
    private static final int DEFAULT_SEPARATOR = 0;

    //___________________________________________

    /** analyze the structures of the given XML node and
     *  return a property set of all found sub nodes.
     *
     *  Such properties are organized as [name, value] pairs.
     *  The type of a xml node will be detected automaticly.
     *  Following types are supported:
     *      xs:int          => java.lang.Integer
     *      xs:bool         => java.lang.Boolean
     *      xs:string       => java.lang.String
     *      oor:string-list => java.util.LinkedList[java.lang.String]
     *      oor:set         => java.util.Vector[java.lang.Object]
     *      oor:localized   => java.util.HashMap[java.lang.Object]
     *
     *  @param  aNode
     *          points directly to the xml node, where we should analyze
     *          the children nodes.
     *
     *  @return [java.util.HashMap]
     *          contains every node name as key and its string(!) as value.
     */
    public static java.util.HashMap convertNodeToPropSet(org.w3c.dom.Node aNode)
        throws java.lang.Exception
    {
        java.util.HashMap aPropSet = new java.util.HashMap();

        // get all child nodes, which seems to be properties
        java.util.Vector      lChildNodes = XMLHelper.extractChildNodesByTagName(aNode, XMLTAG_PROP);
        java.util.Enumeration en1         = lChildNodes.elements();
        while(en1.hasMoreElements())
        {
            org.w3c.dom.Node aChildNode = (org.w3c.dom.Node)en1.nextElement();

            // read its name
            java.lang.String sChildName = XMLHelper.extractNodeAttribByName(aChildNode, XMLATTRIB_OOR_NAME);
            if (sChildName == null)
                throw new java.io.IOException("unsupported format: could not extract child node name");

            // read its type info
            java.lang.String sChildType = XMLHelper.extractNodeAttribByName(aChildNode, XMLATTRIB_OOR_TYPE);
            if (sChildType == null)
            {
                /** Special patch:
                 *      If an xml tag has no type information set ... we can restore it
                 *      by analyzing the already readed tag name :-)
                 *      Not very nice - but it can help to read stripped xml files too. */
                sChildType = XMLHelper.getTypeForTag(sChildName);
                if (sChildType == null)
                    throw new java.io.IOException("unsupported format: could not extract child node type");
            }

            // read its value(s?)
            java.util.Vector      lChildValues = XMLHelper.extractChildNodesByTagName(aChildNode, XMLTAG_VALUE);
            java.util.Enumeration en2          = lChildValues.elements();
            int                   nValue       = 0;
            java.lang.Object      aValue       = null;
            while(en2.hasMoreElements())
            {
                org.w3c.dom.Node aValueNode   = (org.w3c.dom.Node)en2.nextElement();
                java.lang.String sChildLocale = XMLHelper.extractNodeAttribByName(aValueNode, XMLATTRIB_XML_LANG);
                boolean          bLocalized   = (sChildLocale != null);

                ++nValue;

                if (sChildType.equals(XMLTYPE_INTEGER))
                {
                    if (!bLocalized && nValue > 1)
                        throw new java.io.IOException("unsupported format: more then one value for non localized but atomic type detected");
                    java.lang.String sData = ((org.w3c.dom.CharacterData)aValueNode.getFirstChild()).getData();
                    aValue = new java.lang.Integer(sData);
                }
                else
                if (sChildType.equals(XMLTYPE_BOOLEAN))
                {
                    if (!bLocalized && nValue > 1)
                        throw new java.io.IOException("unsupported format: more then one value for non localized but atomic type detected");
                    java.lang.String sData = ((org.w3c.dom.CharacterData)aValueNode.getFirstChild()).getData();
                    aValue = new java.lang.Boolean(sData);
                }
                else
                if (sChildType.equals(XMLTYPE_STRING))
                {
                    if (!bLocalized && nValue > 1)
                        throw new java.io.IOException("unsupported format: more then one value for non localized but atomic type detected");

                    java.lang.String sData = ((org.w3c.dom.CharacterData)aValueNode.getFirstChild()).getData();

                    java.util.HashMap lLocalized = null;
                    if (bLocalized)
                    {
                        if (aValue == null)
                            aValue = new java.util.HashMap();
                        lLocalized = (java.util.HashMap)aValue;
                        lLocalized.put(sChildLocale, sData);
                    }
                    else
                        aValue = sData;
                }
                else
                if (sChildType.equals(XMLTYPE_STRINGLIST))
                {
                    if (!bLocalized && nValue > 1)
                        throw new java.io.IOException("unsupported format: more then one value for non localized but atomic type detected");

                    java.lang.String sSeparator = XMLHelper.extractNodeAttribByName(aChildNode, XMLATTRIB_OOR_SEPARATOR);
                    if (sSeparator == null)
                        sSeparator = " ";

                    java.lang.String          sData      = ((org.w3c.dom.CharacterData)aValueNode.getFirstChild()).getData();
                    sData = sData.replace('\t', ' ');
                    sData = sData.replace('\n', ' ');
                    java.util.StringTokenizer aTokenizer = new java.util.StringTokenizer(sData, sSeparator);
                    java.util.Vector          lList      = new java.util.Vector();
                    while(aTokenizer.hasMoreTokens())
                    {
                        java.lang.String sToken = (java.lang.String)aTokenizer.nextToken();
                        sToken.trim();
                        if (sToken.length() < 1)
                            continue;
                        lList.add(sToken);
                    }
                    aValue = lList;
                }

                aPropSet.put(sChildName, aValue);
            }
        }

        return aPropSet;
    }

    //___________________________________________

    private static java.lang.String getTypeForTag(java.lang.String sTag)
    {
        java.lang.String sType = null;

        if (
            (sTag.equals(Cache.PROPNAME_DATA            )) ||
            (sTag.equals(Cache.PROPNAME_NAME            )) ||
            (sTag.equals(Cache.PROPNAME_UINAME          )) ||
            (sTag.equals(Cache.PROPNAME_MEDIATYPE       )) ||
            (sTag.equals(Cache.PROPNAME_CLIPBOARDFORMAT )) ||
            (sTag.equals(Cache.PROPNAME_PREFERREDFILTER )) ||
            (sTag.equals(Cache.PROPNAME_DETECTSERVICE   )) ||
            (sTag.equals(Cache.PROPNAME_FRAMELOADER     )) ||
            (sTag.equals(Cache.PROPNAME_CONTENTHANDLER  )) ||
            (sTag.equals(Cache.PROPNAME_DOCUMENTSERVICE )) ||
            (sTag.equals(Cache.PROPNAME_FILTERSERVICE   )) ||
            (sTag.equals(Cache.PROPNAME_TEMPLATENAME    )) ||
            (sTag.equals(Cache.PROPNAME_TYPE            )) ||
            (sTag.equals(Cache.PROPNAME_UICOMPONENT     ))
           )
            sType = XMLTYPE_STRING;
        else
        if (
            (sTag.equals(Cache.PROPNAME_PREFERRED   )) ||
            (sTag.equals("Installed"                ))
           )
            sType = XMLTYPE_BOOLEAN;
        else
        if (
            (sTag.equals(Cache.PROPNAME_UIORDER          )) ||
            (sTag.equals(Cache.PROPNAME_DOCUMENTICONID   )) ||
            (sTag.equals(Cache.PROPNAME_FILEFORMATVERSION))
           )
            sType = XMLTYPE_INTEGER;
        else
        if (
            (sTag.equals(Cache.PROPNAME_URLPATTERN  )) ||
            (sTag.equals(Cache.PROPNAME_EXTENSIONS  )) ||
            (sTag.equals(Cache.PROPNAME_USERDATA    )) ||
            (sTag.equals(Cache.PROPNAME_FLAGS       )) ||
            (sTag.equals(Cache.PROPNAME_TYPES       ))
           )
            sType = XMLTYPE_STRINGLIST;

        if (sType == null)
            System.err.println("getTypeForTag("+sTag+") = "+sType);

        return sType;
    }

    //___________________________________________

    /** return a xml representation of the given property set.
     *
     *  @param  aPropSet
     *          a set of <name,value> pairs, which should be translated to xml
     *
     *  @return [java.lang.String]
     *          the xml string representation.
     *
     *  @throws [java.lang.Exception]
     *          if anything during convertion fill fail.
     */
    public static java.lang.String convertPropSetToXML(java.util.HashMap aPropSet   ,
                                                       int               nPrettyTabs)
        throws java.lang.Exception
    {
        java.lang.StringBuffer sXML = new java.lang.StringBuffer(256);

        java.util.Iterator it1 = aPropSet.keySet().iterator();
        while(it1.hasNext())
        {
            java.lang.String sProp = (java.lang.String)it1.next();
            java.lang.Object aVal  = aPropSet.get(sProp);

            sProp = encodeHTMLSigns(sProp);

            // is it a simple type?
            if (
                (aVal instanceof java.lang.Integer) ||
                (aVal instanceof java.lang.Boolean) ||
                (aVal instanceof java.lang.String )
               )
            {
                sXML.append(XMLHelper.convertSimpleObjectToXML(sProp, aVal, nPrettyTabs));
                continue;
            }

            // no!
            // is it a list value?
            if (aVal instanceof java.util.Vector)
            {
                java.util.Vector lVal = (java.util.Vector)aVal;
                sXML.append(XMLHelper.convertListToXML(sProp, lVal, nPrettyTabs));
                continue;
            }

            // its a localized value?
            if (aVal instanceof java.util.HashMap)
            {
                java.util.HashMap lVal = (java.util.HashMap)aVal;
                sXML.append(XMLHelper.convertLocalizedValueToXML(sProp, lVal, nPrettyTabs));
                continue;
            }

            // unknown type!
            java.lang.StringBuffer sMsg = new java.lang.StringBuffer(256);
            sMsg.append("unsupported object type detected.");
            sMsg.append("\ttype ?  : \""+sProp+"\" = "+aVal);
            sMsg.append("\tprop set: \""+aPropSet          );
            throw new java.lang.Exception(sMsg.toString());
        }

        return sXML.toString();
    }

    public static java.lang.String encodeHTMLSigns(java.lang.String sValue)
    {
        java.lang.StringBuffer sSource      = new java.lang.StringBuffer(sValue);
        java.lang.StringBuffer sDestination = new java.lang.StringBuffer(1000  );

        for (int i=0; i<sSource.length(); ++i)
        {
            char c = sSource.charAt(i);
            if (c == '&')
                sDestination.append("&amp;");
            else
                sDestination.append(c);
        }

        java.lang.String sReturn = sDestination.toString();
        if (!sReturn.equals(sValue))
            System.out.println("encode \""+sValue+"\" => \""+sReturn+"\"");

        return sReturn;
    }

    //___________________________________________

    /** return a xml representation of an atomic property.
     *
     *  Atomic property types are e.g. Integer, Boolean, String.
     *
     *  @param  sName
     *          the name of the property.

     *  @param  aValue
     *          the value of the property.
     *
     *  @param  nPrettyTabs
     *          count of tab signs for pretty format the xml code :-)
     *
     *  @return [java.lang.String]
     *          the xml string representation.
     *
     *  @throws [java.lang.Exception]
     *          if anything during convertion fill fail.
     */
    private static java.lang.String convertSimpleObjectToXML(java.lang.String sName      ,
                                                             java.lang.Object aValue     ,
                                                             int              nPrettyTabs)
        throws java.lang.Exception
    {
        java.lang.StringBuffer sXML = new java.lang.StringBuffer(256);
        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");

        if (aValue instanceof java.lang.Integer)
        {
            sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\">");
            sXML.append("<value>"+aValue.toString()+"</value>");
            sXML.append("</prop>\n");
        }
        else
        if (aValue instanceof java.lang.Boolean)
        {
            sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\">");
            sXML.append("<value>"+aValue.toString()+"</value>");
            sXML.append("</prop>\n");
        }
        else
        if (aValue instanceof java.lang.String)
        {
            sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\"");
            java.lang.String sValue = (java.lang.String)aValue;

            sValue = encodeHTMLSigns(sValue);

            if (sValue.length() < 1)
                sXML.append("/>\n");
            else
            {
                sXML.append("><value>"+sValue+"</value>");
                sXML.append("</prop>\n");
            }
        }
        else
        {
            System.err.println("name  = "+sName);
            System.err.println("value = "+aValue);
            // ! can be used outside to detect - that it was not a simple type :-)
            throw new java.lang.Exception("not an atomic type.");
        }

        return sXML.toString();
    }

    //___________________________________________

    /** return a xml representation of a string-list property.
     *
     *  @param  sName
     *          the name of the property.

     *  @param  aValue
     *          the value of the property.
     *
     *  @param  nPrettyTabs
     *          count of tab signs for pretty format the xml code :-)
     *
     *  @return [java.lang.String]
     *          the xml string representation.
     *
     *  @throws [java.lang.Exception]
     *          if anything during convertion fill fail.
     */
    private static java.lang.String convertListToXML(java.lang.String sName      ,
                                                     java.util.Vector aValue     ,
                                                     int              nPrettyTabs)
        throws java.lang.Exception
    {
        java.lang.StringBuffer sXML = new java.lang.StringBuffer(256);

        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");

        int c = aValue.size();
        if (c < 1)
        {
            sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\"/>\n");
            return sXML.toString();
        }

        // step over all list items and add it to a string buffer
        // Every item will be separated by a default separator "\n" first.
        // Because "\n" is not a valid separator at all and can`t occure inside
        // our list items. During we step over all items, we check if our current separator
        // (we use a list of possible ones!) clash with an item.
        // If it clash - we step to the next possible separator.
        // If our list of possible separator values runs out of range we throw
        // an exception :-) Its better then generating of wrong values
        // If we found a valid seperator - we use it to replace our "\n" place holder
        // at the end of the following loop ...

        int                    d        = 0;
        java.lang.StringBuffer sValBuff = new java.lang.StringBuffer(256);
        for (int i=0; i<c; ++i)
        {
            // get the next list item
            java.lang.Object aItem = aValue.get(i);
            if (!(aItem instanceof java.lang.String))
                throw new java.lang.Exception("Current implementation supports string-list only!");

            java.lang.String sValue = (java.lang.String)aItem;

            sValue = encodeHTMLSigns(sValue);

            // append item with default separator, which isn a valid separator at all
            // But supress adding of the separator if last element is reached.
            sValBuff.append(sValue);
            if (i<(c-1))
                sValBuff.append("\n");

            // check for delim clash
            // Attention: An empty (means default) element forbid using
            // of a whitespace character as separator!
            while(true)
            {
                if (d >= DELIMS.length)
                    throw new java.lang.Exception("No valid separator found for a string list item.");
                if (sValue.length() < 1 && DELIMS[d].equals(" "))
                {
                    ++d;
                    continue;
                }
                if (sValue.indexOf(DELIMS[d]) != -1)
                {
                    ++d;
                    continue;
                }
                break;
            }
        }

        // replace default separator with right one
        System.out.println("TODO: must be adapted to java 1.3 :-(");
        System.exit(-1);
//TODO_JAVA        java.lang.String sListVal = sValBuff.toString().replaceAll("\n", DELIMS[d]);
        java.lang.String sListVal = null;

        sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\">");
        if (d == DEFAULT_SEPARATOR)
            sXML.append("<value>");
        else
            sXML.append("<value "+XMLATTRIB_OOR_SEPARATOR+"=\""+DELIMS[d]+"\">");
        sXML.append(sListVal);
        sXML.append("</value>");
        sXML.append("</prop>\n");

        return sXML.toString();
    }

    //___________________________________________

    /** return a xml representation of a localized property.
     *
     *  @param  sName
     *          the name of the property.

     *  @param  aValue
     *          the value of the property.
     *
     *  @param  nPrettyTabs
     *          count of tab signs for pretty format the xml code :-)
     *
     *  @return [java.lang.String]
     *          the xml string representation.
     *
     *  @throws [java.lang.Exception]
     *          if anything during convertion fill fail.
     */
    private static java.lang.String convertLocalizedValueToXML(java.lang.String  sName      ,
                                                               java.util.HashMap aValue     ,
                                                               int               nPrettyTabs)
        throws java.lang.Exception
    {
        java.lang.StringBuffer sXML = new java.lang.StringBuffer(256);

        int c = aValue.size();
        if (c < 1)
            throw new java.lang.Exception("Cant detect type of localized values. Because the given list is empty.");

        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");
        // !Our localized values must be formated at a deeper coloum
        // then its property name!
        ++nPrettyTabs;

        sXML.append("<prop "+XMLATTRIB_OOR_NAME+"=\""+sName+"\">\n");
        java.util.Iterator it = aValue.keySet().iterator();
//        boolean bTypeKnown = false;
        while(it.hasNext())
        {
            java.lang.String sLocale = (java.lang.String)it.next();
            java.lang.Object aLocalizedValue = aValue.get(sLocale);
/*
            if (!bTypeKnown)
            {
                bTypeKnown = true;
                if (aLocalizedValue instanceof java.lang.Integer)
                    sXML.append(" "+XMLATTRIB_OOR_TYPE+"=\""+XMLTYPE_INTEGER+"\">\n");
                else
                if (aLocalizedValue instanceof java.lang.Boolean)
                    sXML.append(" "+XMLATTRIB_OOR_TYPE+"=\""+XMLTYPE_BOOLEAN+"\">\n");
                else
                if (aLocalizedValue instanceof java.lang.String)
                    sXML.append(" "+XMLATTRIB_OOR_TYPE+"=\""+XMLTYPE_STRING+"\">\n");
                else
                    throw new java.lang.Exception("Unsupported type for localized value detected.");
            }
*/
            java.lang.String sLocValue = aLocalizedValue.toString();
            java.lang.String sValue    = encodeHTMLSigns(sLocValue);

            for (int t=0; t<nPrettyTabs; ++t)
                sXML.append("\t");
            sXML.append("<value "+XMLATTRIB_XML_LANG+"=\""+sLocale+"\">"+sValue+"</value>\n");
        }
        --nPrettyTabs;
        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");
        sXML.append("</prop>\n");

        return sXML.toString();
    }

    //___________________________________________

    /** returns the value of an attribute of the given node.
     *
     *  If the given node represent an lement node, may it supports some attributes.
     *  Then this method search for an attribute with the specified name and return it's value.
     *  If nothing could be found ... or the given node isn't a suitable node ... it returns null.
     *
     *  @param  aNode
     *          the node, which should be analyzed.
     *
     *  @param  sAttrib
     *          name of the attribute, which should be searched.
     *
     *  @return The value of the specified attribute if it could be found at the given node.
     *          Can be null if node doesn't support attributes or the searched one does not exist there.
     */
    public static java.lang.String extractNodeAttribByName(org.w3c.dom.Node aNode  ,
                                                           java.lang.String sAttrib)
        throws java.lang.Exception
    {
        // We can get valid attributes for element nodes only!
        if (aNode.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
        {
//            System.err.println("not an element node");
            return null;
        }

        // may it supports attributes in general ... but doesn't have anyone realy.
        org.w3c.dom.NamedNodeMap lAttribs = aNode.getAttributes();
        if (lAttribs==null)
        {
//            System.err.println("no attributes at all");
            return null;
        }

        // step over the attribute list and search for the requested one
        for (int i=0; i<lAttribs.getLength(); ++i)
        {
            org.w3c.dom.Attr aAttrib = (org.w3c.dom.Attr)lAttribs.item(i);
            if (aAttrib.getName().equals(sAttrib))
            {
                java.lang.String sValue = aAttrib.getValue();
                return sValue;
            }
        }

        // the searched attribute was not found!
//        System.err.println("required attribute \""+sAttrib+"\" does not exist for node ["+aNode.toString()+"]");
        return null;
    }

    //___________________________________________

    /** returns a list of children, which are ELEMENT_NODES and have the right tag name.
     *
     *  It analyze the list of all possible child nodes. Only ELEMENT_NODES are candidates.
     *  All other ones will be ignored. Further these element nodes are compared by it's tag
     *  names. If it match with the specified value it's added to the return list.
     *  So the return list includes references to the DOM tree nodes only, which are child
     *  element nodes with the right tag name.
     *
     *  @param  aNode
     *          provides access to the child nodes, which should be analyzed
     *
     *  @param  sTag
     *          the searched tag name.
     *
     * @return  A list of child nodes, which are element nodes and have the right tag name.
     */
    public static java.util.Vector extractChildNodesByTagName(org.w3c.dom.Node aNode,
                                                               java.lang.String sTag )
    {
        // extract first all ELEMENT_NODES of he given parent
        // Such nodes only provide tag names.
        java.util.Vector lChildren          = XMLHelper.extractChildNodesByType(aNode,org.w3c.dom.Node.ELEMENT_NODE);
        java.util.Vector lExtractedChildren = new java.util.Vector(lChildren.size());

        // step over the list and search for the right tags using the specified name
        java.util.Enumeration en = lChildren.elements();
        while (en.hasMoreElements())
        {
            org.w3c.dom.Node aChild = (org.w3c.dom.Node)en.nextElement();
            if (aChild.getNodeName().equals(sTag))
                lExtractedChildren.add(aChild);
        }

        // pack(!) and return the list
        lExtractedChildren.trimToSize();
        return lExtractedChildren;
    }

    //___________________________________________

    /** returns a list of children, which supports the right node type.
     *
     *  It analyze the list of all possible child nodes. If a node represent the right node type
     *  it is added to the return list. Otherwhise it will be ignored.
     *
     *  @param  aNode
     *          provides access to the list of possible children nodes.
     *
     *  @param  nType
     *          represent the searched node type.
     *          Possible values are constant fields of a org.w3c.dom.Node - e.g. org.w3c.dom.Node.ELEMENT_NODE.
     *
     * @return A list of child nodes, which provides the right node type.
     */
    public static java.util.Vector extractChildNodesByType(org.w3c.dom.Node aNode,
                                                           short            nType)
    {
        // get list of all possibe children and reserve enough space for our return list
        // Attention: A null pointer is not allowed for return! (means lExtractedChildren)
        org.w3c.dom.NodeList lChildren          = aNode.getChildNodes();
        int                  c                = lChildren.getLength();
        java.util.Vector     lExtractedChildren = new java.util.Vector(c);

        // step of these children and select only needed ones
        for (int i=0; i<c; ++i)
        {
            org.w3c.dom.Node aChild = lChildren.item(i);
            if (aChild.getNodeType() == nType)
                lExtractedChildren.add(aChild);
        }

        // pack(!) and return the list
        lExtractedChildren.trimToSize();
        return lExtractedChildren;
    }

    //___________________________________________

    /** generates an xml header, using parameters.
     *
     *  @param  sVersion
     *          number of the xml version.
     *
     *  @param  sEncoding
     *          used file encoding.
     *
     *  @param  sPath
     *          name of the configuration root.
     *
     *  @param  sPackage
     *          name of the configuration package.
     *
     *  @param  bLanguagepack
     *          force creation of a special header,
     *          which is needed for language packs only.
     *
     *  @return [java.lang.String]
     *          the generated xml header.

*/
    public static java.lang.String generateHeader(java.lang.String sVersion     ,
                                                  java.lang.String sEncoding    ,
                                                  java.lang.String sPath        ,
                                                  java.lang.String sPackage     ,
                                                  boolean          bLanguagePack)
    {
        java.lang.StringBuffer sHeader = new java.lang.StringBuffer(256);

        if (bLanguagePack)
        {
            sHeader.append("<?xml version=\"");
            sHeader.append(sVersion);
            sHeader.append("\" encoding=\"");
            sHeader.append(sEncoding);
            sHeader.append("\"?>\n");
            sHeader.append("<oor:component-data oor:package=\"");
            sHeader.append(sPath);
            sHeader.append("\" oor:name=\"");
            sHeader.append(sPackage);
            sHeader.append("\" xmlns:install=\"http://openoffice.org/2004/installation\"");
            sHeader.append(" xmlns:oor=\"http://openoffice.org/2001/registry\"");
            sHeader.append(" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"");
            sHeader.append(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
        }
        else
        {
            sHeader.append("<?xml version=\"");
            sHeader.append(sVersion);
            sHeader.append("\" encoding=\"");
            sHeader.append(sEncoding);
            sHeader.append("\"?>\n");
            sHeader.append("<oor:component-data xmlns:oor=\"http://openoffice.org/2001/registry\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" oor:package=\"");
            sHeader.append(sPath);
            sHeader.append("\" oor:name=\"");
            sHeader.append(sPackage);
            sHeader.append("\">\n");
        }

        return sHeader.toString();
    }

    public static java.lang.String generateFooter()
    {
        return "</oor:component-data>\n";
    }
}

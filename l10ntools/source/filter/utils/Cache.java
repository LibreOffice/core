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
 *  It implements a container for all possible entries which are part of the type
 *  and filter mechanism of an office - means all items of the configuration file
 *  "TypeDetection". How these entries will be readed or written can be switch
 *  in different modes. That would make it possible to edit an xml directly or
 *  to contact a running office instance.
 *
 *
 */
public class Cache
{


    /** identifies a file type item of this cache. */
    public static final int E_TYPE = 0;

    /** identifies a filter item of this cache. */
    public static final int E_FILTER = 1;

    /** identifies a detect service item of this cache. */
    public static final int E_DETECTSERVICE = 2;

    /** identifies a frame loader item of this cache. */
    public static final int E_FRAMELOADER = 3;

    /** identifies a content handler item of this cache. */
    public static final int E_CONTENTHANDLER = 4;

    /** indicates an unsupported xml format => error! */
    public static final int FORMAT_UNSUPPORTED = -1;

    /** identify the configuration format of an office 6.0.
     *  The own formated data string is used. */
    public static final int FORMAT_60 = 0;

    /** identify the configuration format of an office 6.y.
     *  Properties are realy xml tags again. */
    public static final int FORMAT_6Y = 1;

    /** identify the configuration format which is used inside
     *  this tooling project. */
    public static final int FORMAT_INTERNAL = 2;

    /** right value for a command line parameter to define a 6.0 version. */
    public static final java.lang.String CMDVAL_FORMAT_60 = "6.0";

    /** right value for a command line parameter to define a 6.Y version. */
    public static final java.lang.String CMDVAL_FORMAT_6Y = "6.Y";

    /** right value for a command line parameter to define an internal xml version! */
    public static final java.lang.String CMDVAL_FORMAT_INTERNAL = "internal";

    // general
    public static final java.lang.String PROPNAME_DATA = "Data";
    public static final java.lang.String PROPNAME_NAME = "Name";
    public static final java.lang.String PROPNAME_UINAME = "UIName";
    public static final java.lang.String PROPNAME_UINAMES = "UINames";

    // type 6.0 ...
    public static final java.lang.String PROPNAME_MEDIATYPE = "MediaType";
    public static final java.lang.String PROPNAME_PREFERRED = "Preferred";
    public static final java.lang.String PROPNAME_CLIPBOARDFORMAT = "ClipboardFormat";
    public static final java.lang.String PROPNAME_DOCUMENTICONID = "DocumentIconID";
    public static final java.lang.String PROPNAME_URLPATTERN = "URLPattern";
    public static final java.lang.String PROPNAME_EXTENSIONS = "Extensions";
    // ... +6.y
    public static final java.lang.String PROPNAME_UIORDER = "UIOrder";
    public static final java.lang.String PROPNAME_PREFERREDFILTER = "PreferredFilter";
    public static final java.lang.String PROPNAME_DETECTSERVICE = "DetectService";
    public static final java.lang.String PROPNAME_FRAMELOADER = "FrameLoader";
    public static final java.lang.String PROPNAME_CONTENTHANDLER = "ContentHandler";

    // filter
    public static final java.lang.String PROPNAME_DOCUMENTSERVICE = "DocumentService";
    public static final java.lang.String PROPNAME_FILEFORMATVERSION = "FileFormatVersion";
    public static final java.lang.String PROPNAME_FILTERSERVICE = "FilterService";
    public static final java.lang.String PROPNAME_FLAGS = "Flags";
    public static final java.lang.String PROPNAME_ORDER = "Order"; // -6.y
    public static final java.lang.String PROPNAME_TEMPLATENAME = "TemplateName";
    public static final java.lang.String PROPNAME_TYPE = "Type";
    public static final java.lang.String PROPNAME_UICOMPONENT = "UIComponent";
    public static final java.lang.String PROPNAME_USERDATA = "UserData";

    // frame loader / detect services / content handler
    public static final java.lang.String PROPNAME_TYPES = "Types";

    //___________________________________________
    // private const

    private static final java.lang.String FILTERSERVICE_NATIVEWARPPER = "com.sun.star.filter.NativeFilterWrapper";
    private static final java.lang.String GENERIC_DETECTSERVICE = "com.sun.star.comp.office.FrameLoader";

    /** its the name of the cfg set, which contains all types. */
    private static final java.lang.String CFGNODE_TYPES = "Types";

    /** its the name of the cfg set, which contains all filters. */
    private static final java.lang.String CFGNODE_FILTERS = "Filters";

    /** its the name of the cfg set, which contains all detect services. */
    private static final java.lang.String CFGNODE_DETECTSERVICES = "DetectServices";

    /** its the name of the cfg set, which contains all frame loaders. */
    private static final java.lang.String CFGNODE_FRAMELOADERS = "FrameLoaders";

    /** its the name of the cfg set, which contains all content handlers. */
    private static final java.lang.String CFGNODE_CONTENTHANDLERS = "ContentHandlers";

    // names for filter flags
    private static final java.lang.String FLAGNAME_3RDPARTYFILTER   = "3RDPARTYFILTER";
    private static final java.lang.String FLAGNAME_ALIEN            = "ALIEN";
    private static final java.lang.String FLAGNAME_ASYNCHRON        = "ASYNCHRON";
    private static final java.lang.String FLAGNAME_BROWSERPREFERRED = "BROWSERPREFERRED";
    private static final java.lang.String FLAGNAME_CONSULTSERVICE   = "CONSULTSERVICE";
    private static final java.lang.String FLAGNAME_DEFAULT          = "DEFAULT";
    private static final java.lang.String FLAGNAME_EXPORT           = "EXPORT";
    private static final java.lang.String FLAGNAME_IMPORT           = "IMPORT";
    private static final java.lang.String FLAGNAME_INTERNAL         = "INTERNAL";
    private static final java.lang.String FLAGNAME_NOTINCHOOSER     = "NOTINCHOOSER";
    private static final java.lang.String FLAGNAME_NOTINFILEDIALOG  = "NOTINFILEDIALOG";
    private static final java.lang.String FLAGNAME_NOTINSTALLED     = "NOTINSTALLED";
    private static final java.lang.String FLAGNAME_OWN              = "OWN";
    private static final java.lang.String FLAGNAME_PACKED           = "PACKED";
    private static final java.lang.String FLAGNAME_PREFERRED        = "PREFERRED";
    private static final java.lang.String FLAGNAME_READONLY         = "READONLY";
    private static final java.lang.String FLAGNAME_TEMPLATE         = "TEMPLATE";
    private static final java.lang.String FLAGNAME_TEMPLATEPATH     = "TEMPLATEPATH";
    private static final java.lang.String FLAGNAME_USESOPTIONS      = "USESOPTIONS";

    private static final java.lang.String FLAGNAME_COMBINED         = "COMBINED";
    private static final java.lang.String FLAGNAME_SUPPORTSSELECTION= "SUPPORTSSELECTION";

    // values for filter flags
    private static final int FLAGVAL_3RDPARTYFILTER    = 0x00080000; // 524288
    private static final int FLAGVAL_ALIEN             = 0x00000040; // 64
    private static final int FLAGVAL_ALL               = 0xffffffff; // 4294967295
    private static final int FLAGVAL_ASYNCHRON         = 0x00004000; // 16384
    private static final int FLAGVAL_BROWSERPREFERRED  = 0x00400000; // 4194304
    private static final int FLAGVAL_CONSULTSERVICE    = 0x00040000; // 262144
    private static final int FLAGVAL_DEFAULT           = 0x00000100; // 256
    private static final int FLAGVAL_EXPORT            = 0x00000002; // 2
    private static final int FLAGVAL_IMPORT            = 0x00000001; // 1
    private static final int FLAGVAL_INTERNAL          = 0x00000008; // 8
    private static final int FLAGVAL_NOTINCHOOSER      = 0x00002000; // 8192
    private static final int FLAGVAL_NOTINFILEDIALOG   = 0x00001000; // 4096
    private static final int FLAGVAL_NOTINSTALLED      = 0x00020000; // 131072
    private static final int FLAGVAL_OWN               = 0x00000020; // 32
    private static final int FLAGVAL_PACKED            = 0x00100000; // 1048576
    private static final int FLAGVAL_PREFERRED         = 0x10000000; // 268435456
    private static final int FLAGVAL_READONLY          = 0x00010000; // 65536
    private static final int FLAGVAL_TEMPLATE          = 0x00000004; // 4
    private static final int FLAGVAL_TEMPLATEPATH      = 0x00000010; // 16
    private static final int FLAGVAL_USESOPTIONS       = 0x00000080; // 128

    private static final int FLAGVAL_COMBINED          = 0x00800000; // ...
    private static final int FLAGVAL_SUPPORTSSELECTION = 0x00000400; // 1024

    //___________________________________________
    // member

    /** list of all located types.
     *  Format: [string,HashMap]
     */
    private java.util.HashMap m_lTypes;

    /** list of all located filters.
     *  Format: [string,HashMap]
     */
    private java.util.HashMap m_lFilters;

    /** list of all located detect services.
     *  Format: [string,HashMap]
     */
    private java.util.HashMap m_lDetectServices;

    /** list of all located frame loader.
     *  Format: [string,HashMap]
     */
    private java.util.HashMap m_lFrameLoaders;

    /** list of all located content handler.
     *  Format: [string,HashMap]
     */
    private java.util.HashMap m_lContentHandlers;

    /** contains all analyzed relations between
     *  filters and types. The key is an internal
     *  type name (can be used as reference into the
     *  list m_lTypes) and the value is a Vector of all
     *  internal filter names, which are registered for
     *  this type.
     *  Format: [string, Vector]
     */
    private java.util.HashMap m_lFilterToTypeRegistrations;

    private int m_nDoubleRegisteredFilters;
    private int m_nTypesForFilters;
    private int m_nTypesForDetectServices;
    private int m_nTypesForFrameLoaders;
    private int m_nTypesForContentHandlers;

    /** can be used to log different informations. */
    private Logger m_aDebug;

    //___________________________________________
    // interface

    /** standard ctor.
     *
     *  Initialize an empty cache instance. You have to use
     *  on of the fromXXX() methods to fill it from different
     *  sources with content.
     */
    public Cache(Logger aDebug)
    {
        reset();
        m_aDebug = aDebug;
    }

    //___________________________________________

    /** free memory and set default values on all members.
     */
    public synchronized void reset()
    {
        m_lTypes                     = new java.util.HashMap();
        m_lFilters                   = new java.util.HashMap();
        m_lFrameLoaders              = new java.util.HashMap();
        m_lDetectServices            = new java.util.HashMap();
        m_lContentHandlers           = new java.util.HashMap();
        m_lFilterToTypeRegistrations = new java.util.HashMap();
        m_aDebug                     = new Logger();
        m_nDoubleRegisteredFilters   = 0;
        m_nTypesForFilters           = 0;
        m_nTypesForDetectServices    = 0;
        m_nTypesForFrameLoaders      = 0;
        m_nTypesForContentHandlers   = 0;
    }

    //___________________________________________

    /** converts a string representation of an xml format
     *  to its int value, which must be used at some interface
     *  methods of this cache.
     *
     *  If the given string does not match to any well known format,
     *  the return value will be FORMAT_UNSUPPORTED. The calli have to
     *  check that. Otherwhise a called interface method at this cache
     *  instance will be rejected by an exception!
     *
     *  @param  sFormat
     *          the string representation
     *          Must be one of our public const values from type CMDVAL_FORMAT_xxx.
     *
     *  @return [int]
     *          the int representation.
     *          Will be one of our public const values from type FORMAT_xxx.
     */
    public static int mapFormatString2Format(java.lang.String sFormat)
    {
        int nFormat = FORMAT_UNSUPPORTED;
        if (sFormat.equalsIgnoreCase(CMDVAL_FORMAT_60))
            nFormat = FORMAT_60;
        else
        if (sFormat.equalsIgnoreCase(CMDVAL_FORMAT_6Y))
            nFormat = FORMAT_6Y;
        else
        if (sFormat.equalsIgnoreCase(CMDVAL_FORMAT_INTERNAL))
            nFormat = FORMAT_INTERNAL;
        return nFormat;
    }

    //___________________________________________

    /** return some statistic values.
     *
     *  Such values can be: - count of container items,
     *                      - ...
     *
     *  @return [java.lang.String]
     *          a formated string, which contains al statistic data.
     */
    public synchronized java.lang.String getStatistics()
    {
        java.lang.StringBuffer sBuffer = new java.lang.StringBuffer(256);

        sBuffer.append("types                          = "+m_lTypes.size()           +"\n");
        sBuffer.append("filters                        = "+m_lFilters.size()         +"\n");
        sBuffer.append("detect services                = "+m_lDetectServices.size()  +"\n");
        sBuffer.append("frame loaders                  = "+m_lFrameLoaders.size()    +"\n");
        sBuffer.append("content handler                = "+m_lContentHandlers.size() +"\n");
        sBuffer.append("double registered filters      = "+m_nDoubleRegisteredFilters+"\n");
        sBuffer.append("types used by filters          = "+m_nTypesForFilters        +"\n");
        sBuffer.append("types used by detect services  = "+m_nTypesForDetectServices +"\n");
        sBuffer.append("types used by frame loaders    = "+m_nTypesForFrameLoaders   +"\n");
        sBuffer.append("types used by content handlers = "+m_nTypesForContentHandlers+"\n");

        return sBuffer.toString();
    }

    //___________________________________________

    /** reset this cache and fill it with new values using the given XML file.
     *
     *  @param  aXML
     *          must be a system file of a suitable XML file, which
     *          include all neccessary type/filter items.
     *
     *  @param  nFormat
     *          identifies the format of the specified xml file,
     *          which must be interpreted.
     */
    public synchronized void fromXML(java.io.File aXML   ,
                                     int          nFormat)
        throws java.lang.Exception
    {
        // clear this cache
        reset();

        // parse it
        javax.xml.parsers.DocumentBuilderFactory aFactory = javax.xml.parsers.DocumentBuilderFactory.newInstance();
        /* Attention:
         *  This call is important. It force right handling of entities during parsing and(!)
         *  writing. It let all possible signs for entities or it's quoted representations
         *  untouched. So this class don't change the original signs of the original file.
         *  Means:
         *      <ul>
         *          <li>(') => (')</li>
         *          <li>(") => (")</li>
         *          <li>(>) => (&gt;)</li>
         *          <li>(<) => (&lt;)</li>
         *          <li>(&gt;) => (&gt;)</li>
         *          <li>(&amp;) => (&amp;)</li>
         *          <li>...</li>
         *      </ul>
         */

        System.out.println("TODO: must be adapted to java 1.3 :-(");
        System.exit(-1);
//TODO_JAVA        aFactory.setExpandEntityReferences(false);

        javax.xml.parsers.DocumentBuilder aBuilder = aFactory.newDocumentBuilder();
        org.w3c.dom.Document              aDOM     = aBuilder.parse(aXML);
        org.w3c.dom.Element               aRoot    = aDOM.getDocumentElement();

        // step over all sets
        java.util.Vector      lSetNodes = XMLHelper.extractChildNodesByTagName(aRoot, XMLHelper.XMLTAG_NODE);
        java.util.Enumeration it1       = lSetNodes.elements();
        while (it1.hasMoreElements())
        {
            // try to find out, which set should be read
            org.w3c.dom.Node aSetNode = (org.w3c.dom.Node)it1.nextElement();
            java.lang.String sSetName = XMLHelper.extractNodeAttribByName(aSetNode, XMLHelper.XMLATTRIB_OOR_NAME);
            if (sSetName == null)
                throw new java.io.IOException("unsupported format: could not extract set name on node ...\n"+aSetNode);

            // map some generic interfaces to the right members!
            int               eType = -1  ;
            java.util.HashMap rMap  = null;

            if (sSetName.equals(CFGNODE_TYPES))
            {
                eType = E_TYPE;
                rMap  = m_lTypes;
            }
            else
            if (sSetName.equals(CFGNODE_FILTERS))
            {
                eType = E_FILTER;
                rMap  = m_lFilters;
            }
            else
            if (sSetName.equals(CFGNODE_FRAMELOADERS))
            {
                eType = E_FRAMELOADER;
                rMap  = m_lFrameLoaders;
            }
            else
            if (sSetName.equals(CFGNODE_DETECTSERVICES))
            {
                eType = E_DETECTSERVICE;
                rMap  = m_lDetectServices;
            }
            else
            if (sSetName.equals(CFGNODE_CONTENTHANDLERS))
            {
                eType = E_CONTENTHANDLER;
                rMap  = m_lContentHandlers;
            }
            else
                throw new java.io.IOException("unsupported format: unknown set name [\""+sSetName+"\"] detected on node ...\n"+aSetNode);

            // load all set entries
            java.util.Vector      lChildNodes = XMLHelper.extractChildNodesByTagName(aSetNode, XMLHelper.XMLTAG_NODE);
            java.util.Enumeration it2         = lChildNodes.elements();
            while (it2.hasMoreElements())
            {
                org.w3c.dom.Node aChildNode = (org.w3c.dom.Node)it2.nextElement();
                java.lang.String sChildName = XMLHelper.extractNodeAttribByName(aChildNode, XMLHelper.XMLATTRIB_OOR_NAME);
                if (sChildName == null)
                    throw new java.io.IOException("unsupported format: could not extract child node name on node ...\n"+aChildNode);
                java.util.HashMap aPropSet = null;

                // Note: Our internal format is different from the source format!
                java.util.HashMap aTempSet = XMLHelper.convertNodeToPropSet(aChildNode);
                switch(eType)
                {
                    case E_TYPE :
                    {
                        aPropSet = Cache.convertTypePropsToInternal(aTempSet, nFormat);
                        m_aDebug.setDetailedInfo("type [\""+sChildName+"\"] converted to internal format");
                    }
                    break;

                    case E_FILTER :
                    {
                        aPropSet = Cache.convertFilterPropsToInternal(aTempSet, nFormat);
                        m_aDebug.setDetailedInfo("filter [\""+sChildName+"\"] converted to internal format");
                    }
                    break;

                    case E_DETECTSERVICE :
                    {
                        aPropSet = Cache.convertDetectServicePropsToInternal(aTempSet, nFormat);
                        m_aDebug.setDetailedInfo("detect service [\""+sChildName+"\"] converted to internal format");
                    }
                    break;

                    case E_FRAMELOADER :
                    {
                        aPropSet = Cache.convertFrameLoaderPropsToInternal(aTempSet, nFormat);
                        m_aDebug.setDetailedInfo("frame loader [\""+sChildName+"\"] converted to internal format");
                    }
                    break;

                    case E_CONTENTHANDLER :
                    {
                        aPropSet = Cache.convertContentHandlerPropsToInternal(aTempSet, nFormat);
                        m_aDebug.setDetailedInfo("content handler [\""+sChildName+"\"] converted to internal format");
                    }
                    break;
                }
                m_aDebug.setDetailedInfo("props = "+aTempSet);
                rMap.put(sChildName, aPropSet);
            }
        }
    }

    //___________________________________________

    /** create some hml views of the current content of this cache.
     *
     *  The given directory is used to create different html files
     *  there. Every of them show another aspect of this cache.
     *  E.g.: - all type/filter properties
     *        - relation ships between types/filters/loaders etc.
     *
     *  @param  aDirectory
     *          points to a system directory, which
     *          can be used completely(!) to generate
     *          the results there.
     *
     *  @param  nFormat
     *          specify in which context the cache items should be
     *          interpreted.
     */
    public synchronized void toHTML(java.io.File     aDirectory,
                                    int              nFormat   ,
                                    java.lang.String sEncoding )
        throws java.lang.Exception
    {
        if (nFormat != FORMAT_6Y)
            throw new java.lang.Exception("HTML views are supported for the new 6.y format only yet.");

        java.lang.StringBuffer sRelationView = new java.lang.StringBuffer(1000);
        sRelationView.append("<html><header><title>Relation View</title></header><body>");
        sRelationView.append("<table border=1>");
        sRelationView.append("<tr><td><b>type</b></td><td><b>detect service</b></td><td><b>preferred filter</b></td><td><b>frame loader</b></td><td><b>content handler</b></td></tr>");

        java.util.Iterator aIt = m_lTypes.keySet().iterator();
        while (aIt.hasNext())
        {
            java.lang.String  sType = (java.lang.String)aIt.next();
            java.util.HashMap aType = (java.util.HashMap)m_lTypes.get(sType);

            sRelationView.append("<tr>");
            sRelationView.append("<td>"+sType+"</td>");

            java.lang.String sVal = (java.lang.String)aType.get(PROPNAME_DETECTSERVICE);
            if (sVal == null || sVal.length()<1)
                sRelationView.append("<td> - </td>");
            else
                sRelationView.append("<td>"+sVal+"</td>");

            sVal = (java.lang.String)aType.get(PROPNAME_PREFERREDFILTER);
            if (sVal == null || sVal.length()<1)
                sRelationView.append("<td> - </td>");
            else
                sRelationView.append("<td>"+sVal+"</td>");

            sVal = (java.lang.String)aType.get(PROPNAME_FRAMELOADER);
            if (sVal == null || sVal.length()<1)
                sRelationView.append("<td> - </td>");
            else
                sRelationView.append("<td>"+sVal+"</td>");

            sVal = (java.lang.String)aType.get(PROPNAME_CONTENTHANDLER);
            if (sVal == null || sVal.length()<1)
                sRelationView.append("<td> - </td>");
            else
                sRelationView.append("<td>"+sVal+"</td>");

            sRelationView.append("</tr>");
        }

        sRelationView.append("</table>");
        sRelationView.append("</body>");

        FileHelper.writeEncodedBufferToFile(new java.io.File(aDirectory, "relation_view.html"), sEncoding, false, sRelationView);

        java.util.HashMap lFilters2TypeRegistration = new java.util.HashMap();
        aIt = m_lFilters.keySet().iterator();
        while (aIt.hasNext())
        {
            java.lang.String  sFilter = (java.lang.String)aIt.next();
            java.util.HashMap aFilter = (java.util.HashMap)m_lFilters.get(sFilter);
            java.lang.String  sType   = (java.lang.String)aFilter.get(PROPNAME_TYPE);

            java.util.Vector lFilters = (java.util.Vector)lFilters2TypeRegistration.get(sType);
            if (lFilters == null)
                lFilters = new java.util.Vector();
            lFilters.add(sFilter);
            lFilters2TypeRegistration.put(sType, lFilters);
        }

        java.lang.StringBuffer sType2FiltersView = new java.lang.StringBuffer(1000);
        sType2FiltersView.append("<html><header><title>Type2Filters View</title></header><body>");
        sType2FiltersView.append("<table border=1>");
        sType2FiltersView.append("<tr><td><b>type</b></td><td><b>filters</b></td></tr>");

        aIt = lFilters2TypeRegistration.keySet().iterator();
        while (aIt.hasNext())
        {
            java.lang.String sType    = (java.lang.String)aIt.next();
            java.util.Vector lFilters = (java.util.Vector)lFilters2TypeRegistration.get(sType);

            sType2FiltersView.append("<tr><td>"+sType+"</td><td>");
            java.util.Enumeration aEn = lFilters.elements();
            while(aEn.hasMoreElements())
                sType2FiltersView.append(aEn.nextElement()+"<br>");
            sType2FiltersView.append("</td></tr>");
        }

        sType2FiltersView.append("</table>");
        sType2FiltersView.append("</body>");

        FileHelper.writeEncodedBufferToFile(new java.io.File(aDirectory, "type2filters_view.html"), sEncoding, false, sType2FiltersView);
    }

    //___________________________________________

    /** converts all items of this cache to its xml representation
     *  and write it to the given file.
     *
     *  @param  aXML
     *          the target file for output.
     *
     *  @param  nFormat
     *          the requested xml format.
     *          see const values FORMAT_xxx too.
     *
     *  @param  sEncoding
     *          specify the file encoding for the generated xml file.
     *
     *  @throws [java.lang.Exception]
     *          if something fail during convertion.
     */
    public synchronized void toXML(java.io.File     aXML     ,
                                   int              nFormat  ,
                                   java.lang.String sEncoding)
        throws java.lang.Exception
    {
        java.lang.StringBuffer sXML = new java.lang.StringBuffer(500000);

        for (int i=0; i<5; ++i)
        {
            // define right sub container
            java.lang.String  sSetName = null;
            java.util.HashMap rMap     = null;
            int               eType    = -1;

            switch(i)
            {
                case 0 :
                {
                    sSetName = CFGNODE_TYPES;
                    rMap     = m_lTypes;
                    eType    = E_TYPE;
                }
                break;

                case 1 :
                {
                    sSetName = CFGNODE_FILTERS;
                    rMap     = m_lFilters;
                    eType    = E_FILTER;
                }
                break;

                case 2 :
                {
                    sSetName = CFGNODE_DETECTSERVICES;
                    rMap     = m_lDetectServices;
                    eType    = E_DETECTSERVICE;
                }
                break;

                case 3 :
                {
                    sSetName = CFGNODE_FRAMELOADERS;
                    rMap     = m_lFrameLoaders;
                    eType    = E_FRAMELOADER;
                }
                break;

                case 4 :
                {
                    sSetName = CFGNODE_CONTENTHANDLERS;
                    rMap     = m_lContentHandlers;
                    eType    = E_CONTENTHANDLER;
                }
                break;
            }

            // generate set
            sXML.append("<node oor:name=\""+sSetName+"\" oor:op=\"replace\">\n");
            java.util.Iterator it = rMap.keySet().iterator();
            while(it.hasNext())
            {
                java.lang.String sItem = (java.lang.String)it.next();
                sXML.append("<node oor:name=\""+sItem+"\" oor:op=\"replace\">\n");
                sXML.append(getItemAsXML(eType, sItem, nFormat));
                sXML.append("</node>\n");
            }
            sXML.append("</node>\n");
        }

        java.io.FileOutputStream   aStream = new java.io.FileOutputStream(aXML.getAbsolutePath(), false);
        java.io.OutputStreamWriter aWriter = new java.io.OutputStreamWriter(aStream, sEncoding);
        java.lang.String           sOut    = sXML.toString();
        aWriter.write(sOut, 0, sOut.length());
        aWriter.flush();
        aWriter.close();
    }

    //___________________________________________

    /** converts a type property set from internal format
     *  to an external one.
     *
     *  @param  aMap
     *          points to the item, which should be converted.
     *
     *  @param  nFormat
     *          specify the requested output format.
     *
     *  @return [java.util.HashMap]
     *          contains the properties in the requested format.
     *
     *  @throws [java.lang.Exception
     *          if something fail during convertion.
     */
    private static java.util.HashMap convertTypePropsToExternal(java.util.HashMap aMap   ,
                                                                int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = new java.util.HashMap();
        // copy Name property ... if it exists!
        if (aMap.containsKey(PROPNAME_NAME))
            aResultMap.put(PROPNAME_NAME, aMap.get(PROPNAME_NAME));
        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // copy UIName property unchanged
                aResultMap.put(PROPNAME_UINAME, aMap.get(PROPNAME_UINAME));

                // ignore properties "UIOrder", "PreferredFilter", "DetectService"
                // They are not supported for 6.0 types.

                // pack all other properties to one "Data" string value
                java.lang.StringBuffer sData = new java.lang.StringBuffer(256);

                sData.append(aMap.get(PROPNAME_PREFERRED));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_MEDIATYPE));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_CLIPBOARDFORMAT));
                sData.append(",");

                java.util.Vector lList = (java.util.Vector)aMap.get(PROPNAME_URLPATTERN);
                int              c     = lList.size();
                int              i     = 0;
                for (i=0; i<c; ++i)
                {
                    sData.append(lList.elementAt(i));
                    if (i<(c-1))
                        sData.append(";");
                }

                lList = (java.util.Vector)aMap.get(PROPNAME_EXTENSIONS);
                c     = lList.size();
                for (i=0; i<c; ++i)
                {
                    sData.append(lList.elementAt(i));
                    if (i<(c-1))
                        sData.append(";");
                }

                sData.append(",");
                sData.append(aMap.get(PROPNAME_DOCUMENTICONID));
                sData.append(",");

                aResultMap.put(PROPNAME_DATA, sData.toString());
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // copy all supported properties directly
                aResultMap.put(PROPNAME_PREFERRED      , aMap.get(PROPNAME_PREFERRED      ));
                aResultMap.put(PROPNAME_MEDIATYPE      , aMap.get(PROPNAME_MEDIATYPE      ));
                aResultMap.put(PROPNAME_URLPATTERN     , aMap.get(PROPNAME_URLPATTERN     ));
                aResultMap.put(PROPNAME_EXTENSIONS     , aMap.get(PROPNAME_EXTENSIONS     ));
                aResultMap.put(PROPNAME_UINAME         , aMap.get(PROPNAME_UINAME         ));
                aResultMap.put(PROPNAME_PREFERREDFILTER, aMap.get(PROPNAME_PREFERREDFILTER));
                aResultMap.put(PROPNAME_DETECTSERVICE  , aMap.get(PROPNAME_DETECTSERVICE  ));
                aResultMap.put(PROPNAME_CLIPBOARDFORMAT, aMap.get(PROPNAME_CLIPBOARDFORMAT));
                aResultMap.put(PROPNAME_UIORDER        , aMap.get(PROPNAME_UIORDER        ));
                /* REMOVED!
                aResultMap.put(PROPNAME_DOCUMENTICONID , aMap.get(PROPNAME_DOCUMENTICONID ));
                 */
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    //___________________________________________

    /** converts a filter property set from internal format
     *  to an external one.
     *
     *  @param  aMap
     *          points to the item, which should be converted.
     *
     *  @param  nFormat
     *          specify the requested output format.
     *
     *  @return [java.util.HashMap]
     *          contains the properties in the requested format.
     *
     *  @throws [java.lang.Exception
     *          if something fail during convertion.
     */
    private static java.util.HashMap convertFilterPropsToExternal(java.util.HashMap aMap   ,
                                                                  int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = new java.util.HashMap();
        // copy Name property ... if it exists!
        if (aMap.containsKey(PROPNAME_NAME))
            aResultMap.put(PROPNAME_NAME, aMap.get(PROPNAME_NAME));
        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // copy UIName property unchanged!
                aResultMap.put(PROPNAME_UINAME, aMap.get(PROPNAME_UINAME));

                // but pack all other properties
                java.lang.StringBuffer sData = new java.lang.StringBuffer(256);

                sData.append(aMap.get(PROPNAME_ORDER));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_TYPE));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_DOCUMENTSERVICE));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_FILTERSERVICE));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_FLAGS));
                sData.append(",");
                java.util.Vector lList = (java.util.Vector)aMap.get(PROPNAME_USERDATA);
                int              c     = lList.size();
                int              i     = 0;
                for (i=0; i<c; ++i)
                {
                    sData.append(lList.elementAt(i));
                    if (i<(c-1))
                        sData.append(";");
                }
                sData.append(",");
                sData.append(aMap.get(PROPNAME_FILEFORMATVERSION));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_TEMPLATENAME));
                sData.append(",");
                sData.append(aMap.get(PROPNAME_UICOMPONENT));
                sData.append(",");

                aResultMap.put(PROPNAME_DATA, sData.toString());
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // supress "Order" property.
                // Will be moved to type entries in 6.y version!

                // supress "UIName" property.
                // Only type entries will be localized in 6.y version!
                /* TODO make it configurable :-) */
                aResultMap.put(PROPNAME_UINAME           , aMap.get(PROPNAME_UINAME           ));

                // copy all supported properties directly
                aResultMap.put(PROPNAME_TYPE             , aMap.get(PROPNAME_TYPE             ));
                aResultMap.put(PROPNAME_DOCUMENTSERVICE  , aMap.get(PROPNAME_DOCUMENTSERVICE  ));
                aResultMap.put(PROPNAME_FILTERSERVICE    , aMap.get(PROPNAME_FILTERSERVICE    ));
                aResultMap.put(PROPNAME_USERDATA         , aMap.get(PROPNAME_USERDATA         ));
                aResultMap.put(PROPNAME_FILEFORMATVERSION, aMap.get(PROPNAME_FILEFORMATVERSION));
                aResultMap.put(PROPNAME_TEMPLATENAME     , aMap.get(PROPNAME_TEMPLATENAME     ));
                aResultMap.put(PROPNAME_UICOMPONENT      , aMap.get(PROPNAME_UICOMPONENT      ));

                // "Flags" will be converted from internal format [int] to
                // the 6.y format [string-list]!
                java.lang.Integer nFlags = (java.lang.Integer)aMap.get(PROPNAME_FLAGS);
                java.util.Vector  lFlags = Cache.convertFilterFlagValues2Names(nFlags);
                aResultMap.put(PROPNAME_FLAGS, lFlags);
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    //___________________________________________

    /** converts a detect service property set from internal format
     *  to an external one.
     *
     *  @param  aMap
     *          points to the item, which should be converted.
     *
     *  @param  nFormat
     *          specify the requested output format.
     *
     *  @return [java.util.HashMap]
     *          contains the properties in the requested format.
     *
     *  @throws [java.lang.Exception
     *          if something fail during convertion.
     */
    private static java.util.HashMap convertDetectServicePropsToExternal(java.util.HashMap aMap   ,
                                                                         int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = null;

        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // no changes!
                aResultMap = aMap;
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // remove localized name
                aResultMap = aMap;
                aResultMap.remove(PROPNAME_UINAME);
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    private static java.util.HashMap convertFrameLoaderPropsToExternal(java.util.HashMap aMap   ,
                                                                       int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = null;

        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // no changes!
                aResultMap = aMap;
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // remove localized name
                aResultMap = aMap;
                aResultMap.remove(PROPNAME_UINAME);
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    private static java.util.HashMap convertContentHandlerPropsToExternal(java.util.HashMap aMap   ,
                                                                          int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = null;

        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // no changes!
                aResultMap = aMap;
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // remove localized name
                aResultMap = aMap;
                aResultMap.remove(PROPNAME_UINAME);
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    //___________________________________________

    /** converts a type property set (using an external format) to
     *  our internal cache format.
     *
     *  Especialy the data format string will be expanded
     *  to its real properties.
     *
     *  Schema:
     *      aMap["UIName"] => aExpandedMap["UIName"]
     *      aMap["Data"  ] => aExpandedMap["Preferred" ], aExpandedMap["MediaType"] etc. ...
     *
     *  @param  aMap
     *          points to the item, which should be converted.

     *  @param  nFormat
     *          specify the external format.
     *
     *  @return [java.util.HashMap]
     *          The new map in internal format.
     */
    private static java.util.HashMap convertTypePropsToInternal(java.util.HashMap aMap   ,
                                                                int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = new java.util.HashMap();
        // copy Name property ... if it exists!
        if (aMap.containsKey(PROPNAME_NAME))
            aResultMap.put(PROPNAME_NAME, aMap.get(PROPNAME_NAME));
        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // copy UIName property unchanged!
                aResultMap.put(PROPNAME_UINAME, aMap.get(PROPNAME_UINAME));

                // generate new property "UIOrder"
                // Its the moved property "Order" of filters for versions >= 6.y!
                aResultMap.put(PROPNAME_UIORDER, new java.lang.Integer(0));

                // generate new property "PreferredFilter"
                // Its a the moved filter flag "Preferred" for versions >= 6.y!
                aResultMap.put(PROPNAME_PREFERREDFILTER, new java.lang.String());

                // generate new property "DetectService"
                // Every type know its detector diretcly from now. No search
                // will be neccessary any longer.
                aResultMap.put(PROPNAME_DETECTSERVICE, new java.lang.String());

                // analyze the Data property of the original map
                // and copy its results (means all expanded properties)
                // to the result map.
                java.lang.String sDataVal = (java.lang.String)aMap.get(PROPNAME_DATA);
                java.util.Vector lTokens  = Cache.splitTokenString(sDataVal, ",");

                int t = 0;
                java.util.Enumeration it = lTokens.elements();
                while (it.hasMoreElements())
                {
                    java.lang.String sToken = (java.lang.String)it.nextElement();
                    switch(t)
                    {
                        case 0 :
                            aResultMap.put(PROPNAME_PREFERRED, new java.lang.Boolean(sToken));
                            break;
                        case 1 :
                            aResultMap.put(PROPNAME_MEDIATYPE, sToken);
                            break;
                        case 2 :
                        {
                            /*HACK ersetze %20 mit " " ...*/
                            int ni = sToken.indexOf("%20");
                            if (ni!=-1)
                            {
                                java.lang.String sPatch = sToken.substring(0,ni) + " " + sToken.substring(ni+3);
                                sToken = sPatch;
                            }
                            aResultMap.put(PROPNAME_CLIPBOARDFORMAT, sToken);
                        }
                            break;
                        case 3 :
                            aResultMap.put(PROPNAME_URLPATTERN, Cache.splitTokenString(sToken, ";"));
                            break;
                        case 4 :
                            aResultMap.put(PROPNAME_EXTENSIONS, Cache.splitTokenString(sToken, ";"));
                            break;
                        case 5 :
                            aResultMap.put(PROPNAME_DOCUMENTICONID, new java.lang.Integer(sToken));
                            break;
                        default :
                            throw new java.lang.Exception("unsupported format for data value of a type \""+aMap.get(PROPNAME_NAME)+"\" detected.");
                    }
                    ++t;
                }
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // copy all supported properties directly
                aResultMap.put(PROPNAME_PREFERRED      , aMap.get(PROPNAME_PREFERRED      ));
                aResultMap.put(PROPNAME_MEDIATYPE      , aMap.get(PROPNAME_MEDIATYPE      ));
                aResultMap.put(PROPNAME_CLIPBOARDFORMAT, aMap.get(PROPNAME_CLIPBOARDFORMAT));
                aResultMap.put(PROPNAME_URLPATTERN     , aMap.get(PROPNAME_URLPATTERN     ));
                aResultMap.put(PROPNAME_EXTENSIONS     , aMap.get(PROPNAME_EXTENSIONS     ));
                aResultMap.put(PROPNAME_DOCUMENTICONID , aMap.get(PROPNAME_DOCUMENTICONID ));
                aResultMap.put(PROPNAME_UINAME         , aMap.get(PROPNAME_UINAME         ));
                aResultMap.put(PROPNAME_UIORDER        , aMap.get(PROPNAME_UIORDER        ));
                aResultMap.put(PROPNAME_PREFERREDFILTER, aMap.get(PROPNAME_PREFERREDFILTER));
                aResultMap.put(PROPNAME_DETECTSERVICE  , aMap.get(PROPNAME_DETECTSERVICE  ));
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    //___________________________________________

    /** converts a filter property set (using an external format) to
     *  our internal cache format.
     *
     *  Especialy the data format string will be expanded
     *  to its real properties.
     *
     *  Schema:
     *      aMap["UIName"] => aExpandedMap["UIName"]
     *      aMap["Data"  ] => aExpandedMap["Order" ], aExpandedMap["Flags"] etc. ...
     *
     *  @param  aMap
     *          points to the item, which should be converted.
     *
     *  @param  nFormat
     *          specify the external format.
     *
     *  @return [java.util.HashMap]
     *          The new map in internal format.
     */
    private static java.util.HashMap convertFilterPropsToInternal(java.util.HashMap aMap   ,
                                                                  int               nFormat)
        throws java.lang.Exception
    {
        java.util.HashMap aResultMap = new java.util.HashMap();
        // copy Name property ... if it exists!
        if (aMap.containsKey(PROPNAME_NAME))
            aResultMap.put(PROPNAME_NAME, aMap.get(PROPNAME_NAME));
        switch(nFormat)
        {
            //-----------------------------------
            case FORMAT_60 :
            {
                // copy UIName property
                aResultMap.put(PROPNAME_UINAME, aMap.get(PROPNAME_UINAME));

                // analyze the Data property of the original map
                // and copy its results (means all expanded properties)
                // to the result map.
                java.lang.String sDataVal = (java.lang.String)aMap.get(PROPNAME_DATA);
                java.util.Vector lTokens  = Cache.splitTokenString(sDataVal, ",");

                int t = 0;
                java.util.Enumeration it = lTokens.elements();
                while (it.hasMoreElements())
                {
                    java.lang.String sToken = (java.lang.String)it.nextElement();
                    switch(t)
                    {
                        case 0 :
                            aResultMap.put(PROPNAME_ORDER, new java.lang.Integer(sToken));
                            break;
                        case 1 :
                            aResultMap.put(PROPNAME_TYPE, sToken);
                            break;
                        case 2 :
                            aResultMap.put(PROPNAME_DOCUMENTSERVICE, sToken);
                            break;
                        case 3 :
                            aResultMap.put(PROPNAME_FILTERSERVICE, sToken);
                            break;
                        case 4 :
                            aResultMap.put(PROPNAME_FLAGS, new java.lang.Integer(sToken));
                            break;
                        case 5 :
                            aResultMap.put(PROPNAME_USERDATA, Cache.splitTokenString(sToken, ";"));
                            break;
                        case 6 :
                            aResultMap.put(PROPNAME_FILEFORMATVERSION, new java.lang.Integer(sToken));
                            break;
                        case 7 :
                            aResultMap.put(PROPNAME_TEMPLATENAME, sToken);
                            break;
                        case 8 :
                            aResultMap.put(PROPNAME_UICOMPONENT, sToken);
                            break;
                        default :
                            throw new java.lang.Exception("unsupported format for data value of a filter detected.");
                    }
                    ++t;
                }

                // its an optional property :-)
                if (!aResultMap.containsKey(PROPNAME_TEMPLATENAME))
                    aResultMap.put(PROPNAME_TEMPLATENAME, new java.lang.String(""));

                // its an optional property :-)
                if (!aResultMap.containsKey(PROPNAME_UICOMPONENT))
                    aResultMap.put(PROPNAME_UICOMPONENT, new java.lang.String(""));
            }
            break;

            //-----------------------------------
            case FORMAT_6Y :
            {
                // "Order" does not exist for 6.y versions! Use default.
                aResultMap.put(PROPNAME_ORDER, new java.lang.Integer(0));

                // "UIName" property does not exist for 6.y versions! use default.
                /* TODO make it configurable :-) */
                aResultMap.put(PROPNAME_UINAME, aMap.get(PROPNAME_UINAME));
                //aResultMap.put(PROPNAME_UINAME, new java.util.HashMap());

                // "Flags" must be converted from names to its values
                java.util.Vector  lFlags = (java.util.Vector)aMap.get(PROPNAME_FLAGS);
                java.lang.Integer nFlags = Cache.convertFilterFlagNames2Values(lFlags);
                aResultMap.put(PROPNAME_FLAGS, nFlags);

                // copy all direct supported properties
                aResultMap.put(PROPNAME_TYPE             , aMap.get(PROPNAME_TYPE             ));
                aResultMap.put(PROPNAME_DOCUMENTSERVICE  , aMap.get(PROPNAME_DOCUMENTSERVICE  ));
                aResultMap.put(PROPNAME_FILTERSERVICE    , aMap.get(PROPNAME_ORDER            ));
                aResultMap.put(PROPNAME_USERDATA         , aMap.get(PROPNAME_USERDATA         ));
                aResultMap.put(PROPNAME_FILEFORMATVERSION, aMap.get(PROPNAME_FILEFORMATVERSION));
                aResultMap.put(PROPNAME_TEMPLATENAME     , aMap.get(PROPNAME_TEMPLATENAME     ));
                aResultMap.put(PROPNAME_UICOMPONENT      , aMap.get(PROPNAME_UICOMPONENT      ));
            }
            break;

            //-----------------------------------
            default :
                throw new java.lang.Exception("unknown format");
        }

        return aResultMap;
    }

    private static java.util.HashMap convertDetectServicePropsToInternal(java.util.HashMap aMap   ,
                                                                         int               nFormat)
        throws java.lang.Exception
    {
        /*FIXME*/
        java.util.HashMap aResultMap = aMap;
        return aResultMap;
    }

    private static java.util.HashMap convertFrameLoaderPropsToInternal(java.util.HashMap aMap   ,
                                                                       int               nFormat)
        throws java.lang.Exception
    {
        /*FIXME*/
        java.util.HashMap aResultMap = aMap;
        return aResultMap;
    }

    private static java.util.HashMap convertContentHandlerPropsToInternal(java.util.HashMap aMap   ,
                                                                          int               nFormat)
        throws java.lang.Exception
    {
        /*FIXME*/
        java.util.HashMap aResultMap = aMap;
        return aResultMap;
    }

    //___________________________________________

    /** converts filter flag names to its int representation.
     *
     *  @param  lFlags
     *          a list of flag names.
     *
     *  @return [java.lang.Integer]
     *          an integer field of all set flags.
     *
     *  @throws [java.lang.Exception]
     *          for unsupported flags or empty flag fields!
     */
    private static java.lang.Integer convertFilterFlagNames2Values(java.util.Vector lFlags)
        throws java.lang.Exception
    {
        int                   nFlags = 0;
        java.util.Enumeration it     = lFlags.elements();
        while(it.hasMoreElements())
        {
            java.lang.String sFlagName = (java.lang.String)it.nextElement();

            if (sFlagName.equals(FLAGNAME_3RDPARTYFILTER))
                nFlags |= FLAGVAL_3RDPARTYFILTER;
            else
            if (sFlagName.equals(FLAGNAME_ALIEN))
                nFlags |= FLAGVAL_ALIEN;
            else
            if (sFlagName.equals(FLAGNAME_ASYNCHRON))
                nFlags |= FLAGVAL_ASYNCHRON;
            else
            if (sFlagName.equals(FLAGNAME_BROWSERPREFERRED))
                nFlags |= FLAGVAL_BROWSERPREFERRED;
            else
            if (sFlagName.equals(FLAGNAME_CONSULTSERVICE))
                nFlags |= FLAGVAL_CONSULTSERVICE;
            else
            if (sFlagName.equals(FLAGNAME_DEFAULT))
                nFlags |= FLAGVAL_DEFAULT;
            else
            if (sFlagName.equals(FLAGNAME_EXPORT))
                nFlags |= FLAGVAL_EXPORT;
            else
            if (sFlagName.equals(FLAGNAME_IMPORT))
                nFlags |= FLAGVAL_IMPORT;
            else
            if (sFlagName.equals(FLAGNAME_INTERNAL))
                nFlags |= FLAGVAL_INTERNAL;
            else
            if (sFlagName.equals(FLAGNAME_NOTINCHOOSER))
                nFlags |= FLAGVAL_NOTINCHOOSER;
            else
            if (sFlagName.equals(FLAGNAME_NOTINFILEDIALOG))
                nFlags |= FLAGVAL_NOTINFILEDIALOG;
            else
            if (sFlagName.equals(FLAGNAME_NOTINSTALLED))
                nFlags |= FLAGVAL_NOTINSTALLED;
            else
            if (sFlagName.equals(FLAGNAME_OWN))
                nFlags |= FLAGVAL_OWN;
            else
            if (sFlagName.equals(FLAGNAME_PACKED))
                nFlags |= FLAGVAL_PACKED;
            else
            if (sFlagName.equals(FLAGNAME_PREFERRED))
                nFlags |= FLAGVAL_PREFERRED;
            else
            if (sFlagName.equals(FLAGNAME_READONLY))
                nFlags |= FLAGVAL_READONLY;
            else
            if (sFlagName.equals(FLAGNAME_TEMPLATE))
                nFlags |= FLAGVAL_TEMPLATE;
            else
            if (sFlagName.equals(FLAGNAME_TEMPLATEPATH))
                nFlags |= FLAGVAL_TEMPLATEPATH;
            else
            if (sFlagName.equals(FLAGNAME_USESOPTIONS))
                nFlags |= FLAGVAL_USESOPTIONS;
            else
            if (sFlagName.equals(FLAGNAME_COMBINED))
                nFlags |= FLAGVAL_COMBINED;
            else
                throw new java.lang.Exception("unsupported filter flag detected: \""+sFlagName+"\"");
        }

        if (nFlags == 0)
            throw new java.lang.Exception("no filter flags?");

        return new java.lang.Integer(nFlags);
    }

    //___________________________________________

    /** converts filter flag values to its string representation.
     *
     *  @param  nFlags
     *          the flag field as int value.
     *
     *  @return [java.util.Vector]
     *          a list of flag names.
     *
     *  @throws [java.lang.Exception]
     *          for unsupported flags or empty flag fields!
     */
    private static java.util.Vector convertFilterFlagValues2Names(java.lang.Integer nFlags)
        throws java.lang.Exception
    {
        java.util.Vector lFlags = new java.util.Vector();
        int              field  = nFlags.intValue();

        if (field == 0)
            throw new java.lang.Exception("no filter flags?");

        if((field & FLAGVAL_IMPORT) == FLAGVAL_IMPORT)
            lFlags.add(FLAGNAME_IMPORT);

        if((field & FLAGVAL_EXPORT) == FLAGVAL_EXPORT)
            lFlags.add(FLAGNAME_EXPORT);

        if((field & FLAGVAL_TEMPLATE) == FLAGVAL_TEMPLATE)
            lFlags.add(FLAGNAME_TEMPLATE);

        if((field & FLAGVAL_INTERNAL) == FLAGVAL_INTERNAL)
            lFlags.add(FLAGNAME_INTERNAL);

        if((field & FLAGVAL_TEMPLATEPATH) == FLAGVAL_TEMPLATEPATH)
            lFlags.add(FLAGNAME_TEMPLATEPATH);

        if((field & FLAGVAL_OWN) == FLAGVAL_OWN)
            lFlags.add(FLAGNAME_OWN);

        if((field & FLAGVAL_ALIEN) == FLAGVAL_ALIEN)
            lFlags.add(FLAGNAME_ALIEN);

        if((field & FLAGVAL_USESOPTIONS) == FLAGVAL_USESOPTIONS)
            lFlags.add(FLAGNAME_USESOPTIONS);

        if((field & FLAGVAL_DEFAULT) == FLAGVAL_DEFAULT)
            lFlags.add(FLAGNAME_DEFAULT);

        if((field & FLAGVAL_NOTINFILEDIALOG) == FLAGVAL_NOTINFILEDIALOG)
            lFlags.add(FLAGNAME_NOTINFILEDIALOG);

        if((field & FLAGVAL_NOTINCHOOSER) == FLAGVAL_NOTINCHOOSER)
            lFlags.add(FLAGNAME_NOTINCHOOSER);

        if((field & FLAGVAL_ASYNCHRON) == FLAGVAL_ASYNCHRON)
            lFlags.add(FLAGNAME_ASYNCHRON);

        if((field & FLAGVAL_READONLY) == FLAGVAL_READONLY)
            lFlags.add(FLAGNAME_READONLY);

        if((field & FLAGVAL_NOTINSTALLED) == FLAGVAL_NOTINSTALLED)
            lFlags.add(FLAGNAME_NOTINSTALLED);

        if((field & FLAGVAL_CONSULTSERVICE) == FLAGVAL_CONSULTSERVICE)
            lFlags.add(FLAGNAME_CONSULTSERVICE);

        if((field & FLAGVAL_3RDPARTYFILTER) == FLAGVAL_3RDPARTYFILTER)
            lFlags.add(FLAGNAME_3RDPARTYFILTER);

        if((field & FLAGVAL_PACKED) == FLAGVAL_PACKED)
            lFlags.add(FLAGNAME_PACKED);

        if((field & FLAGVAL_BROWSERPREFERRED) == FLAGVAL_BROWSERPREFERRED)
            lFlags.add(FLAGNAME_BROWSERPREFERRED);

        if((field & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED)
            lFlags.add(FLAGNAME_PREFERRED);

        if((field & FLAGVAL_COMBINED) == FLAGVAL_COMBINED)
            lFlags.add(FLAGNAME_COMBINED);

        if((field & FLAGVAL_COMBINED) == FLAGVAL_SUPPORTSSELECTION)
            lFlags.add(FLAGNAME_SUPPORTSSELECTION);

        return lFlags;
    }

    //___________________________________________

    /** return a reference to one of our member
     *  lists for types/filters etc ...
     *
     *  @param  eItemType
     *          specify, which item map is required.
     *
     *  @return [java.util.HashMap]
     *          a reference(!) to the right member.
     *
     *  @throws [java.lang.Exception]
     *          if the specified map does not exist.
     */
    private java.util.HashMap getItemMap(int eItemType)
        throws java.lang.Exception
    {
        java.util.HashMap rMap = null;
        switch(eItemType)
        {
            case E_TYPE :
                rMap = m_lTypes;
                break;

            case E_FILTER :
                rMap = m_lFilters;
                break;

            case E_DETECTSERVICE :
                rMap = m_lDetectServices;
                break;

            case E_FRAMELOADER :
                rMap = m_lFrameLoaders;
                break;

            case E_CONTENTHANDLER :
                rMap = m_lContentHandlers;
                break;

            default:
                throw new java.lang.Exception("Invalid item map specified.");
        }
        return rMap;
    }

    //___________________________________________

    /** return the count of items inside a sub container
     *  of this cache.
     *
     *  @param  eItemType
     *          specify, which item map is required.
     *
     *  @throws [java.lang.Exception]
     *          if the specified map does not exist.
     */
    public synchronized int getItemCount(int eItemType)
        throws java.lang.Exception
    {
        java.util.HashMap rMap = getItemMap(eItemType);
        return rMap.size();
    }

    //___________________________________________

    /** get a list of all item names of the specified
     *  sub container.
     *
     *  @param  eItemType
     *          specify, which item map is required.
     *
     *  @throws [java.lang.Exception]
     *          if the specified map does not exist.
     */
    public synchronized java.util.Vector getItemNames(int eItemType)
        throws java.lang.Exception
    {
        java.util.Vector   lNames = new java.util.Vector();
        java.util.HashMap  rMap   = getItemMap(eItemType);
        java.util.Iterator it     = rMap.keySet().iterator();
        while(it.hasNext())
            lNames.add(it.next());
        return lNames;
    }

    //___________________________________________

    /** get a list of all item names of the specified
     *  sub coontainer, where items match to given property set.
     *
     *  Note: The given property set must exist at all
     *  returned items as minimum and every checked property
     *  value must be equals! Using of reg expressions or
     *  similar mechanism will not be supported here.
     *
     *  @param  eItemType
     *          specify, which item map is required.
     *
     *  @param  aPropSet
     *          the set of properties, which must
     *          exist at the returned item as minimum.
     *
     *  @throws [java.lang.Exception]
     *          if the specified map does not exist.
     */
    public synchronized java.util.Vector getMatchedItemNames(int               eItemType,
                                                             java.util.HashMap aPropSet )
        throws java.lang.Exception
    {
        java.util.Vector   lNames = new java.util.Vector();
        java.util.HashMap  rMap   = getItemMap(eItemType);
        java.util.Iterator it     = rMap.keySet().iterator();
        while(it.hasNext())
        {
            java.lang.String  sItemName  = (java.lang.String)it.next();
            java.util.HashMap rItemProps = (java.util.HashMap)rMap.get(sItemName);

            boolean bMatch = Cache.matchPropSet(rItemProps, aPropSet);
            if (bMatch)
                lNames.add(sItemName);
            else
            {
                java.lang.StringBuffer sBuffer = new java.lang.StringBuffer(1000);
                sBuffer.append("entry ["+eItemType+"] \""+sItemName+"\" does not match.\n");
                sBuffer.append("\torg items = {"+rItemProps+"}\n");
                sBuffer.append("\treq items = {"+aPropSet+"}\n");

                m_aDebug.setDetailedInfo(sBuffer.toString());
            }
        }
        return lNames;
    }

    //___________________________________________

    /** check if two property sets are equals in its
     *  shared properties.
     *
     *  Note: Only set properties of the match set will be searched
     *  inside the original set. And its values must be equals.
     *  Using of reg expressions or similar mechanism will not
     *  be supported here.
     *
     *  @param  rOrgProps
     *          the original property set, which should be checked.
     *
     *  @param  rMatchProps
     *          contains the properties, which must be searched
     *          inside rOrgProps.
     *
     *  @return TRUE if all properties of rMatchProps could be located
     *          inside rOrgProps.
     */
    private static boolean matchPropSet(java.util.HashMap rOrgProps  ,
                                        java.util.HashMap rMatchProps)
    {
        java.util.Iterator it = rMatchProps.keySet().iterator();
        while(it.hasNext())
        {
            java.lang.String sMatchName  = (java.lang.String)it.next();
            java.lang.Object aMatchValue = rMatchProps.get(sMatchName);

            if (
                (!rOrgProps.containsKey(sMatchName)            ) ||
                (!rOrgProps.get(sMatchName).equals(aMatchValue))
               )
            {
                return false;
            }
        }
        return true;
    }

    //___________________________________________

    /** return a property set for the queried container item.
     *
     *  @param  eItemType
     *          specify, which item map is required.
     *
     *  @param  sItemName
     *          must be a valid item name of the specified item map.
     *
     *  @return [java.util.HashMap]
     *          the property set of the queried item.
     *          Always different from null!
     *
     *  @throws [java.lang.Exception]
     *          if the specified item does not exists or
     *          seems to be invalid in general (means null!).
     */
    public synchronized java.util.HashMap getItem(int              eItemType,
                                                  java.lang.String sItemName)
        throws java.lang.Exception
    {
        java.util.HashMap rMap  = getItemMap(eItemType);
        java.util.HashMap rItem = (java.util.HashMap)rMap.get(sItemName);
        if (rItem == null)
            throw new java.lang.Exception("Queried item \""+sItemName+"\" does not exist inside this cache.");
        return rItem;
    }

    //___________________________________________

    /** return a requested item in XML format.
     *
     *  @param  eItemType
     *          identify the right sub set of this cache
     *          inside which the requested item should exist.
     *          e.g. E_TYPE, E_FILTER, ...
     *
     *  @param  sItemName
     *          the name of the request item
     *
     *  @param  nXMLFormat
     *          means the format of the generated xml source.
     *
     *  @return [java.lang.String]
     *          a xml formated string, which contains all properties
     *          for this container item.
     */
    public synchronized java.lang.String getItemAsXML(int              eItemType ,
                                                      java.lang.String sItemName ,
                                                      int              nXMLFormat)
        throws java.lang.Exception
    {
        // Note: Our internal format must be converted to the target format!
        java.util.HashMap rItem         = getItem(eItemType, sItemName);
        java.util.HashMap rFormatedItem = null;
        switch(eItemType)
        {
            case E_TYPE :
                {
                    rFormatedItem = Cache.convertTypePropsToExternal(rItem, nXMLFormat);
                    m_aDebug.setGlobalInfo("type to external \""+sItemName+"\"");
                }
                break;

            case E_FILTER :
                {
                    rFormatedItem = Cache.convertFilterPropsToExternal(rItem, nXMLFormat);
                    m_aDebug.setGlobalInfo("filter to external \""+sItemName+"\"");
                }
                break;

            case E_DETECTSERVICE :
                {
                    rFormatedItem = Cache.convertDetectServicePropsToExternal(rItem, nXMLFormat);
                    m_aDebug.setGlobalInfo("detect service to external \""+sItemName+"\"");
                }
                break;

            case E_FRAMELOADER :
                {
                    rFormatedItem = Cache.convertFrameLoaderPropsToExternal(rItem, nXMLFormat);
                    m_aDebug.setGlobalInfo("frame loader to external \""+sItemName+"\"");
                }
                break;

            case E_CONTENTHANDLER :
                {
                    rFormatedItem = Cache.convertContentHandlerPropsToExternal(rItem, nXMLFormat);
                    m_aDebug.setGlobalInfo("content handler to external \""+sItemName+"\"");
                }
                break;
        }

        java.lang.StringBuffer sXML        = new java.lang.StringBuffer(1000);
        int                    nPrettyTabs = 1;
        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");
        sXML.append("<"+XMLHelper.XMLTAG_NODE+" "+XMLHelper.XMLATTRIB_OOR_NAME+"=\""+XMLHelper.encodeHTMLSigns(sItemName)+"\" "+XMLHelper.XMLATTRIB_OOR_OP+"=\""+XMLHelper.XMLATTRIB_OP_REPLACE+"\">\n");
        sXML.append(XMLHelper.convertPropSetToXML(rFormatedItem, nPrettyTabs+1));
        for (int t=0; t<nPrettyTabs; ++t)
            sXML.append("\t");
        sXML.append("</"+XMLHelper.XMLTAG_NODE+">\n");

        return sXML.toString();
    }

    //___________________________________________

    /** split the given string (using the specified delimiter)
     *  and return alist of found string tokens.
     *
     *  Note: Against the normal behaviour of the StringTokenizer class
     *  this method returns empty tokens too.
     *  E.g: "0,,1" will return "0" - "" - "1"
     *
     *  @param  sTokenString
     *          the string value, which should be analyzed.
     *
     *  @param  sDelim
     *          the delimiter, which will be used to differe between tokens.
     *
     *  @return [java.util.Vector]
     *          a list of string tokens. Can be empty - but not null!
     */
    private static java.util.Vector splitTokenString(java.lang.String sTokenString,
                                                     java.lang.String sDelim      )
    {
        java.util.Vector          lTokens       = new java.util.Vector();
        java.util.StringTokenizer aTokenizer    = new java.util.StringTokenizer(sTokenString, sDelim, true);
        boolean                   bLastWasDelim = false;

        while (aTokenizer.hasMoreTokens())
        {
            java.lang.String sToken = aTokenizer.nextToken();
            if (sToken.equals(sDelim))
            {
                if (bLastWasDelim)
                {
                    // last token was a delimiter - new one too
                    // => an empty token must be placed between these
                    //    two delimiters! Add this empty value to the return list.
                    lTokens.add("");
                }
                else
                {
                    // last token was not a delimiter - new one is such delim
                    // => ignore this delimiter - but save the information, that
                    //    it occurred
                    bLastWasDelim = true;
                }
            }
            else
            {
                // new token is no delim
                // => Add it to the return list.
                lTokens.add(sToken);
                // Dont forget to reset this information - so next loop
                // will do the right things!
                bLastWasDelim = false;
            }
        }

        return lTokens;
    }

    //___________________________________________

    /**
     */
    public synchronized void analyze()
    {
        m_nDoubleRegisteredFilters = 0;
        m_nTypesForFilters         = 0;
        m_nTypesForDetectServices  = 0;
        m_nTypesForFrameLoaders    = 0;
        m_nTypesForContentHandlers = 0;

        // create table of types and all registered filters for such types
        // By the way: count all double registrations, where a filter
        // uses the same type then another filter.
        m_lFilterToTypeRegistrations = new java.util.HashMap();
        java.util.Iterator aIt1 = m_lFilters.keySet().iterator();
        while (aIt1.hasNext())
        {
            java.lang.String  sFilter = (java.lang.String)aIt1.next();
            java.util.HashMap aFilter = (java.util.HashMap)m_lFilters.get(sFilter);
            java.lang.String  sType   = (java.lang.String)aFilter.get(PROPNAME_TYPE);

            java.util.Vector lFilters = (java.util.Vector)m_lFilterToTypeRegistrations.get(sType);
            if (lFilters == null)
                lFilters = new java.util.Vector();
            else
                ++m_nDoubleRegisteredFilters;
            lFilters.add(sFilter);
            m_lFilterToTypeRegistrations.put(sType, lFilters);
        }

        // count, how many types are used by filters, frame loaders or content handlers
        aIt1 = m_lTypes.keySet().iterator();
        while (aIt1.hasNext())
        {
            java.lang.String sType = (java.lang.String)aIt1.next();

            java.util.Iterator aIt2 = m_lFilters.keySet().iterator();
            while (aIt2.hasNext())
            {
                java.lang.String  sItem    = (java.lang.String)aIt2.next();
                java.util.HashMap aItem    = (java.util.HashMap)m_lFilters.get(sItem);
                java.lang.String  sTypeReg = (java.lang.String)aItem.get(PROPNAME_TYPE);

                if (sTypeReg.equals(sType))
                {
                    ++m_nTypesForFilters;
                    break;
                }
            }

            aIt2 = m_lDetectServices.keySet().iterator();
            while (aIt2.hasNext())
            {
                java.lang.String  sItem    = (java.lang.String)aIt2.next();
                java.util.HashMap aItem    = (java.util.HashMap)m_lDetectServices.get(sItem);
                java.util.Vector  lTypeReg = (java.util.Vector)aItem.get(PROPNAME_TYPES);

                if (lTypeReg.contains(sType))
                {
                    ++m_nTypesForDetectServices;
                    break;
                }
            }

            aIt2 = m_lFrameLoaders.keySet().iterator();
            while (aIt2.hasNext())
            {
                java.lang.String  sItem    = (java.lang.String)aIt2.next();
                java.util.HashMap aItem    = (java.util.HashMap)m_lFrameLoaders.get(sItem);
                java.util.Vector  lTypeReg = (java.util.Vector)aItem.get(PROPNAME_TYPES);

                if (lTypeReg.contains(sType))
                {
                    ++m_nTypesForFrameLoaders;
                    break;
                }
            }

            aIt2 = m_lContentHandlers.keySet().iterator();
            while (aIt2.hasNext())
            {
                java.lang.String  sItem    = (java.lang.String)aIt2.next();
                java.util.HashMap aItem    = (java.util.HashMap)m_lContentHandlers.get(sItem);
                java.util.Vector  lTypeReg = (java.util.Vector)aItem.get(PROPNAME_TYPES);

                if (lTypeReg.contains(sType))
                {
                    ++m_nTypesForContentHandlers;
                    break;
                }
            }
        }
    }

    //___________________________________________

    /** validate all cache entries.
     *
     *  It checks if all made registrations are valid;
     *  try to repair some simple problems;
     *  create missing informations on demand ...
     *
     *  @param  nFormat
     *          specify, which configuration format
     *          must be checked.
     *
     *  @throws [java.lang.Exception]
     *          if an unrecoverable problem occure.
     */
    public synchronized void validate(int nFormat)
        throws java.lang.Exception
    {
        validateTypes(nFormat);
        validateFilters(nFormat);
    }

    //___________________________________________

    /** validate all type entries of this cache.
     *
     *  @param  nFormat
     *          specify, which configuration format
     *          must be checked.
     *
     *  @throws [java.lang.Exception]
     *          if an unrecoverable problem occure.
     */
    private void validateTypes(int nFormat)
        throws java.lang.Exception
    {
        java.util.Iterator aIt1 = m_lTypes.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String  sType = (java.lang.String)aIt1.next();
            java.util.HashMap aType = (java.util.HashMap)m_lTypes.get(sType);
            if (aType == null)
                throw new java.lang.Exception("type ["+sType+"] dos not exist realy?!");

            if (
                (!aType.containsKey(PROPNAME_MEDIATYPE      )) ||
                (!aType.containsKey(PROPNAME_PREFERRED      )) ||
                (!aType.containsKey(PROPNAME_CLIPBOARDFORMAT)) ||
                (!aType.containsKey(PROPNAME_DOCUMENTICONID )) ||
                (!aType.containsKey(PROPNAME_URLPATTERN     )) ||
                (!aType.containsKey(PROPNAME_EXTENSIONS     )) ||
                (!aType.containsKey(PROPNAME_UINAME         ))
               )
            {
                throw new java.lang.Exception("Type \""+sType+"\" does not contain all neccessary properties for a 6.0/6.Y format.");
            }

            if (
                (((java.util.Vector)aType.get(PROPNAME_EXTENSIONS)).isEmpty()) &&
                (((java.util.Vector)aType.get(PROPNAME_URLPATTERN)).isEmpty())
               )
            {
                throw new java.lang.Exception("Type \""+sType+"\" does not contain any extension nor an url pattern.");
            }

            if (((java.util.HashMap)aType.get(PROPNAME_UINAME)).isEmpty())
                throw new java.lang.Exception("Type \""+sType+"\" is not localized.");

            if (nFormat == FORMAT_6Y)
            {
                if (
                    (!aType.containsKey(PROPNAME_UIORDER        )) ||
                    (!aType.containsKey(PROPNAME_PREFERREDFILTER)) ||
                    (!aType.containsKey(PROPNAME_DETECTSERVICE  ))
                   )
                {
                    throw new java.lang.Exception("Type \""+sType+"\" does not contain all neccessary properties for a 6.Y format.");
                }

                if (((java.lang.Integer)aType.get(PROPNAME_UIORDER)).intValue() < 0)
                    throw new java.lang.Exception("Type \""+sType+"\" has invalid value for prop UIOrder.");

                if (((java.lang.String)aType.get(PROPNAME_DETECTSERVICE)).length() < 1)
                    m_aDebug.setWarning("Type \""+sType+"\" has no detect service registered.");

                java.lang.String sPreferredReg = (java.lang.String)aType.get(PROPNAME_PREFERREDFILTER);
                if (
                    (sPreferredReg          == null) ||
                    (sPreferredReg.length() <  1   )
                   )
                {
                    m_aDebug.setWarning("Type \""+sType+"\" has no preferred filter ...");
                    /*FIXME
                     *  OK - not every type has a filter registered .. but the
                     *  a frame loader MUST(!) exist! Check it.
                     */
                }
                else
                {
                    if (!m_lFilters.containsKey(sPreferredReg))
                        throw new java.lang.Exception("Type \""+sType+"\" has no valid preferred filter registration [\""+sPreferredReg+"\"].");
                }
            }
        }
    }

    //___________________________________________

    /** validate all filter entries of this cache.
     *
     *  @param  nFormat
     *          specify, which configuration format
     *          must be checked.
     *
     *  @throws [java.lang.Exception]
     *          if an unrecoverable problem occure.
     */
    public synchronized void validateFilters(int nFormat)
        throws java.lang.Exception
    {
        java.util.Iterator aIt1 = m_lFilters.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String  sFilter = (java.lang.String)aIt1.next();
            java.util.HashMap aFilter = (java.util.HashMap)m_lFilters.get(sFilter);
            if (aFilter == null)
                throw new java.lang.Exception("filter ["+sFilter+"] dos not exist realy?!");

            if (
                (!aFilter.containsKey(PROPNAME_DOCUMENTSERVICE  )) ||
                (!aFilter.containsKey(PROPNAME_FILEFORMATVERSION)) ||
                (!aFilter.containsKey(PROPNAME_FILTERSERVICE    )) ||
                (!aFilter.containsKey(PROPNAME_FLAGS            )) ||
                (!aFilter.containsKey(PROPNAME_TEMPLATENAME     )) ||
                (!aFilter.containsKey(PROPNAME_TYPE             )) ||
                (!aFilter.containsKey(PROPNAME_UICOMPONENT      )) ||
                (!aFilter.containsKey(PROPNAME_USERDATA         ))
               )
            {
                throw new java.lang.Exception("Filter \""+sFilter+"\" does not contain all neccessary properties for a 6.0/6.Y format.");
            }

            if (((java.lang.Integer)aFilter.get(PROPNAME_FLAGS)).intValue() < 1)
                throw new java.lang.Exception("Filter \""+sFilter+"\" does not have a valid flag field.");

            if (!m_lTypes.containsKey(aFilter.get(PROPNAME_TYPE)))
                throw new java.lang.Exception("Filter \""+sFilter+"\" is not registered for a well known type.");

            if (nFormat == FORMAT_60)
            {
                if (
                    (!aFilter.containsKey(PROPNAME_ORDER )) ||
                    (!aFilter.containsKey(PROPNAME_UINAME))
                   )
                {
                    throw new java.lang.Exception("Filter \""+sFilter+"\" does not contain all neccessary properties for a 6.0 format.");
                }

                if (((java.lang.Integer)aFilter.get(PROPNAME_ORDER)).intValue() < 0)
                    throw new java.lang.Exception("Filter \""+sFilter+"\" does not have a valid Order value.");

                if (((java.util.HashMap)aFilter.get(PROPNAME_UINAME)).isEmpty())
                    throw new java.lang.Exception("Filter \""+sFilter+"\" is not localized.");
            }
/*TODO
    depends from the configuration item "remove_filter_flag_preferred" ...

            if (nFormat == FORMAT_6Y)
            {
                int flags = ((java.lang.Integer)aFilter.get(PROPNAME_FLAGS)).intValue();
                if ((flags & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED)
                    throw new java.lang.Exception("Filter \""+sFilter+"\" has superflous Preferred flag set. Please remove this flag. ["+flags+"]");
            }
*/
        }
    }

    /*TODO
     * - remove graphic filters!
     * - move detect services to types
     */

    public synchronized void transform60to6Y(boolean bCreateCombineFilterFlag         ,
                                             boolean bRemoveFilterFlagBrowserPreferred,
                                             boolean bRemoveFilterFlagPreferred       ,
                                             boolean bRemoveFilterFlag3rdparty        ,
                                             boolean bRemoveFilterUINames             ,
                                             boolean bRemoveGraphicFilters            ,
                                             boolean bSetDefaultDetector              )
        throws java.lang.Exception
    {
        // remove some superflous cache entries ...
        // everything related to "load macros"
        // Macros should be dispatched instead of loaded!
        if (m_lTypes.containsKey("macro"))
        {
            m_lTypes.remove("macro");
            m_aDebug.setDetailedInfo("superflous type \"macro\" was removed");
        }
        if (m_lFrameLoaders.containsKey("com.sun.star.comp.sfx2.SfxMacroLoader"))
        {
            m_lFrameLoaders.remove("com.sun.star.comp.sfx2.SfxMacroLoader");
            m_aDebug.setDetailedInfo("superflous frame loader \"com.sun.star.comp.sfx2.SfxMacroLoader\" was removed");
        }

        // step over all filters and check her properties and references
        java.util.Vector   lPreferredFilters = new java.util.Vector();
        java.util.Vector   lNoRealFilters    = new java.util.Vector();
        java.util.Iterator aIt1              = m_lFilters.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String  sFilter  = (java.lang.String)aIt1.next();
            java.util.HashMap aFilter  = (java.util.HashMap)m_lFilters.get(sFilter);

            // remove the "graphic helper filters" used by draw and impress
            // They dont have any valid document service name set and cant be handled
            // by our generic FrameLoader!
            // They must be moved to her own configuration ...

            if (
                (bRemoveGraphicFilters                                                 ) &&
                (((java.lang.String)aFilter.get(PROPNAME_DOCUMENTSERVICE)).length() < 1)
               )
            {
                lNoRealFilters.add(sFilter);
                continue;
            }

            java.lang.String  sTypeReg = (java.lang.String)aFilter.get(PROPNAME_TYPE);
            java.util.HashMap aType    = (java.util.HashMap)m_lTypes.get(sTypeReg);

            // move UINames of filters to types
            java.util.HashMap  lFilterUINames = (java.util.HashMap)aFilter.get(PROPNAME_UINAME);
            java.util.HashMap  lTypeUINames   = (java.util.HashMap)aType.get(PROPNAME_UINAME);
            java.util.HashMap  lPatchUINames  = new java.util.HashMap();

            java.util.Iterator pUINames = lTypeUINames.keySet().iterator();
            while(pUINames.hasNext())
            {
                java.lang.String sLocale = (java.lang.String)pUINames.next();
                java.lang.String sValue  = (java.lang.String)lTypeUINames.get(sLocale);
                lPatchUINames.put(sLocale, sValue);
            }

            pUINames = lFilterUINames.keySet().iterator();
            while(pUINames.hasNext())
            {
                java.lang.String sLocale = (java.lang.String)pUINames.next();
                java.lang.String sValue  = (java.lang.String)lFilterUINames.get(sLocale);
                lPatchUINames.put(sFilter+":"+sLocale, sValue);
            }
            aType.put(PROPNAME_UINAME, lPatchUINames);

            // set generic filter service wrapper for our own native filters!
            // By the way: The format types of such filters can be detected by our
            // generic detector too.
            if (
                (bSetDefaultDetector                                                 ) &&
                (((java.lang.String)aFilter.get(PROPNAME_FILTERSERVICE)).length() < 1)
               )
            {
 /*ME_THINKING               aFilter.put(PROPNAME_FILTERSERVICE, FILTERSERVICE_NATIVEWARPPER);*/
                aType.put(PROPNAME_DETECTSERVICE, GENERIC_DETECTSERVICE);
            }

            // move the preferred filter information to any type
            // Set the filter name to the type for which the filter is registered.
            // If this type already have a set PreferredFilter value, check if the current filter
            // has the preferred flag set. If not ignore it - otherwhise overwrite the
            // current information at the type. But look for multiple preferred filter relations ...
            // means: look if more the one filter has set the preferred flag for the same type!

            /* Attention!
             *
             * Dont remove the preferred flag from any filter! ... not here.
             * Otherwhise next loop can't detect ambigous preferred registrations!
             * Add filter to a temp. list, which can be used later to remove the preferred
             * flag ...
             */

            int     flags1     = ((java.lang.Integer)aFilter.get(PROPNAME_FLAGS)).intValue();
            java.lang.String sDocSrv = (java.lang.String)aFilter.get(PROPNAME_DOCUMENTSERVICE);
            if (sDocSrv.length()>0)// without a doc service its not a real filter - its a graphic filter!
            {
                boolean preferred1 = ((flags1 & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED);
                if (preferred1)
                    lPreferredFilters.add(aFilter);

                java.lang.String sAlreadyRegisteredFilter = (java.lang.String)aType.get(PROPNAME_PREFERREDFILTER);
                // no registration => set this filter as "any possible one"!
                if (sAlreadyRegisteredFilter.length() < 1)
                    aType.put(PROPNAME_PREFERREDFILTER, sFilter);
                else
                {
                    java.util.HashMap aAlreadyRegisteredFilter = (java.util.HashMap)m_lFilters.get(sAlreadyRegisteredFilter);
                    int               flags2                   = ((java.lang.Integer)aAlreadyRegisteredFilter.get(PROPNAME_FLAGS)).intValue();
                    boolean           preferred2               = ((flags2 & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED);

                    // two preferred filters for the same type! => error
                    if (preferred1 && preferred2)
                    {
                        java.lang.StringBuffer sMsg = new java.lang.StringBuffer(256);
                        sMsg.append("More the one preferred filter detected for the same type.\n");
                        sMsg.append("\ttype      = \""+sTypeReg+"\"\n");
                        sMsg.append("\tfilter[1] = \""+sAlreadyRegisteredFilter+"\"\n");
                        sMsg.append("\tfilter[2] = \""+sFilter+"\"\n");
                        throw new java.lang.Exception(sMsg.toString());
                    }
                    else
                    // overwrite the "any possible" filter with a real preferred one
                    if (preferred1 && !preferred2)
                        aType.put(PROPNAME_PREFERREDFILTER, sFilter);
                }
            }

            // create the new combined filter flag if required
            if (bCreateCombineFilterFlag)
            {
                if (
                    ((flags1 & FLAGVAL_IMPORT) == FLAGVAL_IMPORT) &&
                    ((flags1 & FLAGVAL_EXPORT) == FLAGVAL_EXPORT)
                   )
                {
                    flags1 |=  FLAGVAL_COMBINED;
                    flags1 &= ~FLAGVAL_IMPORT  ;
                    flags1 &= ~FLAGVAL_EXPORT  ;
                    aFilter.put(PROPNAME_FLAGS, new java.lang.Integer(flags1));
                }
            }

            // remove some obsolete filter flags
            if (bRemoveFilterFlagBrowserPreferred)
            {
                flags1 &= ~FLAGVAL_BROWSERPREFERRED;
                aFilter.put(PROPNAME_FLAGS, new java.lang.Integer(flags1));
            }

            if (bRemoveFilterFlag3rdparty)
            {
                flags1 &= ~FLAGVAL_3RDPARTYFILTER;
                aFilter.put(PROPNAME_FLAGS, new java.lang.Integer(flags1));
            }

            // if its a filter with an UI order ...
            // move this information to the registered type.
            // Note: Because more then one filter can be registered for the same type.
            // Handle it as an error ... till we find a better transformation!
            java.lang.Integer nOrder   = (java.lang.Integer)aFilter.get(PROPNAME_ORDER);
            java.lang.Integer nUIOrder = (java.lang.Integer)aType.get(PROPNAME_UIORDER);
            int               order    = nOrder.intValue();
            int               uiorder  = nUIOrder.intValue();

            if (order > 0)
            {
                if (
                    (uiorder < 1    ) ||
                    (uiorder > order)
                   )
                {
                    aType.put(PROPNAME_UIORDER, nOrder);
                    m_aDebug.setDetailedInfo("moved order value "+nOrder+" from filter \""+sFilter+"\" to type \""+sTypeReg+"\"");
                }
                else
                    m_aDebug.setDetailedInfo("ignore order value [order="+nOrder+",uiorder="+nUIOrder+"] for filter \""+sFilter+"\" and type \""+sTypeReg+"\"");
            }
        }

        // NOW ... remove the preferred flags from every filter, which it has set.
        java.util.Enumeration aIt2 = null;
        if (bRemoveFilterFlagPreferred)
        {
            aIt2 = lPreferredFilters.elements();
            while (aIt2.hasMoreElements())
            {
                java.util.HashMap aFilter = (java.util.HashMap)aIt2.nextElement();
                int               flags   = ((java.lang.Integer)aFilter.get(PROPNAME_FLAGS)).intValue();
                flags &= ~FLAGVAL_PREFERRED;
                aFilter.put(PROPNAME_FLAGS, new java.lang.Integer(flags));
            }
        }

        // NOW ... remove all "no real filters" like the graphich helper filters of
        // draw and impress!
        aIt2 = lNoRealFilters.elements();
        while (aIt2.hasMoreElements())
            m_lFilters.remove(aIt2.nextElement());

        // step over all detect services and move this information directly to
        // the corresponding types
        // Overwrite possibel default registrations with a real existing one!
        aIt1 = m_lDetectServices.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String      sDetector = (java.lang.String)aIt1.next();
            java.util.HashMap     aDetector = (java.util.HashMap)m_lDetectServices.get(sDetector);
            java.util.Vector      lTypeReg  = (java.util.Vector)aDetector.get(PROPNAME_TYPES);
            aIt2 = lTypeReg.elements();
            while(aIt2.hasMoreElements())
            {
                java.lang.String  sTypeReg                   = (java.lang.String)aIt2.nextElement();
                java.util.HashMap aType                      = (java.util.HashMap)m_lTypes.get(sTypeReg);

                if (aType == null)
                {
                    m_aDebug.setWarning("Detector \""+sDetector+"\" seem to be registered for unknown type \""+sTypeReg+"\"");
                    continue;
                }

                java.lang.Object  aAlreadyRegisteredDetector = aType.get(PROPNAME_DETECTSERVICE);
                if (aAlreadyRegisteredDetector != null && ((java.lang.String)aAlreadyRegisteredDetector).length() > 0)
                {
                    java.lang.StringBuffer sMsg = new java.lang.StringBuffer(256);
                    sMsg.append("type \""+sTypeReg+"\" has ambigous registrations of a detect service\n");
                    sMsg.append("\tdetect service[1] = \""+(java.lang.String)aAlreadyRegisteredDetector+"\"\n");
                    sMsg.append("\tdetect service[2] = \""+sDetector+"\"\n");
                    m_aDebug.setWarning(sMsg.toString());
                }
                aType.put(PROPNAME_DETECTSERVICE, sDetector);
                m_aDebug.setGlobalInfo("move detector \""+sDetector+"\" to type \""+sTypeReg+"\"");
            }
        }

        // because all detect service was registered as type properties directly ...
        // remove all detect service objects of this cache!
        m_lDetectServices.clear();

        // step over all frame loader and move this information directly to
        // the corresponding types
        // Overwrite possibel default registrations with a real existing one!
        aIt1 = m_lFrameLoaders.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String      sLoader  = (java.lang.String)aIt1.next();
            java.util.HashMap     aLoader  = (java.util.HashMap)m_lFrameLoaders.get(sLoader);
            java.util.Vector      lTypeReg = (java.util.Vector)aLoader.get(PROPNAME_TYPES);
            aIt2 = lTypeReg.elements();
            while(aIt2.hasMoreElements())
            {
                java.lang.String  sTypeReg                 = (java.lang.String)aIt2.nextElement();
                java.util.HashMap aType                    = (java.util.HashMap)m_lTypes.get(sTypeReg);
                java.lang.String  sAlreadyRegisteredLoader = (java.lang.String)aType.get(PROPNAME_FRAMELOADER);
                if (sAlreadyRegisteredLoader != null && sAlreadyRegisteredLoader.length() > 0)
                {
                    java.lang.StringBuffer sMsg = new java.lang.StringBuffer(256);
                    sMsg.append("type \""+sTypeReg+"\" has ambigous registrations of a frame loader\n");
                    sMsg.append("\tframe loader[1] = \""+sAlreadyRegisteredLoader+"\"\n");
                    sMsg.append("\tframe loader[2] = \""+sLoader+"\"\n");
                    m_aDebug.setWarning(sMsg.toString());
                }
                aType.put(PROPNAME_FRAMELOADER, sLoader);
                System.out.println("move loader \""+sLoader+"\" to type \""+sTypeReg+"\"");
            }
        }

        m_lFrameLoaders.clear();

        // step over all content handler and move this information directly to
        // the corresponding types
        // Overwrite possibel default registrations with a real existing one!
        aIt1 = m_lContentHandlers.keySet().iterator();
        while(aIt1.hasNext())
        {
            java.lang.String      sHandler = (java.lang.String)aIt1.next();
            java.util.HashMap     aHandler = (java.util.HashMap)m_lContentHandlers.get(sHandler);
            java.util.Vector      lTypeReg = (java.util.Vector)aHandler.get(PROPNAME_TYPES);
            aIt2 = lTypeReg.elements();
            while(aIt2.hasMoreElements())
            {
                java.lang.String  sTypeReg                  = (java.lang.String)aIt2.nextElement();
                java.util.HashMap aType                     = (java.util.HashMap)m_lTypes.get(sTypeReg);
                java.lang.String  sAlreadyRegisteredHandler = (java.lang.String)aType.get(PROPNAME_CONTENTHANDLER);
                if (sAlreadyRegisteredHandler != null && sAlreadyRegisteredHandler.length() > 0)
                {
                    java.lang.StringBuffer sMsg = new java.lang.StringBuffer(256);
                    sMsg.append("type \""+sTypeReg+"\" has ambigous registrations of a content handler\n");
                    sMsg.append("\tcontent handler[1] = \""+sAlreadyRegisteredHandler+"\"\n");
                    sMsg.append("\tcontent handler[2] = \""+sHandler+"\"\n");
                    m_aDebug.setWarning(sMsg.toString());
                }
                aType.put(PROPNAME_CONTENTHANDLER, sHandler);
                System.out.println("move handler \""+sHandler+"\" to type \""+sTypeReg+"\"");
            }
        }

        m_lContentHandlers.clear();

/*
        int c = m_lTypes.size();
        java.lang.String[] lT1 = new java.lang.String[c];
        java.lang.String[] lT2 = new java.lang.String[c];
        long nUPS = 0;

        int i = 0;
        aIt1 = m_lTypes.keySet().iterator();
        while(aIt1.hasNext())
        {
            lT1[i] = (java.lang.String)aIt1.next();
            lT2[i] = lT1[i];
            ++i;
        }

        for (int i1=0; i1<c; ++i1)
        {
            java.lang.String  sT1 = lT1[i1];
            java.util.HashMap aT1 = (java.util.HashMap)m_lTypes.get(sT1);

            for (int i2=i1; i2<c; ++i2)
            {
                java.lang.String  sT2 = lT1[i2];
                java.util.HashMap aT2 = (java.util.HashMap)m_lTypes.get(sT2);

                if (!sT1.equals(sT2))
                {
                    if (
                        aT1.get(PROPNAME_MEDIATYPE).equals(aT2.get(PROPNAME_MEDIATYPE)) &&
                        aT1.get(PROPNAME_CLIPBOARDFORMAT).equals(aT2.get(PROPNAME_CLIPBOARDFORMAT)) &&
                        aT1.get(PROPNAME_URLPATTERN).equals(aT2.get(PROPNAME_URLPATTERN)) &&
                        aT1.get(PROPNAME_EXTENSIONS).equals(aT2.get(PROPNAME_EXTENSIONS))
                       )
                    {
                        System.err.println("UUPPSS----------------------------------------------------------------------------");
                        System.err.println("[1]\""+sT1+"\" equals [2]\""+sT2+"\"");
                        System.err.println("\tprops 1: "+aT1);
                        System.err.println("\tprops 2: "+aT2);
                        System.err.println("----------------------------------------------------------------------------------\n");
                        ++nUPS;
//                        throw new java.lang.Exception("UUPPS :-)");
                    }
                }
            }
        }
        System.err.println("count of UPS = "+nUPS);
*/
    }
}

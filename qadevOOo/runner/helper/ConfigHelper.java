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

package helper;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.util.*;

/**
 * This <CODE>ConfigHelper</CODE> makes it possible to access the
 * configuration and change their content.<P>
 * <P>
 * Example: <P>
 * Listing of the <CODE>Configuration</CODE> Views.xcu:<P>
 * &lt;oor:component-data xmlns:oor=&quot;http://openoffice.org/2001/registry&quot; xmlns:xs=&quot;http://www.w3.org/2001/XMLSchema&quot; oor:name=&quot;Views&quot; oor:package=&quot;org.openoffice.Office&quot;&gt;<p>
 *   &lt;node oor:name=&quot;Windows&quot;&gt;<P>
 *       <&lt;node oor:name=&quot;SplitWindow0&quot; oor:op=&quot;replace&quot;&gt;<P>
 *           &lt;prop oor:name=&quot;Visible&quot; oor:type=&quot;xs:boolean&quot;&gt;<P>
 *                &lt;value&gt;false&lt;/value&gt;<P>
 *           &lt;/prop&gt;<P>
 *           &lt;prop oor:name=&quot;WindowState&quot; oor:type=&quot;xs:string&quot;&gt;<P>
 *               &lt;value/&gt;<P>
 *           &lt;/prop&gt;<P>
 *            &lt;node oor:name=&quot;UserData&quot;&gt;<P>
 *               &lt;prop oor:name=&quot;UserItem&quot; oor:op=&quot;replace&quot;
 *                oor:type=&quot;xs:string&quot;&gt;<P>
 *                   &lt;value&gt;V1,2,0&lt;/value&gt;<P>
 *               &lt;/prop&gt;<P>
 *           &lt;/node&gt;<P>
 *        &lt;/node&gt;<P>
 *  &lt;/node&gt;<P>
 * <P>
 * <CODE>Definition</CODE><P>
 * <ul>
 *    <li><CODE>&lt;node oor:name=&quot;Windows&quot;&gt;</CODE>
 *        represents a <CODE>Set</CODE> and is a <CODE>XNameContainer</CODE></LI>
 *    <li><CODE>&lt;node oor:name=&quot;SplitWindow0&quot;&gt;</CODE>
 *        represents a <CODE>Group</CODE> and is a <CODE>XNameReplace</CODE></LI>
 *    <li><CODE>&lt;prop oor:name=&quot;Visible&quot;&gt;</CODE>
 *        represents a pr<CODE></CODE>operty of the group</li>
 *    <li><CODE>&lt;node oor:name=&quot;UserData&quot;&gt;</CODE>
 *        represents a <CODE>extensible group</CODE> and is a <CODE>XNameContainer</CODE></LI>
 *    <li><CODE>&lt;prop oor:name=&quot;UserItem&quot;&gt;</CODE>
 *        represents a <CODE>property</CODE> of the extensible group</LI>
 * </UL>
 * We assume in the following examples the existance of:<P>
 * <CODE>ConfigHelper aConfig = new ConfigHelper(xMSF, "org.openoffice.Office.Views", false);</CODE>
 * <ul>
 *    <li>If you like to insert a new <CODE>Group</CODE> into the <CODE>Set</CODE> "Windows":<p>
 *        <CODE>XNameReplace xMyGroup = aConfig.getOrInsertGroup("Windows", "myGroup");</CODE><P>
 *        The method <CODE>getOrInsertGroup()</CODE> uses the
 *        <CODE>XSingleServiceFactory</CODE> to create the skeleton of a new group.
 *
 *    </li>
 *    <li>If you like to change the property "WindowState" of "myGroup"
 *        of the Set "Windows"<p>
 *        <CODE>aConfig.updateGroupProperty(
 *          "Windows","myGroup", "WindowState", "952,180,244,349;1;0,0,0,0;");</CODE>
 *    </li>
 *    <li>If you like to change the property "myProp" of the extensible group
 *        "myExtGroup" which is an extensible group of "my2ndGroup" of the
 *        Set "Windows":<p>
 *        <CODE>aConfig.insertOrUpdateExtensibleGroupProperty(
 *              "Windows", "my2ndGroup", "myExtGroup", "myProp","TheValue");</CODE>
 *    </li>
 * </ul>
 */
public class ConfigHelper
{
    private XMultiServiceFactory m_xSMGR = null;
    private XHierarchicalNameAccess m_xConfig = null;


    public ConfigHelper(XMultiServiceFactory xSMGR       ,
                        String               sConfigPath ,
                        boolean              bReadOnly   )
        throws com.sun.star.uno.Exception
    {
        m_xSMGR = xSMGR;

        XMultiServiceFactory xConfigRoot = UnoRuntime.queryInterface(
        XMultiServiceFactory.class,
        m_xSMGR.createInstance(
        "com.sun.star.configuration.ConfigurationProvider"));

        PropertyValue[] lParams = new PropertyValue[1];
        lParams[0] = new PropertyValue();
        lParams[0].Name  = "nodepath";
        lParams[0].Value = sConfigPath;

        Object aConfig;
        if (bReadOnly)
            aConfig = xConfigRoot.createInstanceWithArguments(
                            "com.sun.star.configuration.ConfigurationAccess",
                            lParams);
        else
            aConfig = xConfigRoot.createInstanceWithArguments(
                            "com.sun.star.configuration.ConfigurationUpdateAccess",
                            lParams);

        m_xConfig = UnoRuntime.queryInterface(
                            XHierarchicalNameAccess.class,
                            aConfig);

        if (m_xConfig == null)
            throw new com.sun.star.uno.Exception("Could not open configuration \""+sConfigPath+"\"");
    }


    public Object readRelativeKey(String sRelPath,
                                  String sKey    )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(
                                    XPropertySet.class,
                                    m_xConfig.getByHierarchicalName(sRelPath));
            return xPath.getPropertyValue(sKey);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }


    public void writeRelativeKey(String sRelPath,
                                 String sKey    ,
                                 Object aValue  )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(
                                    XPropertySet.class,
                                    m_xConfig.getByHierarchicalName(sRelPath));
            xPath.setPropertyValue(sKey, aValue);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }


    /**
     * Updates the configuration.<p>
     * This must be called after you have changed the configuration
     * else you changes will be lost.
     */
    public void flush()
    {
        try
        {
            XChangesBatch xBatch = UnoRuntime.queryInterface(
                                        XChangesBatch.class,
                                        m_xConfig);
            xBatch.commitChanges();
        }
        catch(com.sun.star.uno.Exception ex)
        {}
    }


    public static Object readDirectKey(XMultiServiceFactory xSMGR      ,
                                       String               sConfigFile,
                                       String               sRelPath   ,
                                       String               sKey       )
        throws com.sun.star.uno.Exception
    {
        ConfigHelper aConfig = new ConfigHelper(xSMGR, sConfigFile, true);
        return aConfig.readRelativeKey(sRelPath, sKey);
    }


    public static void writeDirectKey(XMultiServiceFactory xSMGR      ,
                                      String               sConfigFile,
                                      String               sRelPath   ,
                                      String               sKey       ,
                                      Object               aValue     )
        throws com.sun.star.uno.Exception
    {
        ConfigHelper aConfig = new ConfigHelper(xSMGR, sConfigFile, false);
        aConfig.writeRelativeKey(sRelPath, sKey, aValue);
        aConfig.flush();
    }


    /**
     * Insert a structured node (group) in a name container (set)
     * or else update it and retrun the <CODE>XNameReplace</CODE> of it.<P>
     * The <CODE>XSingleServiceFacttory</CODE> of the <CODE>set</CODE> will be used
     * to create a new group. This group is specific to its set and
     * creates defined entries.
     * @return The [inserted] group of the set
     * @param groupName The name of the goup which should be returned
     * @param setName The name of the set
     * @throws com.sun.star.uno.Exception throws
     *         <CODE>com.sun.star.uno.Exeception</CODE> on any error.
     */
    public XNameReplace getOrInsertGroup(String setName, String groupName)
        throws  com.sun.star.uno.Exception

    {
        XNameContainer xSetCont = this.getSet(setName);

        XNameReplace xChildAccess = null;

        try {
            xSetCont.getByName(groupName);
            xChildAccess = UnoRuntime.queryInterface(
                            XNameReplace.class,xSetCont);
        } catch(com.sun.star.container.NoSuchElementException e) {
             // proceed with inserting
        }

        if (xChildAccess == null)  {
            XSingleServiceFactory xChildfactory = UnoRuntime.queryInterface(XSingleServiceFactory.class,xSetCont);

            Object xNewChild = xChildfactory.createInstance();

            xSetCont.insertByName(groupName, xNewChild);

            xChildAccess = UnoRuntime.queryInterface(XNameContainer.class,xNewChild);
       }

        return xChildAccess;
    }

     /**
     * Update a property of a group container of a set container
     * @param setName the name of the <CODE>set</CODE> which containes the <CODE>group</CODE>
     * @param groupName the name of the <CODE>group</CODE> which property should be changed
     * @param propName the name of the property which should be changed
     * @param propValue the value the property should get
     * @throws com.sun.star.uno.Exception throws <CODE>com.sun.star.uno.Exeception</CODE> on any error.
     */
    public void updateGroupProperty(String setName,
                                    String groupName,
                                    String propName,
                                    Object propValue)
        throws  com.sun.star.uno.Exception
    {
        XNameContainer xSetCont = this.getSet(setName);

        XPropertySet xProp = null;
        try {
        xProp = UnoRuntime.queryInterface(
                                    XPropertySet.class,
                                    xSetCont.getByName(groupName));
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new com.sun.star.uno.Exception(
                "could not get group '" + groupName +
               "' from set '"+ setName +"':\n" + e.toString());
        }
        try{
            xProp.setPropertyValue(propName, propValue);
        } catch (com.sun.star.uno.Exception e) {
             throw new com.sun.star.uno.Exception(
                "could not set property '" + propName +
                "' from group '"+ groupName +
                "' from set '"+ setName +"':\n" + e.toString());
        }
    }


    /**
     * Insert a property in an extensible group container or else update it
     * @param setName the name of the <CODE>set</CODE> which containes the <CODE>group</CODE>
     * @param group The name of the <CODE>group</CODE> which conatins the <CODE>extensible group</CODE>.
     * @param extGroup The name of the <CODE>extensible group</CODE> which
     *                  [should] contain the property
     * @param propName The name of the property.
     * @param propValue The value of the property.
     * @throws com.sun.star.uno.Exception throws <CODE>com.sun.star.uno.Exeception</CODE> on any error.
     */
    public void insertOrUpdateExtensibleGroupProperty(
                    String setName,
                    String group,
                    String extGroup,
                    String propName,
                    Object propValue)
        throws  com.sun.star.uno.Exception
    {
        XNameContainer xSetCont = this.getSet(setName);

        XNameReplace xGroupAccess = null;
        XNameContainer xExtGroupCont = null;

        try {
            Object xGroup=xSetCont.getByName(group);
            xGroupAccess = UnoRuntime.queryInterface(
                            XNameReplace.class,xGroup);
        } catch(com.sun.star.container.NoSuchElementException e) {
             throw new com.sun.star.uno.Exception(
                "could not get group '" + group +
                "' from set '"+ setName +"':\n" + e.toString());
        }

        try {
            Object xGroup=xGroupAccess.getByName(extGroup);
            xExtGroupCont = UnoRuntime.queryInterface(
                            XNameContainer.class,xGroup);
        } catch(com.sun.star.container.NoSuchElementException e) {
             throw new com.sun.star.uno.Exception(
                "could not get extensilbe group '"+extGroup+
                "' from group '"+ group +
                "' from set '"+ setName +"':\n" + e.toString());
        }

        try {
            xExtGroupCont.insertByName(propName, propValue);
        }
        catch(com.sun.star.container.ElementExistException e) {
            xExtGroupCont .replaceByName(propName, propValue);
        }

    }


    /**
     * Returns a <CODE>XNameContainer</CODE> of the <CODE>Set</CODE>
     * of the <CODE>Configuration</CODE>
     * @param setName the name of the Set which sould be returned
     * @throws com.sun.star.uno.Exception on any error
     * @return A XNameContainer of the Set
     */
    public XNameContainer getSet(String setName)
        throws com.sun.star.uno.Exception
    {
        XNameReplace xCont = UnoRuntime.queryInterface(XNameReplace.class, m_xConfig);

        Object oSet = xCont.getByName(setName);

        if (oSet == null)
             throw new com.sun.star.uno.Exception(
                "could not get set '" + setName + ": null");

        return UnoRuntime.queryInterface(
                                                XNameContainer.class, oSet);

    }
}

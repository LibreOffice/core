/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.web.data;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import com.sun.star.wizards.common.*;

import org.w3c.dom.Document;
import org.w3c.dom.Node;

public class CGSession extends ConfigSetItem implements XMLProvider
{

    public String cp_InDirectory;
    public String cp_OutDirectory;
    public String cp_Name;
    public CGContent cp_Content = new CGContent();
    public CGDesign cp_Design = new CGDesign();
    public CGGeneralInfo cp_GeneralInfo = new CGGeneralInfo();
    public ConfigSet cp_Publishing = new ConfigSet(CGPublish.class);
    public CGStyle style; // !!!
    public boolean valid = false;

    public Node createDOM(Node parent)
    {
        Node root = XMLHelper.addElement(parent, "session",
                new String[]
                {
                    "name", "screen-size"
                },
                new String[]
                {
                    cp_Name, getScreenSize()
                });

        //cp_Design.createDOM(root);
        cp_GeneralInfo.createDOM(root);
        //cp_Publishing.createDOM(root);
        cp_Content.createDOM(root);

        return root;
    }

    private String getScreenSize()
    {
        switch (cp_Design.cp_OptimizeDisplaySize)
        {
            case 0:
                return "640";
            case 1:
                return "800";
            case 2:
                return "1024";
            default:
                return "800";
        }
    }

    public CGLayout getLayout()
    {
        return (CGLayout) ((CGSettings) root).cp_Layouts.getElement(cp_Design.cp_Layout);
    }

    public CGStyle getStyle()
    {
        return (CGStyle) ((CGSettings) root).cp_Styles.getElement(cp_Design.cp_Style);
    }

    public void setLayout(short[] layout)
    {
        //dummy
    }

    public Node createDOM()
            throws ParserConfigurationException
    {

        DocumentBuilderFactory factory =
                DocumentBuilderFactory.newInstance();
        Document doc = factory.newDocumentBuilder().newDocument();
        createDOM(doc);
        return doc;
    }
}

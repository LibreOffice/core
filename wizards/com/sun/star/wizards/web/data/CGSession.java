/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGSession.java,v $
 * $Revision: 1.5 $
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

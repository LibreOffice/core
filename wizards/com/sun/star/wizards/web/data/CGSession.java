/*************************************************************************
 *
 *  $RCSfile: CGSession.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:18:21 $
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
 */

package com.sun.star.wizards.web.data;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import com.sun.star.wizards.common.*;

import org.w3c.dom.Document;
import org.w3c.dom.Node;


public class CGSession   extends ConfigSetItem implements XMLProvider {
    public String        cp_InDirectory;
    public String        cp_OutDirectory;
    public String        cp_Name;

    public CGContent     cp_Content = new CGContent();
    public CGDesign      cp_Design = new CGDesign();
    public CGGeneralInfo cp_GeneralInfo = new CGGeneralInfo();
    public ConfigSet     cp_Publishing = new ConfigSet(CGPublish.class);


    public CGStyle       style; // !!!
    public boolean       valid = false;

    public Node createDOM(Node parent) {
        Node root = XMLHelper.addElement(parent, "session",
            new String[] { "name" , "screen-size" },
            new String[] { cp_Name , getScreenSize() } );

        //cp_Design.createDOM(root);
        cp_GeneralInfo.createDOM(root);
        //cp_Publishing.createDOM(root);
        cp_Content.createDOM(root);

        return root;
    }

    private String getScreenSize() {
        switch (cp_Design.cp_OptimizeDisplaySize) {
            case 0 : return "640";
            case 1 : return "800";
            case 2 : return "1024";
            default : return "800";
        }
    }

    public CGLayout getLayout() {
        return (CGLayout)((CGSettings)root).cp_Layouts.getElement(cp_Design.cp_Layout);
    }

    public CGStyle getStyle() {
       return (CGStyle)((CGSettings)root).cp_Styles.getElement(cp_Design.cp_Style);
    }

    public void setLayout(short[] layout) {
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

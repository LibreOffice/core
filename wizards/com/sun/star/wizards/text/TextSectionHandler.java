/*************************************************************************
*
*  $RCSfile: TextSectionHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:49:13 $
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

package com.sun.star.wizards.text;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.SectionFileLink;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

public class TextSectionHandler {

    public XTextSectionsSupplier xTextSectionsSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;

    /** Creates a new instance of TextSectionHandler */
    public TextSectionHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xTextSectionsSupplier = (XTextSectionsSupplier) UnoRuntime.queryInterface(XTextSectionsSupplier.class, xTextDocument);
    }

    public void removeTextSectionbyName(String SectionName) {
        try {
            com.sun.star.container.XNameAccess xAllTextSections = xTextSectionsSupplier.getTextSections();
            if (xAllTextSections.hasByName(SectionName) == true) {
                Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
                XTextContent xTextContentTextSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextSection);
                xTextDocument.getText().removeTextContent(xTextContentTextSection);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public boolean hasTextSectionByName(String SectionName) {
        com.sun.star.container.XNameAccess xAllTextSections = xTextSectionsSupplier.getTextSections();
        if (xAllTextSections.hasByName(SectionName) == true) {
            return true;
        } else {
            return false;
        }
    }

    public void removeLastTextSection() {
        try {
            XIndexAccess xAllTextSections = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            Object oTextSection = xAllTextSections.getByIndex(xAllTextSections.getCount() - 1);
            XTextContent xTextContentTextSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextSection);
            xTextDocument.getText().removeTextContent(xTextContentTextSection);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void removeAllTextSections() {
        try {
            Object oTextSection;
            com.sun.star.text.XText xText = xTextDocument.getText();
            XIndexAccess xAllTextSections = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            int TextSectionCount = xAllTextSections.getCount();
            for (int i = TextSectionCount - 1; i >= 0; i--) {
                oTextSection = xAllTextSections.getByIndex(i);
                XTextContent xTextContentTextSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextSection);
                xText.removeTextContent(xTextContentTextSection);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void breakLinkofTextSections() {
        try {
            Object oTextSection;
            XIndexAccess xAllTextSections = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            int iSectionCount = xAllTextSections.getCount();
            SectionFileLink oSectionLink = new SectionFileLink();
            oSectionLink.FileURL = "";
            for (int i = 0; i < iSectionCount; i++) {
                oTextSection = xAllTextSections.getByIndex(i);
                Helper.setUnoPropertyValues(oTextSection, new String[] { "FileLink", "LinkRegion" }, new Object[] { oSectionLink, "" });
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void linkSectiontoTemplate(String TemplateName, String SectionName) {
        try {
            Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
            linkSectiontoTemplate(oTextSection, TemplateName, SectionName);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public void linkSectiontoTemplate(Object oTextSection, String TemplateName, String SectionName) {
        SectionFileLink oSectionLink = new SectionFileLink();
        oSectionLink.FileURL = TemplateName;
        Helper.setUnoPropertyValue(oTextSection, "FileLink", oSectionLink);
        Helper.setUnoPropertyValue(oTextSection, "LinkRegion", SectionName);
        XNamed xSectionName = (XNamed) UnoRuntime.queryInterface(XNamed.class, oTextSection);
        String NewSectionName = xSectionName.getName();
        if (NewSectionName.compareTo(SectionName) != 0)
            xSectionName.setName(SectionName);
    }

    public void insertTextSection(String GroupName, String TemplateName) {
        try {
            Object xTextSection;
            if (xTextSectionsSupplier.getTextSections().hasByName(GroupName) == true)
                xTextSection = xTextSectionsSupplier.getTextSections().getByName(GroupName);
            else {
                xTextSection = xMSFDoc.createInstance("com.sun.star.text.TextSection");
                XTextContent xTextContentSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextSection);
                com.sun.star.text.XTextCursor xTextCursor = xTextDocument.getText().createTextCursor();
                xTextCursor.gotoEnd(false);
                xTextCursor.getText().insertTextContent(xTextCursor, xTextContentSection, false);
            }
            linkSectiontoTemplate(xTextSection, TemplateName, GroupName);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }
}

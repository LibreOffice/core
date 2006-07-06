/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextSectionHandler.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:31:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.text;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.SectionFileLink;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

public class TextSectionHandler {

    public XTextSectionsSupplier xTextSectionsSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;
    private XText xText;

    /** Creates a new instance of TextSectionHandler */
    public TextSectionHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xText = xTextDocument.getText();
        xTextSectionsSupplier = (XTextSectionsSupplier) UnoRuntime.queryInterface(XTextSectionsSupplier.class, xTextDocument);
    }

    public void removeTextSectionbyName(String SectionName) {
        try {
            XNameAccess xAllTextSections = xTextSectionsSupplier.getTextSections();
            if (xAllTextSections.hasByName(SectionName) == true) {
                Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
                removeTextSection(oTextSection);
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
            removeTextSection(oTextSection);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void removeTextSection(Object _oTextSection){
    try{
        XTextContent xTextContentTextSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, _oTextSection);
        xText.removeTextContent(xTextContentTextSection);
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }}

    public void removeAllTextSections() {
        removeAllTextSections(false);
    }

    public void removeAllTextSections(boolean _bRemoveVisibleOnly) {
    try {
        boolean bRemoveTextSection = !_bRemoveVisibleOnly;
        XIndexAccess xAllTextSections = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
        int TextSectionCount = xAllTextSections.getCount();
        for (int i = TextSectionCount - 1; i >= 0; i--) {
            XTextContent xTextContentTextSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xAllTextSections.getByIndex(i));
            if (_bRemoveVisibleOnly){
                XPropertySet xTextSectionPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextContentTextSection);
                bRemoveTextSection = AnyConverter.toBoolean(xTextSectionPropertySet.getPropertyValue("IsVisible"));
            }
            if (bRemoveTextSection)
                xText.removeTextContent(xTextContentTextSection);
        }
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }}


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

    public void breakLinkOfTextSection(Object oTextSection) {
        SectionFileLink oSectionLink = new SectionFileLink();
        oSectionLink.FileURL = "";
        Helper.setUnoPropertyValues(oTextSection, new String[] { "FileLink", "LinkRegion" }, new Object[] { oSectionLink, "" });
    }

    public void linkSectiontoTemplate(String TemplateName, String SectionName) {
        try {
            Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
            linkSectiontoTemplate(oTextSection, TemplateName, SectionName);
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
    }

    public void linkSectiontoTemplate(Object oTextSection, String TemplateName, String SectionName) {
        SectionFileLink oSectionLink = new SectionFileLink();
        oSectionLink.FileURL = TemplateName;
        Helper.setUnoPropertyValues(oTextSection, new String[] { "FileLink", "LinkRegion" }
        , new Object[] { oSectionLink, SectionName });
        XNamed xSectionName = (XNamed) UnoRuntime.queryInterface(XNamed.class, oTextSection);
        String NewSectionName = xSectionName.getName();
        if (NewSectionName.compareTo(SectionName) != 0)
            xSectionName.setName(SectionName);
    }

    public void insertTextSection(String GroupName, String TemplateName, boolean _bAddParagraph) {
    try {
        if (_bAddParagraph){
            XTextCursor xTextCursor = xText.createTextCursor();
            xText.insertControlCharacter(xTextCursor, ControlCharacter.PARAGRAPH_BREAK, false);
            //      Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
            xTextCursor.collapseToEnd();
        }
        XTextCursor xSecondTextCursor = xText.createTextCursor();
        xSecondTextCursor.gotoEnd(false);
        insertTextSection( GroupName, TemplateName, xSecondTextCursor );
    }
    catch (IllegalArgumentException e) {
        e.printStackTrace(System.out);
    }}

    public void insertTextSection( String sectionName, String templateName, XTextCursor position ) {
        try {
            Object xTextSection;
            if (xTextSectionsSupplier.getTextSections().hasByName(sectionName) == true)
                xTextSection = xTextSectionsSupplier.getTextSections().getByName(sectionName);
            else {
                xTextSection = xMSFDoc.createInstance("com.sun.star.text.TextSection");
                XTextContent xTextContentSection = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextSection);
                position.getText().insertTextContent(position , xTextContentSection, false);
            }
            linkSectiontoTemplate(xTextSection, templateName, sectionName);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }
}

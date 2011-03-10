/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.wizards.text;

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
import com.sun.star.wizards.common.PropertyNames;

public class TextSectionHandler
{

    public XTextSectionsSupplier xTextSectionsSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;
    private XText xText;

    /** Creates a new instance of TextSectionHandler */
    public TextSectionHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument)
    {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xText = xTextDocument.getText();
        xTextSectionsSupplier = UnoRuntime.queryInterface(XTextSectionsSupplier.class, xTextDocument);
    }

    public void removeTextSectionbyName(String SectionName)
    {
        try
        {
            XNameAccess xAllTextSections = xTextSectionsSupplier.getTextSections();
            if (xAllTextSections.hasByName(SectionName))
            {
                Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
                removeTextSection(oTextSection);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public boolean hasTextSectionByName(String SectionName)
    {
        com.sun.star.container.XNameAccess xAllTextSections = xTextSectionsSupplier.getTextSections();
        return xAllTextSections.hasByName(SectionName);
    }

    public void removeLastTextSection()
    {
        try
        {
            XIndexAccess xAllTextSections = UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            Object oTextSection = xAllTextSections.getByIndex(xAllTextSections.getCount() - 1);
            removeTextSection(oTextSection);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void removeTextSection(Object _oTextSection)
    {
        try
        {
            XTextContent xTextContentTextSection = UnoRuntime.queryInterface(XTextContent.class, _oTextSection);
            xText.removeTextContent(xTextContentTextSection);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void removeInvisibleTextSections()
    {
        try
        {
            XIndexAccess xAllTextSections = UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            int TextSectionCount = xAllTextSections.getCount();
            for (int i = TextSectionCount - 1; i >= 0; i--)
            {
                XTextContent xTextContentTextSection = UnoRuntime.queryInterface(XTextContent.class, xAllTextSections.getByIndex(i));
                XPropertySet xTextSectionPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xTextContentTextSection);
                boolean bRemoveTextSection = (!AnyConverter.toBoolean(xTextSectionPropertySet.getPropertyValue("IsVisible")));
                if (bRemoveTextSection)
                {
                    xText.removeTextContent(xTextContentTextSection);
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void removeAllTextSections()
    {
        try
        {
            XIndexAccess xAllTextSections = UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            int TextSectionCount = xAllTextSections.getCount();
            for (int i = TextSectionCount - 1; i >= 0; i--)
            {
                XTextContent xTextContentTextSection = UnoRuntime.queryInterface(XTextContent.class, xAllTextSections.getByIndex(i));
                XPropertySet xTextSectionPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xTextContentTextSection);
                xText.removeTextContent(xTextContentTextSection);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void breakLinkofTextSections()
    {
        try
        {
            Object oTextSection;
            XIndexAccess xAllTextSections = UnoRuntime.queryInterface(XIndexAccess.class, xTextSectionsSupplier.getTextSections());
            int iSectionCount = xAllTextSections.getCount();
            SectionFileLink oSectionLink = new SectionFileLink();
            oSectionLink.FileURL = PropertyNames.EMPTY_STRING;
            for (int i = 0; i < iSectionCount; i++)
            {
                oTextSection = xAllTextSections.getByIndex(i);
                Helper.setUnoPropertyValues(oTextSection, new String[]
                        {
                            "FileLink", "LinkRegion"
                        }, new Object[]
                        {
                            oSectionLink, PropertyNames.EMPTY_STRING
                        });
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void breakLinkOfTextSection(Object oTextSection)
    {
        SectionFileLink oSectionLink = new SectionFileLink();
        oSectionLink.FileURL = PropertyNames.EMPTY_STRING;
        Helper.setUnoPropertyValues(oTextSection, new String[]
                {
                    "FileLink", "LinkRegion"
                }, new Object[]
                {
                    oSectionLink, PropertyNames.EMPTY_STRING
                });
    }

    public void linkSectiontoTemplate(String TemplateName, String SectionName)
    {
        try
        {
            Object oTextSection = xTextSectionsSupplier.getTextSections().getByName(SectionName);
            linkSectiontoTemplate(oTextSection, TemplateName, SectionName);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void linkSectiontoTemplate(Object oTextSection, String TemplateName, String SectionName)
    {
        SectionFileLink oSectionLink = new SectionFileLink();
        oSectionLink.FileURL = TemplateName;
        Helper.setUnoPropertyValues(oTextSection, new String[]
                {
                    "FileLink", "LinkRegion"
                }, new Object[]
                {
                    oSectionLink, SectionName
                });
        XNamed xSectionName = UnoRuntime.queryInterface(XNamed.class, oTextSection);
        String NewSectionName = xSectionName.getName();
        if (NewSectionName.compareTo(SectionName) != 0)
        {
            xSectionName.setName(SectionName);
        }
    }

    public void insertTextSection(String GroupName, String TemplateName, boolean _bAddParagraph)
    {
        try
        {
            if (_bAddParagraph)
            {
                XTextCursor xTextCursor = xText.createTextCursor();
                xText.insertControlCharacter(xTextCursor, ControlCharacter.PARAGRAPH_BREAK, false);
                //      Helper.setUnoPropertyValue(xTextCursor, "PageDescName", "First Page");
                xTextCursor.collapseToEnd();
            }
            XTextCursor xSecondTextCursor = xText.createTextCursor();
            xSecondTextCursor.gotoEnd(false);
            insertTextSection(GroupName, TemplateName, xSecondTextCursor);
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void insertTextSection(String sectionName, String templateName, XTextCursor position)
    {
        try
        {
            Object xTextSection;
            if (xTextSectionsSupplier.getTextSections().hasByName(sectionName))
            {
                xTextSection = xTextSectionsSupplier.getTextSections().getByName(sectionName);
            }
            else
            {
                xTextSection = xMSFDoc.createInstance("com.sun.star.text.TextSection");
                XTextContent xTextContentSection = UnoRuntime.queryInterface(XTextContent.class, xTextSection);
                position.getText().insertTextContent(position, xTextContentSection, false);
            }
            linkSectiontoTemplate(xTextSection, templateName, sectionName);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
}

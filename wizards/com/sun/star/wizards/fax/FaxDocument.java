/*
 ************************************************************************
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
package com.sun.star.wizards.fax;

import com.sun.star.wizards.common.*;
import com.sun.star.wizards.text.*;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.text.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.XPropertySet;
import com.sun.star.style.NumberingType;
import com.sun.star.style.ParagraphAdjust;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyle;

public class FaxDocument extends TextDocument
{

    XDesktop xDesktop;
    boolean keepLogoFrame = true;
    boolean keepTypeFrame = true;

    public FaxDocument(XMultiServiceFactory xMSF, XTerminateListener listener)
    {
        super(xMSF, listener, "WIZARD_LIVE_PREVIEW");
    }

    public XWindowPeer getWindowPeer()
    {
        return UnoRuntime.queryInterface(XWindowPeer.class, xTextDocument);
    }

    public void switchElement(String sElement, boolean bState)
    {
        try
        {
            TextSectionHandler mySectionHandler = new TextSectionHandler(xMSF, xTextDocument);
            Object oSection = mySectionHandler.xTextSectionsSupplier.getTextSections().getByName(sElement);
            Helper.setUnoPropertyValue(oSection, "IsVisible", Boolean.valueOf(bState));

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void updateDateFields()
    {
        TextFieldHandler FH = new TextFieldHandler(xMSFDoc, xTextDocument);
        FH.updateDateFields();
    }

    public void switchFooter(String sPageStyle, boolean bState, boolean bPageNumber, String sText)
    {
        if (xTextDocument != null)
        {
            xTextDocument.lockControllers();
            try
            {
                XStyleFamiliesSupplier xStyleFamiliesSupplier = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
                com.sun.star.container.XNameAccess xNameAccess = null;
                xNameAccess = xStyleFamiliesSupplier.getStyleFamilies();

                com.sun.star.container.XNameContainer xPageStyleCollection = null;
                xPageStyleCollection = UnoRuntime.queryInterface(com.sun.star.container.XNameContainer.class, xNameAccess.getByName("PageStyles"));

                XText xFooterText;
                XStyle xPageStyle = UnoRuntime.queryInterface(XStyle.class, xPageStyleCollection.getByName(sPageStyle));

                if (bState)
                {
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", Boolean.TRUE);
                    xFooterText = UnoRuntime.queryInterface(XText.class, Helper.getUnoPropertyValue(xPageStyle, "FooterText"));
                    xFooterText.setString(sText);
                    if (bPageNumber)
                    {
                        //Adding the Page Number
                        XTextCursor myCursor = xFooterText.createTextCursor();
                        myCursor.gotoEnd(false);
                        xFooterText.insertControlCharacter(myCursor, ControlCharacter.PARAGRAPH_BREAK, false);
                        XPropertySet xCursorPSet = UnoRuntime.queryInterface(XPropertySet.class, myCursor);
                        xCursorPSet.setPropertyValue("ParaAdjust", ParagraphAdjust.CENTER);
                        XTextField xPageNumberField = UnoRuntime.queryInterface(XTextField.class, xMSFDoc.createInstance("com.sun.star.text.TextField.PageNumber"));
                        XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xPageNumberField);
                        xPSet.setPropertyValue("SubType", PageNumberType.CURRENT);
                        xPSet.setPropertyValue("NumberingType", new Short(NumberingType.ARABIC));
                        xFooterText.insertTextContent(xFooterText.getEnd(), xPageNumberField, false);
                    }
                }
                else
                {
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", Boolean.FALSE);
                }
                xTextDocument.unlockControllers();
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }
    }

    public boolean hasElement(String sElement)
    {
        if (xTextDocument != null)
        {
            TextSectionHandler mySectionHandler = new TextSectionHandler(xMSF, xTextDocument);
            return mySectionHandler.hasTextSectionByName(sElement);
        }
        else
        {
            return false;
        }
    }

    public void switchUserField(String sFieldName, String sNewContent, boolean bState)
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(xMSF, xTextDocument);
        if (bState)
        {
            myFieldHandler.changeUserFieldContent(sFieldName, sNewContent);
        }
        else
        {
            myFieldHandler.changeUserFieldContent(sFieldName, PropertyNames.EMPTY_STRING);
        }
    }

    public void fillSenderWithUserData()
    {
        try
        {
            TextFieldHandler myFieldHandler = new TextFieldHandler(xMSFDoc, xTextDocument);
            Object oUserDataAccess = Configuration.getConfigurationRoot(xMSF, "org.openoffice.UserProfile/Data", false);
            myFieldHandler.changeUserFieldContent("Company", (String) Helper.getUnoObjectbyName(oUserDataAccess, "o"));
            myFieldHandler.changeUserFieldContent("Street", (String) Helper.getUnoObjectbyName(oUserDataAccess, "street"));
            myFieldHandler.changeUserFieldContent("PostCode", (String) Helper.getUnoObjectbyName(oUserDataAccess, "postalcode"));
            myFieldHandler.changeUserFieldContent(PropertyNames.PROPERTY_STATE, (String) Helper.getUnoObjectbyName(oUserDataAccess, "st"));
            myFieldHandler.changeUserFieldContent("City", (String) Helper.getUnoObjectbyName(oUserDataAccess, "l"));
            myFieldHandler.changeUserFieldContent("Fax", (String) Helper.getUnoObjectbyName(oUserDataAccess, "facsimiletelephonenumber"));
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void killEmptyUserFields()
    {
        TextFieldHandler myFieldHandler = new TextFieldHandler(xMSF, xTextDocument);
        myFieldHandler.removeUserFieldByContent(PropertyNames.EMPTY_STRING);
    }

    public void killEmptyFrames()
    {
        try
        {
            if (!keepLogoFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Company Logo", xTextDocument);
                if (xTF != null)
                {
                    xTF.dispose();
                }
            }
            if (!keepTypeFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Communication Type", xTextDocument);
                if (xTF != null)
                {
                    xTF.dispose();
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }
}

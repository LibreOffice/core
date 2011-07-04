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
package com.sun.star.wizards.letter;

import com.sun.star.wizards.common.*;
import com.sun.star.wizards.text.*;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.table.BorderLine;
import com.sun.star.text.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.drawing.XShape;
import com.sun.star.beans.XPropertySet;
import com.sun.star.style.NumberingType;
import com.sun.star.style.ParagraphAdjust;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyle;

public class LetterDocument extends TextDocument
{

    XDesktop xDesktop;
    boolean keepLogoFrame = true;
    boolean keepBendMarksFrame = true;
    boolean keepLetterSignsFrame = true;
    boolean keepSenderAddressRepeatedFrame = true;
    boolean keepAddressFrame = true;

    public LetterDocument(XMultiServiceFactory xMSF, XTerminateListener listener)
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
            try
            {
                xTextDocument.lockControllers();
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
            TextSectionHandler SH = new TextSectionHandler(xMSF, xTextDocument);
            return SH.hasTextSectionByName(sElement);
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
            myFieldHandler.changeUserFieldContent("City", (String) Helper.getUnoObjectbyName(oUserDataAccess, "l"));
            myFieldHandler.changeUserFieldContent(PropertyNames.PROPERTY_STATE, (String) Helper.getUnoObjectbyName(oUserDataAccess, "st"));
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
            if (!keepBendMarksFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Bend Marks", xTextDocument);
                if (xTF != null)
                {
                    xTF.dispose();
                }
            }
            if (!keepLetterSignsFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Letter Signs", xTextDocument);
                if (xTF != null)
                {
                    xTF.dispose();
                }
            }
            if (!keepSenderAddressRepeatedFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Sender Address Repeated", xTextDocument);
                if (xTF != null)
                {
                    xTF.dispose();
                }
            }
            if (!keepAddressFrame)
            {
                XTextFrame xTF = TextFrameHandler.getFrameByName("Sender Address", xTextDocument);
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

    public class BusinessPaperObject
    {

        public int iWidth;
        public int iHeight;
        public int iXPos;
        public int iYPos;
        XTextFrame xFrame;
        XShape xShape;

        public BusinessPaperObject(String FrameText, int Width, int Height, int XPos, int YPos)
        {

            iWidth = Width;
            iHeight = Height;
            iXPos = XPos;
            iYPos = YPos;

            try
            {
                xFrame = UnoRuntime.queryInterface(XTextFrame.class, xMSFDoc.createInstance("com.sun.star.text.TextFrame"));
                xShape = UnoRuntime.queryInterface(XShape.class, xFrame);

                setFramePosition();
                Helper.setUnoPropertyValue(xShape, "AnchorType", TextContentAnchorType.AT_PAGE);
                Helper.setUnoPropertyValue(xShape, "SizeType", new Short(SizeType.FIX));

                Helper.setUnoPropertyValue(xFrame, "TextWrap", WrapTextMode.THROUGHT);
                Helper.setUnoPropertyValue(xFrame, "Opaque", Boolean.TRUE);
                Helper.setUnoPropertyValue(xFrame, "BackColor", 15790320);

                BorderLine myBorder = new BorderLine();
                myBorder.OuterLineWidth = 0;
                Helper.setUnoPropertyValue(xFrame, "LeftBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "RightBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "TopBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "BottomBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "Print", Boolean.FALSE);

                XTextCursor xTextCursor = xTextDocument.getText().createTextCursor();
                xTextCursor.gotoEnd(true);
                XText xText = xTextDocument.getText();
                xText.insertTextContent(xTextCursor, xFrame, false);

                XText xFrameText = xFrame.getText();
                XTextCursor xFrameCursor = xFrameText.createTextCursor();
                XPropertySet xCursorProps = UnoRuntime.queryInterface(XPropertySet.class, xFrameCursor);
                xCursorProps.setPropertyValue("CharWeight", new Float(com.sun.star.awt.FontWeight.BOLD));
                xCursorProps.setPropertyValue("CharColor", 16777215);
                xCursorProps.setPropertyValue("CharFontName", "Albany");
                xCursorProps.setPropertyValue("CharHeight", new Float(18));

                xFrameText.insertString(xFrameCursor, FrameText, false);

            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }

        public void setFramePosition()
        {
            Helper.setUnoPropertyValue(xFrame, "HoriOrient", new Short(HoriOrientation.NONE));
            Helper.setUnoPropertyValue(xFrame, "VertOrient", new Short(VertOrientation.NONE));
            Helper.setUnoPropertyValue(xFrame, PropertyNames.PROPERTY_HEIGHT, new Integer(iHeight));
            Helper.setUnoPropertyValue(xFrame, PropertyNames.PROPERTY_WIDTH, new Integer(iWidth));
            Helper.setUnoPropertyValue(xFrame, "HoriOrientPosition", new Integer(iXPos));
            Helper.setUnoPropertyValue(xFrame, "VertOrientPosition", new Integer(iYPos));
            Helper.setUnoPropertyValue(xFrame, "HoriOrientRelation", new Short(RelOrientation.PAGE_FRAME));
            Helper.setUnoPropertyValue(xFrame, "VertOrientRelation", new Short(RelOrientation.PAGE_FRAME));
        }

        public void removeFrame()
        {
            if (xFrame != null)
            {
                try
                {
                    xTextDocument.getText().removeTextContent(xFrame);
                }
                catch (Exception e)
                {
                    e.printStackTrace(System.out);
                }
            }
        }
    }
}

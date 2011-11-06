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

package com.sun.star.wizards.letter;

import com.sun.star.wizards.common.*;
import com.sun.star.wizards.document.*;
import com.sun.star.wizards.text.*;
import com.sun.star.frame.XDesktop;
import com.sun.star.table.BorderLine;
import com.sun.star.text.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyValue;
import com.sun.star.drawing.XShape;
import com.sun.star.beans.XPropertySet;
import com.sun.star.awt.Size;
import com.sun.star.style.NumberingType;
import com.sun.star.style.ParagraphAdjust;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyle;

public class LetterDocument extends TextDocument {

    XDesktop xDesktop;
    XMultiServiceFactory xDocMSF;
    XTextDocument xTextDocument;
    Size DocSize = null;

    public LetterDocument(XMultiServiceFactory xMSF) {
        super(xMSF);
    }

    public XTextDocument loadTemplate(String sDefaultTemplate) {
        PropertyValue loadValues[] = new PropertyValue[3];
        //      open document in the Preview mode
        loadValues[0] = new PropertyValue();
        loadValues[0].Name = "ReadOnly";
        loadValues[0].Value = Boolean.TRUE;
        loadValues[1] = new PropertyValue();
        loadValues[1].Name = "AsTemplate";
        loadValues[1].Value = Boolean.FALSE;
        loadValues[2] = new PropertyValue();
        loadValues[2].Name = "Preview";
        loadValues[2].Value = Boolean.TRUE;


        Object oDoc = OfficeDocument.load(xFrame, sDefaultTemplate, "_self", loadValues);
        xTextDocument = (com.sun.star.text.XTextDocument) oDoc;
        DocSize = getPageSize();
        xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDocument);

        ViewHandler myViewHandler = new ViewHandler(xDocMSF, xTextDocument);
        try {
            myViewHandler.setViewSetting("ZoomType", new Short(com.sun.star.view.DocumentZoomType.ENTIRE_PAGE));
        } catch (Exception e) {
            e.printStackTrace();
        }

        return xTextDocument;
    }

    public XWindowPeer getWindowPeer() {
        XWindowPeer xWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xTextDocument);
        return xWindowPeer;
    }

    public void switchElement(String sElement, boolean bState) {
        try {
            TextSectionHandler mySectionHandler = new TextSectionHandler(xMSF, xTextDocument);
            Object oSection = mySectionHandler.xTextSectionsSupplier.getTextSections().getByName(sElement);
            Helper.setUnoPropertyValue(oSection, "IsVisible", new Boolean(bState));

        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void switchFooter(String sPageStyle, boolean bState, boolean bPageNumber, String sText) {
        if (xTextDocument != null) {
            try {
                XStyleFamiliesSupplier xStyleFamiliesSupplier = (XStyleFamiliesSupplier) com.sun.star.uno.UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
                com.sun.star.container.XNameAccess xNameAccess = null;
                xNameAccess = xStyleFamiliesSupplier.getStyleFamilies();

                com.sun.star.container.XNameContainer xPageStyleCollection = null;
                xPageStyleCollection = (com.sun.star.container.XNameContainer) UnoRuntime.queryInterface(com.sun.star.container.XNameContainer.class, xNameAccess.getByName("PageStyles"));

                XText xFooterText;
                XStyle xPageStyle = (XStyle) UnoRuntime.queryInterface(XStyle.class, xPageStyleCollection.getByName(sPageStyle));

                if (bState) {
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", new Boolean(true));
                    xFooterText = (XText) UnoRuntime.queryInterface(XText.class, Helper.getUnoPropertyValue(xPageStyle, "FooterText"));
                    xFooterText.setString(sText);
                    if (bPageNumber) {
                        //Adding the Page Number
                        XTextCursor myCursor = xFooterText.createTextCursor();
                        myCursor.gotoEnd(false);
                        xFooterText.insertControlCharacter(myCursor, ControlCharacter.PARAGRAPH_BREAK, false);
                        XPropertySet xCursorPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, myCursor);
                        //XTextField xPageNumberField = (XTextField) UnoRuntime.queryInterface(XTextField.class, xDocMSF.createInstance("com.sun.star.style.ParagraphProperties"));
                        xCursorPSet.setPropertyValue("ParaAdjust", ParagraphAdjust.CENTER);
                        XTextField xPageNumberField = (XTextField) UnoRuntime.queryInterface(XTextField.class, xDocMSF.createInstance("com.sun.star.text.TextField.PageNumber"));
                        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xPageNumberField);
                        xPSet.setPropertyValue("SubType", PageNumberType.CURRENT);
                        xPSet.setPropertyValue("NumberingType", new Short (NumberingType.ARABIC));
                        xFooterText.insertTextContent(xFooterText.getEnd(), xPageNumberField, false);
                    }
                } else {
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", new Boolean(false));
                }
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }
    }

    public boolean hasElement(String sElement) {
        if (xTextDocument != null) {
            TextSectionHandler mySectionHandler = new TextSectionHandler(xMSF, xTextDocument);
            return mySectionHandler.hasTextSectionByName(sElement);
        } else {
            return false;
        }
    }

    public void switchUserField(String sFieldName, String sNewContent, boolean bState) {
        TextFieldHandler myFieldHandler = new TextFieldHandler(xMSF, xTextDocument);
        if (bState) {
            myFieldHandler.changeUserFieldContent(sFieldName, sNewContent);
        } else {
            myFieldHandler.changeUserFieldContent(sFieldName, "");
        }
    }

    public void fillSenderWithUserData() {
        try {
            TextFieldHandler myFieldHandler = new TextFieldHandler(xDocMSF, xTextDocument);
            Object oUserDataAccess = Configuration.getConfigurationRoot(xMSF, "org.openoffice.UserProfile/Data", false);
            myFieldHandler.changeUserFieldContent("Company", (String) Helper.getUnoObjectbyName(oUserDataAccess, "o"));
            myFieldHandler.changeUserFieldContent("Street", (String) Helper.getUnoObjectbyName(oUserDataAccess, "street"));
            myFieldHandler.changeUserFieldContent("PostCode", (String) Helper.getUnoObjectbyName(oUserDataAccess, "postalcode"));
            myFieldHandler.changeUserFieldContent("City", (String) Helper.getUnoObjectbyName(oUserDataAccess, "l"));
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void killEmptyUserFields() {
        TextFieldHandler myFieldHandler = new TextFieldHandler(xMSF, xTextDocument);
        myFieldHandler.removeUserFieldByContent("Salutation");
        myFieldHandler.removeUserFieldByContent("Greeting");
    }

    public void loadResult(String sLoadURL, boolean bAsTemplate) {
        PropertyValue loadValues[] = new PropertyValue[1];
        loadValues[0] = new PropertyValue();
        loadValues[0].Name = "AsTemplate";
        loadValues[0].Value = new Boolean(bAsTemplate);
        String sFrame = "_self";
        OfficeDocument.load(xDesktop, sLoadURL, sFrame, loadValues);
    }

    public Size getPageSize() {
        try {
            XStyleFamiliesSupplier xStyleFamiliesSupplier = (XStyleFamiliesSupplier) com.sun.star.uno.UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
            com.sun.star.container.XNameAccess xNameAccess = null;
            xNameAccess = xStyleFamiliesSupplier.getStyleFamilies();
            com.sun.star.container.XNameContainer xPageStyleCollection = null;
            xPageStyleCollection = (com.sun.star.container.XNameContainer) UnoRuntime.queryInterface(com.sun.star.container.XNameContainer.class, xNameAccess.getByName("PageStyles"));
            XStyle xPageStyle = (XStyle) UnoRuntime.queryInterface(XStyle.class, xPageStyleCollection.getByName("First Page"));
            return (Size) Helper.getUnoPropertyValue(xPageStyle, "Size");

        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public class BusinessPaperObject {

        public int iWidth;
        public int iHeight;
        public int iXPos;
        public int iYPos;

        XTextFrame xFrame;
        XShape xShape;

        public BusinessPaperObject(String FrameText, int Width, int Height, int XPos, int YPos) {

            iWidth = Width;
            iHeight = Height;
            iXPos = XPos;
            iYPos = YPos;

            try {
                xFrame = (XTextFrame) UnoRuntime.queryInterface(XTextFrame.class, xDocMSF.createInstance("com.sun.star.text.TextFrame"));
                xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xFrame);

                setFramePosition();
                Helper.setUnoPropertyValue(xShape, "AnchorType", TextContentAnchorType.AT_PAGE);
                Helper.setUnoPropertyValue(xShape, "SizeType", new Short(SizeType.FIX));

                Helper.setUnoPropertyValue(xFrame, "TextWrap", WrapTextMode.THROUGHT);
                Helper.setUnoPropertyValue(xFrame, "Opaque", Boolean.FALSE);
                Helper.setUnoPropertyValue(xFrame, "BackColor", new Integer(15790320));

                BorderLine myBorder = new BorderLine();
                myBorder.OuterLineWidth = 0;
                Helper.setUnoPropertyValue(xFrame, "LeftBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "RightBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "TopBorder", myBorder);
                Helper.setUnoPropertyValue(xFrame, "BottomBorder", myBorder);

                XTextCursor xTextCursor = xTextDocument.getText().createTextCursor();
                xTextCursor.gotoEnd(true);
                XText xText = xTextDocument.getText();
                xText.insertTextContent(xTextCursor, xFrame, false);

                XText xFrameText = xFrame.getText();
                XTextCursor xFrameCursor = xFrameText.createTextCursor();
                XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xFrameCursor);
                xCursorProps.setPropertyValue("CharWeight", new Float(com.sun.star.awt.FontWeight.BOLD));
                xCursorProps.setPropertyValue("CharColor", new Integer(16777215));
                xCursorProps.setPropertyValue("CharFontName", new String("Albany"));
                xCursorProps.setPropertyValue("CharHeight", new Float(18));

                xFrameText.insertString(xFrameCursor, FrameText, false);

            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
        }

        public void setFramePosition() {
            Helper.setUnoPropertyValue(xFrame, "HoriOrient", new Short(HoriOrientation.NONE));
            Helper.setUnoPropertyValue(xFrame, "VertOrient", new Short(VertOrientation.NONE));
            Helper.setUnoPropertyValue(xFrame, "Height", new Integer(iHeight));
            Helper.setUnoPropertyValue(xFrame, "Width", new Integer(iWidth));
            Helper.setUnoPropertyValue(xFrame, "HoriOrientPosition", new Integer(iXPos));
            Helper.setUnoPropertyValue(xFrame, "VertOrientPosition", new Integer(iYPos));
            Helper.setUnoPropertyValue(xFrame, "HoriOrientRelation", new Short(RelOrientation.PAGE_FRAME));
            Helper.setUnoPropertyValue(xFrame, "VertOrientRelation", new Short(RelOrientation.PAGE_FRAME));
        }

        public void removeFrame() {
            if (xFrame != null) {
                try {
                    xTextDocument.getText().removeTextContent(xFrame);
                } catch (Exception e) {
                    e.printStackTrace(System.out);
                }
            }
        }

    }

}

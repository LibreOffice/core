/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.form;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NoValidPathException;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.document.DatabaseControl;
import com.sun.star.wizards.document.GridControl;
import com.sun.star.wizards.document.TimeStampControl;
import com.sun.star.wizards.text.TextStyleHandler;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.event.XItemListenerAdapter;

public class StyleApplier
{

    private final XPropertySet xPageStylePropertySet;
    private final XMultiServiceFactory xMSF;
    private final XRadioButton optNoBorder;
    private final XRadioButton opt3DLook;
    private final XListBox lstStyles;
    private final FormDocument curFormDocument;
    private short iOldLayoutPos;
    private String[] StyleNames;
    private String[] FileNames;
    private static final int SOBACKGROUNDCOLOR = 0;
    private static final int SODBTEXTCOLOR = 1;
    private static final int SOLABELTEXTCOLOR = 2;
    private static final int SOBORDERCOLOR = 5;
    private Short IBorderValue = Short.valueOf((short) 1);

    public StyleApplier(WizardDialog CurUnoDialog, FormDocument _curFormDocument)
    {
            this.curFormDocument = _curFormDocument;
            xMSF = curFormDocument.xMSF;

            TextStyleHandler oTextStyleHandler = new TextStyleHandler(curFormDocument.xTextDocument);
            xPageStylePropertySet = oTextStyleHandler.getStyleByName("PageStyles", "Standard");
            short curtabindex = (short) (FormWizard.SOSTYLE_PAGE * 100);
            Integer IStyleStep = Integer.valueOf(FormWizard.SOSTYLE_PAGE);
            String sPageStyles = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 86);
            String sNoBorder = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 29);
            String s3DLook = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 30);
            String sFlat = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 31);
            String sFieldBorder = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 28);
            setStyles();
            short[] SelLayoutPos;
            SelLayoutPos = new short[]
                    {
                        0
                    };

            CurUnoDialog.insertLabel("lblStyles",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], sPageStyles, 92, 25, IStyleStep, Short.valueOf(curtabindex++), 90
                    });

            lstStyles = CurUnoDialog.insertListBox("lstStyles", null, new XItemListenerAdapter() {
                        @Override
                        public void itemStateChanged(ItemEvent event) {
                            changeLayout();
                        }
                    },
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.SELECTED_ITEMS, PropertyNames.PROPERTY_STEP, PropertyNames.STRING_ITEM_LIST, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        143, "HID:WIZARDS_HID_DLGFORM_LSTSTYLES", 92, 35, SelLayoutPos, IStyleStep, this.StyleNames, Short.valueOf(curtabindex++), 90
                    });

            optNoBorder = CurUnoDialog.insertRadioButton("otpNoBorder", new XItemListenerAdapter() {
                        @Override
                        public void itemStateChanged(ItemEvent event) {
                            changeBorderLayouts();
                        }
                    },
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDNOBORDER", sNoBorder, 196, 39, IStyleStep, Short.valueOf(curtabindex++), "0", 93
                    });

            opt3DLook = CurUnoDialog.insertRadioButton("otp3DLook", new XItemListenerAdapter() {
                        @Override
                        public void itemStateChanged(ItemEvent event) {
                            changeBorderLayouts();
                        }
                    },
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMD3DBORDER", s3DLook, 196, 53, Short.valueOf((short) 1), IStyleStep, Short.valueOf(curtabindex++), "1", 93
                    });

            CurUnoDialog.insertRadioButton("otpFlat", new XItemListenerAdapter() {
                        @Override
                        public void itemStateChanged(ItemEvent event) {
                            changeBorderLayouts();
                        }
                    },
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[10], "HID:WIZARDS_HID_DLGFORM_CMDSIMPLEBORDER", sFlat, 196, 67, IStyleStep, Short.valueOf(curtabindex++), "2", 93
                    });

            CurUnoDialog.insertFixedLine("lnFieldBorder",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        UIConsts.INTEGERS[8], sFieldBorder, 192, 25, IStyleStep, Short.valueOf(curtabindex++), 98
                    });
    }

    private void setStyles()
    {
        try
        {
            Object oRootNode = Configuration.getConfigurationRoot(xMSF, "org.openoffice.Office.FormWizard/FormWizard/Styles", false);
            XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, oRootNode);
            String[] StyleNodeNames = xNameAccess.getElementNames();
            StyleNames = new String[StyleNodeNames.length];
            FileNames = new String[StyleNodeNames.length];
            for (int i = 0; i < StyleNodeNames.length; i++)
            {
                Object oStyleNode = xNameAccess.getByName(StyleNodeNames[i]);
                StyleNames[i] = (String) Helper.getUnoPropertyValue(oStyleNode, PropertyNames.PROPERTY_NAME);
                FileNames[i] = (String) Helper.getUnoPropertyValue(oStyleNode, "CssHref");
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private short getStyleIndex()
    {
        try
        {
            short[] SelFields = (short[]) AnyConverter.toArray(Helper.getUnoPropertyValue(UnoDialog.getModel(lstStyles), PropertyNames.SELECTED_ITEMS));
            if (SelFields != null)
            {
                return SelFields[0];
            }
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.err);
        }
        return (short) -1;
    }

    public void applyStyle(boolean _bapplyalways)
    {
        short iStyle = getStyleIndex();
        if ((iStyle != iOldLayoutPos) || _bapplyalways)
        {
            if (iStyle > -1)
            {
                iOldLayoutPos = iStyle;
                String sFileName = FileNames[iStyle];
                int[] iStyles = getStyleColors(sFileName);
                applyDBControlProperties(iStyles);
            }
        }
    }

    private void changeLayout()
    {
        short iPos = lstStyles.getSelectedItemPos();
        if (iPos != iOldLayoutPos)
        {
            iOldLayoutPos = iPos;
            String sFileName = FileNames[iPos];
            int[] iStyles = getStyleColors(sFileName);
            applyDBControlProperties(iStyles);
        }
        curFormDocument.unlockallControllers();
    }

    public Short getBorderType()
    {
        return IBorderValue;
    }

    private void changeBorderLayouts()
    {
        try
        {
            curFormDocument.xTextDocument.lockControllers();

            if (optNoBorder.getState())
            {
                IBorderValue = Short.valueOf((short) 0);
            }
            else if (opt3DLook.getState())
            {
                IBorderValue = Short.valueOf((short) 1);
            }
            else
            {
                IBorderValue = Short.valueOf((short) 2);
            }
            for (int m = 0; m < curFormDocument.oControlForms.size(); m++)
            {
                FormDocument.ControlForm curControlForm = curFormDocument.oControlForms.get(m);
                if (curControlForm.getArrangemode() == FormWizard.AS_GRID)
                {
                    GridControl oGridControl = curControlForm.getGridControl();
                    oGridControl.xPropertySet.setPropertyValue(PropertyNames.PROPERTY_BORDER, IBorderValue);
                }
                else
                {
                    DatabaseControl[] DBControls = curControlForm.getDatabaseControls();
                    for (int n = 0; n < DBControls.length; n++)
                    {
                        if (DBControls[n].xServiceInfo.supportsService("com.sun.star.drawing.ShapeCollection"))
                        {
                            TimeStampControl oTimeStampControl = (TimeStampControl) DBControls[n];
                            for (int i = 0; i < 2; i++)
                            {
                                XPropertySet xPropertySet = oTimeStampControl.getControlofGroupShapeByIndex(i);
                                if (xPropertySet == null) {
                                    continue;
                                }
                                if (xPropertySet.getPropertySetInfo().hasPropertyByName(PropertyNames.PROPERTY_BORDER))
                                {
                                    xPropertySet.setPropertyValue(PropertyNames.PROPERTY_BORDER, IBorderValue);
                                }
                            }
                        }
                        else
                        {
                            if (DBControls[n].xPropertySet.getPropertySetInfo().hasPropertyByName(PropertyNames.PROPERTY_BORDER))
                            {
                                DBControls[n].xPropertySet.setPropertyValue(PropertyNames.PROPERTY_BORDER, IBorderValue);
                            }
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        curFormDocument.unlockallControllers();
    }



    private int getStyleColor(String[] _sDataList, String _sHeader, String _sPropertyDescription)
    {
        int index = JavaTools.FieldInList(_sDataList, _sHeader);
        if (index > -1)
        {
            String sPropName = PropertyNames.EMPTY_STRING;
            while (((sPropName.indexOf('}') < 0) && (index < _sDataList.length - 1)))
            {
                String scurline = _sDataList[index++];
                if ((scurline.indexOf(_sPropertyDescription)) > 0)
                {
                    if (scurline.indexOf(':') > 0)
                    {
                        String[] sPropList = JavaTools.ArrayoutofString(scurline, ":");
                        String sPropValue = sPropList[1];
                        sPropValue = sPropValue.trim();
                        if (sPropValue.indexOf('#') > -1)
                        {
                            sPropValue = JavaTools.replaceSubString(sPropValue, PropertyNames.EMPTY_STRING, PropertyNames.SEMI_COLON);
                            sPropValue = JavaTools.replaceSubString(sPropValue, PropertyNames.EMPTY_STRING, PropertyNames.SPACE);
                            return Integer.decode(sPropValue).intValue();
                        }
                    }
                }
            }
        }
        return -1;
    }

    private String getStylePath()
    {
        String StylesPath = "";
        try
        {
            StylesPath = FileAccess.getOfficePath(xMSF, "Config", "", "");
            StylesPath = FileAccess.combinePaths(xMSF, StylesPath, "/wizard/form/styles");
        }
        catch (NoValidPathException e)
        {
        }
        return StylesPath;
    }

    private int[] getStyleColors(String _filename)
    {
        String sFilePath = getStylePath() + "/" + _filename;
        int[] oStylePropList = new int[6];
        String[] sData = FileAccess.getDataFromTextFile(xMSF, sFilePath);
        oStylePropList[SOBACKGROUNDCOLOR] = getStyleColor(sData, ".toctitle {", "background-color:");
        oStylePropList[SODBTEXTCOLOR] = getStyleColor(sData, ".doctitle {", "color:");
        oStylePropList[SOLABELTEXTCOLOR] = getStyleColor(sData, ".toctitle {", "color:");
        oStylePropList[SOBORDERCOLOR] = getStyleColor(sData, ".tcolor {", "border-color:");
        return oStylePropList;
    }

    private void setDBControlColors(XPropertySet xPropertySet, int[] _iStyleColors)
    {
        try
        {
            if (xPropertySet.getPropertySetInfo().hasPropertyByName("TextColor"))
            {
                if (_iStyleColors[SODBTEXTCOLOR] > -1)
                {
                    xPropertySet.setPropertyValue("TextColor", Integer.decode("#00000"));
                }
            }
            if (xPropertySet.getPropertySetInfo().hasPropertyByName("BackgroundColor"))
            {
                xPropertySet.setPropertyValue("BackgroundColor", Integer.decode("#DDDDDD"));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private void applyDBControlProperties(int[] _iStyleColors)
    {
        try
        {
            for (int m = 0; m < curFormDocument.oControlForms.size(); m++)
            {
                FormDocument.ControlForm curControlForm = curFormDocument.oControlForms.get(m);
                if (curControlForm.getArrangemode() == FormWizard.AS_GRID)
                {
                    if (_iStyleColors[SOLABELTEXTCOLOR] > -1)
                    {
                        curControlForm.oGridControl.xPropertySet.setPropertyValue("TextColor", Integer.valueOf(_iStyleColors[SODBTEXTCOLOR]));
                    }
                    curControlForm.oGridControl.xPropertySet.setPropertyValue("BackgroundColor", Integer.decode("#DDDDDD"));
                }
                else
                {
                    DatabaseControl[] DBControls = curControlForm.getDatabaseControls();
                    for (int n = 0; n < DBControls.length; n++)
                    {
                        if (_iStyleColors[SODBTEXTCOLOR] > -1)
                        {
                            DatabaseControl aDBControl = DBControls[n];
                            if (aDBControl != null)
                            {
                                if (aDBControl.xServiceInfo.supportsService("com.sun.star.drawing.ShapeCollection"))
                                {
                                    TimeStampControl oTimeStampControl = (TimeStampControl) aDBControl;
                                    for (int i = 0; i < 2; i++)
                                    {
                                        XPropertySet xPropertySet = oTimeStampControl.getControlofGroupShapeByIndex(i);
                                        if (xPropertySet == null) {
                                            continue;
                                        }
                                        setDBControlColors(xPropertySet, _iStyleColors);
                                    }
                                }
                                else
                                {
                                    setDBControlColors(aDBControl.xPropertySet, _iStyleColors);
                                }
                            }
                        }
                    }
                    Control[] LabelControls = curControlForm.getLabelControls();
                    for (int n = 0; n < LabelControls.length; n++)
                    {
                        if (_iStyleColors[SOLABELTEXTCOLOR] > -1)
                        {
                            LabelControls[n].xPropertySet.setPropertyValue("TextColor", Integer.valueOf(_iStyleColors[SOLABELTEXTCOLOR]));
                        }
                    }
                }
            }
            xPageStylePropertySet.setPropertyValue("BackColor", Integer.valueOf(_iStyleColors[SOBACKGROUNDCOLOR]));
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }
}

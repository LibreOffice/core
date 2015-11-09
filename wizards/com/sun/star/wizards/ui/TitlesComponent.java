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
package com.sun.star.wizards.ui;

import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.*;
import com.sun.star.beans.*;
import com.sun.star.awt.Size;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XTextListener;

import java.util.*;

public class TitlesComponent extends ControlScroller
{

    private int iLabelPosX;
    private static final int iRelLabelPosXDist = 6;
    private String[] fieldnames;
    private Map<String, String> fieldtitleset;
    private String[] fieldtitles;
    private static final String SOLABELPREFIX = "lblColumnName_";
    private static final String SOTITLEPREFIX = "lblTitleName_";

    public TitlesComponent(WizardDialog _CurUnoDialog, int _iStep, int _iCompPosX, int _iCompPosY, int _iCompWidth, int _uitextfieldcount, String _slblColumnNames, String _slblColumnTitles, int _firsthelpindex)
    {
        super(_CurUnoDialog, _iStep, _iCompPosX, _iCompPosY, _iCompWidth, _uitextfieldcount, 18, _firsthelpindex);
        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblColumnNames", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                    8, _slblColumnNames, Integer.valueOf(iLabelPosX), Integer.valueOf(iCompPosY - 10), IStep, 60
                });
        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblColumnTitles", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                    8, _slblColumnTitles, 90, Integer.valueOf(iCompPosY - 10), IStep, 152
                });
    }

    @Override
    protected void insertControlGroup(int i, int ypos)
    {
        iLabelPosX = iCompPosX + iRelLabelPosXDist;
        ControlRow oControlRow = new ControlRow(i, ypos);
        ControlGroupVector.add(oControlRow);

    }

    @Override
    protected void setControlGroupVisible(int _index, boolean _bIsVisible)
    {
        CurUnoDialog.setControlVisible(getColumnName(_index), _bIsVisible);
        CurUnoDialog.setControlVisible(getTitleName(_index), _bIsVisible);
    }

    public void addTextListener(XTextListener _xTextListener)
    {
        for (int i = 0; i < super.ControlGroupVector.size(); i++)
        {
            ControlRow curControlRow = (ControlRow) ControlGroupVector.get(i);
            XTextComponent xTextBox = curControlRow.xTextComponent;
            if ((xTextBox != null) && (_xTextListener != null))
            {
                xTextBox.addTextListener(_xTextListener);
            }
        }
    }

    class ControlRow
    {

        private XTextComponent xTextComponent;

        public ControlRow(int _index, int _iCompPosY)
        {
            String slabelname = getColumnName(_index);
            String stextfieldname = getTitleName(_index);

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", slabelname,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        16, Boolean.TRUE, Integer.valueOf(iLabelPosX), Integer.valueOf(_iCompPosY + 1), UIConsts.INVISIBLESTEP, Short.valueOf(curtabindex++), 30
                    });

            xTextComponent = CurUnoDialog.insertTextField(stextfieldname, 0, null,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        12, HelpIds.getHelpIdString(curHelpIndex++), Integer.valueOf(iLabelPosX + 30), Integer.valueOf(_iCompPosY), UIConsts.INVISIBLESTEP, Short.valueOf(curtabindex++), Integer.valueOf(iCompWidth - 90 - 20)
                    });
        }
    }

    private String getColumnName(int _index)
    {
        return SOLABELPREFIX + Integer.toString(_index + 1);
    }

    private String getTitleName(int _index)
    {
        return SOTITLEPREFIX + Integer.toString(_index + 1);
    }

    @Override
    protected void initializeScrollFields()
    {
        for (int i = 0; i < fieldnames.length; i++)
        {
            PropertyValue[] currowproperties = new PropertyValue[2];
            currowproperties[0] = Properties.createProperty(getColumnName(i), fieldnames[i], i);
            currowproperties[1] = Properties.createProperty(getTitleName(i), fieldtitles[i], i); //getTitleName(i)
            registerControlGroup(currowproperties, i);
        }
    }

    private void adjustPosSizes()
    {
        String sLongestFieldName = JavaTools.getlongestArrayItem(fieldnames);
        Size aSize = CurUnoDialog.getpreferredLabelSize(getColumnName(0), sLongestFieldName);
        double dblMAPConversion = CurUnoDialog.getMAPConversionFactor(getColumnName(0));
        int iFieldNameWidth = getFieldNameWidth(aSize.Width, dblMAPConversion) + 10;
        Integer FieldNameWidth = Integer.valueOf(iFieldNameWidth);
        Integer TitlePosX = Integer.valueOf(iLabelPosX + iFieldNameWidth + 2);
        Integer TitleWidth = Integer.valueOf(iCompPosX + iCompWidth - TitlePosX.intValue() - iScrollBarWidth - 6);
        for (short i = 0; i <= ncurfieldcount; i++)
        {
            CurUnoDialog.setControlProperty(getColumnName(i), PropertyNames.PROPERTY_WIDTH, FieldNameWidth);
            CurUnoDialog.setControlProperties(getTitleName(i), new String[]
                    {
                        PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_WIDTH
                    }, new Object[]
                    {
                        TitlePosX, TitleWidth
                    });
        }
        CurUnoDialog.setControlProperty("lblColumnNames", PropertyNames.PROPERTY_WIDTH, FieldNameWidth);
        CurUnoDialog.setControlProperty("lblColumnTitles", PropertyNames.PROPERTY_POSITION_X, TitlePosX);
    }

    public void initialize(String[] _fieldnames, Map<String, String> _fieldtitleset)
    {
        this.fieldnames = _fieldnames;
        this.fieldtitleset = _fieldtitleset;
        setFieldTitles();
        initializeScrollFields();
        super.initialize(fieldnames.length);
        adjustPosSizes();
    }

    private void setFieldTitles()
    {
        fieldtitles = new String[fieldnames.length];
        for (int i = 0; i < fieldnames.length; i++)
        {
            if (this.fieldtitleset.containsKey(fieldnames[i]))
            {
                Object curvalue = fieldtitleset.get(fieldnames[i]);
                if (curvalue != null)
                {
                    fieldtitles[i] = (String) curvalue;
                }
                else
                {
                    fieldtitles[i] = fieldnames[i];
                }
            }
            else
            {
                fieldtitles[i] = fieldnames[i];
            }
        }
    }

    public String getFieldNameByTitleControl(Object _fieldtitlemodel)
    {
        String sTitleModelName = (String) Helper.getUnoPropertyValue(_fieldtitlemodel, PropertyNames.PROPERTY_NAME);
        String sindex = JavaTools.getSuffixNumber(sTitleModelName);
        return (String) CurUnoDialog.getControlProperty(SOLABELPREFIX + sindex, PropertyNames.PROPERTY_LABEL);
    }

    public String[] getFieldTitles()
    {
        PropertyValue[][] titlelist = this.getScrollFieldValues();
        PropertyValue[] currowproperties;
        PropertyValue curtitleproperty;
        for (int i = 0; i < titlelist.length; i++)
        {
            currowproperties = titlelist[i];
            curtitleproperty = currowproperties[1];
            fieldtitles[i] = (String) curtitleproperty.Value;
        }
        refreshtitleset();
        return fieldtitles;
    }

    private void refreshtitleset()
    {
        for (int i = 0; i < fieldnames.length; i++)
        {
            fieldtitleset.put(fieldnames[i], fieldtitles[i]);
        }
    }

    private int getFieldNameWidth(int iMAPControlWidth, double dblMAPConversion)
    {
        int iFieldNameWidth = (int) (1.15 * (iMAPControlWidth) / dblMAPConversion);
        double dblAvailableWidth = (iCompWidth - iScrollBarWidth - iRelLabelPosXDist);
        if (iFieldNameWidth > (0.5 * (dblAvailableWidth)))
        {
            iFieldNameWidth = (int) (0.5 * (dblAvailableWidth));
        }
        return iFieldNameWidth;
    }
}

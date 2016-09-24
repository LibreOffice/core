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

import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.wizards.common.*;

import java.util.*;

public abstract class ControlScroller
{

    protected WizardDialog CurUnoDialog;
    protected int ncurfieldcount;
    private int nblockincrement;
    private int nlineincrement;
    private int nscrollvalue = 0;
    private int ntotfieldcount;
    private XScrollBar xScrollBar;
    private ArrayList<PropertyValue[]> scrollfields;
    protected int iCompPosX;
    protected int iCompPosY;
    protected int iCompWidth;
    protected int iCompHeight;
    protected short curtabindex;
    protected Integer IStep;
    int iScrollBarWidth = 10;
    private int SORELFIRSTPOSY = 3;
    protected int curHelpIndex;
    private String sIncSuffix;
    protected ArrayList<Object> ControlGroupVector = new ArrayList<Object>();

    private class AdjustmentListenerImpl implements com.sun.star.awt.XAdjustmentListener
    {

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }

        public void adjustmentValueChanged(AdjustmentEvent AdjustEvent)
        {
            scrollControls();
        }
    }

    /**
     *
     * @param _CurUnoDialog
     * @param _iStep
     * @param _iCompPosX
     * @param _iCompPosY
     * @param _iCompWidth
     * @param _nblockincrement
     * @param _firsthelpindex
     */
    // TODO add parameters for tabindices and helpindex
    protected ControlScroller(WizardDialog _CurUnoDialog, int _iStep, int _iCompPosX, int _iCompPosY, int _iCompWidth, int _nblockincrement, int _nlinedistance, int _firsthelpindex)
    {
        this.nblockincrement = _nblockincrement;
        this.CurUnoDialog = _CurUnoDialog;
        this.curHelpIndex = _firsthelpindex;
        curtabindex = UnoDialog.setInitialTabindex(_iStep);
        IStep = Integer.valueOf(_iStep);
        this.iCompPosX = _iCompPosX;
        this.iCompPosY = _iCompPosY;
        this.iCompWidth = _iCompWidth;
        this.iCompHeight = 2 * SORELFIRSTPOSY + nblockincrement * _nlinedistance;
        int iStartPosY = iCompPosY + SORELFIRSTPOSY;
        int ScrollHeight = iCompHeight - 2;
        nlineincrement = 1;
        sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.getDlgNameAccess(), "TitleScrollBar");
        xScrollBar = CurUnoDialog.insertScrollBar("TitleScrollBar" + sIncSuffix, 0,
                new AdjustmentListenerImpl(),
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Short.valueOf((short) 0), Boolean.TRUE, Integer.valueOf(ScrollHeight), HelpIds.getHelpIdString(curHelpIndex), Integer.valueOf(ScrollBarOrientation.VERTICAL), Integer.valueOf(iCompPosX + iCompWidth - iScrollBarWidth - 1), Integer.valueOf(iCompPosY + 1), IStep, Integer.valueOf(iScrollBarWidth)
                });
        scrollfields = new ArrayList<PropertyValue[]>();
        int ypos = iStartPosY + SORELFIRSTPOSY;
        for (int i = 0; i < nblockincrement; i++)
        {
            insertControlGroup(i, ypos);
            ypos += _nlinedistance;
        }
    }



    /**
     * @param _ntotfieldcount The number of fields that are to be administered by the ControlScroller
     */
    protected void initialize(int _ntotfieldcount)
    {
        try
        {
            ntotfieldcount = _ntotfieldcount;
            setCurFieldCount();
            nscrollvalue = 0;
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue", Integer.valueOf(nscrollvalue));
            if (ntotfieldcount > nblockincrement)
            {
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, "BlockIncrement", "LineIncrement", "ScrollValue", "ScrollValueMax"
                        }, new Object[]
                        {
                            Boolean.TRUE, Integer.valueOf(nblockincrement), Integer.valueOf(nlineincrement), Integer.valueOf(nscrollvalue), Integer.valueOf(ntotfieldcount - nblockincrement)
                        });
            }
            else
            {
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, "ScrollValue"
                        }, new Object[]
                        {
                            Boolean.FALSE, Integer.valueOf(nscrollvalue)
                        });
            }
            fillupControls(true);
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
        }
    }

    private void fillupControls(boolean binitialize)
    {
        for (int a = 0; a < this.nblockincrement; a++)
        {
            if (a < ncurfieldcount)
            {
                fillupControls(a);
            }
            if (binitialize)
            {
                setControlGroupVisible(a, (a < this.ncurfieldcount));
            }
        }
        if (binitialize)
        {
            CurUnoDialog.repaintDialogStep();
        }
    }

    private void fillupControls(int guiRow)
    {
        PropertyValue[] nameProps = scrollfields.get(guiRow);
        PropertyValue[] valueProps = scrollfields.get(guiRow + nscrollvalue);
        for (int n = 0; n < nameProps.length; n++)
        {
            if (CurUnoDialog.getDlgNameAccess().hasByName(nameProps[n].Name))
            {
                setControlData(nameProps[n].Name, valueProps[n].Value);
            }
            else
            {
                throw new IllegalArgumentException("No such control !");
            }
        }
    }

    private void setScrollValue(int _nscrollvalue)
    {
        if (_nscrollvalue >= 0)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue", Integer.valueOf(_nscrollvalue));
            scrollControls();
        }
    }

    protected void setScrollValue(int _nscrollvalue, int _ntotfieldcount)
    {
        setTotalFieldCount(_ntotfieldcount);
        setScrollValue(_nscrollvalue);
    }

    protected int getTotalFieldCount()
    {
        return ntotfieldcount;
    }

    protected int getCurFieldCount()
    {
        return ncurfieldcount;
    }

    private void setCurFieldCount()
    {
        if (ntotfieldcount > nblockincrement)
        {
            ncurfieldcount = nblockincrement;
        }
        else
        {
            ncurfieldcount = ntotfieldcount;
        }
    }

    protected void setTotalFieldCount(int _ntotfieldcount)
    {
        this.ntotfieldcount = _ntotfieldcount;
        setCurFieldCount();
        if (ntotfieldcount > nblockincrement)
        {
            Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, "ScrollValueMax"
                    }, new Object[]
                    {
                        Boolean.TRUE, Integer.valueOf(ntotfieldcount - nblockincrement)
                    });
        }
        else
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        }
    }

    protected void toggleComponent(boolean _bdoenable)
    {
        boolean bdoenable = _bdoenable && (ntotfieldcount > nblockincrement);
        CurUnoDialog.setControlProperty("TitleScrollBar" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenable));
    }

    protected void toggleControls(boolean _bdoenable)
    {
        for (int n = 0; n < scrollfields.size(); n++)
        {
            PropertyValue[] curproperties = scrollfields.get(n);
            for (int m = 0; m < curproperties.length; m++)
            {
                PropertyValue curproperty = curproperties[m];
                CurUnoDialog.setControlProperty(curproperty.Name, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
            }
        }

    }

    protected int getScrollValue()
    {
        return nscrollvalue;
    }







    protected int getBlockIncrementation()
    {
        return nblockincrement;
    }

    private void scrollControls()
    {
        try
        {
            scrollRowsInfo();
            nscrollvalue = ((Integer) Helper.getUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue")).intValue();
            if (nscrollvalue + nblockincrement >= ntotfieldcount)
            {
                nscrollvalue = (ntotfieldcount) - nblockincrement;
            }
            fillupControls(false);
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
        }
    }

    private void scrollRowsInfo()
    {
        int cols =
                scrollfields.size() > 0
                ? scrollfields.get(0).length
                : 0;
        for (int a = 0; a < ncurfieldcount; a++)
        {
            for (int n = 0; n < cols; n++)
            {
                fieldInfo(a, n);
            }
        }
    }

    /**
     * updates the corresponding data to
     * the control in guiRow and column
     * @param guiRow 0 based row index
     * @param column 0 based column index
     * @return the propertyValue object corresponding to
     * this control.
     */
    private PropertyValue fieldInfo(int guiRow, int column)
    {
        if (guiRow + nscrollvalue < scrollfields.size())
        {
            return fieldInfo(
                    scrollfields.get(guiRow + nscrollvalue)[column],
                    scrollfields.get(guiRow)[column]);
        }
        else
        {
            return null;
        }
    }

    private PropertyValue fieldInfo(PropertyValue valueProp, PropertyValue nameProp)
    {
        if (CurUnoDialog.getDlgNameAccess().hasByName(nameProp.Name))
        {
            valueProp.Value = getControlData(nameProp.Name);
        }
        else
        {
            valueProp.Value = nameProp.Value;
        }
        return valueProp;
    }

    protected void unregisterControlGroup(int _index)
    {
        scrollfields.remove(_index);
    }

    protected void registerControlGroup(PropertyValue[] _currowproperties, int _i)
    {
        if (_i == 0)
        {
            scrollfields.clear();
        }
        if (_i >= scrollfields.size())
        {
            scrollfields.add(_currowproperties);
        }
        else
        {
            scrollfields.set(_i, _currowproperties);
        }
    }



    private void setControlData(String controlname, Object newvalue)
    {
        Object oControlModel = UnoDialog.getModel(CurUnoDialog.xDlgContainer.getControl(controlname));
        String propertyname = UnoDialog.getDisplayProperty(oControlModel);
        if (!propertyname.equals(PropertyNames.EMPTY_STRING))
        {
            CurUnoDialog.setControlProperty(controlname, propertyname, newvalue);
        }
    }

    private Object getControlData(String controlname)
    {
        Object oControlModel = UnoDialog.getModel(CurUnoDialog.xDlgContainer.getControl(controlname));
        String propertyname = UnoDialog.getDisplayProperty(oControlModel);
        if (!propertyname.equals(PropertyNames.EMPTY_STRING))
        {
            return CurUnoDialog.getControlProperty(controlname, propertyname);
        }
        else
        {
            return null;
        }
    }

    protected PropertyValue[][] getScrollFieldValues()
    {
        scrollRowsInfo();
        PropertyValue[] curproperties;
        PropertyValue[][] retproperties;
        retproperties = new PropertyValue[scrollfields.size()][];
        try
        {
            for (int i = 0; i < scrollfields.size(); i++)
            {
                curproperties = scrollfields.get(i);
                retproperties[i] = curproperties;
            }
            return retproperties;
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace(System.err);
            return null;
        }
    }

    protected abstract void initializeScrollFields();

    /** inserts a group of controls into the component. The group may either be a row or a column of controls
     *  The controls should be put on Step 99 (means made invisible at first). All the controlrows that are needed are than
     *  made visible automatically when calling "initialize(_fieldcount)"
     * @param _index The index of the control group
     * @param npos Can be an x coordinate or an y coordinate which depends on the orientation of the scrollbar
     */
    protected abstract void insertControlGroup(int _index, int npos);

    protected abstract void setControlGroupVisible(int _index, boolean _bIsVisible);
}

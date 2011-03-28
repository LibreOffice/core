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
package com.sun.star.wizards.ui;

import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.common.HelpIds;

import java.util.*;

public abstract class ControlScroller
{

    protected WizardDialog CurUnoDialog;
    protected XMultiServiceFactory xMSF;
    private Object oImgControl;
    protected int ncurfieldcount;
    protected int nblockincrement;
    private int nlineincrement;
    protected int nscrollvalue = 0;
    protected int ntotfieldcount;
    XScrollBar xScrollBar;
    protected Vector scrollfields;
    protected Integer ICompPosX;
    protected int iCompPosX;
    protected Integer ICompPosY;
    protected int iCompPosY;
    protected Integer ICompWidth;
    protected int iCompWidth;
    protected Integer ICompHeight;
    protected int iCompHeight;
    protected int iStartPosY;
    protected short curtabindex;
    int iStep;
    protected Integer IStep;
    protected int linedistance;
    int iScrollBarWidth = 10;
    int SORELFIRSTPOSY = 3;
    protected int curHelpIndex;
    String sIncSuffix;
    protected Vector ControlGroupVector = new Vector();
    protected PeerConfig oTitlePeerConfig;

    class AdjustmentListenerImpl implements com.sun.star.awt.XAdjustmentListener
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
     * @param iStep
     * @param iCompPosX
     * @param iCompPosY
     * @param iCompWidth
     * @param _nblockincrement
     * @param _firsthelpindex
     * @author bc93774
     */
    // TODO add parameters for tabindices and helpindex
    protected ControlScroller(WizardDialog _CurUnoDialog, XMultiServiceFactory _xMSF, int _iStep, int _iCompPosX, int _iCompPosY, int _iCompWidth, int _nblockincrement, int _nlinedistance, int _firsthelpindex)
    {
        this.xMSF = _xMSF;
        this.nblockincrement = _nblockincrement;
        this.CurUnoDialog = _CurUnoDialog;
        this.iStep = _iStep;
        this.curHelpIndex = _firsthelpindex;
        curtabindex = UnoDialog.setInitialTabindex(iStep);
        this.linedistance = _nlinedistance;
        IStep = new Integer(iStep);
        this.iCompPosX = _iCompPosX;
        this.iCompPosY = _iCompPosY;
        this.ICompPosX = new Integer(iCompPosX);
        this.ICompPosY = new Integer(iCompPosY);
        this.iCompWidth = _iCompWidth;
        this.ICompWidth = new Integer(iCompWidth);
        this.iCompHeight = 2 * SORELFIRSTPOSY + nblockincrement * linedistance;
        iStartPosY = iCompPosY + SORELFIRSTPOSY;
        int ScrollHeight = iCompHeight - 2;
        nlineincrement = 1;
        sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.getDlgNameAccess(), "imgBackground");
        oImgControl = CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgBackground" + sIncSuffix,
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short("1"), new Integer(iCompHeight), ICompPosX, new Integer(iCompPosY), IStep, ICompWidth
                });
        oImgControl = CurUnoDialog.xDlgContainer.getControl("imgBackground" + sIncSuffix);
        setComponentMouseTransparent();
        xScrollBar = CurUnoDialog.insertScrollBar("TitleScrollBar" + sIncSuffix, 0,
                new AdjustmentListenerImpl(),
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 0), Boolean.TRUE, new Integer(ScrollHeight), HelpIds.getHelpIdString(curHelpIndex), new Integer(ScrollBarOrientation.VERTICAL), new Integer(iCompPosX + iCompWidth - iScrollBarWidth - 1), new Integer(iCompPosY + 1), IStep, new Integer(iScrollBarWidth)
                });
        scrollfields = new Vector();
        int ypos = iStartPosY + SORELFIRSTPOSY;
        for (int i = 0; i < nblockincrement; i++)
        {
            insertControlGroup(i, ypos);
            ypos += linedistance;
        }
    }

    public void setComponentMouseTransparent()
    {
        CurUnoDialog.getPeerConfiguration().setPeerProperties(oImgControl, new String[]
                {
                    "MouseTransparent"
                }, new Boolean[]
                {
                    Boolean.TRUE
                });
    }

    protected void setScrollBarOrientationHorizontal()
    {
        Helper.setUnoPropertyValue(xScrollBar, PropertyNames.ORIENTATION, new Integer(ScrollBarOrientation.HORIZONTAL));
    }

    /**
     * @author bc93774
     * @param _ntotfieldcount: The number of fields that are to be administered by the ControlScroller
     */
    protected void initialize(int _ntotfieldcount)
    {
        try
        {
            boolean bisVisible;
            ntotfieldcount = _ntotfieldcount;
            setCurFieldCount();
            nscrollvalue = 0;
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue", new Integer(nscrollvalue));
            if (ntotfieldcount > nblockincrement)
            {
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, "BlockIncrement", "LineIncrement", "ScrollValue", "ScrollValueMax"
                        }, new Object[]
                        {
                            Boolean.TRUE, new Integer(nblockincrement), new Integer(nlineincrement), new Integer(nscrollvalue), new Integer(ntotfieldcount - nblockincrement)
                        });
            }
            else
            {
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, "ScrollValue"
                        }, new Object[]
                        {
                            Boolean.FALSE, new Integer(nscrollvalue)
                        });
            }
            fillupControls(true);
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
        }
    }

    protected void fillupControls(boolean binitialize)
    {
        int newindex;
        PropertyValue[] oldproperties;
        PropertyValue[] newproperties;
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

    protected void fillupControls(int guiRow)
    {
        PropertyValue[] nameProps = (PropertyValue[]) scrollfields.get(guiRow);
        PropertyValue[] valueProps = (PropertyValue[]) scrollfields.get(guiRow + nscrollvalue);
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

    protected void setScrollValue(int _nscrollvalue)
    {
        if (_nscrollvalue >= 0)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue", new Integer(_nscrollvalue));
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
                        Boolean.TRUE, new Integer(ntotfieldcount - nblockincrement)
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
            PropertyValue[] curproperties = (PropertyValue[]) scrollfields.elementAt(n);
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

    protected void setLineIncrementation(int _nlineincrement)
    {
        this.nlineincrement = _nlineincrement;
        Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "LineIncrement", new Integer(nlineincrement));

    }

    protected int getLineIncrementation()
    {
        return nlineincrement;
    }

    protected void setBlockIncrementation(int _nblockincrement)
    {
        this.nblockincrement = _nblockincrement;
        Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, "BlockIncrement", "ScrollValueMax"
                }, new Object[]
                {
                        Boolean.valueOf(ntotfieldcount > nblockincrement), new Integer(nblockincrement), new Integer(ntotfieldcount - nblockincrement)
                });
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

    protected void scrollRowsInfo()
    {
        int cols =
                scrollfields.size() > 0
                ? ((PropertyValue[]) scrollfields.get(0)).length
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
    protected PropertyValue fieldInfo(int guiRow, int column)
    {
        if (guiRow + nscrollvalue < scrollfields.size())
        {
            return fieldInfo(
                    ((PropertyValue[]) scrollfields.elementAt(guiRow + nscrollvalue))[column],
                    ((PropertyValue[]) scrollfields.elementAt(guiRow))[column]);
            //System.out.println("getting field info for : " + guiRow + "/" + column  + ":" + pv.Value + "(" + pv.Name + ")" );
        }
        else
        {
            return null;
        }
    }

    protected PropertyValue fieldInfo(PropertyValue valueProp, PropertyValue nameProp)
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
            scrollfields.removeAllElements();
        }
        if (_i >= scrollfields.size())
        {
            scrollfields.addElement(_currowproperties);
        }
        else
        {
            scrollfields.setElementAt(_currowproperties, _i);
        }
    }

    protected PropertyValue[] getControlGroupInfo(int _i)
    {
        return (PropertyValue[]) scrollfields.elementAt(_i);
    }

    protected void setControlData(String controlname, Object newvalue)
    {
        Object oControlModel = UnoDialog.getModel(CurUnoDialog.xDlgContainer.getControl(controlname));
        String propertyname = UnoDialog.getDisplayProperty(oControlModel);
        if (!propertyname.equals(PropertyNames.EMPTY_STRING))
        {
            CurUnoDialog.setControlProperty(controlname, propertyname, newvalue);
        }
    }

    protected Object getControlData(String controlname)
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
                curproperties = (PropertyValue[]) scrollfields.elementAt(i);
                retproperties[i] = curproperties;
            }
            return retproperties;
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace(System.out);
            return null;
        }
    }

    protected abstract void initializeScrollFields();

    /** inserts a group of controls into the component. The group may either be a row or a column of controls
     *  The controls should be put on Step 99 (means made invisible at first). All the controlrows that are needed are than
     *  made visible automatically when calling "initialize(_fieldcount)"
     * @author bc93774
     * @param i: The index of the control group
     * @param npos: Can be an x coordinate or an y coordinate which depends on the orientation of the scrollbar
     */
    protected abstract void insertControlGroup(int _index, int npos);

    protected abstract void setControlGroupVisible(int _index, boolean _bIsVisible);
}

/*************************************************************************
 *
 *  $RCSfile: ControlScroller.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:04:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _____________________________________
 *
 */

package com.sun.star.wizards.ui;
import com.sun.star.awt.XScrollBar;
import com.sun.star.awt.AdjustmentEvent;
import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.lang.*;
import com.sun.star.wizards.common.*;

import java.util.*;

public abstract class ControlScroller {
    protected WizardDialog CurUnoDialog;
    protected XMultiServiceFactory xMSF;
    protected int ncurfieldcount;
    private int nblockincrement;
    private int nlineincrement;
    private int nscrollvalue = 0;
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

    class AdjustmentListenerImpl implements com.sun.star.awt.XAdjustmentListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public void adjustmentValueChanged(AdjustmentEvent AdjustEvent) {
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
    protected ControlScroller(WizardDialog _CurUnoDialog, XMultiServiceFactory _xMSF, int _iStep, int _iCompPosX, int _iCompPosY, int _iCompWidth, int _nblockincrement, int _nlinedistance, int _firsthelpindex) {
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
        sIncSuffix = com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.xDlgNameAccess, "lblFields");
        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlImageControlModel", "imgBackground" + sIncSuffix, new String[] { "Border", "Height", "PositionX", "PositionY", "Step", "Width" }, new Object[] { new Short("1"), new Integer(iCompHeight), ICompPosX, new Integer(iCompPosY), IStep, ICompWidth });
        Object oImgControl = CurUnoDialog.xDlgContainer.getControl("imgBackground" + sIncSuffix);
        PeerConfigHelper oTitlePeerConfig = new PeerConfigHelper(CurUnoDialog.xUnoDialog);
        oTitlePeerConfig.setPeerProperties(oImgControl, new String[] { "MouseTransparent" }, new Boolean[] { Boolean.TRUE });
        xScrollBar = CurUnoDialog.insertScrollBar("TitleScrollBar" + sIncSuffix, 0, new AdjustmentListenerImpl(), new String[] { "Border", "Enabled", "Height", "Orientation", "PositionX", "PositionY", "Step", "Width" }, new Object[] { new Short("0"), new Boolean(true), new Integer(ScrollHeight), new Integer(ScrollBarOrientation.VERTICAL), new Integer(iCompPosX + iCompWidth - iScrollBarWidth - 1), new Integer(iCompPosY + 1), IStep, new Integer(iScrollBarWidth)});
        scrollfields = new Vector();
        int ypos = iStartPosY + SORELFIRSTPOSY;
        for (int i = 0; i < nblockincrement; i++) {
            insertControlGroup(i, ypos);
            ypos += linedistance;
        }
    }

    protected void setScrollBarOrientationHorizontal() {
        Helper.setUnoPropertyValue(xScrollBar, "Orientation", new Integer(ScrollBarOrientation.HORIZONTAL));
    }

    /**
     * @author bc93774
     * @param _ntotfieldcount: The number of fields that are to be administered by the ControlScroller
     */
    protected void initialize(int _ntotfieldcount) {
        try {
            boolean bisVisible;
            ntotfieldcount = _ntotfieldcount;
            setCurFieldCount();
            nscrollvalue = 0;
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), new String("ScrollValue"), new Integer(nscrollvalue));
            if (ntotfieldcount > nblockincrement)
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[] { "Enabled", "BlockIncrement", "LineIncrement", "ScrollValue", "ScrollValueMax" }, new Object[] { Boolean.TRUE, new Integer(nblockincrement), new Integer(nlineincrement), new Integer(nscrollvalue), new Integer(ntotfieldcount - nblockincrement)});
            else
                Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[] { "Enabled", "ScrollValue" }, new Object[] { Boolean.FALSE, new Integer(nscrollvalue)});
            fillupControls(true);
        } catch (java.lang.Exception ex) {
            ex.printStackTrace();
        }
    }

    protected void fillupControls(boolean binitialize) {
        int newindex;
        PropertyValue[] oldproperties;
        PropertyValue[] newproperties;
        for (int a = 0; a < this.nblockincrement; a++) {
            if (a < ncurfieldcount){
                newindex = (a) + nscrollvalue;
                oldproperties = (PropertyValue[]) scrollfields.elementAt(a);
                newproperties = (PropertyValue[]) scrollfields.elementAt(newindex);
                for (int n = 0; n < oldproperties.length; n++) {
                    if (CurUnoDialog.xDlgNameAccess.hasByName(oldproperties[n].Name))
                        setControlData(oldproperties[n].Name, newproperties[n].Value);
                    else
                        oldproperties[n].Value = newproperties[n].Value;
                }
            }
            if (binitialize)
                setControlGroupVisible(a, (a < this.ncurfieldcount));
        }
        if (binitialize)
            CurUnoDialog.repaintDialogStep();

    }

    protected void setScrollValue(int _nscrollvalue) {
        if (_nscrollvalue >= 0) {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), new String("ScrollValue"), new Integer(_nscrollvalue));
            scrollControls();
        }
    }

    protected void setScrollValue(int _nscrollvalue, int _ntotfieldcount) {
        setTotalFieldCount(_ntotfieldcount);
        setScrollValue(_nscrollvalue);
    }

    protected int getTotalFieldCount() {
        return ntotfieldcount;
    }

    protected int getCurFieldCount() {
        return ncurfieldcount;
    }

    private void setCurFieldCount() {
        if (ntotfieldcount > nblockincrement)
            ncurfieldcount = nblockincrement;
        else
            ncurfieldcount = ntotfieldcount;
    }

    protected void setTotalFieldCount(int _ntotfieldcount) {
        this.ntotfieldcount = _ntotfieldcount;
        setCurFieldCount();
        if (ntotfieldcount > nblockincrement)
            Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[] { "Enabled", "ScrollValueMax" }, new Object[] { Boolean.TRUE, new Integer(ntotfieldcount - nblockincrement)});
        else
            Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), "Enabled", Boolean.FALSE);
    }

    protected void toggleComponent(boolean _bdoenable) {
        boolean bdoenable = _bdoenable && (ntotfieldcount > nblockincrement);
        CurUnoDialog.setControlProperty("TitleScrollBar" + sIncSuffix, "Enabled", new Boolean(bdoenable));
    }

    protected void toggleControls(boolean _bdoenable) {
        for (int n = 0; n < scrollfields.size(); n++) {
            PropertyValue[] curproperties = (PropertyValue[]) scrollfields.elementAt(n);
            for (int m = 0; m < curproperties.length; m++) {
                PropertyValue curproperty = curproperties[m];
                CurUnoDialog.setControlProperty(curproperty.Name, "Enabled", new Boolean(_bdoenable));
            }
        }

    }

    protected int getScrollValue() {
        return nscrollvalue;
    }

    protected void setLineIncrementation(int _nlineincrement) {
        this.nlineincrement = _nlineincrement;
        Helper.setUnoPropertyValue(UnoDialog.getModel(xScrollBar), new String("LineIncrement"), new Integer(nlineincrement));

    }

    protected int getLineIncrementation() {
        return nlineincrement;
    }

    protected void setBlockIncrementation(int _nblockincrement) {
        this.nblockincrement = _nblockincrement;
        Helper.setUnoPropertyValues(UnoDialog.getModel(xScrollBar), new String[] { "Enabled", "BlockIncrement", "ScrollValueMax" }, new Object[] { new Boolean(ntotfieldcount > nblockincrement), new Integer(nblockincrement), new Integer(ntotfieldcount - nblockincrement)});
    }

    protected int getBlockIncrementation() {
        return nblockincrement;
    }

    private void scrollControls() {
        try {
            scrollRowsInfo();
            nscrollvalue = ((Integer) Helper.getUnoPropertyValue(UnoDialog.getModel(xScrollBar), "ScrollValue")).intValue();
            if (nscrollvalue + nblockincrement >= ntotfieldcount)
                nscrollvalue = (ntotfieldcount) - nblockincrement;
            fillupControls(false);
        } catch (java.lang.Exception ex) {
            ex.printStackTrace();
        }
    }

    protected void scrollRowsInfo() {
        int newindex;
        PropertyValue[] oldproperties;
        PropertyValue[] newproperties;
        for (int a = 0; a < ncurfieldcount; a++) {
            newindex = (a) + nscrollvalue;
            oldproperties = (PropertyValue[]) scrollfields.elementAt(a);
            newproperties = (PropertyValue[]) scrollfields.elementAt(newindex);
            for (int n = 0; n < oldproperties.length; n++)
                if (CurUnoDialog.xDlgNameAccess.hasByName(oldproperties[n].Name))
                    newproperties[n].Value = getControlData(oldproperties[n].Name);
                else
                    newproperties[n].Value = oldproperties[n].Value;
        }
    }

    protected void unregisterControlGroup(int _index) {
        scrollfields.remove(_index);
    }

    protected void registerControlGroup(PropertyValue[] _currowproperties, int _i) {
        if (_i == 0)
            scrollfields.removeAllElements();
        if (_i >= scrollfields.size())
            scrollfields.addElement(_currowproperties);
        else
            scrollfields.setElementAt(_currowproperties, _i);
    }

    protected PropertyValue[] getControlGroupInfo(int _i) {
        return (PropertyValue[]) scrollfields.elementAt(_i);
    }

    protected void setControlData(String controlname, Object newvalue) {
        Object oControlModel = UnoDialog.getModel(CurUnoDialog.xDlgContainer.getControl(controlname));
        String propertyname = UnoDialog.getDisplayProperty(oControlModel);
        if (propertyname != "") {
            CurUnoDialog.setControlProperty(controlname, propertyname, newvalue);
        }
    }

    protected Object getControlData(String controlname) {
        Object oControlModel = UnoDialog.getModel(CurUnoDialog.xDlgContainer.getControl(controlname));
        String propertyname = UnoDialog.getDisplayProperty(oControlModel);
        if (propertyname != "")
            return CurUnoDialog.getControlProperty(controlname, propertyname);
        else
            return null;
    }

    protected PropertyValue[][] getScrollFieldValues() {
        scrollRowsInfo();
        PropertyValue[] curproperties;
        PropertyValue[][] retproperties;
        retproperties = new PropertyValue[scrollfields.size()][];
        try {
            for (int i = 0; i < scrollfields.size(); i++) {
                curproperties = (PropertyValue[]) scrollfields.elementAt(i);
                retproperties[i] = curproperties;
            }
            return retproperties;
        } catch (java.lang.Exception ex) {
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
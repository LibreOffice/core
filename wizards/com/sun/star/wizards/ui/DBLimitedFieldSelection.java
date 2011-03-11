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

import com.sun.star.awt.XListBox;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public abstract class DBLimitedFieldSelection
{

    protected XMultiServiceFactory xMSF;
    protected WizardDialog CurUnoDialog;
    protected String sNoField;
    protected Integer IStep;
    protected Integer ICompPosX;
    protected Integer ICompPosY;
    protected Integer ICompWidth;
    protected final int rowcount = 4;
    protected final int MAXSELINDEX = rowcount - 1;
    protected short curtabindex;
    protected int iCurPosY;
    protected int FirstHelpIndex;
    protected int iCompPosX;
    // protected int MaxSelIndex;

    public DBLimitedFieldSelection(WizardDialog _CurUnoDialog, int iStep, int _iCompPosX, int iCompPosY, int iCompWidth, int _FirstHelpIndex)
    {
        this.CurUnoDialog = _CurUnoDialog;
        xMSF = CurUnoDialog.xMSF;
        FirstHelpIndex = _FirstHelpIndex;
        curtabindex = (short) (iStep * 100);
        sNoField = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 8);
        IStep = new Integer(iStep);
        iCompPosX = _iCompPosX;
        ICompPosX = new Integer(iCompPosX);
        ICompPosY = new Integer(iCompPosY);
        ICompWidth = new Integer(iCompWidth);
        // boolean bDoEnable;
        iCurPosY = iCompPosY;
        for (int i = 0; i < rowcount; i++)
        {
            insertControlGroup(i);
        }
    }

    protected abstract void insertControlGroup(int index);

    protected abstract void toggleControlRow(int CurIndex, boolean bDoEnable);

    protected abstract void enableNextControlRow(int CurIndex);

    protected abstract void updateFromNextControlRow(int CurIndex);

    // protected abstract void setMaxSelIndex();

    protected abstract int getMaxSelIndex();

    protected void moveupSelectedItems(int CurIndex, boolean bDoEnable)
    {
        // short iNextItemPos;
        if ((bDoEnable == false) && (MAXSELINDEX > CurIndex))
        {
            for (int i = CurIndex; i < MAXSELINDEX; i++)
            {
                updateFromNextControlRow(i);
            }
            if (getMaxSelIndex() < rowcount - 2)
            {
                toggleControlRow(getMaxSelIndex() + 2, false);
            }
        }
        else
        {
            toggleControlRow(CurIndex + 1, bDoEnable);
        }
    }

    protected String[] addNoneFieldItemToList(String[] _FieldNames)
    {
        int FieldCount = _FieldNames.length;
        String[] ViewFieldNames = new String[FieldCount + 1];
        ViewFieldNames[0] = sNoField;
        for (int i = 0; i < FieldCount; i++)
        {
            ViewFieldNames[i + 1] = _FieldNames[i];
        }
        return ViewFieldNames;
    }

    protected void initializeListBox(XListBox xListBox, String[] _AllFieldNames, String[] _SelFieldNames, int curindex)
    {
        short[] SelList = null;
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), PropertyNames.STRING_ITEM_LIST, _AllFieldNames);
        if (_SelFieldNames != null)
        {
            if (curindex < _SelFieldNames.length)
            {
                int index = JavaTools.FieldInList(_AllFieldNames, _SelFieldNames[curindex]);
                if (index > -1)
                {
                    SelList = new short[] { (short) (index) };
                }
                else
                {
                    SelList = new short[] { (short) (0) };
                }
                Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), PropertyNames.SELECTED_ITEMS, SelList);
                return;
            }
        }
        SelList = new short[] { (short) (0) };
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), PropertyNames.SELECTED_ITEMS, SelList);

    }

    protected void initializeListBox(XListBox xListBox, String[] _AllFieldNames, String _SelFieldName)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), PropertyNames.STRING_ITEM_LIST, _AllFieldNames);
        short[] SelList = null;
        int index = JavaTools.FieldInList(_AllFieldNames, _SelFieldName);
        SelList = new short[] { (short) (index) };
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), PropertyNames.SELECTED_ITEMS, SelList);
    }
}



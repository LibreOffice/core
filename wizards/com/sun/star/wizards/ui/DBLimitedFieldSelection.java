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

import com.sun.star.awt.XListBox;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

/**
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


    protected abstract int getMaxSelIndex();

    protected void moveupSelectedItems(int CurIndex, boolean bDoEnable)
    {
        if ((!bDoEnable) && (MAXSELINDEX > CurIndex))
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
        System.arraycopy(_FieldNames, 0, ViewFieldNames, 1, FieldCount);
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



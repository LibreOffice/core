/*************************************************************************
 *
 *  $RCSfile: DBLimitedFieldSelection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:41:05 $
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
 *  Contributor(s): Berend Cornelius
 *
 */
package com.sun.star.wizards.ui;

import com.sun.star.awt.XListBox;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public abstract class DBLimitedFieldSelection {
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
    protected int MaxSelIndex;


    public DBLimitedFieldSelection(WizardDialog _CurUnoDialog, int iStep, int _iCompPosX, int iCompPosY, int iCompWidth, int _FirstHelpIndex){
        this.CurUnoDialog = _CurUnoDialog;
        xMSF = CurUnoDialog.xMSF;
        FirstHelpIndex =  _FirstHelpIndex;
        curtabindex = (short) (iStep * 100);
        sNoField = CurUnoDialog.oResource.getResText(UIConsts.RID_REPORT + 8);
        IStep = new Integer(iStep);
        iCompPosX = _iCompPosX;
        ICompPosX = new Integer(iCompPosX);
        ICompPosY = new Integer(iCompPosY);
        ICompWidth = new Integer(iCompWidth);
        boolean bDoEnable;
        iCurPosY = iCompPosY;
        for (int i = 0; i < rowcount; i++) {
            insertControlGroup(i);
        }
    }


    protected abstract void insertControlGroup(int index);

    protected abstract void toggleControlRow(int CurIndex, boolean bDoEnable);

    protected abstract void enableNextControlRow(int CurIndex);

    protected abstract void updateFromNextControlRow(int CurIndex);

    protected abstract void setMaxSelIndex();


    protected void moveupSelectedItems(int CurIndex, boolean bDoEnable){
        short iNextItemPos;
        if ((bDoEnable == false) && (MAXSELINDEX > CurIndex)) {
            for (int i = CurIndex; i < MAXSELINDEX; i++) {
                updateFromNextControlRow(i);
            }
            if (MaxSelIndex < rowcount - 2)
                toggleControlRow(MaxSelIndex + 2, false);
        } else
            toggleControlRow(CurIndex + 1, bDoEnable);
    }


    protected String[] addNoneFieldItemToList(String[] _FieldNames){
        int FieldCount = _FieldNames.length;
        String[] ViewFieldNames = new String[FieldCount + 1];
        ViewFieldNames[0] = sNoField;
        for (int i = 0; i < FieldCount; i++)
            ViewFieldNames[i + 1] = _FieldNames[i];
        return ViewFieldNames;
    }


    protected void initializeListBox(XListBox xListBox, String[] _AllFieldNames, String[] _SelFieldNames, int curindex){
        short[] SelList = null;
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox),"StringItemList", _AllFieldNames);
        if (_SelFieldNames != null){
            if (curindex < _SelFieldNames.length){
                int index = JavaTools.FieldInList(_AllFieldNames, _SelFieldNames[curindex]);
                if (index > -1)
                    SelList = new short[] {(short) (index)};
                else
                    SelList = new short[] {(short) (0)};
                Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), "SelectedItems", SelList);
                return;
            }
        }
        SelList = new short[] {(short) (0)};
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), "SelectedItems", SelList);

    }


    protected void initializeListBox(XListBox xListBox, String[] _AllFieldNames, String _SelFieldName){
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox),"StringItemList", _AllFieldNames);
        short[] SelList = null;
        int index = JavaTools.FieldInList(_AllFieldNames, _SelFieldName);
        SelList = new short[] {(short) (index)};
        Helper.setUnoPropertyValue(UnoDialog.getModel(xListBox), "SelectedItems", SelList);
    }
}



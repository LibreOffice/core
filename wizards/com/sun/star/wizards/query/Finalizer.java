/*************************************************************************
*
*  $RCSfile: Finalizer.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:44:59 $
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
*  Contributor(s): _______________________________________
*
*/

package com.sun.star.wizards.query;

import com.sun.star.wizards.common.*;
import com.sun.star.awt.XRadioButton;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.CommandType;
import com.sun.star.uno.*;
import com.sun.star.wizards.ui.*;

public class Finalizer {
    UnoDialog2 CurUnoDialog;
    String resQuery;
    Object txtSummary;
    Object txtTitle;
    XRadioButton xRadioDisplayQuery;
    XRadioButton xRadioModifyQuery;

    QuerySummary CurDBMetaData;

    public Finalizer(UnoDialog2 _CurUnoDialog, QuerySummary _CurDBMetaData) {
        short curtabindex = (short) (100 * QueryWizard.SOSUMMARYPAGE);
        String reslblQueryTitle;
        String resoptDisplayQuery;
        String resoptModifyQuery;
        String resflnSummary;
        String reslblHowGoOn;
        this.CurUnoDialog = _CurUnoDialog;
        this.CurDBMetaData = _CurDBMetaData;
        reslblQueryTitle = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 5);
        resoptDisplayQuery = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 6);
        resoptModifyQuery = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 7);
        resflnSummary = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 87);
        reslblHowGoOn = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 8);
        resQuery = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 1);
        int curHelpIndex = 40955;

        CurUnoDialog.insertLabel("lblQueryTitle", new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                                new Object[] { new Integer(8), reslblQueryTitle, new Integer(95), new Integer(22), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(52)});
        txtTitle = CurUnoDialog.insertTextField("txtQueryTitle", 0, null, new String[] { "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                                new Object[] { new Integer(12), "HID:" + curHelpIndex++, new Integer(95), new Integer(32), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(90)});
        CurUnoDialog.insertLabel("lblHowGoOn", new String[] { "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                                new Object[] { new Integer(16), reslblHowGoOn, Boolean.TRUE, new Integer(192), new Integer(22), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(112)});
        this.xRadioDisplayQuery = CurUnoDialog.insertRadioButton("optDisplayQuery",
                                                    new String[] { "Height",  "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width" },
                                                    new Object[] { new Integer(9), "HID:" + curHelpIndex++, resoptDisplayQuery, new Integer(196), new Integer(40), new Short((short) 1), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(56)});

        this.xRadioModifyQuery = CurUnoDialog.insertRadioButton("optModifyQuery",
                                                new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                                new Object[] { new Integer(10), "HID:" + curHelpIndex++, resoptModifyQuery, new Integer(196), new Integer(50), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(56)});
        CurUnoDialog.insertFixedLine("flnSummary", new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                                new Object[] { new Integer(10), resflnSummary, new Integer(95), new Integer(62), new Integer(8), new Short(curtabindex++), new Integer(201)});
        txtSummary = CurUnoDialog.insertTextField("txtSummary", 0, null, new String[] { "Height", "HelpURL", "MultiLine", "PositionX", "PositionY", "ReadOnly", "Step", "VScroll", "Width" },
                                                new Object[] { new Integer(102), "HID:" + curHelpIndex++, Boolean.TRUE, new Integer(95), new Integer(74), Boolean.TRUE, new Integer(8), Boolean.TRUE, new Integer(201)});
    }

    /* TODO: The title textbox always has to be updated when
          a new Table has been selected if it is clear that the user has not made any input meanwhile
        *     */
    protected String initialize() {
        try {
            String sCurQueryName = AnyConverter.toString(Helper.getUnoPropertyValue(UnoDialog.getModel(txtTitle), "Text"));
            if (sCurQueryName != null) {
                if (sCurQueryName.equals("")) {
                    String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
                    sCurQueryName = resQuery + "_" + sCommandNames[0];
                    sCurQueryName = Desktop.getUniqueName(CurDBMetaData.xQueryNames, sCurQueryName);
                    Helper.setUnoPropertyValue(UnoDialog.getModel(txtTitle), "Text", sCurQueryName);
                }
            }
            CurDBMetaData.setSummaryString();
            CurUnoDialog.setControlProperty("txtSummary", "Text", CurDBMetaData.getSummaryString());
            return sCurQueryName;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return "";
        }
    }

    private String getTitle() {
        return (String) Helper.getUnoPropertyValue(UnoDialog.getModel(txtTitle), "Text");
    }

    public void finish() {
        try {
            CurDBMetaData.oSQLQueryComposer = new SQLQueryComposer(CurDBMetaData);
            String queryname = getTitle();
            boolean bsuccess = CurDBMetaData.oSQLQueryComposer.setQueryCommand(queryname, CurUnoDialog.xWindow, true);
            if (bsuccess) {
                String sfinalname = CurDBMetaData.createQuery(CurDBMetaData.oSQLQueryComposer, getTitle());
                short igoon = AnyConverter.toShort(Helper.getUnoPropertyValue(UnoDialog.getModel(xRadioDisplayQuery), "State"));
                if (igoon == (short) 1)
                    CurDBMetaData.switchtoDataViewmode(sfinalname, CommandType.QUERY);
                else
                    CurDBMetaData.switchtoDesignmode(sfinalname, CommandType.QUERY);
                CurUnoDialog.xDialog.endExecute();

            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }
}
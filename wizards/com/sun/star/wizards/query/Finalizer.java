/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Finalizer.java,v $
 * $Revision: 1.10 $
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
package com.sun.star.wizards.query;

import com.sun.star.wizards.common.*;
import com.sun.star.awt.XRadioButton;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.uno.*;
import com.sun.star.wizards.ui.*;

public class Finalizer
{

    private QueryWizard CurUnoDialog;
    private String resQuery;
    private Object m_aTxtSummary;
    private Object m_aTxtTitle;
    private XRadioButton xRadioDisplayQuery;
    private XRadioButton xRadioModifyQuery;
    private QuerySummary CurDBMetaData;

    public Finalizer(QueryWizard _CurUnoDialog, QuerySummary _CurDBMetaData)
    {
        short curtabindex = (short) (100 * QueryWizard.SOSUMMARYPAGE);
        String reslblQueryTitle;
        String resoptDisplayQuery;
        String resoptModifyQuery;
        String resflnSummary;
        String reslblHowGoOn;
        this.CurUnoDialog = _CurUnoDialog;
        this.CurDBMetaData = _CurDBMetaData;
        reslblQueryTitle = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 5);
        resoptDisplayQuery = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 6);
        resoptModifyQuery = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 7);
        resflnSummary = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 87);
        reslblHowGoOn = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 8);
        resQuery = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 1);
        int curHelpIndex = 40955;

        CurUnoDialog.insertLabel("lblQueryTitle", new String[]
                {
                    "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(8), reslblQueryTitle, new Integer(95), new Integer(27), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(52)
                });
        m_aTxtTitle = CurUnoDialog.insertTextField("txtQueryTitle", 0, null, new String[]
                {
                    "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(12), "HID:" + curHelpIndex++, new Integer(95), new Integer(37), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(90)
                });
        CurUnoDialog.insertLabel("lblHowGoOn", new String[]
                {
                    "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(16), reslblHowGoOn, Boolean.TRUE, new Integer(192), new Integer(27), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(112)
                });
        this.xRadioDisplayQuery = CurUnoDialog.insertRadioButton("optDisplayQuery",
                new String[]
                {
                    "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(9), "HID:" + curHelpIndex++, resoptDisplayQuery, new Integer(192), new Integer(46), new Short((short) 1), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(118)
                });

        this.xRadioModifyQuery = CurUnoDialog.insertRadioButton("optModifyQuery",
                new String[]
                {
                    "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(10), "HID:" + curHelpIndex++, resoptModifyQuery, new Integer(192), new Integer(56), new Integer(QueryWizard.SOSUMMARYPAGE), new Short(curtabindex++), new Integer(118)
                });
        CurUnoDialog.insertFixedLine("flnSummary", new String[]
                {
                    "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"
                },
                new Object[]
                {
                    new Integer(10), resflnSummary, new Integer(95), new Integer(68), new Integer(8), new Short(curtabindex++), new Integer(209)
                });
        m_aTxtSummary = CurUnoDialog.insertTextField("txtSummary", 0, null, new String[]
                {
                    "Height", "HelpURL", "MultiLine", "PositionX", "PositionY", "ReadOnly", "Step", "VScroll", "Width"
                },
                new Object[]
                {
                    new Integer(96), "HID:" + curHelpIndex++, Boolean.TRUE, new Integer(95), new Integer(80), Boolean.TRUE, new Integer(8), Boolean.TRUE, new Integer(209)
                });
    }

    /* TODO: The title textbox always has to be updated when
    a new Table has been selected if it is clear that the user has not made any input meanwhile
     */
    protected String initialize()
    {
        try
        {
            String sCurQueryName = AnyConverter.toString(Helper.getUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text"));
            if (sCurQueryName != null)
            {
                if (sCurQueryName.equals(""))
                {
                    String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
                    sCurQueryName = resQuery + "_" + sCommandNames[0];
                    sCurQueryName = CurDBMetaData.ConnectionTools.getObjectNames().suggestName(CommandType.QUERY, sCurQueryName);
                    Helper.setUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text", sCurQueryName);
                }
            }

            CurDBMetaData.setSummaryString();
            CurUnoDialog.setControlProperty("txtSummary", "Text", CurDBMetaData.getSummaryString());
            return sCurQueryName;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return "";
        }
    }

    private String getTitle()
    {
        final String sTitle = (String) Helper.getUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text");
        return sTitle;
    }

    public XComponent[] finish()
    {
        XComponent[] ret = null;
        try
        {
            CurDBMetaData.oSQLQueryComposer = new SQLQueryComposer(CurDBMetaData);
            String queryname = getTitle();
            boolean bsuccess = CurDBMetaData.oSQLQueryComposer.setQueryCommand(queryname, CurUnoDialog.xWindow, true, true);
            if (bsuccess)
            {
                bsuccess = CurDBMetaData.createQuery(CurDBMetaData.oSQLQueryComposer, queryname);
                if (bsuccess)
                {
                    short igoon = AnyConverter.toShort(Helper.getUnoPropertyValue(UnoDialog.getModel(xRadioDisplayQuery), "State"));
                    if (igoon == (short) 1)
                    {
                        ret = CurDBMetaData.switchtoDataViewmode(queryname,
                                CommandType.QUERY,
                                CurUnoDialog.getCurFrame());
                    }
                    else
                    {
                        ret = CurDBMetaData.switchtoDesignmode(queryname,
                                CommandType.QUERY,
                                CurUnoDialog.getCurFrame());
                    }
                    CurUnoDialog.xDialog.endExecute();
                }
            }
            CurDBMetaData.oSQLQueryComposer = null;
            CurDBMetaData = null;
            CurUnoDialog = null;
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace();
        }
        return ret;
    }
}

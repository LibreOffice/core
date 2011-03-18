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
package com.sun.star.wizards.query;

import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.CommandType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Finalizer
{
    private QueryWizard m_queryWizard;
    private String resQuery;
    private XTextComponent m_aTxtTitle;
    private XRadioButton xRadioDisplayQuery;
    private QuerySummary CurDBMetaData;

    public Finalizer( QueryWizard i_queryWizard, QuerySummary _CurDBMetaData )
    {
        short curtabindex = (short) (100 * QueryWizard.SOSUMMARY_PAGE);
        String reslblQueryTitle;
        String resoptDisplayQuery;
        String resoptModifyQuery;
        String resflnSummary;
        String reslblHowGoOn;
        this.m_queryWizard = i_queryWizard;
        this.CurDBMetaData = _CurDBMetaData;
        reslblQueryTitle = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 5);
        resoptDisplayQuery = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 6);
        resoptModifyQuery = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 7);
        resflnSummary = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 87);
        reslblHowGoOn = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 8);
        resQuery = m_queryWizard.m_oResource.getResText(UIConsts.RID_QUERY + 1);
        int curHelpIndex = 40955;

        m_queryWizard.insertLabel("lblQueryTitle", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(8), reslblQueryTitle, new Integer(95), new Integer(27), new Integer(QueryWizard.SOSUMMARY_PAGE), new Short(curtabindex++), new Integer(52)
                });
        m_aTxtTitle = m_queryWizard.insertTextField("txtQueryTitle", "changeTitle", this, new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(12), HelpIds.getHelpIdString(curHelpIndex++), new Integer(95), new Integer(37), new Integer(QueryWizard.SOSUMMARY_PAGE), new Short(curtabindex++), new Integer(90)
                });
        m_queryWizard.insertLabel("lblHowGoOn", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(16), reslblHowGoOn, Boolean.TRUE, new Integer(192), new Integer(27), new Integer(QueryWizard.SOSUMMARY_PAGE), new Short(curtabindex++), new Integer(112)
                });
        this.xRadioDisplayQuery = m_queryWizard.insertRadioButton("optDisplayQuery",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(9), HelpIds.getHelpIdString(curHelpIndex++), resoptDisplayQuery, new Integer(192), new Integer(46), new Short((short) 1), new Integer(QueryWizard.SOSUMMARY_PAGE), new Short(curtabindex++), new Integer(118)
                });

        m_queryWizard.insertRadioButton("optModifyQuery",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(10), HelpIds.getHelpIdString(curHelpIndex++), resoptModifyQuery, new Integer(192), new Integer(56), new Integer(QueryWizard.SOSUMMARY_PAGE), new Short(curtabindex++), new Integer(118)
                });
        m_queryWizard.insertFixedLine("flnSummary", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(10), resflnSummary, new Integer(95), new Integer(68), new Integer(8), new Short(curtabindex++), new Integer(209)
                });
        m_queryWizard.insertTextField("txtSummary", 0, null, new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ReadOnly", PropertyNames.PROPERTY_STEP, "VScroll", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(96), HelpIds.getHelpIdString(curHelpIndex++), Boolean.TRUE, new Integer(95), new Integer(80), Boolean.TRUE, new Integer(8), Boolean.TRUE, new Integer(209)
                });
    }

    public void changeTitle()
    {
        final String TitleName = m_aTxtTitle.getText();
        m_queryWizard.enableFinishButton( TitleName.length() > 0 );
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
                    sCurQueryName = CurDBMetaData.suggestName( CommandType.QUERY, sCurQueryName );
                    Helper.setUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text", sCurQueryName);
                }
            }

            CurDBMetaData.setSummaryString();
            m_queryWizard.setControlProperty("txtSummary", "Text", CurDBMetaData.getSummaryString());
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

    public String finish()
    {
        String queryName = getTitle();
        if  (   CurDBMetaData.getSQLQueryComposer().setQueryCommand( m_queryWizard.xWindow, true, true )
            &&  CurDBMetaData.createQuery( CurDBMetaData.getSQLQueryComposer(), queryName )
            )
            return queryName;

        return "";
    }

    public final boolean displayQueryDesign()
    {
        try
        {
            final short state = AnyConverter.toShort( Helper.getUnoPropertyValue( UnoDialog.getModel( xRadioDisplayQuery ), PropertyNames.PROPERTY_STATE ) );
            final boolean viewMode = state == (short)1;
            return !viewMode;
        }
        catch ( IllegalArgumentException ex )
        {
            Logger.getLogger( Finalizer.class.getName() ).log( Level.SEVERE, null, ex );
        }
        return false;
    }
}

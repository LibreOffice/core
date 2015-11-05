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
package com.sun.star.wizards.query;

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.event.XTextListenerAdapter;

import java.util.logging.Level;
import java.util.logging.Logger;

public class Finalizer
{
    private final QueryWizard m_queryWizard;
    private final String resQuery;
    private final XTextComponent m_aTxtTitle;
    private final XRadioButton xRadioDisplayQuery;
    private final QuerySummary CurDBMetaData;

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
                    8, reslblQueryTitle, 95, 27, Integer.valueOf(QueryWizard.SOSUMMARY_PAGE), Short.valueOf(curtabindex++), 52
                });
        m_aTxtTitle = m_queryWizard.insertTextField("txtQueryTitle", new XTextListenerAdapter() {
                    @Override
                    public void textChanged(TextEvent event) {
                        changeTitle();
                    }
                }, new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    12, HelpIds.getHelpIdString(curHelpIndex++), 95, 37, Integer.valueOf(QueryWizard.SOSUMMARY_PAGE), Short.valueOf(curtabindex++), 90
                });
        m_queryWizard.insertLabel("lblHowGoOn", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    16, reslblHowGoOn, Boolean.TRUE, 192, 27, Integer.valueOf(QueryWizard.SOSUMMARY_PAGE), Short.valueOf(curtabindex++), 112
                });
        this.xRadioDisplayQuery = m_queryWizard.insertRadioButton("optDisplayQuery",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    9, HelpIds.getHelpIdString(curHelpIndex++), resoptDisplayQuery, 192, 46, Short.valueOf((short) 1), Integer.valueOf(QueryWizard.SOSUMMARY_PAGE), Short.valueOf(curtabindex++), 118
                });

        m_queryWizard.insertRadioButton("optModifyQuery",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, HelpIds.getHelpIdString(curHelpIndex++), resoptModifyQuery, 192, 56, Integer.valueOf(QueryWizard.SOSUMMARY_PAGE), Short.valueOf(curtabindex++), 118
                });
        m_queryWizard.insertFixedLine("flnSummary", new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, resflnSummary, 95, 68, 8, Short.valueOf(curtabindex++), 209
                });
        m_queryWizard.insertTextField("txtSummary", 0, null, new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.READ_ONLY, PropertyNames.PROPERTY_STEP, "VScroll", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    96, HelpIds.getHelpIdString(curHelpIndex++), Boolean.TRUE, 95, 80, Boolean.TRUE, 8, Boolean.TRUE, 209
                });
    }

    private void changeTitle()
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
            if (sCurQueryName != null && sCurQueryName.equals(PropertyNames.EMPTY_STRING))
            {
                String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
                sCurQueryName = resQuery + "_" + sCommandNames[0];
                sCurQueryName = CurDBMetaData.suggestName( CommandType.QUERY, sCurQueryName );
                Helper.setUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text", sCurQueryName);
            }
            CurDBMetaData.setSummaryString();
            m_queryWizard.setControlProperty("txtSummary", "Text", CurDBMetaData.getSummaryString());
            return sCurQueryName;
        }
        catch (IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
            return PropertyNames.EMPTY_STRING;
        }
        catch (SQLException exception)
        {
            exception.printStackTrace(System.err);
            return PropertyNames.EMPTY_STRING;
        }

    }

    private String getTitle()
    {
        return (String) Helper.getUnoPropertyValue(UnoDialog.getModel(m_aTxtTitle), "Text");
    }

    public String finish()
    {
        String queryName = getTitle();
        if  (   CurDBMetaData.getSQLQueryComposer().setQueryCommand( m_queryWizard.xWindow, true, true )
            &&  CurDBMetaData.createQuery( CurDBMetaData.getSQLQueryComposer(), queryName )
            )
            return queryName;

        return PropertyNames.EMPTY_STRING;
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

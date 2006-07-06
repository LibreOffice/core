/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SQLQueryComposer.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:16:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.db;

import java.util.Vector;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdb.XColumn;
import com.sun.star.sdb.XSQLQueryComposer;
import com.sun.star.sdb.XSQLQueryComposerFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.XSingleSelectQueryAnalyzer;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.SQLException;
import com.sun.star.lang.XInitialization;
import com.sun.star.awt.XWindow;

import com.sun.star.wizards.common.*;

public class SQLQueryComposer {

    public XColumnsSupplier xColSuppl;
    XSQLQueryComposerFactory xSQLComposerFactory;
    XSQLQueryComposer xSQLQueryComposer;
    QueryMetaData CurDBMetaData;
    String selectclause;
    String fromclause;
    public XSingleSelectQueryAnalyzer xQueryAnalyzer;
    Vector composedCommandNames = new Vector(1);
    public XSingleSelectQueryComposer xQueryComposer;
    XMultiServiceFactory xMSF;
    boolean bincludeGrouping = true;

    public SQLQueryComposer(QueryMetaData _CurDBMetaData) {
        try {
            this.CurDBMetaData = _CurDBMetaData;
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, CurDBMetaData.DBConnection);
            Object oQueryComposer = xMSF.createInstance("com.sun.star.sdb.SingleSelectQueryComposer");
            xQueryAnalyzer = (XSingleSelectQueryAnalyzer) UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class, oQueryComposer);
            xQueryComposer = (XSingleSelectQueryComposer) UnoRuntime.queryInterface(XSingleSelectQueryComposer.class, xQueryAnalyzer);
            xSQLComposerFactory = (XSQLQueryComposerFactory) UnoRuntime.queryInterface(XSQLQueryComposerFactory.class, CurDBMetaData.DBConnection);
            XSQLQueryComposer xSQLComposer = xSQLComposerFactory.createQueryComposer();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    private boolean addtoSelectClause(String DisplayFieldName) throws SQLException {
        if (bincludeGrouping){
            if (CurDBMetaData.xDBMetaData.supportsGroupByUnrelated()) {
                if (CurDBMetaData.GroupFieldNames != null) {
                    if (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, DisplayFieldName) > -1)
                        return false;
                }
            }
        }
        return true;
    }

    public void appendSelectClause(boolean _baddAliasFieldNames) throws SQLException {
        String sSelectBaseClause = "SELECT ";
        selectclause = sSelectBaseClause;
        for (int i = 0; i < CurDBMetaData.FieldColumns.length; i++) {
            if (addtoSelectClause(CurDBMetaData.FieldColumns[i].DisplayFieldName)) {
                int iAggregate = CurDBMetaData.getAggregateIndex(CurDBMetaData.FieldColumns[i].DisplayFieldName);
                if (iAggregate > -1) {
                    selectclause += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + getComposedAliasFieldName(CurDBMetaData.AggregateFieldNames[iAggregate][0]) + ")";
                    if (_baddAliasFieldNames)
                        selectclause += getAliasFieldNameClause(CurDBMetaData.AggregateFieldNames[iAggregate][0]);
                } else {
                    selectclause += getComposedAliasFieldName(CurDBMetaData.FieldColumns[i].DisplayFieldName);
                    if (_baddAliasFieldNames)
                        selectclause += getAliasFieldNameClause(CurDBMetaData.FieldColumns[i].DisplayFieldName);
                }
                selectclause += ", ";
            }
        }
        if (selectclause.equals(sSelectBaseClause)){
            selectclause = selectclause.substring(0, selectclause.length() - 1);
        }
        else{
            selectclause = selectclause.substring(0, selectclause.length() - 2);
        }
    }


    public String getAliasFieldNameClause(String _FieldName) {
        String FieldTitle = CurDBMetaData.getFieldTitle(_FieldName);
        if (!FieldTitle.equals(_FieldName))
            return " AS " + CommandName.quoteName(FieldTitle, CurDBMetaData.getIdentifierQuote());
        else
            return "";
    }

    public void appendFilterConditions() throws SQLException {
        try {
            for (int i = 0; i < CurDBMetaData.FilterConditions.length; i++) {
                String CurFieldName = CurDBMetaData.FilterConditions[i][0].Name;
                xQueryComposer.setStructuredFilter(CurDBMetaData.FilterConditions);
            }
            String s = xQueryAnalyzer.getQuery();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void prependSortingCriteria() throws SQLException{
        XIndexAccess xColumnIndexAccess = xQueryAnalyzer.getOrderColumns();
        xQueryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.SortFieldNames.length; i++)
            appendSortingCriterion(i, false);
        for (int i = 0; i < xColumnIndexAccess.getCount(); i++){
            try {
                XPropertySet xColumnPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xColumnIndexAccess.getByIndex(i));
                String sName = (String) xColumnPropertySet.getPropertyValue("Name");
                if (JavaTools.FieldInTable(CurDBMetaData.SortFieldNames, sName) == -1){
                    boolean bascend = AnyConverter.toBoolean(xColumnPropertySet.getPropertyValue("IsAscending"));
                    xQueryComposer.appendOrderByColumn(xColumnPropertySet, bascend);
                }
            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
        }
    }

    private void appendSortingCriterion(int _SortIndex, boolean _baddAliasFieldNames ) throws SQLException{
          XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.SortFieldNames[_SortIndex][0], _baddAliasFieldNames);
        boolean bascend = (CurDBMetaData.SortFieldNames[_SortIndex][1] == "ASC");
        xQueryComposer.appendOrderByColumn(xColumn, bascend);
    }


    public void appendSortingcriteria(boolean _baddAliasFieldNames) throws SQLException {
        String sOrder = "";
        xQueryComposer.setOrder("");
        for (int i = 0; i < CurDBMetaData.SortFieldNames.length; i++) {
            int iAggregate = CurDBMetaData.getAggregateIndex(CurDBMetaData.SortFieldNames[i][0]);
            if (iAggregate > -1){
                sOrder = xQueryAnalyzer.getOrder();
                if (sOrder.length() > 0)
                    sOrder += ", ";
                sOrder += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + CurDBMetaData.AggregateFieldNames[iAggregate][0] + ")";
                sOrder += " " + CurDBMetaData.SortFieldNames[i][1];
                xQueryComposer.setOrder(sOrder);
            }
            else
                appendSortingCriterion(i, _baddAliasFieldNames);
            sOrder = xQueryAnalyzer.getOrder();
        }
    }

    public void appendGroupByColumns(boolean _baddAliasFieldNames) throws SQLException {
        for (int i = 0; i < CurDBMetaData.GroupFieldNames.length; i++) {
            XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.GroupFieldNames[i], _baddAliasFieldNames);
            xQueryComposer.appendGroupByColumn(xColumn);
        }
        String s = xQueryAnalyzer.getQuery();
    }

    public void setDBMetaData(QueryMetaData _oDBMetaData) {
        this.CurDBMetaData = _oDBMetaData;
    }

    private PropertyValue[][] replaceConditionsByAlias(PropertyValue _filterconditions[][]) {
        for (int n = 0; n < _filterconditions.length; n++) {
            for (int m = 0; m < _filterconditions[n].length; m++) {
                _filterconditions[n][m].Name = getComposedAliasFieldName(_filterconditions[n][m].Name);
            }
        }
        return _filterconditions;
    }

    public String getQuery() {
        return xQueryAnalyzer.getQuery();
    }


    public boolean setQueryCommand(String QueryName, XWindow _xParentWindow, boolean _bincludeGrouping, boolean _baddAliasFieldNames) {
        try {
            String s;
            bincludeGrouping = _bincludeGrouping;
            fromclause = "FROM";
            String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
            for (int i = 0; i < sCommandNames.length; i++){
                CommandName curCommandName = new CommandName(CurDBMetaData, sCommandNames[i]); //(setComposedCommandName)
                curCommandName.setAliasName(getuniqueAliasName(curCommandName.getTableName()));
                fromclause += " " + curCommandName.getComposedName() + " " + quoteName(curCommandName.getAliasName());
                if (i < sCommandNames.length - 1)
                    fromclause += ", ";
                composedCommandNames.add(curCommandName);
            }
            appendSelectClause(_baddAliasFieldNames);
            String queryclause = selectclause + " " + fromclause;
            xQueryAnalyzer.setQuery(queryclause);
            if (CurDBMetaData.FilterConditions != null){
                if (CurDBMetaData.FilterConditions.length > 0) {
                    CurDBMetaData.FilterConditions = replaceConditionsByAlias(CurDBMetaData.FilterConditions);
                    xQueryComposer.setStructuredFilter(CurDBMetaData.FilterConditions);
                }
            }
            s = xQueryAnalyzer.getQuery();
            if (_bincludeGrouping){
                appendGroupByColumns(_baddAliasFieldNames);
                if (CurDBMetaData.GroupByFilterConditions.length > 0)
                    xQueryComposer.setStructuredHavingClause(CurDBMetaData.GroupByFilterConditions);
            }
            appendSortingcriteria(_baddAliasFieldNames);
            return true;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            displaySQLErrorDialog(exception, _xParentWindow);
            return false;
        }
    }


    private String getComposedAliasFieldName(String _fieldname){
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByDisplayName(_fieldname);
        CommandName curComposedCommandName = getComposedCommandByDisplayName(CurFieldColumn.getCommandName());
        String curAliasName = curComposedCommandName.getAliasName();
        return quoteName(curAliasName) + "." + quoteName(CurFieldColumn.FieldName);
    }


    private CommandName getComposedCommandByAliasName(String _AliasName) {
        if (composedCommandNames != null) {
            CommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++) {
                curComposedName = (CommandName) composedCommandNames.elementAt(i);
                if (curComposedName.getAliasName().equals(_AliasName))
                    return curComposedName;
            }
        }
        return null;
    }


    public CommandName getComposedCommandByDisplayName(String _DisplayName) {
        if (composedCommandNames != null) {
            CommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++) {
                curComposedName = (CommandName) composedCommandNames.elementAt(i);
                if (curComposedName.getDisplayName().equals(_DisplayName))
                    return curComposedName;
            }
        }
        return null;
    }


    public String getuniqueAliasName(String _TableName) {
        int a = 0;
        String AliasName = "";
        boolean bAliasNameexists = true;
        String locAliasName = _TableName;
        while (bAliasNameexists == true) {
            bAliasNameexists = (getComposedCommandByAliasName(locAliasName) != null);
            if (bAliasNameexists) {
                a++;
                locAliasName = _TableName + "_" + String.valueOf(a);
            } else
                AliasName = locAliasName;
        }
        return AliasName;
    }


    private String quoteName(String _sname){
        return CommandName.quoteName(_sname, CurDBMetaData.getIdentifierQuote());
    }


    public void displaySQLErrorDialog(Exception _exception, XWindow _xParentWindow) {
        try {
            Object oErrorDialog = CurDBMetaData.xMSF.createInstance("com.sun.star.sdb.ErrorMessageDialog");
            XInitialization xInitialize = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oErrorDialog);
            XExecutableDialog xExecute = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oErrorDialog);
            PropertyValue[] rDispatchArguments = new PropertyValue[3];
            rDispatchArguments[0] = Properties.createProperty("Title", Configuration.getProductName(CurDBMetaData.xMSF) + " Base");
            rDispatchArguments[1] = Properties.createProperty("ParentWindow", _xParentWindow);
            rDispatchArguments[2] = Properties.createProperty("SQLException", _exception);
            xInitialize.initialize(rDispatchArguments);
            xExecute.execute();
            //TODO dispose???
        } catch (Exception typeexception) {
            typeexception.printStackTrace(System.out);
        }
    }
}

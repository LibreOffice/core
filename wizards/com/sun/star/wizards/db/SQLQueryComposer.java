/*************************************************************************
*
*  $RCSfile: SQLQueryComposer.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:41:25 $
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

package com.sun.star.wizards.db;

import java.util.Vector;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdb.XSQLQueryComposer;
import com.sun.star.sdb.XSQLQueryComposerFactory;
import com.sun.star.ui.dialogs.XExecutableDialog;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.SQLException;
import com.sun.star.lang.*;
import com.sun.star.awt.XWindow;

import com.sun.star.wizards.common.*;

public class SQLQueryComposer {

    public XColumnsSupplier xColSuppl;
    XSQLQueryComposerFactory xSQLComposerFactory;
    XSQLQueryComposer xSQLQueryComposer;
    QueryMetaData CurDBMetaData;
    //    String Command;
    String selectclause;
    String fromclause;
    String sIdentifierQuoteString = "";
    public XSingleSelectQueryAnalyzer xQueryAnalyzer;
    Vector composedCommandNames = new Vector(1);
    public XSingleSelectQueryComposer xQueryComposer;
    String sCatalogSep;
    boolean bCatalogAtStart;
    XMultiServiceFactory xMSF;

    public SQLQueryComposer(QueryMetaData _CurDBMetaData) {
        try {
            this.CurDBMetaData = _CurDBMetaData;
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, CurDBMetaData.DBConnection);
            Object oQueryComposer = xMSF.createInstance("com.sun.star.sdb.SingleSelectQueryComposer");
            xQueryAnalyzer = (XSingleSelectQueryAnalyzer) UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class, oQueryComposer);
            xQueryComposer = (XSingleSelectQueryComposer) UnoRuntime.queryInterface(XSingleSelectQueryComposer.class, xQueryAnalyzer);
            xSQLComposerFactory = (XSQLQueryComposerFactory) UnoRuntime.queryInterface(XSQLQueryComposerFactory.class, CurDBMetaData.DBConnection);
            XSQLQueryComposer xSQLComposer = xSQLComposerFactory.createQueryComposer();
            sIdentifierQuoteString = CurDBMetaData.xDBMetaData.getIdentifierQuoteString();
            sCatalogSep = CurDBMetaData.xDBMetaData.getCatalogSeparator();
            bCatalogAtStart = CurDBMetaData.xDBMetaData.isCatalogAtStart();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    private boolean addtoSelectClause(String FieldName) throws SQLException {
        if (CurDBMetaData.xDBMetaData.supportsGroupByUnrelated()) {
            if (CurDBMetaData.GroupFieldNames != null) {
                if (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, FieldName) > -1)
                    return false;
            }
        }
        return true;
    }

    public void appendSelectClause() throws SQLException {
        selectclause = "SELECT ";
        for (int i = 0; i < CurDBMetaData.FieldNames.length; i++) {
            if (addtoSelectClause(CurDBMetaData.FieldNames[i])) {
                int iAggregate = CurDBMetaData.getAggregateIndex(CurDBMetaData.FieldNames[i]);
                if (iAggregate > -1) {
                    selectclause += CurDBMetaData.AggregateFieldNames[iAggregate][1] + "(" + getComposedAliasFieldName(CurDBMetaData.AggregateFieldNames[iAggregate][0]) + ")";
                    selectclause += getAliasFieldNameClause(CurDBMetaData.AggregateFieldNames[iAggregate][0]);
                } else {
                    selectclause += getComposedAliasFieldName(CurDBMetaData.FieldNames[i]);
                    selectclause += getAliasFieldNameClause(CurDBMetaData.FieldNames[i]);
                }
                selectclause += ", ";
            }
        }
        selectclause = selectclause.substring(0, selectclause.length() - 2);
    }

    public String getAliasFieldNameClause(String _FieldName) {
        String FieldTitle = CurDBMetaData.getFieldTitle(_FieldName);
        if (!FieldTitle.equals(_FieldName))
            return " AS " + quoteName(FieldTitle);
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

    public void appendSortingcriteria() throws SQLException {
        String sOrder = "";
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
            else {
                XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.SortFieldNames[i][0]);
                boolean bascend = (CurDBMetaData.SortFieldNames[i][1] == "ASC");
                xQueryComposer.appendOrderByColumn(xColumn, bascend);
            }
            sOrder = xQueryAnalyzer.getOrder();
        }
    }

    public void appendGroupByColumns() throws SQLException {
        for (int i = 0; i < CurDBMetaData.GroupFieldNames.length; i++) {
            XPropertySet xColumn = CurDBMetaData.getColumnObjectByFieldName(CurDBMetaData.GroupFieldNames[i]);
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

    // TODO: this method should not be called when it is not really necessary. At least this is for the execution of the query
    public boolean setQueryCommand(String QueryName, XWindow _xParentWindow, boolean _bincludeGrouping) {
        try {
            String s;
            CurDBMetaData.setfieldtitles();
            fromclause = "FROM";
            String[] sCommandNames = CurDBMetaData.getIncludedCommandNames();
            ComposedCommandName curComposedCommandName;
            for (int i = 0; i < sCommandNames.length; i++) {
                curComposedCommandName = new ComposedCommandName(sCommandNames[i]);
                fromclause += " " + curComposedCommandName.sComposedName;
                if (i < sCommandNames.length - 1)
                    fromclause += ", ";
                composedCommandNames.add(curComposedCommandName);
            }
            appendSelectClause();
            String queryclause = selectclause + " " + fromclause;
            xQueryAnalyzer.setQuery(queryclause);
            if (CurDBMetaData.FilterConditions != null) {
                if (CurDBMetaData.FilterConditions.length > 0) {
                    CurDBMetaData.FilterConditions = replaceConditionsByAlias(CurDBMetaData.FilterConditions);
                    xQueryComposer.setStructuredFilter(CurDBMetaData.FilterConditions);
                }
            }
            s = xQueryAnalyzer.getQuery();
            if (_bincludeGrouping){
                appendGroupByColumns();
                if (CurDBMetaData.GroupByFilterConditions.length > 0)
                    xQueryComposer.setStructuredHavingFilter(CurDBMetaData.GroupByFilterConditions);
            }
            s = xQueryAnalyzer.getQuery();
            appendSortingcriteria();
            s = xQueryAnalyzer.getQuery();
            return true;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            displaySQLErrorDialog(exception, _xParentWindow);
            return false;
        }
    }

    private String getComposedAliasFieldName(String _fieldname){
        FieldColumn CurFieldColumn = CurDBMetaData.getFieldColumnByDisplayName(_fieldname);
        ComposedCommandName curComposedCommandName = getComposedCommandByDisplayName(CurFieldColumn.getCommandName());
        String curAliasName = curComposedCommandName.AliasName;
        return quoteName(curAliasName) + "." + quoteName(CurFieldColumn.FieldName);
    }


    private ComposedCommandName getComposedCommandByAliasName(String _AliasName) {
        if (composedCommandNames != null) {
            ComposedCommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++) {
                curComposedName = (ComposedCommandName) composedCommandNames.elementAt(i);
                if (curComposedName.AliasName.equals(_AliasName))
                    return curComposedName;
            }
        }
        return null;
    }

    private ComposedCommandName getComposedCommandByDisplayName(String _DisplayName) {
        if (composedCommandNames != null) {
            ComposedCommandName curComposedName;
            for (int i = 0; i < composedCommandNames.size(); i++) {
                curComposedName = (ComposedCommandName) composedCommandNames.elementAt(i);
                if (curComposedName.DisplayName.equals(_DisplayName))
                    return curComposedName;
            }
        }
        return null;
    }

    private String quoteName(String sName) {
        if (sName == null)
            sName = "";
        String ReturnQuote = "";
        ReturnQuote = sIdentifierQuoteString + sName + sIdentifierQuoteString;
        return ReturnQuote;
    }

    /*    public boolean addtoSelectClause(String CurFieldName){
        boolean bAddToClause = true;
        try{
        bAddToClause = true;
    /*        if (xDBMetaData.supportsGroupBy() == true)
                // if xMetaData.supportsGroupByUnrelated() // dann Groupby Feldnamen nicht ins select clause
            bAddToClause = (Tools.FieldInList(GroupFieldNames, CurFieldName) < 0) && (xDBMetaData.supportsGroupByUnrelated() == false);
            if (bAddToClause == false)
                bAddToClause = (Tools.FieldInTable(SortFieldNames, CurFieldName) < 0) && (xDBMetaData.supportsOrderByUnrelated() == false); */
    //    }
    /*    catch(Exception exception){
            exception.printStackTrace(System.out);
        bAddToClause = false;
        }
        return bAddToClause;
        }*/

    protected class ComposedCommandName {
        public String CatalogName;
        public String SchemaName;
        public String TableName;
        public String AliasName;
        public String DisplayName;
        public String sComposedName = "";

        public ComposedCommandName(String _DisplayName) {
            try {
                this.DisplayName = _DisplayName;
                int iIndex;
                if (CurDBMetaData.xDBMetaData.supportsCatalogsInDataManipulation() == true) { // ...dann Catalog mit in TableName
                    iIndex = _DisplayName.indexOf(sCatalogSep);
                    if (iIndex >= 0) {
                        if (bCatalogAtStart == true) {
                            CatalogName = _DisplayName.substring(0, iIndex);
                            _DisplayName = _DisplayName.substring(iIndex + 1, _DisplayName.length());
                        } else {
                            CatalogName = _DisplayName.substring(iIndex + 1, _DisplayName.length());
                            _DisplayName = _DisplayName.substring(0, iIndex);
                        }
                    }
                }
                if (CurDBMetaData.xDBMetaData.supportsSchemasInDataManipulation() == true) {
                    String[] NameList;
                    NameList = new String[0];
                    NameList = JavaTools.ArrayoutofString(_DisplayName, ".");
                    SchemaName = NameList[0];
                    TableName = NameList[1]; // Todo: Was ist mit diesem Fall: CatalogSep = "." und CatalogName = ""
                } else
                    TableName = _DisplayName;
                setComposedCommandName();
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }

        public void setuniqueAliasName(String _TableName) {
            int a = 0;
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
        }

        public void setComposedCommandName() {
            if (CatalogName != null)
                if (bCatalogAtStart == true)
                    sComposedName = quoteName(CatalogName) + sCatalogSep;
            if (SchemaName != null)
                sComposedName += quoteName(SchemaName) + ".";
            if (sComposedName == "")
                sComposedName = quoteName(TableName);
            else
                sComposedName += quoteName(TableName);
            if ((bCatalogAtStart == false) && (CatalogName != null))
                sComposedName += sCatalogSep + quoteName(CatalogName);
            setuniqueAliasName(TableName);
            sComposedName += " " + quoteName(AliasName);
        }
    }

    public void displaySQLErrorDialog(Exception _exception, XWindow _xParentWindow) {
        try {
            Object oErrorDialog = CurDBMetaData.xMSF.createInstance("com.sun.star.sdb.ErrorMessageDialog");
            XInitialization xInitialize = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oErrorDialog);
            XExecutableDialog xExecute = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oErrorDialog);
            PropertyValue[] rDispatchArguments = new PropertyValue[3];
            //TODO replace by resource
            rDispatchArguments[0] = Properties.createProperty("Title", Configuration.getProductName(CurDBMetaData.xMSF) + "Base");
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

/*************************************************************************
*
*  $RCSfile: CommandMetaData.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: hr $ $Date: 2004-08-02 17:19:19 $
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

import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.beans.*;
import com.sun.star.uno.UnoRuntime;
import java.util.*;
import com.sun.star.lang.Locale;
import com.sun.star.util.XNumberFormats;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.*;

public class CommandMetaData extends DBMetaData {
    public Map FieldTitleSet;
    public String[] AllFieldNames;
    public FieldColumn[] DBFieldColumns;

    public String[] FieldNames = new String[] {};
    public String[] GroupFieldNames = new String[] {};
    public String[][] SortFieldNames = new String[][] {};
    public String[] RecordFieldNames = new String[] {};
    public String[][] AggregateFieldNames = new String[][] {};
    public String[] NumericFieldNames = new String[] {};
    public String[] NonAggregateFieldNames;
    public int[] FieldTypes;
    public int CommandType;

    public CommandMetaData(XMultiServiceFactory xMSF, Locale CharLocale, XNumberFormats NumberFormats) {
        super(xMSF, CharLocale, NumberFormats);
    }

    public CommandMetaData(XMultiServiceFactory xMSF) {
        super(xMSF);
    }

    public void setFieldColumns(boolean _bgetDefaultValue) {
        DBFieldColumns = new FieldColumn[FieldNames.length];
        for (int i = 0; i < FieldNames.length; i++) {
            DBFieldColumns[i] = new FieldColumn(this, FieldNames[i]);
            if (_bgetDefaultValue)
                DBFieldColumns[i].getDefaultValue();
        }
    }

    public XPropertySet getColumnObjectByFieldName(String _FieldColumnName) {
        try {
            FieldColumn CurFieldColumn = getFieldColumnByDisplayName(_FieldColumnName);
            String CurCommandName = CurFieldColumn.getCommandName();
            CommandObject oCommand = getTableByName(CurCommandName);
            Object oColumn = oCommand.xColumns.getByName(CurFieldColumn.FieldName);
            XPropertySet xColumn = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oColumn);
            return xColumn;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public FieldColumn getFieldColumn(String _ColumnName) {
        for (int i = 0; i < FieldNames.length; i++) {
            if (DBFieldColumns[i].FieldName.equals(_ColumnName))
                return DBFieldColumns[i];
        }
        throw new com.sun.star.uno.RuntimeException();
    }

    public FieldColumn getFieldColumnByDisplayName(String _DisplayName) {
        for (int i = 0; i < FieldNames.length; i++) {
            if (DBFieldColumns[i].DisplayFieldName.equals(_DisplayName))
                return DBFieldColumns[i];
        }
        throw new com.sun.star.uno.RuntimeException();
    }

    public void setFieldNames(String[] _FieldNames) {
        this.FieldNames = _FieldNames;
    }

    public void getFieldNamesOfCommand(String _commandname, int _commandtype) {
        try {
            Object oField;
            java.util.Vector ResultFieldNames = new java.util.Vector(10);
            String[] FieldNames;
            CommandObject oCommand = this.getCommandByName(_commandname, _commandtype);
            FieldNames = oCommand.xColumns.getElementNames();

            java.util.Arrays.sort(FieldNames);
            if (FieldNames.length > 0) {
                for (int n = 0; n < FieldNames.length; n++) {
                    oField = oCommand.xColumns.getByName(FieldNames[n]);
                    int iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
                    // BinaryFieldTypes are not included in the WidthList
                    if (JavaTools.FieldInIntTable(WidthList, iType) >= 0) {
                        ResultFieldNames.addElement(_commandname + "." + FieldNames[n]);
                    }
                }
                FieldNames = new String[FieldNames.length];
                FieldTypes = new int[FieldNames.length];
                AllFieldNames = new String[ResultFieldNames.size()];
                ResultFieldNames.copyInto(AllFieldNames);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public boolean hasNumericalFields() {
        return hasNumericalFields(FieldNames);
    }

    public boolean isnumeric(String _DisplayFieldName) {
        try {
            String CurCommandName = FieldColumn.getCommandName(_DisplayFieldName);
            String CurFieldName = FieldColumn.getFieldName(_DisplayFieldName);
            CommandObject oTable = super.getTableByName(CurCommandName);
            Object oField = oTable.xColumns.getByName(CurFieldName);
            int iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
            int ifound = java.util.Arrays.binarySearch(NumericTypes, iType);
            if ((ifound < NumericTypes.length) && (ifound > 0))
                return (NumericTypes[ifound] == iType);
            else
                return false;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return false;
        }
    }

    public String[] setNumericFields() {
        try {

            Vector numericfieldsvector = new java.util.Vector();
            for (int i = 0; i < FieldNames.length; i++) {
                if (isnumeric(FieldNames[i]))
                    numericfieldsvector.addElement(FieldNames[i]);
            }
            NumericFieldNames = new String[numericfieldsvector.size()];
            numericfieldsvector.toArray(NumericFieldNames);
            return NumericFieldNames;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return new String[] {
            };
        }
    }


    public String[] setNonAggregateFieldNames(){
        try {
            Vector nonaggregatefieldsvector = new java.util.Vector();
            for (int i = 0; i < FieldNames.length; i++) {
                if (JavaTools.FieldInTable(AggregateFieldNames, FieldNames[i]) == -1)
                    nonaggregatefieldsvector.addElement(FieldNames[i]);
            }
            NonAggregateFieldNames = new String[nonaggregatefieldsvector.size()];
            nonaggregatefieldsvector.toArray(NonAggregateFieldNames);
            return NonAggregateFieldNames;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return new String[] {
            };
        }
    }

    /**
     * the fieldnames passed over are not necessarily the ones that are defined in the class
     * @param FieldNames
     * @return
     */
    public boolean hasNumericalFields(String[] _DisplayFieldNames) {
        if (_DisplayFieldNames != null) {
            if (_DisplayFieldNames.length > 0) {
                for (int i = 0; i < _DisplayFieldNames.length; i++)
                    if (isnumeric(_DisplayFieldNames[i]))
                        return true;
            }
        }
        return false;
    }

    public String getFieldTitle(String FieldName) {
        String FieldTitle = FieldName;
        int TitleIndex = JavaTools.FieldInList(FieldNames, FieldName);
        if (this.FieldTitleSet != null){
            FieldTitle = (String) this.FieldTitleSet.get(FieldName); //FieldTitles[TitleIndex];
            if (FieldTitle == null)
                return FieldName;
        }
        return FieldTitle;
    }

    public void setGroupFieldNames(String[] GroupFieldNames) {
        this.GroupFieldNames = GroupFieldNames;
    }

    public String[] getGroupFieldNames() {
        return GroupFieldNames;
    }

    public void setRecordFieldNames() {
        String CurFieldName;
        int GroupFieldCount;
        int TotFieldCount = FieldNames.length;
        //    int SortFieldCount = SortFieldNames[0].length;
        GroupFieldCount = JavaTools.getArraylength(GroupFieldNames);
        RecordFieldNames = new String[TotFieldCount - GroupFieldCount];

        int a = 0;
        for (int i = 0; i < TotFieldCount; i++) {
            CurFieldName = FieldNames[i];
            if (JavaTools.FieldInList(GroupFieldNames, CurFieldName) < 0) {
                RecordFieldNames[a] = CurFieldName;
                a += 1;
            }
        }
    }


    public void switchtoDesignmode(String _commandname, int _commandtype) {
        PropertyValue[] rDispatchArguments = new PropertyValue[5];
        rDispatchArguments[0] = Properties.createProperty("DataSourceName", this.DataSourceName);
        rDispatchArguments[1] = Properties.createProperty("QueryDesignView", Boolean.TRUE);
        rDispatchArguments[2] = Properties.createProperty("CreateView", Boolean.FALSE);
        rDispatchArguments[3] = Properties.createProperty("ActiveConnection", this.DBConnection);
        if (_commandtype == com.sun.star.sdb.CommandType.QUERY) {
            rDispatchArguments[4] = Properties.createProperty("CurrentQuery", _commandname);
            showCommandView(".component:DB/QueryDesign", rDispatchArguments);
        } else {
            rDispatchArguments[4] = Properties.createProperty("CurrentTable", _commandname);
            showCommandView(".component:DB/TableDesign", rDispatchArguments);
        }

    }

    public void switchtoDataViewmode(String _commandname, int _commandtype) {
        PropertyValue[] rDispatchArguments = new PropertyValue[7];
        rDispatchArguments[0] = Properties.createProperty("DataSourceName", this.DataSourceName);
        rDispatchArguments[1] = Properties.createProperty("ActiveConnection", this.DBConnection);
        rDispatchArguments[2] = Properties.createProperty("Command", _commandname);
        rDispatchArguments[3] = Properties.createProperty("CommandType", new Integer(_commandtype));
        rDispatchArguments[4] = Properties.createProperty("ShowTreeView", Boolean.FALSE);
        rDispatchArguments[5] = Properties.createProperty("ShowTreeViewButton", Boolean.FALSE);
        rDispatchArguments[6] = Properties.createProperty("ShowMenu", Boolean.TRUE);
        showCommandView(".component:DB/DataSourceBrowser", rDispatchArguments);
    }

    //
    public void showCommandView(String surl, PropertyValue[] _rArgs) {
        try {
            XDesktop xDesktop = Desktop.getDesktop(xMSF);
            XComponentLoader xLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
            xLoader.loadComponentFromURL(surl, "_default", FrameSearchFlag.TASKS | FrameSearchFlag.CREATE, _rArgs);
            //TODO ".component:DB/TableDesign"
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

}

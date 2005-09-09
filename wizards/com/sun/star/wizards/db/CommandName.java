/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommandName.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:23:34 $
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

import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.Exception;
import com.sun.star.wizards.common.JavaTools;

public class CommandName{
    private CommandMetaData CurDBMetaData;
    private String CatalogName = null;
    private String SchemaName = null;
    private String TableName = null;
    private String DisplayName = null;
    private String ComposedName = "";
    private String AliasName = "";
    private boolean bCatalogAtStart;
    private String sCatalogSep;
    private String sIdentifierQuote;
    private boolean baddQuotation = true;


    public CommandName(CommandMetaData _CurDBMetaData, String _DisplayName){
        CurDBMetaData = _CurDBMetaData;
        setComposedCommandName(_DisplayName);
    }


    public CommandName(CommandMetaData _CurDBMetaData, String _CatalogName, String _SchemaName, String _TableName, boolean _baddQuotation){
        baddQuotation = _baddQuotation;
        CurDBMetaData = _CurDBMetaData;
        if (_CatalogName != null){
            if (!_CatalogName.equals("")){
                CatalogName = _CatalogName;
            }
        }
        if (_SchemaName != null){
            if (!_SchemaName.equals("")){
                SchemaName = _SchemaName;
            }
        }
        if (_TableName != null){
            if (!_TableName.equals("")){
                TableName = _TableName;
            }
        }
        setComposedCommandName();
    }


    private void setComposedCommandName(String _DisplayName) {
    try{
        if (this.setMetaDataAttributes()){
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
                TableName = NameList[1]; // TODO Was ist mit diesem Fall: CatalogSep = "." und CatalogName = ""
            } else
                TableName = _DisplayName;
            setComposedCommandName();
    }
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }}


    public void setComposedCommandName() {
        if (this.setMetaDataAttributes()){
            if (CatalogName != null)
                if (bCatalogAtStart == true)
                    ComposedName = quoteName(CatalogName) + sCatalogSep;
            if (SchemaName != null)
                ComposedName += quoteName(SchemaName) + ".";
            if (ComposedName == "")
                ComposedName = quoteName(TableName);
            else
                ComposedName += quoteName(TableName);
            if ((bCatalogAtStart == false) && (CatalogName != null))
                ComposedName += sCatalogSep + quoteName(CatalogName);
        }
    }


    private boolean setMetaDataAttributes(){
    try {
        bCatalogAtStart = CurDBMetaData.xDBMetaData.isCatalogAtStart();
        sCatalogSep = CurDBMetaData.xDBMetaData.getCatalogSeparator();
        sIdentifierQuote = CurDBMetaData.xDBMetaData.getIdentifierQuoteString();
        return true;
    } catch (SQLException e) {
        e.printStackTrace(System.out);
        return false;
    }}


    public String quoteName(String _sName) {
        if (baddQuotation)
            return quoteName(_sName, this.CurDBMetaData.getIdentifierQuote());
        else
            return _sName;
    }


    public static String quoteName(String sName, String _sIdentifierQuote) {
        if (sName == null)
            sName = "";
        String ReturnQuote = "";
        ReturnQuote = _sIdentifierQuote + sName + _sIdentifierQuote;
        return ReturnQuote;
    }


    public void setAliasName(String _AliasName){
        AliasName = _AliasName;
    }


    public String getAliasName(){
        return AliasName;
    }

    /**
     * @return Returns the catalogName.
     */
    public String getCatalogName() {
        return CatalogName;
    }
    /**
     * @return Returns the composedName.
     */
    public String getComposedName() {
        return ComposedName;
    }
    /**
     * @return Returns the displayName.
     */
    public String getDisplayName() {
        return DisplayName;
    }
    /**
     * @return Returns the schemaName.
     */
    public String getSchemaName() {
        return SchemaName;
    }
    /**
     * @return Returns the tableName.
     */
    public String getTableName() {
        return TableName;
    }
}

/*************************************************************************
*
*  $RCSfile: CommandName.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: vg $ $Date: 2005-02-21 13:52:56 $
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

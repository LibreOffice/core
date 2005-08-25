/*************************************************************************
 *
 *  $RCSfile: DatabaseControl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $  $Date: 2005-08-25 13:12:42 $
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
package com.sun.star.wizards.document;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.db.FieldColumn;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class DatabaseControl extends Control {
    int ifieldtype;


    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn){
        super();
        if (_curfieldcolumn.FieldType != DataType.TIMESTAMP)
            createGridColumn(_oGridControl, _curfieldcolumn, _curfieldcolumn.FieldType, _curfieldcolumn.FieldTitle);
    }


    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle){
        super();
        createGridColumn(_oGridControl, _curfieldcolumn, _fieldtype, _columntitle);
    }


    private void createGridColumn(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle){
    try {
        ifieldtype = _fieldtype;
        String suniqueName = Desktop.getUniqueName(_oGridControl.xNameAccess, _curfieldcolumn.FieldName);
        XPropertySet xPropColumn = _oGridControl.xGridColumnFactory.createColumn(getGridColumnName());
        xPropColumn.setPropertyValue("Name", suniqueName);
        xPropColumn.setPropertyValue("Hidden", new Boolean(_fieldtype == DataType.LONGVARBINARY)); //TODO CONTROLType abfragen!!!!!!
        xPropColumn.setPropertyValue("DataField", _curfieldcolumn.FieldName);
        xPropColumn.setPropertyValue("Label", _columntitle);
        xPropColumn.setPropertyValue("Width", new Integer(0));  // Width of column is adjusted to Columname
        setNumerics();
        _oGridControl.xNameContainer.insertByName(_curfieldcolumn.FieldName, xPropColumn);
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public DatabaseControl (FormHandler _oFormHandler, String _sServiceName, Point _aPoint){
        super(_oFormHandler, _sServiceName, _aPoint);
    }


    public DatabaseControl(FormHandler _oFormHandler, XNameContainer _xFormName, String _curFieldName, int _fieldtype, Point _aPoint) {
    super(_oFormHandler, _xFormName, _oFormHandler.getControlType(_fieldtype), _curFieldName, _aPoint, null);
    try {
        this.ifieldtype = _fieldtype;
        Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
        setNumerics();
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public DatabaseControl(FormHandler _oFormHandler, XShapes _xGroupShapes, XNameContainer _xFormName, String _curFieldName, int _fieldtype, Point _aPoint) {
    super(_oFormHandler, _xGroupShapes, _xFormName, _oFormHandler.getControlType(_fieldtype), _aPoint, null);
    try {
        this.ifieldtype = _fieldtype;
        Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
        setNumerics();
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    private String getGridColumnName(){
        for (int i = 0; i < FormHandler.oControlData.length; i++){
            if (FormHandler.oControlData[i].DataType == this.ifieldtype)
                return FormHandler.oControlData[i].GridColumnName;
        }
        return "";
    }


    public int getDBHeight(){
        if (this.icontroltype == FormHandler.SOIMAGECONTROL)
            return oFormHandler.getImageControlHeight();
        else{
            if (this.icontroltype == DataType.LONGVARCHAR){
                Helper.setUnoPropertyValue(xControlModel, "MultiLine", Boolean.TRUE);
                return (oFormHandler.getDBRefHeight() * 4);
            }
            else if (this.icontroltype == FormHandler.SOCHECKBOX)
                return super.getPreferredHeight();
            else
                return oFormHandler.getDBRefHeight();
        }
    }


    public void setNumerics(){
    try {
        if (this.icontroltype == FormHandler.SONUMERICCONTROL){
            xPropertySet.setPropertyValue("TreatAsNumber", Boolean.TRUE);
            switch (ifieldtype){
                case DataType.BIGINT:
                    xPropertySet.setPropertyValue("EffectiveMax", new Double(2147483647 * 2147483647));
                    xPropertySet.setPropertyValue("EffectiveMin", new Double(-(-2147483648 * -2147483648)));
//                  oLocObject.DecimalAccuracy = 0
                    break;
                case DataType.INTEGER:
                    xPropertySet.setPropertyValue("EffectiveMax", new Double(2147483647));
                    xPropertySet.setPropertyValue("EffectiveMin", new Double(-2147483648));
                    break;
                case DataType.SMALLINT:
                    xPropertySet.setPropertyValue("EffectiveMax", new Double(32767));
                    xPropertySet.setPropertyValue("EffectiveMin", new Double(-32768));
                    break;
                case DataType.TINYINT:
                    xPropertySet.setPropertyValue("EffectiveMax", new Double(127));
                    xPropertySet.setPropertyValue("EffectiveMin", new Double(-128));
                    break;
                case DataType.FLOAT:
// TODO:            oLocObject.DecimalAccuracy = ...
                    xPropertySet.setPropertyValue("EffectiveDefault", new Double(1000));
                    break;
                case DataType.REAL:
// TODO:            oLocObject.DecimalAccuracy = ...
                    xPropertySet.setPropertyValue("EffectiveDefault", new Double(1000));
                    break;
                case DataType.DOUBLE:
// TODO:            oLocObject.DecimalAccuracy = ...
                    xPropertySet.setPropertyValue("EffectiveDefault", new Double(1000));
                    break;
                case DataType.DECIMAL:
// TODO:            oLocObject.DecimalAccuracy = ...
                    xPropertySet.setPropertyValue("EffectiveDefault", new Double(1000));
                    break;
                case DataType.NUMERIC:
// TODO:            oLocObject.DecimalAccuracy = ...
                    xPropertySet.setPropertyValue("EffectiveDefault", new Integer(1000));
                    break;

// TODO: HelpText???
            }
//          If oLocObject.PropertySetinfo.HasPropertyByName("Width")Then // Note: an Access AutoincrementField does not provide this property Width
//              oLocObject.Width = CurFieldLength + CurScale + 1
//          End If
//          If CurIsCurrency Then
// TODO: How do you set currencies?
//          End If
        }
        else if (icontroltype == FormHandler.SOTEXTBOX){     // com.sun.star.sdbc.DataType.CHAR, com.sun.star.sdbc.DataType.VARCHAR, com.sun.star.sdbc.DataType.LONGVARCHAR
//          If CurFieldLength = 0 Then           // Or oLocObject.MaxTextLen > SBMAXTEXTSIZE
//              oLocObject.MaxTextLen = SBMAXTEXTSIZE
//              CurFieldLength = SBMAXTEXTSIZE
//          else
//              oLocObject.MaxTextLen = CurFieldLength
//          End If
//          oLocObject.DefaultText = CurDefaultValue
        }
//      else if (CurControlType == cDateBox)
// TODO Why does this not work?:        oLocObject.DefaultDate = CurDefaultValue
        else if (icontroltype == FormHandler.SOTIMECONTROL) // com.sun.star.sdbc.DataType.DATE, com.sun.star.sdbc.DataType.TIME
            xPropertySet.setPropertyValue("DefaultTime", new Integer(3000));
// TODO: Property TimeFormat? frome where?
        else if (icontroltype == FormHandler.SOCHECKBOX){
// TODO Why does this not work?:        oLocObject.DefautState = CurDefaultValue
        }
//      If oLocObject.PropertySetInfo.HasPropertybyName("FormatKey") Then
//          oLocObject.FormatKey = CurFormatKey
//      End If
    } catch (Exception e) {
        // TODO Auto-generated catch block
        e.printStackTrace(System.out);
    }}


    /**
     * @return
     */
}

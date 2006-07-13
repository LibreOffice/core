/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DatabaseControl.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 15:17:41 $
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
 ************************************************************************/package com.sun.star.wizards.document;

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
    protected int ifieldtype;
    private int iMemofieldwidth = IIMGFIELDWIDTH;
    private int iMemofieldheight = -1;



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
            if (this.ifieldtype == DataType.LONGVARCHAR){
                if (iMemofieldheight == -1){
                    Helper.setUnoPropertyValue(xControlModel, "MultiLine", Boolean.TRUE);
                    iMemofieldheight = oFormHandler.getDBRefHeight() * 4;
                }
                return iMemofieldheight;
            }
            else if (this.icontroltype == FormHandler.SOCHECKBOX)
                return super.getPreferredHeight();
            else
                return oFormHandler.getDBRefHeight();
        }
    }


    public int getDBWidth(){
        if (this.ifieldtype == DataType.LONGVARCHAR){
            return 2 * getDBHeight();
        }
        else{
            return getPreferredWidth();
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
        }
        else if (icontroltype == FormHandler.SOTEXTBOX){     // com.sun.star.sdbc.DataType.CHAR, com.sun.star.sdbc.DataType.VARCHAR, com.sun.star.sdbc.DataType.LONGVARCHAR
        }
        else if (icontroltype == FormHandler.SOCHECKBOX){
// TODO Why does this not work?:        oLocObject.DefautState = CurDefaultValue
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    /**
     * @return
     */
}

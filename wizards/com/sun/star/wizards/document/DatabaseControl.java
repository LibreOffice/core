/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DatabaseControl.java,v $
 * $Revision: 1.9 $
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
import com.sun.star.uno.AnyConverter;
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
public class DatabaseControl extends Control
{

    protected int ifieldtype;
    private int iMemofieldwidth = IIMGFIELDWIDTH;
    private int iMemofieldheight = -1;
    private FieldColumn m_FieldColumn;

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn)
    {
        super();
        m_FieldColumn = _curfieldcolumn;
        if (_curfieldcolumn.FieldType != DataType.TIMESTAMP)
        {
            createGridColumn(_oGridControl, _curfieldcolumn, _curfieldcolumn.FieldType, _curfieldcolumn.getFieldTitle());
        }
    }

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle)
    {
        super();
        m_FieldColumn = _curfieldcolumn;
        createGridColumn(_oGridControl, _curfieldcolumn, _fieldtype, _columntitle);
    }

    private void createGridColumn(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle)
    {
        try
        {
            ifieldtype = _fieldtype;
            String suniqueName = Desktop.getUniqueName(_oGridControl.xNameAccess, _curfieldcolumn.m_sFieldName);
            XPropertySet xPropColumn = _oGridControl.xGridColumnFactory.createColumn(getGridColumnName());
            xPropColumn.setPropertyValue("Name", suniqueName);
            xPropColumn.setPropertyValue("Hidden", new Boolean(_fieldtype == DataType.LONGVARBINARY)); //TODO CONTROLType abfragen!!!!!!
            xPropColumn.setPropertyValue("DataField", _curfieldcolumn.m_sFieldName);
            xPropColumn.setPropertyValue("Label", _columntitle);
            xPropColumn.setPropertyValue("Width", new Integer(0));  // Width of column is adjusted to Columname
            setNumerics();
            _oGridControl.xNameContainer.insertByName(_curfieldcolumn.m_sFieldName, xPropColumn);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public DatabaseControl(FormHandler _oFormHandler, String _sServiceName, Point _aPoint)
    {
        super(_oFormHandler, _sServiceName, _aPoint);
    }

    public DatabaseControl(FormHandler _oFormHandler, XNameContainer _xFormName, String _curFieldName, int _fieldtype, Point _aPoint)
    {
        super(_oFormHandler, _xFormName, _oFormHandler.getControlType(_fieldtype), _curFieldName, _aPoint, null);
        try
        {
            this.ifieldtype = _fieldtype;
            Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
            setNumerics();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public DatabaseControl(FormHandler _oFormHandler, XShapes _xGroupShapes, XNameContainer _xFormName, String _curFieldName, int _fieldtype, Point _aPoint)
    {
        super(_oFormHandler, _xGroupShapes, _xFormName, _oFormHandler.getControlType(_fieldtype), _aPoint, null);
        try
        {
            this.ifieldtype = _fieldtype;
            Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
            setNumerics();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private String getGridColumnName()
    {
        for (int i = 0; i < FormHandler.oControlData.length; i++)
        {
            if (FormHandler.oControlData[i].DataType == this.ifieldtype)
            {
                return FormHandler.oControlData[i].GridColumnName;
            }
        }
        return "";
    }

    public int getDBHeight()
    {
        if (this.icontroltype == FormHandler.SOIMAGECONTROL)
        {
            return oFormHandler.getImageControlHeight();
        }
        else
        {
            if (this.ifieldtype == DataType.LONGVARCHAR)
            {
                if (iMemofieldheight == -1)
                {
                    Helper.setUnoPropertyValue(xControlModel, "MultiLine", Boolean.TRUE);
                    iMemofieldheight = oFormHandler.getDBRefHeight() * 4;
                }
                return iMemofieldheight;
            }
            else if (this.icontroltype == FormHandler.SOCHECKBOX)
            {
                return super.getPreferredHeight();
            }
            else
            {
                return oFormHandler.getDBRefHeight();
            }
        }
    }

    public int getDBWidth()
    {
        if (this.ifieldtype == DataType.LONGVARCHAR)
        {
            return 2 * getDBHeight();
        }
        else
        {
            return getPreferredWidth();
        }
    }

    public void setNumerics()
    {
        try
        {
            if (this.icontroltype == FormHandler.SONUMERICCONTROL)
            {
                xPropertySet.setPropertyValue("TreatAsNumber", Boolean.TRUE);
                switch (ifieldtype)
                {
                    case DataType.BIGINT:
                        xPropertySet.setPropertyValue("EffectiveMax", new Double(2147483647 * 2147483647));
                        xPropertySet.setPropertyValue("EffectiveMin", new Double(-(-2147483648 * -2147483648)));
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
                    case DataType.REAL:
                    case DataType.DOUBLE:
                    case DataType.DECIMAL:
                    case DataType.NUMERIC:
                        break;
                }
            }
            else if (icontroltype == FormHandler.SOTEXTBOX)
            {     // com.sun.star.sdbc.DataType.CHAR, com.sun.star.sdbc.DataType.VARCHAR, com.sun.star.sdbc.DataType.LONGVARCHAR
            }
            else if (icontroltype == FormHandler.SOCHECKBOX)
            {
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }
    /**
     * @return
     */
}

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

package com.sun.star.wizards.document;

import com.sun.star.awt.Point;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XShapes;
import com.sun.star.sdbc.DataType;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.FieldColumn;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class DatabaseControl extends Control
{

    private int m_nFieldType;
    // private int iMemofieldwidth = IIMGFIELDWIDTH;
    // private int iMemofieldheight = -1;
    // private FieldColumn m_FieldColumn;

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn)
    {
        super();
        // m_FieldColumn = _curfieldcolumn;
        if (_curfieldcolumn.getFieldType() != DataType.TIMESTAMP)
        {
            createGridColumn(_oGridControl, _curfieldcolumn, _curfieldcolumn.getFieldType(), _curfieldcolumn.getFieldTitle());
        }
    }

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle)
    {
        super();
        // m_FieldColumn = _curfieldcolumn;
        createGridColumn(_oGridControl, _curfieldcolumn, _fieldtype, _columntitle);
    }

    protected int getFieldType()
    {
        return m_nFieldType;
    }

    private void createGridColumn(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle)
    {
        try
        {
            m_nFieldType = _fieldtype;
            String sFieldName = _curfieldcolumn.getFieldName();
            String sUniqueName = Desktop.getUniqueName(_oGridControl.xNameAccess, sFieldName);

            String sGridColumnName = getGridColumnName();
            XPropertySet xPropColumn = _oGridControl.xGridColumnFactory.createColumn(sGridColumnName);
            xPropColumn.setPropertyValue(PropertyNames.PROPERTY_NAME, sUniqueName);
            boolean bHidden = false;
            if (_fieldtype == DataType.LONGVARBINARY)  //TODO CONTROLType abfragen!!!!!!
            {
                bHidden = true;
            }
            xPropColumn.setPropertyValue("Hidden", new Boolean(bHidden));
            xPropColumn.setPropertyValue("DataField", sFieldName);
            xPropColumn.setPropertyValue(PropertyNames.PROPERTY_LABEL, _columntitle);
            xPropColumn.setPropertyValue(PropertyNames.PROPERTY_WIDTH, 0);  // Width of column is adjusted to Columname

            XPropertySetInfo xPSI = xPropColumn.getPropertySetInfo();
            if ( xPSI.hasPropertyByName( "MouseWheelBehavior" ) )
                xPropColumn.setPropertyValue( "MouseWheelBehavior", new Short( com.sun.star.awt.MouseWheelBehavior.SCROLL_DISABLED ) );

            setNumericLimits();
            _oGridControl.xNameContainer.insertByName(sFieldName, xPropColumn);
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
            m_nFieldType = _fieldtype;
            Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
            setNumericLimits();
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
            m_nFieldType = _fieldtype;
            Helper.setUnoPropertyValue(xControlModel, "DataField", _curFieldName);
            setNumericLimits();
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
            if (FormHandler.oControlData[i].DataType == getFieldType())
            {
                return FormHandler.oControlData[i].GridColumnName;
            }
        }
        return "";
    }

    public int getControlHeight()
    {
        if (getControlType() == FormHandler.SOIMAGECONTROL)
        {
            // return oFormHandler.getImageControlHeight();
            final int nMemofieldheight = oFormHandler.getControlReferenceHeight() * 4;
            return nMemofieldheight;
        }
        else
        {
            if (getFieldType() == DataType.LONGVARCHAR)
            {
                // Helper.setUnoPropertyValue(xControlModel, PropertyNames.PROPERTY_MULTILINE, Boolean.TRUE);
                final int nMemofieldheight = oFormHandler.getControlReferenceHeight() * 4;
                return nMemofieldheight;
            }
            else if (getControlType() == FormHandler.SOCHECKBOX)
            {
                return super.getPreferredHeight();
            }
        }
        return oFormHandler.getControlReferenceHeight();
    }

    public int getControlWidth()
    {
        if (getControlType() == FormHandler.SOIMAGECONTROL)
        {
            return 2 * getControlHeight();
        }
        if (getFieldType() == DataType.LONGVARCHAR)
        {
            return 2 * getControlHeight();
        }
        else
        {
            return getPreferredWidth();
        }
    }

    private static long m_nLongMax = 0;
    public static long getLongMax()
    {
        if (m_nLongMax == 0)
        {
        }
        return m_nLongMax;
    }
    public void setNumericLimits()
    {
        try
        {
            if (getControlType() == FormHandler.SONUMERICCONTROL)
            {
                xPropertySet.setPropertyValue("TreatAsNumber", Boolean.TRUE);
                // Math.
                // 2^63 - 1 = 9223372036854775807
                // int nIntMax = 0x7fffffff;
                // int nIntMin = -0x80000000;
                // long nLongMax = nIntMax;
                // nLongMax <<= (4*4);
                // nLongMax |= 0xffff;
                // nLongMax <<= (4*4);
                // nLongMax |= 0xffff;

                // long nLongMin = nIntMin;
                // nLongMin <<= (8*4);

                // long nLong2 = (long)9223372036854775807;

                switch (getFieldType())
                {
                    case DataType.BIGINT:
                        xPropertySet.setPropertyValue("EffectiveMax", new Double(Long.MAX_VALUE));
                        xPropertySet.setPropertyValue("EffectiveMin", new Double(Long.MIN_VALUE));
                        break;
                    case DataType.INTEGER:
                        xPropertySet.setPropertyValue("EffectiveMax", new Double(Integer.MAX_VALUE));
                        xPropertySet.setPropertyValue("EffectiveMin", new Double(Integer.MIN_VALUE));
                        break;
                    case DataType.SMALLINT:
                        xPropertySet.setPropertyValue("EffectiveMax", new Double(Short.MAX_VALUE));
                        xPropertySet.setPropertyValue("EffectiveMin", new Double(Short.MIN_VALUE));
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
            // else if (getControlType() == FormHandler.SOTEXTBOX)
            // {     // com.sun.star.sdbc.DataType.CHAR, com.sun.star.sdbc.DataType.VARCHAR, com.sun.star.sdbc.DataType.LONGVARCHAR
            // }
            else if (getControlType() == FormHandler.SOIMAGECONTROL)
            {
                xPropertySet.setPropertyValue("ScaleMode", com.sun.star.awt.ImageScaleMode.Isotropic);
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

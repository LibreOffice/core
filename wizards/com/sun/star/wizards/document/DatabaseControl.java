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
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class DatabaseControl extends Control
{

    private int m_nFieldType;

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn)
    {
        super();
        if (_curfieldcolumn.getFieldType() != DataType.TIMESTAMP)
        {
            createGridColumn(_oGridControl, _curfieldcolumn, _curfieldcolumn.getFieldType(), _curfieldcolumn.getFieldTitle());
        }
    }

    public DatabaseControl(GridControl _oGridControl, FieldColumn _curfieldcolumn, int _fieldtype, String _columntitle)
    {
        super();
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
            xPropColumn.setPropertyValue("Hidden", Boolean.valueOf(bHidden));
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
            e.printStackTrace(System.err);
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
            e.printStackTrace(System.err);
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
            e.printStackTrace(System.err);
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
        return PropertyNames.EMPTY_STRING;
    }

    public int getControlHeight()
    {
        if (getControlType() == FormHandler.SOIMAGECONTROL)
        {
            return oFormHandler.getControlReferenceHeight() * 4;
        }
        else
        {
            if (getFieldType() == DataType.LONGVARCHAR)
            {
                return oFormHandler.getControlReferenceHeight() * 4;
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
            else if (getControlType() == FormHandler.SOIMAGECONTROL)
            {
                xPropertySet.setPropertyValue("ScaleMode", com.sun.star.awt.ImageScaleMode.Isotropic);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }
    /**
     * @return
     */
}

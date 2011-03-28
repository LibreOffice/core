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
package com.sun.star.wizards.form;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.DataType;
import com.sun.star.task.XStatusIndicator;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.*;
import com.sun.star.wizards.document.Control;
import com.sun.star.wizards.document.DatabaseControl;
import com.sun.star.wizards.document.FormHandler;
import com.sun.star.wizards.document.Shape;
import com.sun.star.wizards.document.TimeStampControl;

public class FormControlArranger
{

    public static final String LABELCONTROL = "LabelControl";
    protected DatabaseControl[] DBControlList = null;
    private XNameContainer xFormName;
    private XMultiServiceFactory xMSF;
    private Control[] LabelControlList = null;
    private XStatusIndicator xProgressBar;
    private FieldColumn[] FieldColumns;
    // Control curLabelControl;
    private int icurArrangement;
    private boolean bIsFirstRun;
    private boolean bIsVeryFirstRun;
    private boolean bControlsareCreated;
    private int cXOffset;
    private int cYOffset;
    private static final int cVertDistance = 200;
    private static final int cHoriDistance = 300;
    private static final int cLabelGap = 100;
    private static final double CMAXREDUCTION = 0.7;
    private FormHandler oFormHandler;
    private int iReduceWidth;
    private int m_currentLabelPosX;
    private int m_currentLabelPosY;
    private int m_currentControlPosX;
    private int m_currentControlPosY;
    private int m_LabelHeight;
    private int m_LabelWidth;
    private int m_dbControlHeight;
    private int m_dbControlWidth;
    private int m_MaxLabelWidth;
    private int nFormWidth;
    private int nFormHeight;
    private int m_currentMaxRowHeight;
    private int nSecMaxRowY;
    private int m_maxPostionX;
    private int a;
    private int StartA;
    private int m_controlMaxPosY = 0;     //the maximum YPosition of a DBControl in the form
    private Short NBorderType = new Short((short) 1); //3-D Border

    public FormControlArranger(FormHandler _oFormHandler, XNameContainer _xFormName, CommandMetaData oDBMetaData, XStatusIndicator _xProgressBar, Point _StartPoint, Size _FormSize)
    {
        FieldColumns = oDBMetaData.FieldColumns;
        xMSF = oDBMetaData.xMSF;
        xFormName = _xFormName;
        xProgressBar = _xProgressBar;
        LabelControlList = new Control[FieldColumns.length];
        DBControlList = new DatabaseControl[FieldColumns.length];
        oFormHandler = _oFormHandler;
        cXOffset = _StartPoint.X;
        cYOffset = _StartPoint.Y;
        setFormSize(_FormSize);
    }
    // Note: on all Controls except for the checkbox the Label has to be set
    // a bit under the DBControl because its Height is also smaller

    private int getLabelDiffHeight(int _index)
    {
        final DatabaseControl curDBControl = DBControlList[_index];
        if (curDBControl != null && curDBControl.getControlType() == FormHandler.SOCHECKBOX)
        {
            return getCheckBoxDiffHeight(_index);
        }
        return oFormHandler.getBasicLabelDiffHeight();
    }

    public void setBorderType(short _nBorderType)
    {
        NBorderType = new Short(_nBorderType);
    }

    public Control[] getLabelControlList()
    {
        return LabelControlList;
    }

    private int getCheckBoxDiffHeight(int LastIndex)
    {
        if (LastIndex < DBControlList.length && DBControlList[LastIndex].getControlType() == FormHandler.SOCHECKBOX)
        {
            return (int) ((oFormHandler.getControlReferenceHeight() - DBControlList[LastIndex].getControlHeight()) / 2);
        }
        return 0;
    }

    private boolean isReducable(int _index, int i_labelWidth, int i_dbControlWidth)
    {
        boolean bisreducable = false;
        int ntype = FieldColumns[_index].getFieldType();
        switch (ntype)
        {
            case DataType.TINYINT:
            case DataType.SMALLINT:
            case DataType.INTEGER:
            case DataType.FLOAT:
            case DataType.DATE:
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.REAL:
            case DataType.DOUBLE:
            case DataType.NUMERIC:
            case DataType.DECIMAL:
            case DataType.BIT:
            case DataType.BOOLEAN:
                bisreducable = false;
                break;
            case DataType.VARCHAR:
                short nTextLen;
                try
                {
                    nTextLen = AnyConverter.toShort(DBControlList[_index].xPropertySet.getPropertyValue("MaxTextLen"));
                    if ((nTextLen == 0) || (nTextLen > 20))
                    {
                        bisreducable = true;
                    }
                }
                catch (Exception e)
                {
                    e.printStackTrace(System.out);
                }
                break;
            case DataType.BIGINT:
                bisreducable = true;
                break;
            default:
                bisreducable = true;
        }
        if (bisreducable && i_labelWidth > 0.9 * CMAXREDUCTION * i_dbControlWidth)
        {
            bisreducable = false;
        }
        return bisreducable;
    }

    private int getControlGroupWidth()
    {
        if (m_dbControlWidth > m_LabelWidth)
        {
            return m_dbControlWidth;
        }
        else
        {
            return m_LabelWidth;
        }
    }

    private void checkJustifiedPosition(int a)
    {
        int nBaseWidth = nFormWidth + cXOffset;
        int nLeftDist = m_maxPostionX - nBaseWidth;
        int nRightDist = nBaseWidth - (DBControlList[a].getPosition().X - cHoriDistance);
        if (nLeftDist < 0.5 * nRightDist)
        {
            // Fieldwidths in the line can be made smaller..
            adjustLineWidth(StartA, a, nLeftDist, -1);
            m_currentLabelPosY = m_currentMaxRowHeight + cVertDistance;
            m_currentControlPosY = m_currentLabelPosY + m_LabelHeight;
            m_currentLabelPosX = cXOffset;
            m_currentControlPosX = cXOffset;
            bIsFirstRun = true;
            StartA = a + 1;
        }
        else
        {
            // FieldWidths in the line can be made wider...
            if (m_currentControlPosY + m_dbControlHeight == m_currentMaxRowHeight)
            {
                // The last Control was the highest in the row
                m_currentLabelPosY = nSecMaxRowY;
            }
            else
            {
                m_currentLabelPosY = m_currentMaxRowHeight;
            }
            m_currentLabelPosY += cVertDistance;
            m_currentControlPosY = m_currentLabelPosY + m_LabelHeight;
            m_currentControlPosX = cXOffset;
            m_currentLabelPosX = cXOffset;
            LabelControlList[a].setPosition(new Point(cXOffset, m_currentLabelPosY));
            DBControlList[a].setPosition(new Point(cXOffset, m_currentControlPosY));
            bIsFirstRun = true;
            checkOuterPoints(m_currentControlPosX, m_dbControlWidth > m_LabelWidth ? m_dbControlWidth : m_LabelWidth, m_currentControlPosY, m_dbControlHeight, true);
            m_currentLabelPosX = m_maxPostionX + cHoriDistance;
            m_currentControlPosX = m_currentLabelPosX;
            adjustLineWidth(StartA, a - 1, nRightDist, 1);
            StartA = a;
        }
    }

    private int getCorrWidth(int StartIndex, int EndIndex, int nDist, int Widthfactor)
    {
        int ShapeCount;
        if (Widthfactor > 0)
        {
            // shapes are made wide
            ShapeCount = EndIndex - StartIndex + 1;
        }
        else
        {
            // shapes are made more narrow
            ShapeCount = iReduceWidth;
        }
        return (nDist) / ShapeCount;
    }

    /**
     *
     * @param StartIndex
     * @param EndIndex
     * @param nDist
     * @param WidthFactor is either '+1' or '-1' and determines whether the control shapes widths are to be made smaller or larger
     */
    private void adjustLineWidth(int StartIndex, int EndIndex, int nDist, int WidthFactor)
    {
        int CorrWidth = getCorrWidth(StartIndex, EndIndex, nDist, WidthFactor);
        int iLocTCPosX = cXOffset;
        for (int i = StartIndex; i <= EndIndex; i++)
        {
            int nControlBaseWidth = 0;
            DatabaseControl dbControl = DBControlList[i];
            Control curLabelControl = LabelControlList[i];
            if (i != StartIndex)
            {
                curLabelControl.setPosition(new Point(iLocTCPosX, curLabelControl.getPosition().Y));
                dbControl.setPosition(new Point(iLocTCPosX, curLabelControl.getPosition().Y + m_LabelHeight));
            }
            final Size labelSize = curLabelControl.getSize();
            Size controlSize = dbControl.getSize();
            if (((labelSize.Width > controlSize.Width)) && (WidthFactor > 0))
            {
                nControlBaseWidth = labelSize.Width;
            }
            else
            {
                nControlBaseWidth = controlSize.Width;
            }
            if (FieldColumns[i].getFieldType() == DataType.TIMESTAMP)
            {
                TimeStampControl oDBTimeStampControl = (TimeStampControl) dbControl;
                nControlBaseWidth = oDBTimeStampControl.getSize().Width;
            }
            if (WidthFactor > 0 || isReducable(i, labelSize.Width, controlSize.Width))
            {
                controlSize.Width = nControlBaseWidth + WidthFactor * CorrWidth;
                dbControl.setSize(controlSize);
                controlSize = dbControl.getSize();
            }

            if (labelSize.Width > controlSize.Width)
            {
                iLocTCPosX += labelSize.Width;
            }
            else
            {
                iLocTCPosX += controlSize.Width;
            }
            iLocTCPosX += cHoriDistance;
        }
        if (WidthFactor > 0)
        {
            iReduceWidth = 1;
        }
        else
        {
            iReduceWidth = 0;
        }
    }

    private void checkOuterPoints(int i_nXPos, int i_nWidth, int i_nYPos, int i_nHeight, boolean i_bIsDBField)
    {
        if (icurArrangement == FormWizard.IN_BLOCK_TOP && i_bIsDBField)
        {
            // Only at DBControls you can measure the Value of nMaxRowY
            if (bIsFirstRun)
            {
                m_currentMaxRowHeight = i_nYPos + i_nHeight;
                nSecMaxRowY = m_currentMaxRowHeight;
            }
            else
            {
                int nRowY = i_nYPos + i_nHeight;
                if (nRowY >= m_currentMaxRowHeight)
                {
                    nSecMaxRowY = m_currentMaxRowHeight;
                    m_currentMaxRowHeight = nRowY;
                }
            }
        }
        // Find the outer right point
        if (bIsFirstRun)
        {
            m_maxPostionX = i_nXPos + i_nWidth;
            bIsFirstRun = false;
        }
        else
        {
            int nColRightX = i_nXPos + i_nWidth;
            if (nColRightX > m_maxPostionX)
            {
                m_maxPostionX = nColRightX;
            }
        }
    }

    public void positionControls(int _icurArrangement, Point _aStartPoint, Size _aFormSize, short _iAlign, Short _NBorderType)
    {
        try
        {
            NBorderType = _NBorderType;
            setStartPoint(_aStartPoint);
            icurArrangement = _icurArrangement;
            initializePosSizes();
            initializeControlColumn(-1);
            bIsVeryFirstRun = true;
            m_currentMaxRowHeight = 0;
            nSecMaxRowY = 0;
            m_maxPostionX = 0;
            xProgressBar.start("", FieldColumns.length);
            for (int i = 0; i < FieldColumns.length; i++)
            {
                try
                {
                    insertLabel(i, _iAlign);
                    insertDBControl(i);
                    bIsVeryFirstRun = false;
                    DBControlList[i].setPropertyValue(LABELCONTROL, LabelControlList[i].xPropertySet);
                    resetPosSizes(i);
                    xProgressBar.setValue(i + 1);
                }
                catch (RuntimeException e)
                {
                }
            }
            xProgressBar.end();
            bControlsareCreated = true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public boolean areControlsexisting()
    {
        if (DBControlList != null)
        {
            if (DBControlList.length > 0)
            {
                return (DBControlList[0] != null);
            }
        }
        return false;
    }

    private void initializeControlColumn(int LastIndex)
    {
        bIsFirstRun = true;
        StartA = LastIndex + 1;
        a = 0;
    }

    private void resetPosSizes(int LastIndex)
    {
        int nYRefPos = m_currentControlPosY;
        switch (icurArrangement)
        {
            case FormWizard.COLUMNAR_LEFT:
                m_currentControlPosY = m_currentControlPosY + m_dbControlHeight + cVertDistance + getCheckBoxDiffHeight(LastIndex);
                nYRefPos = m_currentControlPosY;
                if ((m_currentControlPosY > cYOffset + nFormHeight) || (LastIndex == (FieldColumns.length - 1)))
                {
                    repositionColumnarLeftControls(LastIndex);
                    m_currentLabelPosX = m_maxPostionX + 2 * cHoriDistance;
                    m_currentControlPosX = m_currentLabelPosX + cLabelGap + m_MaxLabelWidth;
                    m_currentControlPosY = cYOffset;
                    nYRefPos = m_currentControlPosY;
                    initializeControlColumn(LastIndex);
                }
                else
                {
                    /*a = a + 1;*/
                    /* a += 1;*/
                    ++a;
                }
                m_currentLabelPosY = m_currentControlPosY + getLabelDiffHeight(LastIndex);
                if ((nYRefPos + m_dbControlHeight) > m_controlMaxPosY)
                {
                    m_controlMaxPosY = nYRefPos + m_dbControlHeight;
                }

                break;
            case FormWizard.COLUMNAR_TOP:
                m_currentLabelPosY = m_currentControlPosY + m_dbControlHeight + cVertDistance + getCheckBoxDiffHeight(LastIndex);

                if ((m_currentLabelPosY > cYOffset + nFormHeight) || (LastIndex == (FieldColumns.length - 1)))
                {
                    m_currentControlPosX = m_maxPostionX + cHoriDistance;
                    m_currentLabelPosX = m_currentControlPosX;
                    nYRefPos = m_currentControlPosY;
                    m_currentControlPosY = cYOffset + m_LabelHeight + cVertDistance;
                    m_currentLabelPosY = cYOffset;
                    initializeControlColumn(LastIndex);
                }
                else
                {
                    ++a;
                }
                if ((nYRefPos + m_dbControlHeight + cVertDistance) > m_controlMaxPosY)
                {
                    m_controlMaxPosY = nYRefPos + m_dbControlHeight + cVertDistance;
                }
                break;

            case FormWizard.IN_BLOCK_TOP:
                if (isReducable(a, m_LabelWidth, m_dbControlWidth))
                {
                    ++iReduceWidth;
                }
                //if (m_maxPostionX > (nFormWidth-cXOffset-cXOffset)) // cXOffset + nFormWidth
                if (m_maxPostionX > cXOffset + nFormWidth)
                {
                    checkJustifiedPosition(a);
                    nYRefPos = m_currentControlPosY;
                }
                else
                {
                    m_currentLabelPosX = m_maxPostionX + cHoriDistance;
                }
                if (a == FieldColumns.length - 1)
                {
                    checkJustifiedPosition(a);
                    nYRefPos = m_currentControlPosY;
                }
                m_currentControlPosX = m_currentLabelPosX;
                ++a;
                if ((nYRefPos + m_dbControlHeight) > m_controlMaxPosY)
                {
                    m_controlMaxPosY = nYRefPos + m_dbControlHeight;
                }
                break;
        }
    }

    private void repositionColumnarLeftControls(int LastIndex)
    {
        bIsFirstRun = true;
        for (int i = StartA; i <= LastIndex; i++)
        {
            if (i == StartA)
            {
                m_currentLabelPosX = LabelControlList[i].getPosition().X;
                m_currentControlPosX = m_currentLabelPosX + m_MaxLabelWidth + cHoriDistance;
            }
            LabelControlList[i].setSize(new Size(m_MaxLabelWidth, m_LabelHeight));
            resetDBShape(DBControlList[i], m_currentControlPosX);
            checkOuterPoints(m_currentControlPosX, m_dbControlWidth, m_currentControlPosY, m_dbControlHeight, true);
        }
    }

    private void resetDBShape(Shape _curDBControl, int iXPos)
    {
        m_dbControlWidth = _curDBControl.getSize().Width;
        m_dbControlHeight = _curDBControl.getSize().Height;
        _curDBControl.setPosition(new Point(iXPos, _curDBControl.getPosition().Y));
    }

    private void initializePosSizes()
    {
        m_controlMaxPosY = 0;
        m_currentLabelPosX = cXOffset;
        m_LabelWidth = 2000;
        m_dbControlWidth = 2000;
        m_dbControlHeight = oFormHandler.getControlReferenceHeight();
        m_LabelHeight = oFormHandler.getLabelHeight();
        iReduceWidth = 0;
        if (icurArrangement == FormWizard.COLUMNAR_LEFT)
        {
            m_currentLabelPosY = cYOffset + getLabelDiffHeight(0);
            m_currentControlPosX = cXOffset + 3050;
            m_currentControlPosY = cYOffset;
        }
        else
        {
            m_currentControlPosX = cXOffset;
            m_currentLabelPosY = cYOffset;
        }
    }

    private void insertLabel(int i, int _iAlign)
    {
        try
        {
            Point aPoint = new Point(m_currentLabelPosX, m_currentLabelPosY);
            Size aSize = new Size(m_LabelWidth, m_LabelHeight);
            if (bControlsareCreated)
            {
                LabelControlList[i].setPosition(aPoint);
                if (icurArrangement != FormWizard.COLUMNAR_LEFT)
                {
                    m_LabelWidth = LabelControlList[i].getPreferredWidth(FieldColumns[i].getFieldTitle());
                    aSize.Width = m_LabelWidth;
                    LabelControlList[i].setSize(aSize);
                }
                else
                {
                    m_LabelWidth = LabelControlList[i].getSize().Width;
                }
            }
            else
            {
                final String sFieldName = FieldColumns[i].getFieldName();
                LabelControlList[i] = new Control(oFormHandler, xFormName, FormHandler.SOLABEL, sFieldName, aPoint, aSize);
                if (bIsVeryFirstRun && icurArrangement == FormWizard.COLUMNAR_TOP)
                {
                    m_currentControlPosY = m_currentLabelPosY + m_LabelHeight;
                }
                final String sTitle = FieldColumns[i].getFieldTitle();
                m_LabelWidth = LabelControlList[i].getPreferredWidth(sTitle);
                aSize.Width = m_LabelWidth;
                LabelControlList[i].setSize(aSize);
            }
            Control curLabelControl = LabelControlList[i];
            if (icurArrangement == FormWizard.COLUMNAR_LEFT)
            {
                // Note This If Sequence must be called before retrieving the outer Points
                if (bIsFirstRun)
                {
                    m_MaxLabelWidth = m_LabelWidth;
                    bIsFirstRun = false;
                }
                else if (m_LabelWidth > m_MaxLabelWidth)
                {
                    m_MaxLabelWidth = m_LabelWidth;
                }
            }
            checkOuterPoints(m_currentLabelPosX, m_LabelWidth, m_currentLabelPosY, m_LabelHeight, false);
            if ((icurArrangement == FormWizard.COLUMNAR_TOP) || (icurArrangement == FormWizard.IN_BLOCK_TOP))
            {
                m_currentControlPosX = m_currentLabelPosX;
                m_currentControlPosY = m_currentLabelPosY + m_LabelHeight;
                curLabelControl.xPropertySet.setPropertyValue(PropertyNames.PROPERTY_ALIGN, new Short((short) com.sun.star.awt.TextAlign.LEFT));
            }
            else
            {
                curLabelControl.xPropertySet.setPropertyValue(PropertyNames.PROPERTY_ALIGN, new Short((short) _iAlign));
            }
            if (!bControlsareCreated)
            {
                curLabelControl.setSize(new Size(m_LabelWidth, m_LabelHeight));
            }
//      if (CurHelpText != ""){
//          oModel.HelpText = CurHelptext;
//      }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void insertDBControl(int i)
    {
        try
        {
            String sFieldName = FieldColumns[i].getFieldName();
            int nFieldType = FieldColumns[i].getFieldType();

            Point aPoint = new Point(m_currentControlPosX, m_currentControlPosY);
            if (bControlsareCreated)
            {
                DBControlList[i].setPosition(aPoint);
            }
            else
            {
                if (nFieldType == DataType.TIMESTAMP)
                {
                    DBControlList[i] = new TimeStampControl(new Resource(xMSF, "FormWizard", "dbw"), oFormHandler, xFormName, sFieldName, aPoint);
                }
                else
                {
                    DBControlList[i] = new DatabaseControl(oFormHandler, xFormName, sFieldName, nFieldType, aPoint);
                    if (DBControlList[i].getControlType() == FormHandler.SOCHECKBOX)
                    {
                        // Checkboxes have no Label near by
                        DBControlList[i].setPropertyValue(PropertyNames.PROPERTY_LABEL, "");
                    }
                }
            }
            DatabaseControl aDBControl = DBControlList[i];
            m_dbControlHeight = aDBControl.getControlHeight();
            m_dbControlWidth = aDBControl.getControlWidth();
            if (nFieldType != DataType.TIMESTAMP)
            {
                aDBControl.setSize(new Size(m_dbControlWidth, m_dbControlHeight));
            }
            if (aDBControl.getControlType() == FormHandler.SOCHECKBOX)
            {
                m_currentControlPosY = m_currentControlPosY + /*(int)*/ ((oFormHandler.getControlReferenceHeight() - m_dbControlHeight) / 2);
                aPoint = new Point(m_currentControlPosX, m_currentControlPosY);
                aDBControl.setPosition(aPoint);
            }
            if (nFieldType == DataType.LONGVARCHAR) /* memo */

            {
                Helper.setUnoPropertyValue(LabelControlList[i], PropertyNames.PROPERTY_MULTILINE, Boolean.TRUE);
            }
            checkOuterPoints(m_currentControlPosX, m_dbControlWidth, m_currentControlPosY, m_dbControlHeight, true);
            aDBControl.setPropertyValue(PropertyNames.PROPERTY_BORDER, NBorderType);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private int assignFieldLength(int _fieldlength)
    {
        if (_fieldlength >= 65535)
        {
            return -1;
        }
        else
        {
            return _fieldlength;
        }
    }

    public int getFormHeight()
    {
        return m_controlMaxPosY - cYOffset;
    }

    public int getEntryPointY()
    {
        if (icurArrangement == FormWizard.COLUMNAR_TOP)
        {
            Control curLabelControl2 = LabelControlList[0];
            return curLabelControl2.getPosition().Y;
        }
        else
        {
            DatabaseControl curDBControl2 = DBControlList[0];
            return curDBControl2.getPosition().Y;
        }
    }

    public void setStartPoint(Point _aPoint)
    {
        cXOffset = _aPoint.X;
        cYOffset = _aPoint.Y;
    }

    public void adjustYPositions(int _diffY)
    {
        for (int i = 0; i < DBControlList.length; i++)
        {
            Point aPoint = DBControlList[i].getPosition();
            DBControlList[i].setPosition(new Point(aPoint.X, aPoint.Y - _diffY));
            aPoint = LabelControlList[i].getPosition();
            LabelControlList[i].setPosition(new Point(aPoint.X, aPoint.Y - _diffY));
        }
        m_controlMaxPosY = -_diffY;
        cYOffset = -_diffY;
    }

    public void setFormSize(Size _FormSize)
    {
        nFormHeight = _FormSize.Height;
        nFormWidth = _FormSize.Width;
    }
}

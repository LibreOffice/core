/*
 ************************************************************************
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
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.XUnitConversion;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XFixedLine;
import com.sun.star.report.XFixedText;
import com.sun.star.report.XFormattedField;
import com.sun.star.report.XGroup;
import com.sun.star.report.XGroups;
import com.sun.star.report.XImageControl;
import com.sun.star.report.XReportComponent;
import com.sun.star.report.XReportControlModel;
import com.sun.star.report.XReportDefinition;
import com.sun.star.report.XSection;
import com.sun.star.drawing.XShape;
import com.sun.star.sdbc.DataType;
import com.sun.star.style.XStyle;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XNumberFormatTypes;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.PropertySetHelper;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportBuilderLayouter;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.UIConsts;
import java.util.HashMap;
import java.util.Locale;

/**
 * This class is abstract and more like a helper for create different layouts for Report Builder Wizard.
 * @author ll93751
 */// TODO: let a little bit place between 2 formatted fields
// TODO: move all sectionobjects which have a connect to the right max page width to left if there is a orientation change.
abstract public class ReportBuilderLayouter implements IReportBuilderLayouter
{

    private IReportDefinitionReadAccess m_xReportDefinitionReadAccess;
    private Resource m_aResource;

    /**
     * dispose the ReportBuilderLayouter
     */
    public void dispose()
    {
        closeDesignTemplate();
        m_xReportDefinitionReadAccess = null;
    // m_xReportBuilderLayouter = null;
    }

    /**
     * The Constructor is protected, this is a abstract class, use Tabular or other to create an instance.
     * @param _xDefinitionAccess
     * @param _aResource
     */
    protected ReportBuilderLayouter(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        m_xReportDefinitionReadAccess = _xDefinitionAccess;
        m_aResource = _aResource;
    }

    /**
     * Access to the current ReportDefinition
     * @return the ReportDefinition
     */
    public XReportDefinition getReportDefinition()
    {
        // we have to ask our parent for this value
        return m_xReportDefinitionReadAccess.getReportDefinition();
    }

    /**
     * Access to the global MultiServiceFactory
     * @return the global Service Factory
     */
    private XMultiServiceFactory getGlobalMSF()
    {
        return m_xReportDefinitionReadAccess.getGlobalMSF();
    }

    /**
     * Layout the complete report new, by all information we know until this time.
     *
     * If there are some information less, it is no problem for this function, they will be leaved out.
     * It is possible to call this function after every change, but be careful, could take a little bit longer.
     */
    public synchronized void layout()
    {
        try
        {
            // we bring the clear and insert methods together, this should be a little bit smoother
            clearReportHeader();
            insertReportHeader();

            clearReportFooter();
            insertReportFooter();

            clearPageHeader();
            insertPageHeader();

            clearPageFooter();
            insertPageFooter();

            clearGroups();
            insertGroups();

            clearDetails();
            // clearFieldTitles();
            insertDetailFieldTitles();
            insertDetailFields();
        }
        catch (java.lang.ArrayIndexOutOfBoundsException e)
        {
            // could happen, if you change the count of values
        }
        catch (java.lang.RuntimeException e)
        {
            throw e;
        }
        catch (Exception e)
        {
            // Exception not set. Not from interest here.
            int dummy = 0;
        }
    }
    // -------------------------------------------------------------------------
    /**
     * Remove all Groups
     */
    protected void clearGroups()
    {
        final XGroups xGroups = getReportDefinition().getGroups();
//        int nCount = xGroups.getCount();
        while (xGroups.hasElements())
        {
            try
            {
                xGroups.removeByIndex(0);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // Exception not set. Not from interest here.
            }
        }
    }
    // -------------------------------------------------------------------------
    /**
     * Remove all fields
     * @param _xSectionToClear
     */
    private void emptySection(XSection _xSectionToClear)
    {
        if (_xSectionToClear == null)
        {
            return;
        }
        //      int nCount = _xSectionToClear.getCount();
        while (_xSectionToClear.hasElements())
        {
            try
            {
                final Object aObj = _xSectionToClear.getByIndex(0);
                final XShape aShape = (XShape) UnoRuntime.queryInterface(XShape.class, aObj);
                _xSectionToClear.remove(aShape);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // Exception not set.
                int dummy = 0;
            }

        }
    }

    private void clearDetails()
    {
        final XSection xSection = getReportDefinition().getDetail();
        emptySection(xSection);
    }
//    public void clearFieldTitles()
//    {
//    }
    /**
     *
     * @param _nGroupCount
     * @return the left indent in 1/100mm
     */
    protected int getLeftGroupIndent(int _nGroupCount)
    {
// TODO: Fix values for group indent (1/2cm) only the first 2 groups
        int nIndent = 0;
        final int MAX_INDENT = 2;
        if (_nGroupCount <= MAX_INDENT)
        {
            nIndent = _nGroupCount * LayoutConstants.IndentFactorWidth;
        }
        else
        {
            nIndent = MAX_INDENT * LayoutConstants.IndentFactorWidth;
        }
        return nIndent;
    }
    /**
     * Get left page indent.
     * The left indent is the area on the left side which will no be printed.
     * The default is 2000 1/100mm what is 2cm of DIN A4.
     * @return the left indent in 1/100mm
     */
    int m_nLeftIndent = -1;

    protected int getLeftPageIndent()
    {
        if (m_nLeftIndent < 0)
        {
            m_nLeftIndent = getFromPageStyles("LeftMargin", 2000);
        }
        return m_nLeftIndent;
    }
    /**
     * Get right page indent.
     * The right indent is the area on the right side which will no be printed.
     * The default is 2000 1/100mm what is 2cm of DIN A4.
     * @return the right indent in 1/100mm
     */
    int m_nRightIndent = -1;

    protected int getRightPageIndent()
    {
        if (m_nRightIndent < 0)
        {
            m_nRightIndent = getFromPageStyles("RightMargin", 2000);
        }
        return m_nRightIndent;
    }

    private XStyle getUsedStyle(String _sStyleName)
    {
        XStyle aUsedStyle = null;
        final XNameAccess xNameAccess = getReportDefinition().getStyleFamilies();
        try
        {
            // <Analysecode>
            // String[] aNames = xNameAccess.getElementNames();
            // </Analysecode>

            // get all Page Styles
            final Object aPageStylesObj = xNameAccess.getByName(_sStyleName);
            final XNameAccess xContainer = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, aPageStylesObj);

            // step over all Page Styles, search the one which is in use
            final String[] aElementNames = xContainer.getElementNames();
            for (int i = 0; i < aElementNames.length; i++)
            {
                final String sName = aElementNames[i];
                final Object aObj = xContainer.getByName(sName);
                final XStyle xStyle = (XStyle) UnoRuntime.queryInterface(XStyle.class, aObj);
                if (xStyle.isInUse())
                {
                    aUsedStyle = xStyle;
                    break;
                }
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            // TODO: Exception not set.
            e.printStackTrace();
        }
        return aUsedStyle;
    }

    protected int getFromPageStyles(String _sStyleName, int _nDefault)
    {
        int nValue = _nDefault;
        final XStyle xStyle = getUsedStyle("PageStyles");
        if (xStyle != null)
        {
            // we found the page style which is in use
            final PropertySetHelper aHelper = new PropertySetHelper(xStyle);
            nValue = aHelper.getPropertyValueAsInteger(_sStyleName, nValue);
        }
        return nValue;
    }

    protected void setToPageStyles(String _sStyleName, Object _aObj)
    {
        final XStyle xStyle = getUsedStyle("PageStyles");
        if (xStyle != null)
        {
            final PropertySetHelper aHelper = new PropertySetHelper(xStyle);
            aHelper.setPropertyValueDontThrow(_sStyleName, _aObj);
        }
    }
    /**
     * Get page width. The default is 21000 1/100mm what is 21cm of DIN A4.
     * @return the Width of the page in 1/100mm
     */
    int m_nPageWidth = -1;

    protected int getPageWidth()
    {
        if (m_nPageWidth < 0)
        {
            m_nPageWidth = getFromPageStyles(PropertyNames.PROPERTY_WIDTH, 21000);
        }
        return m_nPageWidth;
    }
    // -------------------------------------------------------------------------
    /**
     * Stores the Group names. To insert/create a report with such group names, call layout()
     * @param _aGroupNames
     */
    public void insertGroupNames(String[] _aGroupNames)
    {
        m_aGroupNames = _aGroupNames;
    }

    protected void copyGroupProperties(int _nGroup)
    {
        if (getDesignTemplate() != null)
        {
            try
            {
                final XGroups xForeignGroups = getDesignTemplate().getGroups();
                int nGroups = xForeignGroups.getCount();
                Object aForeignGroup = xForeignGroups.getByIndex(_nGroup);
                XGroup xForeignGroup = (XGroup) UnoRuntime.queryInterface(XGroup.class, aForeignGroup);
                XSection xForeignGroupSection = xForeignGroup.getHeader();

                if (xForeignGroupSection != null)
                {
                    final XGroups xGroups = getReportDefinition().getGroups();
                    Object aGroup = xGroups.getByIndex(_nGroup);
                    XGroup xGroup = (XGroup) UnoRuntime.queryInterface(XGroup.class, aGroup);
                    XSection xGroupSection = xGroup.getHeader();

                    // copy Properties
                    copyProperties(xForeignGroupSection, xGroupSection);
                }
            }
            catch (Exception e)
            {
                int dummy = 0;
            }
        }
    }
    // -------------------------------------------------------------------------
    protected void insertGroups()
    {
        if (m_aGroupNames == null)
        {
            return;
        }

        final XGroups xGroups = getReportDefinition().getGroups();
        final int nLeftPageIndent = getLeftPageIndent();
        final int nLabelWidth = getMaxLabelWidth(); // 3000;
        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());
        final int nFieldWidth = nUsablePageWidth - nLabelWidth;

        // after done with all groups, we need access to the last group, for set property 'KeepTogether' so we remember it.
        XGroup aLastGroup = null;

        for (int i = 0; i < m_aGroupNames.length; i++)
        {
            final XGroup xGroup = xGroups.createGroup();
            aLastGroup = xGroup;

            // TODO: debug what the m_aGroupNames are, "field:[...]" or the "fieldname"
            xGroup.setExpression(m_aGroupNames[i]);
            xGroup.setHeaderOn(true);
            // we don't want any group footer (default off)
            // xGroup.setFooterOn(true);

            try
            {
                // we need to append by hand
                // TODO: documentation is unclear here, that we have to insert by hand
                int nCount = xGroups.getCount();
                xGroups.insertByIndex(nCount, xGroup);

                final XSection xGroupSection = xGroup.getHeader();
                copyGroupProperties(nCount);

                Rectangle aRect = new Rectangle();
                aRect.X = nLeftPageIndent + getLeftGroupIndent(i);
                SectionObject aSO = getDesignTemplate().getGroupLabel(i);
                int nLabelHeight = 0;
//               if (aSO != null)
//               {
                nLabelHeight = aSO.getHeight(LayoutConstants.LabelHeight);
                aRect = insertLabel(xGroupSection, getTitleFromFieldName(m_aGroupNames[i]), aRect, nLabelWidth, aSO);
//               }
//               else
//               {
//                   nLabelHeight = 500;
//                   aRect = insertBoldLabel(xGroupSection, getTitleFromFieldName(m_aGroupNames[i]), aRect, nLabelWidth);
//               }
                final String sGroupName = convertToFieldName(m_aGroupNames[i]);
                aSO = getDesignTemplate().getGroupTextField(i);
                insertFormattedField(xGroupSection, sGroupName, aRect, nFieldWidth, aSO);

                // draw a line under the label/formattedfield
                aRect.X = nLeftPageIndent + getLeftGroupIndent(i);
                aRect.Y = nLabelHeight;
                final int nLineWidth = getPageWidth() - getRightPageIndent() - aRect.X;
                final int nLineHeight = LayoutConstants.LineHeight;
                insertHorizontalLine(xGroupSection, aRect, nLineWidth, nLineHeight);
                xGroupSection.setHeight(nLabelHeight + nLineHeight);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
            }
        }

        // hold the inner group together
        if (aLastGroup != null)
        {
            doNotBreakInTable(aLastGroup);
        }
    }

    // -------------------------------------------------------------------------
    /**
     * Give a list off all field title names to insert the field title names, call layout()
     * @param _aFieldTitleNames
     */
    public void insertFieldTitles(String[] _aFieldTitleNames)
    {
        m_aFieldTitleNames = _aFieldTitleNames;
    }

    // -------------------------------------------------------------------------
    protected String getTitleFromFieldName(String _sField)
    {
        for (int i = 0; i < m_aFieldNames.length; i++)
        {
            if (m_aFieldNames[i].equals(_sField))
            {
                return m_aFieldTitleNames[i];
            }
        }
        return "";
    }

        protected int getTypeFromFieldName(String _sField)
    {
        for (int i = 0; i < m_aFieldNames.length; i++)
        {
            if (m_aFieldNames[i].equals(_sField))
            {
                return m_aFieldTypes[i];
            }
        }
        return 0;
    }

    protected boolean listContains(String[] _aList, String _aValue)
    {
        for (int i = 0; i < _aList.length; i++)
        {
            if (_aList[i].equals(_aValue))
            {
                return true;
            }
        }
        return false;
    }
    // -------------------------------------------------------------------------
    /**
     * Helper to get all field names without the names which are already in the group names
     * @param _aList
     * @param _aGetResultFrom
     * @return
     */
    protected String[] getNamesWithoutGroupNames(String[] _aList, String[] _aGetResultFrom)
    {
        if (_aList == null /* || _aGetResultsFrom == null */)
        {
            return new String[]
                    {
                    }; /* empty list */
        }
        if (getCountOfGroups() == 0)
        {
            if (_aGetResultFrom != null)
            {
                return _aGetResultFrom;
            }
            return _aList;
        }
        final int nNewLength = _aList.length - getCountOfGroups();
        String[] aNewList = new String[nNewLength];
        int j = 0;
        for (int i = 0; i < _aList.length; i++)
        {
            final String sField = _aList[i];
            if (listContains(m_aGroupNames, sField) == true)
            {
                continue;
            }
            if (_aGetResultFrom != null)
            {
                aNewList[j++] = _aGetResultFrom[i];
            }
            else
            {
                aNewList[j++] = sField;
            }
            if (j == nNewLength)
            {
                break; // Emergency break, we leave the result list.
            }
        }
        return aNewList;
    }

    // -------------------------------------------------------------------------
    protected int calculateFieldWidth(int _nLeftIndent, int _nFieldCount)
    {
        int nWidth = 3000;
        if (_nFieldCount > 0)
        {
            nWidth = (getPageWidth() - getLeftPageIndent() - getRightPageIndent() - _nLeftIndent) / _nFieldCount;
        }
        return nWidth;
    }

    protected String[] getFieldTitleNames()
    {
        return getNamesWithoutGroupNames(m_aFieldNames, m_aFieldTitleNames);
    }
    // -------------------------------------------------------------------------
    abstract protected void insertDetailFieldTitles();
    // -------------------------------------------------------------------------
    /**
     * Give a list off all field names to insert the field names, call layout()
     * @param _aFieldNames
     */
    public void insertFieldNames(String[] _aFieldNames)
    {
        m_aFieldNames = _aFieldNames;
    }

    public void insertFieldTypes(int[] _aFieldTypes)
    {
        m_aFieldTypes = _aFieldTypes;
    }

    public void insertFieldWidths(int[] _aFieldWidths)
    {
        m_aFieldWidths = _aFieldWidths;
    }

    protected int getCountOfGroups()
    {
        return ((m_aGroupNames == null) ? 0 : m_aGroupNames.length);
    }

    // -------------------------------------------------------------------------
    protected String[] getFieldNames()
    {
        return getNamesWithoutGroupNames(m_aFieldNames, null);
    }

    abstract protected void insertDetailFields();

    protected void copyDetailProperties()
    {
        if (getDesignTemplate() != null)
        {
            try
            {
                XSection xForeignSection = getDesignTemplate().getDetail();
                if (xForeignSection != null)
                {
                    XSection xSection = getReportDefinition().getDetail();

                    // copy Properties
                    copyProperties(xForeignSection, xSection);
                }
            }
            catch (Exception e)
            {
            }
        }
    }
    // -------------------------------------------------------------------------
    protected Rectangle insertLabel(XSection _xSection, String _sLabel, Rectangle _aRect, int _nWidth, SectionObject _aSO)
    {
        if (_xSection != null)
        {
            try
            {
                final Object aFixedText = getMSFofReportDefinition().createInstance("com.sun.star.report.FixedText");
                final XFixedText xFixedText = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, aFixedText);


                int nHeight = LayoutConstants.LabelHeight;        // default height of label is fixed.
                if (_aSO != null)
                {
                    if (_aSO instanceof SectionEmptyObject)
                    {
                        float fCharWeight = _aSO.getCharWeight(com.sun.star.awt.FontWeight.NORMAL);
                        if (fCharWeight > 0.1f)
                        {
                            xFixedText.setCharWeight(fCharWeight);
                        }
                    }
                    else
                    {
// TODO: there seems to be some problems with copy all properties from the design template to the current design
                        final FontDescriptor aFD = _aSO.getFontDescriptor();
                        if (aFD != null)
                        {
                            xFixedText.setFontDescriptor(aFD);
                            copyProperties(_aSO.getParent(), xFixedText);
                        }
                        nHeight = _aSO.getHeight(LayoutConstants.LabelHeight);
                    }
                }
                xFixedText.setLabel(_sLabel);

                xFixedText.setPositionX(_aRect.X);
                xFixedText.setPositionY(_aRect.Y);

                // Width will calculate from outside.
                // We have to set, because there exist no right default (0)
                xFixedText.setWidth(_nWidth);
                _aRect.X += _nWidth;
                xFixedText.setHeight(nHeight);
                _xSection.add(xFixedText);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
                int dummy = 0;
            }
        }
        return _aRect;
    }
    // -------------------------------------------------------------------------
    protected String convertToFieldName(String _sElementName)
    {
        final StringBuffer aDataField = new StringBuffer(32);
        aDataField.append("field:[").append(_sElementName).append(']');
        return aDataField.toString();

    }
    protected String convertFromFieldName(String _sName)
    {
        if (_sName.startsWith("field:["))
        {
            int nCloseBrace = _sName.lastIndexOf("]");
            final String sName = _sName.substring(7, nCloseBrace).trim();
            return sName;
        }
        return _sName;
    }
    // -------------------------------------------------------------------------
    /**
     * Insert a already formatted field name into a given section
     *
     * Use 'convertToFieldName(dbfield)' to convert a dbfield name in the right.
     *
     * @param _xSection        in which section the formatted field will store
     * @param _sFormattedfield as String a dbfield or an other function
     * @param _aRect           .X, .Y are the absolute position (1/100mm) where the formatted field will set
     * @param _nWidth          the width of the field in 1/100mm
     * @param _aSO
     * @return a new Rectangle with the new Rect.X position, Rect.Y will not change.
     */
    protected Rectangle insertFormattedField(XSection _xSection, String _sFormattedfield, Rectangle _aRect, int _nWidth, SectionObject _aSO)
    {
        return insertFormattedField(_xSection, _sFormattedfield, _aRect, _nWidth, _aSO, (short) com.sun.star.awt.TextAlign.LEFT);
    }

    protected Rectangle insertFormattedField(XSection _xSection, String _sFormattedfield, Rectangle _aRect, int _nWidth, SectionObject _aSO, short _nAlignment)
    {
        if (_xSection != null)
        {
            try
            {
                Object aField;
                int nHeight = LayoutConstants.FormattedFieldHeight;

                int nType = getTypeFromFieldName(convertFromFieldName(_sFormattedfield));
                if (nType == DataType.BINARY ||
                    nType == DataType.VARBINARY ||
                    nType == DataType.LONGVARBINARY)
                {
                    aField = getMSFofReportDefinition().createInstance("com.sun.star.report.ImageControl");
                    nHeight = LayoutConstants.BinaryHeight;
                }
                else
                {
                    aField = getMSFofReportDefinition().createInstance("com.sun.star.report.FormattedField");
                    nHeight = LayoutConstants.FormattedFieldHeight;
                    if (nType == DataType.LONGVARCHAR) /* memo */
                    {
                        nHeight = LayoutConstants.MemoFieldHeight; // special case for memo
                    }
                }
                _aRect.Height = nHeight;

                final XReportControlModel xReportControlModel = (XReportControlModel) UnoRuntime.queryInterface(XReportControlModel.class, aField);
                if (xReportControlModel != null)
                {
                    // #i86907# not documented right in idl description.
                    xReportControlModel.setDataField(_sFormattedfield);
                    if (_aSO != null)
                    {
    // TODO: there seems to be some problems with copy all properties from the design template to the current design
                        final FontDescriptor aFD = _aSO.getFontDescriptor();
                        if (aFD != null)
                        {
                            xReportControlModel.setFontDescriptor(aFD);
                            copyProperties(_aSO.getParent(), xReportControlModel);
                        }
                        nHeight = _aSO.getHeight(nHeight);
                    }
                    xReportControlModel.setPositionX(_aRect.X);
                    xReportControlModel.setPositionY(_aRect.Y);
                    xReportControlModel.setWidth(_nWidth);
                    _aRect.X += _nWidth;
                    xReportControlModel.setHeight(nHeight);

                    if (nType == DataType.BINARY ||
                        nType == DataType.VARBINARY ||
                        nType == DataType.LONGVARBINARY)
                    {
                        // aField = getMSFofReportDefinition().createInstance("com.sun.star.report.ImageControl");
                        final XImageControl xImageControl = (XImageControl) UnoRuntime.queryInterface(XImageControl.class, xReportControlModel);
                        if (xImageControl != null)
                        {
                            // xImageControl.setScaleImage(true);

                            xImageControl.setScaleMode(com.sun.star.awt.ImageScaleMode.Isotropic);
                        }
                    }
                    else
                    {
                        try
                        {
                            xReportControlModel.setParaAdjust(_nAlignment);
                            // if (nType == DataType.LONGVARCHAR)
                            // {
                            //     xReportControlModel.???
                            // }
                        }
                        catch (com.sun.star.beans.UnknownPropertyException e)
                        {
                            // seems we not able to set ParaAdjust
                        }
                    }
                    // spezial case rpt:now() (default date format)
                    if (_sFormattedfield.equals("rpt:now()"))
                    {
                        final XFormattedField xFormattedField = (XFormattedField) UnoRuntime.queryInterface(XFormattedField.class, xReportControlModel);

                        XNumberFormatsSupplier x = xFormattedField.getFormatsSupplier();
                        XNumberFormats xFormats = x.getNumberFormats();
                        XNumberFormatTypes x3 = (XNumberFormatTypes) UnoRuntime.queryInterface(XNumberFormatTypes.class, xFormats);
                        Locale.getDefault();
                        com.sun.star.lang.Locale aLocale = new com.sun.star.lang.Locale();
                        aLocale.Country = Locale.getDefault().getCountry();
                        aLocale.Language = Locale.getDefault().getLanguage();

                        int nFormat = x3.getStandardFormat(com.sun.star.util.NumberFormat.DATE, aLocale);
                        xFormattedField.setFormatKey(nFormat);
                    }
                    _xSection.add(xReportControlModel);
                }
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
                int dummy = 0;
            }
        }
        return _aRect;
    }

    // -------------------------------------------------------------------------
// TODO: check with Query, this code will not work with Queries
    public void setTableName(int _aType, String _sTableName)
    {
        m_aCommandType = _aType;
        m_sTableName = _sTableName;

        // getReportDefinition().setCommandType(com.sun.star.sdb.CommandType.TABLE);
        getReportDefinition().setCommandType(_aType);
        getReportDefinition().setCommand(_sTableName);
    }    // -------------------------------------------------------------------------
    protected XMultiServiceFactory m_xMSF;

    protected XMultiServiceFactory getMSFofReportDefinition()
    {
        if (m_xMSF == null)
        {
            final XMultiServiceFactory xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, getReportDefinition());
            m_xMSF = xMSF;
        }
        return m_xMSF;
    }
    // -------------------------------------------------------------------------
    protected Rectangle insertVerticalLine(XSection _xSection, Rectangle _aRect, int _nWidth, int _nHeight)
    {
        return insertLine(_xSection, _aRect, _nWidth, _nHeight, 1);
    }

    protected Rectangle insertHorizontalLine(XSection _xSection, Rectangle _aRect, int _nWidth, int _nHeight)
    {
        return insertLine(_xSection, _aRect, _nWidth, _nHeight, 0);
    }

    protected Rectangle insertLine(XSection _xSection, Rectangle _aRect, int _nWidth, int _nHeight, int _nOrientation)
    {
        if (_xSection != null)
        {
            try
            {
                final Object aFixedLine = getMSFofReportDefinition().createInstance("com.sun.star.report.FixedLine");
                final XFixedLine xFixedLine = (XFixedLine) UnoRuntime.queryInterface(XFixedLine.class, aFixedLine);

                xFixedLine.setOrientation(_nOrientation);
                // TODO: line width is fixed
                xFixedLine.setLineWidth(8);

                xFixedLine.setPositionX(_aRect.X);
                xFixedLine.setPositionY(_aRect.Y);

                xFixedLine.setWidth(_nWidth);
                _aRect.X += _nWidth;
                xFixedLine.setHeight(_nHeight);
                _xSection.add(xFixedLine);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
                int dummy = 0;
            }
        }
        return _aRect;
    }
    // -------------------------------------------------------------------------
    protected void clearReportHeader()
    {
        XSection xSection;
        try
        {
            if (getReportDefinition().getReportHeaderOn())
            {
                xSection = getReportDefinition().getReportHeader();
                emptySection(xSection);
            }
        }
        catch (com.sun.star.container.NoSuchElementException e)
        {
            // Exception not set. Not really from interest.
        }
    }

    protected void insertReportHeader()
    {
        if (getDesignTemplate() != null)
        {
            if (getDesignTemplate().getReportHeaderOn())
            {
                // copy all Section information from Page Header to our Header
                try
                {
                    XSection xForeignSection = getDesignTemplate().getReportHeader();

                    if (xForeignSection != null)
                    {
                        getReportDefinition().setReportHeaderOn(true);
                        XSection xSection = getReportDefinition().getReportHeader();

                        // copy Sections
                        copySection(xForeignSection, xSection);
                    }
                }
                catch (Exception e)
                {
                }
            }
            else
            {
                // we won't a page header
                getReportDefinition().setReportHeaderOn(false);
            }
        }
    }

    protected void clearReportFooter()
    {
        XSection xSection;
        try
        {
            if (getReportDefinition().getReportFooterOn())
            {
                xSection = getReportDefinition().getReportFooter();
                emptySection(xSection);
            }
        }
        catch (com.sun.star.container.NoSuchElementException e)
        {
            // Exception not set. Not really from interest.
        }
    }

    protected void insertReportFooter()
    {
        if (getDesignTemplate() != null)
        {
            if (getDesignTemplate().getReportFooterOn())
            {
                // copy all Section information from Page Header to our Header
                try
                {
                    XSection xForeignSection = getDesignTemplate().getReportFooter();

                    if (xForeignSection != null)
                    {
                        getReportDefinition().setReportFooterOn(true);
                        XSection xSection = getReportDefinition().getReportFooter();

                        // copy Sections
                        copySection(xForeignSection, xSection);
                    }
                }
                catch (Exception e)
                {
                }
            }
            else
            {
                // we won't a page header
                getReportDefinition().setReportFooterOn(false);
            }
        }
    }
    // -------------------------------------------------------------------------
    protected void clearPageHeader()
    {
        XSection xSection;
        try
        {
            if (getReportDefinition().getPageHeaderOn())
            {
                xSection = getReportDefinition().getPageHeader();
                emptySection(xSection);
            }
        }
        catch (com.sun.star.container.NoSuchElementException e)
        {
            // Exception not set. Not really from interest.
        }
    }

//    protected static String getDateString(String _sFormat)
//        {
//            GregorianCalendar aCalendar = new GregorianCalendar();
//            StringBuffer aBuf = new StringBuffer();
//
//            Locale aLocale = new Locale("en","US");
//            SimpleDateFormat aFormat = new SimpleDateFormat(_sFormat, aLocale);
//            aBuf = aFormat.format(aCalendar.getTime(), aBuf, new FieldPosition(0) );
//            // DebugHelper.writeInfo("Date: " + aBuf.toString());
//            return aBuf.toString();
//    }

//    protected String getCurrentDate()
//    {
//        Calendar aCalendar = Calendar.getInstance();
////        aCalendar.setTimeInMillis(1202382900000L);
////        aCalendar.
//        Date aDate = new Date();
//        String sDate = DateFormat.getDateInstance().format(aDate);
//        // String aCalStr = aCalendar.toString();
//        return sDate;
//        //
////        Date aDate = new Date();
////        aDate.setSeconds(0);
////        aDate.setMinutes(15);
////        aDate.setHours(12);
////        // aDate.setMonth(2);
////        // aDate.setYear(2008);
////        // aDate.setDay(7);
////        long nTime = aDate.getTime();
////        Long aLong = new Long(nTime);
////        String aStr = aLong.toString();
////
////        Date aNewDate = new Date(1202382900000L);
//////         aNewDate.
////        String aDateStr = aNewDate.toString();
//////         Datetime aNewTime = new Time(1202382900);
//////         String aTimeStr = aNewTime.toString();
////
//
//    }
    protected void clearPageFooter()
    {
        XSection xSection;
        try
        {
            if (getReportDefinition().getPageFooterOn())
            {
                xSection = getReportDefinition().getPageFooter();
                emptySection(xSection);
            }
        }
        catch (com.sun.star.container.NoSuchElementException e)
        {
            // Exception not set. Should not from interest here.
        }
    }

    public void setPageOrientation(int _nOrientation)
    {
        final int nWidth = getFromPageStyles(PropertyNames.PROPERTY_WIDTH, 0);
        final int nHeight = getFromPageStyles(PropertyNames.PROPERTY_HEIGHT, 0);

        if (com.sun.star.wizards.report.ReportLayouter.SOOPTLANDSCAPE == _nOrientation)
        {
            setToPageStyles("IsLandscape", Boolean.TRUE);
            if (nWidth < nHeight)
            {
                setToPageStyles(PropertyNames.PROPERTY_WIDTH, new Integer(nHeight));
                setToPageStyles(PropertyNames.PROPERTY_HEIGHT, new Integer(nWidth));
            }
        }
        else
        {
            setToPageStyles("IsLandscape", Boolean.FALSE);
            if (nHeight < nWidth)
            {
                setToPageStyles(PropertyNames.PROPERTY_WIDTH, new Integer(nHeight));
                setToPageStyles(PropertyNames.PROPERTY_HEIGHT, new Integer(nWidth));
            }
        }
        // dirty the PageWidth
        m_nPageWidth = -1;
    }

    /**
     * Returns the width and height of a given string (_sText) in 1/100mm drawn in the given font descriptor.
     * TODO: This function is a performance leak, we could store already calculated values in a map, to build a cache. Access should be much faster then.
     *
     * @param _sText
     * @param _aFont
     * @return width of given text in 1/100mm
     */
    Size getPreferredSize(String _sText, FontDescriptor _aFont)
    {
        Size aSizeMM_100TH = new Size(0, 0);
        try
        {
            // Object aControlContainer = getGlobalMSF().createInstance("com.sun.star.awt.UnoControlContainer");
            // XControlContainer xControlContainer = (XControlContainer)UnoRuntime.queryInterface(XControlContainer.class, aControlContainer);

            final Object aFixedTextModel = getGlobalMSF().createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            final XControlModel xFixedTextModel = (XControlModel) UnoRuntime.queryInterface(XControlModel.class, aFixedTextModel);

            final PropertySetHelper aPropertySetHelper = new PropertySetHelper(xFixedTextModel);
//          aPropertySetHelper.showProperties();
            aPropertySetHelper.setPropertyValueDontThrow("FontDescriptor", _aFont);

            final Object aUnoCtrlFixedText = getGlobalMSF().createInstance("com.sun.star.awt.UnoControlFixedText");
//            XServiceInfo xServiceInfo2 = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, aUnoCtrlFixedText);
//            String[] sServices2 = xServiceInfo2.getSupportedServiceNames();

            final XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, aUnoCtrlFixedText);
            xWindow.setVisible(false);

            final XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, aUnoCtrlFixedText);
            xControl.setModel(xFixedTextModel);

            final com.sun.star.awt.XFixedText xFixedText = (com.sun.star.awt.XFixedText) UnoRuntime.queryInterface(com.sun.star.awt.XFixedText.class, aUnoCtrlFixedText);
            xFixedText.setText(_sText);

            final XLayoutConstrains xLayoutConstraints = (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, aUnoCtrlFixedText);
            final Size aSizeInPixel = xLayoutConstraints.getPreferredSize();

            final XWindowPeer xPeerOfReportDefinition = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, getReportDefinition().getCurrentController().getFrame().getComponentWindow());
            xControl.createPeer(null, xPeerOfReportDefinition);

            final XWindowPeer x = xControl.getPeer();

            final XUnitConversion xConversion = (XUnitConversion) UnoRuntime.queryInterface(XUnitConversion.class, x);
            aSizeMM_100TH = xConversion.convertSizeToLogic(aSizeInPixel, com.sun.star.util.MeasureUnit.MM_100TH);
            // xToolkit.createScreenCompatibleDevice(_nWidth, _nWidth).
            // XWindow x = getReportDefinition().getCurrentController().getFrame().getContainerWindow();
            // Object aObj = _xSection.getParent();

            // we don't need the created objects any longer
            final XComponent xFixedTextDeleter = (XComponent) UnoRuntime.queryInterface(XComponent.class, xFixedText);
            xFixedTextDeleter.dispose();

            final XComponent xFixedTextModelDeleter = (XComponent) UnoRuntime.queryInterface(XComponent.class, aFixedTextModel);
            xFixedTextModelDeleter.dispose();
        }
        catch (Exception e)
        {
            // TODO: Exception not set.
        }
        return aSizeMM_100TH;
    }

    protected String getTableName()
    {
        if (m_sTableName != null)
        {
            return m_sTableName;
        }
        return "";
    }

    protected String getUserNameFromConfiguration()
    {
        String sFirstName = "";
        String sLastName = "";
        try
        {
            Object oProdNameAccess = Configuration.getConfigurationRoot(getGlobalMSF(), "org.openoffice.UserProfile/Data", false);
            sFirstName = (String) Helper.getUnoObjectbyName(oProdNameAccess, "givenname");
            sLastName = (String) Helper.getUnoObjectbyName(oProdNameAccess, "sn");
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        return sFirstName + " " + sLastName;
    }

    /**
     * Helper function, to copy all not read only properties of _xFromSection to _xToSection
     * @param _xFromSection
     * @param _xToSection
     */
    private void copyProperties(Object _aFrom, Object _aTo)
    {
        XPropertySet xFrom = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _aFrom);
        XPropertySet xTo = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, _aTo);


        XPropertySetInfo xForeignPropInfo = xFrom.getPropertySetInfo();
        XPropertySetInfo xSectionPropInfo = xTo.getPropertySetInfo();
        Property[] aAllProperties = xForeignPropInfo.getProperties();
        for (int i = 0; i < aAllProperties.length; i++)
        {
            String sPropertyName = aAllProperties[i].Name;
            if (xSectionPropInfo.hasPropertyByName(sPropertyName))
            {
                try
                {
                    Property aDestProp = xForeignPropInfo.getPropertyByName(sPropertyName);
                    if ((aDestProp.Attributes & PropertyAttribute.READONLY) == 0)
                    {
                        xTo.setPropertyValue(sPropertyName, xFrom.getPropertyValue(sPropertyName));
                    }
                }
                catch (Exception e)
                {
                }
            }
        }
    }

    /**
     * Helper Funktion to copy the whole content of _xFromSection to the _xToSection
     * @param _xFromSection
     * @param _xToSection
     */
    private void copySection(XSection _xFromSection, XSection _xToSection)
    {
        copyProperties(_xFromSection, _xToSection);

        try
        {
            XEnumeration xEnum = _xFromSection.createEnumeration();
            while (xEnum.hasMoreElements())
            {
                Object aEnumObj = xEnum.nextElement();
                XReportComponent aComponent = (XReportComponent) UnoRuntime.queryInterface(XReportComponent.class, aEnumObj);

                // XCloneable aClone = (XCloneable)UnoRuntime.queryInterface(XCloneable.class, aEnumObj);
                if (aComponent != null)
                {
                    Object aClone = aComponent.createClone();
                    if (aClone != null)
                    {
                        String sName = aComponent.getName();
                        com.sun.star.awt.Point aPosition = aComponent.getPosition();
                        Size aSize = aComponent.getSize();

                        XShape aShape = (XShape) UnoRuntime.queryInterface(XShape.class, aClone);
                        String sShapeType = aShape.getShapeType();
                        com.sun.star.awt.Point aShapePoint = aShape.getPosition();
                        Size aShapeSize = aShape.getSize();

                        // normally 'createClone' will create a real clone of the component,
                        // but there seems some problems, we have to controll.
                        copyProperties(aComponent, aClone);

                        // aShape.setPosition(aComponent.getPosition());
                        // aShape.setSize(aComponent.getSize());
                        _xToSection.add(aShape);
                    }
                }
            }
        }
        catch (Exception e)
        {
        }
    // String sName = xForeignSection.getName();
    // int dummy = 0;
    }

    protected void insertPageHeader()
    {
        if (getDesignTemplate() != null)
        {
            if (getDesignTemplate().getPageHeaderOn())
            {
                // copy all Section information from Page Header to our Header
                try
                {
                    XSection xForeignSection = getDesignTemplate().getPageHeader();

                    if (xForeignSection != null)
                    {
                        getReportDefinition().setPageHeaderOn(true);
                        XSection xSection = getReportDefinition().getPageHeader();

                        // copy Sections
                        copySection(xForeignSection, xSection);
                    }
                }
                catch (Exception e)
                {
                }
                return;
            }
            else
            {
                // we won't a page header
                // getReportDefinition().setPageHeaderOn(true);
                getReportDefinition().setPageHeaderOn(false);
            }
        }
        else
        {
            if (getReportDefinition() == null)
            {
                return;
            }
            // there is no foreign report definition
            // TODO: #i86902# rpt:Title() out of the document

            // TODO: #i86902# rpt:Author() can't set with something like rpt:author()
            // TODO: #i86902# more fieldnames need.
            final String sTitleTitle = getResource().getResText(UIConsts.RID_REPORT + 86); // "Title:"
            final String sTitle = getTableName(); // "Default title, this is a first draft report generated by the new report wizard.";
            final String sAuthorTitle = getResource().getResText(UIConsts.RID_REPORT + 87); // "Author:"
            final String sAuthor = getUserNameFromConfiguration(); // "You";     // rpt:fieldvalue();
            final String sDateTitle = getResource().getResText(UIConsts.RID_REPORT + 88); // "Date:"
            // TODO: #i86911# Date: we need to set the style of the date.
            final String sDate = "rpt:now()"; // getDateString("EEE, d MMM yyyy HH:mm:ss zzzz");

            try
            {
                getReportDefinition().setPageHeaderOn(true);
                XSection xSection = null;
                xSection = getReportDefinition().getPageHeader();

                Rectangle aRect = new Rectangle();
                aRect.X = getLeftPageIndent();
                SectionObject aSOLabel = SectionEmptyObject.create();
                aSOLabel.setFontToBold();
                aRect.Y = aSOLabel.getHeight(LayoutConstants.LabelHeight);

                final int nWidth = 3000;

                aRect = insertLabel(xSection, sTitleTitle, aRect, nWidth, aSOLabel);

                final int nTitleWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - 3000;
                // aRect = insertFormattedField(xSection, "rpt:Title()", aRect, nTitleWidth);
                aRect = insertLabel(xSection, sTitle, aRect, nTitleWidth, aSOLabel);

                aRect.Y += aSOLabel.getHeight(LayoutConstants.LabelHeight) + LayoutConstants.LineHeight;

                aRect.X = getLeftPageIndent();
                aRect = insertLabel(xSection, sAuthorTitle, aRect, nWidth, aSOLabel);
                // aRect = insertFormattedField(xSection, "rpt:Author()", aRect, nWidth);
                aRect = insertLabel(xSection, sAuthor, aRect, nTitleWidth, aSOLabel);

                aRect.Y += aSOLabel.getHeight(LayoutConstants.LabelHeight);

                aRect.X = getLeftPageIndent();
                aRect = insertLabel(xSection, sDateTitle, aRect, nWidth, aSOLabel);
                // aRect = insertFormattedField(xSection, "rpt:Date()", aRect, nWidth);
                aRect = insertFormattedField(xSection, sDate, aRect, nTitleWidth, aSOLabel);

                aRect.Y += aSOLabel.getHeight(LayoutConstants.FormattedFieldHeight) + LayoutConstants.LineHeight;

                // draw a line under the label/formattedfield
                aRect.X = getLeftPageIndent();
                final int nLineWidth = getPageWidth() - getRightPageIndent() - aRect.X;
                final int nLineHeight = LayoutConstants.LineHeight;
                insertHorizontalLine(xSection, aRect, nLineWidth, nLineHeight);

                aRect.Y += nLineHeight;

                xSection.setHeight(aRect.Y);
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
            }
        }
    }

    protected void insertPageFooter()
    {
        if (getDesignTemplate() != null)
        {
            if (getDesignTemplate().getPageFooterOn())
            {
                try
                {
                    XSection xForeignSection = getDesignTemplate().getPageFooter();

                    if (xForeignSection != null)
                    {
                        getReportDefinition().setPageFooterOn(true);
                        XSection xSection = getReportDefinition().getPageFooter();

                        // copy Sections
                        copySection(xForeignSection, xSection);
                    }
                }
                catch (Exception e)
                {
                }
            }
            else
            {
                // getReportDefinition().setPageFooterOn(true);
                getReportDefinition().setPageFooterOn(false);
            }
        }
        else
        {
            if (getReportDefinition() == null)
            {
                return;
            }

            // TODO: how should we arrive this code (set page and pagecount in the middle of the page footer)
            // If there exists a design template, don't use it.

            // we don't have a default report definition
            final String sPageOf = getResource().getResText(UIConsts.RID_REPORT + 89); // 'Page #page# of #count#'

            // Convert
            // 'Page #page# of #count#'
            // to something like
            // '\"Page \" & PageNumber() & \" of \" & PageCount()'
            // due to the fact that is is not fixed, where #page# or #count# occurs, we make it
            // a little bit trickier.
            // we first surround the string with double quotes,
            // second, replace the #...#
            // last, we remove double 'double quotes'.
            final String sSurroundDoubleQuotes = "\"" + sPageOf + "\"";
            final String sPageNumber = sSurroundDoubleQuotes.replaceAll("#page#", "\" & PageNumber() & \"");
            final String sPageCount = sPageNumber.replaceAll("#count#", "\" & PageCount() & \"");
            final String sNoLastUnusedQuotes = sPageCount.replaceAll(" & \\\"\\\"", "");
            final String sNoFirstUnusedQuotes = sNoLastUnusedQuotes.replaceAll("\\\"\\\" & ", "");

            final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent();

            try
            {
                getReportDefinition().setPageFooterOn(true);
                XSection xSection = null;
                xSection = getReportDefinition().getPageFooter();

                Rectangle aRect = new Rectangle();
                aRect.X = getLeftPageIndent();

                // draw a line over the label/formattedfield
                final int nLineWidth = getPageWidth() - getRightPageIndent() - aRect.X;
                final int nLineHeight = LayoutConstants.LineHeight;
                insertHorizontalLine(xSection, aRect, nLineWidth, nLineHeight);

                aRect.Y += nLineHeight;
                aRect.Y += LayoutConstants.LabelHeight;

                final int nWidth = nUsablePageWidth;
                aRect.X = getLeftPageIndent();

                aRect = insertFormattedField(xSection, "rpt:" + sNoFirstUnusedQuotes, aRect, nWidth, null, (short) com.sun.star.awt.TextAlign.CENTER);

                aRect.Y += LayoutConstants.FormattedFieldHeight + LayoutConstants.LineHeight;
                xSection.setHeight(aRect.Y);
            }
            catch (Exception e)
            {
                // TODO: Exception not set.
            }
        }
    }

    protected Resource getResource()
    {
        return m_aResource;
    }
    protected int m_aCommandType; // Table or Query
    protected String m_sTableName;
    protected String[] m_aGroupNames;
    protected String[] m_aFieldNames;
    protected String[] m_aFieldTitleNames;
    protected int[] m_aFieldWidths;
    protected int[] m_aFieldTypes;
    // protected int[]    m_nLeftIndent;
    private DesignTemplate m_xDesignTemplate = null;

    public void initializeData(IReportBuilderLayouter _aOther)
    {
        if (_aOther instanceof ReportBuilderLayouter)
        {
            final ReportBuilderLayouter aOther = (ReportBuilderLayouter) _aOther;
            m_aCommandType = aOther.m_aCommandType;
            m_sTableName = aOther.m_sTableName;
            m_aGroupNames = aOther.m_aGroupNames;
            m_aFieldNames = aOther.m_aFieldNames;
            m_aFieldTitleNames = aOther.m_aFieldTitleNames;
            m_aFieldWidths = aOther.m_aFieldWidths;
            m_aFieldTypes = aOther.m_aFieldTypes;
            // m_nLeftIndent = aOther.m_nLeftIndent;
            m_xDesignTemplate = aOther.m_xDesignTemplate;

            // dirty PageWidth
            m_nPageWidth = -1;
        }
    }

    /**
     * Get the maximal label width of all labels
     * @return the width in 1/100mm
     */
    protected int getMaxLabelWidth()
    {
        int nWidth = 0;
        final String[] aFieldTitles = m_aFieldTitleNames; // we want all Field Titles here // getFieldTitleNames();
        for (int i = 0; i < aFieldTitles.length; i++)
        {
            final String sLabel = aFieldTitles[i];
            nWidth = Math.max(nWidth, getLabelWidth(sLabel));
        }
        for (int i = 0; i < m_aGroupNames.length; i++)
        {
            final String sGroupName = m_aGroupNames[i];
            final SectionObject a = getDesignTemplate().getGroupLabel(i);
            final FontDescriptor aFD = a.getFontDescriptor();
            nWidth = Math.max(nWidth, getLabelWidth(sGroupName, aFD));
        }

        if (nWidth == 0)
        {
            nWidth = 3000;
        }
        else
        {
            nWidth += 500;
        }
        return nWidth;
    }

    /**
     * Get width of a given string (Label) in 1/100mm
     * @param _sLabel
     * @return the width in 1/100mm
     */
    protected int getLabelWidth(String _sLabel)
    {
        return getLabelWidth(_sLabel, 0.0f, 0.0f);
    }
    XFixedText m_aFixedTextHelper = null;
    HashMap m_aLabelWidthMap;

    protected int getLabelWidth(String _sLabel, FontDescriptor _aFD)
    {
        float fCharWeight = 0.0f;
        float fCharHeight = 0.0f;
        if (_aFD != null)
        {
            fCharWeight = _aFD.Weight;
            fCharHeight = _aFD.Height;
        }
        return getLabelWidth(_sLabel, fCharWeight, fCharHeight);
    }

    protected int getLabelWidth(String _sLabel, float _nCharWeight, float _nCharHeight)
    {
        int nWidth = 0;

        if (m_aLabelWidthMap == null)
        {
            m_aLabelWidthMap = new HashMap();
        }
        // At first, try to get the Width out of a HashMap (Cache)
        StringBuffer aKey = new StringBuffer(40);
        final String sKey = aKey.append(_sLabel).append(_nCharWeight).append(_nCharHeight).toString();
        if (m_aLabelWidthMap.containsKey(sKey))
        {
            final Object aWidth = m_aLabelWidthMap.get(sKey);
            final Integer aIntegerWidth = (Integer) aWidth;
            nWidth = aIntegerWidth.intValue();
        }
        else
        {
            try
            {
                if (m_aFixedTextHelper == null)
                {
                    final Object aFixedText = getMSFofReportDefinition().createInstance("com.sun.star.report.FixedText");
                    m_aFixedTextHelper = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, aFixedText);
                }

                m_aFixedTextHelper.setLabel(_sLabel);
                if (_nCharWeight > 0.1f)
                {
                    m_aFixedTextHelper.setCharWeight(_nCharWeight);
                }
                if (_nCharHeight > 0.1f)
                {
                    m_aFixedTextHelper.setCharHeight(_nCharHeight);
                }

                final FontDescriptor xFont = m_aFixedTextHelper.getFontDescriptor();
                final Size aSize = getPreferredSize(_sLabel, xFont);
                nWidth = aSize.Width;
                // cache the found width
                m_aLabelWidthMap.put(sKey, new Integer(nWidth));
            }
            catch (com.sun.star.uno.Exception e)
            {
                // TODO: Exception not set.
            }
        }
        return nWidth;
    }

    protected void doNotBreakInTable(Object _xSectionOrGroup)
    {
//         try
//         {
//            _xSection.setKeepTogether(true);
        final PropertySetHelper aHelper = new PropertySetHelper(_xSectionOrGroup);
        aHelper.setPropertyValueDontThrow("KeepTogether", Boolean.TRUE);
//        }
//        catch (com.sun.star.uno.Exception e)
//        {
//            // Exception not set, but not really from interest.
//        }
    }

    protected DesignTemplate getDesignTemplate()
    {
        if (m_xDesignTemplate == null)
        {
            // initialise the report definition.
            String sDefaultHeaderLayout = m_xReportDefinitionReadAccess.getDefaultHeaderLayout();
            loadAndSetBackgroundTemplate(sDefaultHeaderLayout);

        // copy all functions from the design template to the current report definition
//            XFunctions xOtherFunctions = m_xDesignTemplate.getReportDefinition().getFunctions();
//            final int nFunctionCount = xOtherFunctions.getCount();
//            for (int i=0;i<nFunctionCount;i++)
//            {
//                try
//                {
//                    Object aOtherFunction = xOtherFunctions.getByIndex(i);
//                    XFunctions xFunctions = getReportDefinition().getFunctions();
//                    XFunction xFunction = xFunctions.createFunction();
//
//                    copyProperties(aOtherFunction, xFunction);
//                    xFunctions.insertByIndex(xFunctions.getCount(), xFunction);
//                }
//                catch (com.sun.star.lang.IllegalArgumentException e)
//                {
//                }
//                catch (IndexOutOfBoundsException ex)
//                {
//                    ex.printStackTrace();
//                }
//                catch (WrappedTargetException ex)
//                {
//                    ex.printStackTrace();
//                }
//
//            }

        }
        return m_xDesignTemplate;
    }

    /**
     * If there already exists a foreign report definition, which we use to get the layout from
     * close it.
     * Veto is not allowed here.
     */
    private void closeDesignTemplate()
    {
        if (m_xDesignTemplate != null)
        {
            m_xDesignTemplate.close();
            m_xDesignTemplate = null;
        }
    }

    /**
     * load the given string as a template and use it's content to paint the other
     * @param LayoutTemplatePath
     */
    public void loadAndSetBackgroundTemplate(String LayoutTemplatePath)
    {
        closeDesignTemplate();

        String sName = FileAccess.getFilename(LayoutTemplatePath);
        if (sName.toLowerCase().equals("default.otr_") ||
                LayoutTemplatePath.equals("DefaultLayoutOfHeaders"))
        {
            // this is the default layout, we don't want to have a layout for this.
        }
        else
        {
            XMultiServiceFactory xMSF = getGlobalMSF();
            m_xDesignTemplate = DesignTemplate.create(xMSF, LayoutTemplatePath);
        }
    }
}


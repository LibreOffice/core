/*
 * ***********************************************************************
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
 * **********************************************************************
 */
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.beans.PropertyValue;
// import com.sun.star.container.NoSuchElementException;
// import com.sun.star.lang.IndexOutOfBoundsException;
// import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XFixedText;
import com.sun.star.report.XFormattedField;
import com.sun.star.report.XGroup;
import com.sun.star.report.XGroups;
import com.sun.star.report.XReportDefinition;
import com.sun.star.report.XSection;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;

/**
 *
 * @author ll93751
 */
public class DesignTemplate
{

    private XReportDefinition m_xReportDefinition;
    XMultiServiceFactory m_xGlobalMSF;
    String m_sFilename;

    private DesignTemplate()
    {
    }
    // -------------------------------------------------------------------------
    // All functions from XReportDefinition, we use
    // -------------------------------------------------------------------------
    XGroups getGroups()
    {
        return m_xReportDefinition.getGroups();
    }

    XSection getDetail()
    {
        return m_xReportDefinition.getDetail();
    }

    boolean getReportHeaderOn()
    {
        return m_xReportDefinition.getReportHeaderOn();
    }

    XSection getReportHeader() throws com.sun.star.container.NoSuchElementException
    {
        return m_xReportDefinition.getReportHeader();
    }

    boolean getReportFooterOn()
    {
        return m_xReportDefinition.getReportFooterOn();
    }

    XSection getReportFooter() throws com.sun.star.container.NoSuchElementException
    {
        return m_xReportDefinition.getReportFooter();
    }

    boolean getPageHeaderOn()
    {
        return m_xReportDefinition.getPageHeaderOn();
    }

    XSection getPageHeader() throws com.sun.star.container.NoSuchElementException
    {
        return m_xReportDefinition.getPageHeader();
    }

    boolean getPageFooterOn()
    {
        return m_xReportDefinition.getPageFooterOn();
    }

    XSection getPageFooter() throws com.sun.star.container.NoSuchElementException
    {
        return m_xReportDefinition.getPageFooter();
    }    // -------------------------------------------------------------------------
    private SectionObject m_aDetailLabel;
    private SectionObject m_aDetailTextField;
    private SectionObject[] m_aGroupLabel;
    private SectionObject[] m_aGroupTextField;

    SectionObject getDetailLabel()
    {
        if (m_aDetailLabel == null)
        {
            return SectionEmptyObject.create();
        }
        return m_aDetailLabel;
    }

    SectionObject getDetailTextField()
    {
        if (m_aDetailTextField == null)
        {
            return SectionEmptyObject.create();
        }
        return m_aDetailTextField;
    }

    SectionObject getGroupTextField(int _nGroup)
    {
        return getGroupSectionObject(m_aGroupTextField, _nGroup);
    }

    SectionObject getGroupLabel(int _nGroup)
    {
        return getGroupSectionObject(m_aGroupLabel, _nGroup);
    }

    private SectionObject getGroupSectionObject(SectionObject _aGroupList[], int _nGroup)
    {
        SectionObject a = null;
        if (_nGroup < _aGroupList.length)
        {
            a = _aGroupList[_nGroup];
        }
        else
        {
            // if there are not enough groups, the last one will win
            if (_aGroupList.length > 0)
            {
                a = _aGroupList[_aGroupList.length - 1];
            }
        }
        // we show if the group is null
        if (a == null)
        {
            a = SectionEmptyObject.create();
            // a empty group has to use bold font
            a.setPropertyValue("CharWeight", new Float(com.sun.star.awt.FontWeight.BOLD));
        }
        return a;
    }

    // we analyse the loaded ReportDefinition, we want to know the FontDescriptor of all XSections
    private void analyseReportDefinition()
    {
        final XSection xDetailSection = m_xReportDefinition.getDetail();
        final int nDetailCount = xDetailSection.getCount();
        for (int i = 0; i < nDetailCount; i++)
        {
            try
            {
                Object aObj = xDetailSection.getByIndex(i);
                // is aObj a label
                // is aObj a textfield
                // m_aDetailLabel_FD
                // m_aDetailTextField_FD
                XFixedText aFixedText = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, aObj);
                if (aFixedText != null &&
                        m_aDetailLabel == null)
                {
                    m_aDetailLabel = SectionLabel.create(aFixedText);
                }
                else
                {
                    XFormattedField aFormattedField = (XFormattedField) UnoRuntime.queryInterface(XFormattedField.class, aObj);
                    if (aFormattedField != null &&
                            m_aDetailTextField == null)
                    {
                        m_aDetailTextField = SectionTextField.create(aFormattedField);
                    }
                }
                int dummy = 0;
            }
            catch (com.sun.star.lang.IndexOutOfBoundsException ex)
            {
            }
            catch (com.sun.star.lang.WrappedTargetException ex)
            {
            }
        }

        final XGroups xGroups = m_xReportDefinition.getGroups();
        final int nGroupCount = xGroups.getCount();
        // create a m_aGroupLabel_FD[]
        // create a m_aGroupTextField_FD[]
        m_aGroupLabel = new SectionObject[nGroupCount];
        m_aGroupTextField = new SectionObject[nGroupCount];

        for (int nGroup = 0; nGroup < nGroupCount; nGroup++)
        {
            try
            {
                Object aGroup = xGroups.getByIndex(nGroup);
                XGroup xGroup = (XGroup) UnoRuntime.queryInterface(XGroup.class, aGroup);
                XSection xGroupSection = xGroup.getHeader();

                final int nCount = xGroupSection.getCount();
                for (int i = 0; i < nCount; i++)
                {
                    try
                    {
                        Object aObj = xGroupSection.getByIndex(i);
                        XFixedText aFixedText = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, aObj);
                        // is aObj a label
                        // is aObj a textfield
                        if (aFixedText != null &&
                                m_aGroupLabel[nGroup] == null)
                        {
                            m_aGroupLabel[nGroup] = SectionLabel.create(aFixedText);
                        }
                        else
                        {
                            XFormattedField aFormattedField = (XFormattedField) UnoRuntime.queryInterface(XFormattedField.class, aObj);
                            if (aFormattedField != null &&
                                    m_aGroupTextField[nGroup] == null)
                            {
                                m_aGroupTextField[nGroup] = SectionTextField.create(aFormattedField);
                            }
                        }
                        int dummy = 0;
                    }
                    catch (com.sun.star.lang.IndexOutOfBoundsException ex)
                    {
                    }
                    catch (com.sun.star.lang.WrappedTargetException ex)
                    {
                    }
                }
            }
            catch (com.sun.star.container.NoSuchElementException ex)
            {
            }
            catch (com.sun.star.lang.IndexOutOfBoundsException ex)
            {
            }
            catch (com.sun.star.lang.WrappedTargetException ex)
            {
            }
        }
    }

    /**
     * close our current ReportDefinition
     */
    void close()
    {
        try
        {
            m_xReportDefinition.close(true);
        }
        catch (com.sun.star.util.CloseVetoException e)
        {
        }
    }

    /**
     * create a new DesignTemplate by try to load a otr file from the given path.
     * Internally we store the loaded ReportDefinition.
     *
     * @param _xMSF
     * @param _sPath
     * @return
     */
    public static DesignTemplate create(XMultiServiceFactory _xMSF, String _sPath)
    {
        DesignTemplate a = new DesignTemplate();
        try
        {
            a.load(_xMSF, _sPath);
        }
        catch (com.sun.star.lang.WrappedTargetRuntimeException e)
        {
            // this should not happen
            a = null;
            throw new java.lang.RuntimeException(e.getMessage());
        }
        catch (com.sun.star.uno.Exception e)
        {
            a = null;
        }
        catch (Exception e)
        {
            a = null;
        }
        return a;
    }

    private void load(XMultiServiceFactory _xMSF, String _sPath) throws com.sun.star.uno.Exception
    {
        m_xGlobalMSF = _xMSF;
        m_sFilename = _sPath;
        final Object aObj = _xMSF.createInstance("com.sun.star.report.ReportDefinition");
        m_xReportDefinition = (XReportDefinition) UnoRuntime.queryInterface(XReportDefinition.class, aObj);

        PropertyValue[] aLoadProps = new PropertyValue[2];
        aLoadProps[0] = Properties.createProperty(PropertyNames.URL, _sPath);
        aLoadProps[1] = Properties.createProperty(PropertyNames.READ_ONLY, Boolean.TRUE);

        m_xReportDefinition.load(aLoadProps);
        analyseReportDefinition();
    }

    public XReportDefinition getReportDefinition()
    {
        return m_xReportDefinition;
    }
}

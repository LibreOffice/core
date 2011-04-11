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
package com.sun.star.wizards.text;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Vector;

import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.text.XTextRange;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.DateTime;
import com.sun.star.util.XRefreshable;
import com.sun.star.util.XUpdatable;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;

public class TextFieldHandler
{

    public XTextFieldsSupplier xTextFieldsSupplier;
    private XMultiServiceFactory xMSFDoc;

    /**
     * Creates a new instance of TextFieldHandler
     * @param xMSF
     * @param xTextDocument
     */
    public TextFieldHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument)
    {
        this.xMSFDoc = xMSF;
        xTextFieldsSupplier = (XTextFieldsSupplier) UnoRuntime.queryInterface(XTextFieldsSupplier.class, xTextDocument);
    }

    public void refreshTextFields()
    {
        XRefreshable xUp = (XRefreshable) UnoRuntime.queryInterface(XRefreshable.class, xTextFieldsSupplier.getTextFields());
        xUp.refresh();
    }

    public String getUserFieldContent(XTextCursor xTextCursor)
    {
        try
        {
            XTextRange xTextRange = xTextCursor.getEnd();
            Object oTextField = Helper.getUnoPropertyValue(xTextRange, "TextField");
            if (com.sun.star.uno.AnyConverter.isVoid(oTextField))
            {
                return "";
            }
            else
            {
                XDependentTextField xDependent = (XDependentTextField) UnoRuntime.queryInterface(XDependentTextField.class, oTextField);
                XPropertySet xMaster = xDependent.getTextFieldMaster();
                String UserFieldContent = (String) xMaster.getPropertyValue("Content");
                return UserFieldContent;
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        return "";
    }

    public void insertUserField(XTextCursor xTextCursor, String FieldName, String FieldTitle)
    {
        try
        {
            XInterface xField = (XInterface) xMSFDoc.createInstance("com.sun.star.text.TextField.User");
            XDependentTextField xDepField = (XDependentTextField) UnoRuntime.queryInterface(XDependentTextField.class, xField);
            XTextContent xFieldContent = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xField);
            if (xTextFieldsSupplier.getTextFieldMasters().hasByName("com.sun.star.text.FieldMaster.User." + FieldName))
            {
                Object oMaster = xTextFieldsSupplier.getTextFieldMasters().getByName("com.sun.star.text.FieldMaster.User." + FieldName);
                XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, oMaster);
                xComponent.dispose();
            }
            XPropertySet xPSet = createUserField(FieldName, FieldTitle);
            xDepField.attachTextFieldMaster(xPSet);
            xTextCursor.getText().insertTextContent(xTextCursor, xFieldContent, false);

//            try
//            {
//                XPropertySet xTestProp = xDepField.getTextFieldMaster();
//                String UserFieldName = (String) xTestProp.getPropertyValue(PropertyNames.PROPERTY_NAME);
//                // UserFieldName == FieldName?
//                int dummy = 0;
//            }
//            catch (com.sun.star.uno.Exception e)
//            {
//                int dummy2 = 0;
//            }

        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public XPropertySet createUserField(String FieldName, String FieldTitle) throws com.sun.star.uno.Exception
    {
        Object oMaster = xMSFDoc.createInstance("com.sun.star.text.FieldMaster.User");
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oMaster);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, FieldName);
        xPSet.setPropertyValue("Content", FieldTitle);

        // DEBUG
        // String sFieldName = (String)xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME);

        return xPSet;
    }

    private XDependentTextField[] getTextFieldsByProperty(String _PropertyName, Object _aPropertyValue, String _TypeName) throws Exception
    {
        try
        {
            XDependentTextField[] xDependentFields;
            Vector xDependentVector = new Vector();
            if (xTextFieldsSupplier.getTextFields().hasElements())
            {
                XEnumeration xEnum = xTextFieldsSupplier.getTextFields().createEnumeration();
                while (xEnum.hasMoreElements())
                {
                    Object oTextField = xEnum.nextElement();
                    XDependentTextField xDependent = (XDependentTextField) UnoRuntime.queryInterface(XDependentTextField.class, oTextField);
                    XPropertySet xPropertySet = xDependent.getTextFieldMaster();
                    if (xPropertySet.getPropertySetInfo().hasPropertyByName(_PropertyName))
                    {
                        Object oValue = xPropertySet.getPropertyValue(_PropertyName);
                        // TODO replace the following comparison via com.sun.star.uno.Any.Type
                        if (AnyConverter.isString(oValue))
                        {
                            if (_TypeName.equals("String"))
                            {
                                String sValue = AnyConverter.toString(oValue);
                                if (sValue.equals(_aPropertyValue))
                                {
                                    xDependentVector.addElement(xDependent);
                                }
                            }
                        }
                        else if (AnyConverter.isShort(oValue))
                        {
                            if (_TypeName.equals("Short"))
                            {
                                short iShortParam = ((Short) _aPropertyValue).shortValue();
                                short ishortValue = AnyConverter.toShort(oValue);
                                if (ishortValue == iShortParam)
                                {
                                    xDependentVector.addElement(xDependent);
                                }
                            }
                        }
                    }
                }
            }
            if (xDependentVector.size() > 0)
            {
                xDependentFields = new XDependentTextField[xDependentVector.size()];
                xDependentVector.toArray(xDependentFields);
                return xDependentFields;
            }
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace(System.out);
        }
        return null;
    }

    public void changeUserFieldContent(String _FieldName, String _FieldContent)
    {
        try
        {
            XDependentTextField[] xDependentTextFields = getTextFieldsByProperty(PropertyNames.PROPERTY_NAME, _FieldName, "String");
            if (xDependentTextFields != null)
            {
                for (int i = 0; i < xDependentTextFields.length; i++)
                {
                    xDependentTextFields[i].getTextFieldMaster().setPropertyValue("Content", _FieldContent);
                }
                refreshTextFields();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void updateDocInfoFields()
    {
        try
        {
            XEnumeration xEnum = xTextFieldsSupplier.getTextFields().createEnumeration();
            while (xEnum.hasMoreElements())
            {
                Object oTextField = xEnum.nextElement();
                XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oTextField);

                if (xSI.supportsService("com.sun.star.text.TextField.ExtendedUser"))
                {
                    XUpdatable xUp = (XUpdatable) UnoRuntime.queryInterface(XUpdatable.class, oTextField);
                    xUp.update();
                }
                if (xSI.supportsService("com.sun.star.text.TextField.User"))
                {
                    XUpdatable xUp = (XUpdatable) UnoRuntime.queryInterface(XUpdatable.class, oTextField);
                    xUp.update();
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void updateDateFields()
    {
        try
        {
            XEnumeration xEnum = xTextFieldsSupplier.getTextFields().createEnumeration();
            Calendar cal = new GregorianCalendar();
            DateTime dt = new DateTime();
            dt.Day = (short) cal.get(Calendar.DAY_OF_MONTH);
            dt.Year = (short) cal.get(Calendar.YEAR);
            dt.Month = (short) cal.get(Calendar.MONTH);
            dt.Month++;

            while (xEnum.hasMoreElements())
            {
                Object oTextField = xEnum.nextElement();
                XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oTextField);

                if (xSI.supportsService("com.sun.star.text.TextField.DateTime"))
                {
                    XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oTextField);
                    xPSet.setPropertyValue("IsFixed", Boolean.FALSE);
                    xPSet.setPropertyValue("DateTimeValue", dt);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void fixDateFields(boolean _bSetFixed)
    {
        try
        {
            XEnumeration xEnum = xTextFieldsSupplier.getTextFields().createEnumeration();
            while (xEnum.hasMoreElements())
            {
                Object oTextField = xEnum.nextElement();
                XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oTextField);
                if (xSI.supportsService("com.sun.star.text.TextField.DateTime"))
                {
                    XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oTextField);
                    xPSet.setPropertyValue("IsFixed", new Boolean(_bSetFixed));
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void removeUserFieldByContent(String _FieldContent)
    {
        try
        {
            XDependentTextField[] xDependentTextFields = getTextFieldsByProperty("Content", _FieldContent, "String");
            if (xDependentTextFields != null)
            {
                for (int i = 0; i < xDependentTextFields.length; i++)
                {
                    xDependentTextFields[i].dispose();
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public void changeExtendedUserFieldContent(short UserDataPart, String _FieldContent)
    {
        try
        {
            XDependentTextField[] xDependentTextFields = getTextFieldsByProperty("UserDataType", new Short(UserDataPart), "Short");
            if (xDependentTextFields != null)
            {
                for (int i = 0; i < xDependentTextFields.length; i++)
                {
                    xDependentTextFields[i].getTextFieldMaster().setPropertyValue("Content", _FieldContent);
                }
            }
            refreshTextFields();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }
}

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
package com.sun.star.wizards.text;

import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyleLoader;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.PropertyNames;

public class TextStyleHandler
{

    public XStyleFamiliesSupplier xStyleFamiliesSupplier;

    /** Creates a new instance of TextStyleHandler */
    public TextStyleHandler(XTextDocument xTextDocument)
    {
        xStyleFamiliesSupplier = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
    }

    public void loadStyleTemplates(String sTemplateUrl, String OptionString)
    {
        try
        {
            XStyleLoader xStyleLoader = UnoRuntime.queryInterface(XStyleLoader.class, xStyleFamiliesSupplier.getStyleFamilies());
            com.sun.star.beans.PropertyValue[] StyleOptions = xStyleLoader.getStyleLoaderOptions();
            String CurOptionName = PropertyNames.EMPTY_STRING;
            int PropCount = StyleOptions.length;
            for (int i = 0; i < PropCount; i++)
            {
                CurOptionName = StyleOptions[i].Name;
                StyleOptions[i].Value = Boolean.valueOf(CurOptionName.equals(OptionString) || CurOptionName.equals("OverwriteStyles"));
            }
            xStyleLoader.loadStylesFromURL(sTemplateUrl, StyleOptions);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public XPropertySet getStyleByName(String sStyleFamily, String sStyleName)
    {
        try
        {
            XPropertySet xPropertySet = null;
            Object oStyleFamily = xStyleFamiliesSupplier.getStyleFamilies().getByName(sStyleFamily);
            XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, oStyleFamily);
            if (xNameAccess.hasByName(sStyleName))
            {
                xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xNameAccess.getByName(sStyleName));
            }
            return xPropertySet;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return null;
    }

    public Size changePageAlignment(XPropertySet _xPropPageStyle, boolean _bIsLandscape)
    {
        try
        {
            _xPropPageStyle.setPropertyValue("IsLandscape", Boolean.valueOf(_bIsLandscape));
            Size aPageSize = (Size) AnyConverter.toObject(Size.class, _xPropPageStyle.getPropertyValue("Size"));
            int nPageWidth = aPageSize.Width;
            int nPageHeight = aPageSize.Height;
            Size aSize = new Size(nPageHeight, nPageWidth);
            _xPropPageStyle.setPropertyValue("Size", aSize);
            return (Size) AnyConverter.toObject(Size.class, _xPropPageStyle.getPropertyValue("Size"));
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }
}

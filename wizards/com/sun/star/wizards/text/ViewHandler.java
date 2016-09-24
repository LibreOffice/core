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

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextViewCursor;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.XViewSettingsSupplier;
import com.sun.star.wizards.common.Helper;

public class ViewHandler
{

    private final XTextViewCursorSupplier xTextViewCursorSupplier;
    private final XStyleFamiliesSupplier xStyleFamiliesSupplier;
    private final XViewSettingsSupplier xViewSettingsSupplier;

    /** Creates a new instance of View */
    public ViewHandler(XTextDocument xTextDocument)
    {
        xTextViewCursorSupplier = UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xTextDocument.getCurrentController());
        xViewSettingsSupplier = UnoRuntime.queryInterface(XViewSettingsSupplier.class, xTextDocument.getCurrentController());
        xStyleFamiliesSupplier = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
    }

    public void selectFirstPage(TextTableHandler oTextTableHandler)
    {
        try
        {
            XPageCursor xPageCursor = UnoRuntime.queryInterface(XPageCursor.class, xTextViewCursorSupplier.getViewCursor());
            XTextCursor xViewTextCursor = UnoRuntime.queryInterface(XTextCursor.class, xPageCursor);
            xPageCursor.jumpToFirstPage();
            xPageCursor.jumpToStartOfPage();
            Helper.setUnoPropertyValue(xPageCursor, "PageDescName", "First Page");
            Object oPageStyles = xStyleFamiliesSupplier.getStyleFamilies().getByName("PageStyles");
            XNameAccess xName = UnoRuntime.queryInterface(XNameAccess.class, oPageStyles);
            Object oPageStyle = xName.getByName("First Page");
            XIndexAccess xAllTextTables = UnoRuntime.queryInterface(XIndexAccess.class, oTextTableHandler.xTextTablesSupplier.getTextTables());
            XTextContent xTextTable = UnoRuntime.queryInterface(XTextContent.class, xAllTextTables.getByIndex(0));
            XTextRange xRange = UnoRuntime.queryInterface(XTextRange.class, xTextTable.getAnchor().getText());
            xViewTextCursor.gotoRange(xRange, false);
            XTextRange xHeaderRange = (XTextRange) Helper.getUnoPropertyValue(oPageStyle, "HeaderText", XTextRange.class);
            if (!com.sun.star.uno.AnyConverter.isVoid(xHeaderRange))
            {
                xViewTextCursor.gotoRange(xHeaderRange, false);
                xViewTextCursor.collapseToStart();
            }
            else
            {
                System.out.println("No Headertext available");
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void setViewSetting(String Setting, Object Value) throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException
    {
        xViewSettingsSupplier.getViewSettings().setPropertyValue(Setting, Value);
    }

    public void collapseViewCursorToStart()
    {
        XTextViewCursor xTextViewCursor = xTextViewCursorSupplier.getViewCursor();
        xTextViewCursor.collapseToStart();
    }
}

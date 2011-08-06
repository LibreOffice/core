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

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
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

    private XTextViewCursorSupplier xTextViewCursorSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;
    private XStyleFamiliesSupplier xStyleFamiliesSupplier;
    private XViewSettingsSupplier xViewSettingsSupplier;

    /** Creates a new instance of View */
    public ViewHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument)
    {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
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
//            if (xPageCursor.getPage() == (short) 1) {
//                Helper.setUnoPropertyValue(xTextTable, "PageDescName", "First Page");
//                TextTableHandler.resetBreakTypeofTextTable(xTextTable);
//            }
            //      }
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
            exception.printStackTrace(System.out);
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

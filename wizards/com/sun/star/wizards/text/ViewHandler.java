/*************************************************************************
*
*  $RCSfile: ViewHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:49:50 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*/

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

public class ViewHandler {
    private XTextViewCursorSupplier xTextViewCursorSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;
    private XStyleFamiliesSupplier xStyleFamiliesSupplier;
    private XViewSettingsSupplier xViewSettingsSupplier;

    /** Creates a new instance of View */
    public ViewHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xTextViewCursorSupplier = (XTextViewCursorSupplier) UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xTextDocument.getCurrentController());
        xViewSettingsSupplier = (XViewSettingsSupplier) UnoRuntime.queryInterface(XViewSettingsSupplier.class, xTextDocument.getCurrentController());
        xStyleFamiliesSupplier = (XStyleFamiliesSupplier) UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
    }

    public void selectFirstPage(TextTableHandler oTextTableHandler) {
        try {
            XTextRange xRange;
            XTextContent xTextTable;
            XPageCursor xPageCursor = (XPageCursor) UnoRuntime.queryInterface(XPageCursor.class, xTextViewCursorSupplier.getViewCursor());
            XTextCursor xViewTextCursor = (XTextCursor) UnoRuntime.queryInterface(XTextCursor.class, xPageCursor);
            xPageCursor.jumpToFirstPage();
            xPageCursor.jumpToStartOfPage();
            Helper.setUnoPropertyValue(xPageCursor, "PageDescName", "First Page");
            Object oPageStyles = xStyleFamiliesSupplier.getStyleFamilies().getByName("PageStyles");
            XNameAccess xName = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oPageStyles);
            Object oPageStyle = xName.getByName("First Page");
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, oTextTableHandler.xTextTablesSupplier.getTextTables());
            int TextTablesCount = xAllTextTables.getCount();
            //      for (int i = 0; i<TextTablesCount; i++){
            xTextTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xAllTextTables.getByIndex(0));
            xRange = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xTextTable.getAnchor().getText());
            xViewTextCursor.gotoRange(xRange, false);
            if (xPageCursor.getPage() == (short) 1) {
                Helper.setUnoPropertyValue(xTextTable, "PageDescName", "First Page");
                TextTableHandler.resetBreakTypeofTextTable(xTextTable);
            }
            //      }
            XTextRange xHeaderRange = (XTextRange) Helper.getUnoPropertyValue(oPageStyle, "HeaderText", XTextRange.class);
            if (com.sun.star.uno.AnyConverter.isVoid(xHeaderRange) == false) {
                xViewTextCursor.gotoRange(xHeaderRange, false);
                xViewTextCursor.collapseToStart();
            } else
                System.out.println("No Headertext available");

        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void setViewSetting(String Setting, Object Value) throws UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException {
        xViewSettingsSupplier.getViewSettings().setPropertyValue(Setting, Value);
    }

    public void collapseViewCursorToStart() {
        XTextViewCursor xTextViewCursor = xTextViewCursorSupplier.getViewCursor();
        xTextViewCursor.collapseToStart();
    }

}

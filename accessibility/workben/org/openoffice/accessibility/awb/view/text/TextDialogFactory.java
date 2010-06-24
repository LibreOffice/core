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

package org.openoffice.accessibility.awb.view.text;

import javax.swing.JDialog;
import javax.swing.text.JTextComponent;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;


/** Factory for dialogs of the text views.
*/
public class TextDialogFactory
{
    static public JDialog CreateSelectionDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextActionDialog(
            xContext,
            "Select range:",
            "select")
            {
                boolean TextAction (XAccessibleText xText)
                    throws IndexOutOfBoundsException
                {
                    return xText.setSelection(
                        GetSelectionStart(),
                        GetSelectionEnd() );
                }
            };
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }

    static public JDialog CreateCopyDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextActionDialog(
            xContext,
            "Select range and copy:",
            "copy")
            {
                boolean TextAction (XAccessibleText xText)
                    throws IndexOutOfBoundsException
                {
                    return xText.copyText(
                        GetSelectionStart(),
                        GetSelectionEnd());
                }
            };
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }
    static public JDialog CreateCutDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextActionDialog(
            xContext,
            "Select range and cut:",
            "cut")
            {
                boolean EditableTextAction (XAccessibleEditableText xText)
                    throws IndexOutOfBoundsException
                {
                    return xText.cutText(
                        GetSelectionStart(),
                        GetSelectionEnd() );
                }
            };
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }
    static public JDialog CreatePasteDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextActionDialog (
            xContext,
            "Place Caret and paste:",
            "paste")
            {
                boolean EditableTextAction (XAccessibleEditableText xText)
                    throws IndexOutOfBoundsException
                {
                    return xText.pasteText(maText.getCaretPosition());
                }
            };
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }
    static public JDialog CreateEditDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextEditDialog (
            xContext,
            "Edit text:",
            "edit");
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }
    static public JDialog CreateFormatDialog (XAccessibleContext xContext)
    {
        JDialog aDialog = new TextAttributeDialog (xContext);
        if (aDialog != null)
            aDialog.show();
        return aDialog;
    }
}

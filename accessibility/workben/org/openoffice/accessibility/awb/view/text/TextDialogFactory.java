/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

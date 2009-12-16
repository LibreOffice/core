/*************************************************************************
 *
 *  $RCSfile: TextDialogFactory.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:43 $
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

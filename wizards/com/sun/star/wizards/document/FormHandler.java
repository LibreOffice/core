/*************************************************************************
*
*  $RCSfile: FormHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:41:47 $
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
package com.sun.star.wizards.document;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.wizards.common.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.container.XNamed;

public class FormHandler {

    public XFormsSupplier xFormsSupplier;
    private com.sun.star.lang.XMultiServiceFactory xMSFDoc;
    private XDrawPage xDrawPage;
    private XDrawPageSupplier xDrawPageSupplier;

    /** Creates a new instance of FormHandler */
    public FormHandler(com.sun.star.lang.XMultiServiceFactory xMSF, com.sun.star.text.XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        xDrawPageSupplier = (XDrawPageSupplier) UnoRuntime.queryInterface(XDrawPageSupplier.class, xTextDocument);
        xDrawPage = xDrawPageSupplier.getDrawPage();
        xFormsSupplier = (XFormsSupplier) UnoRuntime.queryInterface(XFormsSupplier.class, xDrawPage);
    }

    public XNameContainer getDocumentForms() {
        XNameContainer xNamedForms = xFormsSupplier.getForms();
        return xNamedForms;
    }

    public String getValueofHiddenControl(XNameAccess xNamedForm, String ControlName, String sMsg) throws com.sun.star.wizards.document.FormHandler.UnknownHiddenControlException {
        try {
            if (xNamedForm.hasByName(ControlName)) {
                String ControlValue = AnyConverter.toString(com.sun.star.wizards.common.Helper.getUnoPropertyValue(xNamedForm.getByName(ControlName), "HiddenValue"));
                return ControlValue;
            } else
                throw new UnknownHiddenControlException(xNamedForm, ControlName, sMsg);
        } catch (com.sun.star.uno.Exception exception) {
            throw new UnknownHiddenControlException(xNamedForm, ControlName, sMsg);
        }
    }

    public void insertHiddenControl(XNameAccess xNameAccess, XNameContainer xNamedForm, String ControlName, String ControlValue) {
        try {
            XInterface xHiddenControl;
            if (xNameAccess.hasByName(ControlName) == true)
                xHiddenControl = (XInterface) AnyConverter.toObject(new com.sun.star.uno.Type(XInterface.class), xNameAccess.getByName(ControlName));
            else {
                xHiddenControl = (XInterface) xMSFDoc.createInstance("com.sun.star.form.component.HiddenControl");
                xNamedForm.insertByName(ControlName, xHiddenControl);
            }
            Helper.setUnoPropertyValue(xHiddenControl, "HiddenValue", ControlValue);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public class UnknownHiddenControlException extends java.lang.Throwable {
        public UnknownHiddenControlException(XNameAccess xNamedForm, String ControlName, String sMsgHiddenControlisMissing) {
            XNamed xNamed = (XNamed) UnoRuntime.queryInterface(XNamed.class, xNamedForm);
            String FormName = xNamed.getName();
            sMsgHiddenControlisMissing = JavaTools.replaceSubString(sMsgHiddenControlisMissing, FormName, "<REPORTFORM>");
            sMsgHiddenControlisMissing = JavaTools.replaceSubString(sMsgHiddenControlisMissing, ControlName, "<CONTROLNAME>");
            SystemDialog.showMessageBox(xMSFDoc, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sMsgHiddenControlisMissing);
        }
    }

    public XNameContainer insertFormbyName(String FormName) {
        try {
            XInterface xDBForm;
            XNameContainer xNamedForms = getDocumentForms();
            XNameAccess xForms = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xNamedForms);
            if (xForms.hasByName(FormName) == false) {
                xDBForm = (XInterface) xMSFDoc.createInstance("com.sun.star.form.component.Form");
                xNamedForms.insertByName(FormName, xDBForm);
            } else
                xDBForm = (XInterface) AnyConverter.toObject(new com.sun.star.uno.Type(XInterface.class), com.sun.star.wizards.common.Helper.getUnoObjectbyName(xForms, FormName));
            XNameContainer xNamedForm = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xDBForm);
            return xNamedForm;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }
}

/*************************************************************************
*
*  $RCSfile: Resource.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:37:47 $
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

package com.sun.star.wizards.common;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XInvocation;

public class Resource {
    XInvocation xInvocation;
    XMultiServiceFactory xMSF;
    String Unit;
    String Module;

    /** Creates a new instance of Resource */
    public Resource(XMultiServiceFactory _xMSF, String _Unit, String _Module) {
        this.xMSF = _xMSF;
        this.Unit = _Unit;
        this.Module = _Module;
        this.xInvocation = initResources();
    }

    public String getResText(int nID) {
        try {
            short[][] PointerArray = new short[1][];
            Object[][] DummyArray = new Object[1][];
            Object[] nIDArray = new Object[1];
            nIDArray[0] = new Integer(nID);
            String IDString = (String) xInvocation.invoke("getString", nIDArray, PointerArray, DummyArray);
            return IDString;
        } catch (Exception exception) {
            exception.printStackTrace();
            throw new java.lang.IllegalArgumentException("Resource with ID not" + String.valueOf(nID) + "not found");
        }
    }

    public String[] getResArray(int nID, int iCount) {
        try {
            String[] ResArray = new String[iCount];
            for (int i = 0; i < iCount; i++) {
                ResArray[i] = getResText(nID + i);
            }
            return ResArray;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            throw new java.lang.IllegalArgumentException("Resource with ID not" + String.valueOf(nID) + "not found");
        }
    }

    public XInvocation initResources() {
        try {
            com.sun.star.uno.XInterface xResource = (com.sun.star.uno.XInterface) xMSF.createInstance("com.sun.star.resource.VclStringResourceLoader");
            if (xResource == null) {
                showCommonResourceError(xMSF);
                throw new IllegalArgumentException();
            } else {
                XInvocation xResInvoke = (XInvocation) com.sun.star.uno.UnoRuntime.queryInterface(XInvocation.class, xResource);
                xResInvoke.setValue("FileName", Module);
                return xResInvoke;
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            showCommonResourceError(xMSF);
            return null;
        }
    }

    public static void showCommonResourceError(XMultiServiceFactory xMSF) {
        String ProductName = Configuration.getProductName(xMSF);
        String sError = "The files required could not be found.\\nPlease start the %PRODUCTNAME Setup and choose 'Repair'.";
        sError = JavaTools.replaceSubString(sError, ProductName, "%PRODUCTNAME");
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sError);
    }

}

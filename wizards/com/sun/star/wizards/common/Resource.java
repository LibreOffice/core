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

package com.sun.star.wizards.common;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XInvocation;
import com.sun.star.beans.PropertyValue;

public class Resource
{

    XInvocation xInvocation;
    XMultiServiceFactory xMSF;
    String Unit;
    String Module;

    /** Creates a new instance of Resource
     * @param _xMSF
     * @param _Unit
     * @param _Module
     */
    public Resource(XMultiServiceFactory _xMSF, String _Unit, String _Module)
    {
        this.xMSF = _xMSF;
        this.Unit = _Unit;
        this.Module = _Module;
        this.xInvocation = initResources();
    }

    public String getResText(int nID)
    {
        try
        {
            short[][] PointerArray = new short[1][];
            Object[][] DummyArray = new Object[1][];
            Object[] nIDArray = new Object[1];
            nIDArray[0] = new Integer(nID);
            return (String) xInvocation.invoke("getString", nIDArray, PointerArray, DummyArray);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            throw new java.lang.IllegalArgumentException("Resource with ID not" + String.valueOf(nID) + "not found");
        }
    }

    public PropertyValue[] getStringList(int nID)
    {
        try
        {
            short[][] PointerArray = new short[1][];
            Object[][] DummyArray = new Object[1][];
            Object[] nIDArray = new Object[1];
            nIDArray[0] = new Integer(nID);
            //Object bla = xInvocation.invoke("getStringList", nIDArray, PointerArray, DummyArray);
            return (PropertyValue[]) xInvocation.invoke("getStringList", nIDArray, PointerArray, DummyArray);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            throw new java.lang.IllegalArgumentException("Resource with ID not" + String.valueOf(nID) + "not found");
        }
    }

    public String[] getResArray(int nID, int iCount)
    {
        try
        {
            String[] ResArray = new String[iCount];
            for (int i = 0; i < iCount; i++)
            {
                ResArray[i] = getResText(nID + i);
            }
            return ResArray;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            throw new java.lang.IllegalArgumentException("Resource with ID not" + String.valueOf(nID) + "not found");
        }
    }

    public XInvocation initResources()
    {
        try
        {
            com.sun.star.uno.XInterface xResource = (com.sun.star.uno.XInterface) xMSF.createInstance("com.sun.star.resource.VclStringResourceLoader");
            if (xResource == null)
            {
                showCommonResourceError(xMSF);
                throw new IllegalArgumentException();
            }
            else
            {
                XInvocation xResInvoke = com.sun.star.uno.UnoRuntime.queryInterface(XInvocation.class, xResource);
                xResInvoke.setValue("FileName", Module);
                return xResInvoke;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
            showCommonResourceError(xMSF);
            return null;
        }
    }

    public static void showCommonResourceError(XMultiServiceFactory xMSF)
    {
        String ProductName = Configuration.getProductName(xMSF);
        String sError = "The files required could not be found.\nPlease start the %PRODUCTNAME Setup and choose 'Repair'.";
        sError = JavaTools.replaceSubString(sError, ProductName, "%PRODUCTNAME");
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sError);
    }
}

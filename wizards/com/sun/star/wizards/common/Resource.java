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

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XInvocation;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

public class Resource
{

    XMultiServiceFactory xMSF;
    String Module;
    XIndexAccess xStringIndexAccess;
    XIndexAccess xStringListIndexAccess;

    /** Creates a new instance of Resource
     * @param _xMSF
     * @param _Unit
     * @param _Module
     */
    public Resource(XMultiServiceFactory _xMSF, String _Unit /* unused */, String _Module)
    {
        this.xMSF = _xMSF;
        this.Module = _Module;
        try
        {
            Object[] aArgs = new Object[1];
            aArgs[0] = this.Module;
            XInterface xResource = (XInterface) xMSF.createInstanceWithArguments(
                "org.libreoffice.resource.ResourceIndexAccess",
                aArgs);
            if (xResource == null)
                throw new Exception("could not initialize ResourceIndexAccess");
            XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(
                XNameAccess.class,
                xResource);
            if (xNameAccess == null)
                throw new Exception("ResourceIndexAccess is no XNameAccess");
            this.xStringIndexAccess = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class,
                xNameAccess.getByName("String"));
            this.xStringListIndexAccess = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class,
                xNameAccess.getByName("StringList"));
            if(this.xStringListIndexAccess == null)
                throw new Exception("could not initialize xStringListIndexAccess");
            if(this.xStringIndexAccess == null)
                throw new Exception("could not initialize xStringIndexAccess");
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            showCommonResourceError(xMSF);
        }
    }

    public String getResText(int nID)
    {
        try
        {
            return (String)this.xStringIndexAccess.getByIndex(nID);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            throw new java.lang.IllegalArgumentException("Resource with ID not " + String.valueOf(nID) + "not found");
        }
    }

    public PropertyValue[] getStringList(int nID)
    {
        try
        {
            return (PropertyValue[])this.xStringListIndexAccess.getByIndex(nID);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            throw new java.lang.IllegalArgumentException("Resource with ID not " + String.valueOf(nID) + "not found");
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

    public static void showCommonResourceError(XMultiServiceFactory xMSF)
    {
        String ProductName = Configuration.getProductName(xMSF);
        String sError = "The files required could not be found.\nPlease start the %PRODUCTNAME Setup and choose 'Repair'.";
        sError = JavaTools.replaceSubString(sError, ProductName, "%PRODUCTNAME");
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sError);
    }
}

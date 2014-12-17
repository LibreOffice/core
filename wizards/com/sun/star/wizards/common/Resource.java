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

package com.sun.star.wizards.common;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

public class Resource
{

    private XIndexAccess xStringIndexAccess;

    /** Creates a new instance of Resource
     */
    public Resource(XMultiServiceFactory _xMSF, String _Module)
    {
        try
        {
            Object[] aArgs = new Object[] { _Module };
            XInterface xResource = (XInterface) _xMSF.createInstanceWithArguments(
                "org.libreoffice.resource.ResourceIndexAccess",
                aArgs);
            if (xResource == null)
                throw new Exception("could not initialize ResourceIndexAccess");
            XNameAccess xNameAccess = UnoRuntime.queryInterface(
                XNameAccess.class,
                xResource);
            if (xNameAccess == null)
                throw new Exception("ResourceIndexAccess is no XNameAccess");
            this.xStringIndexAccess = UnoRuntime.queryInterface(
                XIndexAccess.class,
                xNameAccess.getByName("String"));
            XIndexAccess xStringListIndexAccess = UnoRuntime.queryInterface(
                XIndexAccess.class,
                xNameAccess.getByName("StringList"));
            if(xStringListIndexAccess == null)
                throw new Exception("could not initialize xStringListIndexAccess");
            if(this.xStringIndexAccess == null)
                throw new Exception("could not initialize xStringIndexAccess");
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
            showCommonResourceError(_xMSF);
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
            throw new java.lang.IllegalArgumentException("Resource with ID not " + nID + "not found", exception);
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
            throw new java.lang.IllegalArgumentException("Resource with ID not" + nID + "not found", exception);
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

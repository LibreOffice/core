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
package com.sun.star.wizards.table;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;

/**
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class CGCategory
{

    public String Name;
    public int Index;
    private String[] Tables;
    private final String CGROOTPATH = "/org.openoffice.Office.TableWizard/TableWizard/";
    XMultiServiceFactory xMSF;
    XNameAccess xNameAccessTablesNode;
    XNameAccess xNameAccessCurBusinessNode;
    Object oconfigView;

    public CGCategory(XMultiServiceFactory _xMSF)
    {
        xMSF = _xMSF;
    }

    public void initialize(String category)
    {
        try
        {
            oconfigView = Configuration.getConfigurationRoot(xMSF, CGROOTPATH, false);  //business/Tables   
            xNameAccessCurBusinessNode = Configuration.getChildNodebyName(
                UnoRuntime.queryInterface(XNameAccess.class, oconfigView),
                category);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public String[] getTableNames()
    {
        try
        {
            xNameAccessTablesNode = UnoRuntime.queryInterface(XNameAccess.class, xNameAccessCurBusinessNode.getByName("Tables"));
            return Configuration.getNodeDisplayNames(xNameAccessTablesNode);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }
}

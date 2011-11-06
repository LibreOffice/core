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


package com.sun.star.wizards.table;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
// import com.sun.star.wizards.common.ConfigGroup;
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
            e.printStackTrace(System.out);
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
            e.printStackTrace(System.out);
            return null;
        }
    }
}

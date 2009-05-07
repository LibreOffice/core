/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGCategory.java,v $
 * $Revision: 1.6 $
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

    public void initialize(int _index)
    {
        try
        {
            oconfigView = Configuration.getConfigurationRoot(xMSF, CGROOTPATH, false);  //business/Tables
            xNameAccessCurBusinessNode = Configuration.getChildNodebyIndex(oconfigView, _index);
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
            xNameAccessTablesNode = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xNameAccessCurBusinessNode.getByName("Tables"));
            String[] sTableNames = Configuration.getNodeDisplayNames(xNameAccessTablesNode);
            return sTableNames;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return null;
        }
    }
}

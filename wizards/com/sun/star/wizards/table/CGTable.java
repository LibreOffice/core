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
package com.sun.star.wizards.table;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.PropertyNames;

public class CGTable
{

    XMultiServiceFactory xMSF;
    XNameAccess xNameAccessFieldsNode;
    XNameAccess xNameAccessTableNode;
    public String Index;
    public String Name;
    private Object oconfigView;
    private final String CGROOTPATH = "/org.openoffice.Office.TableWizard/TableWizard/";

    public CGTable(XMultiServiceFactory _xMSF)
    {
        xMSF = _xMSF;
    }

    public void initialize(XNameAccess _xNameAccessParentNode, int _index)
    {
        try
        {
            xNameAccessTableNode = Configuration.getChildNodebyIndex(_xNameAccessParentNode, _index);
            xNameAccessFieldsNode = Configuration.getChildNodebyName(xNameAccessTableNode, "Fields");
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public String[] getFieldNames(boolean _bgetbyShortName, int _imaxcolumnchars)
    {
        try
        {
            String[] fieldnames = null;
            if (_bgetbyShortName)
            {
                fieldnames = Configuration.getNodeChildNames(xNameAccessFieldsNode, "ShortName");
                for (int i = 0; i < fieldnames.length; i++)
                {
                    if (fieldnames[i].length() > _imaxcolumnchars)
                    {
                        fieldnames[i] = fieldnames[i].substring(0, _imaxcolumnchars);
                    }
                }
            }
            else
            {
                fieldnames = Configuration.getNodeChildNames(xNameAccessFieldsNode, PropertyNames.PROPERTY_NAME);
            }
            for (int i = 0; i < fieldnames.length; i++)
            {
                fieldnames[i] = Desktop.removeSpecialCharacters(xMSF, Configuration.getOfficeLocale(xMSF), fieldnames[i]);
            }
            return fieldnames;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return null;
        }
    }
}

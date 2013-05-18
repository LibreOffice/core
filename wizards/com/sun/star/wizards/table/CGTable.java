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
            e.printStackTrace(System.err);
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
                fieldnames[i] = Desktop.removeSpecialCharacters(xMSF, Configuration.getLocale(xMSF), fieldnames[i]);
            }
            return fieldnames;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }
}

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
package com.sun.star.wizards.web.data;

import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.*;
import org.w3c.dom.Node;

public class CGGeneralInfo extends ConfigGroup implements XMLProvider
{

    public String cp_Title;
    public String cp_Description;
    public String cp_Author;
    public int cp_CreationDate;
    public int cp_UpdateDate;
    public String cp_Email;
    public String cp_Copyright;

    public Node createDOM(Node parent)
    {
        return XMLHelper.addElement(parent, "general-info",
                new String[]
                {
                    "title", "author", "description", "creation-date", "update-date", "email", "copyright"
                },
                new String[]
                {
                    cp_Title, cp_Author, cp_Description, String.valueOf(cp_CreationDate), String.valueOf(cp_UpdateDate), cp_Email, cp_Copyright
                });
    }

    public Integer getCreationDate()
    {
        if (cp_CreationDate == 0)
        {
            cp_CreationDate = currentDate();
        }
        return new Integer(cp_CreationDate);
    }

    public Integer getUpdateDate()
    {
        if (cp_UpdateDate == 0)
        {
            cp_UpdateDate = currentDate();
        }
        return new Integer(cp_UpdateDate);
    }

    public void setCreationDate(Integer i)
    {
        //System.out.println(i);
        cp_CreationDate = i.intValue();
    }

    public void setUpdateDate(Integer i)
    {
        cp_UpdateDate = i.intValue();
    }

    private int currentDate()
    {
        DateTime dt = JavaTools.getDateTime(System.currentTimeMillis());
        //System.out.println();
        return dt.Day + dt.Month * 100 + dt.Year * 10000;
    }
}

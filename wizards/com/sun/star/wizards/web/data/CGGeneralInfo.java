/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGGeneralInfo.java,v $
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

/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLHelper.java,v $
 *
 * $Revision: 1.3.192.1 $
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

/*
 * XMLHelper.java
 *
 * Created on 30. September 2003, 15:38
 */
package com.sun.star.wizards.common;

import org.w3c.dom.*;

/**
 *
 * @author  rpiterman
 */
public class XMLHelper
{

    public static Node addElement(Node parent, String name, String[] attNames, String[] attValues)
    {
        Document doc = parent.getOwnerDocument();
        if (doc == null)
        {
            doc = (Document) parent;
        }
        Element e = doc.createElement(name);
        for (int i = 0; i < attNames.length; i++)
        {
            if (attValues[i] != null && (!attValues[i].equals("")))
            {
                e.setAttribute(attNames[i], attValues[i]);
            }
        }
        parent.appendChild(e);
        return e;
    }

    public static Node addElement(Node parent, String name, String attNames, String attValues)
    {
        return addElement(parent, name, new String[]
                {
                    attNames
                }, new String[]
                {
                    attValues
                });
    }
}

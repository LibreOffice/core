/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGContent.java,v $
 * $Revision: 1.5 $
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

import com.sun.star.wizards.common.*;

import org.w3c.dom.*;

public class CGContent extends ConfigSetItem implements XMLProvider
{

    public String dirName;
    public String cp_Name;
    public String cp_Description;
    public ConfigSet cp_Contents = new ConfigSet(CGContent.class);
    public ConfigSet cp_Documents = new ConfigSet(CGDocument.class);

    public CGContent()
    {
        /*cp_Documents = new ConfigSet(CGDocument.class) {
        protected DefaultListModel createChildrenList() {
        return cp_Contents.getChildrenList();
        }
        };*/
    }

    public Node createDOM(Node parent)
    {

        Node myElement = XMLHelper.addElement(parent, "content",
                new String[]
                {
                    "name", "directory-name", "description", "directory"
                },
                new String[]
                {
                    cp_Name, dirName, cp_Description, dirName
                });

        cp_Documents.createDOM(myElement);

        return myElement;
    }
}

/*************************************************************************
 *
 *  $RCSfile: ScriptBrowseNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-10-09 14:37:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.script.framework.browse;

import drafts.com.sun.star.script.framework.browse.XBrowseNode;
import drafts.com.sun.star.script.framework.browse.BrowseNodeTypes;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Type;

import java.util.*;

public class ScriptBrowseNode extends PropertySet implements XBrowseNode {

    private String name;

    public String uri;
    public boolean editable = false;
    public boolean deletable = false;

    public ScriptBrowseNode(ScriptEntry entry, String location) {
        uri = "script://" + entry.getLogicalName() +
            "?function=" + entry.getLanguageName() +
            "&language=" + entry.getLanguage() +
            "&location=" + location;

        System.out.println("Creating ScriptBrowseNode: " + uri);
        name = entry.getLanguageName();

        if (!entry.getLanguage().toLowerCase().equals("java") &&
            !location.equals("document"))
        {
            editable = true;
            deletable = true;
        }

        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
        registerProperty("URI", new Type(String.class),
            (short)0, "uri");
    }

    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        return new XBrowseNode[0];
    }

    public boolean hasChildNodes() {
        return false;
    }

    public short getType() {
        return BrowseNodeTypes.SCRIPT;
    }

    public String toString() {
        return getName();
    }
}

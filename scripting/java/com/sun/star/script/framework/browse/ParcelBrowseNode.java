/*************************************************************************
 *
 *  $RCSfile: ParcelBrowseNode.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-10-15 17:18:29 $
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
import com.sun.star.uno.XComponentContext;
import com.sun.star.script.framework.provider.PathUtils;
import java.io.*;
import java.util.*;

public class ParcelBrowseNode extends PropertySet implements XBrowseNode  {

    private ParcelDescriptor pd;
    private File dir;
    private String name;
    private String location;
    private Collection browsenodes;
    private XComponentContext m_XCtx;
    public boolean deletable = false;
    public boolean editable = false;

    public ParcelBrowseNode(String name) {
        this.name = name;
        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
    }

    public ParcelBrowseNode(XComponentContext ctx, File dir) {
        this(dir.getName());
        this.dir = dir;
        this.m_XCtx = ctx;
        this.location = PathUtils.toScriptLocation(  m_XCtx, dir.getAbsolutePath() );
        this.pd = ParcelDescriptor.getParcelDescriptor(dir);
        this.deletable = true;
    }

    public ParcelBrowseNode(XComponentContext ctx,InputStream is, String name) {
        this(name);
        this.m_XCtx = ctx;
        this.location = "document";

        try {
            this.pd = new ParcelDescriptor(is);
        }
        catch (IOException ioe) {
            this.pd = null;
        }
    }

    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        if (browsenodes == null)
        {
            loadScripts();
        }
        return (XBrowseNode[])browsenodes.toArray(new XBrowseNode[0]);
    }

    public boolean hasChildNodes() {
        if (browsenodes == null)
        {
            loadScripts();
        }
        return browsenodes.size() > 0;
    }

    public short getType() {
        return BrowseNodeTypes.CONTAINER;
    }

    public String toString() {
        return getName();
    }

    private void loadScripts() {
        browsenodes = new ArrayList();

        if (pd != null) {
            ScriptEntry[] entries = pd.getScriptEntries();

            if (entries != null) {
                for (int i = 0; i < entries.length; i++) {
                    ScriptBrowseNode sbn;
                    if (dir != null) {
                        sbn = new ScriptBrowseNode(entries[i], location, dir);
                    }
                    else {
                        sbn = new ScriptBrowseNode(entries[i], location);
                    }
                    browsenodes.add(sbn);
                }
            }
        }
    }
}

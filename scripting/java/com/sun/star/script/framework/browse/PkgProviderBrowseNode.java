/*************************************************************************
 *
 *  $RCSfile: PkgProviderBrowseNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:55:51 $
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

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.log.*;
import com.sun.star.script.framework.container.*;
import com.sun.star.script.framework.browse.DialogFactory;

import java.io.*;
import java.util.*;
import javax.swing.JOptionPane;

public class PkgProviderBrowseNode extends ProviderBrowseNode
    implements XBrowseNode, XInvocation
{

    public PkgProviderBrowseNode( ScriptProvider provider, ParcelContainer container, XComponentContext xCtx ) {
        super( provider, container, xCtx );
        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor container name = " + container.getName());
        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor container path = " + container.getParcelContainerDir());
        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor, container has num parcels = " + container.getElementNames().length);
        deletable = false;
        editable  = false;
        creatable = false;

    }

    public String getName() {
        String name = "Unknown";
        if ( container != null )
        {
            name =  container.getName();
        }
        return name;
    }

}

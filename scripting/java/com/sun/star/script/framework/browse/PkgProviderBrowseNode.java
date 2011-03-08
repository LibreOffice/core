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

package com.sun.star.script.framework.browse;

import com.sun.star.uno.XComponentContext;

import com.sun.star.script.XInvocation;

import com.sun.star.script.browse.XBrowseNode;

import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.log.*;
import com.sun.star.script.framework.container.*;


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

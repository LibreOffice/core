/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

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

package com.sun.star.script.framework.browse;

import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ScriptProvider;

import com.sun.star.uno.XComponentContext;

public class PkgProviderBrowseNode extends ProviderBrowseNode {

    public PkgProviderBrowseNode(ScriptProvider provider, ParcelContainer container,
                                 XComponentContext xCtx) {

        super(provider, container, xCtx);

        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor container name = " +
                       container.getName());
        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor container path = " +
                       container.getParcelContainerDir());
        LogUtils.DEBUG("*** PkgProviderBrowseNode ctor, container has num parcels = " +
                       container.getElementNames().length);
        deletable = false;
        editable  = false;
        creatable = false;
    }

    @Override public String getName() {
        return (container != null) ? container.getName() : "Unknown";
    }

}

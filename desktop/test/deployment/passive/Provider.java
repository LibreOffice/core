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

package com.sun.star.comp.test.deployment.passive_java;

import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.URL;

public final class Provider extends WeakBase
    implements XServiceInfo, XDispatchProvider
{
    public Provider(XComponentContext context) {
        this.context = context;
    }

    public String getImplementationName() { return implementationName; }

    public boolean supportsService(String ServiceName) {
        return ServiceName.equals(getSupportedServiceNames()[0]); //TODO
    }

    public String[] getSupportedServiceNames() {
        return serviceNames;
    }

    public XDispatch queryDispatch(
        URL URL, String TargetFrameName, int SearchFlags)
    {
        return UnoRuntime.queryInterface(
            XDispatch.class,
            context.getValueByName(
                "/singletons/" +
                "com.sun.star.test.deployment.passive_java_singleton"));
    }

    public XDispatch[] queryDispatches(DispatchDescriptor[] Requests) {
        XDispatch[] s = new XDispatch[Requests.length];
        for (int i = 0; i < s.length; ++i) {
            s[i] = queryDispatch(
                Requests[i].FeatureURL, Requests[i].FrameName,
                Requests[i].SearchFlags);
        }
        return s;
    }

    private final XComponentContext context;

    static final String implementationName =
        "com.sun.star.comp.test.deployment.passive_java";

    static final String[] serviceNames = new String[] {
        "com.sun.star.test.deployment.passive_java" };
}

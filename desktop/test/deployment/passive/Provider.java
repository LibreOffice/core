/*************************************************************************
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
************************************************************************/

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

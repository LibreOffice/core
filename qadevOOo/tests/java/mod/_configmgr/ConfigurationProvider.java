/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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

package mod._configmgr;

import com.sun.star.beans.NamedValue;
import com.sun.star.uno.XComponentContext;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import java.io.PrintWriter;

public final class ConfigurationProvider extends TestCase {
    public ConfigurationProvider() {}

    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log)
    {
        // Create a non-default ConfigurationProvider instance, so that testing
        // its XComponent.dispose does not accidentally dispose the
        // DefaultProvider:
        XComponentContext ctxt = tParam.getComponentContext();
        try {
            return ProviderTestEnvironment.create(
                ctxt.getServiceManager().createInstanceWithArgumentsAndContext(
                    "com.sun.star.configuration.ConfigurationProvider",
                    new Object[] { new NamedValue("Locale", "*") },
                    ctxt));
        } catch (com.sun.star.uno.Exception e) {
            throw new RuntimeException(e);
        }
    }
}

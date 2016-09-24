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

package mod._configmgr;

import com.sun.star.beans.NamedValue;
import com.sun.star.uno.XInterface;
import lib.TestEnvironment;

final class ProviderTestEnvironment {
    private ProviderTestEnvironment() {}

    public static TestEnvironment create(Object provider) {
        TestEnvironment env = new TestEnvironment((XInterface) provider);
        env.addObjRelation(
            "XMSF.serviceNamesWithArgs",
            new String[] {
                "com.sun.star.configuration.ConfigurationAccess",
                "com.sun.star.configuration.ConfigurationUpdateAccess" });
        Object[] args = new Object[] {
            new NamedValue("nodepath", "/org.openoffice.Setup") };
        env.addObjRelation("XMSF.Args", new Object[][] { args, args });
        env.addObjRelation(
            "needArgs", "com.sun.star.configuration.ConfigurationProvider");
        return env;
    }
}

/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.uno.environments.remote;


import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.XInterface;

public interface TestIWorkAt extends XInterface {
    void syncCall() throws Throwable ;
    void asyncCall() throws Throwable ;

    void increment() throws Throwable;

    void notifyme();

    TypeInfo UNOTYPEINFO[] = {
        new MethodTypeInfo("increment", 0, 0),
        new MethodTypeInfo("notifyme", 1, 0),
        new MethodTypeInfo("syncCall", 2, 0),
        new MethodTypeInfo("asyncCall", 3, 0)
            };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

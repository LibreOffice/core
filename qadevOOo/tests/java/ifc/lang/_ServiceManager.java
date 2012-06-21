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

package ifc.lang;

import lib.MultiPropertyTest;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

public class _ServiceManager extends MultiPropertyTest {

    public void _DefaultContext() {
        XComponentContext get = null;
        boolean res = false;
        try {
            get = (XComponentContext)AnyConverter.toObject(new Type(XComponentContext.class), oObj.getPropertyValue("DefaultContext"));
            res = get != null;
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Illegal Argument Exception");
            res = false;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Property is optional and not supported");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("Unexpected Exception");
            wte.printStackTrace(log);
            res = false;
        }
        tRes.tested("DefaultContext", res);

    }

} // EOF _ServiceManager


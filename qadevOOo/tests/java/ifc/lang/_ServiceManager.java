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


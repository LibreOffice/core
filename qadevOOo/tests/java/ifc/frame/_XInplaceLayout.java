/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XInplaceLayout.java,v $
 * $Revision: 1.5 $
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

package ifc.frame;

import com.sun.star.frame.XInplaceLayout;
import lib.MultiMethodTest;

public class _XInplaceLayout extends MultiMethodTest {
    public XInplaceLayout oObj = null;

    public void _setInplaceMenuBar() {
        log.print("This method exists for inplace editing with own modules. ");
        log.println("It makes no sense to call it from Java.");
//        oObj.setInplaceMenuBar(l);
        tRes.tested("setInplaceMenuBar()", true);
    }

    public void _resetInplaceMenuBar() {
        requiredMethod("setInplaceMenuBar()");
        oObj.resetInplaceMenuBar();
        tRes.tested("resetInplaceMenuBar()", true);
    }
}

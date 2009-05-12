/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUIElementFactory.java,v $
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

package ifc.ui;

import com.sun.star.beans.PropertyValue;
import com.sun.star.ui.XUIElement;
import com.sun.star.ui.XUIElementFactory;
import lib.MultiMethodTest;

public class _XUIElementFactory extends MultiMethodTest {

    public XUIElementFactory oObj;

    public void _createUIElement() {
        boolean result = true;
        PropertyValue[] prop = new PropertyValue[0];
        try {
            XUIElement element = oObj.createUIElement("private:resource/menubar/menubar", prop);
            result = (element!= null);
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("createUIElement()", result);
    }
}

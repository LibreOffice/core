/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XAccessibleHypertextLog.java,v $
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
 *
 ************************************************************************/

package org.openoffice.java.accessibility.logging;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/** The AccessibleHypertextImpl mapps all calls to the java AccessibleHypertext
 *  interface to the corresponding methods of the UNO XAccessibleHypertext
 *  interface.
 */
public class XAccessibleHypertextLog extends XAccessibleTextLog
    implements com.sun.star.accessibility.XAccessibleHypertext {

    private com.sun.star.accessibility.XAccessibleHypertext unoObject;

    /** Creates a new instance of XAccessibleTextLog */
    public XAccessibleHypertextLog(XAccessibleHypertext xAccessibleHypertext) {
        super(xAccessibleHypertext);
        unoObject = xAccessibleHypertext;
    }

    public XAccessibleHyperlink getHyperLink(int param)
            throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getHyperLink(param);
    }

    public int getHyperLinkCount() {
        return unoObject.getHyperLinkCount();
    }

    public int getHyperLinkIndex(int param)
            throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getHyperLinkIndex(param);
    }
}

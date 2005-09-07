/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XAccessibleHypertextLog.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:44:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

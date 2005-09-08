/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XInplaceLayout.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:03:09 $
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

package ifc.frame;

import com.sun.star.awt.XPopupMenu;
import com.sun.star.uno.UnoRuntime;
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

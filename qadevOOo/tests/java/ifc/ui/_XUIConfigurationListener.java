/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIConfigurationListener.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:34:58 $
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

package ifc.ui;

import com.sun.star.awt.XPopupMenu;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.ConfigurationEvent;
import com.sun.star.ui.XUIConfigurationListener;
import lib.MultiMethodTest;

public class _XUIConfigurationListener extends MultiMethodTest {

    public XUIConfigurationListener oObj = null;

    public void _elementInserted() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementInserted(event);
        tRes.tested("elementInserted()", true);
    }
    public void _elementRemoved() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementRemoved(event);
        tRes.tested("elementRemoved()", true);
    }
    public void _elementReplaced() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementReplaced(event);
        tRes.tested("elementReplaced()", true);
    }
}

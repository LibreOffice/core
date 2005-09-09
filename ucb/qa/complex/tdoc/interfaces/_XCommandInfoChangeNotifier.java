/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCommandInfoChangeNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:07:20 $
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
package complex.tdoc.interfaces;

import com.sun.star.ucb.XCommandInfoChangeListener;
import com.sun.star.ucb.XCommandInfoChangeNotifier;
import share.LogWriter;

/**
 *
 */
public class _XCommandInfoChangeNotifier {
    public XCommandInfoChangeNotifier oObj = null;
    public LogWriter log = null;
    private CommandInfoChangeListener listener = new CommandInfoChangeListener();

    public boolean _addCommandInfoChangeListener() {
        oObj.addCommandInfoChangeListener(listener);
        return true;
    }

    public boolean _removeCommandInfoChangeListener() {
        oObj.removeCommandInfoChangeListener(listener);
        return true;
    }

    private class CommandInfoChangeListener implements XCommandInfoChangeListener {
        boolean disposing = false;
        boolean infoChanged = false;

        public void commandInfoChange(com.sun.star.ucb.CommandInfoChangeEvent commandInfoChangeEvent) {
            infoChanged = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposing = true;
        }

    }
}

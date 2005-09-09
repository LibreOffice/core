/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPropertiesChangeNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:08:23 $
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

import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertiesChangeNotifier;
import share.LogWriter;

/**
 * Check the XPropertiesChangeNotifier
 */
public class _XPropertiesChangeNotifier {
    public XPropertiesChangeNotifier oObj = null;
    public LogWriter log = null;

    PropertiesChangeListener listener = new PropertiesChangeListener();
    String[] args = null;


    public boolean _addPropertiesChangeListener() {
        oObj.addPropertiesChangeListener(args, listener);
        return true;
    }

    public boolean _removePropertiesChangeListener() {
        oObj.removePropertiesChangeListener(args, listener);
        return true;
    }

    private class PropertiesChangeListener implements XPropertiesChangeListener {
        public boolean disposed = false;
        public boolean propChanged = false;

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposed = true;
        }

        public void propertiesChange(com.sun.star.beans.PropertyChangeEvent[] propertyChangeEvent) {
            propChanged = true;
        }

    }
}

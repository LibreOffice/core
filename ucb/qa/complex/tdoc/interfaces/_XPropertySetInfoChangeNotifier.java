/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPropertySetInfoChangeNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:09:01 $
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

import com.sun.star.beans.XPropertySetInfoChangeListener;
import com.sun.star.beans.XPropertySetInfoChangeNotifier;
import share.LogWriter;

/**
 *
 */
public class _XPropertySetInfoChangeNotifier {
    public XPropertySetInfoChangeNotifier oObj = null;
    public LogWriter log = null;

    PropertySetInfoChangeListener listener = new PropertySetInfoChangeListener();
    String[] args = null;


    public boolean _addPropertiesChangeListener() {
        oObj.addPropertySetInfoChangeListener(listener);
        return true;
    }

    public boolean _removePropertiesChangeListener() {
        oObj.removePropertySetInfoChangeListener(listener);
        return true;
    }

    private class PropertySetInfoChangeListener implements XPropertySetInfoChangeListener {
        public boolean disposed = false;
        public boolean propChanged = false;

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposed = true;
        }

        public void propertySetInfoChange(com.sun.star.beans.PropertySetInfoChangeEvent propertySetInfoChangeEvent) {
        }

    }

}

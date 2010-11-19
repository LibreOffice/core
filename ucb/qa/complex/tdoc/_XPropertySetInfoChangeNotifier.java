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
package complex.tdoc;

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

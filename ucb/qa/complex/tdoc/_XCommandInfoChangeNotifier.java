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

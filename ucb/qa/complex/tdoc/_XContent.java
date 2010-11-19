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

import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentEventListener;
import com.sun.star.ucb.XContentIdentifier;
import share.LogWriter;

/**
 *
 * @author  sg128468
 */
public class _XContent {
    public XContent oObj = null;
    public LogWriter log = null;
    private ContentListener listener = null;

    public boolean _addContentEventListener() {
        listener = new ContentListener();
        oObj.addContentEventListener(listener);
        return true;
    }
    public boolean _getContentType() {
        String type = oObj.getContentType();
        log.println("Type: " + type);
        return type != null && type.indexOf("vnd.sun.star.tdoc") != -1;
    }
    public boolean _getIdentifier() {
        XContentIdentifier xIdent = oObj.getIdentifier();
        String id = xIdent.getContentIdentifier();
        String scheme = xIdent.getContentProviderScheme();
        log.println("Id: " + id);
        log.println("Scheme: " + scheme);
        return id != null && scheme != null && id.indexOf("vnd.sun.star.tdoc") != -1 && scheme.indexOf("vnd.sun.star.tdoc") != -1;
    }
    public boolean _removeContentEventListener() {
        System.out.println("Event: " + (listener.disposed || listener.firedEvent));
        oObj.removeContentEventListener(listener);
        return true;
    }


    private class ContentListener implements XContentEventListener {
        private boolean disposed = false;
        private boolean firedEvent = false;

        public void reset() {
            disposed = false;
            firedEvent = false;
        }

        public void contentEvent(com.sun.star.ucb.ContentEvent contentEvent) {
            firedEvent = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposed = true;
        }

    }
}

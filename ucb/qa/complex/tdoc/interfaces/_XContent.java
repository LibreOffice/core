/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XContent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:08:08 $
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

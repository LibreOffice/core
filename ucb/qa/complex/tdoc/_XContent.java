/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

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

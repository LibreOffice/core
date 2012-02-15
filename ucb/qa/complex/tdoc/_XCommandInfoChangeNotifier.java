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

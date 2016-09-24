/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.tdoc;

import com.sun.star.ucb.XCommandInfoChangeListener;
import com.sun.star.ucb.XCommandInfoChangeNotifier;

/**
 *
 */
public class _XCommandInfoChangeNotifier {
    public XCommandInfoChangeNotifier oObj = null;

    private final CommandInfoChangeListener listener = new CommandInfoChangeListener();

    public boolean _addCommandInfoChangeListener() {
        oObj.addCommandInfoChangeListener(listener);
        return true;
    }

    public boolean _removeCommandInfoChangeListener() {
        oObj.removeCommandInfoChangeListener(listener);
        return true;
    }

    private class CommandInfoChangeListener implements XCommandInfoChangeListener {

        public void commandInfoChange(com.sun.star.ucb.CommandInfoChangeEvent commandInfoChangeEvent) {
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

    }
}

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

package complex.framework.recovery;

import com.sun.star.awt.XWindow;
import util.UITools;

/**
 * Thread to crash the office. This thread dies after the office process
 * is no longer available.
 */
public class KlickButtonThread extends Thread {
    private final XWindow xWindow;
    private final String buttonName;

    public KlickButtonThread(XWindow xWindow, String buttonName) {
        this.xWindow = xWindow;
        this.buttonName = buttonName;
    }

    @Override
    public void run() {
        try{
            UITools oUITools = new UITools(xWindow);

            oUITools.clickButton(buttonName);

        } catch (Exception e){}
    }
}

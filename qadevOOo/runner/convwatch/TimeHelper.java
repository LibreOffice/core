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

package convwatch;

public class TimeHelper
{
    /*
      wait a second the caller don't need to handle the interruptexception
      @param _nSeconds how long should we wait
      @param _sReason  give a good reason, why we have to wait
     */
    static void waitInSeconds(int _nSeconds, String _sReason)
        {
            GlobalLogWriter.get().println("Wait " + String.valueOf(_nSeconds) + " sec. Reason: " + _sReason);
            try {
                java.lang.Thread.sleep(_nSeconds * 1000);
            } catch (java.lang.InterruptedException e2) {}
        }
}

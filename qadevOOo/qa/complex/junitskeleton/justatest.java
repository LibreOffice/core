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

package complex.junitskeleton;

public class justatest {

    public justatest() {
        System.out.println("justatest CTor.");
    }

    public void testfkt() {
        System.out.println("Test called.");
    }

    /**
     * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
     * reset</code> call.
     */
    public static void shortWait() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

}

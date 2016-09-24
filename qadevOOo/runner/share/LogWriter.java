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

package share;

public interface LogWriter {

    /**
     * Method to print
     */
    void println(String msg);

    /**
     * initialization
     *
     */
    boolean initialize(share.DescEntry entry, boolean active);

    /**
     * will mostly be used by outproducers to sum up
     * the information, maybe write them to a db
     */
    boolean summary(share.DescEntry entry);




    /**
     * Set a <CODE>Watcher</CODE> to the <CODE>LogWriter</CODE>
     * This is useful if a test starts a new office instance by itself. In this cases
     * the <CODE>LogWritter</CODE> could retrigger the <CODE>Watcher</CODE>
     * @see share.Watcher
     * @param watcher the new <CODE>Watcher</CODE>
     */
    void setWatcher(Object watcher);

}

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
package stats;

import share.LogWriter;

import java.io.PrintWriter;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class SimpleLogWriter extends PrintWriter implements LogWriter {

    private boolean m_bLogging = false;
    private share.DescEntry entry = null;
    private share.Watcher ow = null;

    public SimpleLogWriter() {
        super(System.out);
        Calendar cal = new GregorianCalendar();
        DecimalFormat dfmt = new DecimalFormat("00");
        super.println("LOG> Log started " +
            dfmt.format(cal.get(Calendar.DAY_OF_MONTH)) + "." +
            dfmt.format(cal.get(Calendar.MONTH)) + "." +
            dfmt.format(cal.get(Calendar.YEAR)) + " - " +
            dfmt.format(cal.get(Calendar.HOUR_OF_DAY)) + ":" +
            dfmt.format(cal.get(Calendar.MINUTE)) + ":" +
            dfmt.format(cal.get(Calendar.SECOND)));
        super.flush();
    }

    public boolean initialize(share.DescEntry _entry, boolean _bLogging) {
        m_bLogging = _bLogging;
        entry = _entry;

        return true;
    }

    @Override
    public void println(String msg) {
        if ((ow == null) && (entry != null))
        {
            this.ow = (share.Watcher) entry.UserDefinedParams.get("Watcher");
            if (this.ow != null)
            {
                this.ow.ping();
            }
        }
        else
        {
            if (ow != null)
            {
                this.ow.ping();
            }
            else
            {
                // special case: ow == null && entry == null
                System.out.println(msg);
            }
        }

        if (m_bLogging) {
            super.println("LOG> " + msg);
            super.flush();
        }
    }

    public boolean summary(share.DescEntry entry) {
        return true;
    }

    public void setWatcher(Object watcher)
    {
        if (watcher != null)
        {
            entry.UserDefinedParams.put("Watcher", watcher);
        }
    }
}

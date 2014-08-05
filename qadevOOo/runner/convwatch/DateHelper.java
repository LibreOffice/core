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

import java.text.SimpleDateFormat;
import java.util.GregorianCalendar;
import java.text.FieldPosition;
import java.util.Locale;

public class DateHelper
{
    public static String getDateTimeForFilename()
        {
            return getDateString("yyyyMMdd-HHmmss");
        }

    public static String getDateTimeForHumanreadableLog()
        {
            return getDateString("[yyyy/MM/dd hh:mm:ss]");
        }

    public static String getDateString(String _sFormat)
        {
            GregorianCalendar aCalendar = new GregorianCalendar();
            StringBuffer aBuf = new StringBuffer();

            Locale aLocale = new Locale("en","US");
            SimpleDateFormat aFormat = new SimpleDateFormat(_sFormat, aLocale);
            aBuf = aFormat.format(aCalendar.getTime(), aBuf, new FieldPosition(0) );
            return aBuf.toString();
        }
}

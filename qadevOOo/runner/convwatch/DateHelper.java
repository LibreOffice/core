/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DateHelper.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            // DebugHelper.writeInfo("Date: " + aBuf.toString());
            return aBuf.toString();
        }
}

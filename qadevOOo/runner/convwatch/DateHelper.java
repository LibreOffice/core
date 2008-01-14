/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DateHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:17:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package integration.forms;

import com.sun.star.form.binding.*;

public class DateValidator extends integration.forms.ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public DateValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return "empty input";

            com.sun.star.util.Date dateValue = (com.sun.star.util.Date)Value;
            if ( isDedicatedInvalidDate( dateValue ) )
                return "this is no valid date";

            if ( !isNextMonthsDate( dateValue ) )
                return "date must denote a day in the current month";
        }
        catch( java.lang.Exception e )
        {
            return "oops. What did you enter for this to happen?";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return false;

            com.sun.star.util.Date dateValue = (com.sun.star.util.Date)
                com.sun.star.uno.AnyConverter.toObject(
                    com.sun.star.util.Date.class, Value);
            if ( isDedicatedInvalidDate( dateValue ) )
                return false;

            if ( !isNextMonthsDate( dateValue ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace( System.err );
        }
        return false;
    }

    private boolean isDedicatedInvalidDate( com.sun.star.util.Date dateValue )
    {
        return ( dateValue.Day == 0 ) && ( dateValue.Month == 0 ) && ( dateValue.Year == 0 );
    }

    private boolean isNextMonthsDate( com.sun.star.util.Date dateValue )
    {
        int overallMonth = dateValue.Year * 12 + dateValue.Month - 1;

        int todaysMonth = new java.util.Date().getMonth();
        int todaysYear = new java.util.Date().getYear() + 1900;
        int todaysOverallMonth = todaysYear * 12 + todaysMonth;

        return overallMonth == todaysOverallMonth;
    }
}

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


public class DateValidator extends ControlValidator
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

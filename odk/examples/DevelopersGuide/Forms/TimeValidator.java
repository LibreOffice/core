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


public class TimeValidator extends ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public TimeValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return "empty input";

            com.sun.star.util.Time timeValue = (com.sun.star.util.Time)Value;
            if ( isInvalidTime( timeValue ) )
                return "this is no valid time";
            if ( !isFullHour( timeValue ) )
                return "time must denote a full hour";
        }
        catch( java.lang.Exception e )
        {
            return "this is no valid time";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return false;

            com.sun.star.util.Time timeValue = (com.sun.star.util.Time)
                com.sun.star.uno.AnyConverter.toObject(
                    com.sun.star.util.Time.class, Value);
            if ( isInvalidTime( timeValue ) )
                return false;
            if ( !isFullHour( timeValue ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace( System.err );
        }
        return false;
    }

    private boolean isInvalidTime( com.sun.star.util.Time timeValue )
    {
        return ( timeValue.Hours == -1 ) && ( timeValue.Minutes == -1 ) && ( timeValue.Seconds == -1 ) && ( timeValue.HundredthSeconds == -1 );
    }

    private boolean isFullHour( com.sun.star.util.Time timeValue )
    {
        return ( timeValue.Minutes == 0 ) && ( timeValue.Seconds == 0 ) && ( timeValue.HundredthSeconds == 0 );
    }
}

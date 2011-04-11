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

public class TimeValidator extends integration.forms.ControlValidator
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

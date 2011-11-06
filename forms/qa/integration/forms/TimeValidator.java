/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package integration.forms;

import com.sun.star.form.binding.*;

/**
 *
 * @author  fs@openoffice.org
 */
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

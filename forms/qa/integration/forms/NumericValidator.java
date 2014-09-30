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
package integration.forms;

public class NumericValidator extends integration.forms.ControlValidator
{

    public String explainInvalid( Object Value )
    {
        try
        {
            double value = ((Double)Value).doubleValue();
            if ( Double.compare( Double.NaN, value ) == 0 )
                return "This is NotANumber";
            if ( !isProperRange( value ) )
                return "The value must be between 0 and 100";
            if ( !isProperDigitCount( value ) )
                return "The value must have at most one decimal digit";
        }
        catch( java.lang.Exception e )
        {
            return "This is no valid number";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            double value = ((Double)Value).doubleValue();
            if ( Double.compare( Double.NaN, value ) == 0 )
                return false;
            if ( !isProperRange( value ) )
                return false;
            return isProperDigitCount( value );
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }

    private boolean isProperRange( double value)
    {
        return ( value >= 0 ) && ( value <= 100 );
    }

    private boolean isProperDigitCount( double value)
    {
        return ( Math.floor( value * 10 ) == value * 10 );
    }
}

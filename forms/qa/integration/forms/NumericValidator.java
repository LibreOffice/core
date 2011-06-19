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

public class NumericValidator extends integration.forms.ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public NumericValidator( )
    {
    }

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
            if ( !isProperDigitCount( value ) )
                return false;
            return true;
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
        return ( java.lang.Math.floor( value * 10 ) == value * 10 );
    }
}

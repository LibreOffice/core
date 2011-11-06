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

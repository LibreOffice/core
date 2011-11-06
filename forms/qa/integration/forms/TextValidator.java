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
public class TextValidator extends integration.forms.ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public TextValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            String value = (String)Value;
            if ( containsZs( value ) )
                return "No Z's allowed here";
            if ( !isProperChunks( value ) )
                return "Need 3 * n characters";
        }
        catch( java.lang.Exception e )
        {
            return "ooops. Unknown error";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            String value = (String)Value;
            if ( containsZs( value ) )
                return false;
            if ( !isProperChunks( value ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }

    private boolean isProperChunks( String value )
    {
        return ( value.length() % 3 ) == 0;
    }

    private boolean containsZs( String value )
    {
        if  (  ( value.indexOf( 'Z' ) != -1 )
            || ( value.indexOf( 'z' ) != -1 )
            )
            return true;
        return false;
    }
}

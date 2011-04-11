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

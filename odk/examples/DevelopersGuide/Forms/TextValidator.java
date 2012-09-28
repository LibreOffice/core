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



/**
 *
 * @author  fs@openoffice.org
 */
public class TextValidator extends ControlValidator
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

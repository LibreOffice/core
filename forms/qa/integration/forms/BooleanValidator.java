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

import com.sun.star.uno.AnyConverter;

public class BooleanValidator extends integration.forms.ControlValidator
{
    private final boolean                         m_preventChecked;

    /** Creates a new instance of BooleanValidator */
    public BooleanValidator( boolean preventChecked )
    {
        m_preventChecked = preventChecked;
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( AnyConverter.isVoid( Value ) )
                return "'indetermined' is not an allowed state";
            boolean value = ((Boolean)Value).booleanValue();
            if ( m_preventChecked && ( value ) )
                return "no no no. Don't check it.";
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
            if ( AnyConverter.isVoid( Value ) )
                return false;

            boolean value = ((Boolean)Value).booleanValue();
            return !(m_preventChecked && ( value ));
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }
}

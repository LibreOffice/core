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

package connectivity.tools;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.IllegalArgumentException;

public class QueryDefinition
{
    XPropertySet    m_queryDef;

    public QueryDefinition( XPropertySet _queryDef )
    {
        m_queryDef = _queryDef;
    }

    /** retrieves the command underlying the query definition
     *
     * This method is a mere wrapped around the <code>getPropertyValue( "Command" )</code> call
     */
    public final String getCommand() throws WrappedTargetException
    {
        String command = null;
        try {
            command = (String)m_queryDef.getPropertyValue( "Command" );
        }
        catch (UnknownPropertyException e) { }

        return command;
    }

    /** retrieves the command underlying the query definition
     *
     * This method is a mere wrapped around the <code>getPropertyValue( "Command" )</code> call
     */
    public void setCommand( String _command ) throws WrappedTargetException
    {
        try
        {
            m_queryDef.setPropertyValue( "Command", _command );
        }
        catch (UnknownPropertyException e) { }
        catch (PropertyVetoException e) { }
        catch (IllegalArgumentException e) { }
    }
};

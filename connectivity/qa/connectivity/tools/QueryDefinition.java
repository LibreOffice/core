/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryDefinition.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:18:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

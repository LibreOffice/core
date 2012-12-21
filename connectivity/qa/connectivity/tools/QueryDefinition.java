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
}

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



package util.db;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;

/** a descriptor for creating a com.sun.star.sdb.DataSource
 *
 * @author fs93730
 */
public class DataSourceDescriptor
{
    /**
    * Representation of <code>'Name'</code> property.
    */
    public String Name = null ;
    /**
    * Representation of <code>'URL'</code> property.
    */
    public String URL = null ;
    /**
    * Representation of <code>'Info'</code> property.
    */
    public PropertyValue[] Info = null ;
    /**
    * Representation of <code>'User'</code> property.
    */
    public String User = null ;
    /**
    * Representation of <code>'Password'</code> property.
    */
    public String Password = null ;
    /**
    * Representation of <code>'IsPasswordRequired'</code> property.
    */
    public Boolean IsPasswordRequired = null ;

    public DataSourceDescriptor( XMultiServiceFactory _orb )
    {
        m_orb = _orb;
    }

    public DataSource createDataSource()
    {
        return new DataSource( m_orb, this );
    }

    private XMultiServiceFactory m_orb = null;
}

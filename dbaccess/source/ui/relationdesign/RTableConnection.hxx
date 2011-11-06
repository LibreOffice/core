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


#ifndef DBAUI_RTABLECONNECTION_HXX
#define DBAUI_RTABLECONNECTION_HXX

#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif

namespace dbaui
{
    class ORelationTableView;
    //==================================================================
    class ORelationTableConnection : public OTableConnection
    {
    public:
        ORelationTableConnection( ORelationTableView* pContainer, const TTableConnectionData::value_type& pTabConnData );
        ORelationTableConnection( const ORelationTableConnection& rConn );
            // wichtiger Kommentar zum CopyConstructor siehe OTableConnection(const OTableConnection&)
        virtual ~ORelationTableConnection();

        ORelationTableConnection& operator=( const ORelationTableConnection& rConn );

        virtual void    Draw( const Rectangle& rRect );
        using OTableConnection::Draw;
    };
}
#endif // DBAUI_RTABLECONNECTION_HXX





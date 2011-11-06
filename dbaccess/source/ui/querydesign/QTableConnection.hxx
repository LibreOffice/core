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


#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#define DBAUI_QUERYTABLECONNECTION_HXX

#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace dbaui
{
    //==================================================================
    class OQueryTableView;
    class OQueryTableConnection : public OTableConnection
    {
        sal_Bool m_bVisited;    // is true if the conn was already visited through the join algorithm
    public:
        OQueryTableConnection(OQueryTableView* pContainer, const TTableConnectionData::value_type& pTabConnData);
        OQueryTableConnection(const OQueryTableConnection& rConn);
        virtual ~OQueryTableConnection();

        OQueryTableConnection& operator=(const OQueryTableConnection& rConn);
        sal_Bool operator==(const OQueryTableConnection& rCompare);

        inline ::rtl::OUString  GetAliasName(EConnectionSide nWhich) const { return static_cast<OQueryTableConnectionData*>(GetData().get())->GetAliasName(nWhich); }

        inline sal_Bool IsVisited() const               { return m_bVisited; }
        inline void     SetVisited(sal_Bool bVisited)   { m_bVisited = bVisited; }

    };
}
#endif // DBAUI_QUERYTABLECONNECTION_HXX

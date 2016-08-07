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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
using namespace dbaui;
//========================================================================
// class OQueryTableConnection
//========================================================================
DBG_NAME(OQueryTableConnection)

//------------------------------------------------------------------------
OQueryTableConnection::OQueryTableConnection(OQueryTableView* pContainer, const TTableConnectionData::value_type& pTabConnData)
    :OTableConnection(pContainer, pTabConnData)
    ,m_bVisited(sal_False)
{
    DBG_CTOR(OQueryTableConnection,NULL);
}


//------------------------------------------------------------------------
OQueryTableConnection::OQueryTableConnection(const OQueryTableConnection& rConn)
    :OTableConnection( rConn )
{
    DBG_CTOR(OQueryTableConnection,NULL);
    // keine eigenen Members, also reicht die Basisklassenfunktionalitaet
}
//------------------------------------------------------------------------
OQueryTableConnection::~OQueryTableConnection()
{
    DBG_DTOR(OQueryTableConnection,NULL);
}

//------------------------------------------------------------------------
OQueryTableConnection& OQueryTableConnection::operator=(const OQueryTableConnection& rConn)
{
    if (&rConn == this)
        return *this;

    OTableConnection::operator=(rConn);
    // keine eigenen Members ...
    return *this;
}

//------------------------------------------------------------------------
sal_Bool OQueryTableConnection::operator==(const OQueryTableConnection& rCompare)
{
    DBG_ASSERT(GetData() && rCompare.GetData(), "OQueryTableConnection::operator== : einer der beiden Teilnehmer hat keine Daten !");

    // allzuviel brauche ich nicht vergleichen (schon gar nicht alle Member) : lediglich die Fenster, an denen wir haengen, und
    // die Indizies in der entsprechenden Tabelle muessen uebereinstimmen
    OQueryTableConnectionData* pMyData = static_cast<OQueryTableConnectionData*>(GetData().get());
    OQueryTableConnectionData* pCompData = static_cast<OQueryTableConnectionData*>(rCompare.GetData().get());

    // Connections werden als gleich angesehen, wenn sie in Source-/Dest-Fenstername und Source-/Dest-FieldIndex uebereinstimmen ...
    return  (   (   (pMyData->getReferencedTable() == pCompData->getReferencedTable()) &&
                    (pMyData->getReferencingTable() == pCompData->getReferencingTable()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_TO)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_FROM))
                )
                ||  // ... oder diese Uebereinstimmung ueber Kreuz besteht
                (   (pMyData->getReferencingTable() == pCompData->getReferencedTable()) &&
                    (pMyData->getReferencedTable() == pCompData->getReferencingTable()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_FROM)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_TO))
                )
            );
}
// -----------------------------------------------------------------------------







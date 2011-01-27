/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "QTableConnection.hxx"
#include <tools/debug.hxx>
#include "QueryTableView.hxx"
#include "ConnectionLine.hxx"
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

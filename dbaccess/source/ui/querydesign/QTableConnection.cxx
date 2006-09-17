/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QTableConnection.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:21:16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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


TYPEINIT1(OQueryTableConnection, OTableConnection);

//========================================================================
// class OQueryTableConnection
//========================================================================
DBG_NAME(OQueryTableConnection);

//------------------------------------------------------------------------
OQueryTableConnection::OQueryTableConnection(OQueryTableView* pContainer, OQueryTableConnectionData* pTabConnData)
    :OTableConnection(pContainer, pTabConnData)
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
OConnectionLine* OQueryTableConnection::CreateConnLine( const OConnectionLine& rConnLine )
{
    // keine Spezialisierung bei den ConnectionLines, also eine Instanz der Standard-Klasse
    return new OConnectionLine(rConnLine);
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
    OQueryTableConnectionData* pMyData = (OQueryTableConnectionData*)GetData();
    OQueryTableConnectionData* pCompData = (OQueryTableConnectionData*)rCompare.GetData();

    // Connections werden als gleich angesehen, wenn sie in Source-/Dest-Fenstername und Source-/Dest-FieldIndex uebereinstimmen ...
    return  (   (   (pMyData->GetSourceWinName() == pCompData->GetSourceWinName()) &&
                    (pMyData->GetDestWinName() == pCompData->GetDestWinName()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_TO)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_FROM))
                )
                ||  // ... oder diese Uebereinstimmung ueber Kreuz besteht
                (   (pMyData->GetSourceWinName() == pCompData->GetDestWinName()) &&
                    (pMyData->GetDestWinName() == pCompData->GetSourceWinName()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_FROM)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_TO))
                )
            );
}
// -----------------------------------------------------------------------------







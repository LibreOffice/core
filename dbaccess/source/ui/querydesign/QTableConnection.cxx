/*************************************************************************
 *
 *  $RCSfile: QTableConnection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:18:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
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







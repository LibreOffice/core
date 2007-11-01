/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QTableConnection.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:29:45 $
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

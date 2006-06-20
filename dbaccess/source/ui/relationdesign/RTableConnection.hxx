/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RTableConnection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:30:43 $
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
    protected:
        OConnectionLine* CreateConnLine( const OConnectionLine& rConnLine );

    public:
        ORelationTableConnection( ORelationTableView* pContainer, ORelationTableConnectionData* pTabConnData );
        ORelationTableConnection( const ORelationTableConnection& rConn );
            // wichtiger Kommentar zum CopyConstructor siehe OTableConnection(const OTableConnection&)
        virtual ~ORelationTableConnection();

        ORelationTableConnection& operator=( const ORelationTableConnection& rConn );

        virtual void    Draw( const Rectangle& rRect );
        using OTableConnection::Draw;
    };
}
#endif // DBAUI_RTABLECONNECTION_HXX





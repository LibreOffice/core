/*************************************************************************
 *
 *  $RCSfile: TableConnectionData.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:05:37 $
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
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#define DBAUI_TABLECONNECTIONDATA_HXX

#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace dbaui
{
#define MAX_CONN_COUNT 2
    //==================================================================
    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine
    //==================================================================


    //==================================================================
    /*
        the class OTableConnectionData contains all connectiondata which exists between two windows
    **/
    class OTableConnectionData
    {
    protected:
        String m_aSourceWinName;
        String m_aDestWinName;
        String m_aConnName;

        ::std::vector<OConnectionLineData*> m_vConnLineData;

        void    Init();
        void    Init(const String& rSourceWinName, const String& rDestWinName, const String& rConnName = String() );
            // Das erste Init baut darauf, dass die 3 String-Members schon korrekt gesetzt sind und aConnLineDataList leer ist.
            // Das zweite stellt genau diesen Zustand her

        virtual OConnectionLineData* CreateLineDataObj();
        virtual OConnectionLineData* CreateLineDataObj( const OConnectionLineData& rConnLineData );

    public:
        TYPEINFO();
        OTableConnectionData();
        OTableConnectionData( const String& rSourceWinName, const String& rDestWinName, const String& rConnName = String() );
        OTableConnectionData( const OTableConnectionData& rConnData );
        virtual ~OTableConnectionData();

        // sich aus einer Quelle initialisieren (das ist mir irgendwie angenehmer als ein virtueller Zuweisungsoperator)
        virtual void CopyFrom(const OTableConnectionData& rSource);

        // eine neue Instanz meines eigenen Typs liefern (braucht NICHT initialisiert sein)
        virtual OTableConnectionData* NewInstance() const;
            // (von OTableConnectionData abgeleitete Klasse muessen entsprechend eine Instanz ihrer Klasse liefern)

        OTableConnectionData& operator=( const OTableConnectionData& rConnData );

        BOOL SetConnLine( USHORT nIndex, const String& rSourceFieldName, const String& rDestFieldName );
        BOOL AppendConnLine( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName );
        void ResetConnLines( BOOL bUseDefaults = TRUE );
            // loescht die Liste der ConnLines, bei bUseDefaults == TRUE werden danach MAX_CONN_COUNT neue Dummy-Linien eingefuegt

        ::std::vector<OConnectionLineData*>* GetConnLineDataList(){ return &m_vConnLineData; }

        String GetSourceWinName() const { return m_aSourceWinName; }
        String GetDestWinName() const { return m_aDestWinName; }
        String GetConnName() const { return m_aConnName; }

        virtual void SetSourceWinName( const String& rSourceWinName ){ m_aSourceWinName = rSourceWinName; }
        virtual void SetDestWinName( const String& rDestWinName ){ m_aDestWinName = rDestWinName; }
        virtual void SetConnName( const String& rConnName ){ m_aConnName = rConnName; }
    };

}
#endif // DBAUI_TABLECONNECTIONDATA_HXX



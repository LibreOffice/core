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
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#define DBAUI_TABLECONNECTIONDATA_HXX

#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#include "TableWindowData.hxx"
#include <vector>
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#include <boost/shared_ptr.hpp>

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
        the class OTableConnectionData contains all connection data which exists between    two windows
    **/
    class OTableConnectionData
    {

    protected:
        TTableWindowData::value_type m_pReferencingTable;
        TTableWindowData::value_type m_pReferencedTable;
        String m_aConnName;

        OConnectionLineDataVec m_vConnLineData;

        void    Init();

        virtual OConnectionLineDataRef CreateLineDataObj();
        virtual OConnectionLineDataRef CreateLineDataObj( const OConnectionLineData& rConnLineData );

        OTableConnectionData& operator=( const OTableConnectionData& rConnData );
    public:
        OTableConnectionData();
        OTableConnectionData(const TTableWindowData::value_type& _pReferencingTable,const TTableWindowData::value_type& _pReferencedTable, const String& rConnName = String() );
        OTableConnectionData( const OTableConnectionData& rConnData );
        virtual ~OTableConnectionData();

        // sich aus einer Quelle initialisieren (das ist mir irgendwie angenehmer als ein virtueller Zuweisungsoperator)
        virtual void CopyFrom(const OTableConnectionData& rSource);

        // eine neue Instanz meines eigenen Typs liefern (braucht NICHT initialisiert sein)
        virtual OTableConnectionData* NewInstance() const;
            // (von OTableConnectionData abgeleitete Klasse muessen entsprechend eine Instanz ihrer Klasse liefern)

        sal_Bool SetConnLine( sal_uInt16 nIndex, const String& rSourceFieldName, const String& rDestFieldName );
        sal_Bool AppendConnLine( const ::rtl::OUString& rSourceFieldName, const ::rtl::OUString& rDestFieldName );
        void ResetConnLines( sal_Bool bUseDefaults = sal_True );

        /** normalizeLines moves the empty lines to the back
        */
        void normalizeLines();
            // loescht die Liste der ConnLines, bei bUseDefaults == sal_True werden danach MAX_CONN_COUNT neue Dummy-Linien eingefuegt

        OConnectionLineDataVec* GetConnLineDataList(){ return &m_vConnLineData; }

        inline TTableWindowData::value_type getReferencingTable() const { return m_pReferencingTable; }
        inline TTableWindowData::value_type getReferencedTable()  const { return m_pReferencedTable;  }

        inline void setReferencingTable(const TTableWindowData::value_type& _pTable) { m_pReferencingTable = _pTable; }
        inline void setReferencedTable(const TTableWindowData::value_type& _pTable)  { m_pReferencedTable  = _pTable; }

        String GetConnName() const { return m_aConnName; }

        virtual void SetConnName( const String& rConnName ){ m_aConnName = rConnName; }
        /** Update create a new connection

            @return true if successful
        */
        virtual sal_Bool Update(){ return sal_True; }
    };

    typedef ::std::vector< ::boost::shared_ptr<OTableConnectionData> >  TTableConnectionData;

}
#endif // DBAUI_TABLECONNECTIONDATA_HXX



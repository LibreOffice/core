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


#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#define DBAUI_TABLECONNECTIONDATA_HXX

#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#include "TableWindowData.hxx"
#include <vector>
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



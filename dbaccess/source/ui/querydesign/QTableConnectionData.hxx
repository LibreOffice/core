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


#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#define DBAUI_QTABLECONNECTIONDATA_HXX

#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
//#ifndef DBAUI_QUERYCONTROLLER_HXX
//#include "querycontroller.hxx"
//#endif

namespace dbaui
{
    class OQueryTableConnectionData : public OTableConnectionData
    {
        sal_Int32       m_nFromEntryIndex;
        sal_Int32       m_nDestEntryIndex;
        EJoinType       m_eJoinType;
        bool            m_bNatural;

        ETableFieldType m_eFromType;
        ETableFieldType m_eDestType;

    protected:
        // fuer das Anlegen und Duplizieren von Lines vom eigenen Typ
        virtual OConnectionLineDataRef CreateLineDataObj();
        virtual OConnectionLineDataRef CreateLineDataObj( const OConnectionLineData& rConnLineData );

        OQueryTableConnectionData& operator=( const OQueryTableConnectionData& rConnData );
    public:
        OQueryTableConnectionData();
        OQueryTableConnectionData( const OQueryTableConnectionData& rConnData );
        OQueryTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,const TTableWindowData::value_type& _pReferencedTable,
            const ::rtl::OUString& rConnName=::rtl::OUString());
        virtual ~OQueryTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource);
        virtual OTableConnectionData* NewInstance() const;


        /** Update create a new connection

            @return true if successful
        */
        virtual sal_Bool Update();

        ::rtl::OUString GetAliasName(EConnectionSide nWhich) const;

        sal_Int32       GetFieldIndex(EConnectionSide nWhich) const { return nWhich==JTCS_TO ? m_nDestEntryIndex : m_nFromEntryIndex; }
        void            SetFieldIndex(EConnectionSide nWhich, sal_Int32 nVal) { if (nWhich==JTCS_TO) m_nDestEntryIndex=nVal; else m_nFromEntryIndex=nVal; }

        ETableFieldType GetFieldType(EConnectionSide nWhich) const { return nWhich==JTCS_TO ? m_eDestType : m_eFromType; }
        void            SetFieldType(EConnectionSide nWhich, ETableFieldType eType) { if (nWhich==JTCS_TO) m_eDestType=eType; else m_eFromType=eType; }

        void            InitFromDrag(const OTableFieldDescRef& rDragLeft, const OTableFieldDescRef& rDragRight);

        EJoinType       GetJoinType() const { return m_eJoinType; };
        void            SetJoinType(const EJoinType& eJT) { m_eJoinType = eJT; };

        inline void setNatural(bool _bNatural) { m_bNatural = _bNatural; }
        inline bool isNatural() const { return m_bNatural; }
    };

}
#endif // DBAUI_QTABLECONNECTIONDATA_HXX

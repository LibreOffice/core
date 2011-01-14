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
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
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

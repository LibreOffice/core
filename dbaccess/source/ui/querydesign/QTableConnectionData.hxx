/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QTableConnectionData.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:30:14 $
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
        virtual BOOL Update();

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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RTableConnectionData.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:17:02 $
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
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#define DBAUI_RTABLECONNECTIONDATA_HXX

#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace dbaui
{
    const UINT16 CARDINAL_UNDEFINED = 0x0000;
    const UINT16 CARDINAL_ONE_MANY  = 0x0001;
    const UINT16 CARDINAL_MANY_ONE  = 0x0002;
    const UINT16 CARDINAL_ONE_ONE   = 0x0004;

    class OConnectionLineData;
    //==================================================================
    class ORelationTableConnectionData :    public OTableConnectionData
    {
        friend bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs);
        friend bool operator!=(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs) { return !(lhs == rhs); }

        ::osl::Mutex    m_aMutex;
        ::rtl::OUString m_sDatabaseName;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        sal_Int32 m_nCardinality;

        BOOL checkPrimaryKey(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable,EConnectionSide _eEConnectionSide) const;
        BOOL IsSourcePrimKey()  const { return checkPrimaryKey(getReferencingTable()->getTable(),JTCS_FROM);    }
        BOOL IsDestPrimKey()    const { return checkPrimaryKey(getReferencedTable()->getTable(),JTCS_TO);       }

    protected:
        virtual OConnectionLineDataRef CreateLineDataObj();
        virtual OConnectionLineDataRef CreateLineDataObj( const OConnectionLineData& rConnLineData );

        ORelationTableConnectionData& operator=( const ORelationTableConnectionData& rConnData );
    public:
        ORelationTableConnectionData();
        ORelationTableConnectionData( const ORelationTableConnectionData& rConnData );
        ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                      const TTableWindowData::value_type& _pReferencedTable,
                                      const ::rtl::OUString& rConnName = ::rtl::OUString() );
        virtual ~ORelationTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource);
        virtual OTableConnectionData* NewInstance() const { return new ORelationTableConnectionData(); }

        inline ::rtl::OUString GetDatabaseName() const { return m_sDatabaseName; }

        /** Update create a new relation

            @return true if successful
        */
        virtual BOOL Update();


        void        SetCardinality();
        inline void SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        inline void SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        inline sal_Int32    GetUpdateRules() const { return m_nUpdateRules; }
        inline sal_Int32    GetDeleteRules() const { return m_nDeleteRules; }
        inline sal_Int32    GetCardinality() const { return m_nCardinality; }

        BOOL        IsConnectionPossible();
        void        ChangeOrientation();
        BOOL        DropRelation();
    };
}

#endif // DBAUI_RTABLECONNECTIONDATA_HXX




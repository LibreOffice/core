/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#define DBAUI_RTABLECONNECTIONDATA_HXX

#include "TableConnectionData.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "QEnumTypes.hxx"

namespace dbaui
{
    const sal_uInt16 CARDINAL_UNDEFINED = 0x0000;
    const sal_uInt16 CARDINAL_ONE_MANY  = 0x0001;
    const sal_uInt16 CARDINAL_MANY_ONE  = 0x0002;
    const sal_uInt16 CARDINAL_ONE_ONE   = 0x0004;

    class OConnectionLineData;
    //==================================================================
    class ORelationTableConnectionData :    public OTableConnectionData
    {
        friend bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs);
        friend bool operator!=(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs) { return !(lhs == rhs); }

        ::osl::Mutex    m_aMutex;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        sal_Int32 m_nCardinality;

        sal_Bool checkPrimaryKey(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& i_xTable,EConnectionSide _eEConnectionSide) const;
        sal_Bool IsSourcePrimKey()  const { return checkPrimaryKey(getReferencingTable()->getTable(),JTCS_FROM);    }
        sal_Bool IsDestPrimKey()    const { return checkPrimaryKey(getReferencedTable()->getTable(),JTCS_TO);       }

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

        /** Update create a new relation

            @return true if successful
        */
        virtual sal_Bool Update();


        void        SetCardinality();
        inline void SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        inline void SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        inline sal_Int32    GetUpdateRules() const { return m_nUpdateRules; }
        inline sal_Int32    GetDeleteRules() const { return m_nDeleteRules; }
        inline sal_Int32    GetCardinality() const { return m_nCardinality; }

        sal_Bool        IsConnectionPossible();
        void        ChangeOrientation();
        sal_Bool        DropRelation();
    };
}

#endif // DBAUI_RTABLECONNECTIONDATA_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

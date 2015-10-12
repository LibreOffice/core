/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RTABLECONNECTIONDATA_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RTABLECONNECTIONDATA_HXX

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
    class ORelationTableConnectionData :    public OTableConnectionData
    {
        friend bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs);

        ::osl::Mutex    m_aMutex;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        sal_Int32 m_nCardinality;

        bool checkPrimaryKey(const css::uno::Reference< css::beans::XPropertySet>& i_xTable, EConnectionSide _eEConnectionSide) const;
        bool IsSourcePrimKey()  const { return checkPrimaryKey(getReferencingTable()->getTable(),JTCS_FROM);    }
        bool IsDestPrimKey()    const { return checkPrimaryKey(getReferencedTable()->getTable(),JTCS_TO);       }

    protected:
        virtual OConnectionLineDataRef CreateLineDataObj() override;

        ORelationTableConnectionData& operator=( const ORelationTableConnectionData& rConnData );
    public:
        ORelationTableConnectionData();
        ORelationTableConnectionData( const ORelationTableConnectionData& rConnData );
        ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                      const TTableWindowData::value_type& _pReferencedTable,
                                      const OUString& rConnName = OUString() );
        virtual ~ORelationTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource) override;
        virtual OTableConnectionData* NewInstance() const override { return new ORelationTableConnectionData(); }

        /** Update create a new relation

            @return true if successful
        */
        virtual bool Update() override;

        void        SetCardinality();
        inline void SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        inline void SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        inline sal_Int32    GetUpdateRules() const { return m_nUpdateRules; }
        inline sal_Int32    GetDeleteRules() const { return m_nDeleteRules; }
        inline sal_Int32    GetCardinality() const { return m_nCardinality; }

        bool        IsConnectionPossible();
        void        ChangeOrientation();
        bool        DropRelation();
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_RTABLECONNECTIONDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

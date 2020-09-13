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
#pragma once

#include "TableConnectionData.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "QEnumTypes.hxx"

namespace dbaui
{
    enum class Cardinality {
        Undefined, OneMany, ManyOne, OneOne
    };

    class OConnectionLineData;
    class ORelationTableConnectionData final : public OTableConnectionData
    {
        friend bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs);

        ::osl::Mutex    m_aMutex;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        Cardinality m_nCardinality;

        bool checkPrimaryKey(const css::uno::Reference< css::beans::XPropertySet>& i_xTable, EConnectionSide _eEConnectionSide) const;
        bool IsSourcePrimKey()  const { return checkPrimaryKey(getReferencingTable()->getTable(),JTCS_FROM);    }
        bool IsDestPrimKey()    const { return checkPrimaryKey(getReferencedTable()->getTable(),JTCS_TO);       }

        ORelationTableConnectionData& operator=( const ORelationTableConnectionData& rConnData );
    public:
        ORelationTableConnectionData();
        ORelationTableConnectionData( const ORelationTableConnectionData& rConnData );
        ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                      const TTableWindowData::value_type& _pReferencedTable,
                                      const OUString& rConnName = OUString() );
        virtual ~ORelationTableConnectionData() override;

        virtual void CopyFrom(const OTableConnectionData& rSource) override;
        virtual std::shared_ptr<OTableConnectionData> NewInstance() const override { return std::make_shared<ORelationTableConnectionData>(); }

        /** Update create a new relation

            @return true if successful
        */
        virtual bool Update() override;

        void        SetCardinality();
        void SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        void SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        sal_Int32    GetUpdateRules() const { return m_nUpdateRules; }
        sal_Int32    GetDeleteRules() const { return m_nDeleteRules; }
        Cardinality  GetCardinality() const { return m_nCardinality; }

        void        IsConnectionPossible();
        void        ChangeOrientation();
        void        DropRelation();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

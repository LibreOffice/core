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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QTABLECONNECTIONDATA_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QTABLECONNECTIONDATA_HXX

#include "TableConnectionData.hxx"
#include "TableFieldDescription.hxx"
#include "QEnumTypes.hxx"
#include <tools/rtti.hxx>

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
        // for creation and duplication of lines of own type
        virtual OConnectionLineDataRef CreateLineDataObj() SAL_OVERRIDE;

        OQueryTableConnectionData& operator=( const OQueryTableConnectionData& rConnData );
    public:
        OQueryTableConnectionData();
        OQueryTableConnectionData( const OQueryTableConnectionData& rConnData );
        OQueryTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,const TTableWindowData::value_type& _pReferencedTable,
            const OUString& rConnName=OUString());
        virtual ~OQueryTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource) SAL_OVERRIDE;
        virtual OTableConnectionData* NewInstance() const SAL_OVERRIDE;


        /** Update create a new connection

            @return true if successful
        */
        virtual bool Update() SAL_OVERRIDE;

        OUString GetAliasName(EConnectionSide nWhich) const;

        sal_Int32       GetFieldIndex(EConnectionSide nWhich) const { return nWhich==JTCS_TO ? m_nDestEntryIndex : m_nFromEntryIndex; }
        void            SetFieldIndex(EConnectionSide nWhich, sal_Int32 nVal) { if (nWhich==JTCS_TO) m_nDestEntryIndex=nVal; else m_nFromEntryIndex=nVal; }

        void            SetFieldType(EConnectionSide nWhich, ETableFieldType eType) { if (nWhich==JTCS_TO) m_eDestType=eType; else m_eFromType=eType; }

        void            InitFromDrag(const OTableFieldDescRef& rDragLeft, const OTableFieldDescRef& rDragRight);

        EJoinType       GetJoinType() const { return m_eJoinType; };
        void            SetJoinType(const EJoinType& eJT) { m_eJoinType = eJT; };

        inline void setNatural(bool _bNatural) { m_bNatural = _bNatural; }
        inline bool isNatural() const { return m_bNatural; }
    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QTABLECONNECTIONDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

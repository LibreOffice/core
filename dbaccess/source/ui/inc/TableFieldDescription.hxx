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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEFIELDDESCRIPTION_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEFIELDDESCRIPTION_HXX

#include <vector>

#include "QEnumTypes.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ref.hxx>
#include <vcl/vclptr.hxx>

#include <salhelper/simplereferenceobject.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

namespace vcl { class Window; }

namespace dbaui
{
    class OTableFieldDesc : public ::salhelper::SimpleReferenceObject
    {
    private:
       std::vector< OUString >
                                    m_aCriteria;

        OUString             m_aTableName;
        OUString             m_aAliasName;   ///< table range
        OUString             m_aFieldName;   ///< column
        OUString             m_aFieldAlias;  ///< column alias
        OUString             m_aFunctionName;///< contains the function name (only if m_eFunctionType != FKT_NONE)

        VclPtr<vcl::Window>         m_pTabWindow;

        sal_Int32                   m_eDataType;
        sal_Int32                   m_eFunctionType;
        ETableFieldType             m_eFieldType;
        EOrderDir                   m_eOrderDir;
        sal_Int32                   m_nIndex;
        sal_Int32                   m_nColWidth;
        sal_uInt16                  m_nColumnId;
        bool                    m_bGroupBy;
        bool                    m_bVisible;

        // when adding new fields, please take care of IsEmpty!

    public:
        OTableFieldDesc();
        OTableFieldDesc(const OUString& rTable, const OUString& rField );
        OTableFieldDesc(const OTableFieldDesc& rRS);
        virtual ~OTableFieldDesc() override;

        inline bool IsEmpty() const;

        OTableFieldDesc& operator=( const OTableFieldDesc& _aField );

        bool IsVisible() const { return m_bVisible;}
        bool IsGroupBy() const { return m_bGroupBy;}

        void SetVisible( bool bVis=true ) { m_bVisible = bVis; }
        void SetGroupBy( bool bGb ) { m_bGroupBy = bGb; }
        void SetTabWindow( vcl::Window* pWin ){ m_pTabWindow = pWin; }
        void SetField( const OUString& rF ) { m_aFieldName = rF; }
        void SetFieldAlias( const OUString& rF ) { m_aFieldAlias = rF; }
        void SetTable( const OUString& rT ) { m_aTableName = rT; }
        void SetAlias( const OUString& rT ) { m_aAliasName = rT; }
        void SetFunction( const OUString& rT ) { m_aFunctionName = rT; }
        void SetOrderDir( EOrderDir eDir ) { m_eOrderDir = eDir; }
        void SetDataType( sal_Int32 eTyp ) { m_eDataType = eTyp; }
        void SetFieldType( ETableFieldType eTyp ) { m_eFieldType = eTyp; }
        void SetCriteria( sal_uInt16 nIdx, const OUString& rCrit );
        void SetColWidth( sal_Int32 nWidth ) { m_nColWidth = nWidth; }
        void SetFieldIndex( sal_Int32 nFieldIndex ) { m_nIndex = nFieldIndex; }
        void SetFunctionType( sal_Int32 eTyp ) { m_eFunctionType = eTyp; }
        void SetColumnId(sal_uInt16 _nColumnId) { m_nColumnId = _nColumnId; }

        const OUString& GetField() const { return m_aFieldName;}
        const OUString& GetFieldAlias() const { return m_aFieldAlias;}
        const OUString& GetTable() const { return m_aTableName;}
        const OUString& GetAlias() const { return m_aAliasName;}
        const OUString& GetFunction() const { return m_aFunctionName;}
        sal_Int32 GetDataType() const { return m_eDataType; }
        ETableFieldType GetFieldType() const { return m_eFieldType; }
        EOrderDir GetOrderDir() const { return m_eOrderDir; }
        OUString GetCriteria( sal_uInt16 nIdx ) const;
        sal_Int32 GetColWidth() const { return m_nColWidth; }
        sal_Int32 GetFieldIndex() const { return m_nIndex; }
        vcl::Window* GetTabWindow() const { return m_pTabWindow;}
        sal_Int32 GetFunctionType() const { return m_eFunctionType; }
        sal_uInt16 GetColumnId() const { return m_nColumnId;}

        bool isAggregateFunction() const { return (m_eFunctionType & FKT_AGGREGATE) == FKT_AGGREGATE; }
        bool isOtherFunction() const { return (m_eFunctionType & FKT_OTHER) == FKT_OTHER; }
        bool isNumeric() const { return (m_eFunctionType & FKT_NUMERIC) == FKT_NUMERIC; }
        bool isNoneFunction() const { return m_eFunctionType == FKT_NONE; }
        bool isCondition() const { return (m_eFunctionType & FKT_CONDITION) == FKT_CONDITION;  }
        bool isNumericOrAggregateFunction() const { return isNumeric() || isAggregateFunction(); }

        bool HasCriteria() const
        {
            for (auto const& criteria : m_aCriteria)
            {
                if(!criteria.isEmpty())
                    return true;
            }
            return false;
        }

        const std::vector< OUString>&  GetCriteria() const { return m_aCriteria; }

        void Load( const css::beans::PropertyValue& i_rSettings, const bool i_bIncludingCriteria );
        void Save( ::comphelper::NamedValueCollection& o_rSettings, const bool i_bIncludingCriteria );
    };

    inline bool OTableFieldDesc::IsEmpty()  const
    {
        bool bEmpty = (m_aTableName.isEmpty()    &&
                           m_aAliasName.isEmpty()    &&
                           m_aFieldName.isEmpty()    &&
                           m_aFieldAlias.isEmpty()   &&
                           m_aFunctionName.isEmpty() &&
                          !HasCriteria());
        return bEmpty;
    }

    typedef ::rtl::Reference< OTableFieldDesc> OTableFieldDescRef;
    typedef std::vector<OTableFieldDescRef> OTableFields;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

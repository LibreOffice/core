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

#include "TableRow.hxx"
#include <tools/debug.hxx>
#include "FieldDescriptions.hxx"
#include <algorithm>
#include <comphelper/types.hxx>
#include <tools/string.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

// class OTableRow
DBG_NAME(OTableRow)
OTableRow::OTableRow()
    :m_pActFieldDescr( NULL )
    ,m_nPos( -1 )
    ,m_bReadOnly( false )
    ,m_bOwnsDescriptions(false)
{
    DBG_CTOR(OTableRow,NULL);
}

OTableRow::OTableRow(const Reference< XPropertySet >& xAffectedCol)
    :m_pActFieldDescr( NULL )
    ,m_nPos( -1 )
    ,m_bReadOnly( false )
    ,m_bOwnsDescriptions(true)
{
    DBG_CTOR(OTableRow,NULL);
    m_pActFieldDescr = new OFieldDescription(xAffectedCol);
}

OTableRow::OTableRow( const OTableRow& rRow, long nPosition )
    :m_pActFieldDescr(NULL)
    ,m_nPos( nPosition )
    ,m_bReadOnly(rRow.IsReadOnly())
    ,m_bOwnsDescriptions(false)
{
    DBG_CTOR(OTableRow,NULL);

    OFieldDescription* pSrcField = rRow.GetActFieldDescr();
    if(pSrcField)
    {
        m_pActFieldDescr = new OFieldDescription(*pSrcField);
        m_bOwnsDescriptions = true;
    }
}

OTableRow::~OTableRow()
{
    DBG_DTOR(OTableRow,NULL);
    if(m_bOwnsDescriptions)
        delete m_pActFieldDescr;
}

void OTableRow::SetPrimaryKey( sal_Bool bSet )
{
    DBG_CHKTHIS(OTableRow,NULL);
    if(m_pActFieldDescr)
        m_pActFieldDescr->SetPrimaryKey(bSet);
}

sal_Bool OTableRow::IsPrimaryKey() const
{
    DBG_CHKTHIS(OTableRow,NULL);
    return m_pActFieldDescr && m_pActFieldDescr->IsPrimaryKey();
}

void OTableRow::SetFieldType( const TOTypeInfoSP& _pType, sal_Bool _bForce )
{
    DBG_CHKTHIS(OTableRow,NULL);
    if ( _pType.get() )
    {
        if( !m_pActFieldDescr )
        {
            m_pActFieldDescr = new OFieldDescription();
            m_bOwnsDescriptions = true;
        }
        m_pActFieldDescr->FillFromTypeInfo(_pType,_bForce,sal_True);
    }
    else
    {
        delete m_pActFieldDescr;
        m_pActFieldDescr = NULL;
    }
}

namespace dbaui
{
    SvStream& operator<<( SvStream& _rStr, const OTableRow& _rRow )
    {
        _rStr << _rRow.m_nPos;
        OFieldDescription* pFieldDesc = _rRow.GetActFieldDescr();
        if(pFieldDesc)
        {
            _rStr << (sal_Int32)1;
            _rStr.WriteUniOrByteString(pFieldDesc->GetName(), _rStr.GetStreamCharSet());
            _rStr.WriteUniOrByteString(pFieldDesc->GetDescription(), _rStr.GetStreamCharSet());
            _rStr.WriteUniOrByteString(pFieldDesc->GetHelpText(), _rStr.GetStreamCharSet());
            double nValue = 0.0;
            Any aValue = pFieldDesc->GetControlDefault();
            if ( aValue >>= nValue )
            {
                _rStr << sal_Int32(1);
                _rStr << nValue;
            }
            else
            {
                _rStr << sal_Int32(2);
                _rStr.WriteUniOrByteString(::comphelper::getString(aValue), _rStr.GetStreamCharSet());
            }

            _rStr << pFieldDesc->GetType();

            _rStr << pFieldDesc->GetPrecision();
            _rStr << pFieldDesc->GetScale();
            _rStr << pFieldDesc->GetIsNullable();
            _rStr << pFieldDesc->GetFormatKey();
            _rStr << (sal_Int32)pFieldDesc->GetHorJustify();
            _rStr << sal_Int32(pFieldDesc->IsAutoIncrement() ? 1 : 0);
            _rStr << sal_Int32(pFieldDesc->IsPrimaryKey() ? 1 : 0);
            _rStr << sal_Int32(pFieldDesc->IsCurrency() ? 1 : 0);
        }
        else
            _rStr << (sal_Int32)0;
        return _rStr;
    }
    SvStream& operator>>( SvStream& _rStr, OTableRow& _rRow )
    {
        _rStr >> _rRow.m_nPos;
        sal_Int32 nValue = 0;
        _rStr >> nValue;
        if ( nValue )
        {
            OFieldDescription* pFieldDesc = new OFieldDescription();
            _rRow.m_pActFieldDescr = pFieldDesc;
            String sValue = _rStr.ReadUniOrByteString(_rStr.GetStreamCharSet());
            pFieldDesc->SetName(sValue);

            sValue = _rStr.ReadUniOrByteString(_rStr.GetStreamCharSet());
            pFieldDesc->SetDescription(sValue);
            sValue = _rStr.ReadUniOrByteString(_rStr.GetStreamCharSet());
            pFieldDesc->SetHelpText(sValue);

            _rStr >> nValue;
            Any aControlDefault;
            switch ( nValue )
            {
                case 1:
                {
                    double nControlDefault;
                    _rStr >> nControlDefault;
                    aControlDefault <<= nControlDefault;
                    break;
                }
                case 2:
                    sValue = _rStr.ReadUniOrByteString(_rStr.GetStreamCharSet());
                    aControlDefault <<= OUString(sValue);
                    break;
            }

            pFieldDesc->SetControlDefault(aControlDefault);

            _rStr >> nValue;
            pFieldDesc->SetTypeValue(nValue);

            _rStr >> nValue;
            pFieldDesc->SetPrecision(nValue);
            _rStr >> nValue;
            pFieldDesc->SetScale(nValue);
            _rStr >> nValue;
            pFieldDesc->SetIsNullable(nValue);
            _rStr >> nValue;
            pFieldDesc->SetFormatKey(nValue);
            _rStr >> nValue;
            pFieldDesc->SetHorJustify((SvxCellHorJustify)nValue);

            _rStr >> nValue;
            pFieldDesc->SetAutoIncrement(nValue != 0);
            _rStr >> nValue;
            pFieldDesc->SetPrimaryKey(nValue != 0);
            _rStr >> nValue;
            pFieldDesc->SetCurrency(nValue != 0);
        }
        return _rStr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

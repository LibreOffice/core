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

#include <TableRow.hxx>
#include <tools/stream.hxx>
#include <FieldDescriptions.hxx>
#include <comphelper/types.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

OTableRow::OTableRow()
    :m_pActFieldDescr( nullptr )
    ,m_nPos( -1 )
    ,m_bReadOnly( false )
    ,m_bOwnsDescriptions(false)
{
}

OTableRow::OTableRow( const OTableRow& rRow, tools::Long nPosition )
    :m_pActFieldDescr(nullptr)
    ,m_nPos( nPosition )
    ,m_bReadOnly(rRow.IsReadOnly())
    ,m_bOwnsDescriptions(false)
{

    OFieldDescription* pSrcField = rRow.GetActFieldDescr();
    if(pSrcField)
    {
        m_pActFieldDescr = new OFieldDescription(*pSrcField);
        m_bOwnsDescriptions = true;
    }
}

OTableRow::~OTableRow()
{
    if(m_bOwnsDescriptions)
        delete m_pActFieldDescr;
}

void OTableRow::SetPrimaryKey( bool bSet )
{
    if(m_pActFieldDescr)
        m_pActFieldDescr->SetPrimaryKey(bSet);
}

bool OTableRow::IsPrimaryKey() const
{
    return m_pActFieldDescr && m_pActFieldDescr->IsPrimaryKey();
}

void OTableRow::SetFieldType( const TOTypeInfoSP& _pType, bool _bForce )
{
    if ( _pType )
    {
        if( !m_pActFieldDescr )
        {
            m_pActFieldDescr = new OFieldDescription();
            m_bOwnsDescriptions = true;
        }
        m_pActFieldDescr->FillFromTypeInfo(_pType,_bForce,true);
    }
    else
    {
        delete m_pActFieldDescr;
        m_pActFieldDescr = nullptr;
    }
}

namespace dbaui
{
    SvStream& WriteOTableRow( SvStream& _rStr, const OTableRow& _rRow )
    {
        _rStr.WriteInt32( _rRow.m_nPos );
        OFieldDescription* pFieldDesc = _rRow.GetActFieldDescr();
        if(pFieldDesc)
        {
            _rStr.WriteInt32( 1 );
            _rStr.WriteUniOrByteString(pFieldDesc->GetName(), _rStr.GetStreamCharSet());
            _rStr.WriteUniOrByteString(pFieldDesc->GetDescription(), _rStr.GetStreamCharSet());
            _rStr.WriteUniOrByteString(pFieldDesc->GetHelpText(), _rStr.GetStreamCharSet());
            double nValue = 0.0;
            Any aValue = pFieldDesc->GetControlDefault();
            if ( aValue >>= nValue )
            {
                _rStr.WriteInt32( 1 );
                _rStr.WriteDouble( nValue );
            }
            else
            {
                _rStr.WriteInt32( 2 );
                _rStr.WriteUniOrByteString(::comphelper::getString(aValue), _rStr.GetStreamCharSet());
            }

            _rStr.WriteInt32( pFieldDesc->GetType() );

            _rStr.WriteInt32( pFieldDesc->GetPrecision() );
            _rStr.WriteInt32( pFieldDesc->GetScale() );
            _rStr.WriteInt32( pFieldDesc->GetIsNullable() );
            _rStr.WriteInt32( pFieldDesc->GetFormatKey() );
            _rStr.WriteInt32( static_cast<sal_Int32>(pFieldDesc->GetHorJustify()) );
            _rStr.WriteInt32( pFieldDesc->IsAutoIncrement() ? 1 : 0 );
            _rStr.WriteInt32( pFieldDesc->IsPrimaryKey() ? 1 : 0 );
            _rStr.WriteInt32( pFieldDesc->IsCurrency() ? 1 : 0 );
        }
        else
            _rStr.WriteInt32( 0 );
        return _rStr;
    }
    SvStream& ReadOTableRow( SvStream& _rStr, OTableRow& _rRow )
    {
        _rStr.ReadInt32( _rRow.m_nPos );
        sal_Int32 nValue = 0;
        _rStr.ReadInt32( nValue );
        if ( nValue )
        {
            OFieldDescription* pFieldDesc = new OFieldDescription();
            _rRow.m_pActFieldDescr = pFieldDesc;
            pFieldDesc->SetName(_rStr.ReadUniOrByteString(_rStr.GetStreamCharSet()));
            pFieldDesc->SetDescription(_rStr.ReadUniOrByteString(_rStr.GetStreamCharSet()));
            pFieldDesc->SetHelpText(_rStr.ReadUniOrByteString(_rStr.GetStreamCharSet()));

            _rStr.ReadInt32( nValue );
            Any aControlDefault;
            switch ( nValue )
            {
                case 1:
                {
                    double nControlDefault;
                    _rStr.ReadDouble( nControlDefault );
                    aControlDefault <<= nControlDefault;
                    break;
                }
                case 2:
                    aControlDefault <<= _rStr.ReadUniOrByteString(_rStr.GetStreamCharSet());
                    break;
            }

            pFieldDesc->SetControlDefault(aControlDefault);

            _rStr.ReadInt32( nValue );
            pFieldDesc->SetTypeValue(nValue);

            _rStr.ReadInt32( nValue );
            pFieldDesc->SetPrecision(nValue);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetScale(nValue);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetIsNullable(nValue);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetFormatKey(nValue);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetHorJustify(static_cast<SvxCellHorJustify>(nValue));

            _rStr.ReadInt32( nValue );
            pFieldDesc->SetAutoIncrement(nValue != 0);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetPrimaryKey(nValue != 0);
            _rStr.ReadInt32( nValue );
            pFieldDesc->SetCurrency(nValue != 0);
        }
        return _rStr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

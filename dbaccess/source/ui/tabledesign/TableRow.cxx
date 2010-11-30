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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "TableRow.hxx"
#include <tools/debug.hxx>
#include "FieldDescriptions.hxx"
#include <algorithm>
#include <comphelper/types.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

//========================================================================
// class OTableRow
//========================================================================
DBG_NAME(OTableRow)
//------------------------------------------------------------------------------
OTableRow::OTableRow()
    :m_pActFieldDescr( NULL )
    ,m_nPos( -1 )
    ,m_bReadOnly( false )
    ,m_bOwnsDescriptions(false)
{
    DBG_CTOR(OTableRow,NULL);
}
//------------------------------------------------------------------------------
OTableRow::OTableRow(const Reference< XPropertySet >& xAffectedCol)
    :m_pActFieldDescr( NULL )
    ,m_nPos( -1 )
    ,m_bReadOnly( false )
    ,m_bOwnsDescriptions(true)
{
    DBG_CTOR(OTableRow,NULL);
    m_pActFieldDescr = new OFieldDescription(xAffectedCol);
}
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
OTableRow::~OTableRow()
{
    DBG_DTOR(OTableRow,NULL);
    if(m_bOwnsDescriptions)
        delete m_pActFieldDescr;
}

//------------------------------------------------------------------------------
void OTableRow::SetPrimaryKey( sal_Bool bSet )
{
    DBG_CHKTHIS(OTableRow,NULL);
    if(m_pActFieldDescr)
        m_pActFieldDescr->SetPrimaryKey(bSet);
}
// -----------------------------------------------------------------------------
sal_Bool OTableRow::IsPrimaryKey() const
{
    DBG_CHKTHIS(OTableRow,NULL);
    return m_pActFieldDescr && m_pActFieldDescr->IsPrimaryKey();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
namespace dbaui
{
    // -----------------------------------------------------------------------------
    SvStream& operator<<( SvStream& _rStr, const OTableRow& _rRow )
    {
        _rStr << _rRow.m_nPos;
        OFieldDescription* pFieldDesc = _rRow.GetActFieldDescr();
        if(pFieldDesc)
        {
            _rStr << (sal_Int32)1;
            _rStr.WriteByteString(pFieldDesc->GetName());
            _rStr.WriteByteString(pFieldDesc->GetDescription());
            _rStr.WriteByteString(pFieldDesc->GetHelpText());
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
                _rStr.WriteByteString(::comphelper::getString(aValue));
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
    // -----------------------------------------------------------------------------
    SvStream& operator>>( SvStream& _rStr, OTableRow& _rRow )
    {
        _rStr >> _rRow.m_nPos;
        sal_Int32 nValue = 0;
        _rStr >> nValue;
        if ( nValue )
        {
            OFieldDescription* pFieldDesc = new OFieldDescription();
            _rRow.m_pActFieldDescr = pFieldDesc;
            String sValue;
            _rStr.ReadByteString(sValue);
            pFieldDesc->SetName(sValue);

            _rStr.ReadByteString(sValue);
            pFieldDesc->SetDescription(sValue);
            _rStr.ReadByteString(sValue);
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
                    _rStr.ReadByteString(sValue);
                    aControlDefault <<= ::rtl::OUString(sValue);
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
    // -----------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

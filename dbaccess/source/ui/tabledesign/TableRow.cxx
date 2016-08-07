/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
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
        } // if(pFieldDesc)
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




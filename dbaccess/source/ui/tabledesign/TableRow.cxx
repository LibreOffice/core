/*************************************************************************
 *
 *  $RCSfile: TableRow.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:18:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
DBG_NAME(OTableRow);
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
OTableRow::OTableRow( const OTableRow& rRow, long nPosition ) :
    m_nPos( nPosition )
    ,m_bReadOnly(rRow.IsReadOnly())
    ,m_pActFieldDescr(NULL)
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
            _rStr.WriteByteString(pFieldDesc->GetName());
            _rStr.WriteByteString(pFieldDesc->GetDescription());
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
        return _rStr;
    }
    // -----------------------------------------------------------------------------
    SvStream& operator>>( SvStream& _rStr, OTableRow& _rRow )
    {
        _rStr >> _rRow.m_nPos;

        OFieldDescription* pFieldDesc = new OFieldDescription();
        _rRow.m_pActFieldDescr = pFieldDesc;
        if(pFieldDesc)
        {
            String sValue;
            _rStr.ReadByteString(sValue);
            pFieldDesc->SetName(sValue);

            _rStr.ReadByteString(sValue);
            pFieldDesc->SetDescription(sValue);

            sal_Int32 nValue;
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




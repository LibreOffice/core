/*************************************************************************
 *
 *  $RCSfile: TableRow.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
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
    ,m_bReadOnly( sal_False )
    ,m_bFirstNameModify( sal_True )
    ,m_bFirstDescrModify( sal_True )
    ,m_bOwnsDescriptions(sal_False)
{
    DBG_CTOR(OTableRow,NULL);
}
//------------------------------------------------------------------------------
OTableRow::OTableRow(const Reference< XPropertySet >& xAffectedCol)
    :m_pActFieldDescr( NULL )
    ,m_nPos( -1 )
    ,m_bReadOnly( sal_False )
    ,m_bFirstNameModify( sal_True )
    ,m_bFirstDescrModify( sal_True )
    ,m_bOwnsDescriptions(sal_True)
{
    DBG_CTOR(OTableRow,NULL);
    m_pActFieldDescr = new OFieldDescription(xAffectedCol);
}
//------------------------------------------------------------------------------
OTableRow::OTableRow( const OTableRow& rRow, long nPosition ) :
    m_nPos( nPosition )
    ,m_bReadOnly(rRow.IsReadOnly())
    ,m_bFirstNameModify(rRow.IsFirstNameModify())
    ,m_bFirstDescrModify(rRow.IsFirstDescrModify())
    ,m_pActFieldDescr(NULL)
    ,m_bOwnsDescriptions(sal_False)
{
    DBG_CTOR(OTableRow,NULL);

    OFieldDescription* pSrcField = rRow.GetActFieldDescr();
    if(pSrcField)
        m_pActFieldDescr = new OFieldDescription(*pSrcField);
}

//------------------------------------------------------------------------------
OTableRow::~OTableRow()
{
    if(m_bOwnsDescriptions)
        delete m_pActFieldDescr;
    DBG_DTOR(OTableRow,NULL);
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
void OTableRow::SetFieldType( const OTypeInfo* _pType )
{
    DBG_CHKTHIS(OTableRow,NULL);
    if(_pType)
    {
        if( !m_pActFieldDescr )
        {
            m_pActFieldDescr = new OFieldDescription();
            switch(_pType->nType)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                    m_pActFieldDescr->SetPrecision(::std::min<sal_Int32>(sal_Int32(50),_pType->nPrecision));
                    break;
                default:
                    if(_pType->nPrecision && _pType->nMaximumScale)
                    {
                        m_pActFieldDescr->SetPrecision(5);
                        m_pActFieldDescr->SetScale(0);
                    }
                    else if(_pType->nPrecision)
                        m_pActFieldDescr->SetPrecision(::std::min<sal_Int32>(sal_Int32(16),_pType->nPrecision));
            }
        }
        // type checking
        if(m_pActFieldDescr->GetPrecision() > _pType->nPrecision)
            m_pActFieldDescr->SetPrecision(_pType->nPrecision);
        if(m_pActFieldDescr->GetScale() > _pType->nMaximumScale)
            m_pActFieldDescr->SetScale(0);
        if(!_pType->bNullable && m_pActFieldDescr->IsNullable())
            m_pActFieldDescr->SetIsNullable(ColumnValue::NO_NULLS);
        if(!_pType->bAutoIncrement && m_pActFieldDescr->IsAutoIncrement())
            m_pActFieldDescr->SetAutoIncrement(sal_False);


        m_pActFieldDescr->SetType(_pType);
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
            _rStr.WriteByteString(pFieldDesc->GetDefaultValue());

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

            _rStr.ReadByteString(sValue);
            pFieldDesc->SetDefaultValue(sValue);

            sal_Int32 nValue;
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




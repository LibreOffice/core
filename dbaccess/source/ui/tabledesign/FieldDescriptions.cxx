/*************************************************************************
 *
 *  $RCSfile: FieldDescriptions.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:49:40 $
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

#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBU_TBL_HRC_
#include "dbu_tbl.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

#define DEFAULT_VARCHAR_PRECSION    50
#define DEFAULT_OTHER_PRECSION      16
#define DEFAULT_NUMERIC_PRECSION    5
#define DEFAULT_NUMERIC_SCALE       0


using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

//========================================================================
// class OFieldDescription
//========================================================================
DBG_NAME(OFieldDescription);
//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription() :
    m_bIsPrimaryKey(sal_False)
    ,m_nFormatKey(0)
    ,m_bIsAutoIncrement(sal_False)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_nScale(0)
    ,m_nPrecision(0)
    ,m_pType(NULL)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nType(DataType::VARCHAR)
    ,m_bIsCurrency(sal_False)
{
    DBG_CTOR(OFieldDescription,NULL);
}

//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription( const OFieldDescription& rDescr ) :
     m_sName(rDescr.m_sName)
    ,m_sTypeName(rDescr.m_sTypeName)
    ,m_sDescription(rDescr.m_sDescription)
    ,m_aDefaultValue(rDescr.m_aDefaultValue)
    ,m_aControlDefault(rDescr.m_aControlDefault)
    ,m_sAutoIncrementValue(rDescr.m_sAutoIncrementValue)
    ,m_pType(rDescr.m_pType)
    ,m_nPrecision(rDescr.m_nPrecision)
    ,m_nScale(rDescr.m_nScale)
    ,m_nIsNullable(rDescr.m_nIsNullable)
    ,m_nFormatKey(rDescr.m_nFormatKey)
    ,m_eHorJustify(rDescr.m_eHorJustify)
    ,m_bIsAutoIncrement(rDescr.m_bIsAutoIncrement)
    ,m_bIsPrimaryKey(rDescr.m_bIsPrimaryKey)
    ,m_nType(DataType::VARCHAR)
{
    DBG_CTOR(OFieldDescription,NULL);
}
// -----------------------------------------------------------------------------
OFieldDescription::OFieldDescription(   const ::rtl::OUString&  _sName,
                    const ::rtl::OUString&  _sTypeName,
                    const ::rtl::OUString&  _sDescription,
                    const ::com::sun::star::uno::Any&   _aDefaultValue,
                    const ::com::sun::star::uno::Any&   _aControlDefault,
                    const ::rtl::OUString&  _sAutoIncrementValue,
                    const TOTypeInfoSP&     _pType,
                    sal_Int32               _nPrecision,
                    sal_Int32               _nScale,
                    sal_Int32               _nIsNullable,
                    sal_Int32               _nFormatKey,
                    SvxCellHorJustify       _eHorJustify,
                    sal_Bool                _bIsAutoIncrement,
                    sal_Bool                _bIsPrimaryKey,
                    sal_Bool                _bIsCurrency)    :
 m_sName(_sName)
,m_sTypeName(_sTypeName)
,m_sDescription(_sDescription)
,m_aDefaultValue(_aDefaultValue)
,m_aControlDefault(_aControlDefault)
,m_sAutoIncrementValue(_sAutoIncrementValue)
,m_pType(_pType)
,m_nPrecision(_nPrecision)
,m_nScale(_nScale)
,m_nIsNullable(_nIsNullable)
,m_nFormatKey(_nFormatKey)
,m_eHorJustify(_eHorJustify)
,m_bIsAutoIncrement(_bIsAutoIncrement)
,m_bIsPrimaryKey(_bIsPrimaryKey)
,m_bIsCurrency(_bIsCurrency)
{
     DBG_DTOR(OFieldDescription,NULL);
}

//------------------------------------------------------------------------------
OFieldDescription::~OFieldDescription()
{
    DBG_DTOR(OFieldDescription,NULL);
}
//------------------------------------------------------------------------------
OFieldDescription::OFieldDescription(const Reference< XPropertySet >& xAffectedCol)
    :m_bIsPrimaryKey(sal_False)
    ,m_nFormatKey(0)
    ,m_bIsAutoIncrement(sal_False)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_nScale(0)
    ,m_nPrecision(0)
    ,m_pType(NULL)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nType(DataType::VARCHAR)
    ,m_bIsCurrency(sal_False)
{
    DBG_CTOR(OFieldDescription,NULL);
    OSL_ENSURE(xAffectedCol.is(),"PropetySet can notbe null!");
    if(xAffectedCol.is())
    {
        Reference<XPropertySetInfo> xPropSetInfo = xAffectedCol->getPropertySetInfo();
        if(xPropSetInfo->hasPropertyByName(PROPERTY_NAME))
            SetName(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_NAME)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_DESCRIPTION))
            SetDescription(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_DESCRIPTION)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE))
            SetDefaultValue( xAffectedCol->getPropertyValue(PROPERTY_DEFAULTVALUE) );

        if(xPropSetInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
            SetControlDefault( xAffectedCol->getPropertyValue(PROPERTY_CONTROLDEFAULT) );

        if(xPropSetInfo->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION))
            SetAutoIncrementValue(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_AUTOINCREMENTCREATION)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_TYPE))
            SetTypeValue(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_TYPE)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_PRECISION))
            SetPrecision(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_PRECISION)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_SCALE))
            SetScale(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_SCALE)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_ISNULLABLE))
            SetIsNullable(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_ISNULLABLE)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_FORMATKEY))
            SetFormatKey(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY)));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_ALIGN))
            SetHorJustify( ::dbaui::mapTextJustify(::comphelper::getINT16(xAffectedCol->getPropertyValue(PROPERTY_ALIGN))));
        if(xPropSetInfo->hasPropertyByName(PROPERTY_ISAUTOINCREMENT))
            SetAutoIncrement(::cppu::any2bool(xAffectedCol->getPropertyValue(PROPERTY_ISAUTOINCREMENT)));
    }
}
// -----------------------------------------------------------------------------
void OFieldDescription::FillFromTypeInfo(const TOTypeInfoSP& _pType,sal_Bool _bForce,sal_Bool _bReset)
{
    TOTypeInfoSP pOldType = getTypeInfo();
    if ( _pType != pOldType )
    {
        // reset type depending information
        if ( _bReset )
        {
            SetFormatKey(0);
            SetControlDefault(Any());
        }

        sal_Bool bForce = _bForce || pOldType.get() == NULL || pOldType->nType != _pType->nType;
        switch ( _pType->nType )
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
                if ( bForce )
                {
                    sal_Int32 nPrec = DEFAULT_VARCHAR_PRECSION;
                    if ( GetPrecision() )
                        nPrec = GetPrecision();
                    SetPrecision(::std::min<sal_Int32>(nPrec,_pType->nPrecision));
                }
                break;
            default:
                if ( bForce )
                {
                    sal_Int32 nPrec = DEFAULT_OTHER_PRECSION;
                    if ( GetPrecision() )
                        nPrec = GetPrecision();
                    if ( _pType->nPrecision )
                        SetPrecision(::std::min<sal_Int32>(nPrec ? nPrec : DEFAULT_NUMERIC_PRECSION,_pType->nPrecision));
                    if ( _pType->nMaximumScale )
                        SetScale(::std::min<sal_Int32>(GetScale() ? GetScale() : DEFAULT_NUMERIC_SCALE,_pType->nMaximumScale));
                }
        }
        if ( !_pType->aCreateParams.getLength() )
        {
            SetPrecision(_pType->nPrecision);
            SetScale(_pType->nMinimumScale);
        }
        if ( !_pType->bNullable && IsNullable() )
            SetIsNullable(ColumnValue::NO_NULLS);
        if ( !_pType->bAutoIncrement && IsAutoIncrement() )
            SetAutoIncrement(sal_False);
        SetCurrency( _pType->bCurrency );
        SetType(_pType);
    }
}
// -----------------------------------------------------------------------------





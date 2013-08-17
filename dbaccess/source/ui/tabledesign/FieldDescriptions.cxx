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

#include "FieldDescriptions.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "dbu_tbl.hrc"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "dbustrings.hrc"
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include "UITools.hxx"
#include <com/sun/star/util/NumberFormat.hpp>

#define DEFAULT_VARCHAR_PRECSION    100
#define DEFAULT_OTHER_PRECSION      16
#define DEFAULT_NUMERIC_PRECSION    5
#define DEFAULT_NUMERIC_SCALE       0

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

// class OFieldDescription
DBG_NAME(OFieldDescription)
OFieldDescription::OFieldDescription()
    :m_pType()
    ,m_nType(DataType::VARCHAR)
    ,m_nPrecision(0)
    ,m_nScale(0)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nFormatKey(0)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_bIsAutoIncrement(sal_False)
    ,m_bIsPrimaryKey(sal_False)
    ,m_bIsCurrency(sal_False)
    ,m_bHidden(sal_False)
{
    DBG_CTOR(OFieldDescription,NULL);
}

OFieldDescription::OFieldDescription( const OFieldDescription& rDescr )
    :m_aDefaultValue(rDescr.m_aDefaultValue)
    ,m_aControlDefault(rDescr.m_aControlDefault)
    ,m_aWidth(rDescr.m_aWidth)
    ,m_aRelativePosition(rDescr.m_aRelativePosition)
    ,m_pType(rDescr.m_pType)
    ,m_xDest(rDescr.m_xDest)
    ,m_xDestInfo(rDescr.m_xDestInfo)
    ,m_sName(rDescr.m_sName)
    ,m_sTypeName(rDescr.m_sTypeName)
    ,m_sDescription(rDescr.m_sDescription)
    ,m_sAutoIncrementValue(rDescr.m_sAutoIncrementValue)
    ,m_nType(rDescr.m_nType)
    ,m_nPrecision(rDescr.m_nPrecision)
    ,m_nScale(rDescr.m_nScale)
    ,m_nIsNullable(rDescr.m_nIsNullable)
    ,m_nFormatKey(rDescr.m_nFormatKey)
    ,m_eHorJustify(rDescr.m_eHorJustify)
    ,m_bIsAutoIncrement(rDescr.m_bIsAutoIncrement)
    ,m_bIsPrimaryKey(rDescr.m_bIsPrimaryKey)
    ,m_bIsCurrency(rDescr.m_bIsCurrency)
    ,m_bHidden(rDescr.m_bHidden)
{
    DBG_CTOR(OFieldDescription,NULL);
}

OFieldDescription::~OFieldDescription()
{
    DBG_DTOR(OFieldDescription,NULL);
}

OFieldDescription::OFieldDescription(const Reference< XPropertySet >& xAffectedCol,sal_Bool _bUseAsDest)
    :m_pType()
    ,m_nType(DataType::VARCHAR)
    ,m_nPrecision(0)
    ,m_nScale(0)
    ,m_nIsNullable(ColumnValue::NULLABLE)
    ,m_nFormatKey(0)
    ,m_eHorJustify(SVX_HOR_JUSTIFY_STANDARD)
    ,m_bIsAutoIncrement(sal_False)
    ,m_bIsPrimaryKey(sal_False)
    ,m_bIsCurrency(sal_False)
    ,m_bHidden(sal_False)
{
    DBG_CTOR(OFieldDescription,NULL);
    OSL_ENSURE(xAffectedCol.is(),"PropetySet can notbe null!");
    if ( xAffectedCol.is() )
    {
        if ( _bUseAsDest )
        {
            m_xDest = xAffectedCol;
            m_xDestInfo = xAffectedCol->getPropertySetInfo();
        }
        else
        {
            try
            {
                Reference<XPropertySetInfo> xPropSetInfo = xAffectedCol->getPropertySetInfo();
                if(xPropSetInfo->hasPropertyByName(PROPERTY_NAME))
                    SetName(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_NAME)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_DESCRIPTION))
                    SetDescription(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_DESCRIPTION)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_HELPTEXT))
                {
                    OUString sHelpText;
                    xAffectedCol->getPropertyValue(PROPERTY_HELPTEXT) >>= sHelpText;
                    SetHelpText(sHelpText);
                }
                if(xPropSetInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE))
                    SetDefaultValue( xAffectedCol->getPropertyValue(PROPERTY_DEFAULTVALUE) );

                if(xPropSetInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                    SetControlDefault( xAffectedCol->getPropertyValue(PROPERTY_CONTROLDEFAULT) );

                if(xPropSetInfo->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION))
                    SetAutoIncrementValue(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_AUTOINCREMENTCREATION)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_TYPE))
                    SetTypeValue(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_TYPE)));
                if (xPropSetInfo->hasPropertyByName(PROPERTY_TYPENAME))
                    SetTypeName(::comphelper::getString(xAffectedCol->getPropertyValue(PROPERTY_TYPENAME)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_PRECISION))
                    SetPrecision(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_PRECISION)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_SCALE))
                    SetScale(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_SCALE)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_ISNULLABLE))
                    SetIsNullable(::comphelper::getINT32(xAffectedCol->getPropertyValue(PROPERTY_ISNULLABLE)));
                if(xPropSetInfo->hasPropertyByName(PROPERTY_FORMATKEY))
                {
                    const Any aValue = xAffectedCol->getPropertyValue(PROPERTY_FORMATKEY);
                    if ( aValue.hasValue() )
                        SetFormatKey(::comphelper::getINT32(aValue));
                }
                if(xPropSetInfo->hasPropertyByName(PROPERTY_RELATIVEPOSITION))
                    m_aRelativePosition = xAffectedCol->getPropertyValue(PROPERTY_RELATIVEPOSITION);
                if(xPropSetInfo->hasPropertyByName(PROPERTY_WIDTH))
                    m_aWidth = xAffectedCol->getPropertyValue(PROPERTY_WIDTH);
                if(xPropSetInfo->hasPropertyByName(PROPERTY_HIDDEN))
                    xAffectedCol->getPropertyValue(PROPERTY_HIDDEN) >>= m_bHidden;
                if(xPropSetInfo->hasPropertyByName(PROPERTY_ALIGN))
                {
                    const Any aValue = xAffectedCol->getPropertyValue(PROPERTY_ALIGN);
                    if ( aValue.hasValue() )
                        SetHorJustify( ::dbaui::mapTextJustify(::comphelper::getINT32(aValue)));
                }
                if(xPropSetInfo->hasPropertyByName(PROPERTY_ISAUTOINCREMENT))
                    SetAutoIncrement(::cppu::any2bool(xAffectedCol->getPropertyValue(PROPERTY_ISAUTOINCREMENT)));
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
}

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
            case DataType::TIMESTAMP:
                if ( bForce && _pType->nMaximumScale)
                {
                    SetScale(::std::min<sal_Int32>(GetScale() ? GetScale() : DEFAULT_NUMERIC_SCALE,_pType->nMaximumScale));
                }
                break;
            default:
                if ( bForce )
                {
                    sal_Int32 nPrec = DEFAULT_OTHER_PRECSION;
                    switch ( _pType->nType )
                    {
                        case DataType::BIT:
                        case DataType::BLOB:
                        case DataType::CLOB:
                            nPrec = _pType->nPrecision;
                            break;
                        default:
                            if ( GetPrecision() )
                                nPrec = GetPrecision();
                            break;
                    }

                    if ( _pType->nPrecision )
                        SetPrecision(::std::min<sal_Int32>(nPrec ? nPrec : DEFAULT_NUMERIC_PRECSION,_pType->nPrecision));
                    if ( _pType->nMaximumScale )
                        SetScale(::std::min<sal_Int32>(GetScale() ? GetScale() : DEFAULT_NUMERIC_SCALE,_pType->nMaximumScale));
                }
        }
        if ( _pType->aCreateParams.isEmpty() )
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
        SetTypeName(_pType->aTypeName);
    }
}

void OFieldDescription::SetName(const OUString& _rName)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_NAME) )
            m_xDest->setPropertyValue(PROPERTY_NAME,makeAny(_rName));
        else
            m_sName = _rName;
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetHelpText(const OUString& _sHelpText)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_HELPTEXT) )
            m_xDest->setPropertyValue(PROPERTY_HELPTEXT,makeAny(_sHelpText));
        else
            m_sHelpText = _sHelpText;
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetDescription(const OUString& _rDescription)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_DESCRIPTION) )
            m_xDest->setPropertyValue(PROPERTY_DESCRIPTION,makeAny(_rDescription));
        else
            m_sDescription = _rDescription;
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetDefaultValue(const Any& _rDefaultValue)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE) )
            m_xDest->setPropertyValue(PROPERTY_DEFAULTVALUE,makeAny(_rDefaultValue));
        else
            m_aDefaultValue = _rDefaultValue;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetControlDefault(const Any& _rControlDefault)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT) )
            m_xDest->setPropertyValue(PROPERTY_CONTROLDEFAULT,makeAny(_rControlDefault));
        else
            m_aControlDefault = _rControlDefault;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetAutoIncrementValue(const OUString& _sAutoIncValue)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION) )
            m_xDest->setPropertyValue(PROPERTY_AUTOINCREMENTCREATION,makeAny(_sAutoIncValue));
        else
            m_sAutoIncrementValue = _sAutoIncValue;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetType(TOTypeInfoSP _pType)
{
    m_pType = _pType;
    if ( m_pType.get() )
    {
        try
        {
            if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_TYPE) )
                m_xDest->setPropertyValue(PROPERTY_TYPE,makeAny(m_pType->nType));
            else
                m_nType = m_pType->nType;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void OFieldDescription::SetTypeValue(sal_Int32 _nType)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_TYPE) )
            m_xDest->setPropertyValue(PROPERTY_TYPE,makeAny(_nType));
        else
        {
            m_nType = _nType;
            OSL_ENSURE(!m_pType.get(),"Invalid call here!");
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetPrecision(const sal_Int32& _rPrecision)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_PRECISION) )
            m_xDest->setPropertyValue(PROPERTY_PRECISION,makeAny(_rPrecision));
        else
            m_nPrecision = _rPrecision;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetScale(const sal_Int32& _rScale)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_SCALE) )
            m_xDest->setPropertyValue(PROPERTY_SCALE,makeAny(_rScale));
        else
            m_nScale = _rScale;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetIsNullable(const sal_Int32& _rIsNullable)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ISNULLABLE) )
            m_xDest->setPropertyValue(PROPERTY_ISNULLABLE,makeAny(_rIsNullable));
        else
            m_nIsNullable = _rIsNullable;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetFormatKey(const sal_Int32& _rFormatKey)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_FORMATKEY) )
            m_xDest->setPropertyValue(PROPERTY_FORMATKEY,makeAny(_rFormatKey));
        else
            m_nFormatKey = _rFormatKey;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetHorJustify(const SvxCellHorJustify& _rHorJustify)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ALIGN) )
            m_xDest->setPropertyValue(PROPERTY_ALIGN,makeAny( dbaui::mapTextAllign(_rHorJustify)));
        else
            m_eHorJustify = _rHorJustify;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetAutoIncrement(sal_Bool _bAuto)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ISAUTOINCREMENT) )
            m_xDest->setPropertyValue(PROPERTY_ISAUTOINCREMENT,makeAny(_bAuto));
        else
            m_bIsAutoIncrement = _bAuto;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::SetPrimaryKey(sal_Bool _bPKey)
{
    m_bIsPrimaryKey = _bPKey;
    if ( _bPKey )
        SetIsNullable(::com::sun::star::sdbc::ColumnValue::NO_NULLS);
}

void OFieldDescription::SetCurrency(sal_Bool _bIsCurrency)
{
    m_bIsCurrency = _bIsCurrency;
}

OUString             OFieldDescription::GetName()                const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_NAME) )
        return ::comphelper::getString(m_xDest->getPropertyValue(PROPERTY_NAME));
    else
        return m_sName;
}

OUString             OFieldDescription::GetDescription()         const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_DESCRIPTION) )
        return ::comphelper::getString(m_xDest->getPropertyValue(PROPERTY_DESCRIPTION));
    else
        return m_sDescription;
}

OUString             OFieldDescription::GetHelpText()            const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_HELPTEXT) )
        return ::comphelper::getString(m_xDest->getPropertyValue(PROPERTY_HELPTEXT));
    else
        return m_sHelpText;
}

::com::sun::star::uno::Any  OFieldDescription::GetControlDefault()      const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT) )
        return m_xDest->getPropertyValue(PROPERTY_CONTROLDEFAULT);
    else
        return m_aControlDefault;
}

OUString             OFieldDescription::GetAutoIncrementValue()  const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION) )
        return ::comphelper::getString(m_xDest->getPropertyValue(PROPERTY_AUTOINCREMENTCREATION));
    else
        return m_sAutoIncrementValue;
}

sal_Int32                   OFieldDescription::GetType()                const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_TYPE) )
        return ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_TYPE));
    else
        return m_pType.get() ? m_pType->nType : m_nType;
}

OUString             OFieldDescription::GetTypeName()            const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_TYPENAME) )
        return ::comphelper::getString(m_xDest->getPropertyValue(PROPERTY_TYPENAME));
    else
        return m_pType.get() ? m_pType->aTypeName : m_sTypeName;
}

sal_Int32                   OFieldDescription::GetPrecision()           const
{
    sal_Int32 nPrec = m_nPrecision;
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_PRECISION) )
        nPrec = ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_PRECISION));

    TOTypeInfoSP pTypeInfo = getTypeInfo();
    if ( pTypeInfo )
    {
        switch ( pTypeInfo->nType )
        {
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::BIGINT:
                if ( !nPrec )
                    nPrec = pTypeInfo->nPrecision;
                break;
        }
    }

    return nPrec;
}

sal_Int32                   OFieldDescription::GetScale()               const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_SCALE) )
        return ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_SCALE));
    else
        return m_nScale;
}

sal_Int32                   OFieldDescription::GetIsNullable()          const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ISNULLABLE) )
        return ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_ISNULLABLE));
    else
        return m_nIsNullable;
}

sal_Int32                   OFieldDescription::GetFormatKey()           const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_FORMATKEY) )
        return ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_FORMATKEY));
    else
        return m_nFormatKey;
}

SvxCellHorJustify           OFieldDescription::GetHorJustify()          const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ALIGN) )
        return ::dbaui::mapTextJustify(::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_ALIGN)));
    else
        return m_eHorJustify;
}

TOTypeInfoSP                OFieldDescription::getTypeInfo()            const
{
    return m_pType;
}

TOTypeInfoSP                OFieldDescription::getSpecialTypeInfo() const
{
    TOTypeInfoSP pSpecialType( new OTypeInfo() );
    *pSpecialType = *m_pType;
    pSpecialType->nPrecision = GetPrecision();
    pSpecialType->nMaximumScale = static_cast<sal_Int16>(GetScale());
    pSpecialType->bAutoIncrement = IsAutoIncrement(); // http://dba.openoffice.org/issues/show_bug.cgi?id=115398 fixed by ludob
    return pSpecialType;
}

sal_Bool                    OFieldDescription::IsAutoIncrement()        const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ISAUTOINCREMENT) )
        return ::cppu::any2bool(m_xDest->getPropertyValue(PROPERTY_ISAUTOINCREMENT));
    else
        return m_bIsAutoIncrement;
}

sal_Bool                    OFieldDescription::IsPrimaryKey()           const
{
    return m_bIsPrimaryKey;
}

sal_Bool                    OFieldDescription::IsCurrency()             const
{
        return m_bIsCurrency;
}

sal_Bool                    OFieldDescription::IsNullable()             const
{
    if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_ISNULLABLE) )
        return ::comphelper::getINT32(m_xDest->getPropertyValue(PROPERTY_ISNULLABLE)) == ::com::sun::star::sdbc::ColumnValue::NULLABLE;
    else
        return m_nIsNullable == ::com::sun::star::sdbc::ColumnValue::NULLABLE;
}

void OFieldDescription::SetTypeName(const OUString& _sTypeName)
{
    try
    {
        if ( m_xDest.is() && m_xDestInfo->hasPropertyByName(PROPERTY_TYPENAME) )
            m_xDest->setPropertyValue(PROPERTY_TYPENAME,makeAny(_sTypeName));
        else
            m_sTypeName = _sTypeName;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OFieldDescription::copyColumnSettingsTo(const Reference< XPropertySet >& _rxColumn)
{
    if ( _rxColumn.is() )
    {
        Reference<XPropertySetInfo> xInfo = _rxColumn->getPropertySetInfo();

        if ( GetFormatKey() != NumberFormat::ALL && xInfo->hasPropertyByName(PROPERTY_FORMATKEY) )
            _rxColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(GetFormatKey()));
        if ( GetHorJustify() != SVX_HOR_JUSTIFY_STANDARD && xInfo->hasPropertyByName(PROPERTY_ALIGN) )
            _rxColumn->setPropertyValue(PROPERTY_ALIGN,makeAny(dbaui::mapTextAllign(GetHorJustify())));
        if ( !GetHelpText().isEmpty() && xInfo->hasPropertyByName(PROPERTY_HELPTEXT) )
            _rxColumn->setPropertyValue(PROPERTY_HELPTEXT,makeAny(GetHelpText()));
        if ( GetControlDefault().hasValue() && xInfo->hasPropertyByName(PROPERTY_CONTROLDEFAULT) )
            _rxColumn->setPropertyValue(PROPERTY_CONTROLDEFAULT,GetControlDefault());

        if(xInfo->hasPropertyByName(PROPERTY_RELATIVEPOSITION))
            _rxColumn->setPropertyValue(PROPERTY_RELATIVEPOSITION,m_aRelativePosition);
        if(xInfo->hasPropertyByName(PROPERTY_WIDTH))
            _rxColumn->setPropertyValue(PROPERTY_WIDTH,m_aWidth);
        if(xInfo->hasPropertyByName(PROPERTY_HIDDEN))
            _rxColumn->setPropertyValue(PROPERTY_HIDDEN,makeAny(m_bHidden));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

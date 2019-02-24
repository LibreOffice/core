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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDDESCRIPTIONS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDDESCRIPTIONS_HXX

#include "QEnumTypes.hxx"
#include <editeng/svxenum.hxx>
#include "TypeInfo.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

namespace dbaui
{
    class OFieldDescription
    {
    private:
        css::uno::Any      m_aControlDefault;  // the value which the control inserts as default
        css::uno::Any      m_aWidth;               // sal_Int32 or void
        css::uno::Any      m_aRelativePosition;    // sal_Int32 or void

        TOTypeInfoSP       m_pType;

        css::uno::Reference< css::beans::XPropertySet >       m_xDest;
        css::uno::Reference< css::beans::XPropertySetInfo >   m_xDestInfo;

        OUString     m_sName;
        OUString     m_sTypeName;
        OUString     m_sDescription;
        OUString     m_sHelpText;

        OUString            m_sAutoIncrementValue;
        sal_Int32           m_nType;    // only used when m_pType is null
        sal_Int32           m_nPrecision;
        sal_Int32           m_nScale;
        sal_Int32           m_nIsNullable;
        sal_Int32           m_nFormatKey;
        SvxCellHorJustify   m_eHorJustify;
        bool                m_bIsAutoIncrement;
        bool                m_bIsPrimaryKey;
        bool                m_bIsCurrency;
        bool                m_bHidden;

    public:
        OFieldDescription();
        OFieldDescription( const OFieldDescription& rDescr );
        OFieldDescription(const css::uno::Reference< css::beans::XPropertySet >& _xAffectedCol
                         ,bool _bUseAsDest = false);
        ~OFieldDescription();

        void SetName(const OUString& _rName);
        void SetDescription(const OUString& _rDescription);
        void SetHelpText(const OUString& _sHelptext);
        void SetDefaultValue(const css::uno::Any& _rDefaultValue);
        void SetControlDefault(const css::uno::Any& _rControlDefault);
        void SetAutoIncrementValue(const OUString& _sAutoIncValue);
        void SetType(const TOTypeInfoSP& _pType);
        void SetTypeValue(sal_Int32 _nType);
        void SetTypeName(const OUString& _sTypeName);
        void SetPrecision(sal_Int32 _rPrecision);
        void SetScale(sal_Int32 _rScale);
        void SetIsNullable(sal_Int32 _rIsNullable);
        void SetFormatKey(sal_Int32 _rFormatKey);
        void SetHorJustify(const SvxCellHorJustify& _rHorJustify);
        void SetAutoIncrement(bool _bAuto);
        void SetPrimaryKey(bool _bPKey);
        void SetCurrency(bool _bIsCurrency);

        /** copies the content of the field description into the column
            @param  _rxColumn the dest
        */
        void copyColumnSettingsTo(const css::uno::Reference< css::beans::XPropertySet >& _rxColumn);

        void FillFromTypeInfo(const TOTypeInfoSP& _pType,bool _bForce,bool _bReset);

        OUString             GetName()               const;
        OUString             GetDescription()        const;
        OUString             GetHelpText()           const;
        css::uno::Any  GetControlDefault()     const;
        OUString             GetAutoIncrementValue() const;
        sal_Int32                   GetType()               const;
        OUString             GetTypeName()           const;
        sal_Int32                   GetPrecision()          const;
        sal_Int32                   GetScale()              const;
        sal_Int32                   GetIsNullable()         const;
        sal_Int32                   GetFormatKey()          const;
        SvxCellHorJustify           GetHorJustify()         const;
        const TOTypeInfoSP&         getTypeInfo()           const { return m_pType;}
        TOTypeInfoSP                getSpecialTypeInfo()    const;
        bool                    IsAutoIncrement()       const;
        bool                    IsPrimaryKey()          const { return m_bIsPrimaryKey;}
        bool                    IsCurrency()            const { return m_bIsCurrency;}
        bool                    IsNullable()            const;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDDESCRIPTIONS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

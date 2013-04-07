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
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#define DBAUI_FIELDDESCRIPTIONS_HXX

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
        ::com::sun::star::uno::Any      m_aDefaultValue;    // the default value from the database
        ::com::sun::star::uno::Any      m_aControlDefault;  // the value which the control inserts as default
        ::com::sun::star::uno::Any      m_aWidth;               // sal_Int32 or void
        ::com::sun::star::uno::Any      m_aRelativePosition;    // sal_Int32 or void

        TOTypeInfoSP    m_pType;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDestInfo;

        OUString     m_sName;
        OUString     m_sTypeName;
        OUString     m_sDescription;
        OUString     m_sHelpText;

        OUString     m_sAutoIncrementValue;
        sal_Int32           m_nType;    // only used when m_pType is null
        sal_Int32           m_nPrecision;
        sal_Int32           m_nScale;
        sal_Int32           m_nIsNullable;
        sal_Int32           m_nFormatKey;
        SvxCellHorJustify   m_eHorJustify;
        sal_Bool            m_bIsAutoIncrement;
        sal_Bool            m_bIsPrimaryKey;
        sal_Bool            m_bIsCurrency;
        sal_Bool            m_bHidden;

    public:
        OFieldDescription();
        OFieldDescription( const OFieldDescription& rDescr );
        OFieldDescription(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xAffectedCol
                         ,sal_Bool _bUseAsDest = sal_False);
        ~OFieldDescription();

        void SetName(const OUString& _rName);
        void SetDescription(const OUString& _rDescription);
        void SetHelpText(const OUString& _sHelptext);
        void SetDefaultValue(const ::com::sun::star::uno::Any& _rDefaultValue);
        void SetControlDefault(const ::com::sun::star::uno::Any& _rControlDefault);
        void SetAutoIncrementValue(const OUString& _sAutoIncValue);
        void SetType(TOTypeInfoSP _pType);
        void SetTypeValue(sal_Int32 _nType);
        void SetTypeName(const OUString& _sTypeName);
        void SetPrecision(const sal_Int32& _rPrecision);
        void SetScale(const sal_Int32& _rScale);
        void SetIsNullable(const sal_Int32& _rIsNullable);
        void SetFormatKey(const sal_Int32& _rFormatKey);
        void SetHorJustify(const SvxCellHorJustify& _rHorJustify);
        void SetAutoIncrement(sal_Bool _bAuto);
        void SetPrimaryKey(sal_Bool _bPKey);
        void SetCurrency(sal_Bool _bIsCurrency);

        /** copies the content of the field eescription into the column
            @param  _rxColumn the dest
        */
        void copyColumnSettingsTo(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn);

        void FillFromTypeInfo(const TOTypeInfoSP& _pType,sal_Bool _bForce,sal_Bool _bReset);

        OUString             GetName()               const;
        OUString             GetDescription()        const;
        OUString             GetHelpText()           const;
        ::com::sun::star::uno::Any  GetControlDefault()     const;
        OUString             GetAutoIncrementValue() const;
        sal_Int32                   GetType()               const;
        OUString             GetTypeName()           const;
        sal_Int32                   GetPrecision()          const;
        sal_Int32                   GetScale()              const;
        sal_Int32                   GetIsNullable()         const;
        sal_Int32                   GetFormatKey()          const;
        SvxCellHorJustify           GetHorJustify()         const;
        TOTypeInfoSP                getTypeInfo()           const;
        TOTypeInfoSP                getSpecialTypeInfo()    const;
        sal_Bool                    IsAutoIncrement()       const;
        sal_Bool                    IsPrimaryKey()          const;
        sal_Bool                    IsCurrency()            const;
        sal_Bool                    IsNullable()            const;
    };
}
#endif // DBAUI_FIELDDESCRIPTIONS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

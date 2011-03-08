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

        ::rtl::OUString     m_sName;
        ::rtl::OUString     m_sTypeName;
        ::rtl::OUString     m_sDescription;
        ::rtl::OUString     m_sHelpText;

        ::rtl::OUString     m_sAutoIncrementValue;
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

        void SetName(const ::rtl::OUString& _rName);
        void SetDescription(const ::rtl::OUString& _rDescription);
        void SetHelpText(const ::rtl::OUString& _sHelptext);
        void SetDefaultValue(const ::com::sun::star::uno::Any& _rDefaultValue);
        void SetControlDefault(const ::com::sun::star::uno::Any& _rControlDefault);
        void SetAutoIncrementValue(const ::rtl::OUString& _sAutoIncValue);
        void SetType(TOTypeInfoSP _pType);
        void SetTypeValue(sal_Int32 _nType);
        void SetTypeName(const ::rtl::OUString& _sTypeName);
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

        ::rtl::OUString             GetName()               const;
        ::rtl::OUString             GetDescription()        const;
        ::rtl::OUString             GetHelpText()           const;
        ::com::sun::star::uno::Any  GetControlDefault()     const;
        ::rtl::OUString             GetAutoIncrementValue() const;
        sal_Int32                   GetType()               const;
        ::rtl::OUString             GetTypeName()           const;
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

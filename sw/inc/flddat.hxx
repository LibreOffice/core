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

#ifndef INCLUDED_SW_INC_FLDDAT_HXX
#define INCLUDED_SW_INC_FLDDAT_HXX

#include <tools/solar.h>

#include "fldbas.hxx"

class DateTime;
class Date;
namespace tools { class Time; }

enum SwDateSubFormat
{
    DATE_FIX,
    DATE_VAR
};

class SAL_DLLPUBLIC_RTTI SwDateTimeFieldType final : public SwValueFieldType
{
public:
        SwDateTimeFieldType(SwDoc* pDoc);

        virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SW_DLLPUBLIC SwDateTimeField final : public SwValueField
{
        sal_uInt16              m_nSubType;
        long                m_nOffset;    // Offset in minutes.

        virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
        virtual std::unique_ptr<SwField> Copy() const override;

public:
        SwDateTimeField(SwDateTimeFieldType* pType, sal_uInt16 nSubType = DATEFLD,
                    sal_uLong nFormat = 0, LanguageType nLng = LANGUAGE_SYSTEM);

        virtual sal_uInt16      GetSubType() const override;
        virtual void            SetSubType(sal_uInt16 nSub) override;

        virtual double          GetValue() const override;

        virtual void            SetPar2(const OUString& rStr) override;
        virtual OUString        GetPar2() const override;

        void             SetOffset(long nMinutes)    { m_nOffset = nMinutes; }
        long             GetOffset() const           { return m_nOffset; }

        Date                    GetDate() const;
        tools::Time             GetTime() const;
        void                    SetDateTime(const DateTime& rDT);
        static double           GetDateTime(SwDoc& rDoc, const DateTime& rDT);

        virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt16 nMId ) const override;
        virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt16 nMId ) override;
};

#endif // INCLUDED_SW_INC_FLDDAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

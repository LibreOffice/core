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

#include "fldbas.hxx"

class DateTime;
class Date;
namespace tools { class Time; }

enum SwDateFormat
{
    DF_BEGIN,
    // new formats
    DFF_SSYS = DF_BEGIN,
    DFF_LSYS ,
    DFF_DMY ,
    DFF_DMYY ,
    DFF_DMMY ,
    DFF_DMMYY,
    DFF_DMMMY ,
    DFF_DMMMYY,
    DFF_DDMMY ,
    DFF_DDMMMY ,
    DFF_DDMMMYY ,
    DFF_DDDMMMY ,
    DFF_DDDMMMYY ,
    DFF_MY ,
    DFF_MD ,
    DFF_YMD ,
    DFF_YYMD ,
    DF_END ,
    // Compatibility
    DF_SSYS     = DFF_SSYS,
    DF_LSYS     = DFF_LSYS,
    DF_SHORT    = DFF_DMY,
    DF_SCENT    = DFF_DMYY,
    DF_LMON     = DFF_DMMYY,
    DF_LMONTH   = DFF_DMMMYY,
    DF_LDAYMON  = DFF_DDMMMYY,
    DF_LDAYMONTH= DFF_DDDMMMYY
};

enum SwTimeFormat
{
    TF_BEGIN,
    TF_SYSTEM=TF_BEGIN,
    TF_SSMM_24,
    TF_SSMM_12,
    TF_END
};

enum SwTimeSubFormat
{
    TIME_FIX,
    TIME_VAR
};

enum SwDateSubFormat
{
    DATE_FIX,
    DATE_VAR
};

class SwDateTimeFieldType : public SwValueFieldType
{
public:
        SwDateTimeFieldType(SwDoc* pDoc);

        virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwDateTimeField : public SwValueField
{
        sal_uInt16              nSubType;
        long                nOffset;    // Offset in minutes.

        virtual OUString    Expand() const override;
        virtual SwField*    Copy() const override;

public:
        SwDateTimeField(SwDateTimeFieldType* pType, sal_uInt16 nSubType = DATEFLD,
                    sal_uLong nFormat = 0, sal_uInt16 nLng = 0);

        virtual sal_uInt16          GetSubType() const override;
        virtual void            SetSubType(sal_uInt16 nSub) override;

        virtual double          GetValue() const override;

        virtual void            SetPar2(const OUString& rStr) override;
        virtual OUString   GetPar2() const override;

        inline bool             IsDate() const
                                { return (nSubType & DATEFLD) != 0; }

        inline void             SetOffset(long nMinutes)    { nOffset = nMinutes; }
        inline long             GetOffset() const           { return nOffset; }

        Date                    GetDate(bool bUseOffset = false) const;
        tools::Time             GetTime(bool bUseOffset = false) const;
        void                    SetDateTime(const DateTime& rDT);
        static double           GetDateTime(SwDoc* pDoc, const DateTime& rDT);

        virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt16 nMId ) const override;
        virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt16 nMId ) override;
};

#endif // INCLUDED_SW_INC_FLDDAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

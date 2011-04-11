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

#ifndef SW_FLDDAT_HXX
#define SW_FLDDAT_HXX

#include <tools/string.hxx>

#include "fldbas.hxx"

class DateTime;
class Date;
class Time;


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

        virtual SwFieldType*    Copy() const;
};


class SW_DLLPUBLIC SwDateTimeField : public SwValueField
{
        sal_uInt16              nSubType;
        long                nOffset;    // Offset in minutes.

        virtual String      Expand() const;
        virtual SwField*    Copy() const;

public:
        SwDateTimeField(SwDateTimeFieldType* pType, sal_uInt16 nSubType = DATEFLD,
                    sal_uLong nFmt = 0, sal_uInt16 nLng = 0);

        virtual sal_uInt16          GetSubType() const;
        virtual void            SetSubType(sal_uInt16 nSub);

        virtual double          GetValue() const;

        virtual void            SetPar2(const String& rStr);
        virtual String          GetPar2() const;

        inline sal_Bool             IsDate() const
                                { return (nSubType & DATEFLD) != 0; }

        inline void             SetOffset(long nMinutes)    { nOffset = nMinutes; }
        inline long             GetOffset() const           { return nOffset; }

        Date                    GetDate(sal_Bool bUseOffset = sal_False) const;
        Time                    GetTime(sal_Bool bUseOffset = sal_False) const;
        void                    SetDateTime(const DateTime& rDT);
        static double           GetDateTime(SwDoc* pDoc, const DateTime& rDT);

        virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nMId ) const;
        virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nMId );
};

#endif // SW_FLDDAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _FLDDAT_HXX
#define _FLDDAT_HXX


#include "fldbas.hxx"
class DateTime; 
namespace binfilter {



/*--------------------------------------------------------------------
    Beschreibung: Formate
 --------------------------------------------------------------------*/

enum SwDateFormat
{
    DF_BEGIN,
    //neue Formate:
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
//	Kompatibilitaet:
    DF_SSYS 	= DFF_SSYS,
    DF_LSYS 	= DFF_LSYS,
    DF_SHORT 	= DFF_DMY,
    DF_SCENT 	= DFF_DMYY,
    DF_LMON		= DFF_DMMYY,
    DF_LMONTH	= DFF_DMMMYY,
    DF_LDAYMON	= DFF_DDMMMYY,
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

/*--------------------------------------------------------------------
    Beschreibung: Datum/Uhrzeitfeld
 --------------------------------------------------------------------*/

class SwDateTimeFieldType : public SwValueFieldType
{
public:
        SwDateTimeFieldType(SwDoc* pDoc);

        virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Datum/Uhrzeitfeld
 --------------------------------------------------------------------*/

class SwDateTimeField : public SwValueField
{
        USHORT				nSubType;
        long				nOffset;	// Offset in Minuten

public:
        SwDateTimeField(SwDateTimeFieldType* pType, USHORT nSubType = DATEFLD,
                    ULONG nFmt = 0, USHORT nLng = 0);

        virtual USHORT			GetSubType() const;
/*N*/ 		virtual void			SetSubType(USHORT nSub); //SW40.SDW 

        virtual double			GetValue() const;


        inline BOOL				IsDate() const
                                { return (nSubType & DATEFLD) != 0; }

        inline void				SetOffset(long nMinutes)	{ nOffset = nMinutes; }
        inline long				GetOffset() const			{ return nOffset; }

        Date                    GetDate(BOOL bUseOffset = FALSE) const;
        Time                    GetTime(BOOL bUseOffset = FALSE) const;
        void                    SetDateTime(const DateTime& rDT);
        static double           GetDateTime(SwDoc* pDoc, const DateTime& rDT);

        virtual String 			Expand() const;
        virtual SwField* 		Copy() const;

    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

} //namespace binfilter
#endif // _FLDDAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

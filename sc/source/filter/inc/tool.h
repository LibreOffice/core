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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_TOOL_H
#define INCLUDED_SC_SOURCE_FILTER_INC_TOOL_H

#include <attrib.hxx>
#include <document.hxx>
#include <osl/diagnose.h>

// Defaultwerte
const sal_uInt8 nDezStd = 0;        // Dezimalstellen fuer Standard-Zellen
const sal_uInt8 nDezFloat = 2;  //        "         "  Float-Zellen

struct LotusContext;

void        PutFormString(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Char *pString);

void        SetFormat(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt8 nFormat, sal_uInt8 nSt);

double      SnumToDouble( sal_Int16 nVal );

double      Snum32ToDouble( sal_uInt32 nValue );

typedef sal_uInt16 StampTyp;

#define MAKE_STAMP(nF,nS) ((nS&0x0F)+((nF&0x7F)*16))
            // Bit 0...3  = Bit 0...3 von Stellenzahl
            // Bit 4...10 = Bit 0...6 von Formatbyte

class FormIdent
{
private:
    StampTyp        nStamp;         // Identifikations-Schluessel
    SfxUInt32Item*  pAttr;          // zugehoeriges Attribut
public:
                    FormIdent( void )
                    {
                        nStamp = 0;
                        pAttr = NULL;
                    }

                    FormIdent( sal_uInt8 nFormat, sal_uInt8 nSt, SfxUInt32Item& rAttr )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                        pAttr = &rAttr;
                    }

                    FormIdent( sal_uInt8 nFormat, sal_uInt8 nSt )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                        pAttr = NULL;
                    }

    bool            operator ==( const FormIdent& rComp ) const
                    {
                        return ( nStamp == rComp.nStamp );
                    }

    bool            operator ==( const StampTyp& rStamp ) const
                    {
                        return ( nStamp == rStamp );
                    }

    StampTyp        GetStamp( void ) const
                    {
                        return nStamp;
                    }

    SfxUInt32Item*  GetAttr( void )
                    {
                        return pAttr;
                    }

    void            SetStamp( sal_uInt8 nFormat, sal_uInt8 nSt )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                    }
};


#define __nSize 2048


class FormCache
{
private:
    FormIdent           aIdents[ __nSize ]; //gepufferte Formate
    sal_Bool                bValid[ __nSize ];
    FormIdent           aCompareIdent;      // zum Vergleichen
    sal_uInt8               nDefaultFormat;     // Defaultformat der Datei
    SvNumberFormatter*  pFormTable;         // Value-Format-Table-Anker
    StampTyp            nIndex;
    LanguageType        eLanguage;          // Systemsprache

    SfxUInt32Item*      NewAttr( sal_uInt8 nFormat, sal_uInt8 nSt );
public:
                        FormCache( ScDocument*, sal_uInt8 nNewDefaultFormat = 0xFF );
                        ~FormCache();

    inline const SfxUInt32Item* GetAttr( sal_uInt8 nFormat, sal_uInt8 nSt );
};


inline const SfxUInt32Item* FormCache::GetAttr( sal_uInt8 nFormat, sal_uInt8 nSt )
{
    // PREC:    nFormat = Lotus-Format-Byte
    //          nSt = Stellenzahl
    // POST:    return = zu nFormat und nSt passendes SC-Format
    SfxUInt32Item*      pAttr;
    SfxUInt32Item*      pRet;

    aCompareIdent.SetStamp( nFormat, nSt );
    nIndex = aCompareIdent.GetStamp();
    OSL_ENSURE( nIndex < __nSize, "FormCache::GetAttr(): Oups... not this way!" );
    if( bValid[ nIndex ] )
        pRet = aIdents[ nIndex ].GetAttr();
    else
    {
        // neues Attribut anlegen
        pAttr = NewAttr( nFormat, nSt );
        OSL_ENSURE( pAttr, "FormCache::GetAttr(): Nothing to save" );

        aIdents[ nIndex ] = FormIdent( nFormat, nSt, *pAttr );
        bValid[ nIndex ] = sal_True;

        pRet = pAttr;
    }
    return pRet;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#pragma once

#include <document.hxx>

#include <i18nlangtag/lang.h>
#include <svl/intitem.hxx>
#include <types.hxx>
#include <osl/diagnose.h>

// Default values
const sal_uInt8 nFractionalStd = 0;        // Number of digits in fractional part for standard cells
const sal_uInt8 nFractionalFloat = 2;      //        "         "         "         "  float cells

struct LotusContext;

void        PutFormString(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, char *pString);

void        SetFormat(LotusContext& rContext, SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt8 nFormat, sal_uInt8 nSt);

double      SnumToDouble( sal_Int16 nVal );

double      Snum32ToDouble( sal_uInt32 nValue );

typedef sal_uInt16 StampTyp;

#define MAKE_STAMP(nF,nS) ((nS&0x0F)+((nF&0x7F)*16))
            // Bit 0...3  = Bit 0...3 of number of digits
            // Bit 4...10 = Bit 0...6 of Formatbyte

class FormIdent
{
private:
    StampTyp        nStamp;         // ID key
    std::unique_ptr<SfxUInt32Item> pAttr;          // associated attribute
public:
                    FormIdent( void )
                    {
                        nStamp = 0;
                        pAttr = nullptr;
                    }

                    FormIdent( sal_uInt8 nFormat, sal_uInt8 nSt, SfxUInt32Item& rAttr )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                        pAttr.reset(&rAttr);
                    }

    StampTyp        GetStamp( void ) const
                    {
                        return nStamp;
                    }

    SfxUInt32Item*  GetAttr( void )
                    {
                        return pAttr.get();
                    }

    void            SetStamp( sal_uInt8 nFormat, sal_uInt8 nSt )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                    }
};


#define nSize_ 2048


class FormCache
{
private:
    FormIdent           aIdents[ nSize_ ]; //buffered formats
    bool                bValid[ nSize_ ];
    FormIdent           aCompareIdent;      // for comparing
    SvNumberFormatter*  pFormTable;         // value format table anchor
    StampTyp            nIndex;
    LanguageType        eLanguage;          // System language

    SfxUInt32Item*      NewAttr( sal_uInt8 nFormat, sal_uInt8 nSt );
public:
                        FormCache( const ScDocument* );
                        ~FormCache();

    inline const SfxUInt32Item* GetAttr( sal_uInt8 nFormat, sal_uInt8 nSt );
};


inline const SfxUInt32Item* FormCache::GetAttr( sal_uInt8 nFormat, sal_uInt8 nSt )
{
    // PREC:    nFormat = Lotus format byte
    //          nSt = Number of digit
    // POST:    return = SC-format fitting nFormat and nSt
    SfxUInt32Item*      pAttr;
    SfxUInt32Item*      pRet;

    aCompareIdent.SetStamp( nFormat, nSt );
    nIndex = aCompareIdent.GetStamp();
    OSL_ENSURE( nIndex < nSize_, "FormCache::GetAttr(): Oups... not this way!" );
    if( bValid[ nIndex ] )
        pRet = aIdents[ nIndex ].GetAttr();
    else
    {
        // create new attribute
        pAttr = NewAttr( nFormat, nSt );
        assert(pAttr && "FormCache::GetAttr(): Nothing to save");

        aIdents[ nIndex ] = FormIdent( nFormat, nSt, *pAttr );
        bValid[ nIndex ] = true;

        pRet = pAttr;
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

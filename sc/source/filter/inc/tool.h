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

#ifndef SC_TOOL_H
#define SC_TOOL_H

#include <attrib.hxx>    //!!! noch noetig?????
#include <document.hxx>

// Defaultwerte
const sal_uInt8 nDezStd = 0;        // Dezimalstellen fuer Standard-Zellen
const sal_uInt8 nDezFloat = 2;  //        "         "  Float-Zellen

void        PutFormString( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Char *pString );

void        SetFormat( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt8 nFormat, sal_uInt8 nSt );

void        InitPage( void );

String      DosToSystem( sal_Char *pSource );

double      SnumToDouble( sal_Int16 nVal );

double          Snum32ToDouble( sal_uInt32 nValue );

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

    sal_Bool            operator ==( const FormIdent& rComp ) const
                    {
                        return ( nStamp == rComp.nStamp );
                    }

    sal_Bool            operator ==( const StampTyp& rStamp ) const
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
    void                SetDefaultFormat( sal_uInt8 nD = 0xFF )
                        {
                            nDefaultFormat = nD;
                        }
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
    DBG_ASSERT( nIndex < __nSize, "FormCache::GetAttr(): Uuuuuuups... so nicht!" );
    if( bValid[ nIndex ] )
        pRet = aIdents[ nIndex ].GetAttr();
    else
    {
        // neues Attribut anlegen
        pAttr = NewAttr( nFormat, nSt );
        DBG_ASSERT( pAttr, "FormCache::GetAttr(): Nix Speicherus" );

        aIdents[ nIndex ] = FormIdent( nFormat, nSt, *pAttr );
        bValid[ nIndex ] = sal_True;

        pRet = pAttr;
    }
    return pRet;
}

#endif


/*************************************************************************
 *
 *  $RCSfile: tool.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOL_H
#define _TOOL_H

#include <attrib.hxx>    //!!! noch noetig?????
#include <document.hxx>

// Defaultwerte
const BYTE  nDezStd = 0;        // Dezimalstellen fuer Standard-Zellen
const BYTE  nDezFloat = 2;  //        "         "  Float-Zellen

void        PutFormString( UINT16 nCol, UINT16 nRow, UINT16 nTab, sal_Char *pString );

void        SetFormat( UINT16 nCol, UINT16 nRow, UINT16 nTab, BYTE nFormat, BYTE nSt );

sal_Char*   ReplaceWith( sal_Char* pString, sal_Char cVon, sal_Char cNach );

void        InitPage( void );

String      DosToSystem( sal_Char *pSource );

double      SnumToDouble( INT16 nVal );

typedef UINT16 StampTyp;

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

                    FormIdent( BYTE nFormat, BYTE nSt, SfxUInt32Item& rAttr )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                        pAttr = &rAttr;
                    }

                    FormIdent( BYTE nFormat, BYTE nSt )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                        pAttr = NULL;
                    }

    BOOL            operator ==( const FormIdent& rComp ) const
                    {
                        return ( nStamp == rComp.nStamp );
                    }

    BOOL            operator ==( const StampTyp& rStamp ) const
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

    void            SetStamp( BYTE nFormat, BYTE nSt )
                    {
                        nStamp = MAKE_STAMP( nFormat, nSt );
                    }
};


#define __nSize 2048




class FormCache
{
private:
    FormIdent           aIdents[ __nSize ]; //gepufferte Formate
    BOOL                bValid[ __nSize ];
    FormIdent           aCompareIdent;      // zum Vergleichen
    BYTE                nDefaultFormat;     // Defaultformat der Datei
    SvNumberFormatter*  pFormTable;         // Value-Format-Table-Anker
    StampTyp            nIndex;
    LanguageType        eLanguage;          // Systemsprache

    SfxUInt32Item*      NewAttr( BYTE nFormat, BYTE nSt );
public:
                        FormCache( ScDocument*, BYTE nNewDefaultFormat = 0xFF );
                        ~FormCache();

    inline const SfxUInt32Item* GetAttr( BYTE nFormat, BYTE nSt );
    void                SetDefaultFormat( BYTE nD = 0xFF )
                        {
                            nDefaultFormat = nD;
                        }
};


inline const SfxUInt32Item* FormCache::GetAttr( BYTE nFormat, BYTE nSt )
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
        bValid[ nIndex ] = TRUE;

        pRet = pAttr;
    }
    return pRet;
}

#endif


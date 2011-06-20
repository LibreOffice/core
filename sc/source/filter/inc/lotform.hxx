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

#ifndef SC_LOTFORM_HXX
#define SC_LOTFORM_HXX

#include "formel.hxx"
#include <tools/string.hxx>




enum FUNC_TYPE
{
    FT_Return = 0,  // End Formula
    FT_FuncFix0,    // Funktion, 0 Parameter
    FT_FuncFix1,    // Funktion, 0 Parameter
    FT_FuncFix2,    // Funktion, 0 Parameter
    FT_FuncFix3,    // Funktion, 0 Parameter
    FT_FuncFix4,    // Funktion, 0 Parameter
    FT_FuncVar,     // ~, var. P.
    FT_Neg,         // Negierung
    FT_Op,          // Operator
    FT_NotImpl,     // nicht implementiert
    FT_ConstFloat,  // Double (8-Byte)
    FT_Variable,    // Single Ref
    FT_Range,       // Double Ref
    FT_Braces,      // Klammmern
    FT_ConstInt,    // Integer
    FT_ConstString, // String
    FT_NOP,         // nichts
    // zusaetzlich ab WK3
    FT_Cref,        // Cell Reference
    FT_Rref,        // Range Reference
    FT_Nrref,       // Named range reference
    FT_Absnref,     // Absolut named range
    FT_Erref,       // Err range reference
    FT_Ecref,       // Err cell reference
    FT_Econstant,   // Err constant
    FT_Splfunc,     // SPLfunction
    FT_Const10Float,// Float (10-Byte)
    FT_Snum         // Const Short Num
    // fuer 'Problemfaelle' beim Import
};




class LotusToSc : public LotusConverterBase
{
private:
    CharSet             eSrcChar;
    TokenId             nAddToken;  // ')+1.0'
    TokenId             nSubToken;  // ~
    TokenId             n0Token;    // '0.0';
    // ---------------------------------------------------------------
    static FUNC_TYPE    IndexToType( sal_uInt8 );
    static DefTokenId   IndexToToken( sal_uInt8 );
    static FUNC_TYPE    IndexToTypeWK123( sal_uInt8 );
    static DefTokenId   IndexToTokenWK123( sal_uInt8 );
    void                DoFunc( DefTokenId eOc, sal_uInt8 nAnz, const sal_Char* pExtName );
    void                LotusRelToScRel( sal_uInt16 nCol, sal_uInt16 nRow,
                            ScSingleRefData& rSRD );
    sal_Bool                bWK3;       // alternative Codeumsetzung statt fuer < WK1
        sal_Bool                            bWK123;         // alternative for 123
    // -------------------------------------------------------------------
    void                ReadSRD( ScSingleRefData& rSRD, sal_uInt8 nFlags );
    inline void         ReadCRD( ScComplexRefData& rCRD, sal_uInt8 nFlags );
    void                IncToken( TokenId &rParam );
                        // ACHTUNG: hier wird die aktuelle Token-Kette im Pool
                        // mit '(<rParam>)+1' fortgeschrieben und mit
                        // Store() abgeschlossen!
    void                DecToken( TokenId& rParam );
                        // ACHTUNG: ~
    void                NegToken( TokenId& rParam );
                        // ACHTUNG: wie ~, nur wird '-(<rParam>)' gebildet
public:
                        LotusToSc( SvStream& aStr, CharSet eSrc, sal_Bool b );
    virtual ConvErr     Convert( const ScTokenArray*& rpErg, sal_Int32& nRest,
                                    const FORMULA_TYPE eFT = FT_CellFormula );

    void                Reset( const ScAddress& rEingPos );
    inline void         SetWK3( void );

private:
    using               LotusConverterBase::Reset;
};


inline void LotusToSc::ReadCRD( ScComplexRefData& rCRD, sal_uInt8 nRelBit )
{
    // erster Teil
    ReadSRD( rCRD.Ref1, nRelBit );

    // zweiter Teil
    ReadSRD( rCRD.Ref2, nRelBit >> 3 );
}


inline void LotusToSc::SetWK3( void )
{
        bWK3 = sal_True;
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

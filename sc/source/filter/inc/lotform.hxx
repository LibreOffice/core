/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lotform.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 12:34:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _LOTFORM_HXX
#define _LOTFORM_HXX

#ifndef _FORMEL_HXX
#include "formel.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif




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
    static FUNC_TYPE    IndexToType( BYTE );
    static DefTokenId   IndexToToken( BYTE );
    static FUNC_TYPE    IndexToTypeWK123( BYTE );
    static DefTokenId   IndexToTokenWK123( BYTE );
    void                DoFunc( DefTokenId eOc, BYTE nAnz, const sal_Char* pExtName );
    void                LotusRelToScRel( UINT16 nCol, UINT16 nRow,
                            SingleRefData& rSRD );
    BOOL                bWK3;       // alternative Codeumsetzung statt fuer < WK1
        BOOL                            bWK123;         // alternative for 123
    // -------------------------------------------------------------------
    void                ReadSRD( SingleRefData& rSRD, BYTE nFlags );
    inline void         ReadCRD( ComplRefData& rCRD, BYTE nFlags );
    void                IncToken( TokenId &rParam );
                        // ACHTUNG: hier wird die aktuelle Token-Kette im Pool
                        // mit '(<rParam>)+1' fortgeschrieben und mit
                        // Store() abgeschlossen!
    void                DecToken( TokenId& rParam );
                        // ACHTUNG: ~
    void                NegToken( TokenId& rParam );
                        // ACHTUNG: wie ~, nur wird '-(<rParam>)' gebildet
public:
                        LotusToSc( SvStream& aStr, CharSet eSrc, BOOL b );
    virtual ConvErr     Convert( const ScTokenArray*& rpErg, INT32& nRest,
                                    const FORMULA_TYPE eFT = FT_CellFormula );

    void                Reset( const ScAddress& rEingPos );
    inline void         SetWK3( void );

private:
    using               LotusConverterBase::Reset;
};


inline void LotusToSc::ReadCRD( ComplRefData& rCRD, BYTE nRelBit )
{
    // erster Teil
    ReadSRD( rCRD.Ref1, nRelBit );

    // zweiter Teil
    ReadSRD( rCRD.Ref2, nRelBit >> 3 );
}


inline void LotusToSc::SetWK3( void )
{
        bWK3 = TRUE;
}



#endif


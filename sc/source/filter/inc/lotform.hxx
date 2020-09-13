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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_LOTFORM_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_LOTFORM_HXX

#include "formel.hxx"

enum FUNC_TYPE
{
    FT_Return = 0,  // End Formula
    FT_FuncFix0,    // Function, 0 Parameter
    FT_FuncFix1,    // Function, 0 Parameter
    FT_FuncFix2,    // Function, 0 Parameter
    FT_FuncFix3,    // Function, 0 Parameter
    FT_FuncFix4,    // Function, 0 Parameter
    FT_FuncVar,     // ~, var. P.
    FT_Neg,         // Negation
    FT_Op,          // Operator
    FT_NotImpl,     // not implemented
    FT_ConstFloat,  // Double (8-Byte)
    FT_Variable,    // Single Ref
    FT_Range,       // Double Ref
    FT_Braces,      // Braces
    FT_ConstInt,    // Integer
    FT_ConstString, // String
    FT_NOP,         // nothing
    // additionally since WK3
    FT_Cref,        // Cell Reference
    FT_Rref,        // Range Reference
    FT_Nrref,       // Named range reference
    FT_Absnref,     // Absolute named range
    FT_Erref,       // Err range reference
    FT_Ecref,       // Err cell reference
    FT_Econstant,   // Err constant
    FT_Splfunc,     // SPLfunction
    FT_Const10Float,// Float (10-Byte)
    FT_Snum         // Const Short Num
    // for 'Problem Cases' during Import
};

struct LotusContext;

class LotusToSc : public LotusConverterBase
{
private:
    LotusContext&       m_rContext;
    rtl_TextEncoding    eSrcChar;
    TokenId             nAddToken;  // ')+1.0'
    TokenId             nSubToken;  // ~
    TokenId             n0Token;    // '0.0';

    static FUNC_TYPE    IndexToType( sal_uInt8 );
    static DefTokenId   IndexToToken( sal_uInt8 );
    static FUNC_TYPE    IndexToTypeWK123( sal_uInt8 );
    static DefTokenId   IndexToTokenWK123( sal_uInt8 );
    void                DoFunc( DefTokenId eOc, sal_uInt8 nCnt, const char* pExtName );
    void                LotusRelToScRel(sal_uInt16 nCol, sal_uInt16 nRow, ScSingleRefData& rSRD);
    bool                bWK3;       // alternative Code translation for < WK1
    bool                bWK123;     // alternative for 123

    void                ReadSRD( const ScDocument* pDoc, ScSingleRefData& rSRD, sal_uInt8 nFlags );
    inline void         ReadCRD( const ScDocument& rDoc, ScComplexRefData& rCRD, sal_uInt8 nFlags );
    void                IncToken( TokenId &rParam );
                        // Attention: here the Token-chain is extended in Pool
                        // with '(<rParam>)+1' and finished with Store() !

    void                DecToken( TokenId& rParam );
                        // Attention: ~
    void                NegToken( TokenId& rParam );
                        // Attention: like ~, but with '-(<rParam>)'
public:
    LotusToSc(LotusContext &rContext, SvStream& aStr, svl::SharedStringPool& rSPool, rtl_TextEncoding eSrc, bool b);

    virtual void        Convert( std::unique_ptr<ScTokenArray>& rpErg, sal_Int32& nRest ) override;

    void                Reset( const ScAddress& rEingPos );
    inline void         SetWK3();
    LotusContext&       getContext() { return m_rContext; }

private:
    using               LotusConverterBase::Reset;
};

inline void LotusToSc::ReadCRD( const ScDocument& rDoc, ScComplexRefData& rCRD, sal_uInt8 nRelBit )
{
    // 1st part
    ReadSRD( &rDoc, rCRD.Ref1, nRelBit );

    // 2nd part
    ReadSRD( &rDoc, rCRD.Ref2, nRelBit >> 3 );
}

inline void LotusToSc::SetWK3()
{
        bWK3 = true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

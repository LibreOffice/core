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


#ifndef SC_QPROFORM_HXX
#define SC_QPROFORM_HXX

#include <sal/config.h>
#include "formel.hxx"
#include <tools/string.hxx>
#include "qpro.hxx"

#include <compiler.hxx>
typedef OpCode DefTokenId;

enum FUNC_TYPE
{
    FT_Return,
    FT_FuncFix0,
    FT_FuncFix1,
    FT_FuncFix2,
    FT_FuncFix3,
    FT_FuncFix4,
    FT_FuncFix5,
    FT_FuncFix6,
    FT_FuncVar,
    FT_DLL,
    FT_Neg,
    FT_Op,
    FT_NotImpl,
    FT_ConstFloat,
    FT_Range,
    FT_Braces,
    FT_ConstInt,
    FT_ConstString,
    FT_NOP,
    FT_Cref
};

class QProToSc : public ConverterBase
{
    private:
    TokenId mnAddToken;
    TokenId mnSubToken;
    TokenId mn0Token;
    SvStream& maIn;

    public:
    static const size_t nBufSize = 256;
    QProToSc( SvStream &aStr, const ScAddress& rRefPos );
    ~QProToSc(){ };
    ConvErr Convert( const ScTokenArray*& pArray, sal_uInt16 nLen,
                                                                         const FORMULA_TYPE eFT = FT_CellFormula );
    void DoFunc( DefTokenId eOc, sal_uInt16 nArgs, const sal_Char* pExtString );
    void ReadSRD( ScSingleRefData& rR, sal_Int8 nPage, sal_Int8 nCol, sal_uInt16 rRel );
    void IncToken( TokenId &aParam );
    DefTokenId IndexToToken( sal_uInt16 nToken );
    FUNC_TYPE IndexToType( sal_uInt8 nToken );
    DefTokenId IndexToDLLId( sal_uInt16 nIndex );
    const sal_Char* getString( sal_uInt8 nIndex );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

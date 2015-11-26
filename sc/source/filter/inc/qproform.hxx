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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_QPROFORM_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_QPROFORM_HXX

#include <sal/config.h>
#include "formel.hxx"
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
    SvStream& maIn;

public:
    static const size_t nBufSize = 256;
    QProToSc( SvStream &aStr, svl::SharedStringPool& rSPool, const ScAddress& rRefPos );
    virtual ~QProToSc(){ };
    ConvErr Convert( const ScTokenArray*& pArray, sal_uInt16 nLen,
                                                                         const FORMULA_TYPE eFT = FT_CellFormula );
    void DoFunc( DefTokenId eOc, sal_uInt16 nArgs, const sal_Char* pExtString );
    void ReadSRD( ScSingleRefData& rR, sal_Int8 nPage, sal_Int8 nCol, sal_uInt16 rRel );
    void IncToken( TokenId &aParam );
    static DefTokenId IndexToToken( sal_uInt16 nToken );
    static FUNC_TYPE IndexToType( sal_uInt8 nToken );
    static DefTokenId IndexToDLLId( sal_uInt16 nIndex );
    static const sal_Char* getString( sal_uInt8 nIndex );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

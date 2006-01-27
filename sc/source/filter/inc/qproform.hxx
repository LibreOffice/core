/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: qproform.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-01-27 15:50:18 $
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


#ifndef SC_QPROFORM_HXX
#define SC_QPROFORM_HXX

#include <sal/config.h>
#ifndef _FORMEL_HXX
#include "formel.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef SC_QPRO_HXX
#include "qpro.hxx"
#endif

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
    void ReadSRD( SingleRefData& rR, sal_Int8 nPage, sal_Int8 nCol, sal_uInt16 rRel );
    void IncToken( TokenId &aParam );
    void DecToken( TokenId& aParam );
    void NegToken( TokenId& aParam );
    DefTokenId IndexToToken( sal_uInt16 nToken );
    FUNC_TYPE IndexToType( sal_uInt8 nToken );
    DefTokenId IndexToDLLId( sal_uInt16 nIndex );
    const sal_Char* getString( sal_uInt8 nIndex );
};
#endif

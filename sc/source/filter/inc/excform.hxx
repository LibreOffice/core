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

#ifndef SC_EXCFORM_HXX
#define SC_EXCFORM_HXX

#include "xlformula.hxx"
#include "xiroot.hxx"
#include "formel.hxx"

#include <vector>

class ScRangeList;


class ExcelToSc : public ExcelConverterBase, protected XclImpRoot
{
protected:
    enum ExtensionType { EXTENSION_ARRAY, EXTENSION_NLR, EXTENSION_MEMAREA };
    typedef ::std::vector< ExtensionType >          ExtensionTypeVec;

    sal_Bool                bExternName;    // wenn External Name gefunden wurde
    static const sal_uInt16 nRowMask;
    static const sal_uInt16 nLastInd;       // letzter Index fuer Excel->SC-
                                        // Token Umsetzung
    XclFunctionProvider maFuncProv;
    const XclBiff       meBiff;

    // ---------------------------------------------------------------
    void                DoMulArgs( DefTokenId eId, sal_uInt8 nNumArgs, sal_uInt8 mnMinParamCount = 0 );

    void                ExcRelToScRel( sal_uInt16 nRow, sal_uInt8 nCol, ScSingleRefData&, const sal_Bool bName );

public:
                        ExcelToSc( const XclImpRoot& rRoot );
    virtual             ~ExcelToSc();
    virtual ConvErr     Convert( const ScTokenArray*&, XclImpStream& rStrm, sal_Size nFormulaLen,
                                 bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     ConvertExternName( const ScTokenArray*& rpArray, XclImpStream& rStrm, sal_Size nFormulaLen,
                                           const String& rUrl, const ::std::vector<String>& rTabNames );

    virtual sal_Bool        GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, sal_Size nLen );

    void                GetDummy( const ScTokenArray*& );
    const ScTokenArray* GetBoolErr( XclBoolError );
    sal_Bool                GetShrFmla( const ScTokenArray*&, XclImpStream& rStrm, sal_Size nFormulaLen );

#if 0
                            // return = sal_True -> String-Record folgt!
    static sal_Bool         SetCurVal( ScFormulaCell& rCell, double& rCurVal );
#endif
    static void         SetError( ScFormulaCell& rCell, const ConvErr eErr );

    static inline sal_Bool  IsComplColRange( const sal_uInt16 nCol1, const sal_uInt16 nCol2 );
    static inline sal_Bool  IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 );

    void                SetComplCol( ScComplexRefData& );
    void                SetComplRow( ScComplexRefData& );

    void                ReadExtensions( const ExtensionTypeVec& rExtensions,
                                        XclImpStream& aIn );
    void                ReadExtensionArray( unsigned int n,
                                            XclImpStream& aIn );
    void                ReadExtensionNlr( XclImpStream& aIn );
    void                ReadExtensionMemArea( XclImpStream& aIn );
};


inline sal_Bool ExcelToSc::IsComplColRange( const sal_uInt16 nCol1, const sal_uInt16 nCol2 )
{
    return ( nCol1 == 0x00 ) && ( nCol2 == 0xFF );
}


inline sal_Bool ExcelToSc::IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 )
{
    return ( ( nRow1 & 0x3FFF ) == 0x0000 ) && ( ( nRow2 & 0x3FFF ) == 0x3FFF );
}

// ============================================================================

class XclImpLinkManager;

class ExcelToSc8 : public ExcelToSc
{
public:

    struct ExternalTabInfo
    {
        String      maTabName;
        sal_uInt16  mnFileId;
        bool        mbExternal;

        ExternalTabInfo();
    };

private:
    const XclImpLinkManager&    rLinkMan;

    void                ExcRelToScRel8( sal_uInt16 nRow, sal_uInt16 nCol, ScSingleRefData&,
                            const sal_Bool bName );

    bool                GetExternalFileIdFromXti( sal_uInt16 nIxti, sal_uInt16& rFileId ) const;

    virtual bool        Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo );

public:
                        ExcelToSc8( const XclImpRoot& rRoot );
    virtual             ~ExcelToSc8();

    virtual ConvErr     Convert( const ScTokenArray*& rpTokArray, XclImpStream& rStrm, sal_Size nFormulaLen, bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     ConvertExternName( const ScTokenArray*& rpArray, XclImpStream& rStrm, sal_Size nFormulaLen,
                                           const String& rUrl, const ::std::vector<String>& rTabNames );

    static inline sal_Bool  IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 );

    virtual sal_Bool        GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, sal_Size nLen );
};


inline sal_Bool ExcelToSc8::IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 )
{
    return ( nRow1 == 0x0000 ) && ( nRow2 == 0xFFFF );
}





#endif

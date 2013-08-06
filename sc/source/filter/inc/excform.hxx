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

#ifndef SC_EXCFORM_HXX
#define SC_EXCFORM_HXX

#include "xlformula.hxx"
#include "xiroot.hxx"
#include "formel.hxx"

#include <vector>

class ScFormulaCell;
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
    void                DoMulArgs( DefTokenId eId, sal_uInt8 nNumArgs );

    void                ExcRelToScRel( sal_uInt16 nRow, sal_uInt8 nCol, ScSingleRefData&, const sal_Bool bName );

public:
                        ExcelToSc( const XclImpRoot& rRoot );
    virtual             ~ExcelToSc();
    virtual ConvErr     Convert( const ScTokenArray*&, XclImpStream& rStrm, sal_Size nFormulaLen,
                                 bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen, SCsTAB nTab, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     ConvertExternName( const ScTokenArray*& rpArray, XclImpStream& rStrm, sal_Size nFormulaLen,
                                           const String& rUrl, const ::std::vector<String>& rTabNames );

    virtual sal_Bool        GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, sal_Size nLen );

    void                GetDummy( const ScTokenArray*& );
    const ScTokenArray* GetBoolErr( XclBoolError );
    ScFormulaCellGroupRef GetSharedFormula( XclImpStream& rStrm, SCCOL nCol, sal_Size nFormulaLen );

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
class XclImpExtName;

class ExcelToSc8 : public ExcelToSc
{
public:

    struct ExternalTabInfo
    {
        ScRange         maRange;
        OUString maTabName;
        sal_uInt16      mnFileId;
        bool            mbExternal;

        ExternalTabInfo();
    };

private:
    const XclImpLinkManager&    rLinkMan;

    void                ExcRelToScRel8( sal_uInt16 nRow, sal_uInt16 nCol, ScSingleRefData&,
                            const sal_Bool bName );

    bool                GetExternalFileIdFromXti( sal_uInt16 nIxti, sal_uInt16& rFileId ) const;

    virtual bool        Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo );

    bool                HandleOleLink(sal_uInt16 nXtiIndex, const XclImpExtName& rExtName, ExternalTabInfo& rExtInfo);
public:
                        ExcelToSc8( const XclImpRoot& rRoot );
    virtual             ~ExcelToSc8();

    virtual ConvErr     Convert( const ScTokenArray*& rpTokArray, XclImpStream& rStrm, sal_Size nFormulaLen, bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen, SCsTAB nTab, const FORMULA_TYPE eFT = FT_CellFormula );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

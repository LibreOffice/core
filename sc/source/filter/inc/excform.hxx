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

#pragma once

#include "xlformula.hxx"
#include "xiroot.hxx"
#include "xltools.hxx"
#include "formel.hxx"

#include <vector>

class ScFormulaCell;
class ScRangeList;

class ExcelToSc : public ExcelConverterBase, protected XclImpRoot
{
protected:
    enum ExtensionType { EXTENSION_ARRAY, EXTENSION_NLR, EXTENSION_MEMAREA };
    typedef ::std::vector< ExtensionType >          ExtensionTypeVec;

    static const sal_uInt16 nRowMask;

    XclFunctionProvider maFuncProv;
    const XclBiff       meBiff;

    void                DoMulArgs( DefTokenId eId, sal_uInt8 nNumArgs );

    void                ExcRelToScRel( sal_uInt16 nRow, sal_uInt8 nCol, ScSingleRefData&, const bool bName );

public:
    ExcelToSc( XclImpRoot& rRoot );
    virtual             ~ExcelToSc() override;
    virtual ConvErr     Convert( std::unique_ptr<ScTokenArray>&, XclImpStream& rStrm, std::size_t nFormulaLen,
                                 bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula ) override;

    virtual ConvErr     Convert( ScRangeListTabs&, XclImpStream& rStrm, std::size_t nFormulaLen, SCTAB nTab, const FORMULA_TYPE eFT = FT_CellFormula ) override;

    virtual void        ConvertExternName( std::unique_ptr<ScTokenArray>& rpArray, XclImpStream& rStrm, std::size_t nFormulaLen,
                                           const OUString& rUrl, const ::std::vector<OUString>& rTabNames );

    virtual void        GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, std::size_t nLen );

    std::unique_ptr<ScTokenArray> GetDummy();
    std::unique_ptr<ScTokenArray> GetBoolErr( XclBoolError );

    static bool ReadSharedFormulaPosition( XclImpStream& rStrm, SCCOL& rCol, SCROW& rRow );
    const ScTokenArray* GetSharedFormula( const ScAddress& rRefPos ) const;

    static void         SetError( ScFormulaCell& rCell, const ConvErr eErr );

    static inline bool  IsComplColRange( const sal_uInt16 nCol1, const sal_uInt16 nCol2 );
    static inline bool  IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 );

    void                SetComplCol( ScComplexRefData& );
    void                SetComplRow( ScComplexRefData& );

    void                ReadExtensions( const ExtensionTypeVec& rExtensions,
                                        XclImpStream& aIn );
    void                ReadExtensionArray( unsigned int n,
                                            XclImpStream& aIn );
    static void         ReadExtensionNlr( XclImpStream& aIn );
    void                ReadExtensionMemArea( XclImpStream& aIn );
};

inline bool ExcelToSc::IsComplColRange( const sal_uInt16 nCol1, const sal_uInt16 nCol2 )
{
    return ( nCol1 == 0x00 ) && ( nCol2 == 0xFF );
}

inline bool ExcelToSc::IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 )
{
    return ( ( nRow1 & 0x3FFF ) == 0x0000 ) && ( ( nRow2 & 0x3FFF ) == 0x3FFF );
}

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
                            const bool bName );

    bool                GetExternalFileIdFromXti( sal_uInt16 nIxti, sal_uInt16& rFileId ) const;

    virtual bool        Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo );

    bool                HandleOleLink(sal_uInt16 nXtiIndex, const XclImpExtName& rExtName, ExternalTabInfo& rExtInfo);
public:
    ExcelToSc8( XclImpRoot& rRoot );
    virtual             ~ExcelToSc8() override;

    virtual ConvErr     Convert( std::unique_ptr<ScTokenArray>& rpTokArray, XclImpStream& rStrm, std::size_t nFormulaLen, bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula ) override;

    virtual ConvErr     Convert( ScRangeListTabs&, XclImpStream& rStrm, std::size_t nFormulaLen, SCTAB nTab, const FORMULA_TYPE eFT = FT_CellFormula ) override;

    virtual void        ConvertExternName( std::unique_ptr<ScTokenArray>& rpArray, XclImpStream& rStrm, std::size_t nFormulaLen,
                                           const OUString& rUrl, const ::std::vector<OUString>& rTabNames ) override;

    static inline bool  IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 );

    virtual void        GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, std::size_t nLen ) override;
};

inline bool ExcelToSc8::IsComplRowRange( const sal_uInt16 nRow1, const sal_uInt16 nRow2 )
{
    return ( nRow1 == 0x0000 ) && ( nRow2 == 0xFFFF );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

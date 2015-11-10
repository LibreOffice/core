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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEFORMULA_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEFORMULA_HXX

#include "xlformula.hxx"
#include "xeroot.hxx"
#include <memory>

// External reference log =====================================================

/** Log entry for external references in a formula, used i.e. in change tracking. */
struct XclExpRefLogEntry
{
    const XclExpString* mpUrl;              /// URL of the document containing the first sheet.
    const XclExpString* mpFirstTab;         /// Name of the first sheet.
    const XclExpString* mpLastTab;          /// Name of the last sheet.
    sal_uInt16          mnFirstXclTab;      /// Calc index of the first sheet.
    sal_uInt16          mnLastXclTab;       /// Calc index of the last sheet.

    explicit            XclExpRefLogEntry();
};

/** Vector containing a log for all external references in a formula, used i.e. in change tracking. */
typedef ::std::vector< XclExpRefLogEntry > XclExpRefLog;

// Formula compiler ===========================================================

class ScRangeList;
class XclExpFmlaCompImpl;

/** The formula compiler to create Excel token arrays from Calc token arrays. */
class XclExpFormulaCompiler : protected XclExpRoot
{
public:
    explicit            XclExpFormulaCompiler( const XclExpRoot& rRoot );
    virtual             ~XclExpFormulaCompiler();

    /** Creates and returns the token array of a formula. */
    XclTokenArrayRef    CreateFormula(
                            XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos = nullptr, XclExpRefLog* pRefLog = nullptr );

    /** Creates and returns a token array containing a single cell address. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScAddress& rScPos );

    /** Creates and returns a token array containing a single cell range address. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScRange& rScRange );

    /** Creates and returns the token array for a cell range list. */
    XclTokenArrayRef    CreateFormula( XclFormulaType eType, const ScRangeList& rScRanges );

    /** Creates a single error token containing the passed error code. */
    XclTokenArrayRef    CreateErrorFormula( sal_uInt8 nErrCode );

    /** Creates a single token for a special cell reference.
        @descr  This is used for array formulas and shared formulas (token tExp),
            and multiple operation tables (token tTbl). */
    XclTokenArrayRef    CreateSpecialRefFormula( sal_uInt8 nTokenId, const XclAddress& rXclPos );

    /** Creates a single tNameXR token for a reference to an external name.
        @descr  This is used i.e. for linked macros in push buttons. */
    XclTokenArrayRef    CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName );

    bool IsRef2D( const ScSingleRefData& rRefData ) const;
    bool IsRef2D( const ScComplexRefData& rRefData ) const;

private:
    typedef std::shared_ptr< XclExpFmlaCompImpl > XclExpFmlaCompImplRef;
    XclExpFmlaCompImplRef mxImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

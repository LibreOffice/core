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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XINAME_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XINAME_HXX

#include "xlname.hxx"
#include "xiroot.hxx"
#include "xistream.hxx"

#include "rangenam.hxx"

#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>

class ScRangeData;
class ScTokenArray;

/** Represents a defined name. It may be related to a single sheet or global. */
class XclImpName : protected XclImpRoot, public boost::noncopyable
{
    struct TokenStrmData
    {
        XclImpStream& mrStrm;
        XclImpStreamPos maStrmPos;
        sal_Size mnStrmPos;
        sal_Size mnStrmSize;

        TokenStrmData( XclImpStream& rStrm );
    };

public:
    explicit            XclImpName( XclImpStream& rStrm, sal_uInt16 nXclNameIdx );

    inline const OUString& GetXclName() const { return maXclName; }
    inline const OUString& GetScName() const { return maScName; }
    inline SCTAB        GetScTab() const { return mnScTab; }
    inline const ScRangeData* GetScRangeData() const { return mpScData; }
    inline bool         IsGlobal() const { return mnScTab == SCTAB_MAX; }
    inline bool         IsVBName() const { return mbVBName; }
    bool IsMacro() const { return mbMacro; }
    void ConvertTokens();

private:
    void InsertName(const ScTokenArray* pArray);

    OUString            maXclName;      /// Original name read from the file.
    OUString            maScName;       /// Name inserted into the Calc document.
    const ScRangeData*  mpScData;       /// Pointer to Calc defined name (no ownership).
    sal_Unicode         mcBuiltIn;      /// Excel built-in name index.
    SCTAB               mnScTab;        /// Calc sheet index of local names.
    ScRangeData::Type   meNameType;
    sal_uInt16          mnXclTab;
    sal_uInt16          mnNameIndex;
    bool                mbVBName:1;     /// true = Visual Basic procedure or function.
    bool                mbMacro:1;      /// Whether it's a user-defined macro.

    std::unique_ptr<TokenStrmData> mpTokensData;   /// For later conversion of token array.
};

/** This buffer contains all internal defined names of the document.
    @descr  It manages the position of the names in the document, means if they are
    global or attached to a specific sheet. While inserting the names into the Calc
    document this buffer resolves conflicts caused by equal names from different
    sheets. */
class XclImpNameManager : protected XclImpRoot
{
public:
    explicit            XclImpNameManager( const XclImpRoot& rRoot );

    /** Reads a NAME record and creates an entry in this buffer. */
    void                ReadName( XclImpStream& rStrm );

    /** Tries to find the name used in Calc, based on the original Excel defined name.
        @param nScTab  The sheet index for local names or SCTAB_MAX for global names.
        If no local name is found, tries to find a matching global name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*   FindName( const OUString& rXclName, SCTAB nScTab = SCTAB_MAX ) const;

    /** Returns the defined name specified by its Excel index.
        @param nXclNameIdx  The index of the internal defined name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*   GetName( sal_uInt16 nXclNameIdx ) const;

    void ConvertAllTokens();

private:
    typedef std::vector< std::unique_ptr<XclImpName> > XclImpNameList;
    XclImpNameList      maNameList;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

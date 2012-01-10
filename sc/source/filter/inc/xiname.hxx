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

#ifndef SC_XINAME_HXX
#define SC_XINAME_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include "xlname.hxx"
#include "xiroot.hxx"
#include "xistream.hxx"

#include "rangenam.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

// ============================================================================

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

    inline const String& GetXclName() const { return maXclName; }
    inline const String& GetScName() const { return maScName; }
    inline SCTAB        GetScTab() const { return mnScTab; }
    inline const ScRangeData* GetScRangeData() const { return mpScData; }
    inline bool         IsGlobal() const { return mnScTab == SCTAB_MAX; }
    inline bool         IsVBName() const { return mbVBName; }
    bool IsMacro() const;
    void ConvertTokens();

private:
    void InsertName(const ScTokenArray* pArray);

    String              maXclName;      /// Original name read from the file.
    String              maScName;       /// Name inserted into the Calc document.
    const ScRangeData*  mpScData;       /// Pointer to Calc defined name (no ownership).
    sal_Unicode         mcBuiltIn;      /// Excel built-in name index.
    SCTAB               mnScTab;        /// Calc sheet index of local names.
    RangeType           meNameType;
    sal_uInt16          mnXclTab;
    sal_uInt16          mnNameIndex;
    bool                mbVBName:1;     /// true = Visual Basic procedure or function.
    bool                mbMacro:1;      /// Whether it's a user-defined macro.

    boost::scoped_ptr<TokenStrmData> mpTokensData;   /// For later conversion of token array.
};

// ----------------------------------------------------------------------------

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
    const XclImpName*   FindName( const String& rXclName, SCTAB nScTab = SCTAB_MAX ) const;

    /** Returns the defined name specified by its Excel index.
        @param nXclNameIdx  The index of the internal defined name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*   GetName( sal_uInt16 nXclNameIdx ) const;

    void ConvertAllTokens();

private:
    typedef boost::ptr_vector< XclImpName > XclImpNameList;
    XclImpNameList      maNameList;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef SC_RTFPARSE_HXX
#define SC_RTFPARSE_HXX

#include "eeparser.hxx"

#ifdef SC_RTFPARSE_CXX
#include <boost/ptr_container/ptr_vector.hpp>

struct ScRTFCellDefault
{
    SfxItemSet          aItemSet;
    SCCOL               nCol;
    sal_uInt16              nTwips;         // rechter Rand der Zelle
    SCCOL               nColOverlap;    // MergeCell wenn >1, merged cells wenn 0

                        ScRTFCellDefault( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), nColOverlap(1) {}
};
typedef boost::ptr_vector< ScRTFCellDefault > ScRTFDefaultList;

// deswegen ULONG, typedef bringt's auch nicht :-(
SV_DECL_VARARR_SORT( ScRTFColTwips, sal_uLong, 16, 4)

#else       // SC_RTFPARSE_CXX

struct ScRTFCellDefault;
class ScRTFDefaultList;
class ScRTFColTwips;

#endif      // SC_RTFPARSE_CXX


class EditEngine;

class ScRTFParser : public ScEEParser
{
private:
    ScRTFDefaultList*   pDefaultList;
    ScRTFColTwips*      pColTwips;
    ScRTFCellDefault*   pInsDefault;
    ScRTFCellDefault*   pActDefault;
    ScRTFCellDefault*   pDefMerge;
    sal_uLong               nStartAdjust;
    sal_uInt16              nLastWidth;
    sal_Bool                bNewDef;

    DECL_LINK( RTFImportHdl, ImportInfo* );
    inline void         NextRow();
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                ColAdjust();
    sal_Bool                SeekTwips( sal_uInt16 nTwips, SCCOL* pCol );
    void                NewCellRow( ImportInfo* );

public:
                        ScRTFParser( EditEngine* );
    virtual             ~ScRTFParser();
    virtual sal_uLong       Read( SvStream&, const String& rBaseURL );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

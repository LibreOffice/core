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
#ifndef INCLUDED_SW_INC_TBLAFMT_HXX
#define INCLUDED_SW_INC_TBLAFMT_HXX
/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *     The structure of table auto formatting should not be changed. It is used
 *     by different code of Writer and Calc. If a change is necessary, the
 *     source code of both applications must be changed!
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include <memory>

#include "hintids.hxx"
#include <svx/algitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/rotmodit.hxx>
#include <svl/intitem.hxx>
#include <editeng/lineitem.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtornt.hxx>
#include <swtblfmt.hxx>
#include "swdllapi.h"

struct SwAfVersions;

class SvNumberFormatter;

/*
@remarks
A table has a number of lines. These lines seem to correspond with rows, except in the case of
rows spanning more than one line. Each line contains a number of boxes/cells.

AutoFormat properties are retrieved and stored in a grid of 16 table boxes. A sampling approach
is used to read the data. 4 lines are picked, and 4 boxes are picked from each.

The line picking and box picking algorithms are similar. We start at the first line/box, and pick
lines/boxes one by one for a maximum of 3. The 4th line/box is the last line/box in the current
table/line. If we hit the end of lines/boxes, the last line/box encountered is picked several times.

For example, in a 2x3 table, the 4 lines will be [0, 1, 1, 1]. In each line, the boxes will be
[0, 1, 2, 2]. In a 6x5 table, the 4 lines will be [0, 1, 2, 4] and the boxes per line will be
[0, 1, 2, 5].

As you can see, property extraction/application is lossless for tables that are 4x4 or smaller
(and in fact has a bit of redundnacy). For larger tables, we lose any individual cell formatting
for the range [(3,rows - 1) -> (3, cols - 1)]. That formatting is replaced by formatting from
the saved cells:

            0            1            2           3           4           5
        +-----------------------------------------------------------------------+
     0  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     1  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     2  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------------------------------------------------------------------+
     3  |           |           |           |           |           |           |
        +-----------------------------------------------------------------------+
     4  |           |           |           |           |           |           |
        +-----------------------------------------------------------------------+
     5  |   Saved   |  Saved    |  Saved    |           |           |  Saved    |
        +-----------+-----------+-----------+-----------+-----------+-----------+

The properties saved are divided into three categories:
    1. Character properties: Font, font size, weight, etc.
    2. Box properties: Box, cell background
    3. Table properties: Properties that are set in the Table->Table Properties dialog.

Character and box properties are stored per cell (and are lossy for tables larger than 4x4). Table
properties are stored per-table, and are lossless.
*/
class SW_DLLPUBLIC SwTableAutoFormat
{
    SwTableFormat* m_pTableStyle;
    sal_uInt16 nStrResId;

    // Common flags of Calc and Writer.
    bool bInclFont : 1;
    bool bInclJustify : 1;
    bool bInclFrame : 1;
    bool bInclBackground : 1;
    bool bInclValueFormat : 1;

    // Calc specific flags.
    bool bInclWidthHeight : 1;

public:
    SwTableAutoFormat( const OUString& rName, SwTableFormat* pTableStyle );
    SwTableAutoFormat( const SwTableAutoFormat& rNew );

    SwTableAutoFormat& operator=( const SwTableAutoFormat& rNew );

    SwTableFormat* GetTableStyle() { return m_pTableStyle; }

    void SetBoxFormat( const SwTableBoxFormat& rNew, sal_uInt8 nPos );
    SwTableBoxFormat* GetBoxFormat( sal_uInt8 nPos ) const;

    void SetName( const OUString& rNew ) { m_pTableStyle->SetName( rNew ); nStrResId = USHRT_MAX; }
    OUString GetName() const { return m_pTableStyle->GetName(); }

    enum UpdateFlags { UPDATE_CHAR = 1, UPDATE_BOX = 2, UPDATE_ALL = 3 };
    void UpdateFromSet( sal_uInt8 nPos, const SfxItemSet& rSet,
                                UpdateFlags eFlags, SvNumberFormatter* );
    void UpdateToSet( sal_uInt8 nPos, SfxItemSet& rSet, UpdateFlags eFlags,
                        SvNumberFormatter* ) const ;

    void RestoreTableProperties(SwTable &table) const;
    void StoreTableProperties(const SwTable &table);

    bool IsFont() const         { return bInclFont; }
    bool IsJustify() const      { return bInclJustify; }
    bool IsFrame() const        { return bInclFrame; }
    bool IsBackground() const   { return bInclBackground; }
    bool IsValueFormat() const  { return bInclValueFormat; }

    void SetFont( const bool bNew )         { bInclFont = bNew; }
    void SetJustify( const  bool bNew )     { bInclJustify = bNew; }
    void SetFrame( const bool bNew )        { bInclFrame = bNew; }
    void SetBackground( const bool bNew )   { bInclBackground = bNew; }
    void SetValueFormat( const bool bNew )  { bInclValueFormat = bNew; }
    void SetWidthHeight( const bool bNew )  { bInclWidthHeight = bNew; }

    static SwTableAutoFormat* Load( SvStream& rStream, const SwAfVersions&, SwDoc* pDoc );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;
};

class SW_DLLPUBLIC SwTableAutoFormatTable
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
    SwDoc* m_pDoc;

    SAL_DLLPRIVATE bool Load( SvStream& rStream );
    SAL_DLLPRIVATE bool Save( SvStream& rStream ) const;

public:
    explicit SwTableAutoFormatTable(SwDoc* pDoc);
    ~SwTableAutoFormatTable();

    size_t size() const;
    SwTableAutoFormat const& operator[](size_t i) const;
    SwTableAutoFormat      & operator[](size_t i);
    void InsertAutoFormat(size_t i, std::unique_ptr<SwTableAutoFormat> pFormat);
    void EraseAutoFormat(size_t i);
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(size_t i);

    bool Load();
    bool Save() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

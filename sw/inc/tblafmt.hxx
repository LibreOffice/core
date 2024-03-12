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

#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/autoformathelper.hxx>
#include <unotools/weakref.hxx>
#include <rtl/ref.hxx>
#include "fmtornt.hxx"
#include "swdllapi.h"

struct SwAfVersions;

class SvNumberFormatter;
class SwTable;
class SwXTextCellStyle;
class SwXTextTableStyle;

class SW_DLLPUBLIC SwBoxAutoFormat : public AutoFormatBase
{
private:
    // Writer specific
    std::unique_ptr<SvxFrameDirectionItem>  m_aTextOrientation;
    std::unique_ptr<SwFormatVertOrient>     m_aVerticalAlignment;

    // number format
    OUString                                m_sNumFormatString;
    LanguageType                            m_eSysLanguage;
    LanguageType                            m_eNumFormatLanguage;

    // associated UNO object, if such exists
    unotools::WeakReference<SwXTextCellStyle> m_xAutoFormatUnoObject;

public:
    SwBoxAutoFormat();
    SwBoxAutoFormat( const SwBoxAutoFormat& rNew );
    ~SwBoxAutoFormat();

    /// assignment-op (still used)
    SwBoxAutoFormat& operator=(const SwBoxAutoFormat& rRef);

    /// Comparing based of boxes backgrounds.
    bool operator==(const SwBoxAutoFormat& rRight) const;

    // The get-methods.
    const SvxFrameDirectionItem& GetTextOrientation() const { return *m_aTextOrientation; }
    const SwFormatVertOrient& GetVerticalAlignment() const { return *m_aVerticalAlignment; }

    void GetValueFormat( OUString& rFormat, LanguageType& rLng, LanguageType& rSys ) const
        { rFormat = m_sNumFormatString; rLng = m_eNumFormatLanguage; rSys = m_eSysLanguage; }

    const OUString& GetNumFormatString() const { return m_sNumFormatString; }
    const LanguageType& GetSysLanguage() const { return m_eSysLanguage; }
    const LanguageType& GetNumFormatLanguage() const { return m_eNumFormatLanguage; }

    // The set-methods.
    void SetTextOrientation( const SvxFrameDirectionItem& rNew ) { m_aTextOrientation.reset(rNew.Clone()); }
    void SetVerticalAlignment( const SwFormatVertOrient& rNew ) { m_aVerticalAlignment.reset(rNew.Clone()); }

    void SetValueFormat( const OUString& rFormat, LanguageType eLng, LanguageType eSys )
        { m_sNumFormatString = rFormat; m_eNumFormatLanguage = eLng; m_eSysLanguage = eSys; }

    void SetNumFormatString(const OUString& rNew) { m_sNumFormatString = rNew; }
    void SetSysLanguage(const LanguageType& rNew) { m_eSysLanguage = rNew; }
    void SetNumFormatLanguage(const LanguageType& rNew) { m_eNumFormatLanguage = rNew; }

    unotools::WeakReference<SwXTextCellStyle> const& GetXObject() const
        { return m_xAutoFormatUnoObject; }
    void SetXObject(rtl::Reference<SwXTextCellStyle> const& xObject);

    bool Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;
};

enum class SwTableAutoFormatUpdateFlags { Char = 1, Box = 2 };
namespace o3tl {
    template<> struct typed_flags<SwTableAutoFormatUpdateFlags> : is_typed_flags<SwTableAutoFormatUpdateFlags, 0x03> {};
};

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
(and in fact has a bit of redundancy). For larger tables, we lose any individual cell formatting
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
class SAL_DLLPUBLIC_RTTI SwTableAutoFormat
{
    friend class SwDocTest;
    friend void FinitCore();       // To destroy default pointer.
    static SwBoxAutoFormat* s_pDefaultBoxAutoFormat;

    unotools::WeakReference<SwXTextTableStyle> m_xUnoTextTableStyle;

    OUString m_aName;
    sal_uInt16 m_nStrResId;

    // Common flags of Calc and Writer.
    bool m_bInclFont : 1;
    bool m_bInclJustify : 1;
    bool m_bInclFrame : 1;
    bool m_bInclBackground : 1;
    bool m_bInclValueFormat : 1;

    // Calc specific flags.
    bool m_bInclWidthHeight : 1;

    SwBoxAutoFormat* m_aBoxAutoFormat[ 16 ] = {};

    // Writer-specific options
    std::shared_ptr<SvxFormatKeepItem> m_aKeepWithNextPara;
    sal_uInt16 m_aRepeatHeading;
    bool m_bLayoutSplit;
    bool m_bRowSplit;
    bool m_bCollapsingBorders;
    std::shared_ptr<SvxShadowItem> m_aShadow;

    bool m_bHidden;
    bool m_bUserDefined;
public:
    SW_DLLPUBLIC SwTableAutoFormat( OUString aName );
    SW_DLLPUBLIC SwTableAutoFormat( const SwTableAutoFormat& rNew );
    SW_DLLPUBLIC ~SwTableAutoFormat();

    SW_DLLPUBLIC SwTableAutoFormat& operator=( const SwTableAutoFormat& rNew );

    const SvxFormatKeepItem& GetKeepWithNextPara() const { return *m_aKeepWithNextPara; }
    const SvxShadowItem& GetShadow() const { return *m_aShadow; }

    void SetKeepWithNextPara(const SvxFormatKeepItem& rNew) { m_aKeepWithNextPara.reset(rNew.Clone()); }
    void SetShadow(const SvxShadowItem& rNew) { m_aShadow.reset(rNew.Clone()); }

    void SetBoxFormat( const SwBoxAutoFormat& rNew, sal_uInt8 nPos );
    const SwBoxAutoFormat& GetBoxFormat( sal_uInt8 nPos ) const;
    SW_DLLPUBLIC SwBoxAutoFormat& GetBoxFormat( sal_uInt8 nPos );
    static const SwBoxAutoFormat& GetDefaultBoxFormat();

    void SetName( const OUString& rNew ) { m_aName = rNew; m_nStrResId = USHRT_MAX; }
    const OUString& GetName() const { return m_aName; }

    void UpdateFromSet( sal_uInt8 nPos, const SfxItemSet& rSet,
                                SwTableAutoFormatUpdateFlags eFlags, SvNumberFormatter const * );
    void UpdateToSet( const sal_uInt8 nPos, const bool bSingleRowTable, const bool bSingleColTable,
                        SfxItemSet& rSet, SwTableAutoFormatUpdateFlags eFlags,
                        SvNumberFormatter* ) const ;

    void RestoreTableProperties(SwTable &table) const;
    void StoreTableProperties(const SwTable &table);

    bool IsFont() const         { return m_bInclFont; }
    bool IsJustify() const      { return m_bInclJustify; }
    bool IsFrame() const        { return m_bInclFrame; }
    bool IsBackground() const   { return m_bInclBackground; }
    bool IsValueFormat() const  { return m_bInclValueFormat; }

    /// Check if style is hidden.
    bool IsHidden() const       { return m_bHidden; }
    /// Check if style is defined by user.
    bool IsUserDefined() const  { return m_bUserDefined; }

    void SetFont( const bool bNew )         { m_bInclFont = bNew; }
    void SetJustify( const  bool bNew )     { m_bInclJustify = bNew; }
    void SetFrame( const bool bNew )        { m_bInclFrame = bNew; }
    void SetBackground( const bool bNew )   { m_bInclBackground = bNew; }
    void SetValueFormat( const bool bNew )  { m_bInclValueFormat = bNew; }
    void SetWidthHeight( const bool bNew )  { m_bInclWidthHeight = bNew; }

    /// Set if style is hidden.
    void SetHidden(bool bHidden)            { m_bHidden = bHidden; }
    /// Set if style is user defined.
    void SetUserDefined(bool bUserDefined)  { m_bUserDefined = bUserDefined; }

    /// These methods returns what style (row or column) is applied first on given Cell
    bool FirstRowEndColumnIsRow();
    bool FirstRowStartColumnIsRow();
    bool LastRowEndColumnIsRow();
    bool LastRowStartColumnIsRow();
    bool HasHeaderRow() const;

    bool Load( SvStream& rStream, const SwAfVersions& );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;

    unotools::WeakReference<SwXTextTableStyle> const& GetXObject() const
        { return m_xUnoTextTableStyle; }
    void SetXObject(rtl::Reference<SwXTextTableStyle> const& xObject);

    /// Returns the cell's name postfix. eg. ".1"
    OUString GetTableTemplateCellSubName(const SwBoxAutoFormat& rBoxFormat) const;
    /// Returns a vector of indexes in aBoxAutoFormat array. Returned indexes points to cells which are mapped to a table-template.
    static const std::vector<sal_Int32>& GetTableTemplateMap();

    /**
     * Calculates the relevant position in the table autoformat for a given
     * cell in a given table.
     */
    static sal_uInt8 CountPos(sal_uInt32 nCol, sal_uInt32 nCols, sal_uInt32 nRow, sal_uInt32 nRows);
};

class SW_DLLPUBLIC SwTableAutoFormatTable
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

    SAL_DLLPRIVATE bool Load( SvStream& rStream );
    SAL_DLLPRIVATE bool Save( SvStream& rStream ) const;

public:
    explicit SwTableAutoFormatTable();
    ~SwTableAutoFormatTable();

    size_t size() const;
    SwTableAutoFormat const& operator[](size_t i) const;
    SwTableAutoFormat      & operator[](size_t i);

    /// Append table style to the existing styles.
    void AddAutoFormat(const SwTableAutoFormat& rFormat);

    void InsertAutoFormat(size_t i, std::unique_ptr<SwTableAutoFormat> pFormat);
    void EraseAutoFormat(size_t i);
    void EraseAutoFormat(const OUString& rName);
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(size_t i);
    /// Removes an autoformat. Returns pointer to the removed autoformat or nullptr.
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(const OUString& rName);

    /// Find table style with the provided name, return nullptr when not found.
    SwTableAutoFormat* FindAutoFormat(std::u16string_view rName) const;

    void Load();
    bool Save() const;
};

class SwCellStyleDescriptor
{
    const std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>& m_rCellStyleDesc;
public:
    SwCellStyleDescriptor(const std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>& rCellStyleDesc) : m_rCellStyleDesc(rCellStyleDesc) { }

    const OUString&  GetName() const   { return m_rCellStyleDesc.first; }
};

class SwCellStyleTable
{
    std::vector<std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>> m_aCellStyles;
public:
    SwCellStyleTable();
    ~SwCellStyleTable();

    size_t size() const;
    SwCellStyleDescriptor operator[](size_t i) const;
    void clear();

    /// Add a copy of rBoxFormat
    void AddBoxFormat(const SwBoxAutoFormat& rBoxFormat, const OUString& sName);
    void RemoveBoxFormat(const OUString& sName);
    void ChangeBoxFormatName(std::u16string_view sFromName, const OUString& sToName);
    /// If found returns its name. If not found returns an empty OUString
    OUString GetBoxFormatName(const SwBoxAutoFormat& rBoxFormat) const;
    /// If found returns a ptr to a BoxFormat. If not found returns nullptr
    SwBoxAutoFormat* GetBoxFormat(std::u16string_view sName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

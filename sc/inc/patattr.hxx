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

#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#include <tools/degree.hxx>
#include <editeng/svxenum.hxx>
#include "scdllapi.h"
#include "fonthelper.hxx"
#include <memory>

namespace vcl { class Font; }
class OutputDevice;
class Fraction;
class ScStyleSheet;
class SvNumberFormatter;
class ScDocument;
enum class ScRotateDir : sal_uInt8;

///  how to treat COL_AUTO in GetFont:

enum ScAutoFontColorMode
{
    SC_AUTOCOL_RAW,         ///< COL_AUTO is returned
    SC_AUTOCOL_BLACK,       ///< always use black
    SC_AUTOCOL_PRINT,       ///< black or white, depending on background
    SC_AUTOCOL_DISPLAY,     ///< from style settings, or black/white if needed
    SC_AUTOCOL_IGNOREFONT,  ///< like DISPLAY, but ignore stored font color (assume COL_AUTO)
    SC_AUTOCOL_IGNOREBACK,  ///< like DISPLAY, but ignore stored background color (use configured color)
    SC_AUTOCOL_IGNOREALL    ///< like DISPLAY, but ignore stored font and background colors
};

class SC_DLLPUBLIC ScPatternAttr final : public SfxSetItem
{
    std::optional<OUString>  pName;
    mutable std::optional<size_t> mxHashCode;
    ScStyleSheet*              pStyle;
    sal_uInt64                 mnKey;
public:
                            ScPatternAttr(std::unique_ptr<SfxItemSet>&& pItemSet, const OUString& rStyleName);
                            ScPatternAttr(std::unique_ptr<SfxItemSet>&& pItemSet);
                            ScPatternAttr(SfxItemPool* pItemPool);
                            ScPatternAttr(const ScPatternAttr& rPatternAttr);

                            virtual ~ScPatternAttr() override;

    virtual ScPatternAttr*  Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool            operator==(const SfxPoolItem& rCmp) const override;

    const SfxPoolItem&      GetItem( sal_uInt16 nWhichP ) const
                                        { return GetItemSet().Get(nWhichP); }
    template<class T> const T& GetItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T&>(GetItem(sal_uInt16(nWhich))); }

    static const SfxPoolItem& GetItem( sal_uInt16 nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet );
    template<class T> static const T& GetItem( TypedWhichId<T> nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet )
    { return static_cast<const T&>(GetItem(sal_uInt16(nWhich), rItemSet, pCondSet)); }

    const SfxPoolItem&      GetItem( sal_uInt16 nWhich, const SfxItemSet* pCondSet ) const;
    template<class T> const T& GetItem( TypedWhichId<T> nWhich, const SfxItemSet* pCondSet  ) const
    { return static_cast<const T&>(GetItem(sal_uInt16(nWhich), pCondSet)); }

                            /// @param pWhich are no ranges, but single IDs, 0-terminated
    bool                    HasItemsSet( const sal_uInt16* pWhich ) const;
    void                    ClearItems( const sal_uInt16* pWhich );

    void                    DeleteUnchanged( const ScPatternAttr* pOldAttrs );

    static SvxCellOrientation GetCellOrientation( const SfxItemSet& rItemSet, const SfxItemSet* pCondSet );
    SvxCellOrientation      GetCellOrientation( const SfxItemSet* pCondSet = nullptr ) const;

    /** Static helper function to fill a font object from the passed item set. */
    static void             GetFont( vcl::Font& rFont, const SfxItemSet& rItemSet,
                                        ScAutoFontColorMode eAutoMode,
                                        const OutputDevice* pOutDev = nullptr,
                                        const Fraction* pScale = nullptr,
                                        const SfxItemSet* pCondSet = nullptr,
                                        SvtScriptType nScript = SvtScriptType::NONE, const Color* pBackConfigColor = nullptr,
                                        const Color* pTextConfigColor = nullptr );

    static ScDxfFont        GetDxfFont(const SfxItemSet& rSet, SvtScriptType nScript);
    /** Fills a font object from the own item set. */
    void                    GetFont( vcl::Font& rFont, ScAutoFontColorMode eAutoMode,
                                     const OutputDevice* pOutDev = nullptr,
                                     const Fraction* pScale = nullptr,
                                     const SfxItemSet* pCondSet = nullptr,
                                     SvtScriptType nScript = SvtScriptType::NONE,
                                     const Color* pBackConfigColor = nullptr,
                                     const Color* pTextConfigColor = nullptr ) const;

    /** Converts all Calc items contained in rSrcSet to edit engine items and puts them into rEditSet. */
    static void             FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet = nullptr );
    /** Converts all Calc items contained in the own item set to edit engine items and puts them into pEditSet. */
    void                    FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet = nullptr ) const;

    /** Converts all edit engine items contained in rEditSet to Calc items and puts them into rDestSet. */
    static void             GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet );
    /** Converts all edit engine items contained in pEditSet to Calc items and puts them into the own item set. */
    void                    GetFromEditItemSet( const SfxItemSet* pEditSet );

    void                    FillEditParaItems( SfxItemSet* pSet ) const;

    ScPatternAttr*          PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const;

    void                    SetStyleSheet(ScStyleSheet* pNewStyle, bool bClearDirectFormat = true);
    const ScStyleSheet*     GetStyleSheet() const  { return pStyle; }
    const OUString*         GetStyleName() const;
    void                    UpdateStyleSheet(const ScDocument& rDoc);
    void                    StyleToName();

    bool                    IsVisible() const;
    bool                    IsVisibleEqual( const ScPatternAttr& rOther ) const;

                            /** If font is an old symbol font StarBats/StarMath
                                with text encoding RTL_TEXTENC_SYMBOL */
    bool                    IsSymbolFont() const;

    sal_uInt32              GetNumberFormat( SvNumberFormatter* ) const;
    sal_uInt32              GetNumberFormat( SvNumberFormatter* pFormatter,
                                             const SfxItemSet* pCondSet ) const;

    Degree100               GetRotateVal( const SfxItemSet* pCondSet ) const;
    ScRotateDir             GetRotateDir( const SfxItemSet* pCondSet ) const;

    void                    SetKey(sal_uInt64 nKey);
    sal_uInt64              GetKey() const;

private:
    void                    CalcHashCode() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

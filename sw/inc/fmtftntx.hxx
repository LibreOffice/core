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
#ifndef INCLUDED_SW_INC_FMTFTNTX_HXX
#define INCLUDED_SW_INC_FMTFTNTX_HXX

#include <svl/eitem.hxx>
#include <editeng/numitem.hxx>
#include "hintids.hxx"
#include "format.hxx"
#include "swdllapi.h"

enum SwFootnoteEndPosEnum
{
    FTNEND_ATPGORDOCEND,                ///< at page or document end
    FTNEND_ATTXTEND,                    ///< at end of the current text end
    FTNEND_ATTXTEND_OWNNUMSEQ,          ///< -""- and with own number sequence
    FTNEND_ATTXTEND_OWNNUMANDFMT,       ///< -""- and with own numberformat
    FTNEND_ATTXTEND_END
};

/**
 * SfxPoolItem subclass that is a wrapper around an SwFootnoteEndPosEnum, i.e. to decide where
 * footnotes or endnotes should be collected. Available on the UI as Format -> Sections -> <Name> ->
 * Options -> Footnotes/Endnotes. This is the shared base class for both footnotes and endnotes.
 */
class SW_DLLPUBLIC SwFormatFootnoteEndAtTextEnd : public SfxEnumItem<SwFootnoteEndPosEnum>
{
    OUString m_sPrefix;
    OUString m_sSuffix;
    SvxNumberType m_aFormat;
    sal_uInt16      m_nOffset;

protected:
    SwFormatFootnoteEndAtTextEnd( sal_uInt16 nWhichL, SwFootnoteEndPosEnum ePos )
        : SfxEnumItem( nWhichL, ePos ), m_nOffset( 0 )
    {}

    virtual SfxItemType ItemType() const override = 0;

public:
    virtual bool             operator==( const SfxPoolItem& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    bool IsAtEnd() const { return FTNEND_ATPGORDOCEND != GetValue(); }

    SwFormatFootnoteEndAtTextEnd & operator=( const SwFormatFootnoteEndAtTextEnd & rAttr );
    SwFormatFootnoteEndAtTextEnd(SwFormatFootnoteEndAtTextEnd const &) = default;
        // SfxPoolItem copy function dichotomy

    SvxNumType GetNumType() const        { return m_aFormat.GetNumberingType(); }
    void SetNumType( SvxNumType eType )  { m_aFormat.SetNumberingType(eType); }

    const SvxNumberType& GetSwNumType() const   { return m_aFormat; }

    sal_uInt16 GetOffset() const                { return m_nOffset; }
    void SetOffset( sal_uInt16 nOff )           { m_nOffset = nOff; }

    const OUString& GetPrefix() const      { return m_sPrefix; }
    void SetPrefix(const OUString& rSet)   { m_sPrefix = rSet; }

    const OUString& GetSuffix() const      { return m_sSuffix; }
    void SetSuffix(const OUString& rSet)   { m_sSuffix = rSet; }
};

/// SwFormatFootnoteEndAtTextEnd subclass, specific to footnotes, placed in the item set of an
/// SwSectionFormat.
class SW_DLLPUBLIC SwFormatFootnoteAtTextEnd final : public SwFormatFootnoteEndAtTextEnd
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatFootnoteAtTextEnd)
    SwFormatFootnoteAtTextEnd( SwFootnoteEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFormatFootnoteEndAtTextEnd( RES_FTN_AT_TXTEND, ePos )
    {}

    virtual SwFormatFootnoteAtTextEnd* Clone( SfxItemPool *pPool = nullptr ) const override;
};

/// SwFormatFootnoteEndAtTextEnd subclass, specific to endnotes, placed in the item set of an
/// SwSectionFormat.
class SW_DLLPUBLIC SwFormatEndAtTextEnd final : public SwFormatFootnoteEndAtTextEnd
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatEndAtTextEnd)
    SwFormatEndAtTextEnd( SwFootnoteEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFormatFootnoteEndAtTextEnd( RES_END_AT_TXTEND, ePos )
    {
        SetNumType( SVX_NUM_ROMAN_LOWER );
    }

    virtual SwFormatEndAtTextEnd* Clone( SfxItemPool *pPool = nullptr ) const override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatFootnoteAtTextEnd &SwAttrSet::GetFootnoteAtTextEnd(bool bInP) const
    { return Get( RES_FTN_AT_TXTEND, bInP); }
inline const SwFormatEndAtTextEnd &SwAttrSet::GetEndAtTextEnd(bool bInP) const
    { return Get( RES_END_AT_TXTEND, bInP); }

inline const SwFormatFootnoteAtTextEnd &SwFormat::GetFootnoteAtTextEnd(bool bInP) const
    { return m_aSet.GetFootnoteAtTextEnd(bInP); }
inline const SwFormatEndAtTextEnd &SwFormat::GetEndAtTextEnd(bool bInP) const
    { return m_aSet.GetEndAtTextEnd(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

class SW_DLLPUBLIC SwFormatFootnoteEndAtTextEnd : public SfxEnumItem<SwFootnoteEndPosEnum>
{
    OUString sPrefix;
    OUString sSuffix;
    SvxNumberType aFormat;
    sal_uInt16      nOffset;

protected:
    SwFormatFootnoteEndAtTextEnd( sal_uInt16 nWhichL, SwFootnoteEndPosEnum ePos )
        : SfxEnumItem( nWhichL, ePos ), nOffset( 0 )
    {}

public:
    virtual sal_uInt16       GetValueCount() const override;

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
    /*TODO: SfxPoolItem copy function dichotomy*/SwFormatFootnoteEndAtTextEnd(SwFormatFootnoteEndAtTextEnd const &) = default;

    SvxNumType GetNumType() const        { return aFormat.GetNumberingType(); }
    void SetNumType( SvxNumType eType )  { aFormat.SetNumberingType(eType); }

    const SvxNumberType& GetSwNumType() const   { return aFormat; }

    sal_uInt16 GetOffset() const                { return nOffset; }
    void SetOffset( sal_uInt16 nOff )           { nOffset = nOff; }

    const OUString& GetPrefix() const      { return sPrefix; }
    void SetPrefix(const OUString& rSet)   { sPrefix = rSet; }

    const OUString& GetSuffix() const      { return sSuffix; }
    void SetSuffix(const OUString& rSet)   { sSuffix = rSet; }
};

class SW_DLLPUBLIC SwFormatFootnoteAtTextEnd : public SwFormatFootnoteEndAtTextEnd
{
public:
    SwFormatFootnoteAtTextEnd( SwFootnoteEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFormatFootnoteEndAtTextEnd( RES_FTN_AT_TXTEND, ePos )
    {}

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};

class SW_DLLPUBLIC SwFormatEndAtTextEnd : public SwFormatFootnoteEndAtTextEnd
{
public:
    SwFormatEndAtTextEnd( SwFootnoteEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFormatFootnoteEndAtTextEnd( RES_END_AT_TXTEND, ePos )
    {
        SetNumType( SVX_NUM_ROMAN_LOWER );
    }

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
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

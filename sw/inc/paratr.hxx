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
#ifndef INCLUDED_SW_INC_PARATR_HXX
#define INCLUDED_SW_INC_PARATR_HXX
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include "swdllapi.h"
#include "hintids.hxx"
#include "calbck.hxx"
#include "swatrset.hxx"
#include "format.hxx"
#include "charfmt.hxx"
#include <editeng/adjustitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>

class IntlWrapper;

#define DROP_WHOLEWORD ((sal_uInt16)0x0001)

/** If SwFormatDrop is a Client, it is the CharFormat that describes the font for the
   DropCaps. If it is not a Client, formatting uses the CharFormat of the paragraph.
   If the CharFormat is modified, this change is propagated to the paragraphs
   via the Modify of SwFormatDrop. */
class SW_DLLPUBLIC SwFormatDrop: public SfxPoolItem, public SwClient
{
    SwModify* m_pDefinedIn;       /**< Modify-Object, that contains DropCaps.
                                  Can only be TextFormatCollection/TextNode. */
    sal_uInt16 m_nDistance;       ///< Distance to beginning of text.
    sal_uInt8  m_nLines;          ///< Line count.
    sal_uInt8  m_nChars;          ///< Character count.
    bool   m_bWholeWord;      ///< First word with initials.
public:
    static SfxPoolItem* CreateDefault();

    SwFormatDrop();
    virtual ~SwFormatDrop() override;

    // @@@ public copy ctor, but no copy assignment?
    SwFormatDrop( const SwFormatDrop & );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFormatDrop & operator= (const SwFormatDrop &) = delete;

protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_uInt8 GetLines() const { return m_nLines; }
    sal_uInt8 &GetLines() { return m_nLines; }

    sal_uInt8 GetChars() const { return m_nChars; }
    sal_uInt8 &GetChars() { return m_nChars; }

    bool GetWholeWord() const { return m_bWholeWord; }
    bool &GetWholeWord() { return m_bWholeWord; }

    sal_uInt16 GetDistance() const { return m_nDistance; }
    sal_uInt16 &GetDistance() { return m_nDistance; }

    const SwCharFormat *GetCharFormat() const { return static_cast<const SwCharFormat*>(GetRegisteredIn()); }
    SwCharFormat *GetCharFormat()       { return static_cast<SwCharFormat*>(GetRegisteredIn()); }
    void SetCharFormat( SwCharFormat *pNew );
    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    /// Get and set Modify pointer.
    const SwModify* GetDefinedIn() const { return m_pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew )
    { m_pDefinedIn = const_cast<SwModify*>(pNew); }
};

class SwRegisterItem : public SfxBoolItem
{
public:
    static SfxPoolItem* CreateDefault();

    inline SwRegisterItem( const bool bRegister = false );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
};

inline SwRegisterItem::SwRegisterItem( const bool bRegister ) :
    SfxBoolItem( RES_PARATR_REGISTER, bRegister )
{}

class SW_DLLPUBLIC SwNumRuleItem : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SwNumRuleItem()
        : SfxStringItem( RES_PARATR_NUMRULE, OUString() ) {}

    SwNumRuleItem( const OUString& rRuleName )
        : SfxStringItem( RES_PARATR_NUMRULE, rRuleName ) {}

    SwNumRuleItem& operator=( const SwNumRuleItem& rCpy )
    { SetValue( rCpy.GetValue() ); return *this; }
    SwNumRuleItem(SwNumRuleItem const &) = default; // SfxPoolItem copy function dichotomy

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwParaConnectBorderItem : public SfxBoolItem
{
public:

    inline SwParaConnectBorderItem( const bool bConnect = true );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
};

inline SwParaConnectBorderItem::SwParaConnectBorderItem( const bool bConnect ) :
    SfxBoolItem( RES_PARATR_CONNECT_BORDER, bConnect )
{}

//  Implementation of paragraph-attributes methods of SwAttrSet
inline const SvxLineSpacingItem &SwAttrSet::GetLineSpacing(bool bInP) const
    {   return Get( RES_PARATR_LINESPACING,bInP); }
inline const SvxAdjustItem &SwAttrSet::GetAdjust(bool bInP) const
    {   return Get( RES_PARATR_ADJUST,bInP); }
inline const SvxFormatSplitItem &SwAttrSet::GetSplit(bool bInP) const
    {   return Get( RES_PARATR_SPLIT,bInP); }
inline const SwRegisterItem &SwAttrSet::GetRegister(bool bInP) const
    {   return Get( RES_PARATR_REGISTER,bInP); }
inline const SvxWidowsItem &SwAttrSet::GetWidows(bool bInP) const
    {   return Get( RES_PARATR_WIDOWS,bInP); }
inline const SvxOrphansItem &SwAttrSet::GetOrphans(bool bInP) const
    {   return Get( RES_PARATR_ORPHANS,bInP); }
inline const SvxTabStopItem &SwAttrSet::GetTabStops(bool bInP) const
    {   return Get( RES_PARATR_TABSTOP,bInP); }
inline const SvxHyphenZoneItem &SwAttrSet::GetHyphenZone(bool bInP) const
    {   return Get(RES_PARATR_HYPHENZONE,bInP); }
inline const SwFormatDrop &SwAttrSet::GetDrop(bool bInP) const
    {   return Get(RES_PARATR_DROP,bInP); }
inline const SwNumRuleItem &SwAttrSet::GetNumRule(bool bInP) const
    {   return Get(RES_PARATR_NUMRULE,bInP); }
inline const SvxScriptSpaceItem& SwAttrSet::GetScriptSpace(bool bInP) const
    {   return Get(RES_PARATR_SCRIPTSPACE,bInP); }
inline const SvxHangingPunctuationItem &SwAttrSet::GetHangingPunctuation(bool bInP) const
    {   return Get(RES_PARATR_HANGINGPUNCTUATION,bInP); }
inline const SvxForbiddenRuleItem &SwAttrSet::GetForbiddenRule(bool bInP) const
    {   return Get(RES_PARATR_FORBIDDEN_RULES, bInP); }
inline const SvxParaVertAlignItem &SwAttrSet::GetParaVertAlign(bool bInP) const
    {   return Get( RES_PARATR_VERTALIGN, bInP ); }
inline const SvxParaGridItem &SwAttrSet::GetParaGrid(bool bInP) const
    {   return Get( RES_PARATR_SNAPTOGRID, bInP ); }
inline const SwParaConnectBorderItem &SwAttrSet::GetParaConnectBorder(bool bInP) const
    {   return Get( RES_PARATR_CONNECT_BORDER, bInP ); }

// Implementation of paragraph-attributes methods of SwFormat
inline const SvxLineSpacingItem &SwFormat::GetLineSpacing(bool bInP) const
    {   return m_aSet.GetLineSpacing(bInP); }
inline const SvxTabStopItem &SwFormat::GetTabStops(bool bInP) const
    {   return m_aSet.GetTabStops(bInP); }
inline const SwFormatDrop &SwFormat::GetDrop(bool bInP) const
    {   return m_aSet.GetDrop(bInP); }
inline const SwNumRuleItem &SwFormat::GetNumRule(bool bInP) const
    {   return m_aSet.GetNumRule(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

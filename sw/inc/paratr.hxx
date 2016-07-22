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
#include <hintids.hxx>
#include <calbck.hxx>
#include <swatrset.hxx>
#include <format.hxx>
#include <charfmt.hxx>
#include <swtypes.hxx>
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

class SwCharFormat;
class IntlWrapper;

#define DROP_WHOLEWORD ((sal_uInt16)0x0001)

/** If SwFormatDrop is a Client, it is the CharFormat that describes the font for the
   DropCaps. If it is not a Client, formatting uses the CharFormat of the paragraph.
   If the CharFormat is modified, this change is propagated to the paragraphs
   via the Modify of SwFormatDrop. */
class SW_DLLPUBLIC SwFormatDrop: public SfxPoolItem, public SwClient
{
    SwModify* pDefinedIn;       /**< Modify-Object, that contains DropCaps.
                                  Can only be TextFormatCollection/TextNode. */
    sal_uInt16 nDistance;       ///< Distance to beginning of text.
    sal_uInt16 nReadFormat;        ///< For Sw3-Reader: CharFormat-Id (load Pool!).
    sal_uInt8  nLines;          ///< Line count.
    sal_uInt8  nChars;          ///< Character count.
    bool   bWholeWord;      ///< First word with initials.
public:
    static SfxPoolItem* CreateDefault();

    SwFormatDrop();
    virtual ~SwFormatDrop();

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
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    inline sal_uInt8 GetLines() const { return nLines; }
    inline sal_uInt8 &GetLines() { return nLines; }

    inline sal_uInt8 GetChars() const { return nChars; }
    inline sal_uInt8 &GetChars() { return nChars; }

    inline bool GetWholeWord() const { return bWholeWord; }
    inline bool &GetWholeWord() { return bWholeWord; }

    inline sal_uInt16 GetDistance() const { return nDistance; }
    inline sal_uInt16 &GetDistance() { return nDistance; }

    inline const SwCharFormat *GetCharFormat() const { return static_cast<const SwCharFormat*>(GetRegisteredIn()); }
    inline SwCharFormat *GetCharFormat()       { return static_cast<SwCharFormat*>(GetRegisteredIn()); }
    void SetCharFormat( SwCharFormat *pNew );
    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    /// Get and set Modify pointer.
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
    { pDefinedIn = const_cast<SwModify*>(pNew); }
};

class SwRegisterItem : public SfxBoolItem
{
public:
    static SfxPoolItem* CreateDefault();

    inline SwRegisterItem( const bool bRegister = false );

    /// @@@ public copy assignment, but no copy ctor?
    inline SwRegisterItem& operator=( const SwRegisterItem& rRegister );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr ) const override;
};

inline SwRegisterItem::SwRegisterItem( const bool bRegister ) :
    SfxBoolItem( RES_PARATR_REGISTER, bRegister )
{}

inline SwRegisterItem& SwRegisterItem::operator=(
    const SwRegisterItem& rRegister )
{
    SetValue( rRegister.GetValue() );
    return *this;
}

class SW_DLLPUBLIC SwNumRuleItem : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SwNumRuleItem()
        : SfxStringItem( RES_PARATR_NUMRULE, OUString() ) {}

    SwNumRuleItem( const OUString& rRuleName )
        : SfxStringItem( RES_PARATR_NUMRULE, rRuleName ) {}

    SwNumRuleItem( const SwNumRuleItem& rCpy )
        : SfxStringItem( RES_PARATR_NUMRULE, rCpy.GetValue() ) {}

    SwNumRuleItem& operator=( const SwNumRuleItem& rCpy )
    { SetValue( rCpy.GetValue() ); return *this; }

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

class SwParaConnectBorderItem : public SfxBoolItem
{
public:

    inline SwParaConnectBorderItem( const bool bConnect = true );

    /// @@@ public copy assignment, but no copy ctor?
    inline SwParaConnectBorderItem& operator=( const SwParaConnectBorderItem& rConnect );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr ) const override;
};

inline SwParaConnectBorderItem::SwParaConnectBorderItem( const bool bConnect ) :
    SfxBoolItem( RES_PARATR_CONNECT_BORDER, bConnect )
{}

inline SwParaConnectBorderItem& SwParaConnectBorderItem::operator=(
    const SwParaConnectBorderItem& rConnect )
{
    SetValue( rConnect.GetValue() );
    return *this;
}

//  Implementation of paragraph-attributes methods of SwAttrSet
inline const SvxLineSpacingItem &SwAttrSet::GetLineSpacing(bool bInP) const
    {   return static_cast<const SvxLineSpacingItem&>(Get( RES_PARATR_LINESPACING,bInP)); }
inline const SvxAdjustItem &SwAttrSet::GetAdjust(bool bInP) const
    {   return static_cast<const SvxAdjustItem&>(Get( RES_PARATR_ADJUST,bInP)); }
inline const SvxFormatSplitItem &SwAttrSet::GetSplit(bool bInP) const
    {   return static_cast<const SvxFormatSplitItem&>(Get( RES_PARATR_SPLIT,bInP)); }
inline const SwRegisterItem &SwAttrSet::GetRegister(bool bInP) const
    {   return static_cast<const SwRegisterItem&>(Get( RES_PARATR_REGISTER,bInP)); }
inline const SvxWidowsItem &SwAttrSet::GetWidows(bool bInP) const
    {   return static_cast<const SvxWidowsItem&>(Get( RES_PARATR_WIDOWS,bInP)); }
inline const SvxOrphansItem &SwAttrSet::GetOrphans(bool bInP) const
    {   return static_cast<const SvxOrphansItem&>(Get( RES_PARATR_ORPHANS,bInP)); }
inline const SvxTabStopItem &SwAttrSet::GetTabStops(bool bInP) const
    {   return static_cast<const SvxTabStopItem&>(Get( RES_PARATR_TABSTOP,bInP)); }
inline const SvxHyphenZoneItem &SwAttrSet::GetHyphenZone(bool bInP) const
    {   return static_cast<const SvxHyphenZoneItem&>(Get(RES_PARATR_HYPHENZONE,bInP)); }
inline const SwFormatDrop &SwAttrSet::GetDrop(bool bInP) const
    {   return static_cast<const SwFormatDrop&>(Get(RES_PARATR_DROP,bInP)); }
inline const SwNumRuleItem &SwAttrSet::GetNumRule(bool bInP) const
    {   return static_cast<const SwNumRuleItem&>(Get(RES_PARATR_NUMRULE,bInP)); }
inline const SvxScriptSpaceItem& SwAttrSet::GetScriptSpace(bool bInP) const
    {   return static_cast<const SvxScriptSpaceItem&>(Get(RES_PARATR_SCRIPTSPACE,bInP)); }
inline const SvxHangingPunctuationItem &SwAttrSet::GetHangingPunctuation(bool bInP) const
    {   return static_cast<const SvxHangingPunctuationItem&>(Get(RES_PARATR_HANGINGPUNCTUATION,bInP)); }
inline const SvxForbiddenRuleItem &SwAttrSet::GetForbiddenRule(bool bInP) const
    {   return static_cast<const SvxForbiddenRuleItem&>(Get(RES_PARATR_FORBIDDEN_RULES, bInP)); }
inline const SvxParaVertAlignItem &SwAttrSet::GetParaVertAlign(bool bInP) const
    {   return static_cast<const SvxParaVertAlignItem&>(Get( RES_PARATR_VERTALIGN, bInP )); }
inline const SvxParaGridItem &SwAttrSet::GetParaGrid(bool bInP) const
    {   return static_cast<const SvxParaGridItem&>(Get( RES_PARATR_SNAPTOGRID, bInP )); }
inline const SwParaConnectBorderItem &SwAttrSet::GetParaConnectBorder(bool bInP) const
    {   return static_cast<const SwParaConnectBorderItem&>(Get( RES_PARATR_CONNECT_BORDER, bInP )); }

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

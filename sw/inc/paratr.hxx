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
#ifndef _PARATR_HXX
#define _PARATR_HXX
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <calbck.hxx>
#include <swatrset.hxx>
#include <format.hxx>
#include <swtypes.hxx>
#include <editeng/adjustitem.hxx>
#include <tools/string.hxx>

class SwCharFmt;
class IntlWrapper;

#define DROP_WHOLEWORD ((sal_uInt16)0x0001)

/** If SwFmtDrop is a Client, it is the CharFmt that describes the font for the
   DropCaps. If it is not a Client, formating uses the CharFmt of the paragraph.
   If the CharFmt is modified, this change is propagated to the paragraphs
   via the Modify of SwFmtDrop. */
class SW_DLLPUBLIC SwFmtDrop: public SfxPoolItem, public SwClient
{
    SwModify* pDefinedIn;       /**< Modify-Object, that contains DropCaps.
                                  Can only be TxtFmtCollection/TxtNode. */
    sal_uInt16 nDistance;       ///< Distance to beginning of text.
    sal_uInt16 nReadFmt;        ///< For Sw3-Reader: CharFormat-Id (load Pool!).
    sal_uInt8  nLines;          ///< Line count.
    sal_uInt8  nChars;          ///< Character count.
    sal_Bool   bWholeWord;      ///< First word with initials.
public:
    TYPEINFO(); ///< Already in base class SwClient.

    SwFmtDrop();
    virtual ~SwFmtDrop();

    // @@@ public copy ctor, but no copy assignment?
    SwFmtDrop( const SwFmtDrop & );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFmtDrop & operator= (const SwFmtDrop &);

protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    inline sal_uInt8 GetLines() const { return nLines; }
    inline sal_uInt8 &GetLines() { return nLines; }

    inline sal_uInt8 GetChars() const { return nChars; }
    inline sal_uInt8 &GetChars() { return nChars; }

    inline sal_Bool GetWholeWord() const { return bWholeWord; }
    inline sal_uInt8 &GetWholeWord() { return bWholeWord; }

    inline sal_uInt16 GetDistance() const { return nDistance; }
    inline sal_uInt16 &GetDistance() { return nDistance; }

   inline const SwCharFmt *GetCharFmt() const { return (SwCharFmt*)GetRegisteredIn(); }
    inline SwCharFmt *GetCharFmt()       { return (SwCharFmt*)GetRegisteredIn(); }
    void SetCharFmt( SwCharFmt *pNew );
    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const;

    /// Get and set Modify pointer.
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
    { pDefinedIn = (SwModify*)pNew; }
};

class SwRegisterItem : public SfxBoolItem
{
public:
    TYPEINFO();

    inline SwRegisterItem( const sal_Bool bRegister = sal_False );

    /// @@@ public copy assignment, but no copy ctor?
    inline SwRegisterItem& operator=( const SwRegisterItem& rRegister );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline SwRegisterItem::SwRegisterItem( const sal_Bool bRegister ) :
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
    TYPEINFO();

    SwNumRuleItem()
        : SfxStringItem( RES_PARATR_NUMRULE, aEmptyStr ) {}

    SwNumRuleItem( const String& rRuleName )
        : SfxStringItem( RES_PARATR_NUMRULE, rRuleName ) {}

    SwNumRuleItem( const SwNumRuleItem& rCpy )
        : SfxStringItem( RES_PARATR_NUMRULE, rCpy.GetValue() ) {}

    SwNumRuleItem& operator=( const SwNumRuleItem& rCpy )
    { SetValue( rCpy.GetValue() ); return *this; }

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );
};

class SwParaConnectBorderItem : public SfxBoolItem
{
public:
    TYPEINFO();

    inline SwParaConnectBorderItem( const sal_Bool bConnect = sal_True );

    /// @@@ public copy assignment, but no copy ctor?
    inline SwParaConnectBorderItem& operator=( const SwParaConnectBorderItem& rConnect );

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline SwParaConnectBorderItem::SwParaConnectBorderItem( const sal_Bool bConnect ) :
    SfxBoolItem( RES_PARATR_CONNECT_BORDER, bConnect )
{}

inline SwParaConnectBorderItem& SwParaConnectBorderItem::operator=(
    const SwParaConnectBorderItem& rConnect )
{
    SetValue( rConnect.GetValue() );
    return *this;
}



/******************************************************************************
 *  Implementation of paragraph-attributes methods of SwAttrSet
 ******************************************************************************/

inline const SvxLineSpacingItem &SwAttrSet::GetLineSpacing(sal_Bool bInP) const
    {   return (const SvxLineSpacingItem&)Get( RES_PARATR_LINESPACING,bInP); }
inline const SvxAdjustItem &SwAttrSet::GetAdjust(sal_Bool bInP) const
    {   return (const SvxAdjustItem&)Get( RES_PARATR_ADJUST,bInP); }
inline const SvxFmtSplitItem &SwAttrSet::GetSplit(sal_Bool bInP) const
    {   return (const SvxFmtSplitItem&)Get( RES_PARATR_SPLIT,bInP); }
inline const SwRegisterItem &SwAttrSet::GetRegister(sal_Bool bInP) const
    {   return (const SwRegisterItem&)Get( RES_PARATR_REGISTER,bInP); }
inline const SvxWidowsItem &SwAttrSet::GetWidows(sal_Bool bInP) const
    {   return (const SvxWidowsItem&)Get( RES_PARATR_WIDOWS,bInP); }
inline const SvxOrphansItem &SwAttrSet::GetOrphans(sal_Bool bInP) const
    {   return (const SvxOrphansItem&)Get( RES_PARATR_ORPHANS,bInP); }
inline const SvxTabStopItem &SwAttrSet::GetTabStops(sal_Bool bInP) const
    {   return (const SvxTabStopItem&)Get( RES_PARATR_TABSTOP,bInP); }
inline const SvxHyphenZoneItem &SwAttrSet::GetHyphenZone(sal_Bool bInP) const
    {   return (const SvxHyphenZoneItem&)Get(RES_PARATR_HYPHENZONE,bInP); }
inline const SwFmtDrop &SwAttrSet::GetDrop(sal_Bool bInP) const
    {   return (const SwFmtDrop&)Get(RES_PARATR_DROP,bInP); }
inline const SwNumRuleItem &SwAttrSet::GetNumRule(sal_Bool bInP) const
    {   return (const SwNumRuleItem&)Get(RES_PARATR_NUMRULE,bInP); }
inline const SvxScriptSpaceItem& SwAttrSet::GetScriptSpace(sal_Bool bInP) const
    {   return (const SvxScriptSpaceItem&)Get(RES_PARATR_SCRIPTSPACE,bInP); }
inline const SvxHangingPunctuationItem &SwAttrSet::GetHangingPunctuation(sal_Bool bInP) const
    {   return (const SvxHangingPunctuationItem&)Get(RES_PARATR_HANGINGPUNCTUATION,bInP); }
inline const SvxForbiddenRuleItem &SwAttrSet::GetForbiddenRule(sal_Bool bInP) const
    {   return (const SvxForbiddenRuleItem&)Get(RES_PARATR_FORBIDDEN_RULES, bInP); }
inline const SvxParaVertAlignItem &SwAttrSet::GetParaVertAlign(sal_Bool bInP) const
    {   return (const SvxParaVertAlignItem&)Get( RES_PARATR_VERTALIGN, bInP ); }
inline const SvxParaGridItem &SwAttrSet::GetParaGrid(sal_Bool bInP) const
    {   return (const SvxParaGridItem&)Get( RES_PARATR_SNAPTOGRID, bInP ); }
inline const SwParaConnectBorderItem &SwAttrSet::GetParaConnectBorder(sal_Bool bInP) const
    {   return (const SwParaConnectBorderItem&)Get( RES_PARATR_CONNECT_BORDER, bInP ); }

/******************************************************************************
 *  Implementation of paragraph-attributes methods of SwFmt
 ******************************************************************************/

inline const SvxLineSpacingItem &SwFmt::GetLineSpacing(sal_Bool bInP) const
    {   return aSet.GetLineSpacing(bInP); }
inline const SvxAdjustItem &SwFmt::GetAdjust(sal_Bool bInP) const
    {   return aSet.GetAdjust(bInP); }
inline const SvxFmtSplitItem &SwFmt::GetSplit(sal_Bool bInP) const
    {   return aSet.GetSplit(bInP); }
inline const SwRegisterItem &SwFmt::GetRegister(sal_Bool bInP) const
    {   return aSet.GetRegister(bInP); }
inline const SvxWidowsItem &SwFmt::GetWidows(sal_Bool bInP) const
    {   return aSet.GetWidows(bInP); }
inline const SvxOrphansItem &SwFmt::GetOrphans(sal_Bool bInP) const
    {   return aSet.GetOrphans(bInP); }
inline const SvxTabStopItem &SwFmt::GetTabStops(sal_Bool bInP) const
    {   return aSet.GetTabStops(bInP); }
inline const SvxHyphenZoneItem &SwFmt::GetHyphenZone(sal_Bool bInP) const
    {   return aSet.GetHyphenZone(bInP); }
inline const SwFmtDrop &SwFmt::GetDrop(sal_Bool bInP) const
    {   return aSet.GetDrop(bInP); }
inline const SwNumRuleItem &SwFmt::GetNumRule(sal_Bool bInP) const
    {   return aSet.GetNumRule(bInP); }
inline const SvxScriptSpaceItem& SwFmt::GetScriptSpace(sal_Bool bInP) const
    {   return aSet.GetScriptSpace(bInP) ; }
inline const SvxHangingPunctuationItem &SwFmt::GetHangingPunctuation(sal_Bool bInP) const
    {   return aSet.GetHangingPunctuation(bInP) ; }
inline const SvxForbiddenRuleItem &SwFmt::GetForbiddenRule(sal_Bool bInP) const
    {   return (const SvxForbiddenRuleItem&)aSet.Get(RES_PARATR_FORBIDDEN_RULES, bInP); }
inline const SvxParaVertAlignItem &SwFmt::GetParaVertAlign(sal_Bool bInP) const
    {   return (const SvxParaVertAlignItem&)aSet.Get( RES_PARATR_VERTALIGN, bInP ); }
inline const SvxParaGridItem &SwFmt::GetParaGrid(sal_Bool bInP) const
    {   return (const SvxParaGridItem&)aSet.Get( RES_PARATR_SNAPTOGRID, bInP ); }
inline const SwParaConnectBorderItem &SwFmt::GetParaConnectBorder(sal_Bool bInP) const
    {   return (const SwParaConnectBorderItem&)aSet.Get( RES_PARATR_CONNECT_BORDER, bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

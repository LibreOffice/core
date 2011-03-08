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

class SwCharFmt;
class IntlWrapper;

#define DROP_WHOLEWORD ((USHORT)0x0001)

// If SwFmtDrop is a Client, it is the CharFmt that describes the font for the
// DropCaps. If it is not a Client, formating uses the CharFmt of the paragraph.
// If the CharFmt is modified, this change is propagated to the paragraphs
// via the Modify of SwFmtDrop.
class SW_DLLPUBLIC SwFmtDrop: public SfxPoolItem, public SwClient
{
    SwModify* pDefinedIn;   // Modify-Object, that contains DropCaps.
                            // Can only be TxtFmtCollection/TxtNode.
    USHORT nDistance;       // Distance to beginning of text.
    USHORT nReadFmt;        // For Sw3-Reader: CharFormat-Id (load Pool!).
    BYTE   nLines;          // Line count.
    BYTE   nChars;          // Character count.
    BOOL   bWholeWord;      // First word with initials.
public:
    TYPEINFO(); // Already in base class SwClient.

    SwFmtDrop();
    virtual ~SwFmtDrop();

    // @@@ public copy ctor, but no copy assignment?
    SwFmtDrop( const SwFmtDrop & );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFmtDrop & operator= (const SwFmtDrop &);
public:

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    inline BYTE GetLines() const { return nLines; }
    inline BYTE &GetLines() { return nLines; }

    inline BYTE GetChars() const { return nChars; }
    inline BYTE &GetChars() { return nChars; }

    inline BOOL GetWholeWord() const { return bWholeWord; }
    inline BYTE &GetWholeWord() { return bWholeWord; }

    inline USHORT GetDistance() const { return nDistance; }
    inline USHORT &GetDistance() { return nDistance; }

    inline const SwCharFmt *GetCharFmt() const { return (SwCharFmt*)pRegisteredIn; }
    inline SwCharFmt *GetCharFmt()       { return (SwCharFmt*)pRegisteredIn; }
    void SetCharFmt( SwCharFmt *pNew );
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    // Get information from Client.
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    // Get and set Modify pointer.
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
    { pDefinedIn = (SwModify*)pNew; }
};

class SwRegisterItem : public SfxBoolItem
{
public:
    TYPEINFO();

    inline SwRegisterItem( const BOOL bRegister = FALSE );

    // @@@ public copy assignment, but no copy ctor?
    inline SwRegisterItem& operator=( const SwRegisterItem& rRegister );

    // "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline SwRegisterItem::SwRegisterItem( const BOOL bRegister ) :
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

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId );
};

class SwParaConnectBorderItem : public SfxBoolItem
{
public:
    TYPEINFO();

    inline SwParaConnectBorderItem( const BOOL bConnect = TRUE );

    // @@@ public copy assignment, but no copy ctor?
    inline SwParaConnectBorderItem& operator=( const SwParaConnectBorderItem& rConnect );

    // "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline SwParaConnectBorderItem::SwParaConnectBorderItem( const BOOL bConnect ) :
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

inline const SvxLineSpacingItem &SwAttrSet::GetLineSpacing(BOOL bInP) const
    {   return (const SvxLineSpacingItem&)Get( RES_PARATR_LINESPACING,bInP); }
inline const SvxAdjustItem &SwAttrSet::GetAdjust(BOOL bInP) const
    {   return (const SvxAdjustItem&)Get( RES_PARATR_ADJUST,bInP); }
inline const SvxFmtSplitItem &SwAttrSet::GetSplit(BOOL bInP) const
    {   return (const SvxFmtSplitItem&)Get( RES_PARATR_SPLIT,bInP); }
inline const SwRegisterItem &SwAttrSet::GetRegister(BOOL bInP) const
    {   return (const SwRegisterItem&)Get( RES_PARATR_REGISTER,bInP); }
inline const SvxWidowsItem &SwAttrSet::GetWidows(BOOL bInP) const
    {   return (const SvxWidowsItem&)Get( RES_PARATR_WIDOWS,bInP); }
inline const SvxOrphansItem &SwAttrSet::GetOrphans(BOOL bInP) const
    {   return (const SvxOrphansItem&)Get( RES_PARATR_ORPHANS,bInP); }
inline const SvxTabStopItem &SwAttrSet::GetTabStops(BOOL bInP) const
    {   return (const SvxTabStopItem&)Get( RES_PARATR_TABSTOP,bInP); }
inline const SvxHyphenZoneItem &SwAttrSet::GetHyphenZone(BOOL bInP) const
    {   return (const SvxHyphenZoneItem&)Get(RES_PARATR_HYPHENZONE,bInP); }
inline const SwFmtDrop &SwAttrSet::GetDrop(BOOL bInP) const
    {   return (const SwFmtDrop&)Get(RES_PARATR_DROP,bInP); }
inline const SwNumRuleItem &SwAttrSet::GetNumRule(BOOL bInP) const
    {   return (const SwNumRuleItem&)Get(RES_PARATR_NUMRULE,bInP); }
inline const SvxScriptSpaceItem& SwAttrSet::GetScriptSpace(BOOL bInP) const
    {   return (const SvxScriptSpaceItem&)Get(RES_PARATR_SCRIPTSPACE,bInP); }
inline const SvxHangingPunctuationItem &SwAttrSet::GetHangingPunctuation(BOOL bInP) const
    {   return (const SvxHangingPunctuationItem&)Get(RES_PARATR_HANGINGPUNCTUATION,bInP); }
inline const SvxForbiddenRuleItem &SwAttrSet::GetForbiddenRule(BOOL bInP) const
    {   return (const SvxForbiddenRuleItem&)Get(RES_PARATR_FORBIDDEN_RULES, bInP); }
inline const SvxParaVertAlignItem &SwAttrSet::GetParaVertAlign(BOOL bInP) const
    {   return (const SvxParaVertAlignItem&)Get( RES_PARATR_VERTALIGN, bInP ); }
inline const SvxParaGridItem &SwAttrSet::GetParaGrid(BOOL bInP) const
    {   return (const SvxParaGridItem&)Get( RES_PARATR_SNAPTOGRID, bInP ); }
inline const SwParaConnectBorderItem &SwAttrSet::GetParaConnectBorder(BOOL bInP) const
    {   return (const SwParaConnectBorderItem&)Get( RES_PARATR_CONNECT_BORDER, bInP ); }

/******************************************************************************
 *  Implementation of paragraph-attributes methods of SwFmt
 ******************************************************************************/

inline const SvxLineSpacingItem &SwFmt::GetLineSpacing(BOOL bInP) const
    {   return aSet.GetLineSpacing(bInP); }
inline const SvxAdjustItem &SwFmt::GetAdjust(BOOL bInP) const
    {   return aSet.GetAdjust(bInP); }
inline const SvxFmtSplitItem &SwFmt::GetSplit(BOOL bInP) const
    {   return aSet.GetSplit(bInP); }
inline const SwRegisterItem &SwFmt::GetRegister(BOOL bInP) const
    {   return aSet.GetRegister(bInP); }
inline const SvxWidowsItem &SwFmt::GetWidows(BOOL bInP) const
    {   return aSet.GetWidows(bInP); }
inline const SvxOrphansItem &SwFmt::GetOrphans(BOOL bInP) const
    {   return aSet.GetOrphans(bInP); }
inline const SvxTabStopItem &SwFmt::GetTabStops(BOOL bInP) const
    {   return aSet.GetTabStops(bInP); }
inline const SvxHyphenZoneItem &SwFmt::GetHyphenZone(BOOL bInP) const
    {   return aSet.GetHyphenZone(bInP); }
inline const SwFmtDrop &SwFmt::GetDrop(BOOL bInP) const
    {   return aSet.GetDrop(bInP); }
inline const SwNumRuleItem &SwFmt::GetNumRule(BOOL bInP) const
    {   return aSet.GetNumRule(bInP); }
inline const SvxScriptSpaceItem& SwFmt::GetScriptSpace(BOOL bInP) const
    {   return aSet.GetScriptSpace(bInP) ; }
inline const SvxHangingPunctuationItem &SwFmt::GetHangingPunctuation(BOOL bInP) const
    {   return aSet.GetHangingPunctuation(bInP) ; }
inline const SvxForbiddenRuleItem &SwFmt::GetForbiddenRule(BOOL bInP) const
    {   return (const SvxForbiddenRuleItem&)aSet.Get(RES_PARATR_FORBIDDEN_RULES, bInP); }
inline const SvxParaVertAlignItem &SwFmt::GetParaVertAlign(BOOL bInP) const
    {   return (const SvxParaVertAlignItem&)aSet.Get( RES_PARATR_VERTALIGN, bInP ); }
inline const SvxParaGridItem &SwFmt::GetParaGrid(BOOL bInP) const
    {   return (const SvxParaGridItem&)aSet.Get( RES_PARATR_SNAPTOGRID, bInP ); }
inline const SwParaConnectBorderItem &SwFmt::GetParaConnectBorder(BOOL bInP) const
    {   return (const SwParaConnectBorderItem&)aSet.Get( RES_PARATR_CONNECT_BORDER, bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

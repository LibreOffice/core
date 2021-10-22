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
#define NUM_ATTRIBUTE_STACKS 44

#include <memory>
#include <vector>
#include <swfntcch.hxx>

class SwTextAttr;
class SwAttrSet;
class IDocumentSettingAccess;
class SwViewShell;
class SfxPoolItem;
extern const sal_uInt8 StackPos[];

/**
 * Used by Attribute Iterators to organize attributes on stacks to
 * find the valid attribute in each category
 */
class SwAttrHandler
{
private:
    std::vector<const SwTextAttr*> m_aAttrStack[NUM_ATTRIBUTE_STACKS]; // stack collection
    const SfxPoolItem* m_pDefaultArray[ NUM_DEFAULT_VALUES ];
    const IDocumentSettingAccess* m_pIDocumentSettingAccess;
    const SwViewShell* m_pShell;

    // This is the base font for the paragraph. It is stored in order to have
    // a template, if we have to restart the attribute evaluation
    std::unique_ptr<SwFont> m_pFnt;

    bool m_bVertLayout;
    bool m_bVertLayoutLRBT;

    const SwTextAttr* GetTop(sal_uInt16 nStack);
    void RemoveFromStack(sal_uInt16 nWhich, const SwTextAttr& rAttr);

    // change font according to pool item
    void FontChg(const SfxPoolItem& rItem, SwFont& rFnt, bool bPush );

    // push attribute to specified stack, returns true, if attribute has
    // been pushed on top of stack (important for stacks containing different
    // attributes with different priority and redlining)
    bool Push( const SwTextAttr& rAttr, const SfxPoolItem& rItem );

    // apply top attribute on stack to font
    void ActivateTop( SwFont& rFnt, sal_uInt16 nStackPos );

public:
    // Ctor
    SwAttrHandler();
    ~SwAttrHandler();

    // set default attributes to values in rAttrSet or from cache
    void Init( const SwAttrSet& rAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess );
    void Init( const SfxPoolItem** pPoolItem, const SwAttrSet* pAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess,
               const SwViewShell* pShell, SwFont& rFnt,
               bool bVertLayout, bool bVertLayoutLRBT );

    bool IsVertLayout() const { return m_bVertLayout; }

    // remove everything from internal stacks, keep default data
    void Reset( );

    // insert specified attribute and change font
    void PushAndChg( const SwTextAttr& rAttr, SwFont& rFnt );

    // remove specified attribute and reset font
    void PopAndChg( const SwTextAttr& rAttr, SwFont& rFnt );
    void Pop( const SwTextAttr& rAttr );

    // apply script dependent attributes
    // void ChangeScript( SwFont& rFnt, const sal_uInt8 nScr );

    // do not call these if you only used the small init function
    inline void ResetFont( SwFont& rFnt ) const;
    inline const SwFont* GetFont() const;

    void GetDefaultAscentAndHeight(SwViewShell const * pShell,
                                   OutputDevice const & rOut,
                                   sal_uInt16& nAscent,
                                   sal_uInt16& nHeight) const;
};

inline void SwAttrHandler::ResetFont( SwFont& rFnt ) const
{
    OSL_ENSURE(m_pFnt, "ResetFont without a font");
    if (m_pFnt)
        rFnt = *m_pFnt;
};

inline const SwFont* SwAttrHandler::GetFont() const
{
    return m_pFnt.get();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
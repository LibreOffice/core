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

#ifndef _ATRHNDL_HXX
#define _ATRHNDL_HXX

#define INITIAL_NUM_ATTR 3
#define NUM_ATTRIBUTE_STACKS 42

#include <txatbase.hxx>
#include <swfntcch.hxx>

class SwAttrSet;
class IDocumentSettingAccess;
class ViewShell;
class SfxPoolItem;
extern const sal_uInt8 StackPos[];

/*************************************************************************
 *                      class SwAttrHandler
 *
 * Used by Attribute Iterators to organize attributes on stacks to
 * find the valid attribute in each category
 *************************************************************************/

class SwAttrHandler
{
private:

    /*************************************************************************
     *                      class SwAttrStack
     *
     * Container for SwTxtAttr Objects
     *************************************************************************/

    class SwAttrStack
    {
    private:
        SwTxtAttr* pInitialArray[ INITIAL_NUM_ATTR ];
        SwTxtAttr** pArray;
        sal_uInt16 nCount; // number of elements on stack
        sal_uInt16 nSize;    // number of positions in Array

    public:
        // Ctor, Dtor
        inline SwAttrStack();
        inline ~SwAttrStack() {
            if ( nSize > INITIAL_NUM_ATTR ) delete [] pArray; }

        // reset stack
        inline void Reset() { nCount = 0; };

        // insert on top
        inline void Push( const SwTxtAttr& rAttr ) { Insert( rAttr, nCount ); };
        // insert at specified position, take care for not inserting behind
        // the value returned by Count()
        void Insert( const SwTxtAttr& rAttr, const sal_uInt16 nPos );

        // remove specified attribute
        void Remove( const SwTxtAttr& rAttr );

        // get attribute from top if exists, otherwise 0
        const SwTxtAttr* Top() const;

        // number of elements on stack
        inline sal_uInt16 Count() const { return nCount; };

        // returns position of rAttr on Stack if found, otherwise USHRT_MAX
        // can be used for Remove of an attribute
        sal_uInt16 Pos( const SwTxtAttr& rAttr ) const;
    };

    SwAttrStack aAttrStack[ NUM_ATTRIBUTE_STACKS ]; // stack collection
    const SfxPoolItem* pDefaultArray[ NUM_DEFAULT_VALUES ];
    const IDocumentSettingAccess* mpIDocumentSettingAccess;
    const ViewShell* mpShell;

    // This is the base font for the paragraph. It is stored in order to have
    // a template, if we have to restart the attribute evaluation
    SwFont* pFnt;

    sal_Bool bVertLayout;

    // change font according to pool item
    void FontChg(const SfxPoolItem& rItem, SwFont& rFnt, sal_Bool bPush );

    // push attribute to specified stack, returns true, if attribute has
    // been pushed on top of stack (important for stacks containing different
    // attributes with different priority and redlining)
    sal_Bool Push( const SwTxtAttr& rAttr, const SfxPoolItem& rItem );

    // apply top attribute on stack to font
    void ActivateTop( SwFont& rFnt, sal_uInt16 nStackPos );

public:
    // Ctor
    SwAttrHandler();
    ~SwAttrHandler();

    // set default attributes to values in rAttrSet or from cache
    void Init( const SwAttrSet& rAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess,
               const ViewShell* pShell );
    void Init( const SfxPoolItem** pPoolItem, const SwAttrSet* pAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess,
               const ViewShell* pShell, SwFont& rFnt,
               sal_Bool bVertLayout );

    // remove everything from internal stacks, keep default data
    void Reset( );

    // insert specified attribute and change font
    void PushAndChg( const SwTxtAttr& rAttr, SwFont& rFnt );

    // remove specified attribute and reset font
    void PopAndChg( const SwTxtAttr& rAttr, SwFont& rFnt );
    void Pop( const SwTxtAttr& rAttr );

    // apply script dependent attributes
    // void ChangeScript( SwFont& rFnt, const sal_uInt8 nScr );

    // returns the default value for stack nStack
    inline const SfxPoolItem& GetDefault( const sal_uInt16 nAttribID ) const;
    // do not call these if you only used the small init function
    inline void ResetFont( SwFont& rFnt ) const;
    inline const SwFont* GetFont() const;

    void GetDefaultAscentAndHeight(ViewShell* pShell,
                                   OutputDevice& rOut,
                                   sal_uInt16& nAscent,
                                   sal_uInt16& nHeight) const;
};

inline const SfxPoolItem& SwAttrHandler::GetDefault( const sal_uInt16 nAttribID ) const
{
    OSL_ENSURE( nAttribID < RES_TXTATR_END,
            "this attrib does not ex."
            );
    OSL_ENSURE( pDefaultArray[ StackPos[ nAttribID ] ], "array not initialized" );
    return *pDefaultArray[ StackPos[ nAttribID ] ];
}

inline void SwAttrHandler::ResetFont( SwFont& rFnt ) const
{
    OSL_ENSURE( pFnt, "ResetFont without a font" );
    if ( pFnt )
        rFnt = *pFnt;
};

inline const SwFont* SwAttrHandler::GetFont() const
{
    return pFnt;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

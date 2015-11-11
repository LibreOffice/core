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

#include <svl/itemset.hxx>
#include <vcl/virdev.hxx>

#include "undostyl.hxx"
#include "docsh.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "printfun.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "globstr.hrc"


//      modify style (cell or page style)

ScStyleSaveData::ScStyleSaveData() :
    pItems( nullptr )
{
}

ScStyleSaveData::ScStyleSaveData( const ScStyleSaveData& rOther ) :
    aName( rOther.aName ),
    aParent( rOther.aParent )
{
    if (rOther.pItems)
        pItems = new SfxItemSet( *rOther.pItems );
    else
        pItems = nullptr;
}

ScStyleSaveData::~ScStyleSaveData()
{
    delete pItems;
}

ScStyleSaveData& ScStyleSaveData::operator=( const ScStyleSaveData& rOther )
{
    aName   = rOther.aName;
    aParent = rOther.aParent;

    delete pItems;
    if (rOther.pItems)
        pItems = new SfxItemSet( *rOther.pItems );
    else
        pItems = nullptr;

    return *this;
}

void ScStyleSaveData::InitFromStyle( const SfxStyleSheetBase* pSource )
{
    if ( pSource )
    {
        aName   = pSource->GetName();
        aParent = pSource->GetParent();
        delete pItems;
        pItems = new SfxItemSet( const_cast<SfxStyleSheetBase*>(pSource)->GetItemSet() );
    }
    else
        *this = ScStyleSaveData();      // empty
}

ScUndoModifyStyle::ScUndoModifyStyle( ScDocShell* pDocSh, SfxStyleFamily eFam,
                    const ScStyleSaveData& rOld, const ScStyleSaveData& rNew ) :
    ScSimpleUndo( pDocSh ),
    eFamily( eFam ),
    aOldData( rOld ),
    aNewData( rNew )
{
}

ScUndoModifyStyle::~ScUndoModifyStyle()
{
}

OUString ScUndoModifyStyle::GetComment() const
{
    sal_uInt16 nId = (eFamily == SFX_STYLE_FAMILY_PARA) ?
                                STR_UNDO_EDITCELLSTYLE :
                                STR_UNDO_EDITPAGESTYLE;
    return ScGlobal::GetRscString( nId );
}

static void lcl_DocStyleChanged( ScDocument* pDoc, SfxStyleSheetBase* pStyle, bool bRemoved )
{
    //! move to document or docshell

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    Point aLogic = pVDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aLogic.X() / 1000.0;
    double nPPTY = aLogic.Y() / 1000.0;
    Fraction aZoom(1,1);
    pDoc->StyleSheetChanged( pStyle, bRemoved, pVDev, nPPTX, nPPTY, aZoom, aZoom );

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}

void ScUndoModifyStyle::DoChange( ScDocShell* pDocSh, const OUString& rName,
                                    SfxStyleFamily eStyleFamily, const ScStyleSaveData& rData )
{
    ScDocument& rDoc = pDocSh->GetDocument();
    ScStyleSheetPool* pStlPool = rDoc.GetStyleSheetPool();
    OUString aNewName = rData.GetName();
    bool bDelete = aNewName.isEmpty();         // no new name -> delete style
    bool bNew = ( rName.isEmpty() && !bDelete );   // creating new style

    SfxStyleSheetBase* pStyle = nullptr;
    if ( !rName.isEmpty() )
    {
        // find old style to modify
        pStyle = pStlPool->Find( rName, eStyleFamily );
        OSL_ENSURE( pStyle, "style not found" );

        if ( pStyle && !bDelete )
        {
            // set new name
            pStyle->SetName( aNewName );
        }
    }
    else if ( !bDelete )
    {
        // create style (with new name)
        pStyle = &pStlPool->Make( aNewName, eStyleFamily, SFXSTYLEBIT_USERDEF );

        if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
            rDoc.GetPool()->CellStyleCreated( aNewName, &rDoc );
    }

    if ( pStyle )
    {
        if ( bDelete )
        {
            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
                lcl_DocStyleChanged( &rDoc, pStyle, true );      // TRUE: remove usage of style
            else
                rDoc.RemovePageStyleInUse( rName );

            // delete style
            pStlPool->Remove( pStyle );
        }
        else
        {
            // modify style

            OUString aNewParent = rData.GetParent();
            if ( aNewParent != pStyle->GetParent() )
                pStyle->SetParent( aNewParent );

            SfxItemSet& rStyleSet = pStyle->GetItemSet();
            const SfxItemSet* pNewSet = rData.GetItems();
            OSL_ENSURE( pNewSet, "no ItemSet for style" );
            if (pNewSet)
                rStyleSet.Set( *pNewSet, false );

            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
            {
                lcl_DocStyleChanged( &rDoc, pStyle, false );     // cell styles: row heights
            }
            else
            {
                // page styles

                if ( bNew && aNewName != rName )
                    rDoc.RenamePageStyleInUse( rName, aNewName );

                if (pNewSet)
                    rDoc.ModifyStyleSheet( *pStyle, *pNewSet );

                pDocSh->PageStyleModified( aNewName, true );
            }
        }
    }

    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );

    //! undo/redo document modifications for deleted styles
    //! undo/redo modifications of number formatter
}

void ScUndoModifyStyle::Undo()
{
    BeginUndo();
    DoChange( pDocShell, aNewData.GetName(), eFamily, aOldData );
    EndUndo();
}

void ScUndoModifyStyle::Redo()
{
    BeginRedo();
    DoChange( pDocShell, aOldData.GetName(), eFamily, aNewData );
    EndRedo();
}

void ScUndoModifyStyle::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

bool ScUndoModifyStyle::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // no repeat possible
}

//      apply page style

ScUndoApplyPageStyle::ApplyStyleEntry::ApplyStyleEntry( SCTAB nTab, const OUString& rOldStyle ) :
    mnTab( nTab ),
    maOldStyle( rOldStyle )
{
}

ScUndoApplyPageStyle::ScUndoApplyPageStyle( ScDocShell* pDocSh, const OUString& rNewStyle ) :
    ScSimpleUndo( pDocSh ),
    maNewStyle( rNewStyle )
{
}

ScUndoApplyPageStyle::~ScUndoApplyPageStyle()
{
}

void ScUndoApplyPageStyle::AddSheetAction( SCTAB nTab, const OUString& rOldStyle )
{
    maEntries.push_back( ApplyStyleEntry( nTab, rOldStyle ) );
}

OUString ScUndoApplyPageStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYPAGESTYLE );
}

void ScUndoApplyPageStyle::Undo()
{
    BeginUndo();
    for( ApplyStyleVec::const_iterator aIt = maEntries.begin(), aEnd = maEntries.end(); aIt != aEnd; ++aIt )
    {
        pDocShell->GetDocument().SetPageStyle( aIt->mnTab, aIt->maOldStyle );
        ScPrintFunc( pDocShell, pDocShell->GetPrinter(), aIt->mnTab ).UpdatePages();
    }
    EndUndo();
}

void ScUndoApplyPageStyle::Redo()
{
    BeginRedo();
    for( ApplyStyleVec::const_iterator aIt = maEntries.begin(), aEnd = maEntries.end(); aIt != aEnd; ++aIt )
    {
        pDocShell->GetDocument().SetPageStyle( aIt->mnTab, maNewStyle );
        ScPrintFunc( pDocShell, pDocShell->GetPrinter(), aIt->mnTab ).UpdatePages();
    }
    EndRedo();
}

void ScUndoApplyPageStyle::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //! set same page style to current tab
}

bool ScUndoApplyPageStyle::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

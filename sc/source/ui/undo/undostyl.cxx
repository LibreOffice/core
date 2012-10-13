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

// -----------------------------------------------------------------------

TYPEINIT1(ScUndoModifyStyle, ScSimpleUndo);
TYPEINIT1(ScUndoApplyPageStyle, ScSimpleUndo);

// -----------------------------------------------------------------------
//
//      modify style (cell or page style)
//

ScStyleSaveData::ScStyleSaveData() :
    pItems( NULL )
{
}

ScStyleSaveData::ScStyleSaveData( const ScStyleSaveData& rOther ) :
    aName( rOther.aName ),
    aParent( rOther.aParent )
{
    if (rOther.pItems)
        pItems = new SfxItemSet( *rOther.pItems );
    else
        pItems = NULL;
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
        pItems = NULL;

    return *this;
}

void ScStyleSaveData::InitFromStyle( const SfxStyleSheetBase* pSource )
{
    if ( pSource )
    {
        aName   = pSource->GetName();
        aParent = pSource->GetParent();
        delete pItems;
        pItems = new SfxItemSet( ((SfxStyleSheetBase*)pSource)->GetItemSet() );
    }
    else
        *this = ScStyleSaveData();      // empty
}

// -----------------------------------------------------------------------

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

rtl::OUString ScUndoModifyStyle::GetComment() const
{
    sal_uInt16 nId = (eFamily == SFX_STYLE_FAMILY_PARA) ?
                                STR_UNDO_EDITCELLSTYLE :
                                STR_UNDO_EDITPAGESTYLE;
    return ScGlobal::GetRscString( nId );
}

static void lcl_DocStyleChanged( ScDocument* pDoc, SfxStyleSheetBase* pStyle, sal_Bool bRemoved )
{
    //! move to document or docshell

    VirtualDevice aVDev;
    Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aLogic.X() / 1000.0;
    double nPPTY = aLogic.Y() / 1000.0;
    Fraction aZoom(1,1);
    pDoc->StyleSheetChanged( pStyle, bRemoved, &aVDev, nPPTX, nPPTY, aZoom, aZoom );

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}

void ScUndoModifyStyle::DoChange( ScDocShell* pDocSh, const String& rName,
                                    SfxStyleFamily eStyleFamily, const ScStyleSaveData& rData )
{
    ScDocument* pDoc = pDocSh->GetDocument();
    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    String aNewName = rData.GetName();
    sal_Bool bDelete = ( aNewName.Len() == 0 );         // no new name -> delete style
    sal_Bool bNew = ( rName.Len() == 0 && !bDelete );   // creating new style

    SfxStyleSheetBase* pStyle = NULL;
    if ( rName.Len() )
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
            pDoc->GetPool()->CellStyleCreated( aNewName );
    }

    if ( pStyle )
    {
        if ( bDelete )
        {
            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
                lcl_DocStyleChanged( pDoc, pStyle, sal_True );      // TRUE: remove usage of style
            else
                pDoc->RemovePageStyleInUse( rName );

            // delete style
            pStlPool->Remove( pStyle );
        }
        else
        {
            // modify style

            String aNewParent = rData.GetParent();
            if ( aNewParent != pStyle->GetParent() )
                pStyle->SetParent( aNewParent );

            SfxItemSet& rStyleSet = pStyle->GetItemSet();
            const SfxItemSet* pNewSet = rData.GetItems();
            OSL_ENSURE( pNewSet, "no ItemSet for style" );
            if (pNewSet)
                rStyleSet.Set( *pNewSet, false );

            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
            {
                lcl_DocStyleChanged( pDoc, pStyle, false );     // cell styles: row heights
            }
            else
            {
                // page styles

                if ( bNew && aNewName != rName )
                    pDoc->RenamePageStyleInUse( rName, aNewName );

                if (pNewSet)
                    pDoc->ModifyStyleSheet( *pStyle, *pNewSet );

                pDocSh->PageStyleModified( aNewName, sal_True );
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

sal_Bool ScUndoModifyStyle::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // no repeat possible
}

// -----------------------------------------------------------------------
//
//      apply page style
//
ScUndoApplyPageStyle::ApplyStyleEntry::ApplyStyleEntry( SCTAB nTab, const String& rOldStyle ) :
    mnTab( nTab ),
    maOldStyle( rOldStyle )
{
}

ScUndoApplyPageStyle::ScUndoApplyPageStyle( ScDocShell* pDocSh, const String& rNewStyle ) :
    ScSimpleUndo( pDocSh ),
    maNewStyle( rNewStyle )
{
}

ScUndoApplyPageStyle::~ScUndoApplyPageStyle()
{
}

void ScUndoApplyPageStyle::AddSheetAction( SCTAB nTab, const String& rOldStyle )
{
    maEntries.push_back( ApplyStyleEntry( nTab, rOldStyle ) );
}

rtl::OUString ScUndoApplyPageStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYPAGESTYLE );
}

void ScUndoApplyPageStyle::Undo()
{
    BeginUndo();
    for( ApplyStyleVec::const_iterator aIt = maEntries.begin(), aEnd = maEntries.end(); aIt != aEnd; ++aIt )
    {
        pDocShell->GetDocument()->SetPageStyle( aIt->mnTab, aIt->maOldStyle );
        ScPrintFunc( pDocShell, pDocShell->GetPrinter(), aIt->mnTab ).UpdatePages();
    }
    EndUndo();
}

void ScUndoApplyPageStyle::Redo()
{
    BeginRedo();
    for( ApplyStyleVec::const_iterator aIt = maEntries.begin(), aEnd = maEntries.end(); aIt != aEnd; ++aIt )
    {
        pDocShell->GetDocument()->SetPageStyle( aIt->mnTab, maNewStyle );
        ScPrintFunc( pDocShell, pDocShell->GetPrinter(), aIt->mnTab ).UpdatePages();
    }
    EndRedo();
}

void ScUndoApplyPageStyle::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //! set same page style to current tab
}

sal_Bool ScUndoApplyPageStyle::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: undostyl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svtools/itemset.hxx>
#include <vcl/virdev.hxx>

#include "undostyl.hxx"
#include "docsh.hxx"
#include "stlpool.hxx"
#include "printfun.hxx"
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

String ScUndoModifyStyle::GetComment() const
{
    USHORT nId = (eFamily == SFX_STYLE_FAMILY_PARA) ?
                                STR_UNDO_EDITCELLSTYLE :
                                STR_UNDO_EDITPAGESTYLE;
    return ScGlobal::GetRscString( nId );
}

void lcl_DocStyleChanged( ScDocument* pDoc, SfxStyleSheetBase* pStyle, BOOL bRemoved )
{
    //! move to document or docshell

    VirtualDevice aVDev;
    Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aLogic.X() / 1000.0;
    double nPPTY = aLogic.Y() / 1000.0;
    Fraction aZoom(1,1);
    pDoc->StyleSheetChanged( pStyle, bRemoved, &aVDev, nPPTX, nPPTY, aZoom, aZoom );
}

// static
void ScUndoModifyStyle::DoChange( ScDocShell* pDocSh, const String& rName,
                                    SfxStyleFamily eStyleFamily, const ScStyleSaveData& rData )
{
    ScDocument* pDoc = pDocSh->GetDocument();
    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    String aNewName = rData.GetName();
    BOOL bDelete = ( aNewName.Len() == 0 );         // no new name -> delete style
    BOOL bNew = ( rName.Len() == 0 && !bDelete );   // creating new style

    SfxStyleSheetBase* pStyle = NULL;
    if ( rName.Len() )
    {
        // find old style to modify
        pStyle = pStlPool->Find( rName, eStyleFamily );
        DBG_ASSERT( pStyle, "style not found" );

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
    }

    if ( pStyle )
    {
        if ( bDelete )
        {
            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
                lcl_DocStyleChanged( pDoc, pStyle, TRUE );      // TRUE: remove usage of style
            else
                pDoc->RemovePageStyleInUse( rName );

            // delete style
            pStlPool->Erase( pStyle );
        }
        else
        {
            // modify style

            String aNewParent = rData.GetParent();
            if ( aNewParent != pStyle->GetParent() )
                pStyle->SetParent( aNewParent );

            SfxItemSet& rStyleSet = pStyle->GetItemSet();
            const SfxItemSet* pNewSet = rData.GetItems();
            DBG_ASSERT( pNewSet, "no ItemSet for style" );
            if (pNewSet)
                rStyleSet.Set( *pNewSet, FALSE );

            if ( eStyleFamily == SFX_STYLE_FAMILY_PARA )
            {
                lcl_DocStyleChanged( pDoc, pStyle, FALSE );     // cell styles: row heights
            }
            else
            {
                // page styles

                if ( bNew && aNewName != rName )
                    pDoc->RenamePageStyleInUse( rName, aNewName );

                if (pNewSet)
                    pDoc->ModifyStyleSheet( *pStyle, *pNewSet );

                pDocSh->PageStyleModified( aNewName, TRUE );
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

void ScUndoModifyStyle::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL ScUndoModifyStyle::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;       // no repeat possible
}

// -----------------------------------------------------------------------
//
//      apply page style
//

ScUndoApplyPageStyle::ScUndoApplyPageStyle( ScDocShell* pDocSh, USHORT nT,
                    const String& rOld, const String& rNew ) :
    ScSimpleUndo( pDocSh ),
    nTab( nT ),
    aOldStyle( rOld ),
    aNewStyle( rNew )
{
}

ScUndoApplyPageStyle::~ScUndoApplyPageStyle()
{
}

String ScUndoApplyPageStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYPAGESTYLE );
}

void ScUndoApplyPageStyle::Undo()
{
    BeginUndo();

    pDocShell->GetDocument()->SetPageStyle( nTab, aOldStyle );

    ScPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab ).UpdatePages();
    EndUndo();
}

void ScUndoApplyPageStyle::Redo()
{
    BeginRedo();

    pDocShell->GetDocument()->SetPageStyle( nTab, aNewStyle );

    ScPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab ).UpdatePages();
    EndRedo();
}

void ScUndoApplyPageStyle::Repeat(SfxRepeatTarget& rTarget)
{
    //! set same page style to current tab
}

BOOL ScUndoApplyPageStyle::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}



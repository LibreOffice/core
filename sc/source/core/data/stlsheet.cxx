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

#include "document.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/printer.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/smplhint.hxx>
#include "attrib.hxx"


#include <vcl/svapp.hxx>

#include "globstr.hrc"
#include "sc.hrc"


TYPEINIT1(ScStyleSheet, SfxStyleSheet);

#define TWO_CM      1134
#define HFDIST_CM   142

//========================================================================

ScStyleSheet::ScStyleSheet( const OUString&     rName,
                            ScStyleSheetPool&   rPoolP,
                            SfxStyleFamily      eFamily,
                            sal_uInt16          nMaskP )

    : SfxStyleSheet   ( rName, rPoolP, eFamily, nMaskP )
    , eUsage( UNKNOWN )
{
}



ScStyleSheet::ScStyleSheet( const ScStyleSheet& rStyle )
    : SfxStyleSheet ( rStyle )
    , eUsage( UNKNOWN )
{
}



ScStyleSheet::~ScStyleSheet()
{
}



bool ScStyleSheet::HasFollowSupport() const
{
    return false;
}



bool ScStyleSheet::HasParentSupport () const
{
    bool bHasParentSupport = false;

    switch ( GetFamily() )
    {
    case SFX_STYLE_FAMILY_PARA: bHasParentSupport = true;   break;
    case SFX_STYLE_FAMILY_PAGE: bHasParentSupport = false;  break;
    default:
        {
            // added to avoid warnings
        }
    }

    return bHasParentSupport;
}



bool ScStyleSheet::SetParent( const OUString& rParentName )
{
    bool bResult = false;
    OUString aEffName = rParentName;
    SfxStyleSheetBase* pStyle = pPool->Find( aEffName, nFamily );
    if (!pStyle)
    {
        SfxStyleSheetIteratorPtr pIter = pPool->CreateIterator( nFamily, SFXSTYLEBIT_ALL );
        pStyle = pIter->First();
        if (pStyle)
            aEffName = pStyle->GetName();
    }

    if ( pStyle && aEffName != GetName() )
    {
        bResult = SfxStyleSheet::SetParent( aEffName );
        if (bResult)
        {
            SfxItemSet& rParentSet = pStyle->GetItemSet();
            GetItemSet().SetParent( &rParentSet );

            // #i113491# Drag&Drop in the stylist's hierarchical view doesn't execute a slot,
            // so the repaint has to come from here (after modifying the ItemSet).
            // RepaintRange checks the document's IsVisible flag and locked repaints.
            ScDocument* pDoc = static_cast<ScStyleSheetPool&>(GetPool()).GetDocument();
            if (pDoc)
                pDoc->RepaintRange( ScRange( 0,0,0, MAXCOL,MAXROW,MAXTAB ) );
        }
    }

    return bResult;
}



SfxItemSet& ScStyleSheet::GetItemSet()
{
    if ( !pSet )
    {
        switch ( GetFamily() )
        {
            case SFX_STYLE_FAMILY_PAGE:
                {
                    // Seitenvorlagen sollen nicht ableitbar sein,
                    // deshalb werden an dieser Stelle geeignete
                    // Werte eingestellt. (==Standard-Seitenvorlage)

                    SfxItemPool& rItemPool = GetPool().GetPool();
                    pSet = new SfxItemSet( rItemPool,
                                           ATTR_BACKGROUND, ATTR_BACKGROUND,
                                           ATTR_BORDER, ATTR_SHADOW,
                                           ATTR_LRSPACE, ATTR_PAGE_SCALETO,
                                           ATTR_WRITINGDIR, ATTR_WRITINGDIR,
                                           ATTR_USERDEF, ATTR_USERDEF,
                                           0 );

                    //  Wenn gerade geladen wird, wird auch der Set hinterher aus der Datei
                    //  gefuellt, es brauchen also keine Defaults gesetzt zu werden.
                    //  GetPrinter wuerde dann auch einen neuen Printer anlegen, weil der
                    //  gespeicherte Printer noch nicht geladen ist!

                    ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
                    if ( pDoc )
                    {
                        // Setzen von sinnvollen Default-Werten:
                        SvxPageItem     aPageItem( ATTR_PAGE );
                        SvxSizeItem     aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetDefaultPaperSize() );

                        SvxSetItem      aHFSetItem(
                                            (const SvxSetItem&)
                                            rItemPool.GetDefaultItem(ATTR_PAGE_HEADERSET) );

                        SfxItemSet&     rHFSet = aHFSetItem.GetItemSet();
                        SvxSizeItem     aHFSizeItem( // 0,5 cm + Abstand
                                            ATTR_PAGE_SIZE,
                                            Size( 0, (long)( 500 / HMM_PER_TWIPS ) + HFDIST_CM ) );

                        SvxULSpaceItem  aHFDistItem ( HFDIST_CM,// nUp
                                                      HFDIST_CM,// nLow
                                                      ATTR_ULSPACE );

                        SvxLRSpaceItem  aLRSpaceItem( TWO_CM,   // nLeft
                                                      TWO_CM,   // nRight
                                                      TWO_CM,   // nTLeft
                                                      0,        // nFirstLineOffset
                                                      ATTR_LRSPACE );
                        SvxULSpaceItem  aULSpaceItem( TWO_CM,   // nUp
                                                      TWO_CM,   // nLow
                                                      ATTR_ULSPACE );
                        SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                        aBoxInfoItem.SetTable( false );
                        aBoxInfoItem.SetDist( true );
                        aBoxInfoItem.SetValid( VALID_DISTANCE, true );

                        aPageItem.SetLandscape( false );

                        rHFSet.Put( aBoxInfoItem );
                        rHFSet.Put( aHFSizeItem );
                        rHFSet.Put( aHFDistItem );
                        rHFSet.Put( SvxLRSpaceItem( 0,0,0,0, ATTR_LRSPACE ) ); // Rand auf Null setzen

                        pSet->Put( aHFSetItem, ATTR_PAGE_HEADERSET );
                        pSet->Put( aHFSetItem, ATTR_PAGE_FOOTERSET );
                        pSet->Put( aBoxInfoItem ); // PoolDefault wg. Formatvorlagen
                                                   // nicht ueberschreiben!

                        //  Writing direction: not as pool default because the default for cells
                        //  must remain FRMDIR_ENVIRONMENT, and each page style's setting is
                        //  supposed to be saved in the file format.
                        //  The page default depends on the system language.
                        SvxFrameDirection eDirection = ScGlobal::IsSystemRTL() ?
                                        FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
                        pSet->Put( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ), ATTR_WRITINGDIR );

                        rItemPool.SetPoolDefaultItem( aPageItem );
                        rItemPool.SetPoolDefaultItem( aPaperSizeItem );
                        rItemPool.SetPoolDefaultItem( aLRSpaceItem );
                        rItemPool.SetPoolDefaultItem( aULSpaceItem );
                        rItemPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
                        ScPageScaleToItem aScaleToItem;
                        rItemPool.SetPoolDefaultItem( aScaleToItem );
                        rItemPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 0 ) );
                    }
                }
                break;

            case SFX_STYLE_FAMILY_PARA:
            default:
                pSet = new SfxItemSet( GetPool().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );
                break;
        }
        bMySet = true;
    }
    if ( nHelpId == HID_SC_SHEET_CELL_ERG1 )
    {
        if ( !pSet->Count() )
        {
            ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
            if ( pDoc )
            {
                sal_uLong nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( NUMBERFORMAT_CURRENCY,ScGlobal::eLnge );
                pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );
            }
        }
    }

    return *pSet;
}



bool ScStyleSheet::IsUsed() const
{
    if ( GetFamily() == SFX_STYLE_FAMILY_PARA )
    {
        // Always query the document to let it decide if a rescan is necessary,
        // and store the state.
        ScDocument* pDoc = ((ScStyleSheetPool*)pPool)->GetDocument();
        if ( pDoc && pDoc->IsStyleSheetUsed( *this, true ) )
            eUsage = USED;
        else
            eUsage = NOTUSED;
        return eUsage == USED;
    }
    else
        return true;
}



void ScStyleSheet::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) )
        if ( ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            GetItemSet().SetParent( NULL );
}



//  schmutzige Tricks, um die Standard-Vorlage immer als "Standard" zu speichern,
//  obwohl der fuer den Benutzer sichtbare Name uebersetzt ist:

const OUString& ScStyleSheet::GetName() const
{
    const OUString& rBase = SfxStyleSheet::GetName();
    const OUString* pForceStdName = ((ScStyleSheetPool*)pPool)->GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const OUString& ScStyleSheet::GetParent() const
{
    const OUString& rBase = SfxStyleSheet::GetParent();
    const OUString* pForceStdName = ((ScStyleSheetPool*)pPool)->GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const OUString& ScStyleSheet::GetFollow() const
{
    const OUString& rBase = SfxStyleSheet::GetFollow();
    const OUString* pForceStdName = ((ScStyleSheetPool*)pPool)->GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

//  Verhindern, dass ein Style "Standard" angelegt wird, wenn das nicht der
//  Standard-Name ist, weil sonst beim Speichern zwei Styles denselben Namen haetten
//  (Beim Laden wird der Style direkt per Make mit dem Namen erzeugt, so dass diese
//  Abfrage dann nicht gilt)
//! Wenn irgendwann aus dem Laden SetName aufgerufen wird, muss fuer das Laden ein
//! Flag gesetzt und abgefragt werden.
//! Die ganze Abfrage muss raus, wenn fuer eine neue Datei-Version die Namens-Umsetzung wegfaellt.

bool ScStyleSheet::SetName( const OUString& rNew )
{
    OUString aFileStdName = OUString(STRING_STANDARD);
    if ( rNew == aFileStdName && aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return false;
    else
        return SfxStyleSheet::SetName( rNew );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

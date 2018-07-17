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

#include <document.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>

#include <scitems.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <sfx2/printer.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/hint.hxx>
#include <attrib.hxx>

#include <vcl/svapp.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <helpids.h>

#define TWO_CM      1134
#define HFDIST_CM   142

ScStyleSheet::ScStyleSheet( const OUString&     rName,
                            const ScStyleSheetPool& rPoolP,
                            SfxStyleFamily      eFamily,
                            SfxStyleSearchBits  nMaskP )

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
    case SfxStyleFamily::Para: bHasParentSupport = true;   break;
    case SfxStyleFamily::Page: bHasParentSupport = false;  break;
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
    SfxStyleSheetBase* pStyle = m_pPool->Find( aEffName, nFamily );
    if (!pStyle)
    {
        std::unique_ptr<SfxStyleSheetIterator> pIter = m_pPool->CreateIterator( nFamily, SfxStyleSearchBits::All );
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
            ScDocument* pDoc = static_cast<ScStyleSheetPool*>(GetPool())->GetDocument();
            if (pDoc)
                pDoc->RepaintRange( ScRange( 0,0,0, MAXCOL,MAXROW,MAXTAB ) );
        }
    }

    return bResult;
}

void ScStyleSheet::ResetParent()
{
    GetItemSet().SetParent(nullptr);
}

SfxItemSet& ScStyleSheet::GetItemSet()
{
    if ( !pSet )
    {
        switch ( GetFamily() )
        {
            case SfxStyleFamily::Page:
                {
                    // Page templates should not be derivable,
                    // therefore suitable values are set at this point.
                    // (== Standard page template)

                    SfxItemPool& rItemPool = GetPool()->GetPool();
                    pSet = new SfxItemSet(
                        rItemPool,
                        svl::Items<
                            ATTR_USERDEF, ATTR_USERDEF,
                            ATTR_WRITINGDIR, ATTR_WRITINGDIR,
                            ATTR_BACKGROUND, ATTR_BACKGROUND,
                            ATTR_BORDER, ATTR_SHADOW,
                            ATTR_LRSPACE, ATTR_PAGE_SCALETO>{} );

                    //  If being loaded also the set is then filled in from the file,
                    //  so the defaults do not need to be set.
                    //  GetPrinter would then also create a new printer,
                    //  because the stored Printer is not loaded yet!

                    ScDocument* pDoc = static_cast<ScStyleSheetPool*>(GetPool())->GetDocument();
                    if ( pDoc )
                    {
                        // Setting reasonable default values:
                        SvxPageItem     aPageItem( ATTR_PAGE );
                        SvxSizeItem     aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetDefaultPaperSize() );

                        SvxSetItem      aHFSetItem(
                                            rItemPool.GetDefaultItem(ATTR_PAGE_HEADERSET) );

                        SfxItemSet&     rHFSet = aHFSetItem.GetItemSet();
                        SvxSizeItem     aHFSizeItem( // 0,5 cm + distance
                                            ATTR_PAGE_SIZE,
                                            Size( 0, long( 500 / HMM_PER_TWIPS ) + HFDIST_CM ) );

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
                        aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );

                        aPageItem.SetLandscape( false );

                        rHFSet.Put( aBoxInfoItem );
                        rHFSet.Put( aHFSizeItem );
                        rHFSet.Put( aHFDistItem );
                        rHFSet.Put( SvxLRSpaceItem( 0,0,0,0, ATTR_LRSPACE ) ); // Set border to Null

                        aHFSetItem.SetWhich(ATTR_PAGE_HEADERSET);
                        pSet->Put( aHFSetItem );
                        aHFSetItem.SetWhich(ATTR_PAGE_FOOTERSET);
                        pSet->Put( aHFSetItem );
                        pSet->Put( aBoxInfoItem ); // Do not overwrite PoolDefault
                                                   // due to format templates


                        //  Writing direction: not as pool default because the default for cells
                        //  must remain SvxFrameDirection::Environment, and each page style's setting is
                        //  supposed to be saved in the file format.
                        //  The page default depends on the system language.
                        SvxFrameDirection eDirection = ScGlobal::IsSystemRTL() ?
                                        SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB;
                        pSet->Put( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ) );

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

            case SfxStyleFamily::Para:
            default:
                pSet = new SfxItemSet( GetPool()->GetPool(), svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>{} );
                break;
        }
        bMySet = true;
    }
    if ( nHelpId == HID_SC_SHEET_CELL_ERG1 )
    {
        if ( !pSet->Count() )
        {
            // Hack to work around that when this code is called from
            // ~ScStyleSheetPool -> ~SfxStyleSheetPool, GetPool() is no longer
            // an ScStyleSheetPool:
            ScStyleSheetPool * pool = dynamic_cast<ScStyleSheetPool *>(
                GetPool());
            if (pool != nullptr) {
                ScDocument* pDoc = pool->GetDocument();
                if ( pDoc )
                {
                    sal_uInt32 nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( SvNumFormatType::CURRENCY,ScGlobal::eLnge );
                    pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );
                }
            }
        }
    }

    return *pSet;
}

bool ScStyleSheet::IsUsed() const
{
    if ( GetFamily() == SfxStyleFamily::Para )
    {
        // Always query the document to let it decide if a rescan is necessary,
        // and store the state.
        ScDocument* pDoc = static_cast<ScStyleSheetPool*>(m_pPool)->GetDocument();
        if ( pDoc && pDoc->IsStyleSheetUsed( *this ) )
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
    if ( rHint.GetId() == SfxHintId::Dying )
        GetItemSet().SetParent( nullptr );
}

// Avoid creating a Style "Standard" if this is not the Standard-Name;
// otherwise two styles would have the same name when storing.
// (on loading the style is created directly per Make with the name; making this query
//  not applicable)
//TODO: If at any time during loading SetName is called, a flag has to be set/checked for loading
//TODO: The whole check has to be removed if for a new file version the name transformation is dropped.

bool ScStyleSheet::SetName(const OUString& rNew, bool bReindexNow)
{
    OUString aFileStdName = STRING_STANDARD;
    if ( rNew == aFileStdName && aFileStdName != ScResId(STR_STYLENAME_STANDARD) )
        return false;
    else
        return SfxStyleSheet::SetName(rNew, bReindexNow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

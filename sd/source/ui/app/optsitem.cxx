/*************************************************************************
 *
 *  $RCSfile: optsitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:30 $
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

#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif

#include "app.hxx"
#include <optsitem.hxx>
#include "cfgids.hxx"
#include "frmview.hxx"

// STATIC DATA -----------------------------------------------------------

#define VERSION_001     ( (INT16) 1 )
#define VERSION_002     ( (INT16) 2 )
#define VERSION_003     ( (INT16) 3 )
#define VERSION_004     ( (INT16) 4 )
#define VERSION_005     ( (INT16) 5 )
#define VERSION_006     ( (INT16) 6 )
#define VERSION_007     ( (INT16) 7 )
#define VERSION_008     ( (INT16) 8 )
#define VERSION_009     ( (INT16) 9 )
#define VERSION_010     ( (INT16) 10 )
#define VERSION_011     ( (INT16) 11 )
#define VERSION_012     ( (INT16) 12 )
#define VERSION_013     ( (INT16) 13 )
#define VERSION_014     ( (INT16) 14 )
#define VERSION_015     ( (INT16) 15 )
#define VERSION_016     ( (INT16) 16 )
#define VERSION_017     ( (INT16) 17 )
#define VERSION_018     ( (INT16) 18 )
#define VERSION_019     ( (INT16) 19 )
#define VERSION_020     ( (INT16) 20 )
#define VERSION_021     ( (INT16) 21 )
#define VERSION_022     ( (INT16) 22 )

/*************************************************************************
|*
|* SdOptionsLayout
|*
\************************************************************************/
#pragma SEG_FUNCDEF(optsitem_01)

SdOptionsLayout::SdOptionsLayout() :
            bRuler              ( 1 ),
            bHelplines          ( 1 ),
            bHandlesBezier      ( 0 ),
            bMoveOutline        ( 1 ),
            bDragStripes        ( 0 ),
            nMetric             ( 0xffff )
{
}

/*************************************************************************
|*
|* UI-Metrik
|*
\************************************************************************/

UINT16 SdOptionsLayout::GetMetric() const
{
    UINT16 nMet = nMetric;

    if (nMet == 0xffff)
    {
        nMet = SFX_APP()->GetOptions().GetMetric();
    }

    return nMet;
}

/*************************************************************************
|*
|* SdOptionsContents
|*
\************************************************************************/

SdOptionsContents::SdOptionsContents() :
            bExternGraphic      ( 0 ),
            bOutlineMode        ( 0 ),
            bHairlineMode       ( 0 ),
            bNoText             ( 0 ),
            bSolidDragging      ( 0 ),
            bSolidMarkHdl       ( 1 )
{
}

/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

SdOptionsMisc::SdOptionsMisc() :
            bStartWithTemplate  ( 1 ),
            bStartWithLayout    ( 1 ),
            bMarkedHitMovesAlways( 1 ),
            bMoveOnlyDragging   ( 0 ),
            bCrookNoContortion  ( 0 ),
            bQuickEdit          ( 1 ),
            bMasterPageCache    ( 1 ),
            bDragWithCopy       ( 0 ),
            bPickThrough        ( 1 ),
            bBigHandles         ( 0 ),
            bDoubleClickTextEdit( 1 ),
            bClickChangeRotation( 0 ),
            bStartWithActualPage( 0 ),
            nPreviewQuality( DRAWMODE_DEFAULT )
{
}

/*************************************************************************
|*
|* SdOptionsSnap
|*
\************************************************************************/

SdOptionsSnap::SdOptionsSnap() :
            bSnapHelplines      ( 1 ),
            bSnapBorder         ( 1 ),
            bSnapFrame          ( 0 ),
            bSnapPoints         ( 0 ),
            bOrtho              ( 0 ),
            bBigOrtho           ( 1 ),
            bRotate             ( 0 ),
            nSnapArea           ( 5 ),
            nAngle              ( 1500 ),
            nBezAngle           ( 1500 )
{
}

/*************************************************************************
|*
|* SdOptionsPrint
|*
\************************************************************************/

SdOptionsPrint::SdOptionsPrint() :
            bDraw               ( 1 ),
            bNotes              ( 0 ),
            bHandout            ( 0 ),
            bOutline            ( 0 ),
            bDate               ( 0 ),
            bTime               ( 0 ),
            bPagename           ( 0 ),
            bHiddenPages        ( 1 ),
            bPagesize           ( 0 ),
            bPagetile           ( 0 ),
            bWarningPrinter     ( 1 ),
            bWarningSize        ( 0 ),
            bWarningOrientation ( 0 ),
            bBooklet            ( 0 ),
            bFront              ( 1 ),
            bBack               ( 1 ),
            bCutPage            ( 0 ),
            bPaperbin           ( 0 ),
            nQuality            ( 0 )
{
}

/*************************************************************************
|*
|* SdOptions
|*
\************************************************************************/

SdOptions::SdOptions(USHORT nCfgId) :
    SfxConfigItem   ( nCfgId ),
    nX              ( 1 ),
    nY              ( 1 ),
    nConfigId       ( nCfgId )
{
    if ( GetMetric() == FUNIT_INCH )
        nDefTab          = 1270;
    else
        nDefTab          = 1250;
}

// ----------------------------------------------------------------------

void SdOptions::Init()
{
    // DefTab
    if ( GetMetric() == FUNIT_INCH )
        nDefTab          = 1270;
    else
        nDefTab          = 1250;

    nX                   = 1;
    nY                   = 1;

    // Layout
    bRuler               = TRUE;
    bHelplines           = TRUE;
    bHandlesBezier       = FALSE;
    bMoveOutline         = TRUE;
    bDragStripes         = FALSE;
    nMetric              = 0xffff;

    // Contents
    bExternGraphic       = FALSE;
    bOutlineMode         = FALSE;
    bHairlineMode        = FALSE;
    bNoText              = FALSE;
    bSolidDragging       = FALSE;
    bSolidMarkHdl        = TRUE;

    // Misc
    bStartWithTemplate   = TRUE;
    bStartWithLayout     = TRUE;
    bMarkedHitMovesAlways= TRUE;
    bMoveOnlyDragging    = FALSE;
    bCrookNoContortion   = FALSE;
    bMasterPageCache     = TRUE;
    bDragWithCopy        = FALSE;
    bPickThrough         = TRUE;
    bBigHandles          = FALSE;
    bDoubleClickTextEdit = TRUE;
    bClickChangeRotation = FALSE;
    bStartWithActualPage = FALSE;
    nPreviewQuality      = DRAWMODE_DEFAULT;

    if (nConfigId == SDCFG_DRAW)
    {
        bQuickEdit       = FALSE;
    }
    else
    {
        bQuickEdit       = TRUE;
    }

    // Fangen/Einrasten
    bSnapHelplines       = TRUE;
    bSnapBorder          = TRUE;
    bSnapFrame           = FALSE;
    bSnapPoints          = FALSE;
    bOrtho               = FALSE;
    bBigOrtho            = TRUE;
    bRotate              = FALSE;
    nSnapArea            = 5;
    nAngle               = 1500;
    nBezAngle            = 1500;

    // Raster
    if ( nConfigId == SDCFG_DRAW )
    {
        if ( GetMetric() == FUNIT_INCH )
        {
            nFldDivisionX    = 254;
            nFldDivisionY    = 254;
        }
        else
        {
            nFldDivisionX    = 250;
            nFldDivisionY    = 250;
        }
    }
    else
    {
        if ( GetMetric() == FUNIT_INCH )
        {
            nFldDivisionX    = 635;
            nFldDivisionY    = 635;
        }
        else
        {
            nFldDivisionX    = 500;
            nFldDivisionY    = 500;
        }
    }

    if ( GetMetric() == FUNIT_INCH )
    {
        nFldDrawX        = 1270;
        nFldDrawY        = 1270;
        nFldSnapX        = 1270;
        nFldSnapY        = 1270;
    }
    else
    {
        nFldDrawX        = 1000;
        nFldDrawY        = 1000;
        nFldSnapX        = 1000;
        nFldSnapY        = 1000;
    }

    bUseGridsnap         = FALSE;
    bSynchronize         = TRUE;
    bGridVisible         = FALSE;
    bEqualGrid           = TRUE;

    // Drucken
    bDraw                = TRUE;
    bNotes               = FALSE;
    bHandout             = FALSE;
    bOutline             = FALSE;
    bDate                = FALSE;
    bTime                = FALSE;
    bPagename            = FALSE;
    bHiddenPages         = TRUE;
    bPagesize            = FALSE;
    bPagetile            = FALSE;
    bWarningPrinter      = TRUE;
    bWarningSize         = FALSE;
    bWarningOrientation  = FALSE;
    bBooklet             = FALSE;
    bFront               = TRUE;
    bBack                = TRUE;
    bCutPage             = FALSE;
    bPaperbin            = FALSE;
    nQuality             = 0;
}

// ----------------------------------------------------------------------

BOOL __EXPORT SdOptions::Store( SvStream& rOut )
{
    rOut << VERSION_022;

    UINT32 nOut = 0;

    // Objectbar, Statusbar ... werden nicht von uns gestreamt
    nOut |= (UINT32) bRuler                     ;
    nOut |= (UINT32) bHelplines             << 1;
    nOut |= (UINT32) bExternGraphic         << 2;
    nOut |= (UINT32) bOutlineMode           << 3;
    nOut |= (UINT32) bHairlineMode          << 4;
    nOut |= (UINT32) bMoveOutline           << 5;
    nOut |= (UINT32) bDragStripes           << 6;
    nOut |= (UINT32) bHandlesBezier         << 7;
    nOut |= (UINT32) bStartWithTemplate     << 8;
    nOut |= (UINT32) bStartWithLayout       << 9;
    nOut |= (UINT32) bMarkedHitMovesAlways  << 10;
    nOut |= (UINT32) bMoveOnlyDragging      << 11;
    nOut |= (UINT32) bNoText                << 12;
    nOut |= (UINT32) bCrookNoContortion     << 13;
    nOut |= (UINT32) bQuickEdit             << 14;

    nOut |= (UINT32) bSnapHelplines         << 15;
    nOut |= (UINT32) bSnapBorder            << 16;
    nOut |= (UINT32) bSnapFrame             << 17;
    nOut |= (UINT32) bSnapPoints            << 18;
    nOut |= (UINT32) bOrtho                 << 19;
    nOut |= (UINT32) bBigOrtho              << 20;
    nOut |= (UINT32) bRotate                << 21;

    nOut |= (UINT32) bUseGridsnap           << 22;
    nOut |= (UINT32) bSynchronize           << 23;
    nOut |= (UINT32) bGridVisible           << 24;
    nOut |= (UINT32) bEqualGrid             << 25;
    nOut |= (UINT32) bMasterPageCache       << 26;
    nOut |= (UINT32) bDragWithCopy          << 27;
    nOut |= (UINT32) bPickThrough           << 28;

    nOut |= (UINT32) bSolidMarkHdl          << 29;
    nOut |= (UINT32) bSolidDragging         << 30;

    rOut <<  nOut;

    // Print-Optionen
    nOut = 0;
    nOut |= (UINT32) bDraw                   ;
    nOut |= (UINT32) bNotes              << 1;
    nOut |= (UINT32) bHandout            << 2;
    nOut |= (UINT32) bOutline            << 3;
    nOut |= (UINT32) bDate               << 4;
    nOut |= (UINT32) bTime               << 5;
    nOut |= (UINT32) bPagename           << 6;
    nOut |= (UINT32) bHiddenPages        << 7;
    nOut |= (UINT32) bPagesize           << 8;
    nOut |= (UINT32) bPagetile           << 9;
    nOut |= (UINT32) bWarningPrinter     << 10;
    nOut |= (UINT32) bWarningSize        << 11;
    nOut |= (UINT32) bWarningOrientation << 12;
    nOut |= (UINT32) bBooklet            << 13;
    nOut |= (UINT32) bFront              << 14;
    nOut |= (UINT32) bBack               << 15;
    nOut |= (UINT32) bPaperbin           << 16;

    // weitere Misc-Optionen
    nOut |= (UINT32) bBigHandles          << 17;
    nOut |= (UINT32) bDoubleClickTextEdit << 18;
    nOut |= (UINT32) bClickChangeRotation << 19;
    nOut |= (UINT32) bStartWithActualPage << 20; /// NEU

    rOut <<  nOut;

    rOut << nFldDrawX;
    rOut << nFldDivisionX;
    rOut << nFldDrawY;
    rOut << nFldDivisionY;
    rOut << nFldSnapX;
    rOut << nFldSnapY;

    rOut << nSnapArea;
    rOut << nAngle;

    //rOut << nDefTab; Ab 013 raus

    rOut << nBezAngle;

    // und ab der 015 wieder drin ! (hohoho)
    rOut << nDefTab;

    // Massstab
    rOut << nX;
    rOut << nY;

    rOut << nMetric;

    // Ausgabequalitaet
    rOut << nQuality;

    // Ausgabequalitaet Preview
    rOut << nPreviewQuality;
    return SfxConfigItem::ERR_OK;
}

// ----------------------------------------------------------------------
#pragma optimize("",off)


int __EXPORT SdOptions::Load( SvStream& rIn )
{
    SetDefault( FALSE );

    INT16  nVersion;
    rIn >> nVersion;

    if( nVersion >= VERSION_001 )
    {
        UINT32 nIn;
        rIn >> nIn;

        // Objectbar, Statusbar ... werden nicht von uns gestreamt
        bRuler               = (BOOL) ( nIn );
        bHelplines           = (BOOL) ( nIn >> 1 );
        bExternGraphic       = (BOOL) ( nIn >> 2 );
        bOutlineMode         = (BOOL) ( nIn >> 3 );
        bHairlineMode        = (BOOL) ( nIn >> 4 );
        bMoveOutline         = (BOOL) ( nIn >> 5 );
        bDragStripes         = (BOOL) ( nIn >> 6 );

        if( nVersion == VERSION_001 )
        {
            bStartWithTemplate   = (BOOL) ( nIn >> 7 );
            bStartWithLayout     = (BOOL) ( nIn >> 8 );
            bMarkedHitMovesAlways= (BOOL) ( nIn >> 9 );
            bMoveOnlyDragging    = (BOOL) ( nIn >> 10 );
            bNoText              = (BOOL) ( nIn >> 11 );
            bCrookNoContortion   = (BOOL) ( nIn >> 12 );

//          bSnapGrid            = (BOOL) ( nIn >> 13 );
            bSnapHelplines       = (BOOL) ( nIn >> 14 );
            bSnapBorder          = (BOOL) ( nIn >> 15 );
            bSnapFrame           = (BOOL) ( nIn >> 16 );
            bSnapPoints          = (BOOL) ( nIn >> 17 );
            bOrtho               = (BOOL) ( nIn >> 18 );
            bBigOrtho            = (BOOL) ( nIn >> 19 );
            bRotate              = (BOOL) ( nIn >> 20 );
        }
        else if( nVersion >= VERSION_002 )
        {
            bHandlesBezier       = (BOOL) ( nIn >> 7 );
            bStartWithTemplate   = (BOOL) ( nIn >> 8 );
            bStartWithLayout     = (BOOL) ( nIn >> 9 );
            bMarkedHitMovesAlways= (BOOL) ( nIn >> 10);
            bMoveOnlyDragging    = (BOOL) ( nIn >> 11 );
            bNoText              = (BOOL) ( nIn >> 12 );
            bCrookNoContortion   = (BOOL) ( nIn >> 13 );

            if( nVersion >= VERSION_006 )
                bQuickEdit       = (BOOL) ( nIn >> 14 );

//          bSnapGrid            = (BOOL) ( nIn >> 14 );
            bSnapHelplines       = (BOOL) ( nIn >> 15 );
            bSnapBorder          = (BOOL) ( nIn >> 16 );
            bSnapFrame           = (BOOL) ( nIn >> 17 );
            bSnapPoints          = (BOOL) ( nIn >> 18 );
            bOrtho               = (BOOL) ( nIn >> 19 );
            bBigOrtho            = (BOOL) ( nIn >> 20 );
            bRotate              = (BOOL) ( nIn >> 21 );

            if( nVersion >= VERSION_003 )
            {
                bUseGridsnap = (BOOL) ( nIn >> 22 );
                bSynchronize = (BOOL) ( nIn >> 23 );
                bGridVisible = (BOOL) ( nIn >> 24 );
                bEqualGrid   = (BOOL) ( nIn >> 25 );

                if( nVersion >= VERSION_007 )
                    bMasterPageCache = (BOOL) ( nIn >> 26 );

                if( nVersion >= VERSION_010 )
                    bDragWithCopy = (BOOL) ( nIn >> 27 );

                if( nVersion >= VERSION_011 )
                    bPickThrough = (BOOL) ( nIn >> 28 );

                if( nVersion >= VERSION_015 )
                {
                    bSolidMarkHdl = (BOOL) ( nIn >> 29 );
                    bSolidDragging = (BOOL) ( nIn >> 30 );
                }

                if( nVersion >= VERSION_004 )
                {
                    rIn >> nIn;

                    bDraw               = (BOOL) ( nIn );
                    bNotes              = (BOOL) ( nIn >> 1 );
                    bHandout            = (BOOL) ( nIn >> 2 );
                    bOutline            = (BOOL) ( nIn >> 3 );
                    bDate               = (BOOL) ( nIn >> 4 );
                    bTime               = (BOOL) ( nIn >> 5 );
                    bPagename           = (BOOL) ( nIn >> 6 );
                    bHiddenPages        = (BOOL) ( nIn >> 7 );
                    bPagesize           = (BOOL) ( nIn >> 8 );
                    if( nVersion >= VERSION_008 )
                    {
                        bPagetile           = (BOOL) ( nIn >> 9 );
                        bWarningPrinter     = (BOOL) ( nIn >> 10 );
                        bWarningSize        = (BOOL) ( nIn >> 11 );
                        bWarningOrientation = (BOOL) ( nIn >> 12 );
                    }
                    else
                    {
                        bWarningPrinter     = (BOOL) ( nIn >> 9 );
                        bWarningSize        = (BOOL) ( nIn >> 10 );
                        bWarningOrientation = (BOOL) ( nIn >> 11 );
                    }
                    if( nVersion >= VERSION_009 )
                    {
                        bBooklet            = (BOOL) ( nIn >> 13);
                        bFront              = (BOOL) ( nIn >> 14);
                        bBack               = (BOOL) ( nIn >> 15);
                    }
                    if( nVersion >= VERSION_016 )
                        bPaperbin           = (BOOL) ( nIn >> 16);

                    if( nVersion >= VERSION_017 )
                    {
                        bBigHandles          = (BOOL) ( nIn >> 17);
                        bDoubleClickTextEdit = (BOOL) ( nIn >> 18);
                        bClickChangeRotation = (BOOL) ( nIn >> 19);
                    }
                    if( nVersion >= VERSION_021 )
                        bStartWithActualPage = (BOOL) ( nIn >> 20); /// NEU
                }

                rIn >> nFldDrawX;
                rIn >> nFldDivisionX;
                rIn >> nFldDrawY;
                rIn >> nFldDivisionY;
                rIn >> nFldSnapX;
                rIn >> nFldSnapY;
            }
        }
        rIn >> nSnapArea;
        rIn >> nAngle;
        if( nVersion >= VERSION_005 && nVersion < VERSION_013 )
        {
            USHORT nDummyDefTab;
            rIn >> nDummyDefTab;
        }
        if( nVersion >= VERSION_007 )
        {
            rIn >> nBezAngle;
        }
        if( nVersion == VERSION_012 ) // Ist ab 13 in OFA
        {
            BOOL bDummy;
            rIn >> bDummy;
            rIn >> bDummy;
        }
        if( nVersion >= VERSION_014 )
        {
            rIn >> nDefTab;
        }
        if( nVersion >= VERSION_018 )
        {
            // Massstab
            rIn >> nX;
            rIn >> nY;
        }
        if( nVersion >= VERSION_019 )
            rIn >> nMetric;

        // Ausgabequalitaet
        if( nVersion >= VERSION_020 )
            rIn >> nQuality;

        if( nVersion >= VERSION_022 )
            rIn >> nPreviewQuality;
    }
    else
    {
        return SfxConfigItem::WARNING_VERSION;
    }
    return SfxConfigItem::ERR_OK;;
}

#pragma optimize("",on)
// ----------------------------------------------------------------------

void __EXPORT SdOptions::UseDefault()
{
    //SetDefault( TRUE );
    Init();
    SfxConfigItem::UseDefault();
}

/*************************************************************************
|*
|* SdOptionsLayoutItem
|*
\************************************************************************/

SdOptionsLayoutItem::SdOptionsLayoutItem( USHORT nWhich ) :
                SfxPoolItem( nWhich )
{
}

// ----------------------------------------------------------------------

SdOptionsLayoutItem::SdOptionsLayoutItem( USHORT nWhich, SdOptions* pOpts,
                                      FrameView* pView ):
                SfxPoolItem( nWhich )
{
    if (pView)
    {
        bRuler               = pView->HasRuler();
        bMoveOutline         = !pView->IsNoDragXorPolys();
        bDragStripes         = pView->IsDragStripes();
        bHandlesBezier       = pView->IsPlusHandlesAlwaysVisible();
        bHelplines           = pView->IsHlplVisible();
    }
    else
    {
        bRuler               = pOpts->IsRulerVisible();
        bMoveOutline         = pOpts->IsMoveOutline();
        bDragStripes         = pOpts->IsDragStripes();
        bHandlesBezier       = pOpts->IsHandlesBezier();
        bHelplines           = pOpts->IsHelplines();
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* __EXPORT SdOptionsLayoutItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsLayoutItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SdOptionsLayoutItem& rItem = (SdOptionsLayoutItem&) rAttr;

    return( bRuler                == rItem.bRuler                &&
            bMoveOutline          == rItem.bMoveOutline          &&
            bDragStripes          == rItem.bDragStripes          &&
            bHandlesBezier        == rItem.bHandlesBezier        &&
            bHelplines            == rItem.bHelplines            );
}

// -----------------------------------------------------------------------

void SdOptionsLayoutItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetRulerVisible( bRuler == 1 );
    pOpts->SetMoveOutline( bMoveOutline == 1 );
    pOpts->SetDragStripes( bDragStripes == 1 );
    pOpts->SetHandlesBezier( bHandlesBezier == 1 );
    pOpts->SetHelplines( bHelplines == 1 );
}

/*************************************************************************
|*
|* SdOptionsContentsItem
|*
\************************************************************************/

SdOptionsContentsItem::SdOptionsContentsItem( USHORT nWhich ) :
                SfxPoolItem( nWhich )
{
}

// ----------------------------------------------------------------------

SdOptionsContentsItem::SdOptionsContentsItem( USHORT nWhich, SdOptions* pOpts,
                                      FrameView* pView ):
                SfxPoolItem( nWhich )
{
    if (pView)
    {
        bExternGraphic       = pView->IsGrafDraft();
        bOutlineMode         = pView->IsFillDraft();
        bHairlineMode        = pView->IsLineDraft();
        bNoText              = pView->IsTextDraft();
        bSolidDragging       = pView->IsSolidDragging();
        bSolidMarkHdl        = pView->IsSolidMarkHdl();
    }
    else
    {
        bExternGraphic       = pOpts->IsExternGraphic();
        bOutlineMode         = pOpts->IsOutlineMode();
        bHairlineMode        = pOpts->IsHairlineMode();
        bNoText              = pOpts->IsNoText();
        bSolidDragging       = pOpts->IsSolidDragging();
        bSolidMarkHdl        = pOpts->IsSolidMarkHdl();
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* __EXPORT SdOptionsContentsItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsContentsItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsContentsItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SdOptionsContentsItem& rItem = (SdOptionsContentsItem&) rAttr;

    return( bExternGraphic        == rItem.bExternGraphic        &&
            bOutlineMode          == rItem.bOutlineMode          &&
            bHairlineMode         == rItem.bHairlineMode         &&
            bNoText               == rItem.bNoText               &&
            bSolidDragging        == rItem.bSolidDragging        &&
            bSolidMarkHdl         == rItem.bSolidMarkHdl         );
}

// -----------------------------------------------------------------------

void SdOptionsContentsItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetExternGraphic( bExternGraphic == 1 );
    pOpts->SetOutlineMode( bOutlineMode == 1 );
    pOpts->SetHairlineMode( bHairlineMode == 1 );
    pOpts->SetNoText( bNoText == 1 );
    pOpts->SetSolidDragging( bSolidDragging == 1 );
    pOpts->SetSolidMarkHdl( bSolidMarkHdl == 1 );
}

/*************************************************************************
|*
|* SdOptionsMiscItem
|*
\************************************************************************/

SdOptionsMiscItem::SdOptionsMiscItem( USHORT nWhich ) :
                SfxPoolItem( nWhich )
{
}

// ----------------------------------------------------------------------

SdOptionsMiscItem::SdOptionsMiscItem( USHORT nWhich, SdOptions* pOpts,
                                      FrameView* pView ):
                SfxPoolItem( nWhich )
{
    bStartWithTemplate   = pOpts->IsStartWithTemplate();
    bStartWithLayout     = pOpts->IsStartWithLayout();
    bStartWithActualPage = pOpts->IsStartWithActualPage(); /// NEU

    if (pView)
    {
        bMarkedHitMovesAlways= pView->IsMarkedHitMovesAlways();
        bMoveOnlyDragging    = pView->IsMoveOnlyDragging();
        bCrookNoContortion   = pView->IsCrookNoContortion();
        bQuickEdit           = pView->IsQuickEdit();
        bMasterPageCache     = pView->IsMasterPagePaintCaching();
        bDragWithCopy        = pView->IsDragWithCopy();
        bPickThrough         = pView->GetModel()->IsPickThroughTransparentTextFrames();
        bBigHandles          = pView->IsBigHandles();
        bDoubleClickTextEdit = pView->IsDoubleClickTextEdit();
        bClickChangeRotation = pView->IsClickChangeRotation();
        nPreviewQuality      = pView->GetPreviewDrawMode();

    }
    else
    {
        bMarkedHitMovesAlways= pOpts->IsMarkedHitMovesAlways();
        bMoveOnlyDragging    = pOpts->IsMoveOnlyDragging();
        bCrookNoContortion   = pOpts->IsCrookNoContortion();
        bQuickEdit           = pOpts->IsQuickEdit();
        bMasterPageCache     = pOpts->IsMasterPagePaintCaching();
        bDragWithCopy        = pOpts->IsDragWithCopy();
        bPickThrough         = pOpts->IsPickThrough();
        bBigHandles          = pOpts->IsBigHandles();
        bDoubleClickTextEdit = pOpts->IsDoubleClickTextEdit();
        bClickChangeRotation = pOpts->IsClickChangeRotation();
        nPreviewQuality      = pOpts->GetPreviewQuality();
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* __EXPORT SdOptionsMiscItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsMiscItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsMiscItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SdOptionsMiscItem& rItem = (SdOptionsMiscItem&) rAttr;

    return( bStartWithTemplate    == rItem.bStartWithTemplate    &&
            bStartWithLayout      == rItem.bStartWithLayout      &&
            bMarkedHitMovesAlways == rItem.bMarkedHitMovesAlways &&
            bMoveOnlyDragging     == rItem.bMoveOnlyDragging     &&
            bCrookNoContortion    == rItem.bCrookNoContortion    &&
            bQuickEdit            == rItem.bQuickEdit            &&
            bDragWithCopy         == rItem.bDragWithCopy         &&
            bPickThrough          == rItem.bPickThrough          &&
            bMasterPageCache      == rItem.bMasterPageCache      &&
            bBigHandles           == rItem.bBigHandles           &&
            bDoubleClickTextEdit  == rItem.bDoubleClickTextEdit  &&
            bClickChangeRotation  == rItem.bClickChangeRotation  &&
            bStartWithActualPage  == rItem.bStartWithActualPage  &&
            nPreviewQuality       == rItem.nPreviewQuality );
}

// -----------------------------------------------------------------------

void SdOptionsMiscItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetStartWithTemplate( bStartWithTemplate == 1 );
    pOpts->SetStartWithLayout( bStartWithLayout == 1 );
    pOpts->SetMarkedHitMovesAlways( bMarkedHitMovesAlways == 1 );
    pOpts->SetMoveOnlyDragging( bMoveOnlyDragging == 1 );
    pOpts->SetCrookNoContortion( bCrookNoContortion == 1 );
    pOpts->SetQuickEdit( bQuickEdit == 1 );
    pOpts->SetMasterPagePaintCaching( bMasterPageCache == 1 );
    pOpts->SetDragWithCopy( bDragWithCopy == 1 );
    pOpts->SetPickThrough( bPickThrough == 1 );
    pOpts->SetBigHandles( bBigHandles == 1 );
    pOpts->SetDoubleClickTextEdit( bDoubleClickTextEdit == 1 );
    //pOpts->SetClickChangeRotation( bClickChangeRotation == 1 );
    pOpts->SetStartWithActualPage( bStartWithActualPage == 1 );
    pOpts->SetPreviewQuality( GetPreviewQuality() );
}

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/

SdOptionsSnapItem::SdOptionsSnapItem( USHORT nWhich ) :
                SfxPoolItem( nWhich )
{
}

// ----------------------------------------------------------------------

SdOptionsSnapItem::SdOptionsSnapItem( USHORT nWhich, SdOptions* pOpts,
                                      FrameView* pView ):
                SfxPoolItem( nWhich )
{
    if (pView)
    {
        bSnapHelplines       = pView->IsHlplSnap();
        bSnapBorder          = pView->IsBordSnap();
        bSnapFrame           = pView->IsOFrmSnap();
        bSnapPoints          = pView->IsOPntSnap();
        bOrtho               = pView->IsOrtho();
        bBigOrtho            = pView->IsBigOrtho();
        bRotate              = pView->IsAngleSnapEnabled();
        nSnapArea            = pView->GetSnapMagneticPixel();
        nAngle               = (INT16) pView->GetSnapAngle();
        nBezAngle            = (INT16) pView->GetEliminatePolyPointLimitAngle();
    }
    else
    {
        bSnapHelplines       = pOpts->IsSnapHelplines();
        bSnapBorder          = pOpts->IsSnapBorder();
        bSnapFrame           = pOpts->IsSnapFrame();
        bSnapPoints          = pOpts->IsSnapPoints();
        bOrtho               = pOpts->IsOrtho();
        bBigOrtho            = pOpts->IsBigOrtho();
        bRotate              = pOpts->IsRotate();
        nSnapArea            = pOpts->GetSnapArea();
        nAngle               = pOpts->GetAngle();
        nBezAngle            = pOpts->GetEliminatePolyPointLimitAngle();
    }

}

// ----------------------------------------------------------------------

SfxPoolItem* __EXPORT SdOptionsSnapItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsSnapItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsSnapItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SdOptionsSnapItem& rItem = (SdOptionsSnapItem&) rAttr;

    return( bSnapHelplines        == rItem.bSnapHelplines        &&
            bSnapBorder           == rItem.bSnapBorder           &&
            bSnapFrame            == rItem.bSnapFrame            &&
            bSnapPoints           == rItem.bSnapPoints           &&
            bOrtho                == rItem.bOrtho                &&
            bBigOrtho             == rItem.bBigOrtho             &&
            bRotate               == rItem.bRotate               &&
            nSnapArea             == rItem.nSnapArea             &&
            nAngle                == rItem.nAngle                &&
            nBezAngle             == rItem.nBezAngle            );
}

// -----------------------------------------------------------------------

void SdOptionsSnapItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetSnapHelplines( bSnapHelplines == 1 );
    pOpts->SetSnapBorder( bSnapBorder == 1 );
    pOpts->SetSnapFrame( bSnapFrame == 1 );
    pOpts->SetSnapPoints( bSnapPoints == 1 );
    pOpts->SetOrtho( bOrtho == 1 );
    pOpts->SetBigOrtho( bBigOrtho == 1 );
    pOpts->SetRotate( bRotate == 1 );
    pOpts->SetSnapArea( nSnapArea );
    pOpts->SetAngle( nAngle );
    pOpts->SetEliminatePolyPointLimitAngle( nBezAngle);

}

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/

SdOptionsPrintItem::SdOptionsPrintItem( USHORT nWhich ) :
                SfxPoolItem( nWhich )
{
}

// ----------------------------------------------------------------------

SdOptionsPrintItem::SdOptionsPrintItem( USHORT nWhich, SdOptions* pOpts ):
                SfxPoolItem( nWhich )
{
    bDraw               = pOpts->IsDraw();
    bNotes              = pOpts->IsNotes();
    bHandout            = pOpts->IsHandout();
    bOutline            = pOpts->IsOutline();
    bDate               = pOpts->IsDate();
    bTime               = pOpts->IsTime();
    bPagename           = pOpts->IsPagename();
    bHiddenPages        = pOpts->IsHiddenPages();
    bPagesize           = pOpts->IsPagesize();
    bPagetile           = pOpts->IsPagetile();
    bWarningPrinter     = pOpts->IsWarningPrinter();
    bWarningSize        = pOpts->IsWarningSize();
    bWarningOrientation = pOpts->IsWarningOrientation();
    bBooklet            = pOpts->IsBooklet();
    bFront              = pOpts->IsFrontPage();
    bBack               = pOpts->IsBackPage();
    bPaperbin           = pOpts->IsPaperbin();
/// Neu
    nQuality            = pOpts->GetOutputQuality();
/// Neu
}

// ----------------------------------------------------------------------

SfxPoolItem* __EXPORT SdOptionsPrintItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsPrintItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsPrintItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SdOptionsPrintItem& rItem = (SdOptionsPrintItem&) rAttr;

    return( bDraw               == rItem.bDraw               &&
            bNotes              == rItem.bNotes              &&
            bHandout            == rItem.bHandout            &&
            bOutline            == rItem.bOutline            &&
            bDate               == rItem.bDate               &&
            bTime               == rItem.bTime               &&
            bPagename           == rItem.bPagename           &&
            bHiddenPages        == rItem.bHiddenPages        &&
            bPagesize           == rItem.bPagesize           &&
            bPagetile           == rItem.bPagetile           &&
            bWarningPrinter     == rItem.bWarningPrinter     &&
            bWarningSize        == rItem.bWarningSize        &&
            bWarningOrientation == rItem.bWarningOrientation &&
            bBooklet            == rItem.bBooklet            &&
            bFront              == rItem.bFront              &&
            bBack               == rItem.bBack               &&
            bPaperbin           == rItem.bPaperbin           &&
/// Neu
            nQuality            == rItem.nQuality           );
/// Neu
}

// -----------------------------------------------------------------------

void SdOptionsPrintItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetDraw( bDraw == 1 );
    pOpts->SetNotes( bNotes == 1 );
    pOpts->SetHandout( bHandout == 1 );
    pOpts->SetOutline( bOutline == 1 );
    pOpts->SetDate( bDate == 1 );
    pOpts->SetTime( bTime == 1 );
    pOpts->SetPagename( bPagename == 1 );
    pOpts->SetHiddenPages( bHiddenPages == 1 );
    pOpts->SetPagesize( bPagesize == 1 );
    pOpts->SetPagetile( bPagetile == 1 );
    pOpts->SetWarningPrinter( bWarningPrinter == 1 );
    pOpts->SetWarningSize( bWarningSize == 1 );
    pOpts->SetWarningOrientation( bWarningOrientation == 1 );
    pOpts->SetBooklet( bBooklet == 1 );
    pOpts->SetFrontPage( bFront == 1 );
    pOpts->SetBackPage( bBack == 1 );
    pOpts->SetPaperbin( bPaperbin == 1 );
/// Neu
    pOpts->SetOutputQuality( nQuality );
/// Neu
}

/*************************************************************************
|*
|* SdOptionsGridItem
|*
\************************************************************************/

SdOptionsGridItem::SdOptionsGridItem( USHORT nWhich, SdOptions* pOpts,
                                      FrameView* pView ):
                SvxGridItem( nWhich )
{
    bSynchronize  = pOpts->GetSynchronize();
    bEqualGrid    = pOpts->GetEqualGrid();

    if( pView )
    {
        nFldDrawX     = pView->GetGridCoarse().Width();
        nFldDrawY     = pView->GetGridCoarse().Height();

        UINT32 nFineX = pView->GetGridFine().Width();
        UINT32 nFineY = pView->GetGridFine().Height();
        nFldDivisionX = ( nFineX ? (nFldDrawX / nFineX - 1) : 0 );
        nFldDivisionY = ( nFineY ? (nFldDrawY / nFineY - 1) : 0 );

        nFldSnapX     = pView->GetSnapGrid().Width();
        nFldSnapY     = pView->GetSnapGrid().Height();
        bUseGridsnap  = pView->IsGridSnap();
        bGridVisible  = pView->IsGridVisible();
    }
    else
    {
        nFldDrawX     = pOpts->GetFldDrawX();
        nFldDrawY     = pOpts->GetFldDrawY();

        UINT32 nFineX = pOpts->GetFldDivisionX();
        UINT32 nFineY = pOpts->GetFldDivisionY();
        nFldDivisionX = ( nFineX ? (nFldDrawX / nFineX - 1) : 0 );
        nFldDivisionY = ( nFineY ? (nFldDrawY / nFineY - 1) : 0 );

        nFldSnapX     = pOpts->GetFldSnapX();
        nFldSnapY     = pOpts->GetFldSnapY();
        bUseGridsnap  = pOpts->GetUseGridSnap();
        bGridVisible  = pOpts->GetGridVisible();
    }
}

// -----------------------------------------------------------------------
void SdOptionsGridItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetFldDrawX    ( nFldDrawX     );
    pOpts->SetFldDivisionX( nFldDrawX ? (nFldDrawX / ( nFldDivisionX + 1 ) ) : 0 );
    pOpts->SetFldDrawY    ( nFldDrawY     );
    pOpts->SetFldDivisionY( nFldDrawY ? (nFldDrawY / ( nFldDivisionY + 1 ) ) : 0 );
    pOpts->SetFldSnapX    ( nFldSnapX     );
    pOpts->SetFldSnapY    ( nFldSnapY     );
    pOpts->SetUseGridSnap ( bUseGridsnap == 1 );
    pOpts->SetSynchronize ( bSynchronize == 1 );
    pOpts->SetGridVisible ( bGridVisible == 1 );
    pOpts->SetEqualGrid   ( bEqualGrid == 1 );
}




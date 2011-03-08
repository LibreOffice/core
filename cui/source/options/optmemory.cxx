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

#include <svtools/langtab.hxx>
#include <svl/zforlist.hxx>
#include <svtools/grfmgr.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/lingucfg.hxx>
#include <svl/szitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/math.hxx>
#include <unotools/undoopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/cacheoptions.hxx>
#include <unotools/fontoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <unotools/startoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/configitem.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/types.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include "cuioptgenrl.hxx"
#include "optpath.hxx"
#include "optsave.hxx"
#include "optlingu.hxx"
#include <svx/xpool.hxx>
#include <svx/dlgutil.hxx>
#include "cuitabarea.hxx"
#include "optmemory.hrc"
#include "optmemory.hxx"
#include <svx/ofaitem.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

#define CONFIG_LANGUAGES "OfficeLanguages"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::utl;
using namespace ::rtl;
using namespace ::sfx2;


#define NF2BYTES        104857.6                        // 2^20/10, used for aNfGraphicObjectCache-unit -> Byte
#define BYTES2NF        (1.0/NF2BYTES)                  // 10/2^20


inline long OfaMemoryOptionsPage::GetNfGraphicCacheVal( void ) const
{
    return static_cast<long>(aNfGraphicCache.GetValue() << 20);
}

inline void OfaMemoryOptionsPage::SetNfGraphicCacheVal( long nSizeInBytes )
{
    aNfGraphicCache.SetValue( nSizeInBytes >> 20 );
}

long OfaMemoryOptionsPage::GetNfGraphicObjectCacheVal( void ) const
{
    return long( ::rtl::math::round( double( aNfGraphicObjectCache.GetValue() ) * NF2BYTES ) );
}

void OfaMemoryOptionsPage::SetNfGraphicObjectCacheVal( long nSizeInBytes )
{
    aNfGraphicObjectCache.SetValue( long( ::rtl::math::round( double( nSizeInBytes ) * BYTES2NF ) ) );
}

inline void OfaMemoryOptionsPage::SetNfGraphicObjectCacheMax( long nSizeInBytes )
{
    aNfGraphicObjectCache.SetMax( long( double( nSizeInBytes ) * BYTES2NF ) );
}

inline void OfaMemoryOptionsPage::SetNfGraphicObjectCacheLast( long nSizeInBytes )
{
    aNfGraphicObjectCache.SetLast( long( double( nSizeInBytes ) * BYTES2NF ) );
}

int OfaMemoryOptionsPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

OfaMemoryOptionsPage::OfaMemoryOptionsPage(Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, CUI_RES( OFA_TP_MEMORY ), rSet ),

    aUndoBox                ( this, CUI_RES( GB_UNDO ) ),
    aUndoText               ( this, CUI_RES( FT_UNDO ) ),
    aUndoEdit               ( this, CUI_RES( ED_UNDO ) ),
    aGbGraphicCache         ( this, CUI_RES( GB_GRAPHICCACHE ) ),
    aFtGraphicCache         ( this, CUI_RES( FT_GRAPHICCACHE ) ),
    aNfGraphicCache         ( this, CUI_RES( NF_GRAPHICCACHE ) ),
    aFtGraphicCacheUnit     ( this, CUI_RES( FT_GRAPHICCACHE_UNIT         ) ),
    aFtGraphicObjectCache   ( this, CUI_RES( FT_GRAPHICOBJECTCACHE ) ),
    aNfGraphicObjectCache   ( this, CUI_RES( NF_GRAPHICOBJECTCACHE ) ),
    aFtGraphicObjectCacheUnit(this, CUI_RES( FT_GRAPHICOBJECTCACHE_UNIT ) ),
    aFtGraphicObjectTime    ( this, CUI_RES( FT_GRAPHICOBJECTTIME ) ),
    aTfGraphicObjectTime    ( this, CUI_RES( TF_GRAPHICOBJECTTIME ) ),
    aFtGraphicObjectTimeUnit( this, CUI_RES( FT_GRAPHICOBJECTTIME_UNIT     ) ),

    aGbOLECache             ( this, CUI_RES( GB_OLECACHE ) ),
    aFtOLECache             ( this, CUI_RES( FT_OLECACHE ) ),
    aNfOLECache             ( this, CUI_RES( NF_OLECACHE ) ),
    aQuickLaunchFL          ( this, CUI_RES( FL_QUICKLAUNCH ) ),
    aQuickLaunchCB          ( this, CUI_RES( CB_QUICKLAUNCH ) )//,
{
#if defined(UNX)
    aQuickLaunchCB.SetText( CUI_RES( STR_QUICKLAUNCH_UNX ) );
#endif
    FreeResource();

    //quick launch only available in Win and OS/2
#if !defined(WNT) && !defined(OS2) && !defined(ENABLE_GTK)
    aQuickLaunchFL.Hide();
    aQuickLaunchCB.Hide();
#endif

    aTfGraphicObjectTime.SetExtFormat( EXTTIMEF_24H_SHORT );

    SetExchangeSupport();

    aNfGraphicCache.SetModifyHdl( LINK( this, OfaMemoryOptionsPage, GraphicCacheConfigHdl ) );
}

// -----------------------------------------------------------------------

OfaMemoryOptionsPage::~OfaMemoryOptionsPage()
{
}

// -----------------------------------------------------------------------

SfxTabPage* OfaMemoryOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaMemoryOptionsPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------

BOOL OfaMemoryOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;

    SvtCacheOptions aCacheOptions;

    // Undo-Schritte
    if ( aUndoEdit.GetText() != aUndoEdit.GetSavedValue() )
        SvtUndoOptions().SetUndoCount((UINT16)aUndoEdit.GetValue());

    // GraphicCache
    aCacheOptions.SetGraphicManagerTotalCacheSize( GetNfGraphicCacheVal() );
    aCacheOptions.SetGraphicManagerObjectCacheSize( GetNfGraphicObjectCacheVal() );

       const Time aTime( aTfGraphicObjectTime.GetTime() );
    aCacheOptions.SetGraphicManagerObjectReleaseTime( aTime.GetSec() + aTime.GetMin() * 60 + aTime.GetHour() * 3600 );

    // create a dummy graphic object to get access to the common GraphicManager
    GraphicObject       aDummyObject;
    GraphicManager&     rGrfMgr = aDummyObject.GetGraphicManager();

    rGrfMgr.SetMaxCacheSize( aCacheOptions.GetGraphicManagerTotalCacheSize() );
    rGrfMgr.SetMaxObjCacheSize( aCacheOptions.GetGraphicManagerObjectCacheSize(), TRUE );
    rGrfMgr.SetCacheTimeout( aCacheOptions.GetGraphicManagerObjectReleaseTime() );

    // OLECache
    aCacheOptions.SetWriterOLE_Objects( static_cast<long>(aNfOLECache.GetValue()) );
    aCacheOptions.SetDrawingEngineOLE_Objects( static_cast<long>(aNfOLECache.GetValue()) );

    if( aQuickLaunchCB.IsChecked() != aQuickLaunchCB.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_QUICKLAUNCHER, aQuickLaunchCB.IsChecked()));
        bModified = TRUE;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void OfaMemoryOptionsPage::Reset( const SfxItemSet& rSet )
{
    SvtCacheOptions     aCacheOptions;
    const SfxPoolItem*  pItem;

    // Undo-Schritte
    aUndoEdit.SetValue( SvtUndoOptions().GetUndoCount() );
    aUndoEdit.SaveValue();

    // GraphicCache
    long    n = aCacheOptions.GetGraphicManagerTotalCacheSize();
    SetNfGraphicCacheVal( n );
    SetNfGraphicObjectCacheVal( Min( static_cast<sal_Int32>(GetNfGraphicCacheVal()), aCacheOptions.GetGraphicManagerObjectCacheSize() ) );

    sal_Int32 nTime = aCacheOptions.GetGraphicManagerObjectReleaseTime();
    Time aTime( (USHORT)( nTime / 3600 ), (USHORT)( ( nTime % 3600 ) / 60 ), (USHORT)( ( nTime % 3600 ) % 60 ) );
    aTfGraphicObjectTime.SetTime( aTime );

    GraphicCacheConfigHdl( &aNfGraphicCache );

    // OLECache
    aNfOLECache.SetValue( Max( aCacheOptions.GetWriterOLE_Objects(), aCacheOptions.GetDrawingEngineOLE_Objects() ) );

    SfxItemState eState = rSet.GetItemState( SID_ATTR_QUICKLAUNCHER, FALSE, &pItem );
    if ( SFX_ITEM_SET == eState )
        aQuickLaunchCB.Check( ( (SfxBoolItem*)pItem )->GetValue() );
    else if ( SFX_ITEM_DISABLED == eState )
    {
        // quickstart not installed
        aQuickLaunchFL.Hide();
        aQuickLaunchCB.Hide();
    }

    aQuickLaunchCB.SaveValue();
}

// -----------------------------------------------------------------------

IMPL_LINK( OfaMemoryOptionsPage, GraphicCacheConfigHdl, NumericField*, EMPTYARG )
{
    long    n = GetNfGraphicCacheVal();
    SetNfGraphicObjectCacheMax( n );
    SetNfGraphicObjectCacheLast( n );

    if( GetNfGraphicObjectCacheVal() > n )
        SetNfGraphicObjectCacheVal( n );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

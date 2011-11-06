/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"
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
#include <cuires.hrc>
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

sal_Bool OfaMemoryOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    SvtCacheOptions aCacheOptions;

    // Undo-Schritte
    if ( aUndoEdit.GetText() != aUndoEdit.GetSavedValue() )
        SvtUndoOptions().SetUndoCount((sal_uInt16)aUndoEdit.GetValue());

    // GraphicCache
    aCacheOptions.SetGraphicManagerTotalCacheSize( GetNfGraphicCacheVal() );
    aCacheOptions.SetGraphicManagerObjectCacheSize( GetNfGraphicObjectCacheVal() );

       const Time aTime( aTfGraphicObjectTime.GetTime() );
    aCacheOptions.SetGraphicManagerObjectReleaseTime( aTime.GetSec() + aTime.GetMin() * 60 + aTime.GetHour() * 3600 );

    // create a dummy graphic object to get access to the common GraphicManager
    GraphicObject       aDummyObject;
    GraphicManager&     rGrfMgr = aDummyObject.GetGraphicManager();

    rGrfMgr.SetMaxCacheSize( aCacheOptions.GetGraphicManagerTotalCacheSize() );
    rGrfMgr.SetMaxObjCacheSize( aCacheOptions.GetGraphicManagerObjectCacheSize(), sal_True );
    rGrfMgr.SetCacheTimeout( aCacheOptions.GetGraphicManagerObjectReleaseTime() );

    // OLECache
    aCacheOptions.SetWriterOLE_Objects( static_cast<long>(aNfOLECache.GetValue()) );
    aCacheOptions.SetDrawingEngineOLE_Objects( static_cast<long>(aNfOLECache.GetValue()) );

    if( aQuickLaunchCB.IsChecked() != aQuickLaunchCB.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_QUICKLAUNCHER, aQuickLaunchCB.IsChecked()));
        bModified = sal_True;
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
    Time aTime( (sal_uInt16)( nTime / 3600 ), (sal_uInt16)( ( nTime % 3600 ) / 60 ), (sal_uInt16)( ( nTime % 3600 ) % 60 ) );
    aTfGraphicObjectTime.SetTime( aTime );

    GraphicCacheConfigHdl( &aNfGraphicCache );

    // OLECache
    aNfOLECache.SetValue( Max( aCacheOptions.GetWriterOLE_Objects(), aCacheOptions.GetDrawingEngineOLE_Objects() ) );

    SfxItemState eState = rSet.GetItemState( SID_ATTR_QUICKLAUNCHER, sal_False, &pItem );
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


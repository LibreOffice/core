/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optmemory.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:42:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <svtools/langtab.hxx>

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SFXSZITEM_HXX
#include <svtools/szitem.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_UNDOOPT_HXX
#include <svtools/undoopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_CACHEOPTIONS_HXX
#include <svtools/cacheoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS3D_HXX
#include <svtools/options3d.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_FONTOPTIONS_HXX
#include <svtools/fontoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCPOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#include <svtools/syslocaleoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_CUIOPTGENRL_HXX
#include "cuioptgenrl.hxx"
#endif
#ifndef _SVX_OPTPATH_HXX
#include "optpath.hxx"
#endif
#ifndef _SVX_OPTSAVE_HXX
#include "optsave.hxx"
#endif
#ifndef _SVX_OPTLINGU_HXX
#include <svx/optlingu.hxx>
#endif
#ifndef _XPOOL_HXX
#include <svx/xpool.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include "dlgutil.hxx"
#endif
#ifndef _SVX_TAB_AREA_HXX
#include "cuitabarea.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif


#include "optmemory.hrc"
#include "optmemory.hxx"
#include "ofaitem.hxx"
#include <svx/dialogs.hrc>
#include "helpid.hrc"
#include <svx/dialmgr.hxx>

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

    SfxTabPage( pParent, SVX_RES( OFA_TP_MEMORY ), rSet ),

    aUndoBox                ( this, SVX_RES( GB_UNDO ) ),
    aUndoText               ( this, SVX_RES( FT_UNDO ) ),
    aUndoEdit               ( this, SVX_RES( ED_UNDO ) ),
    aGbGraphicCache         ( this, SVX_RES( GB_GRAPHICCACHE ) ),
    aFtGraphicCache         ( this, SVX_RES( FT_GRAPHICCACHE ) ),
    aNfGraphicCache         ( this, SVX_RES( NF_GRAPHICCACHE ) ),
    aFtGraphicCacheUnit     ( this, SVX_RES( FT_GRAPHICCACHE_UNIT         ) ),
    aFtGraphicObjectCache   ( this, SVX_RES( FT_GRAPHICOBJECTCACHE ) ),
    aNfGraphicObjectCache   ( this, SVX_RES( NF_GRAPHICOBJECTCACHE ) ),
    aFtGraphicObjectCacheUnit(this, SVX_RES( FT_GRAPHICOBJECTCACHE_UNIT ) ),
    aFtGraphicObjectTime    ( this, SVX_RES( FT_GRAPHICOBJECTTIME ) ),
    aTfGraphicObjectTime    ( this, SVX_RES( TF_GRAPHICOBJECTTIME ) ),
    aFtGraphicObjectTimeUnit( this, SVX_RES( FT_GRAPHICOBJECTTIME_UNIT     ) ),

    aGbOLECache             ( this, SVX_RES( GB_OLECACHE ) ),
    aFtOLECache             ( this, SVX_RES( FT_OLECACHE ) ),
    aNfOLECache             ( this, SVX_RES( NF_OLECACHE ) ),
    aQuickLaunchFL          ( this, SVX_RES( FL_QUICKLAUNCH ) ),
    aQuickLaunchCB          ( this, SVX_RES( CB_QUICKLAUNCH ) )//,
{
#if defined(UNX)
    aQuickLaunchCB.SetText( SVX_RES( STR_QUICKLAUNCH_UNX ) );
#endif
    FreeResource();

#if !defined(WNT) && !defined(ENABLE_GTK)
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
        // quickstart not installed
        aQuickLaunchCB.Disable();

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


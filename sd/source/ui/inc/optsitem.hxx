/*************************************************************************
 *
 *  $RCSfile: optsitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-29 15:39:15 $
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

#ifndef _SD_OPTSITEM_HXX
#define _SD_OPTSITEM_HXX

#ifndef _UTL_CONFIGITEM_HXX_ //autogen
#include <unotools/configitem.hxx>
#endif
#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_OPTGRID_HXX //autogen
#include <svx/optgrid.hxx>
#endif

// -----------------
// - Option ranges -
// -----------------

#define SD_OPTIONS_NONE     0x00000000
#define SD_OPTIONS_ALL      0xffffffff

#define SD_OPTIONS_LAYOUT   0x00000001
#define SD_OPTIONS_CONTENTS 0x00000002
#define SD_OPTIONS_MISC     0x00000004
#define SD_OPTIONS_SNAP     0x00000008
#define SD_OPTIONS_ZOOM     0x00000010
#define SD_OPTIONS_GRID     0x00000020
#define SD_OPTIONS_PRINT    0x00000040

// ------------
// - Forwards -
// ------------

class SfxConfigItem;
class FrameView;
class SdOptions;

// -----------------
// - SdOptionsItem -
// -----------------

class SdOptionsGeneric;

class SdOptionsItem : public ::utl::ConfigItem
{

private:

    const SdOptionsGeneric& mrParent;


public:

                            SdOptionsItem( const SdOptionsGeneric& rParent, const ::rtl::OUString rSubTree );
                            ~SdOptionsItem();

    virtual void            Commit();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetProperties(
                                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames );
    sal_Bool                PutProperties( const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                           const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues );
    void                    SetModified();
};

// --------------------
// - SdOptionsGeneric -
// --------------------

class SdOptionsGeneric
{
friend class SdOptionsItem;

private:

    ::rtl::OUString         maSubTree;
    SdOptionsItem*          mpCfgItem;
    USHORT                  mnConfigId;
    BOOL                    mbInit          : 1;
    BOOL                    mbEnableModify  : 1;

    void                    Commit( SdOptionsItem& rCfgItem ) const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPropertyNames() const;

protected:

    void                    Init() const;
    void                    OptionsChanged() { if( mpCfgItem && mbEnableModify ) mpCfgItem->SetModified(); }

protected:

    virtual void            GetPropNameArray( const char**& ppNames, ULONG& rCount ) const = 0;
    virtual BOOL            ReadData( const ::com::sun::star::uno::Any* pValues ) = 0;
    virtual BOOL            WriteData( ::com::sun::star::uno::Any* pValues ) const = 0;

public:

                            SdOptionsGeneric( USHORT nConfigId, const ::rtl::OUString& rSubTree );
                            ~SdOptionsGeneric();

    const ::rtl::OUString&  GetSubTree() const { return maSubTree; }
    USHORT                  GetConfigId() const { return mnConfigId; }

    void                    EnableModify( BOOL bModify ) { mbEnableModify = bModify; }
};

// -------------------
// - SdOptionsLayout -
// -------------------

class SdOptionsLayout : public SdOptionsGeneric
{
private:

    BOOL    bRuler              : 1;    // Layout/Display/Ruler
    BOOL    bMoveOutline        : 1;    // Layout/Display/Contur
    BOOL    bDragStripes        : 1;    // Layout/Display/Guide
    BOOL    bHandlesBezier      : 1;    // Layout/Display/Bezier
    BOOL    bHelplines          : 1;    // Layout/Display/Helpline
    UINT16  nMetric;                    // Layout/Other/MeasureUnit
    UINT16  nDefTab;                    // Layout/Other/TabStop

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:
            SdOptionsLayout( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsLayout() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsLayout& rOpt ) const;

    BOOL    IsRulerVisible() const { Init(); return (BOOL) bRuler; }
    BOOL    IsMoveOutline() const { Init(); return (BOOL) bMoveOutline; }
    BOOL    IsDragStripes() const { Init(); return (BOOL) bDragStripes; }
    BOOL    IsHandlesBezier() const { Init(); return (BOOL) bHandlesBezier; }
    BOOL    IsHelplines() const { Init(); return (BOOL) bHelplines; }
    UINT16  GetMetric() const { Init(); return( ( 0xffff == nMetric ) ? SFX_APP()->GetOptions().GetMetric() : nMetric ); }
    UINT16  GetDefTab() const { Init(); return nDefTab; }

    void    SetRulerVisible( BOOL bOn = TRUE ) { if( bRuler != bOn ) { OptionsChanged(); bRuler = bOn; } }
    void    SetMoveOutline( BOOL bOn = TRUE ) { if( bMoveOutline != bOn ) { OptionsChanged(); bMoveOutline = bOn; } }
    void    SetDragStripes( BOOL bOn = TRUE ) { if( bDragStripes != bOn ) { OptionsChanged(); bDragStripes = bOn; } }
    void    SetHandlesBezier( BOOL bOn = TRUE ) { if( bHandlesBezier != bOn ) { OptionsChanged(); bHandlesBezier = bOn; } }
    void    SetHelplines( BOOL bOn = TRUE ) { if( bHelplines != bOn ) { OptionsChanged(); bHelplines = bOn; } }
    void    SetMetric( UINT16 nInMetric ) { if( nMetric != nInMetric ) { OptionsChanged(); nMetric = nInMetric; } }
    void    SetDefTab( UINT16 nTab ) { if( nDefTab != nTab ) { OptionsChanged(); nDefTab = nTab; } }
};

// -----------------------------------------------------------------------------

class SdOptionsLayoutItem : public SfxPoolItem, public SdOptionsLayout
{
public:

                            SdOptionsLayoutItem( USHORT nWhich);
                            SdOptionsLayoutItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;
};

// ---------------------
// - SdOptionsContents -
// ---------------------

class SdOptionsContents : public SdOptionsGeneric
{
private:

    BOOL    bExternGraphic  : 1; // Content/Display/PicturePlaceholder
    BOOL    bOutlineMode    : 1; // Content/Display/ContourMode
    BOOL    bHairlineMode   : 1; // Content/Display/LineContour
    BOOL    bNoText         : 1; // Content/Display/TextPlaceholder
    BOOL    bSolidDragging  : 1; // ???
    BOOL    bSolidMarkHdl   : 1; // ???

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsContents( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsContents() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsContents& rOpt ) const;

    BOOL    IsExternGraphic() const { Init(); return (BOOL) bExternGraphic; }
    BOOL    IsOutlineMode() const { Init(); return (BOOL) bOutlineMode; }
    BOOL    IsHairlineMode() const { Init(); return (BOOL) bHairlineMode; }
    BOOL    IsNoText() const { Init(); return (BOOL) bNoText; }
    BOOL    IsSolidDragging() const { Init(); return (BOOL) bSolidDragging; }
    BOOL    IsSolidMarkHdl() const { Init(); return (BOOL) bSolidMarkHdl; }

    void    SetExternGraphic( BOOL bOn = TRUE ) { if( bExternGraphic != bOn ) { OptionsChanged(); bExternGraphic = bOn; } }
    void    SetOutlineMode( BOOL bOn = TRUE ) { if( bOutlineMode != bOn ) { OptionsChanged(); bOutlineMode = bOn; } }
    void    SetHairlineMode( BOOL bOn = TRUE ) { if( bHairlineMode != bOn ) { OptionsChanged(); bHairlineMode = bOn; } }
    void    SetNoText( BOOL bOn = TRUE ) { if( bNoText != bOn ) { OptionsChanged(); bNoText = bOn; } }
    void    SetSolidDragging( BOOL bOn = TRUE ) { if( bSolidDragging != bOn ) { OptionsChanged(); bSolidDragging = bOn; } }
    void    SetSolidMarkHdl( BOOL bOn = TRUE ) { if( bSolidMarkHdl != bOn ) { OptionsChanged(); bSolidMarkHdl = bOn; } }
};

// -----------------------------------------------------------------------------

class SdOptionsContentsItem : public SfxPoolItem, public SdOptionsContents
{
public:

                            SdOptionsContentsItem( USHORT nWhich);
                            SdOptionsContentsItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

};

// -----------------
// - SdOptionsMisc -
// -----------------

class SdOptionsMisc : public SdOptionsGeneric
{
private:

                                            // missing: // Misc/TextObject/Selectable; /Misc/ObjectMoveable; /Misc/SimpleHandles

    BOOL    bStartWithTemplate      : 1;    // Misc/NewDoc/AutoPilot
    BOOL    bMarkedHitMovesAlways   : 1;
    BOOL    bMoveOnlyDragging       : 1;
    BOOL    bCrookNoContortion      : 1;    // Misc/NoDistort
    BOOL    bQuickEdit              : 1;    // Misc/TextObject/QuickEditing
    BOOL    bMasterPageCache        : 1;    // Misc/BackgroundCache
    BOOL    bDragWithCopy           : 1;    // Misc/CopyWhileMoving
    BOOL    bPickThrough            : 1;
    BOOL    bBigHandles             : 1;    // Misc/BigHandles
    BOOL    bDoubleClickTextEdit    : 1;    // Misc/DclickTextedit
    BOOL    bClickChangeRotation    : 1;    // Misc/RotateClick
    BOOL    bStartWithActualPage    : 1;    // Misc/Start/CurrentPage
    ULONG   nPreviewQuality;                // !!!Misc/Preview (double=>integer)!!!

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsMisc( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsMisc() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsMisc& rOpt ) const;

    BOOL    IsStartWithTemplate() const { Init(); return (BOOL) bStartWithTemplate; }
    BOOL    IsMarkedHitMovesAlways() const { Init(); return (BOOL) bMarkedHitMovesAlways; }
    BOOL    IsMoveOnlyDragging() const { Init(); return (BOOL) bMoveOnlyDragging; }
    BOOL    IsCrookNoContortion() const { Init(); return (BOOL) bCrookNoContortion; }
    BOOL    IsQuickEdit() const { Init(); return (BOOL) bQuickEdit; }
    BOOL    IsMasterPagePaintCaching() const { Init(); return (BOOL) bMasterPageCache; }
    BOOL    IsDragWithCopy() const { Init(); return (BOOL) bDragWithCopy; }
    BOOL    IsPickThrough() const { Init(); return (BOOL) bPickThrough; }
    BOOL    IsBigHandles() const { Init(); return (BOOL) bBigHandles; }
    BOOL    IsDoubleClickTextEdit() const { Init(); return (BOOL) bDoubleClickTextEdit; }
    BOOL    IsClickChangeRotation() const { Init(); return (BOOL) bClickChangeRotation; }
    BOOL    IsStartWithActualPage() const { Init(); return (BOOL) bStartWithActualPage; }
    ULONG   GetPreviewQuality() const { Init(); return nPreviewQuality; }

    void    SetStartWithTemplate( BOOL bOn = TRUE ) { if( bStartWithTemplate != bOn ) { OptionsChanged(); bStartWithTemplate = bOn; } }
    void    SetMarkedHitMovesAlways( BOOL bOn = TRUE ) { if( bMarkedHitMovesAlways != bOn ) { OptionsChanged(); bMarkedHitMovesAlways = bOn; } }
    void    SetMoveOnlyDragging( BOOL bOn = TRUE ) { if( bMoveOnlyDragging != bOn ) { OptionsChanged(); bMoveOnlyDragging = bOn; } }
    void    SetCrookNoContortion( BOOL bOn = TRUE ) { if( bCrookNoContortion != bOn ) { OptionsChanged(); bCrookNoContortion = bOn; } }
    void    SetQuickEdit( BOOL bOn = TRUE ) { if( bQuickEdit != bOn ) { OptionsChanged(); bQuickEdit = bOn; } }
    void    SetMasterPagePaintCaching( BOOL bOn = TRUE ) { if( bMasterPageCache != bOn ) { OptionsChanged(); bMasterPageCache = bOn; } }
    void    SetDragWithCopy( BOOL bOn = TRUE ) { if( bDragWithCopy != bOn ) { OptionsChanged(); bDragWithCopy = bOn; } }
    void    SetPickThrough( BOOL bOn = TRUE ) { if( bPickThrough != bOn ) { OptionsChanged(); bPickThrough = bOn; } }
    void    SetBigHandles( BOOL bOn = TRUE ) { if( bBigHandles != bOn ) { OptionsChanged(); bBigHandles = bOn; } }
    void    SetDoubleClickTextEdit( BOOL bOn = TRUE ) { if( bDoubleClickTextEdit != bOn ) { OptionsChanged(); bDoubleClickTextEdit = bOn; } }
    void    SetClickChangeRotation( BOOL bOn = TRUE ) { if( bClickChangeRotation != bOn ) { OptionsChanged(); bClickChangeRotation = bOn; } }
    void    SetStartWithActualPage( BOOL bOn = TRUE ) { if( bStartWithActualPage != bOn ) { OptionsChanged(); bStartWithActualPage = bOn; } }
    void    SetPreviewQuality( ULONG nQual ) { if( nPreviewQuality != nQual ) { OptionsChanged(); nPreviewQuality = nQual; } }
};

// -----------------------------------------------------------------------------

class SdOptionsMiscItem : public SfxPoolItem, public SdOptionsMisc
{
public:

                            SdOptionsMiscItem( USHORT nWhich);
                            SdOptionsMiscItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

};

// -----------------
// - SdOptionsSnap -
// -----------------

class SdOptionsSnap : public SdOptionsGeneric
{
private:

    BOOL    bSnapHelplines  : 1;    // Snap/Object/SnapLine
    BOOL    bSnapBorder     : 1;    // Snap/Object/PageMargin
    BOOL    bSnapFrame      : 1;    // Snap/Object/ObjectFrame
    BOOL    bSnapPoints     : 1;    // Snap/Object/ObjectPoint
    BOOL    bOrtho          : 1;    // Snap/Position/CreatingMoving
    BOOL    bBigOrtho       : 1;    // Snap/Position/ExtendEdges
    BOOL    bRotate         : 1;    // Snap/Position/Rotating
    INT16   nSnapArea;              // Snap/Object/Range
    INT16   nAngle;                 // Snap/Position/RotatingValue
    INT16   nBezAngle;              // Snap/Position/PointReduction

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsSnap( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsSnap() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsSnap& rOpt ) const;

    BOOL    IsSnapHelplines() const { Init(); return (BOOL) bSnapHelplines; }
    BOOL    IsSnapBorder() const { Init(); return (BOOL) bSnapBorder; }
    BOOL    IsSnapFrame() const { Init(); return (BOOL) bSnapFrame; }
    BOOL    IsSnapPoints() const { Init(); return (BOOL) bSnapPoints; }
    BOOL    IsOrtho() const { Init(); return (BOOL) bOrtho; }
    BOOL    IsBigOrtho() const { Init(); return (BOOL) bBigOrtho; }
    BOOL    IsRotate() const { Init(); return (BOOL) bRotate; }
    INT16   GetSnapArea() const { Init(); return nSnapArea; }
    INT16   GetAngle() const { Init(); return nAngle; }
    INT16   GetEliminatePolyPointLimitAngle() const { Init(); return nBezAngle; }

    void    SetSnapHelplines( BOOL bOn = TRUE ) { if( bSnapHelplines != bOn ) { OptionsChanged(); bSnapHelplines = bOn; } }
    void    SetSnapBorder( BOOL bOn = TRUE ) { if( bSnapBorder != bOn ) { OptionsChanged(); bSnapBorder = bOn; } }
    void    SetSnapFrame( BOOL bOn = TRUE ) { if( bSnapFrame != bOn ) { OptionsChanged(); bSnapFrame = bOn; } }
    void    SetSnapPoints( BOOL bOn = TRUE ) { if( bSnapPoints != bOn ) { OptionsChanged(); bSnapPoints = bOn; } }
    void    SetOrtho( BOOL bOn = TRUE ) { if( bOrtho != bOn ) { OptionsChanged(); bOrtho = bOn; } }
    void    SetBigOrtho( BOOL bOn = TRUE ) { if( bBigOrtho != bOn ) { OptionsChanged(); bBigOrtho = bOn; } }
    void    SetRotate( BOOL bOn = TRUE ) { if( bRotate != bOn ) { OptionsChanged(); bRotate = bOn; } }
    void    SetSnapArea( INT16 nIn ) { if( nSnapArea != nIn ) { OptionsChanged(); nSnapArea = nIn; } }
    void    SetAngle( INT16 nIn ) { if( nAngle != nIn ) { OptionsChanged(); nAngle = nIn; } }
    void    SetEliminatePolyPointLimitAngle( INT16 nIn ) { if( nBezAngle != nIn ) { OptionsChanged(); nBezAngle = nIn; } }
};

// -----------------------------------------------------------------------------

class SdOptionsSnapItem : public SfxPoolItem, public SdOptionsSnap
{
public:

                            SdOptionsSnapItem( USHORT nWhich);
                            SdOptionsSnapItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

};

// -----------------
// - SdOptionsZoom -
// -----------------

class SdOptionsZoom : public SdOptionsGeneric
{
private:

    INT32   nX; // Zoom/ScaleX
    INT32   nY; // Zoom/ScaleY

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsZoom( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsZoom() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsZoom& rOpt ) const;

    void    GetScale( INT32& rX, INT32& rY ) const { Init(); rX = nX; rY = nY; }
    void    SetScale( INT32 nInX, INT32 nInY ) { nX = nInX; nY = nInY; }
};

// -----------------------------------------------------------------------------

class SdOptionsZoomItem : public SfxPoolItem, public SdOptionsZoom
{
public:

                            SdOptionsZoomItem( USHORT nWhich);
                            SdOptionsZoomItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

};

// -----------------
// - SdOptionsGrid -
// -----------------

class SdOptionsGrid : public SdOptionsGeneric, public SvxOptionsGrid
{
protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsGrid( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsGrid();

    void    SetDefaults();
    BOOL    operator==( const SdOptionsGrid& rOpt ) const;

    UINT32  GetFldDrawX() const { Init(); return SvxOptionsGrid::GetFldDrawX(); }
    UINT32  GetFldDivisionX() const { Init(); return SvxOptionsGrid::GetFldDivisionX(); }
    UINT32  GetFldDrawY() const { Init(); return SvxOptionsGrid::GetFldDrawY(); }
    UINT32  GetFldDivisionY() const { Init(); return SvxOptionsGrid::GetFldDivisionY(); }
    UINT32  GetFldSnapX() const { Init(); return SvxOptionsGrid::GetFldSnapX(); }
    UINT32  GetFldSnapY() const { Init(); return SvxOptionsGrid::GetFldSnapY(); }
    BOOL    IsUseGridSnap() const { Init(); return SvxOptionsGrid::GetUseGridSnap(); }
    BOOL    IsSynchronize() const { Init(); return SvxOptionsGrid::GetSynchronize(); }
    BOOL    IsGridVisible() const { Init(); return SvxOptionsGrid::GetGridVisible(); }
    BOOL    IsEqualGrid() const { Init(); return SvxOptionsGrid::GetEqualGrid(); }
};

// -----------------------------------------------

class SdOptionsGridItem : public SvxGridItem
{

public:
                            SdOptionsGridItem( USHORT nWhich );
                            SdOptionsGridItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    void                    SetOptions( SdOptions* pOpts ) const;
};

// ------------------
// - SdOptionsPrint -
// ------------------

class SdOptionsPrint : public SdOptionsGeneric
{
private:

    BOOL    bDraw               : 1;    // Print/Content/Drawing
    BOOL    bNotes              : 1;    // Print/Content/Note
    BOOL    bHandout            : 1;    // Print/Content/Handout
    BOOL    bOutline            : 1;    // Print/Content/Outline
    BOOL    bDate               : 1;    // Print/Other/Date
    BOOL    bTime               : 1;    // Print/Other/Time
    BOOL    bPagename           : 1;    // Print/Other/PageName
    BOOL    bHiddenPages        : 1;    // Print/Other/HiddenPage
    BOOL    bPagesize           : 1;    // Print/Page/PageSize
    BOOL    bPagetile           : 1;    // Print/Page/PageTile
    BOOL    bWarningPrinter     : 1;
    BOOL    bWarningSize        : 1;
    BOOL    bWarningOrientation : 1;
    BOOL    bBooklet            : 1;    // Print/Page/Booklet
    BOOL    bFront              : 1;    // Print/Page/BookletFront
    BOOL    bBack               : 1;    // Print/Page/BookletFront
    BOOL    bCutPage            : 1;
    BOOL    bPaperbin           : 1;    // Print/Other/FromPrinterSetup
    UINT16  nQuality;                   // Print/Other/Quality

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsPrint( USHORT nConfigId, BOOL bUseConfig );
            ~SdOptionsPrint() {}

    void    SetDefaults();
    BOOL    operator==( const SdOptionsPrint& rOpt ) const;

    BOOL    IsDraw() const { Init(); return (BOOL) bDraw; }
    BOOL    IsNotes() const { Init(); return (BOOL) bNotes; }
    BOOL    IsHandout() const { Init(); return (BOOL) bHandout; }
    BOOL    IsOutline() const { Init(); return (BOOL) bOutline; }
    BOOL    IsDate() const { Init(); return (BOOL) bDate; }
    BOOL    IsTime() const { Init(); return (BOOL) bTime; }
    BOOL    IsPagename() const { Init(); return (BOOL) bPagename; }
    BOOL    IsHiddenPages() const { Init(); return (BOOL) bHiddenPages; }
    BOOL    IsPagesize() const { Init(); return (BOOL) bPagesize; }
    BOOL    IsPagetile() const { Init(); return (BOOL) bPagetile; }
    BOOL    IsWarningPrinter() const { Init(); return (BOOL) bWarningPrinter; }
    BOOL    IsWarningSize() const { Init(); return (BOOL) bWarningSize; }
    BOOL    IsWarningOrientation() const { Init(); return (BOOL) bWarningOrientation; }
    BOOL    IsBooklet() const { Init(); return (BOOL) bBooklet; }
    BOOL    IsFrontPage() const { Init(); return (BOOL) bFront; }
    BOOL    IsBackPage() const { Init(); return (BOOL) bBack; }
    BOOL    IsCutPage() const { Init(); return (BOOL) bCutPage; }
    BOOL    IsPaperbin() const { Init(); return (BOOL) bPaperbin; }
    UINT16  GetOutputQuality() const { Init(); return nQuality; }

    void    SetDraw( BOOL bOn = TRUE ) { if( bDraw != bOn ) { OptionsChanged(); bDraw = bOn; } }
    void    SetNotes( BOOL bOn = TRUE ) { if( bNotes != bOn ) { OptionsChanged(); bNotes = bOn; } }
    void    SetHandout( BOOL bOn = TRUE ) { if( bHandout != bOn ) { OptionsChanged(); bHandout = bOn; } }
    void    SetOutline( BOOL bOn = TRUE ) { if( bOutline != bOn ) { OptionsChanged(); bOutline = bOn; } }
    void    SetDate( BOOL bOn = TRUE ) { if( bDate != bOn ) { OptionsChanged(); bDate = bOn; } }
    void    SetTime( BOOL bOn = TRUE ) { if( bTime != bOn ) { OptionsChanged(); bTime = bOn; } }
    void    SetPagename( BOOL bOn = TRUE ) { if( bPagename != bOn ) { OptionsChanged(); bPagename = bOn; } }
    void    SetHiddenPages( BOOL bOn = TRUE ) { if( bHiddenPages != bOn ) { OptionsChanged(); bHiddenPages = bOn; } }
    void    SetPagesize( BOOL bOn = TRUE ) { if( bPagesize != bOn ) { OptionsChanged(); bPagesize = bOn; } }
    void    SetPagetile( BOOL bOn = TRUE ) { if( bPagetile != bOn ) { OptionsChanged(); bPagetile = bOn; } }
    void    SetWarningPrinter( BOOL bOn = TRUE ) { if( bWarningPrinter != bOn ) { OptionsChanged(); bWarningPrinter = bOn; } }
    void    SetWarningSize( BOOL bOn = TRUE ) { if( bWarningSize != bOn ) { OptionsChanged(); bWarningSize = bOn; } }
    void    SetWarningOrientation( BOOL bOn = TRUE ) { if( bWarningOrientation != bOn ) { OptionsChanged(); bWarningOrientation = bOn; } }
    void    SetBooklet( BOOL bOn = TRUE ) { if( bBooklet != bOn ) { OptionsChanged(); bBooklet = bOn; } }
    void    SetFrontPage( BOOL bOn = TRUE ) { if( bFront != bOn ) { OptionsChanged(); bFront = bOn; } }
    void    SetBackPage( BOOL bOn = TRUE ) { if( bBack != bOn ) { OptionsChanged(); bBack = bOn; } }
    void    SetCutPage( BOOL bOn = TRUE ) { if( bCutPage != bOn ) { OptionsChanged(); bCutPage = bOn; } }
    void    SetPaperbin( BOOL bOn = TRUE ) { if( bPaperbin != bOn ) { OptionsChanged(); bPaperbin = bOn; } }
    void    SetOutputQuality( UINT16 nInQuality ) { if( nQuality != nInQuality ) { OptionsChanged(); nQuality = nInQuality; } }
};

// -----------------------------------------------------------------------------

class SdOptionsPrintItem : public SfxPoolItem, public SdOptionsPrint
{
public:

                            SdOptionsPrintItem( USHORT nWhich);
                            SdOptionsPrintItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;
};

// -------------
// - SdOptions -
// -------------

class SdOptions : public SdOptionsLayout, public SdOptionsContents,
                  public SdOptionsMisc, public SdOptionsSnap,
                  public SdOptionsZoom, public SdOptionsGrid,
                  public SdOptionsPrint
{
public:

                        SdOptions( USHORT nConfigId );
                        ~SdOptions();

    void                SetDefaults( ULONG nOptionRange );
    void                StoreConfig();
};

#endif // _SD_OPTSITEM_HXX

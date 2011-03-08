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

#ifndef _SD_OPTSITEM_HXX
#define _SD_OPTSITEM_HXX

#include <unotools/configitem.hxx>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <svx/optgrid.hxx>
#include <svx/dlgutil.hxx>
#include "sddllapi.h"

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
class SdOptions;

namespace sd {
class FrameView;
}

// -----------------
// - SdOptionsItem -
// -----------------

class SdOptionsGeneric;

class SD_DLLPUBLIC SdOptionsItem : public ::utl::ConfigItem
{

private:

    const SdOptionsGeneric& mrParent;


public:

    SdOptionsItem( const SdOptionsGeneric& rParent, const ::rtl::OUString rSubTree );
    virtual ~SdOptionsItem();

    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetProperties(
                                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames );
    sal_Bool                PutProperties( const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                           const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues );
    void                    SetModified();
};

// --------------------
// - SdOptionsGeneric -
// --------------------

class SD_DLLPUBLIC SdOptionsGeneric
{
friend class SdOptionsItem;

private:

    ::rtl::OUString         maSubTree;
    SdOptionsItem*          mpCfgItem;
    USHORT                  mnConfigId;
    BOOL                    mbInit          : 1;
    BOOL                    mbEnableModify  : 1;

    SD_DLLPRIVATE void Commit( SdOptionsItem& rCfgItem ) const;
    SD_DLLPRIVATE ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPropertyNames() const;

protected:

    void                    Init() const;
    void                    OptionsChanged() { if( mpCfgItem && mbEnableModify ) mpCfgItem->SetModified(); }

protected:

    virtual void            GetPropNameArray( const char**& ppNames, ULONG& rCount ) const = 0;
    virtual BOOL            ReadData( const ::com::sun::star::uno::Any* pValues ) = 0;
    virtual BOOL            WriteData( ::com::sun::star::uno::Any* pValues ) const = 0;

public:

                            SdOptionsGeneric( USHORT nConfigId, const ::rtl::OUString& rSubTree );
                            virtual ~SdOptionsGeneric();

    const ::rtl::OUString&  GetSubTree() const { return maSubTree; }
    USHORT                  GetConfigId() const { return mnConfigId; }

    void                    EnableModify( BOOL bModify ) { mbEnableModify = bModify; }

    void                    Store();


    static bool             isMetricSystem();
};

// -------------------
// - SdOptionsLayout -
// -------------------

class SD_DLLPUBLIC SdOptionsLayout : public SdOptionsGeneric
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
            virtual ~SdOptionsLayout() {}

    BOOL    operator==( const SdOptionsLayout& rOpt ) const;

    BOOL    IsRulerVisible() const { Init(); return (BOOL) bRuler; }
    BOOL    IsMoveOutline() const { Init(); return (BOOL) bMoveOutline; }
    BOOL    IsDragStripes() const { Init(); return (BOOL) bDragStripes; }
    BOOL    IsHandlesBezier() const { Init(); return (BOOL) bHandlesBezier; }
    BOOL    IsHelplines() const { Init(); return (BOOL) bHelplines; }
    UINT16  GetMetric() const { Init(); return( ( 0xffff == nMetric ) ? (UINT16)SfxModule::GetCurrentFieldUnit() : nMetric ); }
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

class SD_DLLPUBLIC SdOptionsLayoutItem : public SfxPoolItem
{
public:

                            SdOptionsLayoutItem( USHORT nWhich);
                            SdOptionsLayoutItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsLayout&        GetOptionsLayout() { return maOptionsLayout; }
private:
    SdOptionsLayout maOptionsLayout;
};

// ---------------------
// - SdOptionsContents -
// ---------------------

class SD_DLLPUBLIC SdOptionsContents : public SdOptionsGeneric
{
private:
protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsContents( USHORT nConfigId, BOOL bUseConfig );
            virtual ~SdOptionsContents() {}

    BOOL    operator==( const SdOptionsContents& rOpt ) const;
};

// -----------------------------------------------------------------------------

class SD_DLLPUBLIC SdOptionsContentsItem : public SfxPoolItem
{
public:

                            SdOptionsContentsItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsContents&      GetOptionsContents() { return maOptionsContents; }
private:
    SdOptionsContents       maOptionsContents;
};

// -----------------
// - SdOptionsMisc -
// -----------------

class SD_DLLPUBLIC SdOptionsMisc : public SdOptionsGeneric
{
private:

    ULONG   nDefaultObjectSizeWidth;
    ULONG   nDefaultObjectSizeHeight;

    BOOL    bStartWithTemplate      : 1;    // Misc/NewDoc/AutoPilot
    BOOL    bMarkedHitMovesAlways   : 1;    // Misc/ObjectMoveable
    BOOL    bMoveOnlyDragging       : 1;    // Currently, not in use !!!
    BOOL    bCrookNoContortion      : 1;    // Misc/NoDistort
    BOOL    bQuickEdit              : 1;    // Misc/TextObject/QuickEditing
    BOOL    bMasterPageCache        : 1;    // Misc/BackgroundCache
    BOOL    bDragWithCopy           : 1;    // Misc/CopyWhileMoving
    BOOL    bPickThrough            : 1;    // Misc/TextObject/Selectable
    BOOL    bBigHandles             : 1;    // Misc/BigHandles
    BOOL    bDoubleClickTextEdit    : 1;    // Misc/DclickTextedit
    BOOL    bClickChangeRotation    : 1;    // Misc/RotateClick
    BOOL    bStartWithActualPage    : 1;    // Misc/Start/CurrentPage
    BOOL    bSolidDragging          : 1;    // Misc/ModifyWithAttributes
    BOOL    bSolidMarkHdl           : 1;    // /Misc/SimpleHandles
    BOOL    bSummationOfParagraphs  : 1;    // misc/SummationOfParagraphs
    BOOL    bShowUndoDeleteWarning  : 1;    // Misc/ShowUndoDeleteWarning
    // #i75315#
    BOOL    bSlideshowRespectZOrder : 1;    // Misc/SlideshowRespectZOrder
    BOOL    bShowComments           : 1;    // Misc/ShowComments

    sal_Bool    bPreviewNewEffects;
    sal_Bool    bPreviewChangedEffects;
    sal_Bool    bPreviewTransitions;

    sal_Int32   mnDisplay;

    /** This value controls the device to use for formatting documents.
        The currently supported values are 0 for the current printer or 1
        for the printer independent virtual device the can be retrieved from
        the modules.
    */
    USHORT  mnPrinterIndependentLayout;     // Misc/Compatibility/PrinterIndependentLayout
// Misc

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsMisc( USHORT nConfigId, BOOL bUseConfig );
            virtual ~SdOptionsMisc() {}

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
    BOOL    IsSolidDragging() const { Init(); return (BOOL) bSolidDragging; }
    BOOL    IsSolidMarkHdl() const { Init(); return (BOOL) bSolidMarkHdl; }
    BOOL    IsSummationOfParagraphs() const { Init(); return bSummationOfParagraphs != 0; };

    /** Return the currently selected printer independent layout mode.
        @return
            Returns 1 for printer independent layout enabled and 0 when it
            is disabled.  Other values are reserved for future use.
    */
    USHORT  GetPrinterIndependentLayout() const { Init(); return mnPrinterIndependentLayout; };
    BOOL    IsShowUndoDeleteWarning() const { Init(); return (BOOL) bShowUndoDeleteWarning; }
    BOOL    IsSlideshowRespectZOrder() const { Init(); return (BOOL) bSlideshowRespectZOrder; }
    ULONG   GetDefaultObjectSizeWidth() const { Init(); return nDefaultObjectSizeWidth; }
    ULONG   GetDefaultObjectSizeHeight() const { Init(); return nDefaultObjectSizeHeight; }

    sal_Bool    IsPreviewNewEffects() const { Init(); return bPreviewNewEffects; }
    sal_Bool    IsPreviewChangedEffects() const { Init(); return bPreviewChangedEffects; }
    sal_Bool    IsPreviewTransitions() const { Init(); return bPreviewTransitions; }

    sal_Int32   GetDisplay() const { Init(); return mnDisplay; }
    void        SetDisplay( sal_Int32 nDisplay = 0 ) { if( mnDisplay != nDisplay ) { OptionsChanged(); mnDisplay = nDisplay; } }

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
    void    SetSummationOfParagraphs( BOOL bOn = TRUE ){ if ( bOn != bSummationOfParagraphs ) { OptionsChanged(); bSummationOfParagraphs = bOn; } }
    /** Set the printer independent layout mode.
        @param nOn
            The default value is to switch printer independent layout on,
            hence the parameters name.  Use 0 for turning it off.  Other
            values are reserved for future use.
    */
    void    SetPrinterIndependentLayout (USHORT nOn = 1 ){ if ( nOn != mnPrinterIndependentLayout ) { OptionsChanged(); mnPrinterIndependentLayout = nOn; } }
    void    SetSolidDragging( BOOL bOn = TRUE ) { if( bSolidDragging != bOn ) { OptionsChanged(); bSolidDragging = bOn; } }
    void    SetSolidMarkHdl( BOOL bOn = TRUE ) { if( bSolidMarkHdl != bOn ) { OptionsChanged(); bSolidMarkHdl = bOn; } }
    void    SetShowUndoDeleteWarning( BOOL bOn = TRUE ) { if( bShowUndoDeleteWarning != bOn ) { OptionsChanged(); bShowUndoDeleteWarning = bOn; } }
    void    SetSlideshowRespectZOrder( BOOL bOn = TRUE ) { if( bSlideshowRespectZOrder != bOn ) { OptionsChanged(); bSlideshowRespectZOrder = bOn; } }
    void    SetDefaultObjectSizeWidth( ULONG nWidth ) { if( nDefaultObjectSizeWidth != nWidth ) { OptionsChanged(); nDefaultObjectSizeWidth = nWidth; } }
    void    SetDefaultObjectSizeHeight( ULONG nHeight ) { if( nDefaultObjectSizeHeight != nHeight ) { OptionsChanged(); nDefaultObjectSizeHeight = nHeight; } }

    void    SetPreviewNewEffects( sal_Bool bOn )  { if( bPreviewNewEffects != bOn ) { OptionsChanged(); bPreviewNewEffects = bOn; } }
    void    SetPreviewChangedEffects( sal_Bool bOn )  { if( bPreviewChangedEffects != bOn ) { OptionsChanged(); bPreviewChangedEffects = bOn; } }
    void    SetPreviewTransitions( sal_Bool bOn )  { if( bPreviewTransitions != bOn ) { OptionsChanged(); bPreviewTransitions = bOn; } }

    BOOL    IsShowComments() const { Init(); return bShowComments; }
    void    SetShowComments( BOOL bShow )  { if( bShowComments != bShow ) { OptionsChanged(); bShowComments = bShow; } }
};

// -----------------------------------------------------------------------------

class SD_DLLPUBLIC SdOptionsMiscItem : public SfxPoolItem
{
public:

                            SdOptionsMiscItem( USHORT nWhich);
                            SdOptionsMiscItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsMisc&          GetOptionsMisc() { return maOptionsMisc; }
    const SdOptionsMisc&    GetOptionsMisc() const { return maOptionsMisc; }
private:
    SdOptionsMisc           maOptionsMisc;
};

// -----------------
// - SdOptionsSnap -
// -----------------

class SD_DLLPUBLIC SdOptionsSnap : public SdOptionsGeneric
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
            virtual ~SdOptionsSnap() {}

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

class SD_DLLPUBLIC SdOptionsSnapItem : public SfxPoolItem
{
public:

                            SdOptionsSnapItem( USHORT nWhich);
                            SdOptionsSnapItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsSnap&          GetOptionsSnap() { return maOptionsSnap; }
private:
    SdOptionsSnap           maOptionsSnap;
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
            virtual ~SdOptionsZoom() {}

    BOOL    operator==( const SdOptionsZoom& rOpt ) const;

    void    GetScale( INT32& rX, INT32& rY ) const { Init(); rX = nX; rY = nY; }
    void    SetScale( INT32 nInX, INT32 nInY ) { if( nX != nInX || nY != nInY ) { OptionsChanged(); nX = nInX; nY = nInY; } }
};

// -----------------------------------------------------------------------------

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
            virtual ~SdOptionsGrid();

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

    void    SetFldDrawX( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDrawX() ) { OptionsChanged(); SvxOptionsGrid::SetFldDrawX( nSet ); } }
    void    SetFldDivisionX( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDivisionX() ) { OptionsChanged(); SvxOptionsGrid::SetFldDivisionX( nSet ); } }
    void    SetFldDrawY( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDrawY() ) { OptionsChanged(); SvxOptionsGrid::SetFldDrawY( nSet ); } }
    void    SetFldDivisionY( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDivisionY() ) { OptionsChanged(); SvxOptionsGrid::SetFldDivisionY( nSet ); } }
    void    SetFldSnapX( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldSnapX() ) { OptionsChanged(); SvxOptionsGrid::SetFldSnapX( nSet ); } }
    void    SetFldSnapY( UINT32 nSet ) { if( nSet != SvxOptionsGrid::GetFldSnapY() ) { OptionsChanged(); SvxOptionsGrid::SetFldSnapY( nSet ); } }
    void    SetUseGridSnap( BOOL bSet ) { if( bSet != SvxOptionsGrid::GetUseGridSnap() ) { OptionsChanged(); SvxOptionsGrid::SetUseGridSnap( bSet ); } }
    void    SetSynchronize( BOOL bSet ) { if( bSet != SvxOptionsGrid::GetSynchronize() ) { OptionsChanged(); SvxOptionsGrid::SetSynchronize( bSet ); } }
    void    SetGridVisible( BOOL bSet ) { if( bSet != SvxOptionsGrid::GetGridVisible() ) { OptionsChanged(); SvxOptionsGrid::SetGridVisible( bSet ); } }
    void    SetEqualGrid( BOOL bSet ) { if( bSet != SvxOptionsGrid::GetEqualGrid() ) { OptionsChanged(); SvxOptionsGrid::SetEqualGrid( bSet ); } }
};

// -----------------------------------------------

class SdOptionsGridItem : public SvxGridItem
{

public:
                            SdOptionsGridItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    void                    SetOptions( SdOptions* pOpts ) const;
};

// ------------------
// - SdOptionsPrint -
// ------------------

class SD_DLLPUBLIC SdOptionsPrint : public SdOptionsGeneric
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
    BOOL    bWarningPrinter     : 1;    //  These flags you get
    BOOL    bWarningSize        : 1;    //  from the common options,
    BOOL    bWarningOrientation : 1;    //  currently org.openoffice.Office.Common.xml (class OfaMiscCfg ; sfx2/misccfg.hxx )
    BOOL    bBooklet            : 1;    // Print/Page/Booklet
    BOOL    bFront              : 1;    // Print/Page/BookletFront
    BOOL    bBack               : 1;    // Print/Page/BookletFront
    BOOL    bCutPage            : 1;    // NOT persistent !!!
    BOOL    bPaperbin           : 1;    // Print/Other/FromPrinterSetup
    BOOL    mbHandoutHorizontal : 1;    // Order Page previews on Handout Pages horizontal
    UINT16  mnHandoutPages;             // Number of page previews on handout page (only 1/2/4/6/9 are supported)
    UINT16  nQuality;                   // Print/Other/Quality

protected:

    virtual void GetPropNameArray( const char**& ppNames, ULONG& rCount ) const;
    virtual BOOL ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual BOOL WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsPrint( USHORT nConfigId, BOOL bUseConfig );
            virtual ~SdOptionsPrint() {}

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
    BOOL    IsHandoutHorizontal() const { Init(); return mbHandoutHorizontal; }
    UINT16  GetHandoutPages() const { Init(); return mnHandoutPages; }

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
    void    SetHandoutHorizontal( BOOL bHandoutHorizontal ) { if( mbHandoutHorizontal != bHandoutHorizontal ) { OptionsChanged(); mbHandoutHorizontal = bHandoutHorizontal; } }
    void    SetHandoutPages( UINT16 nHandoutPages ) { if( nHandoutPages != mnHandoutPages ) { OptionsChanged(); mnHandoutPages = nHandoutPages; } }
};

// -----------------------------------------------------------------------------

class SD_DLLPUBLIC SdOptionsPrintItem : public SfxPoolItem
{
public:

                            SdOptionsPrintItem( USHORT nWhich);
                            SdOptionsPrintItem( USHORT nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsPrint&         GetOptionsPrint() { return maOptionsPrint; }
    const SdOptionsPrint&   GetOptionsPrint() const { return maOptionsPrint; }
private:
    SdOptionsPrint  maOptionsPrint;
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
                        virtual ~SdOptions();

    void                StoreConfig( ULONG nOptionRange = SD_OPTIONS_ALL );
};

#endif // _SD_OPTSITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

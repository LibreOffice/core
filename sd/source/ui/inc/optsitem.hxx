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

#ifndef INCLUDED_SD_SOURCE_UI_INC_OPTSITEM_HXX
#define INCLUDED_SD_SOURCE_UI_INC_OPTSITEM_HXX

#include <unotools/configitem.hxx>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <svx/optgrid.hxx>
#include <svx/dlgutil.hxx>
#include "sddllapi.h"

// Option ranges

#define SD_OPTIONS_NONE     0x00000000
#define SD_OPTIONS_ALL      0xffffffff

#define SD_OPTIONS_LAYOUT   0x00000001
#define SD_OPTIONS_CONTENTS 0x00000002
#define SD_OPTIONS_MISC     0x00000004
#define SD_OPTIONS_SNAP     0x00000008
#define SD_OPTIONS_ZOOM     0x00000010
#define SD_OPTIONS_GRID     0x00000020
#define SD_OPTIONS_PRINT    0x00000040

class SdOptions;

namespace sd {
class FrameView;
}

class SdOptionsGeneric;

class SD_DLLPUBLIC SdOptionsItem : public ::utl::ConfigItem
{

private:

    const SdOptionsGeneric& mrParent;


public:

    SdOptionsItem( const SdOptionsGeneric& rParent, const OUString rSubTree );
    virtual ~SdOptionsItem();

    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetProperties(
                                const ::com::sun::star::uno::Sequence< OUString >& rNames );
    sal_Bool                PutProperties( const com::sun::star::uno::Sequence< OUString >& rNames,
                                           const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues );
    void                    SetModified();
};

class SD_DLLPUBLIC SdOptionsGeneric
{
friend class SdOptionsItem;

private:

    OUString         maSubTree;
    SdOptionsItem*          mpCfgItem;
    sal_uInt16                  mnConfigId;
    sal_Bool                    mbInit          : 1;
    sal_Bool                    mbEnableModify  : 1;

    SAL_DLLPRIVATE void Commit( SdOptionsItem& rCfgItem ) const;
    SAL_DLLPRIVATE ::com::sun::star::uno::Sequence< OUString > GetPropertyNames() const;

protected:

    void                    Init() const;
    void                    OptionsChanged() { if( mpCfgItem && mbEnableModify ) mpCfgItem->SetModified(); }

protected:

    virtual void            GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const = 0;
    virtual sal_Bool            ReadData( const ::com::sun::star::uno::Any* pValues ) = 0;
    virtual sal_Bool            WriteData( ::com::sun::star::uno::Any* pValues ) const = 0;

public:

                            SdOptionsGeneric( sal_uInt16 nConfigId, const OUString& rSubTree );
                            virtual ~SdOptionsGeneric();

    const OUString&  GetSubTree() const { return maSubTree; }
    sal_uInt16                  GetConfigId() const { return mnConfigId; }

    void                    EnableModify( sal_Bool bModify ) { mbEnableModify = bModify; }

    void                    Store();


    static bool             isMetricSystem();
};

class SD_DLLPUBLIC SdOptionsLayout : public SdOptionsGeneric
{
private:

    sal_Bool    bRuler; // Layout/Display/Ruler
    sal_Bool    bMoveOutline;   // Layout/Display/Contur
    sal_Bool    bDragStripes;   // Layout/Display/Guide
    sal_Bool    bHandlesBezier; // Layout/Display/Bezier
    sal_Bool    bHelplines; // Layout/Display/Helpline
    sal_uInt16  nMetric;                    // Layout/Other/MeasureUnit
    sal_uInt16  nDefTab;                    // Layout/Other/TabStop

protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:
            SdOptionsLayout( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsLayout() {}

    sal_Bool    operator==( const SdOptionsLayout& rOpt ) const;

    sal_Bool    IsRulerVisible() const { Init(); return (sal_Bool) bRuler; }
    sal_Bool    IsMoveOutline() const { Init(); return (sal_Bool) bMoveOutline; }
    sal_Bool    IsDragStripes() const { Init(); return (sal_Bool) bDragStripes; }
    sal_Bool    IsHandlesBezier() const { Init(); return (sal_Bool) bHandlesBezier; }
    sal_Bool    IsHelplines() const { Init(); return (sal_Bool) bHelplines; }
    sal_uInt16  GetMetric() const { Init(); return( ( 0xffff == nMetric ) ? (sal_uInt16)SfxModule::GetCurrentFieldUnit() : nMetric ); }
    sal_uInt16  GetDefTab() const { Init(); return nDefTab; }

    void    SetRulerVisible( sal_Bool bOn = sal_True ) { if( bRuler != bOn ) { OptionsChanged(); bRuler = bOn; } }
    void    SetMoveOutline( sal_Bool bOn = sal_True ) { if( bMoveOutline != bOn ) { OptionsChanged(); bMoveOutline = bOn; } }
    void    SetDragStripes( sal_Bool bOn = sal_True ) { if( bDragStripes != bOn ) { OptionsChanged(); bDragStripes = bOn; } }
    void    SetHandlesBezier( sal_Bool bOn = sal_True ) { if( bHandlesBezier != bOn ) { OptionsChanged(); bHandlesBezier = bOn; } }
    void    SetHelplines( sal_Bool bOn = sal_True ) { if( bHelplines != bOn ) { OptionsChanged(); bHelplines = bOn; } }
    void    SetMetric( sal_uInt16 nInMetric ) { if( nMetric != nInMetric ) { OptionsChanged(); nMetric = nInMetric; } }
    void    SetDefTab( sal_uInt16 nTab ) { if( nDefTab != nTab ) { OptionsChanged(); nDefTab = nTab; } }
};

class SD_DLLPUBLIC SdOptionsLayoutItem : public SfxPoolItem
{
public:

                            SdOptionsLayoutItem( sal_uInt16 nWhich);
                            SdOptionsLayoutItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsLayout&        GetOptionsLayout() { return maOptionsLayout; }
private:
    SdOptionsLayout maOptionsLayout;
};

class SD_DLLPUBLIC SdOptionsContents : public SdOptionsGeneric
{
private:
protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsContents( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsContents() {}

    sal_Bool    operator==( const SdOptionsContents& rOpt ) const;
};

class SD_DLLPUBLIC SdOptionsContentsItem : public SfxPoolItem
{
public:

                            SdOptionsContentsItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsContents&      GetOptionsContents() { return maOptionsContents; }
private:
    SdOptionsContents       maOptionsContents;
};

class SD_DLLPUBLIC SdOptionsMisc : public SdOptionsGeneric
{
private:

    sal_uLong   nDefaultObjectSizeWidth;
    sal_uLong   nDefaultObjectSizeHeight;

    sal_Bool    bStartWithTemplate      : 1;    // Misc/NewDoc/AutoPilot
    sal_Bool    bMarkedHitMovesAlways   : 1;    // Misc/ObjectMoveable
    sal_Bool    bMoveOnlyDragging       : 1;    // Currently, not in use !!!
    sal_Bool    bCrookNoContortion      : 1;    // Misc/NoDistort
    sal_Bool    bQuickEdit              : 1;    // Misc/TextObject/QuickEditing
    sal_Bool    bMasterPageCache        : 1;    // Misc/BackgroundCache
    sal_Bool    bDragWithCopy           : 1;    // Misc/CopyWhileMoving
    sal_Bool    bPickThrough            : 1;    // Misc/TextObject/Selectable
    sal_Bool    bDoubleClickTextEdit    : 1;    // Misc/DclickTextedit
    sal_Bool    bClickChangeRotation    : 1;    // Misc/RotateClick
    sal_Bool    bEnableSdremote         : 1;    // Misc/Start/EnableSdremote
    sal_Bool    bEnablePresenterScreen : 1;    // Misc/Start/EnablePresenterDisplay
    sal_Bool    bSolidDragging          : 1;    // Misc/ModifyWithAttributes
    sal_Bool    bSummationOfParagraphs  : 1;    // misc/SummationOfParagraphs
    sal_Bool    bShowUndoDeleteWarning  : 1;    // Misc/ShowUndoDeleteWarning
    // #i75315#
    sal_Bool    bSlideshowRespectZOrder : 1;    // Misc/SlideshowRespectZOrder
    sal_Bool    bShowComments           : 1;    // Misc/ShowComments

    sal_Bool    bPreviewNewEffects;
    sal_Bool    bPreviewChangedEffects;
    sal_Bool    bPreviewTransitions;

    sal_Int32   mnDisplay;

    sal_Int32 mnPenColor;
    double mnPenWidth;

    /** This value controls the device to use for formatting documents.
        The currently supported values are 0 for the current printer or 1
        for the printer independent virtual device the can be retrieved from
        the modules.
    */
    sal_uInt16  mnPrinterIndependentLayout;     // Misc/Compatibility/PrinterIndependentLayout
// Misc

protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsMisc( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsMisc() {}

    sal_Bool    operator==( const SdOptionsMisc& rOpt ) const;

    sal_Bool    IsStartWithTemplate() const { Init(); return (sal_Bool) bStartWithTemplate; }
    sal_Bool    IsMarkedHitMovesAlways() const { Init(); return (sal_Bool) bMarkedHitMovesAlways; }
    sal_Bool    IsMoveOnlyDragging() const { Init(); return (sal_Bool) bMoveOnlyDragging; }
    sal_Bool    IsCrookNoContortion() const { Init(); return (sal_Bool) bCrookNoContortion; }
    sal_Bool    IsQuickEdit() const { Init(); return (sal_Bool) bQuickEdit; }
    sal_Bool    IsMasterPagePaintCaching() const { Init(); return (sal_Bool) bMasterPageCache; }
    sal_Bool    IsDragWithCopy() const { Init(); return (sal_Bool) bDragWithCopy; }
    sal_Bool    IsPickThrough() const { Init(); return (sal_Bool) bPickThrough; }
    sal_Bool    IsDoubleClickTextEdit() const { Init(); return (sal_Bool) bDoubleClickTextEdit; }
    sal_Bool    IsClickChangeRotation() const { Init(); return (sal_Bool) bClickChangeRotation; }
    sal_Bool    IsEnableSdremote() const { Init(); return (sal_Bool) bEnableSdremote; }
    sal_Bool    IsEnablePresenterScreen() const { Init(); return (sal_Bool) bEnablePresenterScreen; }
    sal_Bool    IsSolidDragging() const { Init(); return (sal_Bool) bSolidDragging; }
    sal_Bool    IsSummationOfParagraphs() const { Init(); return bSummationOfParagraphs != 0; };

    /** Return the currently selected printer independent layout mode.
        @return
            Returns 1 for printer independent layout enabled and 0 when it
            is disabled.  Other values are reserved for future use.
    */
    sal_uInt16  GetPrinterIndependentLayout() const { Init(); return mnPrinterIndependentLayout; };
    sal_Bool    IsShowUndoDeleteWarning() const { Init(); return (sal_Bool) bShowUndoDeleteWarning; }
    sal_Bool    IsSlideshowRespectZOrder() const { Init(); return (sal_Bool) bSlideshowRespectZOrder; }
    sal_uLong   GetDefaultObjectSizeWidth() const { Init(); return nDefaultObjectSizeWidth; }
    sal_uLong   GetDefaultObjectSizeHeight() const { Init(); return nDefaultObjectSizeHeight; }

    sal_Bool    IsPreviewNewEffects() const { Init(); return bPreviewNewEffects; }
    sal_Bool    IsPreviewChangedEffects() const { Init(); return bPreviewChangedEffects; }
    sal_Bool    IsPreviewTransitions() const { Init(); return bPreviewTransitions; }

    sal_Int32   GetDisplay() const;
    void        SetDisplay( sal_Int32 nDisplay = 0 );

    sal_Int32 GetPresentationPenColor() const { Init(); return mnPenColor; }
    void      SetPresentationPenColor( sal_Int32 nPenColor ) { if( mnPenColor != nPenColor ) { OptionsChanged(); mnPenColor = nPenColor; } }

    double    GetPresentationPenWidth() const { Init(); return mnPenWidth; }
    void      SetPresentationPenWidth( double nPenWidth ) { if( mnPenWidth != nPenWidth ) { OptionsChanged(); mnPenWidth = nPenWidth; } }

    void    SetStartWithTemplate( sal_Bool bOn = sal_True ) { if( bStartWithTemplate != bOn ) { OptionsChanged(); bStartWithTemplate = bOn; } }
    void    SetMarkedHitMovesAlways( sal_Bool bOn = sal_True ) { if( bMarkedHitMovesAlways != bOn ) { OptionsChanged(); bMarkedHitMovesAlways = bOn; } }
    void    SetMoveOnlyDragging( sal_Bool bOn = sal_True ) { if( bMoveOnlyDragging != bOn ) { OptionsChanged(); bMoveOnlyDragging = bOn; } }
    void    SetCrookNoContortion( sal_Bool bOn = sal_True ) { if( bCrookNoContortion != bOn ) { OptionsChanged(); bCrookNoContortion = bOn; } }
    void    SetQuickEdit( sal_Bool bOn = sal_True ) { if( bQuickEdit != bOn ) { OptionsChanged(); bQuickEdit = bOn; } }
    void    SetMasterPagePaintCaching( sal_Bool bOn = sal_True ) { if( bMasterPageCache != bOn ) { OptionsChanged(); bMasterPageCache = bOn; } }
    void    SetDragWithCopy( sal_Bool bOn = sal_True ) { if( bDragWithCopy != bOn ) { OptionsChanged(); bDragWithCopy = bOn; } }
    void    SetPickThrough( sal_Bool bOn = sal_True ) { if( bPickThrough != bOn ) { OptionsChanged(); bPickThrough = bOn; } }
    void    SetDoubleClickTextEdit( sal_Bool bOn = sal_True ) { if( bDoubleClickTextEdit != bOn ) { OptionsChanged(); bDoubleClickTextEdit = bOn; } }
    void    SetClickChangeRotation( sal_Bool bOn = sal_True ) { if( bClickChangeRotation != bOn ) { OptionsChanged(); bClickChangeRotation = bOn; } }
    void    SetEnableSdremote( sal_Bool bOn = sal_True ) { if( bEnableSdremote != bOn ) { OptionsChanged(); bEnableSdremote = bOn; } }
    void    SetEnablePresenterScreen( sal_Bool bOn = sal_True ) { if( bEnablePresenterScreen != bOn ) { OptionsChanged(); bEnablePresenterScreen = bOn; } }
    void    SetSummationOfParagraphs( sal_Bool bOn = sal_True ){ if ( bOn != bSummationOfParagraphs ) { OptionsChanged(); bSummationOfParagraphs = bOn; } }
    /** Set the printer independent layout mode.
        @param nOn
            The default value is to switch printer independent layout on,
            hence the parameters name.  Use 0 for turning it off.  Other
            values are reserved for future use.
    */
    void    SetPrinterIndependentLayout (sal_uInt16 nOn = 1 ){ if ( nOn != mnPrinterIndependentLayout ) { OptionsChanged(); mnPrinterIndependentLayout = nOn; } }
    void    SetSolidDragging( sal_Bool bOn = sal_True ) { if( bSolidDragging != bOn ) { OptionsChanged(); bSolidDragging = bOn; } }
    void    SetShowUndoDeleteWarning( sal_Bool bOn = sal_True ) { if( bShowUndoDeleteWarning != bOn ) { OptionsChanged(); bShowUndoDeleteWarning = bOn; } }
    void    SetSlideshowRespectZOrder( sal_Bool bOn = sal_True ) { if( bSlideshowRespectZOrder != bOn ) { OptionsChanged(); bSlideshowRespectZOrder = bOn; } }
    void    SetDefaultObjectSizeWidth( sal_uLong nWidth ) { if( nDefaultObjectSizeWidth != nWidth ) { OptionsChanged(); nDefaultObjectSizeWidth = nWidth; } }
    void    SetDefaultObjectSizeHeight( sal_uLong nHeight ) { if( nDefaultObjectSizeHeight != nHeight ) { OptionsChanged(); nDefaultObjectSizeHeight = nHeight; } }

    void    SetPreviewNewEffects( sal_Bool bOn )  { if( bPreviewNewEffects != bOn ) { OptionsChanged(); bPreviewNewEffects = bOn; } }
    void    SetPreviewChangedEffects( sal_Bool bOn )  { if( bPreviewChangedEffects != bOn ) { OptionsChanged(); bPreviewChangedEffects = bOn; } }
    void    SetPreviewTransitions( sal_Bool bOn )  { if( bPreviewTransitions != bOn ) { OptionsChanged(); bPreviewTransitions = bOn; } }

    sal_Bool    IsShowComments() const { Init(); return bShowComments; }
    void    SetShowComments( sal_Bool bShow )  { if( bShowComments != bShow ) { OptionsChanged(); bShowComments = bShow; } }
};

class SD_DLLPUBLIC SdOptionsMiscItem : public SfxPoolItem
{
public:

                            SdOptionsMiscItem( sal_uInt16 nWhich);
                            SdOptionsMiscItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsMisc&          GetOptionsMisc() { return maOptionsMisc; }
    const SdOptionsMisc&    GetOptionsMisc() const { return maOptionsMisc; }
private:
    SdOptionsMisc           maOptionsMisc;
};

class SD_DLLPUBLIC SdOptionsSnap : public SdOptionsGeneric
{
private:

    sal_Bool    bSnapHelplines  : 1;    // Snap/Object/SnapLine
    sal_Bool    bSnapBorder     : 1;    // Snap/Object/PageMargin
    sal_Bool    bSnapFrame      : 1;    // Snap/Object/ObjectFrame
    sal_Bool    bSnapPoints     : 1;    // Snap/Object/ObjectPoint
    sal_Bool    bOrtho          : 1;    // Snap/Position/CreatingMoving
    sal_Bool    bBigOrtho       : 1;    // Snap/Position/ExtendEdges
    sal_Bool    bRotate         : 1;    // Snap/Position/Rotating
    sal_Int16   nSnapArea;              // Snap/Object/Range
    sal_Int16   nAngle;                 // Snap/Position/RotatingValue
    sal_Int16   nBezAngle;              // Snap/Position/PointReduction

protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsSnap( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsSnap() {}

    sal_Bool    operator==( const SdOptionsSnap& rOpt ) const;

    sal_Bool    IsSnapHelplines() const { Init(); return (sal_Bool) bSnapHelplines; }
    sal_Bool    IsSnapBorder() const { Init(); return (sal_Bool) bSnapBorder; }
    sal_Bool    IsSnapFrame() const { Init(); return (sal_Bool) bSnapFrame; }
    sal_Bool    IsSnapPoints() const { Init(); return (sal_Bool) bSnapPoints; }
    sal_Bool    IsOrtho() const { Init(); return (sal_Bool) bOrtho; }
    sal_Bool    IsBigOrtho() const { Init(); return (sal_Bool) bBigOrtho; }
    sal_Bool    IsRotate() const { Init(); return (sal_Bool) bRotate; }
    sal_Int16   GetSnapArea() const { Init(); return nSnapArea; }
    sal_Int16   GetAngle() const { Init(); return nAngle; }
    sal_Int16   GetEliminatePolyPointLimitAngle() const { Init(); return nBezAngle; }

    void    SetSnapHelplines( sal_Bool bOn = sal_True ) { if( bSnapHelplines != bOn ) { OptionsChanged(); bSnapHelplines = bOn; } }
    void    SetSnapBorder( sal_Bool bOn = sal_True ) { if( bSnapBorder != bOn ) { OptionsChanged(); bSnapBorder = bOn; } }
    void    SetSnapFrame( sal_Bool bOn = sal_True ) { if( bSnapFrame != bOn ) { OptionsChanged(); bSnapFrame = bOn; } }
    void    SetSnapPoints( sal_Bool bOn = sal_True ) { if( bSnapPoints != bOn ) { OptionsChanged(); bSnapPoints = bOn; } }
    void    SetOrtho( sal_Bool bOn = sal_True ) { if( bOrtho != bOn ) { OptionsChanged(); bOrtho = bOn; } }
    void    SetBigOrtho( sal_Bool bOn = sal_True ) { if( bBigOrtho != bOn ) { OptionsChanged(); bBigOrtho = bOn; } }
    void    SetRotate( sal_Bool bOn = sal_True ) { if( bRotate != bOn ) { OptionsChanged(); bRotate = bOn; } }
    void    SetSnapArea( sal_Int16 nIn ) { if( nSnapArea != nIn ) { OptionsChanged(); nSnapArea = nIn; } }
    void    SetAngle( sal_Int16 nIn ) { if( nAngle != nIn ) { OptionsChanged(); nAngle = nIn; } }
    void    SetEliminatePolyPointLimitAngle( sal_Int16 nIn ) { if( nBezAngle != nIn ) { OptionsChanged(); nBezAngle = nIn; } }
};

class SD_DLLPUBLIC SdOptionsSnapItem : public SfxPoolItem
{
public:

                            SdOptionsSnapItem( sal_uInt16 nWhich);
                            SdOptionsSnapItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsSnap&          GetOptionsSnap() { return maOptionsSnap; }
private:
    SdOptionsSnap           maOptionsSnap;
};

class SdOptionsZoom : public SdOptionsGeneric
{
private:

    sal_Int32   nX; // Zoom/ScaleX
    sal_Int32   nY; // Zoom/ScaleY

protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsZoom( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsZoom() {}

    sal_Bool    operator==( const SdOptionsZoom& rOpt ) const;

    void    GetScale( sal_Int32& rX, sal_Int32& rY ) const { Init(); rX = nX; rY = nY; }
    void    SetScale( sal_Int32 nInX, sal_Int32 nInY ) { if( nX != nInX || nY != nInY ) { OptionsChanged(); nX = nInX; nY = nInY; } }
};

class SdOptionsGrid : public SdOptionsGeneric, public SvxOptionsGrid
{
protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsGrid( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsGrid();

    void    SetDefaults();
    sal_Bool    operator==( const SdOptionsGrid& rOpt ) const;

    sal_uInt32  GetFldDrawX() const { Init(); return SvxOptionsGrid::GetFldDrawX(); }
    sal_uInt32  GetFldDivisionX() const { Init(); return SvxOptionsGrid::GetFldDivisionX(); }
    sal_uInt32  GetFldDrawY() const { Init(); return SvxOptionsGrid::GetFldDrawY(); }
    sal_uInt32  GetFldDivisionY() const { Init(); return SvxOptionsGrid::GetFldDivisionY(); }
    sal_uInt32  GetFldSnapX() const { Init(); return SvxOptionsGrid::GetFldSnapX(); }
    sal_uInt32  GetFldSnapY() const { Init(); return SvxOptionsGrid::GetFldSnapY(); }
    sal_Bool    IsUseGridSnap() const { Init(); return SvxOptionsGrid::GetUseGridSnap(); }
    sal_Bool    IsSynchronize() const { Init(); return SvxOptionsGrid::GetSynchronize(); }
    sal_Bool    IsGridVisible() const { Init(); return SvxOptionsGrid::GetGridVisible(); }
    sal_Bool    IsEqualGrid() const { Init(); return SvxOptionsGrid::GetEqualGrid(); }

    void    SetFldDrawX( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDrawX() ) { OptionsChanged(); SvxOptionsGrid::SetFldDrawX( nSet ); } }
    void    SetFldDivisionX( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDivisionX() ) { OptionsChanged(); SvxOptionsGrid::SetFldDivisionX( nSet ); } }
    void    SetFldDrawY( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDrawY() ) { OptionsChanged(); SvxOptionsGrid::SetFldDrawY( nSet ); } }
    void    SetFldDivisionY( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldDivisionY() ) { OptionsChanged(); SvxOptionsGrid::SetFldDivisionY( nSet ); } }
    void    SetFldSnapX( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldSnapX() ) { OptionsChanged(); SvxOptionsGrid::SetFldSnapX( nSet ); } }
    void    SetFldSnapY( sal_uInt32 nSet ) { if( nSet != SvxOptionsGrid::GetFldSnapY() ) { OptionsChanged(); SvxOptionsGrid::SetFldSnapY( nSet ); } }
    void    SetUseGridSnap( sal_Bool bSet ) { if( bSet != SvxOptionsGrid::GetUseGridSnap() ) { OptionsChanged(); SvxOptionsGrid::SetUseGridSnap( bSet ); } }
    void    SetSynchronize( sal_Bool bSet ) { if( bSet != SvxOptionsGrid::GetSynchronize() ) { OptionsChanged(); SvxOptionsGrid::SetSynchronize( bSet ); } }
    void    SetGridVisible( sal_Bool bSet ) { if( bSet != SvxOptionsGrid::GetGridVisible() ) { OptionsChanged(); SvxOptionsGrid::SetGridVisible( bSet ); } }
    void    SetEqualGrid( sal_Bool bSet ) { if( bSet != SvxOptionsGrid::GetEqualGrid() ) { OptionsChanged(); SvxOptionsGrid::SetEqualGrid( bSet ); } }
};

class SdOptionsGridItem : public SvxGridItem
{

public:
                            SdOptionsGridItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    void                    SetOptions( SdOptions* pOpts ) const;
};

class SD_DLLPUBLIC SdOptionsPrint : public SdOptionsGeneric
{
private:

    sal_Bool    bDraw               : 1;    // Print/Content/Drawing
    sal_Bool    bNotes              : 1;    // Print/Content/Note
    sal_Bool    bHandout            : 1;    // Print/Content/Handout
    sal_Bool    bOutline            : 1;    // Print/Content/Outline
    sal_Bool    bDate               : 1;    // Print/Other/Date
    sal_Bool    bTime               : 1;    // Print/Other/Time
    sal_Bool    bPagename           : 1;    // Print/Other/PageName
    sal_Bool    bHiddenPages        : 1;    // Print/Other/HiddenPage
    sal_Bool    bPagesize           : 1;    // Print/Page/PageSize
    sal_Bool    bPagetile           : 1;    // Print/Page/PageTile
    sal_Bool    bWarningPrinter     : 1;    //  These flags you get
    sal_Bool    bWarningSize        : 1;    //  from the common options,
    sal_Bool    bWarningOrientation : 1;    //  currently org.openoffice.Office.Common.xml (class OfaMiscCfg ; sfx2/misccfg.hxx )
    sal_Bool    bBooklet            : 1;    // Print/Page/Booklet
    sal_Bool    bFront              : 1;    // Print/Page/BookletFront
    sal_Bool    bBack               : 1;    // Print/Page/BookletFront
    sal_Bool    bCutPage            : 1;    // NOT persistent !!!
    sal_Bool    bPaperbin           : 1;    // Print/Other/FromPrinterSetup
    sal_Bool    mbHandoutHorizontal : 1;    // Order Page previews on Handout Pages horizontal
    sal_uInt16  mnHandoutPages;             // Number of page previews on handout page (only 1/2/4/6/9 are supported)
    sal_uInt16  nQuality;                   // Print/Other/Quality

protected:

    virtual void GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const;
    virtual sal_Bool ReadData( const ::com::sun::star::uno::Any* pValues );
    virtual sal_Bool WriteData( ::com::sun::star::uno::Any* pValues ) const;

public:

            SdOptionsPrint( sal_uInt16 nConfigId, sal_Bool bUseConfig );
            virtual ~SdOptionsPrint() {}

    sal_Bool    operator==( const SdOptionsPrint& rOpt ) const;

    sal_Bool    IsDraw() const { Init(); return (sal_Bool) bDraw; }
    sal_Bool    IsNotes() const { Init(); return (sal_Bool) bNotes; }
    sal_Bool    IsHandout() const { Init(); return (sal_Bool) bHandout; }
    sal_Bool    IsOutline() const { Init(); return (sal_Bool) bOutline; }
    sal_Bool    IsDate() const { Init(); return (sal_Bool) bDate; }
    sal_Bool    IsTime() const { Init(); return (sal_Bool) bTime; }
    sal_Bool    IsPagename() const { Init(); return (sal_Bool) bPagename; }
    sal_Bool    IsHiddenPages() const { Init(); return (sal_Bool) bHiddenPages; }
    sal_Bool    IsPagesize() const { Init(); return (sal_Bool) bPagesize; }
    sal_Bool    IsPagetile() const { Init(); return (sal_Bool) bPagetile; }
    sal_Bool    IsWarningPrinter() const { Init(); return (sal_Bool) bWarningPrinter; }
    sal_Bool    IsWarningSize() const { Init(); return (sal_Bool) bWarningSize; }
    sal_Bool    IsWarningOrientation() const { Init(); return (sal_Bool) bWarningOrientation; }
    sal_Bool    IsBooklet() const { Init(); return (sal_Bool) bBooklet; }
    sal_Bool    IsFrontPage() const { Init(); return (sal_Bool) bFront; }
    sal_Bool    IsBackPage() const { Init(); return (sal_Bool) bBack; }
    sal_Bool    IsCutPage() const { Init(); return (sal_Bool) bCutPage; }
    sal_Bool    IsPaperbin() const { Init(); return (sal_Bool) bPaperbin; }
    sal_uInt16  GetOutputQuality() const { Init(); return nQuality; }
    sal_Bool    IsHandoutHorizontal() const { Init(); return mbHandoutHorizontal; }
    sal_uInt16  GetHandoutPages() const { Init(); return mnHandoutPages; }

    void    SetDraw( sal_Bool bOn = sal_True ) { if( bDraw != bOn ) { OptionsChanged(); bDraw = bOn; } }
    void    SetNotes( sal_Bool bOn = sal_True ) { if( bNotes != bOn ) { OptionsChanged(); bNotes = bOn; } }
    void    SetHandout( sal_Bool bOn = sal_True ) { if( bHandout != bOn ) { OptionsChanged(); bHandout = bOn; } }
    void    SetOutline( sal_Bool bOn = sal_True ) { if( bOutline != bOn ) { OptionsChanged(); bOutline = bOn; } }
    void    SetDate( sal_Bool bOn = sal_True ) { if( bDate != bOn ) { OptionsChanged(); bDate = bOn; } }
    void    SetTime( sal_Bool bOn = sal_True ) { if( bTime != bOn ) { OptionsChanged(); bTime = bOn; } }
    void    SetPagename( sal_Bool bOn = sal_True ) { if( bPagename != bOn ) { OptionsChanged(); bPagename = bOn; } }
    void    SetHiddenPages( sal_Bool bOn = sal_True ) { if( bHiddenPages != bOn ) { OptionsChanged(); bHiddenPages = bOn; } }
    void    SetPagesize( sal_Bool bOn = sal_True ) { if( bPagesize != bOn ) { OptionsChanged(); bPagesize = bOn; } }
    void    SetPagetile( sal_Bool bOn = sal_True ) { if( bPagetile != bOn ) { OptionsChanged(); bPagetile = bOn; } }
    void    SetWarningPrinter( sal_Bool bOn = sal_True ) { if( bWarningPrinter != bOn ) { OptionsChanged(); bWarningPrinter = bOn; } }
    void    SetWarningSize( sal_Bool bOn = sal_True ) { if( bWarningSize != bOn ) { OptionsChanged(); bWarningSize = bOn; } }
    void    SetWarningOrientation( sal_Bool bOn = sal_True ) { if( bWarningOrientation != bOn ) { OptionsChanged(); bWarningOrientation = bOn; } }
    void    SetBooklet( sal_Bool bOn = sal_True ) { if( bBooklet != bOn ) { OptionsChanged(); bBooklet = bOn; } }
    void    SetFrontPage( sal_Bool bOn = sal_True ) { if( bFront != bOn ) { OptionsChanged(); bFront = bOn; } }
    void    SetBackPage( sal_Bool bOn = sal_True ) { if( bBack != bOn ) { OptionsChanged(); bBack = bOn; } }
    void    SetCutPage( sal_Bool bOn = sal_True ) { if( bCutPage != bOn ) { OptionsChanged(); bCutPage = bOn; } }
    void    SetPaperbin( sal_Bool bOn = sal_True ) { if( bPaperbin != bOn ) { OptionsChanged(); bPaperbin = bOn; } }
    void    SetOutputQuality( sal_uInt16 nInQuality ) { if( nQuality != nInQuality ) { OptionsChanged(); nQuality = nInQuality; } }
    void    SetHandoutHorizontal( sal_Bool bHandoutHorizontal ) { if( mbHandoutHorizontal != bHandoutHorizontal ) { OptionsChanged(); mbHandoutHorizontal = bHandoutHorizontal; } }
    void    SetHandoutPages( sal_uInt16 nHandoutPages ) { if( nHandoutPages != mnHandoutPages ) { OptionsChanged(); mnHandoutPages = nHandoutPages; } }
};

class SD_DLLPUBLIC SdOptionsPrintItem : public SfxPoolItem
{
public:

                            SdOptionsPrintItem( sal_uInt16 nWhich);
                            SdOptionsPrintItem( sal_uInt16 nWhich, SdOptions* pOpts, ::sd::FrameView* pView = NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            operator==( const SfxPoolItem& ) const;

    void                    SetOptions( SdOptions* pOpts ) const;

    SdOptionsPrint&         GetOptionsPrint() { return maOptionsPrint; }
    const SdOptionsPrint&   GetOptionsPrint() const { return maOptionsPrint; }
private:
    SdOptionsPrint  maOptionsPrint;
};

class SdOptions : public SdOptionsLayout, public SdOptionsContents,
                  public SdOptionsMisc, public SdOptionsSnap,
                  public SdOptionsZoom, public SdOptionsGrid,
                  public SdOptionsPrint
{
public:

                        SdOptions( sal_uInt16 nConfigId );
                        virtual ~SdOptions();

    void                StoreConfig( sal_uLong nOptionRange = SD_OPTIONS_ALL );
};

#endif // INCLUDED_SD_SOURCE_UI_INC_OPTSITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: optsitem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:40 $
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

#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif
#ifndef _SVX_OPTGRID_HXX //autogen
#include <svx/optgrid.hxx>
#endif

#ifndef _OPTSITE2_HXX
#include "optsite2.hxx"
#endif

#define SD_OPTIONS_ALL  0

#define SD_OPTIONS_SNAP 2
#define SD_OPTIONS_GRID 3
#define SD_OPTIONS_LAYOUT 4
#define SD_OPTIONS_CONTENTS 5
#define SD_OPTIONS_MISC 6

class SfxConfigItem;
class FrameView;

// class SdOptionsLayout ----------------------------------------------

class SdOptionsLayout
{
protected:
    BOOL    bRuler              : 1;
    BOOL    bMoveOutline        : 1;
    BOOL    bDragStripes        : 1;
    BOOL    bHandlesBezier      : 1;
    BOOL    bHelplines          : 1;
    UINT16  nMetric;

public:
    SdOptionsLayout();
    ~SdOptionsLayout() {}

    BOOL    IsRulerVisible() const       { return (BOOL) bRuler; }
    BOOL    IsMoveOutline() const        { return (BOOL) bMoveOutline; }
    BOOL    IsDragStripes() const        { return (BOOL) bDragStripes; }
    BOOL    IsHandlesBezier() const      { return (BOOL) bHandlesBezier; }
    BOOL    IsHelplines() const          { return (BOOL) bHelplines; }
    UINT16  GetMetric() const;

    void    SetRulerVisible( BOOL bOn = TRUE )       { bRuler = bOn; }
    void    SetMoveOutline( BOOL bOn = TRUE )        { bMoveOutline = bOn; }
    void    SetDragStripes( BOOL bOn = TRUE )        { bDragStripes = bOn; }
    void    SetHandlesBezier( BOOL bOn = TRUE )      { bHandlesBezier = bOn; }
    void    SetHelplines( BOOL bOn = TRUE )          { bHelplines = bOn; }
    void    SetMetric( UINT16 nInMetric )            { nMetric = nInMetric; }
};

// class SdOptionsContents ----------------------------------------------

class SdOptionsContents
{
protected:
    BOOL    bExternGraphic      : 1;
    BOOL    bOutlineMode        : 1;
    BOOL    bHairlineMode       : 1;
    BOOL    bNoText             : 1;
    BOOL    bSolidDragging      : 1;
    BOOL    bSolidMarkHdl       : 1;

public:
    SdOptionsContents();
    ~SdOptionsContents() {}

    BOOL    IsExternGraphic() const      { return (BOOL) bExternGraphic; }
    BOOL    IsOutlineMode() const        { return (BOOL) bOutlineMode; }
    BOOL    IsHairlineMode() const       { return (BOOL) bHairlineMode; }
    BOOL    IsNoText() const             { return (BOOL) bNoText; }
    BOOL    IsSolidDragging() const      { return (BOOL) bSolidDragging; }
    BOOL    IsSolidMarkHdl() const       { return (BOOL) bSolidMarkHdl; }

    void    SetExternGraphic( BOOL bOn = TRUE )      { bExternGraphic = bOn; }
    void    SetOutlineMode( BOOL bOn = TRUE )        { bOutlineMode = bOn; }
    void    SetHairlineMode( BOOL bOn = TRUE )       { bHairlineMode = bOn; }
    void    SetNoText( BOOL bOn = TRUE )             { bNoText = bOn; }
    void    SetSolidDragging( BOOL bOn = TRUE )      { bSolidDragging = bOn; }
    void    SetSolidMarkHdl( BOOL bOn = TRUE )       { bSolidMarkHdl = bOn; }
};

// class SdOptionsMisc ----------------------------------------------

class SdOptionsMisc
{
protected:
    BOOL    bStartWithTemplate  : 1;
    BOOL    bStartWithLayout    : 1;
    BOOL    bMarkedHitMovesAlways : 1;
    BOOL    bMoveOnlyDragging   : 1;
    BOOL    bCrookNoContortion  : 1;
    BOOL    bQuickEdit          : 1;
    BOOL    bMasterPageCache    : 1;
    BOOL    bDragWithCopy       : 1;
    BOOL    bPickThrough        : 1;
    BOOL    bBigHandles         : 1;
    BOOL    bDoubleClickTextEdit: 1;    // Textmodus nach Doppelklick
    BOOL    bClickChangeRotation: 1;    // Einfachklick wechselt Selektions-/Rotationsmodus
    BOOL    bStartWithActualPage: 1;    // Startet die Praesentation immer von der aktuellen Seite
    ULONG   nPreviewQuality;

public:
    SdOptionsMisc();
    ~SdOptionsMisc() {}

    BOOL    IsStartWithTemplate() const  { return (BOOL) bStartWithTemplate; }
    BOOL    IsStartWithLayout() const    { return (BOOL) bStartWithLayout; }
    BOOL    IsMarkedHitMovesAlways() const { return (BOOL) bMarkedHitMovesAlways; }
    BOOL    IsMoveOnlyDragging() const   { return (BOOL) bMoveOnlyDragging; }
    BOOL    IsCrookNoContortion() const  { return (BOOL) bCrookNoContortion; }
    BOOL    IsQuickEdit() const          { return (BOOL) bQuickEdit; }
    BOOL    IsMasterPagePaintCaching() const { return (BOOL) bMasterPageCache; }
    BOOL    IsDragWithCopy() const       { return (BOOL) bDragWithCopy; }
    BOOL    IsPickThrough() const        { return (BOOL) bPickThrough; }
    BOOL    IsBigHandles() const         { return (BOOL) bBigHandles; }
    BOOL    IsDoubleClickTextEdit() const{ return (BOOL) bDoubleClickTextEdit; }
    BOOL    IsClickChangeRotation() const{ return (BOOL) bClickChangeRotation; }
    BOOL    IsStartWithActualPage() const{ return (BOOL) bStartWithActualPage; }
    ULONG   GetPreviewQuality() const    { return nPreviewQuality; }

    // Template und Layout laufen z.Z. synchron!
    void    SetStartWithTemplate( BOOL bOn = TRUE )  { bStartWithTemplate = bOn;
                                                       bStartWithLayout = bOn; }
    void    SetStartWithLayout( BOOL bOn = TRUE )    { bStartWithTemplate = bOn;
                                                       bStartWithLayout = bOn; }
    void    SetMarkedHitMovesAlways( BOOL bOn = TRUE ) { bMarkedHitMovesAlways = bOn; }
    void    SetMoveOnlyDragging( BOOL bOn = TRUE )   { bMoveOnlyDragging = bOn; }
    void    SetCrookNoContortion( BOOL bOn = TRUE )  { bCrookNoContortion = bOn; }
    void    SetQuickEdit( BOOL bOn = TRUE )          { bQuickEdit = bOn; }
    void    SetMasterPagePaintCaching( BOOL bOn = TRUE ) { bMasterPageCache = bOn; }
    void    SetDragWithCopy( BOOL bOn = TRUE )       { bDragWithCopy = bOn; }
    void    SetPickThrough( BOOL bOn = TRUE )        { bPickThrough = bOn; }
    void    SetBigHandles( BOOL bOn = TRUE )         { bBigHandles = bOn; }
    void    SetDoubleClickTextEdit( BOOL bOn = TRUE ){ bDoubleClickTextEdit = bOn; }
    void    SetClickChangeRotation( BOOL bOn = TRUE ){ bClickChangeRotation = bOn; }
    void    SetStartWithActualPage( BOOL bOn = TRUE ){ bStartWithActualPage = bOn; }
    void    SetPreviewQuality( ULONG nQual )         { nPreviewQuality = nQual; }

};

// class SdOptionsSnap ----------------------------------------------

class SdOptionsSnap
{
protected:
    BOOL    bSnapHelplines      : 1;
    BOOL    bSnapBorder         : 1;
    BOOL    bSnapFrame          : 1;
    BOOL    bSnapPoints         : 1;
    BOOL    bOrtho              : 1;
    BOOL    bBigOrtho           : 1;
    BOOL    bRotate             : 1;
    INT16   nSnapArea;
    INT16   nAngle;
    INT16   nBezAngle;

public:
    SdOptionsSnap();
    ~SdOptionsSnap() {}

    BOOL    IsSnapHelplines() const      { return (BOOL) bSnapHelplines; }
    BOOL    IsSnapBorder() const         { return (BOOL) bSnapBorder; }
    BOOL    IsSnapFrame() const          { return (BOOL) bSnapFrame; }
    BOOL    IsSnapPoints() const         { return (BOOL) bSnapPoints; }
    BOOL    IsOrtho() const              { return (BOOL) bOrtho; }
    BOOL    IsBigOrtho() const           { return (BOOL) bBigOrtho; }
    BOOL    IsRotate() const             { return (BOOL) bRotate; }
    INT16   GetSnapArea() const          { return nSnapArea; }
    INT16   GetAngle() const             { return nAngle; }
    INT16   GetEliminatePolyPointLimitAngle() const { return nBezAngle; }

    void    SetSnapHelplines( BOOL bOn = TRUE )      { bSnapHelplines = bOn; }
    void    SetSnapBorder( BOOL bOn = TRUE )         { bSnapBorder = bOn; }
    void    SetSnapFrame( BOOL bOn = TRUE )          { bSnapFrame = bOn; }
    void    SetSnapPoints( BOOL bOn = TRUE )         { bSnapPoints = bOn; }
    void    SetOrtho( BOOL bOn = TRUE )              { bOrtho = bOn; }
    void    SetBigOrtho( BOOL bOn = TRUE )           { bBigOrtho = bOn; }
    void    SetRotate( BOOL bOn = TRUE )             { bRotate = bOn; }
    void    SetSnapArea( INT16 nIn )                 { nSnapArea = nIn; }
    void    SetAngle( INT16 nIn )                    { nAngle = nIn; }
    void    SetEliminatePolyPointLimitAngle( INT16 nIn ) { nBezAngle = nIn; }
};

// class SdOptions --------------------------------------------------

class SdOptions : public SfxConfigItem,
                  public SdOptionsLayout, public SdOptionsContents,
                  public SdOptionsMisc, public SdOptionsSnap,
                  public SdOptionsPrint, public SvxOptionsGrid
{
private:
    UINT16          nDefTab;
    INT32           nX;
    INT32           nY;
    USHORT          nConfigId;

            void    Init();
    virtual int     Load( SvStream& );
    virtual BOOL    Store( SvStream& );
    virtual void    UseDefault();

public:
    SdOptions(USHORT nCfgId);
    ~SdOptions() {}

    UINT16      GetDefTab() const { return nDefTab; }
    void        GetScale( INT32& rX, INT32& rY ) { rX = nX; rY = nY; }

    void    SetDefTab( UINT16 nTab ) { nDefTab = nTab; }
    void    SetScale( INT32 nInX, INT32 nInY ) { nX = nInX; nY = nInY; }
};

// class SdOptionsItem --------------------------------------------------

#ifndef _SD_OPTIONS_ITEM
#define _SD_OPTIONS_ITEM

class SdOptionsLayoutItem : public SfxPoolItem, public SdOptionsLayout
{

public:
    SdOptionsLayoutItem( USHORT nWhich);
    SdOptionsLayoutItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView=NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    SetOptions( SdOptions* pOpts ) const;

};

class SdOptionsContentsItem : public SfxPoolItem, public SdOptionsContents
{

public:
    SdOptionsContentsItem( USHORT nWhich);
    SdOptionsContentsItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView=NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    SetOptions( SdOptions* pOpts ) const;

};

class SdOptionsMiscItem : public SfxPoolItem, public SdOptionsMisc
{

public:
    SdOptionsMiscItem( USHORT nWhich);
    SdOptionsMiscItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView=NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    SetOptions( SdOptions* pOpts ) const;

};

class SdOptionsSnapItem : public SfxPoolItem, public SdOptionsSnap
{

public:
    SdOptionsSnapItem( USHORT nWhich);
    SdOptionsSnapItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView=NULL );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    void                    SetOptions( SdOptions* pOpts ) const;

};

#endif

// class SdOptionsGridItem ----------------------------------------------

#ifndef _SD_OPTIONS_GRID_ITEM
#define _SD_OPTIONS_GRID_ITEM

class SdOptionsGridItem : public SvxGridItem
{

public:
    SdOptionsGridItem( USHORT nWhich );
    SdOptionsGridItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView=NULL );

    void                    SetOptions( SdOptions* pOpts ) const;
};

#endif


#endif // _SD_OPTSITEM_HXX


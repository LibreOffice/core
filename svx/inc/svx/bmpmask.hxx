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
#ifndef _BMPMASK_HXX_
#define _BMPMASK_HXX_

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/graph.hxx>
#include "svx/svxdllapi.h"

#include <svx/dlgctrl.hxx>

/*************************************************************************
|*
|*  class SvxBmpMaskSelectItem
|*
\************************************************************************/
class SvxBmpMask;

class SvxBmpMaskSelectItem : public SfxControllerItem
{
private:
    SvxBmpMask  &rBmpMask;

protected:
    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState );

public:
    SvxBmpMaskSelectItem( sal_uInt16 nId, SvxBmpMask& rMask,
                          SfxBindings& rBindings );
};

/*************************************************************************
|*
|* Derivative from SfxChildWindow as 'container' for Float
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxBmpMaskChildWindow : public SfxChildWindow
{
 public:
                            SvxBmpMaskChildWindow( Window*,
                                                   sal_uInt16, SfxBindings*,
                                                   SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW_WITHID(SvxBmpMaskChildWindow);
};

/*************************************************************************
|*
|*  class SvxBmpMask
|*
\************************************************************************/

class MaskData;
class MaskSet;
class ColorWindow;

class SVX_DLLPUBLIC SvxBmpMask : public SfxDockingWindow
{
    friend class MaskData;
    friend class MaskSet;

    Size                aLastSize;
    ToolBox             aTbxPipette;
    ColorWindow*        pCtlPipette;
    PushButton          aBtnExec;
    FixedLine           aGrpQ;

    FixedText           aFt1;
    FixedText           aFt2;
    FixedText           aFt3;
    CheckBox            aCbx1;
    MaskSet*            pQSet1;
    MetricField         aSp1;
    ColorLB             aLbColor1;

    CheckBox            aCbx2;
    MaskSet*            pQSet2;
    MetricField         aSp2;
    ColorLB             aLbColor2;

    CheckBox            aCbx3;
    MaskSet*            pQSet3;
    MetricField         aSp3;
    ColorLB             aLbColor3;

    CheckBox            aCbx4;
    MaskSet*            pQSet4;
    MetricField         aSp4;
    ColorLB             aLbColor4;

    MaskData*           pData;
    CheckBox            aCbxTrans;
    ColorLB             aLbColorTrans;

    XColorListRef       pColLst;
    Color               aPipetteColor;
    SvxBmpMaskSelectItem aSelItem;

    Image               maImgPipette;

    virtual sal_Bool        Close();

#ifdef BMPMASK_PRIVATE

    sal_uInt16              InitColorArrays( Color* pSrcCols, Color* pDstCols,
                                         sal_uIntPtr* pTols );

    Bitmap              ImpMask( const Bitmap& rBitmap );
    GDIMetaFile         ImpMask( const GDIMetaFile& rMtf );
    Animation           ImpMask( const Animation& rAnimation );
    BitmapEx            ImpMaskTransparent( const BitmapEx& rBitmapEx,
                                            const Color& rColor,
                                            const long nTol );
    BitmapEx            ImpReplaceTransparency( const BitmapEx& rBmpEx,
                                                const Color& rColor );
    Animation           ImpReplaceTransparency( const Animation& rAnim,
                                                const Color& rColor );
    GDIMetaFile         ImpReplaceTransparency( const GDIMetaFile& rMtf,
                                                const Color& rColor );

#endif // BMPMASK_PRIVATE

public:

                        SvxBmpMask( SfxBindings *pBindinx,
                                    SfxChildWindow *pCW,
                                    Window* pParent,
                                    const ResId& rResId );
                        ~SvxBmpMask();

    void                SetColor( const Color& rColor );
    void                PipetteClicked();

    sal_Bool            NeedsColorList() const;
    void                SetColorList( const XColorListRef &pColorList );

    void                SetExecState( sal_Bool bEnable );

    Graphic             Mask( const Graphic& rGraphic );

    sal_Bool                IsEyedropping() const;

    void                onSelect( MaskSet* pSet );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void ApplyStyle();

private:

    /** Set names for accessible objects.  This is necessary for objects
        like the source color checkboxes which initially have no name and
        for which the description would be read by AT.
    */
    SVX_DLLPRIVATE void SetAccessibleNames (void);
};

#endif // _BMPMASK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

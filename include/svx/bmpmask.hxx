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
#ifndef INCLUDED_SVX_BMPMASK_HXX
#define INCLUDED_SVX_BMPMASK_HXX

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/graph.hxx>
#include <svx/svxdllapi.h>

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
                               const SfxPoolItem* pState ) SAL_OVERRIDE;

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

    virtual bool        Close() SAL_OVERRIDE;

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

    bool                NeedsColorList() const;
    void                SetColorList( const XColorListRef &pColorList );

    void                SetExecState( bool bEnable );

    Graphic             Mask( const Graphic& rGraphic );

    bool                IsEyedropping() const;

    void                onSelect( MaskSet* pSet );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    void ApplyStyle();

private:

    /** Set names for accessible objects.  This is necessary for objects
        like the source color checkboxes which initially have no name and
        for which the description would be read by AT.
    */
    SVX_DLLPRIVATE void SetAccessibleNames (void);
};

#endif // INCLUDED_SVX_BMPMASK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

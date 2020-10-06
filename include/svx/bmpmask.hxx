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

#include <sal/types.h>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <tools/color.hxx>
#include <vcl/animate/Animation.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <memory>

namespace vcl { class Window; }
namespace weld { class CustomWeld; }

class SfxBindings;
class SfxModule;

/*************************************************************************
|*
|*  class SvxBmpMaskSelectItem
|*
\************************************************************************/
class SvxBmpMask;

class SvxBmpMaskSelectItem final : public SfxControllerItem
{
    SvxBmpMask  &rBmpMask;

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState ) override;

public:
    SvxBmpMaskSelectItem( SvxBmpMask& rMask,
                          SfxBindings& rBindings );
};

/*************************************************************************
|*
|* Derivative from SfxChildWindow as 'container' for Float
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxBmpMaskChildWindow final : public SfxChildWindow
{
 public:
                            SvxBmpMaskChildWindow( vcl::Window*,
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
class BmpColorWindow;
class ColorListBox;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxBmpMask : public SfxDockingWindow
{
    friend class MaskData;
    friend class MaskSet;

    std::unique_ptr<weld::Toolbar> m_xTbxPipette;
    std::unique_ptr<BmpColorWindow> m_xCtlPipette;
    std::unique_ptr<weld::CustomWeld> m_xCtlPipetteWin;
    std::unique_ptr<weld::Button> m_xBtnExec;

    std::unique_ptr<weld::CheckButton> m_xCbx1;
    std::unique_ptr<MaskSet> m_xQSet1;
    std::unique_ptr<weld::CustomWeld> m_xQSetWin1;
    std::unique_ptr<weld::MetricSpinButton> m_xSp1;
    std::unique_ptr<ColorListBox> m_xLbColor1;

    std::unique_ptr<weld::CheckButton> m_xCbx2;
    std::unique_ptr<MaskSet> m_xQSet2;
    std::unique_ptr<weld::CustomWeld> m_xQSetWin2;
    std::unique_ptr<weld::MetricSpinButton> m_xSp2;
    std::unique_ptr<ColorListBox> m_xLbColor2;

    std::unique_ptr<weld::CheckButton> m_xCbx3;
    std::unique_ptr<MaskSet> m_xQSet3;
    std::unique_ptr<weld::CustomWeld> m_xQSetWin3;
    std::unique_ptr<weld::MetricSpinButton> m_xSp3;
    std::unique_ptr<ColorListBox> m_xLbColor3;

    std::unique_ptr<weld::CheckButton> m_xCbx4;
    std::unique_ptr<MaskSet> m_xQSet4;
    std::unique_ptr<weld::CustomWeld> m_xQSetWin4;
    std::unique_ptr<weld::MetricSpinButton> m_xSp4;
    std::unique_ptr<ColorListBox> m_xLbColor4;

    std::unique_ptr<weld::CheckButton> m_xCbxTrans;
    std::unique_ptr<ColorListBox> m_xLbColorTrans;

    std::unique_ptr<MaskData> m_xData;

    Color               aPipetteColor;
    SvxBmpMaskSelectItem aSelItem;

    virtual bool        Close() override;

    sal_uInt16          InitColorArrays( Color* pSrcCols, Color* pDstCols,
                                         sal_uInt8* pTols );

    void                ImpMask( BitmapEx& rBitmap );
    GDIMetaFile         ImpMask( const GDIMetaFile& rMtf );
    Animation           ImpMask( const Animation& rAnimation );
    BitmapEx            ImpMaskTransparent( const BitmapEx& rBitmapEx,
                                            const Color& rColor,
                                            const sal_uInt8 nTol );

    GDIMetaFile         GetMetaFile(const Graphic& rGraphic);

    static Animation    ImpReplaceTransparency( const Animation& rAnim,
                                                const Color& rColor );
    static GDIMetaFile  ImpReplaceTransparency( const GDIMetaFile& rMtf,
                                                const Color& rColor );

public:

    SvxBmpMask(SfxBindings *pBindinx, SfxChildWindow *pCW, vcl::Window* pParent);
    virtual ~SvxBmpMask() override;
    virtual void dispose() override;

    void                SetColor( const Color& rColor );
    void                PipetteClicked();

    void                SetExecState( bool bEnable );

    Graphic             Mask( const Graphic& rGraphic );

    bool                IsEyedropping() const;

    void                onSelect( const MaskSet* pSet );

private:

    /** Set names for accessible objects.  This is necessary for objects
        like the source color checkboxes which initially have no name and
        for which the description would be read by AT.
    */
    SVX_DLLPRIVATE void SetAccessibleNames();
};

#endif // INCLUDED_SVX_BMPMASK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

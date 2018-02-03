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
#include <svx/xtable.hxx>
#include <vcl/color.hxx>
#include <vcl/animate.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

class CheckBox;
class ColorLB;
class MetricField;
class PushButton;
class SfxBindings;
class SfxModule;
class ToolBox;

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

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxBmpMaskChildWindow : public SfxChildWindow
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
class ColorWindow;
class SvxColorListBox;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxBmpMask : public SfxDockingWindow
{
    friend class MaskData;
    friend class MaskSet;

    VclPtr<ToolBox>            m_pTbxPipette;
    VclPtr<ColorWindow>        m_pCtlPipette;
    VclPtr<PushButton>         m_pBtnExec;

    VclPtr<CheckBox>           m_pCbx1;
    VclPtr<MaskSet>            m_pQSet1;
    VclPtr<MetricField>        m_pSp1;
    VclPtr<SvxColorListBox>    m_pLbColor1;

    VclPtr<CheckBox>           m_pCbx2;
    VclPtr<MaskSet>            m_pQSet2;
    VclPtr<MetricField>        m_pSp2;
    VclPtr<SvxColorListBox>    m_pLbColor2;

    VclPtr<CheckBox>           m_pCbx3;
    VclPtr<MaskSet>            m_pQSet3;
    VclPtr<MetricField>        m_pSp3;
    VclPtr<SvxColorListBox>    m_pLbColor3;

    VclPtr<CheckBox>           m_pCbx4;
    VclPtr<MaskSet>            m_pQSet4;
    VclPtr<MetricField>        m_pSp4;
    VclPtr<SvxColorListBox>    m_pLbColor4;

    MaskData*           pData;
    VclPtr<CheckBox>           m_pCbxTrans;
    VclPtr<SvxColorListBox>    m_pLbColorTrans;

    Color               aPipetteColor;
    SvxBmpMaskSelectItem aSelItem;

    virtual bool        Close() override;

    sal_uInt16          InitColorArrays( Color* pSrcCols, Color* pDstCols,
                                         sal_uInt8* pTols );

    Bitmap              ImpMask( const Bitmap& rBitmap );
    GDIMetaFile         ImpMask( const GDIMetaFile& rMtf );
    Animation           ImpMask( const Animation& rAnimation );
    BitmapEx            ImpMaskTransparent( const BitmapEx& rBitmapEx,
                                            const Color& rColor,
                                            const sal_uInt8 nTol );

    GDIMetaFile         GetMetaFile(const Graphic& rGraphic);

    static BitmapEx     ImpReplaceTransparency( const BitmapEx& rBmpEx,
                                                const Color& rColor );
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

    void                onSelect( MaskSet* pSet );

private:

    /** Set names for accessible objects.  This is necessary for objects
        like the source color checkboxes which initially have no name and
        for which the description would be read by AT.
    */
    SVX_DLLPRIVATE void SetAccessibleNames();
};

#endif // INCLUDED_SVX_BMPMASK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

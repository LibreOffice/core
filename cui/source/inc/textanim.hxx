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
#ifndef _SVX_TEXTANIM_HXX
#define _SVX_TEXTANIM_HXX

#include <vcl/field.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/svdattr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>

class SdrView;

/*************************************************************************
|*
|* Page for changing TextAnimations (running text etc.)
|*
\************************************************************************/

class SvxTextAnimationPage : public SfxTabPage
{
private:
    ListBox*             m_pLbEffect;
    VclBox*              m_pBoxDirection;
    PushButton*          m_pBtnUp;
    PushButton*          m_pBtnLeft;
    PushButton*          m_pBtnRight;
    PushButton*          m_pBtnDown;

    VclFrame*            m_pFlProperties;
    TriStateBox*         m_pTsbStartInside;
    TriStateBox*         m_pTsbStopInside;

    VclBox*              m_pBoxCount;
    TriStateBox*         m_pTsbEndless;
    NumericField*        m_pNumFldCount;

    TriStateBox*         m_pTsbPixel;
    MetricField*         m_pMtrFldAmount;

    TriStateBox*         m_pTsbAuto;
    MetricField*         m_pMtrFldDelay;

    const SfxItemSet&   rOutAttrs;
    SdrTextAniKind      eAniKind;
    FieldUnit           eFUnit;
    SfxMapUnit          eUnit;

    DECL_LINK( SelectEffectHdl_Impl, void * );
    DECL_LINK( ClickEndlessHdl_Impl, void * );
    DECL_LINK( ClickAutoHdl_Impl, void * );
    DECL_LINK( ClickPixelHdl_Impl, void * );
    DECL_LINK( ClickDirectionHdl_Impl, ImageButton * );

    void                SelectDirection( SdrTextAniDirection nValue );
    sal_uInt16              GetSelectedDirection();

public:
    SvxTextAnimationPage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxTextAnimationPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    void         Construct();
};

/*************************************************************************
|*
|* Text-Tab-Dialog
|*
\************************************************************************/
class SvxTextTabDialog : public SfxTabDialog
{
    sal_uInt16          m_nTextId;
    sal_uInt16          m_nTextAnimId;
private:
    const SdrView*      pView;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

            SvxTextTabDialog( Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pView );
            ~SvxTextTabDialog() {};
};


#endif // _SVX_TEXTANIM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

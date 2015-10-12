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
#ifndef INCLUDED_CUI_SOURCE_INC_TEXTANIM_HXX
#define INCLUDED_CUI_SOURCE_INC_TEXTANIM_HXX

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
    static const sal_uInt16     pRanges[];

    VclPtr<ListBox>             m_pLbEffect;
    VclPtr<VclBox>              m_pBoxDirection;
    VclPtr<PushButton>          m_pBtnUp;
    VclPtr<PushButton>          m_pBtnLeft;
    VclPtr<PushButton>          m_pBtnRight;
    VclPtr<PushButton>          m_pBtnDown;

    VclPtr<VclFrame>            m_pFlProperties;
    VclPtr<TriStateBox>         m_pTsbStartInside;
    VclPtr<TriStateBox>         m_pTsbStopInside;

    VclPtr<VclBox>              m_pBoxCount;
    VclPtr<TriStateBox>         m_pTsbEndless;
    VclPtr<NumericField>        m_pNumFldCount;

    VclPtr<TriStateBox>         m_pTsbPixel;
    VclPtr<MetricField>         m_pMtrFldAmount;

    VclPtr<TriStateBox>         m_pTsbAuto;
    VclPtr<MetricField>         m_pMtrFldDelay;

    const SfxItemSet&   rOutAttrs;
    SdrTextAniKind      eAniKind;
    FieldUnit           eFUnit;
    SfxMapUnit          eUnit;

    DECL_LINK_TYPED( SelectEffectHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ClickEndlessHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickAutoHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickPixelHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickDirectionHdl_Impl, Button*, void );

    void                SelectDirection( SdrTextAniDirection nValue );
    sal_uInt16              GetSelectedDirection();

public:
    SvxTextAnimationPage( vcl::Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxTextAnimationPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;
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

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

public:

            SvxTextTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pView );
};


#endif // INCLUDED_CUI_SOURCE_INC_TEXTANIM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

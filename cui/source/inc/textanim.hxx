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
    FixedLine           aFlEffect;
    FixedText           aFtEffects;
    ListBox             aLbEffect;
    FixedText           aFtDirection;
    ImageButton         aBtnUp;
    ImageButton         aBtnLeft;
    ImageButton         aBtnRight;
    ImageButton         aBtnDown;

    FixedLine           aFlProperties;
    TriStateBox         aTsbStartInside;
    TriStateBox         aTsbStopInside;

    FixedText           aFtCount;
    TriStateBox         aTsbEndless;
    NumericField        aNumFldCount;

    FixedText           aFtAmount;
    TriStateBox         aTsbPixel;
    MetricField         aMtrFldAmount;

    FixedText           aFtDelay;
    TriStateBox         aTsbAuto;
    MetricField         aMtrFldDelay;

    const SfxItemSet&   rOutAttrs;
    SdrTextAniKind      eAniKind;
    FieldUnit           eFUnit;
    SfxMapUnit          eUnit;

#ifdef _SVX_TEXTANIM_CXX
    DECL_LINK( SelectEffectHdl_Impl, void * );
    DECL_LINK( ClickEndlessHdl_Impl, void * );
    DECL_LINK( ClickAutoHdl_Impl, void * );
    DECL_LINK( ClickPixelHdl_Impl, void * );
    DECL_LINK( ClickDirectionHdl_Impl, ImageButton * );

    void                SelectDirection( SdrTextAniDirection nValue );
    sal_uInt16              GetSelectedDirection();
#endif

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

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_TEXTANIM_HXX
#define _SVX_TEXTANIM_HXX

// include ---------------------------------------------------------------

#include <vcl/field.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/svdattr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>

class SdrView;

/*************************************************************************
|*
|* Page zum "Andern von TextAnimationen (Lauftext etc.)
|*
\************************************************************************/

class SvxTextAnimationPage : public SfxTabPage
{
private:
    FixedLine           aFlEffect;
    FixedText           aFtEffects;
    ListBox             aLbEffect;
    //ValueSet          aCtlEffect;
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
    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

            SvxTextTabDialog( Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pView );
            ~SvxTextTabDialog() {};
};


#endif // _SVX_TEXTANIM_HXX


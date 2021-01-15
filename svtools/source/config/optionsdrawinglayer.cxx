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

#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>

//  namespaces

using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_START                  "Office.Common/Drawinglayer"
#define DEFAULT_OVERLAYBUFFER           true
#define DEFAULT_PAINTBUFFER             true
#define DEFAULT_STRIPE_COLOR_A          Color(0)
#define DEFAULT_STRIPE_COLOR_B          Color(16581375)
#define DEFAULT_STRIPE_LENGTH           4

// #i73602#
// #i74769#, #i75172# : Change default for Calc and Writer to True
#define DEFAULT_OVERLAYBUFFER_CALC          true
#define DEFAULT_OVERLAYBUFFER_WRITER        true
#define DEFAULT_OVERLAYBUFFER_DRAWIMPRESS   true

// #i74769#, #i75172#
#define DEFAULT_PAINTBUFFER_CALC            true
#define DEFAULT_PAINTBUFFER_WRITER          true
#define DEFAULT_PAINTBUFFER_DRAWIMPRESS     true

// #i4219#
#define DEFAULT_MAXIMUMPAPERWIDTH           600
#define DEFAULT_MAXIMUMPAPERHEIGHT          600
#define DEFAULT_MAXIMUMPAPERLEFTMARGIN      9999
#define DEFAULT_MAXIMUMPAPERRIGHTMARGIN     9999
#define DEFAULT_MAXIMUMPAPERTOPMARGIN       9999
#define DEFAULT_MAXIMUMPAPERBOTTOMMARGIN    9999

// primitives
#define DEFAULT_ANTIALIASING                        true
#define DEFAULT_SNAPHORVERLINESTODISCRETE           true
#define DEFAULT_SOLIDDRAGCREATE                     true
#define DEFAULT_RENDERDECORATEDTEXTDIRECT           true
#define DEFAULT_RENDERSIMPLETEXTDIRECT              true
#define DEFAULT_QUADRATIC3DRENDERLIMIT              1000000
#define DEFAULT_QUADRATICFORMCONTROLRENDERLIMIT     45000

// #i97672# selection settings
#define DEFAULT_TRANSPARENTSELECTION                true
#define DEFAULT_TRANSPARENTSELECTIONPERCENT         75
#define DEFAULT_SELECTIONMAXIMUMLUMINANCEPERCENT    70

#define PROPERTYNAME_OVERLAYBUFFER      OUString("OverlayBuffer"    )
#define PROPERTYNAME_PAINTBUFFER        OUString("PaintBuffer"      )
#define PROPERTYNAME_STRIPE_COLOR_A     OUString("StripeColorA"     )
#define PROPERTYNAME_STRIPE_COLOR_B     OUString("StripeColorB"     )
#define PROPERTYNAME_STRIPE_LENGTH      OUString("StripeLength"     )

// #i73602#
#define PROPERTYNAME_OVERLAYBUFFER_CALC         OUString("OverlayBuffer_Calc")
#define PROPERTYNAME_OVERLAYBUFFER_WRITER       OUString("OverlayBuffer_Writer")
#define PROPERTYNAME_OVERLAYBUFFER_DRAWIMPRESS  OUString("OverlayBuffer_DrawImpress")

// #i74769#, #i75172#
#define PROPERTYNAME_PAINTBUFFER_CALC           OUString("PaintBuffer_Calc")
#define PROPERTYNAME_PAINTBUFFER_WRITER         OUString("PaintBuffer_Writer")
#define PROPERTYNAME_PAINTBUFFER_DRAWIMPRESS    OUString("PaintBuffer_DrawImpress")

// #i4219#
#define PROPERTYNAME_MAXIMUMPAPERWIDTH OUString("MaximumPaperWidth")
#define PROPERTYNAME_MAXIMUMPAPERHEIGHT OUString("MaximumPaperHeight")
#define PROPERTYNAME_MAXIMUMPAPERLEFTMARGIN OUString("MaximumPaperLeftMargin")
#define PROPERTYNAME_MAXIMUMPAPERRIGHTMARGIN OUString("MaximumPaperRightMargin")
#define PROPERTYNAME_MAXIMUMPAPERTOPMARGIN OUString("MaximumPaperTopMargin")
#define PROPERTYNAME_MAXIMUMPAPERBOTTOMMARGIN OUString("MaximumPaperBottomMargin")

// primitives
#define PROPERTYNAME_ANTIALIASING OUString("AntiAliasing")
#define PROPERTYNAME_SNAPHORVERLINESTODISCRETE OUString("SnapHorVerLinesToDiscrete")
#define PROPERTYNAME_SOLIDDRAGCREATE OUString("SolidDragCreate")
#define PROPERTYNAME_RENDERDECORATEDTEXTDIRECT OUString("RenderDecoratedTextDirect")
#define PROPERTYNAME_RENDERSIMPLETEXTDIRECT OUString("RenderSimpleTextDirect")
#define PROPERTYNAME_QUADRATIC3DRENDERLIMIT OUString("Quadratic3DRenderLimit")
#define PROPERTYNAME_QUADRATICFORMCONTROLRENDERLIMIT OUString("QuadraticFormControlRenderLimit")

// #i97672# selection settings
#define PROPERTYNAME_TRANSPARENTSELECTION OUString("TransparentSelection")
#define PROPERTYNAME_TRANSPARENTSELECTIONPERCENT OUString("TransparentSelectionPercent")
#define PROPERTYNAME_SELECTIONMAXIMUMLUMINANCEPERCENT OUString("SelectionMaximumLuminancePercent")

#define PROPERTYHANDLE_OVERLAYBUFFER                0
#define PROPERTYHANDLE_PAINTBUFFER                  1
#define PROPERTYHANDLE_STRIPE_COLOR_A               2
#define PROPERTYHANDLE_STRIPE_COLOR_B               3
#define PROPERTYHANDLE_STRIPE_LENGTH                4

// #i73602#
#define PROPERTYHANDLE_OVERLAYBUFFER_CALC           5
#define PROPERTYHANDLE_OVERLAYBUFFER_WRITER         6
#define PROPERTYHANDLE_OVERLAYBUFFER_DRAWIMPRESS    7

// #i74769#, #i75172#
#define PROPERTYHANDLE_PAINTBUFFER_CALC             8
#define PROPERTYHANDLE_PAINTBUFFER_WRITER           9
#define PROPERTYHANDLE_PAINTBUFFER_DRAWIMPRESS      10

// #i4219#
#define PROPERTYHANDLE_MAXIMUMPAPERWIDTH            11
#define PROPERTYHANDLE_MAXIMUMPAPERHEIGHT           12
#define PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN       13
#define PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN      14
#define PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN        15
#define PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN     16

// primitives
#define PROPERTYHANDLE_ANTIALIASING                     17
#define PROPERTYHANDLE_SNAPHORVERLINESTODISCRETE        18
#define PROPERTYHANDLE_SOLIDDRAGCREATE                  19
#define PROPERTYHANDLE_RENDERDECORATEDTEXTDIRECT        20
#define PROPERTYHANDLE_RENDERSIMPLETEXTDIRECT           21
#define PROPERTYHANDLE_QUADRATIC3DRENDERLIMIT           22
#define PROPERTYHANDLE_QUADRATICFORMCONTROLRENDERLIMIT  23

// #i97672# selection settings
#define PROPERTYHANDLE_TRANSPARENTSELECTION             24
#define PROPERTYHANDLE_TRANSPARENTSELECTIONPERCENT      25
#define PROPERTYHANDLE_SELECTIONMAXIMUMLUMINANCEPERCENT 26

#define PROPERTYCOUNT                               27

class SvtOptionsDrawinglayer_Impl : public ConfigItem
{
public:
    SvtOptionsDrawinglayer_Impl();
    ~SvtOptionsDrawinglayer_Impl() override;

    virtual void Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    bool        IsOverlayBuffer() const { return m_bOverlayBuffer;}
    bool        IsPaintBuffer() const { return m_bPaintBuffer;}
    const Color& GetStripeColorA() const { return m_bStripeColorA;}
    const Color& GetStripeColorB() const { return m_bStripeColorB;}
    sal_uInt16  GetStripeLength() const { return m_nStripeLength;}

    // #i73602#
    bool        IsOverlayBuffer_Calc() const { return m_bOverlayBuffer_Calc;}
    bool        IsOverlayBuffer_Writer() const { return m_bOverlayBuffer_Writer;}
    bool        IsOverlayBuffer_DrawImpress() const { return m_bOverlayBuffer_DrawImpress;}

    // #i74769#, #i75172#
    bool        IsPaintBuffer_Calc() const { return m_bPaintBuffer_Calc;}
    bool        IsPaintBuffer_Writer() const { return m_bPaintBuffer_Writer;}
    bool        IsPaintBuffer_DrawImpress() const { return m_bPaintBuffer_DrawImpress;}

    // #i4219#
    sal_uInt32  GetMaximumPaperWidth() const { return m_nMaximumPaperWidth;}
    sal_uInt32  GetMaximumPaperHeight() const { return m_nMaximumPaperHeight;}
    sal_uInt32  GetMaximumPaperLeftMargin() const { return m_nMaximumPaperLeftMargin;}
    sal_uInt32  GetMaximumPaperRightMargin() const { return m_nMaximumPaperRightMargin;}
    sal_uInt32  GetMaximumPaperTopMargin() const { return m_nMaximumPaperTopMargin;}
    sal_uInt32  GetMaximumPaperBottomMargin() const { return m_nMaximumPaperBottomMargin;}

    // helper
    bool        IsAAPossibleOnThisSystem() const;

    // primitives
    bool        IsAntiAliasing() const { return m_bAntiAliasing;}
    bool        IsSnapHorVerLinesToDiscrete() const { return m_bSnapHorVerLinesToDiscrete;}
    bool        IsSolidDragCreate() const { return m_bSolidDragCreate;}
    bool        IsRenderDecoratedTextDirect() const { return m_bRenderDecoratedTextDirect;}
    bool        IsRenderSimpleTextDirect() const { return m_bRenderSimpleTextDirect;}
    sal_uInt32  GetQuadratic3DRenderLimit() const { return m_nQuadratic3DRenderLimit;}
    sal_uInt32  GetQuadraticFormControlRenderLimit() const { return m_nQuadraticFormControlRenderLimit;}

    void        SetAntiAliasing( bool bState );

    // #i97672# selection settings
    bool        IsTransparentSelection() const { return m_bTransparentSelection;}
    sal_uInt16  GetTransparentSelectionPercent() const { return m_nTransparentSelectionPercent;}
    sal_uInt16  GetSelectionMaximumLuminancePercent() const { return m_nSelectionMaximumLuminancePercent;}

//  private methods

private:
    virtual void ImplCommit() final override;

    static Sequence< OUString > impl_GetPropertyNames();

//  private member

private:

        bool        m_bOverlayBuffer;
        bool        m_bPaintBuffer;
        Color       m_bStripeColorA;
        Color       m_bStripeColorB;
        sal_uInt16  m_nStripeLength;

        // #i73602#
        bool        m_bOverlayBuffer_Calc;
        bool        m_bOverlayBuffer_Writer;
        bool        m_bOverlayBuffer_DrawImpress;

        // #i74769#, #i75172#
        bool        m_bPaintBuffer_Calc;
        bool        m_bPaintBuffer_Writer;
        bool        m_bPaintBuffer_DrawImpress;

        // #i4219#
        sal_uInt32  m_nMaximumPaperWidth;
        sal_uInt32  m_nMaximumPaperHeight;
        sal_uInt32  m_nMaximumPaperLeftMargin;
        sal_uInt32  m_nMaximumPaperRightMargin;
        sal_uInt32  m_nMaximumPaperTopMargin;
        sal_uInt32  m_nMaximumPaperBottomMargin;

        // primitives
        bool        m_bAntiAliasing;
        bool        m_bSnapHorVerLinesToDiscrete;
        bool        m_bSolidDragCreate;
        bool        m_bRenderDecoratedTextDirect;
        bool        m_bRenderSimpleTextDirect;
        sal_uInt32  m_nQuadratic3DRenderLimit;
        sal_uInt32  m_nQuadraticFormControlRenderLimit;

        // #i97672# selection settings
        sal_uInt16  m_nTransparentSelectionPercent;
        sal_uInt16  m_nSelectionMaximumLuminancePercent;
        bool        m_bTransparentSelection;

        // local values
        bool        m_bAllowAA : 1;
        bool        m_bAllowAAChecked : 1;
};

SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl() :
    ConfigItem( ROOTNODE_START  ),
    m_bOverlayBuffer( DEFAULT_OVERLAYBUFFER ),
    m_bPaintBuffer( DEFAULT_PAINTBUFFER ),
    m_bStripeColorA(DEFAULT_STRIPE_COLOR_A),
    m_bStripeColorB(DEFAULT_STRIPE_COLOR_B),
    m_nStripeLength(DEFAULT_STRIPE_LENGTH),

    // #i73602#
    m_bOverlayBuffer_Calc( DEFAULT_OVERLAYBUFFER_CALC ),
    m_bOverlayBuffer_Writer( DEFAULT_OVERLAYBUFFER_WRITER ),
    m_bOverlayBuffer_DrawImpress( DEFAULT_OVERLAYBUFFER_DRAWIMPRESS ),

    // #i74769#, #i75172#
    m_bPaintBuffer_Calc( DEFAULT_PAINTBUFFER_CALC ),
    m_bPaintBuffer_Writer( DEFAULT_PAINTBUFFER_WRITER ),
    m_bPaintBuffer_DrawImpress( DEFAULT_PAINTBUFFER_DRAWIMPRESS ),

    // #i4219#
    m_nMaximumPaperWidth(DEFAULT_MAXIMUMPAPERWIDTH),
    m_nMaximumPaperHeight(DEFAULT_MAXIMUMPAPERHEIGHT),
    m_nMaximumPaperLeftMargin(DEFAULT_MAXIMUMPAPERLEFTMARGIN),
    m_nMaximumPaperRightMargin(DEFAULT_MAXIMUMPAPERRIGHTMARGIN),
    m_nMaximumPaperTopMargin(DEFAULT_MAXIMUMPAPERTOPMARGIN),
    m_nMaximumPaperBottomMargin(DEFAULT_MAXIMUMPAPERBOTTOMMARGIN),

    // primitives
    m_bAntiAliasing(DEFAULT_ANTIALIASING),
    m_bSnapHorVerLinesToDiscrete(DEFAULT_SNAPHORVERLINESTODISCRETE),
    m_bSolidDragCreate(DEFAULT_SOLIDDRAGCREATE),
    m_bRenderDecoratedTextDirect(DEFAULT_RENDERDECORATEDTEXTDIRECT),
    m_bRenderSimpleTextDirect(DEFAULT_RENDERSIMPLETEXTDIRECT),
    m_nQuadratic3DRenderLimit(DEFAULT_QUADRATIC3DRENDERLIMIT),
    m_nQuadraticFormControlRenderLimit(DEFAULT_QUADRATICFORMCONTROLRENDERLIMIT),

    // #i97672# selection settings
    m_nTransparentSelectionPercent(DEFAULT_TRANSPARENTSELECTIONPERCENT),
    m_nSelectionMaximumLuminancePercent(DEFAULT_SELECTIONMAXIMUMLUMINANCEPERCENT),
    m_bTransparentSelection(DEFAULT_TRANSPARENTSELECTION),

    // local values
    m_bAllowAA(true),
    m_bAllowAAChecked(false)
{
    Sequence< OUString >    seqNames( impl_GetPropertyNames() );
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for(sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;

        switch( nProperty )
        {
            case PROPERTYHANDLE_OVERLAYBUFFER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\OverlayBuffer\"?" );
                seqValues[nProperty] >>= m_bOverlayBuffer;
            }
            break;

            case PROPERTYHANDLE_PAINTBUFFER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\PaintBuffer\"?" );
                seqValues[nProperty] >>= m_bPaintBuffer;
            }
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_A:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorA\"?" );
                Color nValue;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorA = nValue;
            }
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeColorB\"?" );
                Color nValue;
                seqValues[nProperty] >>= nValue;
                m_bStripeColorB = nValue;
            }
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\StripeLength\"?" );
                seqValues[nProperty] >>= m_nStripeLength;
            }
            break;

            // #i73602#
            case PROPERTYHANDLE_OVERLAYBUFFER_CALC:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\OverlayBuffer_Calc\"?" );
                seqValues[nProperty] >>= m_bOverlayBuffer_Calc;
            }
            break;

            case PROPERTYHANDLE_OVERLAYBUFFER_WRITER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\OverlayBuffer_Writer\"?" );
                seqValues[nProperty] >>= m_bOverlayBuffer_Writer;
            }
            break;

            case PROPERTYHANDLE_OVERLAYBUFFER_DRAWIMPRESS:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\OverlayBuffer_DrawImpress\"?" );
                seqValues[nProperty] >>= m_bOverlayBuffer_DrawImpress;
            }
            break;

            // #i74769#, #i75172#
            case PROPERTYHANDLE_PAINTBUFFER_CALC:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\PaintBuffer_Calc\"?" );
                seqValues[nProperty] >>= m_bPaintBuffer_Calc;
            }
            break;

            case PROPERTYHANDLE_PAINTBUFFER_WRITER:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\PaintBuffer_Writer\"?" );
                seqValues[nProperty] >>= m_bPaintBuffer_Writer;
            }
            break;

            case PROPERTYHANDLE_PAINTBUFFER_DRAWIMPRESS:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\PaintBuffer_DrawImpress\"?" );
                seqValues[nProperty] >>= m_bPaintBuffer_DrawImpress;
            }
            break;

            // #i4219#
            case PROPERTYHANDLE_MAXIMUMPAPERWIDTH:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperWidth\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperWidth;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERHEIGHT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperHeight\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperHeight;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperLeftMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperLeftMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperRightMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperRightMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperTopMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperTopMargin;
            }
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\MaximumPaperBottomMargin\"?" );
                seqValues[nProperty] >>= m_nMaximumPaperBottomMargin;
            }
            break;

            // primitives
            case PROPERTYHANDLE_ANTIALIASING:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\AntiAliasing\"?" );
                seqValues[nProperty] >>= m_bAntiAliasing;
            }
            break;

            // primitives
            case PROPERTYHANDLE_SNAPHORVERLINESTODISCRETE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\SnapHorVerLinesToDiscrete\"?" );
                seqValues[nProperty] >>= m_bSnapHorVerLinesToDiscrete;
            }
            break;

            case PROPERTYHANDLE_SOLIDDRAGCREATE:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\SolidDragCreate\"?" );
                seqValues[nProperty] >>= m_bSolidDragCreate;
            }
            break;

            case PROPERTYHANDLE_RENDERDECORATEDTEXTDIRECT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\RenderDecoratedTextDirect\"?" );
                seqValues[nProperty] >>= m_bRenderDecoratedTextDirect;
            }
            break;

            case PROPERTYHANDLE_RENDERSIMPLETEXTDIRECT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\RenderSimpleTextDirect\"?" );
                seqValues[nProperty] >>= m_bRenderSimpleTextDirect;
            }
            break;

            case PROPERTYHANDLE_QUADRATIC3DRENDERLIMIT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\Quadratic3DRenderLimit\"?" );
                seqValues[nProperty] >>= m_nQuadratic3DRenderLimit;
            }
            break;

            case PROPERTYHANDLE_QUADRATICFORMCONTROLRENDERLIMIT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\QuadraticFormControlRenderLimit\"?" );
                seqValues[nProperty] >>= m_nQuadraticFormControlRenderLimit;
            }
            break;

            // #i97672# selection settings
            case PROPERTYHANDLE_TRANSPARENTSELECTION:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\TransparentSelection\"?" );
                seqValues[nProperty] >>= m_bTransparentSelection;
            }
            break;

            case PROPERTYHANDLE_TRANSPARENTSELECTIONPERCENT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\TransparentSelectionPercent\"?" );
                seqValues[nProperty] >>= m_nTransparentSelectionPercent;
            }
            break;

            case PROPERTYHANDLE_SELECTIONMAXIMUMLUMINANCEPERCENT:
            {
                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SHORT), "SvtOptionsDrawinglayer_Impl::SvtOptionsDrawinglayer_Impl()\nWho has changed the value type of \"Office.Common\\Drawinglayer\\SelectionMaximumLuminancePercent\"?" );
                seqValues[nProperty] >>= m_nSelectionMaximumLuminancePercent;
            }
            break;
        }
    }
}

SvtOptionsDrawinglayer_Impl::~SvtOptionsDrawinglayer_Impl()
{
        if (IsModified())
            Commit();
}

//  Commit

void SvtOptionsDrawinglayer_Impl::ImplCommit()
{
    Sequence< OUString >    aSeqNames( impl_GetPropertyNames() );
    Sequence< Any >         aSeqValues( aSeqNames.getLength() );

    for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_OVERLAYBUFFER:
                aSeqValues[nProperty] <<= m_bOverlayBuffer;
            break;

            case PROPERTYHANDLE_PAINTBUFFER:
                aSeqValues[nProperty] <<= m_bPaintBuffer;
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_A:
                aSeqValues[nProperty] <<= m_bStripeColorA;
            break;

            case PROPERTYHANDLE_STRIPE_COLOR_B:
                aSeqValues[nProperty] <<= m_bStripeColorB;
            break;

            case PROPERTYHANDLE_STRIPE_LENGTH:
                aSeqValues[nProperty] <<= m_nStripeLength;
            break;

            // #i73602#
            case PROPERTYHANDLE_OVERLAYBUFFER_CALC:
                aSeqValues[nProperty] <<= m_bOverlayBuffer_Calc;
            break;

            case PROPERTYHANDLE_OVERLAYBUFFER_WRITER:
                aSeqValues[nProperty] <<= m_bOverlayBuffer_Writer;
            break;

            case PROPERTYHANDLE_OVERLAYBUFFER_DRAWIMPRESS:
                aSeqValues[nProperty] <<= m_bOverlayBuffer_DrawImpress;
            break;

            // #i74769#, #i75172#
            case PROPERTYHANDLE_PAINTBUFFER_CALC:
                aSeqValues[nProperty] <<= m_bPaintBuffer_Calc;
            break;

            case PROPERTYHANDLE_PAINTBUFFER_WRITER:
                aSeqValues[nProperty] <<= m_bPaintBuffer_Writer;
            break;

            case PROPERTYHANDLE_PAINTBUFFER_DRAWIMPRESS:
                aSeqValues[nProperty] <<= m_bPaintBuffer_DrawImpress;
            break;

            // #i4219#
            case PROPERTYHANDLE_MAXIMUMPAPERWIDTH:
                aSeqValues[nProperty] <<= m_nMaximumPaperWidth;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERHEIGHT:
                aSeqValues[nProperty] <<= m_nMaximumPaperHeight;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERLEFTMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperLeftMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERRIGHTMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperRightMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERTOPMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperTopMargin;
            break;

            case PROPERTYHANDLE_MAXIMUMPAPERBOTTOMMARGIN:
                aSeqValues[nProperty] <<= m_nMaximumPaperBottomMargin;
            break;

            // primitives
            case PROPERTYHANDLE_ANTIALIASING:
                aSeqValues[nProperty] <<= m_bAntiAliasing;
            break;

            case PROPERTYHANDLE_SNAPHORVERLINESTODISCRETE:
                aSeqValues[nProperty] <<= m_bSnapHorVerLinesToDiscrete;
            break;

            case PROPERTYHANDLE_SOLIDDRAGCREATE:
                aSeqValues[nProperty] <<= m_bSolidDragCreate;
            break;

            case PROPERTYHANDLE_RENDERDECORATEDTEXTDIRECT:
                aSeqValues[nProperty] <<= m_bRenderDecoratedTextDirect;
            break;

            case PROPERTYHANDLE_RENDERSIMPLETEXTDIRECT:
                aSeqValues[nProperty] <<= m_bRenderSimpleTextDirect;
            break;

            case PROPERTYHANDLE_QUADRATIC3DRENDERLIMIT:
                aSeqValues[nProperty] <<= m_nQuadratic3DRenderLimit;
            break;

            case PROPERTYHANDLE_QUADRATICFORMCONTROLRENDERLIMIT:
                aSeqValues[nProperty] <<= m_nQuadraticFormControlRenderLimit;
            break;

            // #i97672# selection settings
            case PROPERTYHANDLE_TRANSPARENTSELECTION:
                aSeqValues[nProperty] <<= m_bTransparentSelection;
            break;

            case PROPERTYHANDLE_TRANSPARENTSELECTIONPERCENT:
                aSeqValues[nProperty] <<= m_nTransparentSelectionPercent;
            break;

            case PROPERTYHANDLE_SELECTIONMAXIMUMLUMINANCEPERCENT:
                aSeqValues[nProperty] <<= m_nSelectionMaximumLuminancePercent;
            break;
        }
    }

    PutProperties( aSeqNames, aSeqValues );
}

void SvtOptionsDrawinglayer_Impl::Notify( const css::uno::Sequence<OUString>& )
{
}
// #i73602#

// #i74769#, #i75172#

// #i4219#

// helper
bool SvtOptionsDrawinglayer_Impl::IsAAPossibleOnThisSystem() const
{
    if(!m_bAllowAAChecked)
    {
        SvtOptionsDrawinglayer_Impl* pThat = const_cast< SvtOptionsDrawinglayer_Impl* >(this);
        pThat->m_bAllowAAChecked = true;

#ifdef _WIN32
        // WIN32 uses GDIPlus with VCL for the first incarnation; this will be enhanced
        // in the future to use canvases and the canvas renderer, thus an AA-abled
        // canvas needs to be checked here in the future.
        // Currently, just allow AA for WIN32
#endif

        // check XRenderExtension
        if(m_bAllowAA && !Application::GetDefaultDevice()->SupportsOperation( OutDevSupportType::TransparentRect ))
        {
            pThat->m_bAllowAA = false;
        }
    }

    return m_bAllowAA;
}

// primitives

void SvtOptionsDrawinglayer_Impl::SetAntiAliasing( bool bState )
{
    if(m_bAntiAliasing != bState)
    {
        m_bAntiAliasing = bState;
        SetModified();
    }
}

//  private method

Sequence< OUString > SvtOptionsDrawinglayer_Impl::impl_GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_OVERLAYBUFFER      ,
        PROPERTYNAME_PAINTBUFFER        ,
        PROPERTYNAME_STRIPE_COLOR_A     ,
        PROPERTYNAME_STRIPE_COLOR_B     ,
        PROPERTYNAME_STRIPE_LENGTH      ,

        // #i73602#
        PROPERTYNAME_OVERLAYBUFFER_CALC,
        PROPERTYNAME_OVERLAYBUFFER_WRITER,
        PROPERTYNAME_OVERLAYBUFFER_DRAWIMPRESS,

        // #i74769#, #i75172#
        PROPERTYNAME_PAINTBUFFER_CALC,
        PROPERTYNAME_PAINTBUFFER_WRITER,
        PROPERTYNAME_PAINTBUFFER_DRAWIMPRESS,

        // #i4219#
        PROPERTYNAME_MAXIMUMPAPERWIDTH,
        PROPERTYNAME_MAXIMUMPAPERHEIGHT,
        PROPERTYNAME_MAXIMUMPAPERLEFTMARGIN,
        PROPERTYNAME_MAXIMUMPAPERRIGHTMARGIN,
        PROPERTYNAME_MAXIMUMPAPERTOPMARGIN,
        PROPERTYNAME_MAXIMUMPAPERBOTTOMMARGIN,

        // primitives
        PROPERTYNAME_ANTIALIASING,
        PROPERTYNAME_SNAPHORVERLINESTODISCRETE,
        PROPERTYNAME_SOLIDDRAGCREATE,
        PROPERTYNAME_RENDERDECORATEDTEXTDIRECT,
        PROPERTYNAME_RENDERSIMPLETEXTDIRECT,
        PROPERTYNAME_QUADRATIC3DRENDERLIMIT,
        PROPERTYNAME_QUADRATICFORMCONTROLRENDERLIMIT,

        // #i97672# selection settings
        PROPERTYNAME_TRANSPARENTSELECTION,
        PROPERTYNAME_TRANSPARENTSELECTIONPERCENT,
        PROPERTYNAME_SELECTIONMAXIMUMLUMINANCEPERCENT
    };

    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

namespace {
    //global
    std::weak_ptr<SvtOptionsDrawinglayer_Impl> g_pOptionsDrawinglayer;
}

SvtOptionsDrawinglayer::SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl = g_pOptionsDrawinglayer.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtOptionsDrawinglayer_Impl>();
        g_pOptionsDrawinglayer = m_pImpl;
    }
}

SvtOptionsDrawinglayer::~SvtOptionsDrawinglayer()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

bool SvtOptionsDrawinglayer::IsOverlayBuffer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsOverlayBuffer();
}

//  public method

bool SvtOptionsDrawinglayer::IsPaintBuffer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaintBuffer();
}

//  public method

Color SvtOptionsDrawinglayer::GetStripeColorA() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeColorA();
}

//  public method

Color SvtOptionsDrawinglayer::GetStripeColorB() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeColorB();
}

//  public method

sal_uInt16 SvtOptionsDrawinglayer::GetStripeLength() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetStripeLength();
}

// #i73602#
bool SvtOptionsDrawinglayer::IsOverlayBuffer_Calc() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsOverlayBuffer_Calc();
}

bool SvtOptionsDrawinglayer::IsOverlayBuffer_Writer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsOverlayBuffer_Writer();
}

bool SvtOptionsDrawinglayer::IsOverlayBuffer_DrawImpress() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsOverlayBuffer_DrawImpress();
}

// #i74769#, #i75172#
bool SvtOptionsDrawinglayer::IsPaintBuffer_Calc() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaintBuffer_Calc();
}

bool SvtOptionsDrawinglayer::IsPaintBuffer_Writer() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaintBuffer_Writer();
}

bool SvtOptionsDrawinglayer::IsPaintBuffer_DrawImpress() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsPaintBuffer_DrawImpress();
}

// #i4219#
sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperWidth() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperWidth();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperHeight() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperHeight();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperLeftMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperLeftMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperRightMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperRightMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperTopMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperTopMargin();
}

sal_uInt32 SvtOptionsDrawinglayer::GetMaximumPaperBottomMargin() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMaximumPaperBottomMargin();
}

// helper
bool SvtOptionsDrawinglayer::IsAAPossibleOnThisSystem() const
{
    return m_pImpl->IsAAPossibleOnThisSystem();
}

// primitives
bool SvtOptionsDrawinglayer::IsAntiAliasing() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsAntiAliasing() && IsAAPossibleOnThisSystem();
}

bool SvtOptionsDrawinglayer::IsSnapHorVerLinesToDiscrete() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsAntiAliasing() && m_pImpl->IsSnapHorVerLinesToDiscrete();
}

bool SvtOptionsDrawinglayer::IsSolidDragCreate() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsSolidDragCreate();
}

bool SvtOptionsDrawinglayer::IsRenderDecoratedTextDirect() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsRenderDecoratedTextDirect();
}

bool SvtOptionsDrawinglayer::IsRenderSimpleTextDirect() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsRenderSimpleTextDirect();
}

sal_uInt32 SvtOptionsDrawinglayer::GetQuadratic3DRenderLimit() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetQuadratic3DRenderLimit();
}

sal_uInt32 SvtOptionsDrawinglayer::GetQuadraticFormControlRenderLimit() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetQuadraticFormControlRenderLimit();
}

void SvtOptionsDrawinglayer::SetAntiAliasing( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pImpl->SetAntiAliasing( bState );
}

// #i97672# selection settings
bool SvtOptionsDrawinglayer::IsTransparentSelection() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->IsTransparentSelection();
}

sal_uInt16 SvtOptionsDrawinglayer::GetTransparentSelectionPercent() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    sal_uInt16 aRetval(m_pImpl->GetTransparentSelectionPercent());

    // crop to range [10% .. 90%]
    if(aRetval < 10)
    {
        aRetval = 10;
    }

    if(aRetval > 90)
    {
        aRetval = 90;
    }

    return aRetval;
}

sal_uInt16 SvtOptionsDrawinglayer::GetSelectionMaximumLuminancePercent() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    sal_uInt16 aRetval(m_pImpl->GetSelectionMaximumLuminancePercent());

    // crop to range [0% .. 100%]
    if(aRetval > 90)
    {
        aRetval = 90;
    }

    return aRetval;
}

Color SvtOptionsDrawinglayer::getHilightColor() const
{
    Color aRetval(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    const basegfx::BColor aSelection(aRetval.getBColor());
    const double fLuminance(aSelection.luminance());
    const double fMaxLum(GetSelectionMaximumLuminancePercent() / 100.0);

    if(fLuminance > fMaxLum)
    {
        const double fFactor(fMaxLum / fLuminance);
        const basegfx::BColor aNewSelection(
            aSelection.getRed() * fFactor,
            aSelection.getGreen() * fFactor,
            aSelection.getBlue() * fFactor);

        aRetval = Color(aNewSelection);
    }

    return aRetval;
}

namespace
{
    class theOptionsDrawinglayerMutex : public rtl::Static<osl::Mutex, theOptionsDrawinglayerMutex>{};
}

//  private method

Mutex& SvtOptionsDrawinglayer::GetOwnStaticMutex()
{
    return theOptionsDrawinglayerMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

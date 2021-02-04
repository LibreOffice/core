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

#include <sal/config.h>

#include <initializer_list>
#include <string_view>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/metaact.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <AnnotationEnumeration.hxx>
#include <unomodel.hxx>
#include <unopage.hxx>
#include <svl/itemset.hxx>
#include <svx/svdmodel.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <sdpage.hxx>
#include <unoprnms.hxx>
#include <drawdoc.hxx>
#include <svx/unoshape.hxx>
#include <svl/style.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <svx/svditer.hxx>
#include <vcl/wmf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdview.hxx>
#include <svx/xfillit0.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include "unoobj.hxx"

#include <strings.hxx>
#include <bitmaps.hlst>
#include <unokywds.hxx>
#include "unopback.hxx"
#include <vcl/dibtools.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>

using ::com::sun::star::animations::XAnimationNode;
using ::com::sun::star::animations::XAnimationNodeSupplier;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;

namespace {

// this are the ids for page properties
enum WID_PAGE
{
    WID_PAGE_LEFT, WID_PAGE_RIGHT, WID_PAGE_TOP, WID_PAGE_BOTTOM, WID_PAGE_WIDTH,
    WID_PAGE_HEIGHT, WID_PAGE_EFFECT, WID_PAGE_CHANGE, WID_PAGE_SPEED, WID_PAGE_NUMBER,
    WID_PAGE_ORIENT, WID_PAGE_LAYOUT, WID_PAGE_DURATION, WID_PAGE_HIGHRESDURATION, WID_PAGE_LDNAME, WID_PAGE_LDBITMAP,
    WID_PAGE_BACK, WID_PAGE_PREVIEW, WID_PAGE_PREVIEWBITMAP, WID_PAGE_VISIBLE, WID_PAGE_SOUNDFILE, WID_PAGE_BACKFULL,
    WID_PAGE_BACKVIS, WID_PAGE_BACKOBJVIS, WID_PAGE_USERATTRIBS, WID_PAGE_BOOKMARK, WID_PAGE_ISDARK,
    WID_PAGE_HEADERVISIBLE, WID_PAGE_HEADERTEXT, WID_PAGE_FOOTERVISIBLE, WID_PAGE_FOOTERTEXT,
    WID_PAGE_PAGENUMBERVISIBLE, WID_PAGE_DATETIMEVISIBLE, WID_PAGE_DATETIMEFIXED,
    WID_PAGE_DATETIMETEXT, WID_PAGE_DATETIMEFORMAT, WID_TRANSITION_TYPE, WID_TRANSITION_SUBTYPE,
    WID_TRANSITION_DIRECTION, WID_TRANSITION_FADE_COLOR, WID_TRANSITION_DURATION, WID_LOOP_SOUND,
    WID_NAVORDER, WID_PAGE_PREVIEWMETAFILE
};

}

constexpr OUStringLiteral sEmptyPageName = u"page";

// this function stores the property maps for draw pages in impress and draw
static const SvxItemPropertySet* ImplGetDrawPagePropertySet( bool bImpress, PageKind ePageKind )
{
    static const SfxItemPropertyMapEntry aDrawPagePropertyMap_Impl[] =
    {
        { u"" UNO_NAME_PAGE_BACKGROUND,       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                  beans::PropertyAttribute::MAYBEVOID,0},
        { u"" UNO_NAME_PAGE_BOTTOM,           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_LEFT,             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_RIGHT,            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_TOP,              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_CHANGE,           WID_PAGE_CHANGE,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_DURATION,         WID_PAGE_DURATION,  ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_EFFECT,           WID_PAGE_EFFECT,    ::cppu::UnoType<presentation::FadeEffect>::get(),     0,  0},
        { u"" UNO_NAME_PAGE_HEIGHT,           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_LAYOUT,           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP,     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                          beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_NUMBER,           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_ORIENTATION,      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { u"" UNO_NAME_PAGE_SPEED,            WID_PAGE_SPEED,     ::cppu::UnoType<presentation::AnimationSpeed>::get(), 0,  0},
        { u"" UNO_NAME_PAGE_WIDTH,            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_PREVIEW,          WID_PAGE_PREVIEW,   cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_PREVIEWBITMAP,    WID_PAGE_PREVIEWBITMAP, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_PREVIEWMETAFILE,  WID_PAGE_PREVIEWMETAFILE, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_VISIBLE,          WID_PAGE_VISIBLE,   cppu::UnoType<bool>::get(),                        0, 0},
        { u"" UNO_NAME_OBJ_SOUNDFILE,         WID_PAGE_SOUNDFILE, cppu::UnoType<Any>::get(),              0, 0},
        { sUNO_Prop_IsBackgroundVisible,  WID_PAGE_BACKVIS,   cppu::UnoType<bool>::get(),                        0, 0},
        { sUNO_Prop_IsBackgroundObjectsVisible,   WID_PAGE_BACKOBJVIS,    cppu::UnoType<bool>::get(),                        0, 0},
        { sUNO_Prop_UserDefinedAttributes,WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { sUNO_Prop_BookmarkURL,          WID_PAGE_BOOKMARK,  ::cppu::UnoType<OUString>::get(),             0,  0},
        { u"HighResDuration",              WID_PAGE_HIGHRESDURATION,  ::cppu::UnoType<double>::get(),            0,  0},
        { u"IsBackgroundDark" ,            WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { u"IsFooterVisible",              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { u"FooterText",                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { u"IsPageNumberVisible",          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { u"IsDateTimeVisible",            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},
        { u"IsDateTimeFixed",              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},
        { u"DateTimeText",                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},
        { u"DateTimeFormat",               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},
        { u"TransitionType",               WID_TRANSITION_TYPE, ::cppu::UnoType<sal_Int16>::get(),           0,  0},
        { u"TransitionSubtype",            WID_TRANSITION_SUBTYPE, ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { u"TransitionDirection",          WID_TRANSITION_DIRECTION, ::cppu::UnoType<sal_Bool>::get(),           0,  0},
        { u"TransitionFadeColor",          WID_TRANSITION_FADE_COLOR, ::cppu::UnoType<sal_Int32>::get(),         0,  0},
        { u"" UNO_NAME_PAGE_TRANSITION_DURATION, WID_TRANSITION_DURATION, ::cppu::UnoType<double>::get(),          0,  0},
        { u"LoopSound",                    WID_LOOP_SOUND, cppu::UnoType<bool>::get(),                    0, 0},
        { u"NavigationOrder",              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

#define DRAW_PAGE_NOTES_PROPERTIES \
        { u"" UNO_NAME_PAGE_BOTTOM,           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_PAGE_LEFT,             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_PAGE_RIGHT,            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_PAGE_TOP,              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_PAGE_HEIGHT,           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_PAGE_LAYOUT,           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},                                                                \
        { u"" UNO_NAME_LINKDISPLAYBITMAP,     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                          beans::PropertyAttribute::READONLY, 0},                                \
        { u"" UNO_NAME_LINKDISPLAYNAME,       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},                                \
        { u"" UNO_NAME_PAGE_NUMBER,           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},                                \
        { u"" UNO_NAME_PAGE_ORIENTATION,      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},                                                                \
        { u"" UNO_NAME_PAGE_WIDTH,            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { sUNO_Prop_UserDefinedAttributes,WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),      0,     0},\
        { u"IsHeaderVisible",              WID_PAGE_HEADERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { u"HeaderText",                   WID_PAGE_HEADERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},                                                            \
        { u"IsBackgroundDark",             WID_PAGE_ISDARK,     cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},                                \
        { u"IsFooterVisible",              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { u"FooterText",                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},                                                            \
        { u"IsPageNumberVisible",          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                             \
        { u"IsDateTimeVisible",            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},                                                                 \
        { u"IsDateTimeFixed",              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { u"DateTimeText",                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},                                                            \
        { u"DateTimeFormat",               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},                                                            \
        { u"NavigationOrder",              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},                                                            \
        { u"", 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyMap_Impl[] =
    {
        // this must be the first two entries so they can be excluded for PageKind::Standard
        { u"" UNO_NAME_PAGE_BACKGROUND,       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                  beans::PropertyAttribute::MAYBEVOID,0},
        DRAW_PAGE_NOTES_PROPERTIES
    };
    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyNoBackMap_Impl[] =
    {
        DRAW_PAGE_NOTES_PROPERTIES
    };

#define GRAPHIC_PAGE_PROPERTIES \
        { u"" UNO_NAME_PAGE_BOTTOM,           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_PAGE_LEFT,             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_PAGE_RIGHT,            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_PAGE_TOP,              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_PAGE_HEIGHT,           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_LINKDISPLAYBITMAP,     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                           beans::PropertyAttribute::READONLY, 0},                                             \
        { u"" UNO_NAME_LINKDISPLAYNAME,       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},                                             \
        { u"" UNO_NAME_PAGE_NUMBER,           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},                                             \
        { u"" UNO_NAME_PAGE_ORIENTATION,      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},                                                                             \
        { u"" UNO_NAME_PAGE_WIDTH,            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { u"" UNO_NAME_PAGE_PREVIEW,          WID_PAGE_PREVIEW,   cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},    \
        { u"" UNO_NAME_PAGE_PREVIEWBITMAP,    WID_PAGE_PREVIEWBITMAP, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},\
        { u"" UNO_NAME_PAGE_PREVIEWMETAFILE,  WID_PAGE_PREVIEWMETAFILE, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},\
        { sUNO_Prop_UserDefinedAttributes,WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},                          \
        { sUNO_Prop_BookmarkURL,          WID_PAGE_BOOKMARK,  ::cppu::UnoType<OUString>::get(),             0,  0},                                                                             \
        { u"IsBackgroundDark",             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},                                             \
        { u"NavigationOrder",              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},                                                                         \
        { u"", 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry aGraphicPagePropertyMap_Impl[] =
    {
        { u"" UNO_NAME_PAGE_BACKGROUND,       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                   beans::PropertyAttribute::MAYBEVOID,0},
        GRAPHIC_PAGE_PROPERTIES
    };
    static const SfxItemPropertyMapEntry aGraphicPagePropertyNoBackMap_Impl[] =
    {
        GRAPHIC_PAGE_PROPERTIES
    };

    bool bWithoutBackground = ePageKind != PageKind::Standard && ePageKind != PageKind::Handout;
    const SvxItemPropertySet* pRet = nullptr;
    if( bImpress )
    {
        if( ePageKind == PageKind::Standard )
        {
            //PageKind::Standard always has a background property
            static SvxItemPropertySet aDrawPagePropertySet_Impl( aDrawPagePropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
            pRet = &aDrawPagePropertySet_Impl;
        }
        else
        {
            if(bWithoutBackground)
            {
                static SvxItemPropertySet aDrawPageNotesHandoutPropertyNoBackSet_Impl( aDrawPageNotesHandoutPropertyNoBackMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
                pRet = &aDrawPageNotesHandoutPropertyNoBackSet_Impl;
            }
            else
            {
                static SvxItemPropertySet aDrawPageNotesHandoutPropertySet_Impl( aDrawPageNotesHandoutPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
                pRet = &aDrawPageNotesHandoutPropertySet_Impl;
            }
        }
    }
    else
    {
            if(bWithoutBackground)
            {
                static SvxItemPropertySet aGraphicPagePropertyNoBackSet_Impl( aGraphicPagePropertyNoBackMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
                pRet = &aGraphicPagePropertyNoBackSet_Impl;
            }
            else
            {
                static SvxItemPropertySet aGraphicPagePropertySet_Impl( aGraphicPagePropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
                pRet = &aGraphicPagePropertySet_Impl;
            }
    }
    return pRet;
}

/** this function stores the property map for master pages in impress and draw */
static const SvxItemPropertySet* ImplGetMasterPagePropertySet( PageKind ePageKind )
{
    static const SfxItemPropertyMapEntry aMasterPagePropertyMap_Impl[] =
    {
        { u"" UNO_NAME_PAGE_BACKGROUND,       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                    0,  0},
        { u"" UNO_NAME_PAGE_BOTTOM,           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_LEFT,             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_RIGHT,            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_TOP,              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_HEIGHT,           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP,     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                           beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_NUMBER,           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_ORIENTATION,      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { u"" UNO_NAME_PAGE_WIDTH,            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"BackgroundFullSize",           WID_PAGE_BACKFULL,  cppu::UnoType<bool>::get(),                        0, 0},
        { sUNO_Prop_UserDefinedAttributes,WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { u"IsBackgroundDark",             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    static const SfxItemPropertyMapEntry aHandoutMasterPagePropertyMap_Impl[] =
    {
        { u"" UNO_NAME_PAGE_BOTTOM,           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_LEFT,             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_RIGHT,            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_TOP,              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_HEIGHT,           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_ORIENTATION,      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { u"" UNO_NAME_PAGE_NUMBER,           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_PAGE_WIDTH,            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { u"" UNO_NAME_PAGE_LAYOUT,           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { sUNO_Prop_UserDefinedAttributes,WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { u"IsBackgroundDark",             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { u"IsHeaderVisible",              WID_PAGE_HEADERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { u"HeaderText",                   WID_PAGE_HEADERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { u"IsFooterVisible",              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { u"FooterText",                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { u"IsPageNumberVisible",          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { u"IsDateTimeVisible",            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},
        { u"IsDateTimeFixed",              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},
        { u"DateTimeText",                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},
        { u"DateTimeFormat",               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    const SvxItemPropertySet* pRet = nullptr;
    if( ePageKind == PageKind::Handout )
    {
        static SvxItemPropertySet aHandoutMasterPagePropertySet_Impl( aHandoutMasterPagePropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
        pRet = &aHandoutMasterPagePropertySet_Impl;
    }
    else
    {
        static SvxItemPropertySet aMasterPagePropertySet_Impl( aMasterPagePropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
        pRet = &aMasterPagePropertySet_Impl;
    }
    return pRet;
}

namespace
{
    class theSdGenericDrawPageUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdGenericDrawPageUnoTunnelId> {};
}

const css::uno::Sequence< sal_Int8 > & SdPage::getUnoTunnelId() throw()
{
    return theSdGenericDrawPageUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SdPage::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
        if( isUnoTunnelId<SdPage>(rId) )
        {
                return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
        }
        else
        {
                return FmFormPage::getSomething( rId );
        }
}


void SdPage::throwIfDisposed() const
{
    if( isDisposed() )
        throw lang::DisposedException();
}

SdXImpressDocument* SdPage::GetModel() const
{
    return mpDocModel;
}

bool SdPage::IsImpressDocument() const
{
    return mbIsImpressDocument;
}


// this is called whenever a SdrObject must be created for an empty api shape wrapper
SdrObject * SdPage::CreateSdrObject_( const Reference< drawing::XShape >& xShape )
{
    if( !xShape.is() )
        return nullptr;

    OUString aType( xShape->getShapeType() );
    const OUString aPrefix( "com.sun.star.presentation." );
    if( !aType.startsWith( aPrefix ) )
    {
        SdrObject* pObj = FmFormPage::CreateSdrObject_( xShape );
        return pObj;
    }

    aType = aType.copy( aPrefix.getLength() );

    PresObjKind eObjKind = PresObjKind::NONE;

    if( aType == "TitleTextShape" )
    {
        eObjKind = PresObjKind::Title;
    }
    else if( aType == "OutlinerShape" )
    {
        eObjKind = PresObjKind::Outline;
    }
    else if( aType == "SubtitleShape" )
    {
        eObjKind = PresObjKind::Text;
    }
    else if( aType == "OLE2Shape" )
    {
        eObjKind = PresObjKind::Object;
    }
    else if( aType == "ChartShape" )
    {
        eObjKind = PresObjKind::Chart;
    }
    else if( aType == "CalcShape" )
    {
        eObjKind = PresObjKind::Calc;
    }
    else if( aType == "TableShape" )
    {
        eObjKind = PresObjKind::Table;
    }
    else if( aType == "GraphicObjectShape" )
    {
        eObjKind = PresObjKind::Graphic;
    }
    else if( aType == "OrgChartShape" )
    {
        eObjKind = PresObjKind::OrgChart;
    }
    else if( aType == "PageShape" )
    {
        if( GetPageKind() == PageKind::Notes && IsMasterPage() )
            eObjKind = PresObjKind::Title;
        else
            eObjKind = PresObjKind::Page;
    }
    else if( aType == "NotesShape" )
    {
        eObjKind = PresObjKind::Notes;
    }
    else if( aType == "HandoutShape" )
    {
        eObjKind = PresObjKind::Handout;
    }
    else if( aType == "FooterShape" )
    {
        eObjKind = PresObjKind::Footer;
    }
    else if( aType == "HeaderShape" )
    {
        eObjKind = PresObjKind::Header;
    }
    else if( aType == "SlideNumberShape" )
    {
        eObjKind = PresObjKind::SlideNumber;
    }
    else if( aType == "DateTimeShape" )
    {
        eObjKind = PresObjKind::DateTime;
    }
    else if( aType == "MediaShape" )
    {
        eObjKind = PresObjKind::Media;
    }

    ::tools::Rectangle aRect( eObjKind == PresObjKind::Title ? GetTitleRect() : GetLayoutRect()  );

    const awt::Point aPos( aRect.Left(), aRect.Top() );
    xShape->setPosition( aPos );

    const awt::Size aSize( aRect.GetWidth(), aRect.GetHeight() );
    xShape->setSize( aSize );

    SdrObject *pPresObj = nullptr;
    if( (eObjKind == PresObjKind::Table) || (eObjKind == PresObjKind::Media) )
    {
        pPresObj = FmFormPage::CreateSdrObject_( xShape );
        if( pPresObj )
        {
            SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
            pPresObj->NbcSetStyleSheet(rDoc.GetDefaultStyleSheet(), true);
            InsertPresObj( pPresObj, eObjKind );
        }
    }
    else
    {
        pPresObj = CreatePresObj( eObjKind, false, aRect );
    }

    if( pPresObj )
        pPresObj->SetUserCall( this );

    return pPresObj;
}

// XInterface
Any SAL_CALL SdPage::queryInterface( const uno::Type & rType )
{
    Any aAny;

    if (rType == cppu::UnoType<beans::XPropertySet>::get())
    {
        aAny <<= Reference<beans::XPropertySet>(this);
    }
    else if (rType == cppu::UnoType<container::XNamed>::get())
    {
        aAny <<= Reference<container::XNamed>(this);
    }
    else if (rType == cppu::UnoType<util::XReplaceable>::get())
    {
        aAny <<= Reference<util::XReplaceable>(this);
    }
    else if (rType == cppu::UnoType<util::XSearchable>::get())
    {
        aAny <<= Reference<util::XSearchable>(this);
    }
    else if (rType == cppu::UnoType<document::XLinkTargetSupplier>::get())
    {
        aAny <<= Reference<document::XLinkTargetSupplier>(this);
    }
    else if (rType == cppu::UnoType<drawing::XShapeCombiner>::get())
    {
        aAny <<= Reference<drawing::XShapeCombiner>(this);
    }
    else if (rType == cppu::UnoType<drawing::XShapeBinder>::get())
    {
        aAny <<= Reference<drawing::XShapeBinder>(this);
    }
    else if (rType == cppu::UnoType<beans::XMultiPropertySet>::get())
    {
        aAny <<= Reference<beans::XMultiPropertySet>(this);
    }
    else if (rType == cppu::UnoType<office::XAnnotationAccess>::get())
    {
        aAny <<= Reference<office::XAnnotationAccess>(this);
    }
    else if (IsImpressDocument() && rType == cppu::UnoType<XAnimationNodeSupplier>::get())
    {
        const PageKind ePageKind = GetPageKind();

        if( ePageKind == PageKind::Standard )
            return makeAny( Reference< XAnimationNodeSupplier >( this ) );
    }
    else
        return SdrPage::queryInterface( rType );

    return aAny;
}

// XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL SdPage::getPropertySetInfo()
{
    ::SolarMutexGuard aGuard;
    throwIfDisposed();
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdPage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
        case WID_NAVORDER:
            setNavigationOrder( aValue );
            break;
        case WID_PAGE_LEFT:
        case WID_PAGE_RIGHT:
        case WID_PAGE_TOP:
        case WID_PAGE_BOTTOM:
        case WID_PAGE_LAYOUT:
        case WID_PAGE_DURATION:
        case WID_PAGE_CHANGE:
        {
            sal_Int32 nValue = 0;
            if(!(aValue >>= nValue))
                throw lang::IllegalArgumentException();

            switch( pEntry->nWID )
            {
            case WID_PAGE_LEFT:
                SetLeftBorder(nValue);
                break;
            case WID_PAGE_RIGHT:
                SetRightBorder( nValue );
                break;
            case WID_PAGE_TOP:
                SetUpperBorder( nValue );
                break;
            case WID_PAGE_BOTTOM:
                SetLowerBorder( nValue );
                break;
            case WID_PAGE_CHANGE:
                SetPresChange( static_cast<PresChange>(nValue) );
                break;
            case WID_PAGE_LAYOUT:
                SetAutoLayout( static_cast<AutoLayout>(nValue), true );
                break;
            case WID_PAGE_DURATION:
                SetTime(nValue);
                break;
            }
            break;
        }
        case WID_PAGE_HIGHRESDURATION:
        {
            double fValue = 0;
            if(!(aValue >>= fValue))
                throw lang::IllegalArgumentException();

            SetTime(fValue);
            break;
        }
        case WID_PAGE_WIDTH:
        {
            sal_Int32 nWidth = 0;
            if(!(aValue >>= nWidth))
                throw lang::IllegalArgumentException();

            SetWidth( nWidth );
            break;
        }
        case WID_PAGE_HEIGHT:
        {
            sal_Int32 nHeight = 0;
            if(!(aValue >>= nHeight))
                throw lang::IllegalArgumentException();

            SetHeight( nHeight );
            break;
        }
        case WID_PAGE_ORIENT:
        {
            sal_Int32 nEnum = 0;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            Orientation eOri = (static_cast<view::PaperOrientation>(nEnum) == view::PaperOrientation_PORTRAIT)?Orientation::Portrait:Orientation::Landscape;

            if( eOri != GetOrientation() )
            {
                SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
                const PageKind ePageKind = GetPageKind();

                sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
                    pPage->SetOrientation( eOri );
                }

                nPageCnt = rDoc.GetSdPageCount(ePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
                    pPage->SetOrientation( eOri );
                }
            }
            break;
        }
        case WID_PAGE_EFFECT:
        {
            sal_Int32 nEnum = 0;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            SetFadeEffect( static_cast<presentation::FadeEffect>(nEnum) );
            break;
        }
        case WID_PAGE_BACK:
            setBackground( aValue );
            break;
        case WID_PAGE_SPEED:
        {
            sal_Int32 nEnum = 0;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            setTransitionDuration( nEnum == 0 ? 3.0 : (nEnum == 1 ? 2.0 : 1.0 )  );
            break;
        }
        case WID_PAGE_VISIBLE :
        {
            bool    bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();
            SetExcluded( !bVisible );
            break;
        }
        case WID_PAGE_SOUNDFILE :
        {
            OUString aURL;
            if( aValue >>= aURL )
            {
                SetSoundFile( aURL );
                SetSound( !aURL.isEmpty() );
                break;
            }
            else
            {
                bool bStopSound = false;
                if( aValue >>= bStopSound )
                {
                    SetStopSound( bStopSound );
                    break;
                }
            }

            throw lang::IllegalArgumentException();
        }
        case WID_LOOP_SOUND:
        {
            bool bLoop = false;
            if( ! (aValue >>= bLoop) )
                throw lang::IllegalArgumentException();

            SetLoopSound( bLoop );
            break;
        }
        case WID_PAGE_BACKFULL:
        {
            bool    bFullSize = false;
            if( ! ( aValue >>= bFullSize ) )
                throw lang::IllegalArgumentException();
            SetBackgroundFullSize( bFullSize );
            break;
        }
        case WID_PAGE_BACKVIS:
        {
            bool bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
            if( rDoc.GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
                SdrLayerIDSet aVisibleLayers = TRG_GetMasterPageVisibleLayers();
                aVisibleLayers.Set(rLayerAdmin.GetLayerID(sUNO_LayerName_background), bVisible);
                TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            break;
        }
        case WID_PAGE_BACKOBJVIS:
        {
            bool bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
            if( rDoc.GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
                SdrLayerIDSet aVisibleLayers = TRG_GetMasterPageVisibleLayers();
                aVisibleLayers.Set(rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects), bVisible);
                TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }

            break;
        }
        case WID_PAGE_USERATTRIBS:
        {
            if( !setAlienAttributes( aValue ) )
                throw lang::IllegalArgumentException();
            break;
        }
        case WID_PAGE_BOOKMARK:
        {
            OUString aBookmarkURL;
            if( ! ( aValue >>= aBookmarkURL ) )
                throw lang::IllegalArgumentException();

            setBookmarkURL( aBookmarkURL );
            break;
        }

        case WID_PAGE_HEADERVISIBLE:
        case WID_PAGE_HEADERTEXT:
        case WID_PAGE_FOOTERVISIBLE:
        case WID_PAGE_FOOTERTEXT:
        case WID_PAGE_PAGENUMBERVISIBLE:
        case WID_PAGE_DATETIMEVISIBLE:
        case WID_PAGE_DATETIMEFIXED:
        case WID_PAGE_DATETIMETEXT:
        case WID_PAGE_DATETIMEFORMAT:
        {
            sd::HeaderFooterSettings aHeaderFooterSettings( getHeaderFooterSettings() );

            switch( pEntry->nWID )
            {
            case WID_PAGE_HEADERVISIBLE:
            {
                bool bVisible = false;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbHeaderVisible = bVisible;
                break;
            }
            case WID_PAGE_HEADERTEXT:
            {
                OUString aText;
                if( ! ( aValue >>= aText ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.maHeaderText = aText;
                break;
            }
            case WID_PAGE_FOOTERVISIBLE:
            {
                bool bVisible = false;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbFooterVisible = bVisible;
                break;
            }
            case WID_PAGE_FOOTERTEXT:
            {
                OUString aText;
                if( ! ( aValue >>= aText ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.maFooterText = aText;
                break;
            }
            case WID_PAGE_PAGENUMBERVISIBLE:
            {
                bool bVisible = false;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbSlideNumberVisible = bVisible;
                break;
            }
            case WID_PAGE_DATETIMEVISIBLE:
            {
                bool bVisible = false;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbDateTimeVisible = bVisible;
                break;
            }
            case WID_PAGE_DATETIMEFIXED:
            {
                bool bVisible = false;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbDateTimeIsFixed = bVisible;
                break;
            }
            case WID_PAGE_DATETIMETEXT:
            {
                OUString aText;
                if( ! ( aValue >>= aText ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.maDateTimeText = aText;
                break;
            }
            case WID_PAGE_DATETIMEFORMAT:
            {
                sal_Int32 nValue = 0;
                if( ! ( aValue >>= nValue ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.meDateFormat = static_cast<SvxDateFormat>(nValue & 0x0f);
                aHeaderFooterSettings.meTimeFormat = static_cast<SvxTimeFormat>((nValue >> 4) & 0x0f);
                break;
            }
            }

            if( !(aHeaderFooterSettings == getHeaderFooterSettings()) )
                setHeaderFooterSettings( aHeaderFooterSettings );

            break;
        }

        case WID_PAGE_NUMBER:
            if( (GetPageKind() == PageKind::Handout) && !IsMasterPage() )
            {
                if( !(aValue >>= mnTempPageNumber) )
                    throw lang::IllegalArgumentException();

                break;
            }
            throw beans::PropertyVetoException();

        case WID_PAGE_LDBITMAP:
        case WID_PAGE_LDNAME:
        case WID_PAGE_ISDARK:
            throw beans::PropertyVetoException();

        case WID_TRANSITION_TYPE:
        {
            sal_Int16 nValue = 0;
            if( ! ( aValue >>= nValue ) )
                throw lang::IllegalArgumentException();

            setTransitionType( nValue );
            break;
        }

        case WID_TRANSITION_SUBTYPE:
        {
            sal_Int16 nValue = 0;
            if( ! ( aValue >>= nValue ) )
                throw lang::IllegalArgumentException();

            setTransitionSubtype( nValue );
            break;
        }

        case WID_TRANSITION_DIRECTION:
        {
            bool bValue = false;
            if( ! ( aValue >>= bValue ) )
                throw lang::IllegalArgumentException();

            setTransitionDirection( bValue );
            break;
        }

        case WID_TRANSITION_FADE_COLOR:
        {
            sal_Int32 nValue = 0;
            if( ! ( aValue >>= nValue ) )
                throw lang::IllegalArgumentException();

            setTransitionFadeColor( nValue );
            break;
        }

        case WID_TRANSITION_DURATION:
        {
            double fValue = 0.0;
            if( ! ( aValue >>= fValue ) )
                throw lang::IllegalArgumentException();

            setTransitionDuration( fValue );
            break;
        }

        default:
            throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    GetModel()->SetModified();
}

Any SAL_CALL SdPage::getPropertyValue( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Any aAny;

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    sal_Int16 nEntry = pEntry ? pEntry->nWID : -1;
    switch (nEntry)
    {
    case WID_NAVORDER:
        aAny = getNavigationOrder();
        break;
    case WID_PAGE_LEFT:
        aAny <<= GetLeftBorder();
        break;
    case WID_PAGE_RIGHT:
        aAny <<= GetRightBorder();
        break;
    case WID_PAGE_TOP:
        aAny <<= GetUpperBorder();
        break;
    case WID_PAGE_BOTTOM:
        aAny <<= GetLowerBorder();
        break;
    case WID_PAGE_WIDTH:
        aAny <<= static_cast<sal_Int32>( GetSize().getWidth() );
        break;
    case WID_PAGE_HEIGHT:
        aAny <<= static_cast<sal_Int32>( GetSize().getHeight() );
        break;
    case WID_PAGE_ORIENT:
        aAny <<=
            GetOrientation() == Orientation::Portrait
            ? view::PaperOrientation_PORTRAIT
            : view::PaperOrientation_LANDSCAPE;
        break;
    case WID_PAGE_EFFECT:
        aAny <<= GetFadeEffect();
        break;
    case WID_PAGE_CHANGE:
        aAny <<= static_cast<sal_Int32>( GetPresChange() );
        break;
    case WID_PAGE_SPEED:
        {
            const double fDuration = getTransitionDuration();
            aAny <<= presentation::AnimationSpeed(
                fDuration < 2.0 ? 2 : fDuration > 2.0 ? 0 : 1);
        }
        break;
    case WID_PAGE_LAYOUT:
        aAny <<= static_cast<sal_Int16>( GetAutoLayout() );
        break;
    case WID_PAGE_NUMBER:
        {
            const sal_uInt16 nPageNumber(GetPageNum());

            if(nPageNumber > 0)
            {
                // for all other pages calculate the number
                aAny <<= static_cast<sal_Int16>(static_cast<sal_uInt16>((nPageNumber-1)>>1) + 1);
            }
            else
            {
                aAny <<= mnTempPageNumber;
            }
        }
        break;
    case WID_PAGE_DURATION:
        aAny <<= static_cast<sal_Int32>( GetTime() + .5 );
        break;
    case WID_PAGE_HIGHRESDURATION:
        aAny <<= GetTime();
        break;
    case WID_PAGE_LDNAME:
    {
        const OUString aName( getName() );
        aAny <<= aName;
        break;
    }
    case WID_PAGE_LDBITMAP:
        {
            Reference< awt::XBitmap > xBitmap(VCLUnoHelper::CreateBitmap(BitmapEx(BMP_PAGE)));
            aAny <<= xBitmap;
        }
        break;
    case WID_PAGE_BACK:
        getBackground( aAny );
        break;
    case WID_PAGE_PREVIEW :
    case WID_PAGE_PREVIEWMETAFILE :
        {
            SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
            ::sd::DrawDocShell* pDocShell = rDoc.GetDocSh();
            if ( pDocShell )
            {
                sal_uInt16 nPgNum = 0;
                sal_uInt16 nPageCount = rDoc.GetSdPageCount( PageKind::Standard );
                sal_uInt16 nPageNumber = static_cast<sal_uInt16>( ( GetPageNum() - 1 ) >> 1 );
                while( nPgNum < nPageCount )
                {
                    rDoc.SetSelected( rDoc.GetSdPage( nPgNum, PageKind::Standard ), nPgNum == nPageNumber );
                    nPgNum++;
                }
                std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();
                if (xMetaFile)
                {
                    Size    aSize( GetSize() );
                    xMetaFile->AddAction( new MetaFillColorAction( COL_WHITE, true ), 0 );
                    xMetaFile->AddAction( new MetaRectAction( ::tools::Rectangle( Point(), aSize ) ), 1 );
                    xMetaFile->SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                    xMetaFile->SetPrefSize( aSize );

                    SvMemoryStream aDestStrm( 65535, 65535 );
                    if (nEntry == WID_PAGE_PREVIEW)
                        // Preview: WMF format.
                        ConvertGDIMetaFileToWMF(*xMetaFile, aDestStrm, nullptr, false);
                    else
                        // PreviewMetafile: SVM format.
                        xMetaFile->Write(aDestStrm);
                    Sequence<sal_Int8> aSeq( static_cast<sal_Int8 const *>(aDestStrm.GetData()), aDestStrm.Tell() );
                    aAny <<= aSeq;
                }
            }
        }
        break;

    case WID_PAGE_PREVIEWBITMAP :
        {
            SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
            ::sd::DrawDocShell* pDocShell = rDoc.GetDocSh();
            if ( pDocShell )
            {
                sal_uInt16 nPgNum = 0;
                sal_uInt16 nPageCount = rDoc.GetSdPageCount( PageKind::Standard );
                sal_uInt16 nPageNumber = static_cast<sal_uInt16>( ( GetPageNum() - 1 ) >> 1 );
                while( nPgNum < nPageCount )
                {
                    rDoc.SetSelected( rDoc.GetSdPage( nPgNum, PageKind::Standard ), nPgNum == nPageNumber );
                    nPgNum++;
                }
                std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();
                BitmapEx aBitmap;
                if (xMetaFile && xMetaFile->CreateThumbnail(aBitmap))
                {
                    SvMemoryStream aMemStream;
                    WriteDIB(aBitmap.GetBitmap(), aMemStream, false, false);
                    uno::Sequence<sal_Int8> aSeq( static_cast<sal_Int8 const *>(aMemStream.GetData()), aMemStream.Tell() );
                    aAny <<= aSeq;
                }
            }
        }
        break;

    case WID_PAGE_VISIBLE :
    {
        bool bVisible = !IsExcluded();
        aAny <<= bVisible;
        break;
    }

    case WID_PAGE_SOUNDFILE :
    {
        if( IsStopSound() )
        {
            aAny <<= true;
        }
        else
        {
            OUString aURL;
            if( IsSoundOn() )
                aURL = GetSoundFile();
            aAny <<= aURL;
        }
        break;
    }
    case WID_LOOP_SOUND:
    {
        aAny <<= IsLoopSound();
        break;
    }
    case WID_PAGE_BACKFULL:
    {
        bool bFullSize = IsBackgroundFullSize();
        aAny <<= bFullSize;
        break;
    }
    case WID_PAGE_BACKVIS:
    {
        SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
        if( rDoc.GetMasterPageCount() )
        {
            SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
            SdrLayerIDSet aVisibleLayers = TRG_GetMasterPageVisibleLayers();
            aAny <<= aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(sUNO_LayerName_background));
        }
        else
        {
            aAny <<= false;
        }
        break;
    }
    case WID_PAGE_BACKOBJVIS:
    {
        SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
        if( rDoc.GetMasterPageCount() )
        {
            SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
            SdrLayerIDSet aVisibleLayers = TRG_GetMasterPageVisibleLayers();
            aAny <<= aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects));
        }
        else
        {
            aAny <<= false;
        }
        break;
    }
    case WID_PAGE_USERATTRIBS:
    {
        getAlienAttributes( aAny );
        break;
    }
    case WID_PAGE_BOOKMARK:
    {
        aAny <<= getBookmarkURL();
        break;
    }
    case WID_PAGE_ISDARK:
    {
        aAny <<= GetPageBackgroundColor().IsDark();
        break;
    }
    case WID_PAGE_HEADERVISIBLE:
        aAny <<= getHeaderFooterSettings().mbHeaderVisible;
        break;
    case WID_PAGE_HEADERTEXT:
        {
            const OUString aText( getHeaderFooterSettings().maHeaderText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_FOOTERVISIBLE:
        aAny <<= getHeaderFooterSettings().mbFooterVisible;
        break;
    case WID_PAGE_FOOTERTEXT:
        {
            const OUString aText( getHeaderFooterSettings().maFooterText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_PAGENUMBERVISIBLE:
        aAny <<= getHeaderFooterSettings().mbSlideNumberVisible;
        break;
    case WID_PAGE_DATETIMEVISIBLE:
        aAny <<= getHeaderFooterSettings().mbDateTimeVisible;
        break;
    case WID_PAGE_DATETIMEFIXED:
        aAny <<= getHeaderFooterSettings().mbDateTimeIsFixed;
        break;
    case WID_PAGE_DATETIMETEXT:
        {
            const OUString aText( getHeaderFooterSettings().maDateTimeText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_DATETIMEFORMAT:
        {
            auto const & rSettings = getHeaderFooterSettings();
            sal_Int32 x = static_cast<sal_Int32>(rSettings.meDateFormat) | (static_cast<sal_Int32>(rSettings.meTimeFormat) << 4);
            aAny <<= x;
        }
        break;

    case WID_TRANSITION_TYPE:
        aAny <<= getTransitionType();
        break;

    case WID_TRANSITION_SUBTYPE:
        aAny <<= getTransitionSubtype();
        break;

    case WID_TRANSITION_DIRECTION:
        aAny <<= getTransitionDirection();
        break;

    case WID_TRANSITION_FADE_COLOR:
        aAny <<= getTransitionFadeColor();
        break;

    case WID_TRANSITION_DURATION:
        aAny <<= getTransitionDuration();
        break;

    default:
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }
    return aAny;
}

void SAL_CALL SdPage::addPropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdPage::removePropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdPage::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdPage::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) {}

// XMultiPropertySet
void SAL_CALL SdPage::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
{
    if( aPropertyNames.getLength() != aValues.getLength() )
        throw lang::IllegalArgumentException();

    const OUString* pNames = aPropertyNames.getConstArray();
    const Any* pValues = aValues.getConstArray();
    sal_uInt32 nCount = aValues.getLength();
    while( nCount-- )
    {
        try
        {
            setPropertyValue( *pNames++, *pValues++ );
        }
        catch( beans::UnknownPropertyException& )
        {
            // ignore for multi property set
            // todo: optimize this!
        }
    }
}

Sequence< Any > SAL_CALL SdPage::getPropertyValues( const Sequence< OUString >& aPropertyNames )
{
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence< Any > aValues( nCount );
    std::transform(aPropertyNames.begin(), aPropertyNames.end(), aValues.begin(),
        [this](const OUString& rName) -> Any {
            Any aValue;
            try
            {
                aValue = getPropertyValue(rName);
            }
            catch( beans::UnknownPropertyException& )
            {
                // ignore for multi property set
                // todo: optimize this!
            }
            return aValue;
        });
    return aValues;
}

void SAL_CALL SdPage::addPropertiesChangeListener( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SdPage::removePropertiesChangeListener( const Reference< beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SdPage::firePropertiesChangeEvent( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  )
{
}

Reference< drawing::XShape >  SdPage::CreateShape(SdrObject *pObj) const
{
    DBG_ASSERT( pObj, "SdPage::CreateShape(), invalid call with pObj == 0!" );

    if (!pObj)
        return Reference< drawing::XShape >();

    PresObjKind eKind = GetPresObjKind(pObj);

    SvxShape* pShape = nullptr;

    if(pObj->GetObjInventor() == SdrInventor::Default)
    {
        sal_uInt32 nInventor = pObj->GetObjIdentifier();
        switch( nInventor )
        {
        case OBJ_TITLETEXT:
            pShape = new SvxShapeText( pObj );
            if( GetPageKind() == PageKind::Notes && IsMasterPage() )
            {
                // fake an empty PageShape if it's a title shape on the master page
                pShape->SetShapeType("com.sun.star.presentation.PageShape");
            }
            else
            {
                pShape->SetShapeType("com.sun.star.presentation.TitleTextShape");
            }
            eKind = PresObjKind::NONE;
            break;
        case OBJ_OUTLINETEXT:
            pShape = new SvxShapeText( pObj );
            pShape->SetShapeType("com.sun.star.presentation.OutlinerShape");
            eKind = PresObjKind::NONE;
            break;
        }
    }

    Reference< drawing::XShape >  xShape( pShape );

    if(!xShape.is())
        xShape = FmFormPage::CreateShape( pObj );

    if( eKind != PresObjKind::NONE )
    {
        OUString aShapeType("com.sun.star.presentation.");

        switch( eKind )
        {
        case PresObjKind::Title:
            aShapeType += "TitleTextShape";
            break;
        case PresObjKind::Outline:
            aShapeType += "OutlinerShape";
            break;
        case PresObjKind::Text:
            aShapeType += "SubtitleShape";
            break;
        case PresObjKind::Graphic:
            aShapeType += "GraphicObjectShape";
            break;
        case PresObjKind::Object:
            aShapeType += "OLE2Shape";
            break;
        case PresObjKind::Chart:
            aShapeType += "ChartShape";
            break;
        case PresObjKind::OrgChart:
            aShapeType += "OrgChartShape";
            break;
        case PresObjKind::Calc:
            aShapeType += "CalcShape";
            break;
        case PresObjKind::Table:
            aShapeType += "TableShape";
            break;
        case PresObjKind::Media:
            aShapeType += "MediaShape";
            break;
        case PresObjKind::Page:
            aShapeType += "PageShape";
            break;
        case PresObjKind::Handout:
            aShapeType += "HandoutShape";
            break;
        case PresObjKind::Notes:
            aShapeType += "NotesShape";
            break;
        case PresObjKind::Footer:
            aShapeType += "FooterShape";
            break;
        case PresObjKind::Header:
            aShapeType += "HeaderShape";
            break;
        case PresObjKind::SlideNumber:
            aShapeType += "SlideNumberShape";
            break;
        case PresObjKind::DateTime:
            aShapeType += "DateTimeShape";
            break;
        // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
        case PresObjKind::NONE:
            break;
        }

        if( !pShape )
            pShape = comphelper::getUnoTunnelImplementation<SvxShape>( xShape );

        if( pShape )
            pShape->SetShapeType( aShapeType );
    }

    SvxShape *pSdShape = comphelper::getUnoTunnelImplementation<SvxShape>(xShape);
    if (pSdShape)
    {
        // SdXShape aggregates SvxShape
        new SdXShape(pSdShape, GetModel());
    }
    return xShape;
}

// XServiceInfo
Sequence< OUString > SAL_CALL SdPage::getSupportedServiceNames()
{
    return comphelper::concatSequences(
        FmFormPage::getSupportedServiceNames(),
        std::initializer_list<std::u16string_view>{ u"com.sun.star.drawing.GenericDrawPage",
                                          u"com.sun.star.document.LinkTarget",
                                          u"com.sun.star.document.LinkTargetSupplier" });
}

// XLinkTargetSupplier
Reference< container::XNameAccess > SAL_CALL SdPage::getLinks(  )
{
    return new SdPageLinkTargets( this );
}

void SdPage::setBackground( const Any& )
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

void SdPage::getBackground( Any& )
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

OUString SdPage::getBookmarkURL() const
{
    OUStringBuffer aRet;
    OUString aFileName( GetFileName() );
    if( !aFileName.isEmpty() )
    {
        const OUString aBookmarkName( SdNotMasterPage::getPageApiNameFromUiName( GetBookmarkName() ) );
        aRet.append( aFileName );
        aRet.append( '#' );
        aRet.append( aBookmarkName );
    }

    return aRet.makeStringAndClear();
}

void SdPage::setBookmarkURL( OUString const & rURL )
{
    sal_Int32 nIndex = rURL.indexOf( '#' );
    if( nIndex == -1 )
        return;

    const OUString aFileName( rURL.copy( 0, nIndex ) );
    const OUString aBookmarkName( SdNotMasterPage::getUiNameFromPageApiName( rURL.copy( nIndex+1 )  ) );

    if( !aFileName.isEmpty() && !aBookmarkName.isEmpty() )
    {
        DisconnectLink();
        SetFileName( aFileName );
        SetBookmarkName( aBookmarkName );
        ConnectLink();
    }
}

Reference< drawing::XShape > SAL_CALL SdPage::combine( const Reference< drawing::XShapes >& xShapes )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    DBG_ASSERT(mpView, "SdrView is NULL! [CL]");

    Reference< drawing::XShape > xShape;
    if(mpView==nullptr||!xShapes.is())
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( this );

    SelectObjectsInView( xShapes, pPageView );

    mpView->CombineMarkedObjects( false );

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
            xShape.set( pObj->getUnoShape(), UNO_QUERY );
    }

    mpView->HideSdrPage();

    GetModel()->SetModified();

    return xShape;
}

void SAL_CALL SdPage::split( const Reference< drawing::XShape >& xGroup )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==nullptr||!xGroup.is())
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( this );
    SelectObjectInView( xGroup, pPageView );
    mpView->DismantleMarkedObjects();
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

Reference< drawing::XShape > SAL_CALL SdPage::bind( const Reference< drawing::XShapes >& xShapes )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Reference< drawing::XShape > xShape;
    if(mpView==nullptr||!xShapes.is())
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( this );

    SelectObjectsInView( xShapes, pPageView );

    mpView->CombineMarkedObjects();

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
            xShape.set( pObj->getUnoShape(), UNO_QUERY );
    }

    mpView->HideSdrPage();

    GetModel()->SetModified();

    return xShape;
}

void SAL_CALL SdPage::unbind( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==nullptr||!xShape.is())
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( this );
    SelectObjectInView( xShape, pPageView );
    mpView->DismantleMarkedObjects( true );
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

void SdPage::SetLeftBorder( sal_Int32 nValue )
{
    if( nValue == GetLeftBorder() )
        return;
    FmFormPage::SetLeftBorder(nValue);

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        pPage->SetLeftBorder( nValue );
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        pPage->SetLeftBorder( nValue );
    }
}

void SdPage::SetRightBorder( sal_Int32 nValue )
{
    if( nValue == GetRightBorder() )
        return;
    FmFormPage::SetRightBorder(nValue);

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        if (pPage != this)
            pPage->SetRightBorder( nValue );
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        if (pPage != this)
            pPage->SetRightBorder( nValue );
    }
}

void SdPage::SetUpperBorder( sal_Int32 nValue )
{
    if( nValue == GetUpperBorder() )
        return;
    FmFormPage::SetUpperBorder(nValue);

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        pPage->SetUpperBorder( nValue );
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        pPage->SetUpperBorder( nValue );
    }
}

void SdPage::SetLowerBorder( sal_Int32 nValue )
{
    if( nValue == GetLowerBorder() )
        return;
    FmFormPage::SetLowerBorder(nValue);

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        pPage->SetLowerBorder( nValue );
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        pPage->SetLowerBorder( nValue );
    }
}

static void refreshpage( SdDrawDocument* pDoc, const PageKind ePageKind )
{
    ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
    if ( !pDocShell )
        return;

    ::sd::ViewShell* pViewSh = pDocShell->GetViewShell();

    if( !pViewSh )
        return;

    if( auto pDrawViewShell = dynamic_cast<::sd::DrawViewShell* >(pViewSh) )
        pDrawViewShell->ResetActualPage();

    Size aPageSize = pDoc->GetSdPage(0, ePageKind)->GetSize();
    const tools::Long nWidth = aPageSize.Width();
    const tools::Long nHeight = aPageSize.Height();

    Point aPageOrg(nWidth, nHeight / 2);
    Size aViewSize(nWidth * 3, nHeight * 2);

    pDoc->SetMaxObjSize(aViewSize);

    pViewSh->InitWindows(aPageOrg, aViewSize, Point(-1, -1), true);

    pViewSh->UpdateScrollBars();
}

void SdPage::SetWidth( sal_Int32 nWidth )
{
    Size aSize( GetSize() );
    if( aSize.getWidth() == nWidth )
        return;

    aSize.setWidth( nWidth );

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        pPage->SetSize(aSize);
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        pPage->SetSize(aSize);
    }

    refreshpage( &rDoc, ePageKind );
}

void SdPage::SetHeight( sal_Int32 nHeight )
{
    Size aSize( GetSize() );
    if( aSize.getHeight() == nHeight )
        return;

    aSize.setHeight( nHeight );

    SdDrawDocument& rDoc(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()));
    const PageKind ePageKind = GetPageKind();

    sal_uInt16 i, nPageCnt = rDoc.GetMasterSdPageCount(ePageKind);
    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetMasterSdPage(i, ePageKind);
        pPage->SetSize(aSize);
    }

    nPageCnt = rDoc.GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        SdPage* pPage = rDoc.GetSdPage(i, ePageKind);
        pPage->SetSize(aSize);
    }

    refreshpage( &rDoc, ePageKind );
}

// XComponent
void SdPage::disposing() throw()
{
    mpDocModel = nullptr;
    FmFormPage::disposing();
}


// SdPageLinkTargets
SdPageLinkTargets::SdPageLinkTargets( SdPage* pUnoPage ) throw()
{
    mxPage = pUnoPage;
}

SdPageLinkTargets::~SdPageLinkTargets() throw()
{
}

// XElementAccess
uno::Type SAL_CALL SdPageLinkTargets::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL SdPageLinkTargets::hasElements()
{
    ::SolarMutexGuard aGuard;

    SdrObjListIter aIter( mxPage.get(), SdrIterMode::DeepWithGroups );

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OUString aStr( pObj->GetName() );
        if( aStr.isEmpty() )
            if (auto pOleObj = dynamic_cast< const SdrOle2Obj *>( pObj ))
                aStr = pOleObj->GetPersistName();
        if( !aStr.isEmpty() )
            return true;
    }

    return false;
}

// container::XNameAccess

// XNameAccess
Any SAL_CALL SdPageLinkTargets::getByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    SdPage* pPage = mxPage.get();
    if( pPage != nullptr )
    {
        SdrObject* pObj = FindObject( aName );
        if( pObj )
        {
            Reference< beans::XPropertySet > aRef( pObj->getUnoShape(), uno::UNO_QUERY );
            return makeAny( aRef );
        }
    }

    throw container::NoSuchElementException();
}

Sequence< OUString > SAL_CALL SdPageLinkTargets::getElementNames()
{
    ::SolarMutexGuard aGuard;

    sal_uInt32 nObjCount = 0;

    {
        SdrObjListIter aIter( mxPage.get(), SdrIterMode::DeepWithGroups );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            OUString aStr( pObj->GetName() );
            if( aStr.isEmpty() )
                if (auto pOleObj = dynamic_cast< const SdrOle2Obj *>( pObj ))
                    aStr = pOleObj->GetPersistName();
            if( !aStr.isEmpty() )
                nObjCount++;
        }
    }

    Sequence< OUString > aSeq( nObjCount );
    if( nObjCount > 0 )
    {
        OUString* pStr = aSeq.getArray();

        SdrObjListIter aIter( mxPage.get(), SdrIterMode::DeepWithGroups );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            OUString aStr( pObj->GetName() );
            if( aStr.isEmpty() )
                if (auto pOleObj = dynamic_cast< const SdrOle2Obj *>( pObj ))
                    aStr = pOleObj->GetPersistName();
            if( !aStr.isEmpty() )
                *pStr++ = aStr;
        }
    }

    return aSeq;
}

sal_Bool SAL_CALL SdPageLinkTargets::hasByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    return FindObject( aName ) != nullptr;
}

SdrObject* SdPageLinkTargets::FindObject( std::u16string_view rName ) const throw()
{
    SdPage* pPage = mxPage.get();
    if( pPage == nullptr )
        return nullptr;

    SdrObjListIter aIter( mxPage.get(), SdrIterMode::DeepWithGroups );

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OUString aStr( pObj->GetName() );
        if( aStr.isEmpty() )
            if (auto pOleObj = dynamic_cast< const SdrOle2Obj *>( pObj ))
                aStr = pOleObj->GetPersistName();
        if( !aStr.isEmpty() && (aStr == rName) )
            return pObj;
    }

    return nullptr;
}

// XServiceInfo
OUString SAL_CALL SdPageLinkTargets::getImplementationName()
{
    return "SdPageLinkTargets";
}

sal_Bool SAL_CALL SdPageLinkTargets::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdPageLinkTargets::getSupportedServiceNames()
{
  return { "com.sun.star.document.LinkTargets" };
}

SdNotMasterPage::SdNotMasterPage(SdXImpressDocument* pModel)
    : SdPage( pModel, /*master*/false, ImplGetDrawPagePropertySet( pModel->IsImpressDocument(), PageKind::Standard ) )
{
}

SdNotMasterPage::~SdNotMasterPage() throw()
{
}

// XInterface
Any SAL_CALL SdNotMasterPage::queryInterface( const uno::Type & rType )
{
    if( rType == cppu::UnoType<drawing::XMasterPageTarget>::get() )
    {
        return makeAny( Reference< drawing::XMasterPageTarget >( this ) );
    }
    else if( IsImpressDocument()
             && rType == cppu::UnoType<presentation::XPresentationPage>::get() )
    {
        if( GetPageKind() != PageKind::Handout )
        {
            return makeAny( Reference< presentation::XPresentationPage >( this ) );
        }
    }

    return SdPage::queryInterface( rType );
}

UNO3_GETIMPLEMENTATION2_IMPL( SdNotMasterPage, SdPage );

// XTypeProvider
Sequence< uno::Type > SAL_CALL SdNotMasterPage::getTypes()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( !maTypeSequence.hasElements() )
    {
        const PageKind ePageKind = GetPageKind();
        bool bPresPage = IsImpressDocument() && ePageKind != PageKind::Handout;

        // Collect the types of this class.
        ::std::vector<uno::Type> aTypes;
        aTypes.reserve(13);
        aTypes.push_back(cppu::UnoType<drawing::XDrawPage>::get());
        aTypes.push_back(cppu::UnoType<beans::XPropertySet>::get());
        aTypes.push_back(cppu::UnoType<container::XNamed>::get());
        aTypes.push_back(cppu::UnoType<drawing::XMasterPageTarget>::get());
        aTypes.push_back(cppu::UnoType<lang::XServiceInfo>::get());
        aTypes.push_back(cppu::UnoType<util::XReplaceable>::get());
        aTypes.push_back(cppu::UnoType<document::XLinkTargetSupplier>::get());
        aTypes.push_back(cppu::UnoType<drawing::XShapeCombiner>::get());
        aTypes.push_back(cppu::UnoType<drawing::XShapeBinder>::get());
        aTypes.push_back(cppu::UnoType<office::XAnnotationAccess>::get());
        aTypes.push_back(cppu::UnoType<beans::XMultiPropertySet>::get());
        if( bPresPage )
            aTypes.push_back(cppu::UnoType<presentation::XPresentationPage>::get());
        if( bPresPage && ePageKind == PageKind::Standard )
            aTypes.push_back(cppu::UnoType<XAnimationNodeSupplier>::get());

        // Get types of base class.
        // Join those types in a sequence.
        return comphelper::concatSequences(
            comphelper::containerToSequence(aTypes),
            SdPage::getTypes() );
    }

    return maTypeSequence;
}

Sequence< sal_Int8 > SAL_CALL SdNotMasterPage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SdNotMasterPage::getPageApiName( SdPage* pPage )
{
    return ::getPageApiName( pPage );
}

OUString getPageApiName( SdPage* pPage )
{
    OUString aPageName;

    if(pPage)
    {
        aPageName = pPage->GetRealName();

        if( aPageName.isEmpty() )
        {
            const sal_Int32 nPageNum = ( ( pPage->GetPageNum() - 1 ) >> 1 ) + 1;
            aPageName = sEmptyPageName + OUString::number( nPageNum );
        }
    }

    return aPageName;
}

OUString getPageApiNameFromUiName( const OUString& rUIName )
{
    OUString aApiName;

    OUString aDefPageName(SdResId(STR_PAGE) + " ");

    if( rUIName.startsWith( aDefPageName ) )
    {
        aApiName = OUString::Concat(sEmptyPageName) + rUIName.subView( aDefPageName.getLength() );
    }
    else
    {
        aApiName = rUIName;
    }

    return aApiName;
}

OUString SdNotMasterPage::getPageApiNameFromUiName( const OUString& rUIName )
{
    return ::getPageApiNameFromUiName( rUIName );
}

OUString getUiNameFromPageApiNameImpl( const OUString& rApiName )
{
    const OUString aDefPageName( sEmptyPageName );
    if( rApiName.startsWith( aDefPageName ) )
    {
        OUString aNumber( rApiName.copy( aDefPageName.getLength() ) );

        // create the page number
        sal_Int32 nPageNumber = aNumber.toInt32();

        // check if there are non number characters in the number part
        const sal_Int32 nChars = aNumber.getLength();
        const sal_Unicode* pString = aNumber.getStr();
        sal_Int32 nChar;
        for( nChar = 0; nChar < nChars; nChar++, pString++ )
        {
            if((*pString < '0') || (*pString > '9'))
            {
                // found a non number character, so this is not the default
                // name for this page
                nPageNumber = -1;
                break;
            }
        }

        if( nPageNumber != -1)
        {
            return SdResId(STR_PAGE) + " " + aNumber;
        }
    }

    return rApiName;
}

OUString SdNotMasterPage::getUiNameFromPageApiName( const OUString& rApiName )
{
    return getUiNameFromPageApiNameImpl( rApiName );
}

// XServiceInfo
OUString SAL_CALL SdNotMasterPage::getImplementationName()
{
    return "SdDrawPage";
}

Sequence< OUString > SAL_CALL SdNotMasterPage::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    std::vector<std::u16string_view> aAdd{ u"com.sun.star.drawing.DrawPage" };

    if( IsImpressDocument() )
        aAdd.emplace_back(u"com.sun.star.presentation.DrawPage");

    return comphelper::concatSequences(SdPage::getSupportedServiceNames(), aAdd);
}

sal_Bool SAL_CALL SdNotMasterPage::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

// XNamed
void SAL_CALL SdNotMasterPage::setName( const OUString& rName )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(GetPageKind() == PageKind::Notes)
        return;

    // check if this is the default 'page1234' name
    OUString aName(rName);
    OUString aNumber;
    if(aName.startsWith( sEmptyPageName, &aNumber ))
    {
        // ok, it maybe is, aNumber is the number part after 'page'

        // create the page number
        sal_Int32 nPageNumber = aNumber.toInt32();

        // check if there are non number characters in the number part
        const sal_Int32 nChars = aNumber.getLength();
        const sal_Unicode* pString = aNumber.getStr();
        sal_Int32 nChar;
        for( nChar = 0; nChar < nChars; nChar++, pString++ )
        {
            if((*pString < '0') || (*pString > '9'))
            {
                // found a non number character, so this is not the default
                // name for this page
                nPageNumber = -1;
                break;
            }
        }

        if( nPageNumber == ( ( GetPageNum() - 1 ) >> 1 ) + 1 )
            aName.clear();
    }
    else
    {
        OUString aDefaultPageName( SdResId(STR_PAGE) + " " );
        if( aName.startsWith( aDefaultPageName ) )
            aName.clear();
    }

    FmFormPage::setName( aName );

    sal_uInt16 nNotesPageNum = (GetPageNum()-1)>>1;
    if( GetModel()->GetDoc()->GetSdPageCount( PageKind::Notes ) > nNotesPageNum )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( nNotesPageNum, PageKind::Notes );
        if( pNotesPage )
            pNotesPage->setName(aName);
    }

    // fake a mode change to repaint the page tab bar
    ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
    ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : nullptr;
    if( auto pDrawViewSh = dynamic_cast<::sd::DrawViewShell* >(pViewSh) )
    {
        EditMode eMode = pDrawViewSh->GetEditMode();
        if( eMode == EditMode::Page )
        {
            bool bLayer = pDrawViewSh->IsLayerModeActive();

            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }
    }

    GetModel()->SetModified();
}

OUString SAL_CALL SdNotMasterPage::getName()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return getPageApiName( this );
}

// XMasterPageTarget
Reference< drawing::XDrawPage > SAL_CALL SdNotMasterPage::getMasterPage(  )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    Reference< drawing::XDrawPage > xPage;

    if(TRG_HasMasterPage())
    {
        SdrPage& rMasterPage = TRG_GetMasterPage();
        xPage = &rMasterPage;
    }

    return xPage;
}

void SAL_CALL SdNotMasterPage::setMasterPage( const Reference< drawing::XDrawPage >& xMasterPage )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("setMasterPage");

    throwIfDisposed();

    SdMasterPage* pMasterPage = comphelper::getUnoTunnelImplementation<SdMasterPage>( xMasterPage );
    if( !pMasterPage )
        return;

    TRG_ClearMasterPage();

    SdPage* pSdPage = pMasterPage;
    TRG_SetMasterPage(*pSdPage);

    SetBorder(pSdPage->GetLeftBorder(),pSdPage->GetUpperBorder(),
                      pSdPage->GetRightBorder(),pSdPage->GetLowerBorder() );

    SetSize( pSdPage->GetSize() );
    SetOrientation( pSdPage->GetOrientation() );
    SetLayoutName( pSdPage->GetLayoutName() );

    // set notes master also
    SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (GetPageNum()-1)>>1, PageKind::Notes );

    pNotesPage->TRG_ClearMasterPage();
    sal_uInt16 nNum = TRG_GetMasterPage().GetPageNum() + 1;
    pNotesPage->TRG_SetMasterPage(*getSdrModelFromSdrPage().GetMasterPage(nNum));
    pNotesPage->SetLayoutName( pSdPage->GetLayoutName() );

    GetModel()->SetModified();
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdNotMasterPage::getNotesPage()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(GetModel()->GetDoc() && GetPageNum() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (GetPageNum()-1)>>1, PageKind::Notes );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage );
            return xPage;
        }
    }
    return nullptr;
}

// XIndexAccess
sal_Int32 SAL_CALL SdNotMasterPage::getCount()
{
    return SdPage::getCount();
}

Any SAL_CALL SdNotMasterPage::getByIndex( sal_Int32 Index )
{
    return SdPage::getByIndex( Index );
}

// XElementAccess
uno::Type SAL_CALL SdNotMasterPage::getElementType()
{
    return SdPage::getElementType();
}

sal_Bool SAL_CALL SdNotMasterPage::hasElements()
{
    return SdPage::hasElements();
}

// XShapes
void SAL_CALL SdNotMasterPage::add( const Reference< drawing::XShape >& xShape )
{
    SdPage::add( xShape );
}

void SAL_CALL SdNotMasterPage::remove( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xShape );
    if( pObj )
    {
        RemovePresObj(pObj);
        pObj->SetUserCall(nullptr);
    }

    SdPage::remove( xShape );
}

void SdNotMasterPage::setBackground( const Any& rValue )
{
    Reference< beans::XPropertySet > xSet;

    if( !(rValue >>= xSet) && !rValue.hasValue() )
        throw lang::IllegalArgumentException();

    if( !xSet.is() )
    {
        // the easy case, no background set. Set drawing::FillStyle_NONE to represent this
        getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));
        return;
    }

    // is it our own implementation?
    SdUnoPageBackground* pBack = comphelper::getUnoTunnelImplementation<SdUnoPageBackground>( xSet );

    SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{} );

    if( pBack )
    {
        pBack->fillItemSet( static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage()), aSet );
    }
    else
    {
        SdUnoPageBackground* pBackground = new SdUnoPageBackground();

        Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
        Reference< beans::XPropertySet >  xDestSet( static_cast<beans::XPropertySet*>(pBackground) );
        Reference< beans::XPropertySetInfo >  xDestSetInfo( xDestSet->getPropertySetInfo() );

        const Sequence< beans::Property > aProperties( xDestSetInfo->getProperties() );

        for( const beans::Property& rProp : aProperties )
        {
            const OUString aPropName( rProp.Name );
            if( xSetInfo->hasPropertyByName( aPropName ) )
                xDestSet->setPropertyValue( aPropName,
                        xSet->getPropertyValue( aPropName ) );
        }

        pBackground->fillItemSet( static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage()), aSet );
    }

    if( aSet.Count() == 0 )
    {
        // no background fill, represent by setting drawing::FillStyle_NONE
        getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));
    }
    else
    {
        // background fill, set at page (not sure if ClearItem is needed)
        getSdrPageProperties().ClearItem();
        getSdrPageProperties().PutItemSet(aSet);
    }

    // repaint only
    ActionChanged();
}

// XAnnotationAccess:
Reference< XAnnotation > SAL_CALL SdPage::createAndInsertAnnotation()
{
    Reference< XAnnotation > xRet;
    createAnnotation(xRet);
    return xRet;
}

Reference< XAnnotationEnumeration > SAL_CALL SdPage::createAnnotationEnumeration()
{
    return ::sd::createAnnotationEnumeration( getAnnotations() );
}

void SdNotMasterPage::getBackground(Any& rValue)
{
    const SfxItemSet& rFillAttributes = getSdrPageProperties().GetItemSet();

    if(drawing::FillStyle_NONE == rFillAttributes.Get(XATTR_FILLSTYLE).GetValue())
    {
        // no fill set (switched off by drawing::FillStyle_NONE), clear rValue to represent this
        rValue.clear();
    }
    else
    {
        // there is a fill set, export to rValue
        Reference< beans::XPropertySet > xSet(new SdUnoPageBackground(
            GetModel()->GetDoc(),
            &getSdrPageProperties().GetItemSet()));
        rValue <<= xSet;
    }
}

void SdPage::setNavigationOrder( const Any& rValue )
{
    Reference< XIndexAccess > xIA( rValue, UNO_QUERY );
    if( xIA.is() )
    {
        if( dynamic_cast< SdNotMasterPage* >( xIA.get() ) == this )
        {
            if( HasObjectNavigationOrder() )
                ClearObjectNavigationOrder();

            return;
        }
        else if( static_cast<size_t>(xIA->getCount()) == GetObjCount() )
        {
            setNavigationOrder(Any(xIA));
            return;
        }
    }
    throw IllegalArgumentException();
}

namespace {

class SdNavigationOrderAccess : public ::cppu::WeakImplHelper< XIndexAccess >
{
public:
    explicit SdNavigationOrderAccess(SdrPage const * pPage);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

private:
    std::vector< Reference< XShape > > maShapes;
};

}

SdNavigationOrderAccess::SdNavigationOrderAccess( SdrPage const * pPage )
: maShapes( pPage ? pPage->GetObjCount() : 0 )
{
    if( pPage )
    {
        const size_t nCount = pPage->GetObjCount();
        for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
        {
            SdrObject* pObj = pPage->GetObj( nIndex );
            sal_uInt32 nNavPos = pObj->GetNavigationPosition();
            DBG_ASSERT( !maShapes[nNavPos].is(), "sd::SdNavigationOrderAccess::SdNavigationOrderAccess(), duplicate navigation positions from core!" );
            maShapes[nNavPos].set( pObj->getUnoShape(), UNO_QUERY );
        }
    }
}

// XIndexAccess
sal_Int32 SAL_CALL SdNavigationOrderAccess::getCount(  )
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

Any SAL_CALL SdNavigationOrderAccess::getByIndex( sal_Int32 Index )
{
    if( (Index < 0) || (Index > getCount()) )
        throw IndexOutOfBoundsException();

    return Any( maShapes[Index] );
}

// XElementAccess
Type SAL_CALL SdNavigationOrderAccess::getElementType(  )
{
    return cppu::UnoType<XShape>::get();
}

sal_Bool SAL_CALL SdNavigationOrderAccess::hasElements(  )
{
    return !maShapes.empty();
}

Any SdPage::getNavigationOrder()
{
    if( HasObjectNavigationOrder() )
    {
        return Any( Reference< XIndexAccess >( new SdNavigationOrderAccess( this ) ) );
    }
    else
    {
        return Any( Reference< XIndexAccess >( this ) );
    }
}

SdMasterPage::SdMasterPage(SdXImpressDocument* pSfxModel)
    : SdPage(pSfxModel, /*master*/true, ImplGetMasterPagePropertySet(PageKind::Standard))
{
}

SdMasterPage::~SdMasterPage() throw()
{
}

// XInterface
Any SAL_CALL SdMasterPage::queryInterface( const uno::Type & rType )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Any aAny;

    if( rType == cppu::UnoType<container::XIndexAccess>::get() )
        aAny <<= Reference< container::XIndexAccess >(static_cast<presentation::XPresentationPage*>(this));
    else if( rType == cppu::UnoType<container::XElementAccess>::get() )
        aAny <<=  Reference< container::XElementAccess >(static_cast<presentation::XPresentationPage*>(this));
    else if( rType == cppu::UnoType<container::XNamed>::get() )
        aAny <<=  Reference< container::XNamed >(this);
    else if( rType == cppu::UnoType<presentation::XPresentationPage>::get() &&
             ( IsImpressDocument() &&
               GetPageKind() != PageKind::Handout) )
        aAny <<= Reference< presentation::XPresentationPage >( this );
    else
        return SdPage::queryInterface( rType );

    return aAny;
}

UNO3_GETIMPLEMENTATION2_IMPL( SdMasterPage, SdPage );

// XTypeProvider
Sequence< uno::Type > SAL_CALL SdMasterPage::getTypes()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( !maTypeSequence.hasElements() )
    {
        const PageKind ePageKind = GetPageKind();
        bool bPresPage = IsImpressDocument() && ePageKind != PageKind::Handout;

        // Collect the types of this class.
        ::std::vector<uno::Type> aTypes;
        aTypes.reserve(12);
        aTypes.push_back(cppu::UnoType<drawing::XDrawPage>::get());
        aTypes.push_back(cppu::UnoType<beans::XPropertySet>::get());
        aTypes.push_back(cppu::UnoType<container::XNamed>::get());
        aTypes.push_back(cppu::UnoType<lang::XServiceInfo>::get());
        aTypes.push_back(cppu::UnoType<util::XReplaceable>::get());
        aTypes.push_back(cppu::UnoType<document::XLinkTargetSupplier>::get());
        aTypes.push_back(cppu::UnoType<drawing::XShapeCombiner>::get());
        aTypes.push_back(cppu::UnoType<drawing::XShapeBinder>::get());
        aTypes.push_back(cppu::UnoType<office::XAnnotationAccess>::get());
        aTypes.push_back(cppu::UnoType<beans::XMultiPropertySet>::get());
        if( bPresPage )
            aTypes.push_back(cppu::UnoType<presentation::XPresentationPage>::get());
        if( bPresPage && ePageKind == PageKind::Standard )
            aTypes.push_back(cppu::UnoType<XAnimationNodeSupplier>::get());

        // Get types of base class.
        // Join those types in a sequence.
        return comphelper::concatSequences(
            comphelper::containerToSequence(aTypes),
            SdPage::getTypes() );
    }

    return maTypeSequence;
}

Sequence< sal_Int8 > SAL_CALL SdMasterPage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
OUString SAL_CALL SdMasterPage::getImplementationName()
{
    return "SdMasterPage";
}

Sequence< OUString > SAL_CALL SdMasterPage::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    std::vector<std::u16string_view> aAdd{ u"com.sun.star.drawing.MasterPage" };

    if( GetPageKind() == PageKind::Handout )
        aAdd.emplace_back(u"com.sun.star.presentation.HandoutMasterPage");

    return comphelper::concatSequences(SdPage::getSupportedServiceNames(), aAdd);
}

sal_Bool SAL_CALL SdMasterPage::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

// XElementAccess
sal_Bool SAL_CALL SdMasterPage::hasElements()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return GetObjCount() > 0;
}

uno::Type SAL_CALL SdMasterPage::getElementType()
{
    return SdPage::getElementType();
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPage::getCount()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdPage::getCount();
}

Any SAL_CALL SdMasterPage::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdPage::getByIndex(Index);
}

// intern
void SdMasterPage::setBackground( const Any& rValue )
{
    // we need at least a beans::XPropertySet
    Reference< beans::XPropertySet > xInputSet( rValue, UNO_QUERY );
    if( !xInputSet.is() )
        throw lang::IllegalArgumentException();

    try
    {
        if( GetModel() && IsImpressDocument() )
        {
            Reference< container::XNameAccess >  xFamilies( GetModel()->getStyleFamilies(), UNO_SET_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW ) ;

            Reference< beans::XPropertySet >  xStyleSet( xFamily->getByName( sUNO_PseudoSheet_Background ), UNO_QUERY_THROW );

            Reference< beans::XPropertySetInfo >  xSetInfo( xInputSet->getPropertySetInfo(), UNO_SET_THROW );
            Reference< beans::XPropertyState > xSetStates( xInputSet, UNO_QUERY );

            PropertyEntryVector_t aBackgroundProperties = ImplGetPageBackgroundPropertySet()->getPropertyMap().getPropertyEntries();
            for( const auto& rProp : aBackgroundProperties )
            {
                if( xSetInfo->hasPropertyByName( rProp.sName ) )
                {
                    if( !xSetStates.is() || xSetStates->getPropertyState( rProp.sName ) == beans::PropertyState_DIRECT_VALUE )
                        xStyleSet->setPropertyValue( rProp.sName,    xInputSet->getPropertyValue( rProp.sName ) );
                    else
                        xSetStates->setPropertyToDefault( rProp.sName );
                }
            }
        }
        else
        {
            // first fill an item set
            // is it our own implementation?
            SdUnoPageBackground* pBack = comphelper::getUnoTunnelImplementation<SdUnoPageBackground>( xInputSet );

            SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{} );

            if( pBack )
            {
                pBack->fillItemSet( static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage()), aSet );
            }
            else
            {
                SdUnoPageBackground* pBackground = new SdUnoPageBackground();

                Reference< beans::XPropertySetInfo > xInputSetInfo( xInputSet->getPropertySetInfo(), UNO_SET_THROW );
                Reference< beans::XPropertySet > xDestSet( static_cast<beans::XPropertySet*>(pBackground) );
                Reference< beans::XPropertySetInfo > xDestSetInfo( xDestSet->getPropertySetInfo(), UNO_SET_THROW );

                const uno::Sequence< beans::Property> aProperties( xDestSetInfo->getProperties() );

                for( const beans::Property& rProp : aProperties )
                {
                    const OUString aPropName( rProp.Name );
                    if( xInputSetInfo->hasPropertyByName( aPropName ) )
                        xDestSet->setPropertyValue( aPropName, xInputSet->getPropertyValue( aPropName ) );
                }

                pBackground->fillItemSet( static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage()), aSet );
            }

            // if we find the background style, copy the set to the background
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage());
            SfxStyleSheetBasePool* pSSPool = pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                OUString aLayoutName( GetLayoutName() );
                aLayoutName = OUString::Concat(aLayoutName.subView(0, aLayoutName.indexOf(SD_LT_SEPARATOR)+4)) +
                    STR_LAYOUT_BACKGROUND;
                SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aLayoutName, SfxStyleFamily::Page );

                if( pStyleSheet )
                {
                    pStyleSheet->GetItemSet().Put( aSet );

                    // repaint only
                    ActionChanged();
                    return;
                }
            }

            // if no background style is available, set at page directly. This
            // is an error and should NOT happen (and will be asserted from the SdrPage)
            getSdrPageProperties().PutItemSet(aSet);
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SdMasterPage::setBackground()");
    }
}

void SdMasterPage::getBackground( Any& rValue )
{
    if( !GetModel() )
        return;

    try
    {
        if( IsImpressDocument() )
        {
            Reference< container::XNameAccess > xFamilies( GetModel()->getStyleFamilies(), UNO_SET_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW );

            rValue <<= Reference< beans::XPropertySet >( xFamily->getByName( sUNO_PseudoSheet_Background ), UNO_QUERY_THROW );
        }
        else
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(&getSdrModelFromSdrPage());
            SfxStyleSheetBasePool* pSSPool = pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                OUString aLayoutName( GetLayoutName() );
                aLayoutName = OUString::Concat(aLayoutName.subView(0, aLayoutName.indexOf(SD_LT_SEPARATOR)+4)) +
                    STR_LAYOUT_BACKGROUND;
                SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aLayoutName, SfxStyleFamily::Page );

                if( pStyleSheet )
                {
                    SfxItemSet aStyleSet( pStyleSheet->GetItemSet());
                    if( aStyleSet.Count() )
                    {
                        rValue <<= Reference< beans::XPropertySet >( new SdUnoPageBackground( pDoc, &aStyleSet ) );
                        return;
                    }
                }
            }

            // No style found, use fill attributes from page background. This
            // should NOT happen and is an error
            const SfxItemSet& rFallbackItemSet(getSdrPageProperties().GetItemSet());

            if(drawing::FillStyle_NONE == rFallbackItemSet.Get(XATTR_FILLSTYLE).GetValue())
            {
                rValue <<= Reference< beans::XPropertySet >(
                    new SdUnoPageBackground(GetModel()->GetDoc(), &rFallbackItemSet));
            }
            else
            {
                rValue.clear();
            }
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SdMasterPage::getBackground()");
        rValue.clear();
    }
}

// XNamed
void SAL_CALL SdMasterPage::setName( const OUString& rName )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(GetPageKind() == PageKind::Notes)
        return;

    SdDrawDocument* pDoc = GetModel()->GetDoc();
    bool bOutDummy;

    // Slide Name has to be unique
    if( pDoc && pDoc->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
        return; // throw Exception ?

    FmFormPage::setName( rName );

    if( pDoc )
        pDoc->RenameLayoutTemplate( GetLayoutName(), rName );

    // fake a mode change to repaint the page tab bar
    ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
    ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : nullptr;
    if( auto pDrawViewSh = dynamic_cast< ::sd::DrawViewShell* >(pViewSh) )
    {
        EditMode eMode = pDrawViewSh->GetEditMode();
        if( eMode == EditMode::MasterPage )
        {
            bool bLayer = pDrawViewSh->IsLayerModeActive();

            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }
    }

    GetModel()->SetModified();
}

OUString SAL_CALL SdMasterPage::getName(  )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    OUString aLayoutName( GetLayoutName() );
    return aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdMasterPage::getNotesPage()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(GetModel()->GetDoc() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetMasterSdPage( (GetPageNum()-1)>>1, PageKind::Notes );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage );
            return xPage;
        }
    }
    return nullptr;
}

// XShapes
void SAL_CALL SdMasterPage::add( const Reference< drawing::XShape >& xShape )
{
    SdPage::add( xShape );
}

void SAL_CALL SdMasterPage::remove( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xShape );
    if( pObj && IsPresObj( pObj ) )
        RemovePresObj(pObj);

    SdPage::remove( xShape );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

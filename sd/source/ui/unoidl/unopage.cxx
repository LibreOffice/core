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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/PresentationRange.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/metaact.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <AnnotationEnumeration.hxx>
#include <createunopageimpl.hxx>
#include <unomodel.hxx>
#include <unopage.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>
#include <svx/svdmodel.hxx>
#include <sdresid.hxx>
#include <glob.hrc>
#include <sdpage.hxx>
#include <unoprnms.hxx>
#include <sdattr.hxx>
#include <drawdoc.hxx>
#include <svx/unoshape.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <svx/svdorect.hxx>
#include <osl/mutex.hxx>
#include <svl/style.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <comphelper/extract.hxx>
#include <list>
#include <svx/svditer.hxx>
#include <vcl/wmf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdview.hxx>
#include "View.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "unoobj.hxx"
#include "res_bmp.hrc"
#include "unokywds.hxx"
#include "unopback.hxx"
#include <vcl/dibtools.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoashp.hxx>

using ::com::sun::star::animations::XAnimationNode;
using ::com::sun::star::animations::XAnimationNodeSupplier;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;

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

static sal_Char const sEmptyPageName[sizeof("page")] = "page";

// this function stores the property maps for draw pages in impress and draw
const SvxItemPropertySet* ImplGetDrawPagePropertySet( bool bImpress, PageKind ePageKind )
{
    static const SfxItemPropertyMapEntry aDrawPagePropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                  beans::PropertyAttribute::MAYBEVOID,0},
        { OUString(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_CHANGE),           WID_PAGE_CHANGE,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_DURATION),         WID_PAGE_DURATION,  ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_EFFECT),           WID_PAGE_EFFECT,    ::cppu::UnoType<presentation::FadeEffect>::get(),     0,  0},
        { OUString(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { OUString(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                          beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { OUString(UNO_NAME_PAGE_SPEED),            WID_PAGE_SPEED,     ::cppu::UnoType<presentation::AnimationSpeed>::get(), 0,  0},
        { OUString(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_PREVIEWBITMAP),    WID_PAGE_PREVIEWBITMAP, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_PREVIEWMETAFILE),  WID_PAGE_PREVIEWMETAFILE, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_VISIBLE),          WID_PAGE_VISIBLE,   cppu::UnoType<bool>::get(),                        0, 0},
        { OUString(UNO_NAME_OBJ_SOUNDFILE),         WID_PAGE_SOUNDFILE, cppu::UnoType<Any>::get(),              0, 0},
        { OUString(sUNO_Prop_IsBackgroundVisible),  WID_PAGE_BACKVIS,   cppu::UnoType<bool>::get(),                        0, 0},
        { OUString(sUNO_Prop_IsBackgroundObjectsVisible),   WID_PAGE_BACKOBJVIS,    cppu::UnoType<bool>::get(),                        0, 0},
        { OUString(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { OUString(sUNO_Prop_BookmarkURL),          WID_PAGE_BOOKMARK,  ::cppu::UnoType<OUString>::get(),             0,  0},
        { OUString("HighResDuration"),              WID_PAGE_HIGHRESDURATION,  ::cppu::UnoType<double>::get(),            0,  0},
        { OUString("IsBackgroundDark") ,            WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { OUString("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("FooterText"),                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { OUString("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},
        { OUString("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("DateTimeText"),                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},
        { OUString("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},
        { OUString("TransitionType"),               WID_TRANSITION_TYPE, ::cppu::UnoType<sal_Int16>::get(),           0,  0},
        { OUString("TransitionSubtype"),            WID_TRANSITION_SUBTYPE, ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { OUString("TransitionDirection"),          WID_TRANSITION_DIRECTION, ::cppu::UnoType<sal_Bool>::get(),           0,  0},
        { OUString("TransitionFadeColor"),          WID_TRANSITION_FADE_COLOR, ::cppu::UnoType<sal_Int32>::get(),         0,  0},
        { OUString("TransitionDuration"),           WID_TRANSITION_DURATION, ::cppu::UnoType<double>::get(),          0,  0},
        { OUString("LoopSound"),                    WID_LOOP_SOUND, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("NavigationOrder"),              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

#define DRAW_PAGE_NOTES_PROPERTIES \
        { OUString(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},                                                                \
        { OUString(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                          beans::PropertyAttribute::READONLY, 0},                                \
        { OUString(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},                                \
        { OUString(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},                                \
        { OUString(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},                                                                \
        { OUString(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                \
        { OUString(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),      0,     0},\
        { OUString("IsHeaderVisible"),              WID_PAGE_HEADERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { OUString("HeaderText"),                   WID_PAGE_HEADERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},                                                            \
        { OUString("IsBackgroundDark"),             WID_PAGE_ISDARK,     cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},                                \
        { OUString("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { OUString("FooterText"),                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},                                                            \
        { OUString("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},                                                             \
        { OUString("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},                                                                 \
        { OUString("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},                                                                 \
        { OUString("DateTimeText"),                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},                                                            \
        { OUString("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},                                                            \
        { OUString("NavigationOrder"),              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},                                                            \
        { OUString(), 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyMap_Impl[] =
    {
        // this must be the first two entries so they can be excluded for PageKind::Standard
        { OUString(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                  beans::PropertyAttribute::MAYBEVOID,0},
        DRAW_PAGE_NOTES_PROPERTIES
    };
    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyNoBackMap_Impl[] =
    {
        DRAW_PAGE_NOTES_PROPERTIES
    };

#define GRAPHIC_PAGE_PROPERTIES \
        { OUString(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                           beans::PropertyAttribute::READONLY, 0},                                             \
        { OUString(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},                                             \
        { OUString(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},                                             \
        { OUString(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},                                                                             \
        { OUString(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},                                                                             \
        { OUString(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},    \
        { OUString(UNO_NAME_PAGE_PREVIEWBITMAP),    WID_PAGE_PREVIEWBITMAP, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},\
        { OUString(UNO_NAME_PAGE_PREVIEWMETAFILE),  WID_PAGE_PREVIEWMETAFILE, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},\
        { OUString(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},                          \
        { OUString(sUNO_Prop_BookmarkURL),          WID_PAGE_BOOKMARK,  ::cppu::UnoType<OUString>::get(),             0,  0},                                                                             \
        { OUString("IsBackgroundDark"),             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},                                             \
        { OUString("NavigationOrder"),              WID_NAVORDER, cppu::UnoType<css::container::XIndexAccess>::get(),0,  0},                                                                         \
        { OUString(), 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry aGraphicPagePropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                   beans::PropertyAttribute::MAYBEVOID,0},
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
const SvxItemPropertySet* ImplGetMasterPagePropertySet( PageKind ePageKind )
{
    static const SfxItemPropertyMapEntry aMasterPagePropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      cppu::UnoType<beans::XPropertySet>::get(),                    0,  0},
        { OUString(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  cppu::UnoType<awt::XBitmap>::get(),                           beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    ::cppu::UnoType<OUString>::get(),             beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { OUString(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString("BackgroundFullSize"),           WID_PAGE_BACKFULL,  cppu::UnoType<bool>::get(),                        0, 0},
        { OUString(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { OUString("IsBackgroundDark"),             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    static const SfxItemPropertyMapEntry aHandoutMasterPagePropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    ::cppu::UnoType<view::PaperOrientation>::get(),0, 0},
        { OUString(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    ::cppu::UnoType<sal_Int16>::get(),            beans::PropertyAttribute::READONLY, 0},
        { OUString(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     ::cppu::UnoType<sal_Int32>::get(),            0,  0},
        { OUString(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    ::cppu::UnoType<sal_Int16>::get(),            0,  0},
        { OUString(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),         0,     0},
        { OUString("IsBackgroundDark"),             WID_PAGE_ISDARK,    cppu::UnoType<bool>::get(),                        beans::PropertyAttribute::READONLY, 0},
        { OUString("IsHeaderVisible"),              WID_PAGE_HEADERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("HeaderText"),                   WID_PAGE_HEADERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { OUString("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("FooterText"),                   WID_PAGE_FOOTERTEXT, ::cppu::UnoType<OUString>::get(),                0,  0},
        { OUString("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, cppu::UnoType<bool>::get(),                  0, 0},
        { OUString("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, cppu::UnoType<bool>::get(),                    0, 0},
        { OUString("DateTimeText"),                 WID_PAGE_DATETIMETEXT, ::cppu::UnoType<OUString>::get(),              0,  0},
        { OUString("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, ::cppu::UnoType<sal_Int32>::get(),           0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
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

const css::uno::Sequence< sal_Int8 > & SdGenericDrawPage::getUnoTunnelId() throw()
{
    return theSdGenericDrawPageUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SdGenericDrawPage::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
        if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),
            rId.getConstArray(), 16 ) )
        {
                return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
        }
        else
        {
                return SvxFmDrawPage::getSomething( rId );
        }
}

SdGenericDrawPage::SdGenericDrawPage( SdXImpressDocument* _pModel, SdPage* pInPage, const SvxItemPropertySet* _pSet ) throw()
:       SvxFmDrawPage( static_cast<SdrPage*>(pInPage) ),
        SdUnoSearchReplaceShape(this),
        mpModel     ( _pModel ),
        mpSdrModel(nullptr),
        mbIsImpressDocument(false),
        mnTempPageNumber(0),
        mpPropSet   ( _pSet )
{
    mpSdrModel = SvxFmDrawPage::mpModel;
    if( mpModel )
        mbIsImpressDocument = mpModel->IsImpressDocument();

}

SdGenericDrawPage::~SdGenericDrawPage() throw()
{
}

void SdGenericDrawPage::throwIfDisposed() const
{
    if( (SvxFmDrawPage::mpModel == nullptr) || (mpModel == nullptr) || (SvxFmDrawPage::mpPage == nullptr) )
        throw lang::DisposedException();
}

SdXImpressDocument* SdGenericDrawPage::GetModel() const
{
    if( mpSdrModel != SvxFmDrawPage::mpModel )
        const_cast<SdGenericDrawPage*>(this)->UpdateModel();
    return mpModel;
}

bool SdGenericDrawPage::IsImpressDocument() const
{
    if( mpSdrModel != SvxFmDrawPage::mpModel )
        const_cast<SdGenericDrawPage*>(this)->UpdateModel();
    return mbIsImpressDocument;
}


void SdGenericDrawPage::UpdateModel()
{
    mpSdrModel = SvxFmDrawPage::mpModel;
    if( mpSdrModel )
    {
        uno::Reference< uno::XInterface > xModel( SvxFmDrawPage::mpModel->getUnoModel() );
        mpModel = SdXImpressDocument::getImplementation( xModel );
    }
    else
    {
        mpModel = nullptr;
    }
    mbIsImpressDocument = mpModel && mpModel->IsImpressDocument();
}

// this is called whenever a SdrObject must be created for a empty api shape wrapper
SdrObject * SdGenericDrawPage::CreateSdrObject_( const Reference< drawing::XShape >& xShape )
{
    if( nullptr == SvxFmDrawPage::mpPage || !xShape.is() )
        return nullptr;

    OUString aType( xShape->getShapeType() );
    const OUString aPrefix( "com.sun.star.presentation." );
    if( !aType.startsWith( aPrefix ) )
    {
        SdrObject* pObj = SvxFmDrawPage::CreateSdrObject_( xShape );
        if( pObj && ( (pObj->GetObjInventor() != SdrInventor::Default) || (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
            if( pDoc )
            {
                // #i119287# similar to the code in the SdrObject methods the graphic and ole
                // SdrObjects need another default style than the rest, see task. Adding here, too.
                // TTTT: Same as for #i119287#: Can be removed in branch aw080 again
                const bool bIsSdrGrafObj(dynamic_cast< const SdrGrafObj* >(pObj) !=  nullptr);
                const bool bIsSdrOle2Obj(dynamic_cast< const SdrOle2Obj* >(pObj) !=  nullptr);

                if(bIsSdrGrafObj || bIsSdrOle2Obj)
                {
                    pObj->NbcSetStyleSheet(pDoc->GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(), true);
                }
                else
                {
                    pObj->NbcSetStyleSheet(pDoc->GetDefaultStyleSheet(), true);
                }
            }
        }
        return pObj;
    }

    aType = aType.copy( aPrefix.getLength() );

    PresObjKind eObjKind = PRESOBJ_NONE;

    if( aType == "TitleTextShape" )
    {
        eObjKind = PRESOBJ_TITLE;
    }
    else if( aType == "OutlinerShape" )
    {
        eObjKind = PRESOBJ_OUTLINE;
    }
    else if( aType == "SubtitleShape" )
    {
        eObjKind = PRESOBJ_TEXT;
    }
    else if( aType == "OLE2Shape" )
    {
        eObjKind = PRESOBJ_OBJECT;
    }
    else if( aType == "ChartShape" )
    {
        eObjKind = PRESOBJ_CHART;
    }
    else if( aType == "CalcShape" )
    {
        eObjKind = PRESOBJ_CALC;
    }
    else if( aType == "TableShape" )
    {
        eObjKind = PRESOBJ_TABLE;
    }
    else if( aType == "GraphicObjectShape" )
    {
        eObjKind = PRESOBJ_GRAPHIC;
    }
    else if( aType == "OrgChartShape" )
    {
        eObjKind = PRESOBJ_ORGCHART;
    }
    else if( aType == "PageShape" )
    {
        if( GetPage()->GetPageKind() == PageKind::Notes && GetPage()->IsMasterPage() )
            eObjKind = PRESOBJ_TITLE;
        else
            eObjKind = PRESOBJ_PAGE;
    }
    else if( aType == "NotesShape" )
    {
        eObjKind = PRESOBJ_NOTES;
    }
    else if( aType == "HandoutShape" )
    {
        eObjKind = PRESOBJ_HANDOUT;
    }
    else if( aType == "FooterShape" )
    {
        eObjKind = PRESOBJ_FOOTER;
    }
    else if( aType == "HeaderShape" )
    {
        eObjKind = PRESOBJ_HEADER;
    }
    else if( aType == "SlideNumberShape" )
    {
        eObjKind = PRESOBJ_SLIDENUMBER;
    }
    else if( aType == "DateTimeShape" )
    {
        eObjKind = PRESOBJ_DATETIME;
    }
    else if( aType == "MediaShape" )
    {
        eObjKind = PRESOBJ_MEDIA;
    }

    Rectangle aRect( eObjKind == PRESOBJ_TITLE ? GetPage()->GetTitleRect() : GetPage()->GetLayoutRect()  );

    const awt::Point aPos( aRect.Left(), aRect.Top() );
    xShape->setPosition( aPos );

    const awt::Size aSize( aRect.GetWidth(), aRect.GetHeight() );
    xShape->setSize( aSize );

    SdrObject *pPresObj = nullptr;
    if( (eObjKind == PRESOBJ_TABLE) || (eObjKind == PRESOBJ_MEDIA) )
    {
        pPresObj = SvxFmDrawPage::CreateSdrObject_( xShape );
        if( pPresObj )
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
            if( pDoc )
                pPresObj->NbcSetStyleSheet( pDoc->GetDefaultStyleSheet(), true );
            GetPage()->InsertPresObj( pPresObj, eObjKind );
        }
    }
    else
    {
        pPresObj = GetPage()->CreatePresObj( eObjKind, false, aRect, true );
    }

    if( pPresObj )
        pPresObj->SetUserCall( GetPage() );

    return pPresObj;
}

// XInterface
Any SAL_CALL SdGenericDrawPage::queryInterface( const uno::Type & rType )
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
        const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PageKind::Standard;

        if( ePageKind == PageKind::Standard )
            return makeAny( Reference< XAnimationNodeSupplier >( this ) );
    }
    else
        return SvxDrawPage::queryInterface( rType );

    return aAny;
}

// XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL SdGenericDrawPage::getPropertySetInfo()
{
    ::SolarMutexGuard aGuard;
    throwIfDisposed();
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdGenericDrawPage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
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
                SetLftBorder(nValue);
                break;
            case WID_PAGE_RIGHT:
                SetRgtBorder( nValue );
                break;
            case WID_PAGE_TOP:
                SetUppBorder( nValue );
                break;
            case WID_PAGE_BOTTOM:
                SetLwrBorder( nValue );
                break;
            case WID_PAGE_CHANGE:
                GetPage()->SetPresChange( (PresChange)nValue );
                break;
            case WID_PAGE_LAYOUT:
                GetPage()->SetAutoLayout( (AutoLayout)nValue, true );
                break;
            case WID_PAGE_DURATION:
                GetPage()->SetTime((sal_Int32)nValue);
                break;
            }
            break;
        }
        case WID_PAGE_HIGHRESDURATION:
        {
            double fValue = 0;
            if(!(aValue >>= fValue))
                throw lang::IllegalArgumentException();

            GetPage()->SetTime(fValue);
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

            Orientation eOri = (((view::PaperOrientation)nEnum) == view::PaperOrientation_PORTRAIT)?Orientation::Portrait:Orientation::Landscape;

            if( eOri != GetPage()->GetOrientation() )
            {
                SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
                const PageKind ePageKind = GetPage()->GetPageKind();

                sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
                    pPage->SetOrientation( eOri );
                }

                nPageCnt = pDoc->GetSdPageCount(ePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
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

            GetPage()->SetFadeEffect( (presentation::FadeEffect)nEnum );
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

            GetPage()->setTransitionDuration( nEnum == 0 ? 3.0 : (nEnum == 1 ? 2.0 : 1.0 )  );
            break;
        }
        case WID_PAGE_VISIBLE :
        {
            bool    bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetExcluded( !bVisible );
            break;
        }
        case WID_PAGE_SOUNDFILE :
        {
            OUString aURL;
            if( aValue >>= aURL )
            {
                GetPage()->SetSoundFile( aURL );
                GetPage()->SetSound( !aURL.isEmpty() );
                break;
            }
            else
            {
                bool bStopSound = false;
                if( aValue >>= bStopSound )
                {
                    GetPage()->SetStopSound( bStopSound );
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

            GetPage()->SetLoopSound( bLoop );
            break;
        }
        case WID_PAGE_BACKFULL:
        {
            bool    bFullSize = false;
            if( ! ( aValue >>= bFullSize ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetBackgroundFullSize( bFullSize );
            break;
        }
        case WID_PAGE_BACKVIS:
        {
            bool bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdrPage* pPage = GetPage();
            if( pPage )
            {
                SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(pPage->GetModel());
                if( pDoc->GetMasterPageCount() )
                {
                    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false), bVisible);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                }
            }
            break;
        }
        case WID_PAGE_BACKOBJVIS:
        {
            bool bVisible = false;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdrPage* pPage = GetPage();
            if( pPage )
            {
                SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(pPage->GetModel());
                if( pDoc->GetMasterPageCount() )
                {
                    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false), bVisible);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                }
            }

            break;
        }
        case WID_PAGE_USERATTRIBS:
        {
            if( !GetPage()->setAlienAttributes( aValue ) )
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
            sd::HeaderFooterSettings aHeaderFooterSettings( GetPage()->getHeaderFooterSettings() );

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

                aHeaderFooterSettings.meDateTimeFormat = nValue;
                break;
            }
            }

            if( !(aHeaderFooterSettings == GetPage()->getHeaderFooterSettings()) )
                GetPage()->setHeaderFooterSettings( aHeaderFooterSettings );

            break;
        }

        case WID_PAGE_NUMBER:
            if( (GetPage()->GetPageKind() == PageKind::Handout) && !GetPage()->IsMasterPage() )
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

            GetPage()->setTransitionType( nValue );
            break;
        }

        case WID_TRANSITION_SUBTYPE:
        {
            sal_Int16 nValue = 0;
            if( ! ( aValue >>= nValue ) )
                throw lang::IllegalArgumentException();

            GetPage()->setTransitionSubtype( nValue );
            break;
        }

        case WID_TRANSITION_DIRECTION:
        {
            bool bValue = false;
            if( ! ( aValue >>= bValue ) )
                throw lang::IllegalArgumentException();

            GetPage()->setTransitionDirection( bValue );
            break;
        }

        case WID_TRANSITION_FADE_COLOR:
        {
            sal_Int32 nValue = 0;
            if( ! ( aValue >>= nValue ) )
                throw lang::IllegalArgumentException();

            GetPage()->setTransitionFadeColor( nValue );
            break;
        }

        case WID_TRANSITION_DURATION:
        {
            double fValue = 0.0;
            if( ! ( aValue >>= fValue ) )
                throw lang::IllegalArgumentException();

            GetPage()->setTransitionDuration( fValue );
            break;
        }

        default:
            throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    GetModel()->SetModified();
}

Any SAL_CALL SdGenericDrawPage::getPropertyValue( const OUString& PropertyName )
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
        aAny <<= (sal_Int32)( GetPage()->GetLftBorder() );
        break;
    case WID_PAGE_RIGHT:
        aAny <<= (sal_Int32)( GetPage()->GetRgtBorder() );
        break;
    case WID_PAGE_TOP:
        aAny <<= (sal_Int32)( GetPage()->GetUppBorder() );
        break;
    case WID_PAGE_BOTTOM:
        aAny <<= (sal_Int32)( GetPage()->GetLwrBorder() );
        break;
    case WID_PAGE_WIDTH:
        aAny <<= (sal_Int32)( GetPage()->GetSize().getWidth() );
        break;
    case WID_PAGE_HEIGHT:
        aAny <<= (sal_Int32)( GetPage()->GetSize().getHeight() );
        break;
    case WID_PAGE_ORIENT:
        aAny <<= view::PaperOrientation(
            GetPage()->GetOrientation() == Orientation::Portrait
            ? view::PaperOrientation_PORTRAIT
            : view::PaperOrientation_LANDSCAPE);
        break;
    case WID_PAGE_EFFECT:
        aAny <<= presentation::FadeEffect(GetPage()->GetFadeEffect());
        break;
    case WID_PAGE_CHANGE:
        aAny <<= (sal_Int32)( GetPage()->GetPresChange() );
        break;
    case WID_PAGE_SPEED:
        {
            const double fDuration = GetPage()->getTransitionDuration();
            aAny <<= presentation::AnimationSpeed(
                fDuration < 2.0 ? 2 : fDuration > 2.0 ? 0 : 1);
        }
        break;
    case WID_PAGE_LAYOUT:
        aAny <<= (sal_Int16)( GetPage()->GetAutoLayout() );
        break;
    case WID_PAGE_NUMBER:
        {
            const sal_uInt16 nPageNumber(GetPage()->GetPageNum());

            if(nPageNumber > 0)
            {
                // for all other pages calculate the number
                aAny <<= (sal_Int16)((sal_uInt16)((nPageNumber-1)>>1) + 1);
            }
            else
            {
                aAny <<= mnTempPageNumber;
            }
        }
        break;
    case WID_PAGE_DURATION:
        aAny <<= (sal_Int32)( GetPage()->GetTime() + .5 );
        break;
    case WID_PAGE_HIGHRESDURATION:
        aAny <<= (double)( GetPage()->GetTime() );
        break;
    case WID_PAGE_LDNAME:
    {
        const OUString aName( GetPage()->GetName() );
        aAny <<= aName;
        break;
    }
    case WID_PAGE_LDBITMAP:
        {
            Reference< awt::XBitmap > xBitmap(
                VCLUnoHelper::CreateBitmap( BitmapEx( SdResId( BMP_PAGE ) ) ) );
            aAny <<= xBitmap;
        }
        break;
    case WID_PAGE_BACK:
        getBackground( aAny );
        break;
    case WID_PAGE_PREVIEW :
    case WID_PAGE_PREVIEWMETAFILE :
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
            if ( pDoc )
            {
                ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
                if ( pDocShell )
                {
                    sal_uInt16 nPgNum = 0;
                    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PageKind::Standard );
                    sal_uInt16 nPageNumber = (sal_uInt16)( ( GetPage()->GetPageNum() - 1 ) >> 1 );
                    while( nPgNum < nPageCount )
                    {
                        pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PageKind::Standard ), nPgNum == nPageNumber );
                        nPgNum++;
                    }
                    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();
                    if (xMetaFile)
                    {
                        Point   aPoint;
                        Size    aSize( GetPage()->GetSize() );
                        xMetaFile->AddAction( static_cast<MetaAction*>(new MetaFillColorAction( COL_WHITE, true )), 0 );
                        xMetaFile->AddAction( static_cast<MetaAction*>(new MetaRectAction( Rectangle( aPoint, aSize ) )), 1 );
                        xMetaFile->SetPrefMapMode( MapUnit::Map100thMM );
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
        }
        break;

    case WID_PAGE_PREVIEWBITMAP :
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
            if ( pDoc )
            {
                ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
                if ( pDocShell )
                {
                    sal_uInt16 nPgNum = 0;
                    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PageKind::Standard );
                    sal_uInt16 nPageNumber = (sal_uInt16)( ( GetPage()->GetPageNum() - 1 ) >> 1 );
                    while( nPgNum < nPageCount )
                    {
                        pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PageKind::Standard ), nPgNum == nPageNumber );
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
        }
        break;

    case WID_PAGE_VISIBLE :
    {
        bool bVisible = !GetPage()->IsExcluded();
        aAny <<= bVisible;
        break;
    }

    case WID_PAGE_SOUNDFILE :
    {
        if( GetPage()->IsStopSound() )
        {
            aAny <<= true;
        }
        else
        {
            OUString aURL;
            if( GetPage()->IsSoundOn() )
                aURL = GetPage()->GetSoundFile();
            aAny <<= aURL;
        }
        break;
    }
    case WID_LOOP_SOUND:
    {
        aAny <<= GetPage()->IsLoopSound();
        break;
    }
    case WID_PAGE_BACKFULL:
    {
        bool bFullSize = GetPage()->IsBackgroundFullSize();
        aAny <<= bFullSize;
        break;
    }
    case WID_PAGE_BACKVIS:
    {
        SdrPage* pPage = GetPage();
        if( pPage )
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(pPage->GetModel());
            if( pDoc->GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                aAny <<= aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false));
            }
            else
            {
                aAny <<= false;
            }
        }
        break;
    }
    case WID_PAGE_BACKOBJVIS:
    {
        SdrPage* pPage = GetPage();
        if( pPage )
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(pPage->GetModel());
            if( pDoc->GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                aAny <<= aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false));
            }
            else
            {
                aAny <<= false;
            }
        }
        break;
    }
    case WID_PAGE_USERATTRIBS:
    {
        GetPage()->getAlienAttributes( aAny );
        break;
    }
    case WID_PAGE_BOOKMARK:
    {
        aAny <<= getBookmarkURL();
        break;
    }
    case WID_PAGE_ISDARK:
    {
        aAny <<= GetPage()->GetPageBackgroundColor().IsDark();
        break;
    }
    case WID_PAGE_HEADERVISIBLE:
        aAny <<= GetPage()->getHeaderFooterSettings().mbHeaderVisible;
        break;
    case WID_PAGE_HEADERTEXT:
        {
            const OUString aText( GetPage()->getHeaderFooterSettings().maHeaderText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_FOOTERVISIBLE:
        aAny <<= GetPage()->getHeaderFooterSettings().mbFooterVisible;
        break;
    case WID_PAGE_FOOTERTEXT:
        {
            const OUString aText( GetPage()->getHeaderFooterSettings().maFooterText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_PAGENUMBERVISIBLE:
        aAny <<= GetPage()->getHeaderFooterSettings().mbSlideNumberVisible;
        break;
    case WID_PAGE_DATETIMEVISIBLE:
        aAny <<= GetPage()->getHeaderFooterSettings().mbDateTimeVisible;
        break;
    case WID_PAGE_DATETIMEFIXED:
        aAny <<= GetPage()->getHeaderFooterSettings().mbDateTimeIsFixed;
        break;
    case WID_PAGE_DATETIMETEXT:
        {
            const OUString aText( GetPage()->getHeaderFooterSettings().maDateTimeText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_DATETIMEFORMAT:
        aAny <<= (sal_Int32)GetPage()->getHeaderFooterSettings().meDateTimeFormat;
        break;

    case WID_TRANSITION_TYPE:
        aAny <<= GetPage()->getTransitionType();
        break;

    case WID_TRANSITION_SUBTYPE:
        aAny <<= GetPage()->getTransitionSubtype();
        break;

    case WID_TRANSITION_DIRECTION:
        aAny <<= GetPage()->getTransitionDirection();
        break;

    case WID_TRANSITION_FADE_COLOR:
        aAny <<= GetPage()->getTransitionFadeColor();
        break;

    case WID_TRANSITION_DURATION:
        aAny <<= GetPage()->getTransitionDuration();
        break;

    default:
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }
    return aAny;
}

void SAL_CALL SdGenericDrawPage::addPropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdGenericDrawPage::removePropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdGenericDrawPage::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdGenericDrawPage::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) {}

// XMultiPropertySet
void SAL_CALL SdGenericDrawPage::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
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

Sequence< Any > SAL_CALL SdGenericDrawPage::getPropertyValues( const Sequence< OUString >& aPropertyNames )
{
    const OUString* pNames = aPropertyNames.getConstArray();
    sal_uInt32 nCount = aPropertyNames.getLength();
    Sequence< Any > aValues( nCount );
    Any* pValues = aValues.getArray();
    while( nCount-- )
    {
        Any aValue;
        try
        {
            aValue = getPropertyValue( *pNames++ );
        }
        catch( beans::UnknownPropertyException& )
        {
            // ignore for multi property set
            // todo: optimize this!
        }
        *pValues++ = aValue;
    }
    return aValues;
}

void SAL_CALL SdGenericDrawPage::addPropertiesChangeListener( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SdGenericDrawPage::removePropertiesChangeListener( const Reference< beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SdGenericDrawPage::firePropertiesChangeEvent( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  )
{
}

Reference< drawing::XShape >  SdGenericDrawPage::CreateShape(SdrObject *pObj) const
{
    DBG_ASSERT( GetPage(), "SdGenericDrawPage::CreateShape(), can't create shape for disposed page!" );
    DBG_ASSERT( pObj, "SdGenericDrawPage::CreateShape(), invalid call with pObj == 0!" );

    if (!pObj)
        return Reference< drawing::XShape >();

    if (GetPage())
    {
        PresObjKind eKind = GetPage()->GetPresObjKind(pObj);

        SvxShape* pShape = nullptr;

        if(pObj->GetObjInventor() == SdrInventor::Default)
        {
            sal_uInt32 nInventor = pObj->GetObjIdentifier();
            switch( nInventor )
            {
            case OBJ_TITLETEXT:
                pShape = new SvxShapeText( pObj );
                if( GetPage()->GetPageKind() == PageKind::Notes && GetPage()->IsMasterPage() )
                {
                    // fake a empty PageShape if it's a title shape on the master page
                    pShape->SetShapeType("com.sun.star.presentation.PageShape");
                }
                else
                {
                    pShape->SetShapeType("com.sun.star.presentation.TitleTextShape");
                }
                eKind = PRESOBJ_NONE;
                break;
            case OBJ_OUTLINETEXT:
                pShape = new SvxShapeText( pObj );
                pShape->SetShapeType("com.sun.star.presentation.OutlinerShape");
                eKind = PRESOBJ_NONE;
                break;
            }
        }

        Reference< drawing::XShape >  xShape( pShape );

        if(!xShape.is())
            xShape = SvxFmDrawPage::CreateShape( pObj );

        if( eKind != PRESOBJ_NONE )
        {
            OUString aShapeType("com.sun.star.presentation.");

            switch( eKind )
            {
            case PRESOBJ_TITLE:
                aShapeType += "TitleTextShape";
                break;
            case PRESOBJ_OUTLINE:
                aShapeType += "OutlinerShape";
                break;
            case PRESOBJ_TEXT:
                aShapeType += "SubtitleShape";
                break;
            case PRESOBJ_GRAPHIC:
                aShapeType += "GraphicObjectShape";
                break;
            case PRESOBJ_OBJECT:
                aShapeType += "OLE2Shape";
                break;
            case PRESOBJ_CHART:
                aShapeType += "ChartShape";
                break;
            case PRESOBJ_ORGCHART:
                aShapeType += "OrgChartShape";
                break;
            case PRESOBJ_CALC:
                aShapeType += "CalcShape";
                break;
            case PRESOBJ_TABLE:
                aShapeType += "TableShape";
                break;
            case PRESOBJ_MEDIA:
                aShapeType += "MediaShape";
                break;
            case PRESOBJ_PAGE:
                aShapeType += "PageShape";
                break;
            case PRESOBJ_HANDOUT:
                aShapeType += "HandoutShape";
                break;
            case PRESOBJ_NOTES:
                aShapeType += "NotesShape";
                break;
            case PRESOBJ_FOOTER:
                aShapeType += "FooterShape";
                break;
            case PRESOBJ_HEADER:
                aShapeType += "HeaderShape";
                break;
            case PRESOBJ_SLIDENUMBER:
                aShapeType += "SlideNumberShape";
                break;
            case PRESOBJ_DATETIME:
                aShapeType += "DateTimeShape";
                break;
            // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
            case PRESOBJ_NONE:
            case PRESOBJ_IMAGE:
            case PRESOBJ_MAX:
                break;
            }

            if( !pShape )
                pShape = SvxShape::getImplementation( xShape );

            if( pShape )
                pShape->SetShapeType( aShapeType );
        }

        SvxShape *pSdShape = SvxShape::getImplementation(xShape);
        if (pSdShape)
        {
            // SdXShape aggregates SvxShape
            new SdXShape(pSdShape, GetModel());
        }
        return xShape;
    }
    else
    {
        return SvxFmDrawPage::CreateShape( pObj );
    }

}

// XServiceInfo
Sequence< OUString > SAL_CALL SdGenericDrawPage::getSupportedServiceNames()
{
    Sequence< OUString > aSeq( SvxFmDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.GenericDrawPage",
                                                  "com.sun.star.document.LinkTarget",
                                                  "com.sun.star.document.LinkTargetSupplier"});
    return aSeq;
}

// XLinkTargetSupplier
Reference< container::XNameAccess > SAL_CALL SdGenericDrawPage::getLinks(  )
{
    return new SdPageLinkTargets( this );
}

void SdGenericDrawPage::setBackground( const Any& )
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

void SdGenericDrawPage::getBackground( Any& )
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

OUString SdGenericDrawPage::getBookmarkURL() const
{
    OUStringBuffer aRet;
    if( SvxFmDrawPage::mpPage )
    {
        OUString aFileName( static_cast<SdPage*>(SvxFmDrawPage::mpPage)->GetFileName() );
        if( !aFileName.isEmpty() )
        {
            const OUString aBookmarkName( SdDrawPage::getPageApiNameFromUiName( static_cast<SdPage*>(SvxFmDrawPage::mpPage)->GetBookmarkName() ) );
            aRet.append( aFileName );
            aRet.append( '#' );
            aRet.append( aBookmarkName );
        }
    }

    return aRet.makeStringAndClear();
}

void SdGenericDrawPage::setBookmarkURL( OUString& rURL )
{
    if( SvxFmDrawPage::mpPage )
    {
        sal_Int32 nIndex = rURL.indexOf( '#' );
        if( nIndex != -1 )
        {
            const OUString aFileName( rURL.copy( 0, nIndex ) );
            const OUString aBookmarkName( SdDrawPage::getUiNameFromPageApiName( rURL.copy( nIndex+1 )  ) );

            if( !aFileName.isEmpty() && !aBookmarkName.isEmpty() )
            {
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->DisconnectLink();
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->SetFileName( aFileName );
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->SetBookmarkName( aBookmarkName );
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->ConnectLink();
            }
        }
    }
}

Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::combine( const Reference< drawing::XShapes >& xShapes )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    DBG_ASSERT(SvxFmDrawPage::mpPage,"SdrPage is NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    Reference< drawing::XShape > xShape;
    if(mpView==nullptr||!xShapes.is()||GetPage()==nullptr)
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );

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

void SAL_CALL SdGenericDrawPage::split( const Reference< drawing::XShape >& xGroup )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==nullptr||!xGroup.is()||GetPage()==nullptr)
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );
    SelectObjectInView( xGroup, pPageView );
    mpView->DismantleMarkedObjects();
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::bind( const Reference< drawing::XShapes >& xShapes )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Reference< drawing::XShape > xShape;
    if(mpView==nullptr||!xShapes.is()||GetPage()==nullptr)
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );

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

void SAL_CALL SdGenericDrawPage::unbind( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==nullptr||!xShape.is()||GetPage()==nullptr)
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );
    SelectObjectInView( xShape, pPageView );
    mpView->DismantleMarkedObjects( true );
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

void SdGenericDrawPage::SetLftBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetLftBorder() )
    {
        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetLftBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetLftBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetRgtBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetRgtBorder() )
    {
        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetRgtBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetRgtBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetUppBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetUppBorder() )
    {
        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetUppBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetUppBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetLwrBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetLwrBorder() )
    {
        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetLwrBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetLwrBorder( nValue );
        }
    }
}

static void refreshpage( SdDrawDocument* pDoc, const PageKind ePageKind )
{
    ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
    if ( pDocShell )
    {
        ::sd::ViewShell* pViewSh = pDocShell->GetViewShell();

        if( pViewSh )
        {
            if( dynamic_cast<const ::sd::DrawViewShell* >(pViewSh) !=  nullptr )
                static_cast< ::sd::DrawViewShell*>(pViewSh)->ResetActualPage();

            Size aPageSize = pDoc->GetSdPage(0, ePageKind)->GetSize();
            const long nWidth = aPageSize.Width();
            const long nHeight = aPageSize.Height();

            Point aPageOrg(nWidth, nHeight / 2);
            Size aViewSize(nWidth * 3, nHeight * 2);

            pDoc->SetMaxObjSize(aViewSize);

            pViewSh->InitWindows(aPageOrg, aViewSize, Point(-1, -1), true);

            pViewSh->UpdateScrollBars();
        }
    }
}

void SdGenericDrawPage::SetWidth( sal_Int32 nWidth )
{
    Size aSize( GetPage()->GetSize() );
    if( aSize.getWidth() != nWidth )
    {
        aSize.setWidth( nWidth );

        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        refreshpage( pDoc, ePageKind );
    }
}

void SdGenericDrawPage::SetHeight( sal_Int32 nHeight )
{
    Size aSize( GetPage()->GetSize() );
    if( aSize.getHeight() != nHeight )
    {
        aSize.setHeight( nHeight );

        SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetPage()->GetModel());
        const PageKind ePageKind = GetPage()->GetPageKind();

        sal_uInt16 i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        refreshpage( pDoc, ePageKind );
    }
}

// XInterface
void SdGenericDrawPage::release() throw()
{

    OWeakAggObject::release();
}

// XComponent
void SdGenericDrawPage::disposing() throw()
{
    mpModel = nullptr;
    SvxFmDrawPage::disposing();
}

// XAnimationNodeSupplier
Reference< XAnimationNode > SAL_CALL SdGenericDrawPage::getAnimationNode()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SdPage *pSdPage = static_cast<SdPage*>(SvxFmDrawPage::mpPage);

    return pSdPage->getAnimationNode();
}

// SdPageLinkTargets
SdPageLinkTargets::SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) throw()
{
    mxPage = pUnoPage;
    mpUnoPage = pUnoPage;
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

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != nullptr )
    {
        SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );

        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            OUString aStr( pObj->GetName() );
            if( aStr.isEmpty() && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
            if( !aStr.isEmpty() )
                return true;
        }
    }

    return false;
}

// container::XNameAccess

// XNameAccess
Any SAL_CALL SdPageLinkTargets::getByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    SdPage* pPage = mpUnoPage->GetPage();
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

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != nullptr )
    {
        SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            OUString aStr( pObj->GetName() );
            if( aStr.isEmpty() && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
            if( !aStr.isEmpty() )
                nObjCount++;
        }
    }

    Sequence< OUString > aSeq( nObjCount );
    if( nObjCount > 0 )
    {
        OUString* pStr = aSeq.getArray();

        SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            OUString aStr( pObj->GetName() );
            if( aStr.isEmpty() && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
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

SdrObject* SdPageLinkTargets::FindObject( const OUString& rName ) const throw()
{
    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage == nullptr )
        return nullptr;

    SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OUString aStr( pObj->GetName() );
        if( aStr.isEmpty() && dynamic_cast< const SdrOle2Obj *>( pObj ) !=  nullptr )
            aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
        if( !aStr.isEmpty() && (aStr == rName) )
            return pObj;
    }

    return nullptr;
}

// XServiceInfo
OUString SAL_CALL SdPageLinkTargets::getImplementationName()
{
    return OUString( "SdPageLinkTargets" );
}

sal_Bool SAL_CALL SdPageLinkTargets::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdPageLinkTargets::getSupportedServiceNames()
{
    Sequence< OUString > aSeq { "com.sun.star.document.LinkTargets" };
    return aSeq;
}

// SdDrawPage
SdDrawPage::SdDrawPage(  SdXImpressDocument* pModel, SdPage* pPage ) throw()
: SdGenericDrawPage( pModel, pPage, ImplGetDrawPagePropertySet( pModel->IsImpressDocument(), pPage->GetPageKind() ) )
{
}

SdDrawPage::~SdDrawPage() throw()
{
}

// XInterface
Any SAL_CALL SdDrawPage::queryInterface( const uno::Type & rType )
{
    if( rType == cppu::UnoType<drawing::XMasterPageTarget>::get() )
    {
        return makeAny( Reference< drawing::XMasterPageTarget >( this ) );
    }
    else if( IsImpressDocument()
             && rType == cppu::UnoType<presentation::XPresentationPage>::get() )
    {
        SdPage * p = dynamic_cast<SdPage *>(SvxDrawPage::mpPage);
        if( p == nullptr || p->GetPageKind() != PageKind::Handout )
        {
            return makeAny( Reference< presentation::XPresentationPage >( this ) );
        }
    }

    return SdGenericDrawPage::queryInterface( rType );
}

void SAL_CALL SdDrawPage::acquire() throw()
{
    SvxDrawPage::acquire();
}

void SAL_CALL SdDrawPage::release() throw()
{
    SvxDrawPage::release();
}

UNO3_GETIMPLEMENTATION2_IMPL( SdDrawPage, SdGenericDrawPage );

// XTypeProvider
Sequence< uno::Type > SAL_CALL SdDrawPage::getTypes()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( maTypeSequence.getLength() == 0 )
    {
        const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PageKind::Standard;
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
        const Sequence< uno::Type > aBaseTypes( SdGenericDrawPage::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        // Join those types in a sequence.
        maTypeSequence.realloc(aTypes.size() + nBaseTypes);
        uno::Type* pTypes = maTypeSequence.getArray();
        ::std::vector<uno::Type>::const_iterator iType;
        for (iType=aTypes.begin(); iType!=aTypes.end(); ++iType)
            *pTypes++ = *iType;
        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

Sequence< sal_Int8 > SAL_CALL SdDrawPage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SdDrawPage::getPageApiName( SdPage* pPage )
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
            OUStringBuffer sBuffer;
            sBuffer.append( sEmptyPageName );
            const sal_Int32 nPageNum = ( ( pPage->GetPageNum() - 1 ) >> 1 ) + 1;
            sBuffer.append( nPageNum );
            aPageName = sBuffer.makeStringAndClear();
        }
    }

    return aPageName;
}

OUString getPageApiNameFromUiName( const OUString& rUIName )
{
    OUString aApiName;

    OUString aDefPageName(SD_RESSTR(STR_PAGE) + " ");

    if( rUIName.startsWith( aDefPageName ) )
    {
        aApiName = sEmptyPageName;
        aApiName += rUIName.copy( aDefPageName.getLength() );
    }
    else
    {
        aApiName = rUIName;
    }

    return aApiName;
}

OUString SdDrawPage::getPageApiNameFromUiName( const OUString& rUIName )
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
            OUStringBuffer sBuffer;
            sBuffer.append( SD_RESSTR(STR_PAGE) );
            sBuffer.append( ' ' );
            sBuffer.append( aNumber );
            return sBuffer.makeStringAndClear();
        }
    }

    return rApiName;
}

OUString SdDrawPage::getUiNameFromPageApiName( const OUString& rApiName )
{
    return getUiNameFromPageApiNameImpl( rApiName );
}

// XServiceInfo
OUString SAL_CALL SdDrawPage::getImplementationName()
{
    return OUString( "SdDrawPage" );
}

Sequence< OUString > SAL_CALL SdDrawPage::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.DrawPage"} );

    if( IsImpressDocument() )
        comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.presentation.DrawPage"} );

    return aSeq;
}

sal_Bool SAL_CALL SdDrawPage::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

// XNamed
void SAL_CALL SdDrawPage::setName( const OUString& rName )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    DBG_ASSERT( GetPage() && !GetPage()->IsMasterPage(), "Don't call base implementation for masterpages!" );

    OUString aName( rName );

    if(GetPage() && GetPage()->GetPageKind() != PageKind::Notes)
    {
        // check if this is the default 'page1234' name
        if(aName.startsWith( sEmptyPageName ))
        {
            // ok, it maybe is, first get the number part after 'page'
            OUString aNumber( aName.copy( sizeof( sEmptyPageName ) - 1 ) );

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

            if( nPageNumber == ( ( GetPage()->GetPageNum() - 1 ) >> 1 ) + 1 )
                aName.clear();
        }
        else
        {
            OUString aDefaultPageName( SD_RESSTR(STR_PAGE) + " " );
            if( aName.startsWith( aDefaultPageName ) )
                aName.clear();
        }

        GetPage()->SetName( aName );

        sal_uInt16 nNotesPageNum = (GetPage()->GetPageNum()-1)>>1;
        if( GetModel()->GetDoc()->GetSdPageCount( PageKind::Notes ) > nNotesPageNum )
        {
            SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( nNotesPageNum, PageKind::Notes );
            if( pNotesPage )
                pNotesPage->SetName(aName);
        }

        // fake a mode change to repaint the page tab bar
        ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
        ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : nullptr;
        if( pViewSh && dynamic_cast< const ::sd::DrawViewShell* >(pViewSh) !=  nullptr)
        {
            ::sd::DrawViewShell* pDrawViewSh = static_cast<
                  ::sd::DrawViewShell*>(pViewSh);

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
}

OUString SAL_CALL SdDrawPage::getName()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return getPageApiName( GetPage() );
}

// XMasterPageTarget
Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getMasterPage(  )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(GetPage())
    {
        Reference< drawing::XDrawPages >    xPages( GetModel()->getMasterPages() );
        Reference< drawing::XDrawPage > xPage;

        if(SvxFmDrawPage::mpPage->TRG_HasMasterPage())
        {
            SdrPage& rMasterPage = SvxFmDrawPage::mpPage->TRG_GetMasterPage();
            xPage.set( rMasterPage.getUnoPage(), uno::UNO_QUERY );
        }

        return xPage;
    }
    return nullptr;
}

void SAL_CALL SdDrawPage::setMasterPage( const Reference< drawing::XDrawPage >& xMasterPage )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage)
    {
        SdMasterPage* pMasterPage = SdMasterPage::getImplementation( xMasterPage );
        if( pMasterPage && pMasterPage->isValid() )
        {
            SvxFmDrawPage::mpPage->TRG_ClearMasterPage();

            SdPage* pSdPage = static_cast<SdPage*>(pMasterPage->GetSdrPage());
            SvxFmDrawPage::mpPage->TRG_SetMasterPage(*pSdPage);

            SvxFmDrawPage::mpPage->SetBorder(pSdPage->GetLftBorder(),pSdPage->GetUppBorder(),
                              pSdPage->GetRgtBorder(),pSdPage->GetLwrBorder() );

            SvxFmDrawPage::mpPage->SetSize( pSdPage->GetSize() );
            SvxFmDrawPage::mpPage->SetOrientation( pSdPage->GetOrientation() );
            static_cast<SdPage*>(SvxFmDrawPage::mpPage)->SetLayoutName( pSdPage->GetLayoutName() );

            // set notes master also
            SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PageKind::Notes );

            pNotesPage->TRG_ClearMasterPage();
            sal_uInt16 nNum = (SvxFmDrawPage::mpPage->TRG_GetMasterPage()).GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*SvxFmDrawPage::mpPage->GetModel()->GetMasterPage(nNum));
            pNotesPage->SetLayoutName( pSdPage->GetLayoutName() );

            GetModel()->SetModified();
        }

    }
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getNotesPage()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetModel()->GetDoc() && SvxFmDrawPage::mpPage->GetPageNum() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PageKind::Notes );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return nullptr;
}

// XIndexAccess
sal_Int32 SAL_CALL SdDrawPage::getCount()
{
    return SdGenericDrawPage::getCount();
}

Any SAL_CALL SdDrawPage::getByIndex( sal_Int32 Index )
{
    return SdGenericDrawPage::getByIndex( Index );
}

// XElementAccess
uno::Type SAL_CALL SdDrawPage::getElementType()
{
    return SdGenericDrawPage::getElementType();
}

sal_Bool SAL_CALL SdDrawPage::hasElements()
{
    return SdGenericDrawPage::hasElements();
}

// XShapes
void SAL_CALL SdDrawPage::add( const Reference< drawing::XShape >& xShape )
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdDrawPage::remove( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SvxShape* pShape = SvxShape::getImplementation( xShape );
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj )
        {
            GetPage()->RemovePresObj(pObj);
            pObj->SetUserCall(nullptr);
        }
    }

    SdGenericDrawPage::remove( xShape );
}

void SdDrawPage::setBackground( const Any& rValue )
{
    Reference< beans::XPropertySet > xSet;

    if( !(rValue >>= xSet) && !rValue.hasValue() )
        throw lang::IllegalArgumentException();

    if( !xSet.is() )
    {
        // the easy case, no background set. Set drawing::FillStyle_NONE to represent this
        GetPage()->getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));
        return;
    }

    // is it our own implementation?
    SdUnoPageBackground* pBack = SdUnoPageBackground::getImplementation( xSet );

    SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST );

    if( pBack )
    {
        pBack->fillItemSet( static_cast<SdDrawDocument*>(GetPage()->GetModel()), aSet );
    }
    else
    {
        SdUnoPageBackground* pBackground = new SdUnoPageBackground();

        Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
        Reference< beans::XPropertySet >  xDestSet( static_cast<beans::XPropertySet*>(pBackground) );
        Reference< beans::XPropertySetInfo >  xDestSetInfo( xDestSet->getPropertySetInfo() );

        Sequence< beans::Property > aProperties( xDestSetInfo->getProperties() );
        sal_Int32 nCount = aProperties.getLength();
        beans::Property* pProp = aProperties.getArray();

        while( nCount-- )
        {
            const OUString aPropName( pProp->Name );
            if( xSetInfo->hasPropertyByName( aPropName ) )
                xDestSet->setPropertyValue( aPropName,
                        xSet->getPropertyValue( aPropName ) );

            pProp++;
        }

        pBackground->fillItemSet( static_cast<SdDrawDocument*>(GetPage()->GetModel()), aSet );
    }

    if( aSet.Count() == 0 )
    {
        // no background fill, represent by setting drawing::FillStyle_NONE
        GetPage()->getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));
    }
    else
    {
        // background fill, set at page (not sure if ClearItem is needed)
        GetPage()->getSdrPageProperties().ClearItem();
        GetPage()->getSdrPageProperties().PutItemSet(aSet);
    }

    // repaint only
    SvxFmDrawPage::mpPage->ActionChanged();
}

// XAnnotationAccess:
Reference< XAnnotation > SAL_CALL SdGenericDrawPage::createAndInsertAnnotation()
{
    if( !GetPage() )
        throw DisposedException();

    Reference< XAnnotation > xRet;
    GetPage()->createAnnotation(xRet);
    return xRet;
}

void SAL_CALL SdGenericDrawPage::removeAnnotation(const Reference< XAnnotation > & annotation)
{
    GetPage()->removeAnnotation(annotation);
}

Reference< XAnnotationEnumeration > SAL_CALL SdGenericDrawPage::createAnnotationEnumeration()
{
    return ::sd::createAnnotationEnumeration( GetPage()->getAnnotations() );
}

void SdDrawPage::getBackground( Any& rValue ) throw()
{
    const SfxItemSet& rFillAttributes = GetPage()->getSdrPageProperties().GetItemSet();

       if(drawing::FillStyle_NONE == static_cast<const XFillStyleItem&>(rFillAttributes.Get(XATTR_FILLSTYLE)).GetValue())
    {
        // no fill set (switched off by drawing::FillStyle_NONE), clear rValue to represent this
        rValue.clear();
    }
    else
    {
        // there is a fill set, export to rValue
        Reference< beans::XPropertySet > xSet(new SdUnoPageBackground(
            GetModel()->GetDoc(),
            &GetPage()->getSdrPageProperties().GetItemSet()));
        rValue <<= xSet;
    }
}

void SdGenericDrawPage::setNavigationOrder( const Any& rValue )
{
    Reference< XIndexAccess > xIA( rValue, UNO_QUERY );
    if( xIA.is() )
    {
        if( dynamic_cast< SdDrawPage* >( xIA.get() ) == this )
        {
            if( GetPage()->HasObjectNavigationOrder() )
                GetPage()->ClearObjectNavigationOrder();

            return;
        }
        else if( static_cast<size_t>(xIA->getCount()) == GetPage()->GetObjCount() )
        {
            GetPage()->SetNavigationOrder(xIA);
            return;
        }
    }
    throw IllegalArgumentException();
}

class SdNavigationOrderAccess : public ::cppu::WeakImplHelper< XIndexAccess >
{
public:
    explicit SdNavigationOrderAccess(SdrPage* pPage);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

private:
    std::vector< Reference< XShape > > maShapes;
};

SdNavigationOrderAccess::SdNavigationOrderAccess( SdrPage* pPage )
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

Any SdGenericDrawPage::getNavigationOrder()
{
    if( GetPage()->HasObjectNavigationOrder() )
    {
        return Any( Reference< XIndexAccess >( new SdNavigationOrderAccess( GetPage() ) ) );
    }
    else
    {
        return Any( Reference< XIndexAccess >( this ) );
    }
}

// class SdMasterPage
SdMasterPage::SdMasterPage( SdXImpressDocument* pModel, SdPage* pPage ) throw()
    : SdGenericDrawPage(pModel, pPage, ImplGetMasterPagePropertySet(pPage->GetPageKind()))
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
               GetPage()  && GetPage()->GetPageKind() != PageKind::Handout) )
        aAny <<= Reference< presentation::XPresentationPage >( this );
    else
        return SdGenericDrawPage::queryInterface( rType );

    return aAny;
}

void SAL_CALL SdMasterPage::acquire() throw()
{
    SvxDrawPage::acquire();
}

void SAL_CALL SdMasterPage::release() throw()
{
    SvxDrawPage::release();
}

UNO3_GETIMPLEMENTATION2_IMPL( SdMasterPage, SdGenericDrawPage );

// XTypeProvider
Sequence< uno::Type > SAL_CALL SdMasterPage::getTypes()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( maTypeSequence.getLength() == 0 )
    {
        const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PageKind::Standard;
        bool bPresPage = IsImpressDocument() && SvxFmDrawPage::mpPage && ePageKind != PageKind::Handout;

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
        const Sequence< uno::Type > aBaseTypes( SdGenericDrawPage::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        // Join those types in a sequence.
        maTypeSequence.realloc(aTypes.size() + nBaseTypes);
        uno::Type* pTypes = maTypeSequence.getArray();
        ::std::vector<uno::Type>::const_iterator iType;
        for (iType=aTypes.begin(); iType!=aTypes.end(); ++iType)
            *pTypes++ = *iType;
        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
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
    return OUString( "SdMasterPage" );
}

Sequence< OUString > SAL_CALL SdMasterPage::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.MasterPage"} );

    if( SvxFmDrawPage::mpPage && static_cast<SdPage*>(SvxFmDrawPage::mpPage)->GetPageKind() == PageKind::Handout )
        comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.presentation.HandoutMasterPage"} );

    return aSeq;
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

    if( SvxFmDrawPage::mpPage == nullptr )
        return false;

    return SvxFmDrawPage::mpPage->GetObjCount() > 0;
}

uno::Type SAL_CALL SdMasterPage::getElementType()
{
    return SdGenericDrawPage::getElementType();
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPage::getCount()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdGenericDrawPage::getCount();
}

Any SAL_CALL SdMasterPage::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdGenericDrawPage::getByIndex(Index);
}

// intern
void SdMasterPage::setBackground( const Any& rValue )
{
    // we need at least an beans::XPropertySet
    Reference< beans::XPropertySet > xInputSet( rValue, UNO_QUERY );
    if( !xInputSet.is() )
        throw lang::IllegalArgumentException();

    try
    {
        if( GetModel() && IsImpressDocument() )
        {
            Reference< container::XNameAccess >  xFamilies( GetModel()->getStyleFamilies(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW ) ;
            if( xFamily.is() )
            {
                OUString aStyleName(sUNO_PseudoSheet_Background);

                Reference< beans::XPropertySet >  xStyleSet( xFamily->getByName( aStyleName ), UNO_QUERY_THROW );

                Reference< beans::XPropertySetInfo >  xSetInfo( xInputSet->getPropertySetInfo(), UNO_QUERY_THROW );
                Reference< beans::XPropertyState > xSetStates( xInputSet, UNO_QUERY );

                PropertyEntryVector_t aBackgroundProperties = ImplGetPageBackgroundPropertySet()->getPropertyMap().getPropertyEntries();
                PropertyEntryVector_t::const_iterator aIt = aBackgroundProperties.begin();
                while( aIt != aBackgroundProperties.end() )
                {
                    if( xSetInfo->hasPropertyByName( aIt->sName ) )
                    {
                        if( !xSetStates.is() || xSetStates->getPropertyState( aIt->sName ) == beans::PropertyState_DIRECT_VALUE )
                            xStyleSet->setPropertyValue( aIt->sName,    xInputSet->getPropertyValue( aIt->sName ) );
                        else
                            xSetStates->setPropertyToDefault( aIt->sName );
                    }

                    ++aIt;
                }
            }
        }
        else
        {
            // first fill an item set
            // is it our own implementation?
            SdUnoPageBackground* pBack = SdUnoPageBackground::getImplementation( xInputSet );

            SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST );

            if( pBack )
            {
                pBack->fillItemSet( static_cast<SdDrawDocument*>(GetPage()->GetModel()), aSet );
            }
            else
            {
                SdUnoPageBackground* pBackground = new SdUnoPageBackground();

                Reference< beans::XPropertySetInfo > xInputSetInfo( xInputSet->getPropertySetInfo(), UNO_QUERY_THROW );
                Reference< beans::XPropertySet > xDestSet( static_cast<beans::XPropertySet*>(pBackground) );
                Reference< beans::XPropertySetInfo > xDestSetInfo( xDestSet->getPropertySetInfo(), UNO_QUERY_THROW );

                uno::Sequence< beans::Property> aProperties( xDestSetInfo->getProperties() );
                sal_Int32 nCount = aProperties.getLength();
                beans::Property* pProp = aProperties.getArray();

                while( nCount-- )
                {
                    const OUString aPropName( pProp->Name );
                    if( xInputSetInfo->hasPropertyByName( aPropName ) )
                        xDestSet->setPropertyValue( aPropName, xInputSet->getPropertyValue( aPropName ) );

                    pProp++;
                }

                pBackground->fillItemSet( static_cast<SdDrawDocument*>(SvxFmDrawPage::mpPage->GetModel()), aSet );
            }

            // if we find the background style, copy the set to the background
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(SvxFmDrawPage::mpPage->GetModel());
            SfxStyleSheetBasePool* pSSPool = pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                OUString aLayoutName( static_cast< SdPage* >( SvxFmDrawPage::mpPage )->GetLayoutName() );
                aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR)+4);
                aLayoutName += SD_RESSTR(STR_LAYOUT_BACKGROUND);
                SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aLayoutName, SD_STYLE_FAMILY_MASTERPAGE );

                if( pStyleSheet )
                {
                    pStyleSheet->GetItemSet().Put( aSet );

                    // repaint only
                    SvxFmDrawPage::mpPage->ActionChanged();
                    return;
                }
            }

            // if no background style is available, set at page directly. This
            // is an error and should NOT happen (and will be asserted from the SdrPage)
            GetPage()->getSdrPageProperties().PutItemSet(aSet);
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::SdMasterPage::setBackground(), exception caught!");
    }
}

void SdMasterPage::getBackground( Any& rValue )
{
    if( GetModel() ) try
    {
        if( IsImpressDocument() )
        {
            Reference< container::XNameAccess > xFamilies( GetModel()->getStyleFamilies(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW );

            const OUString aStyleName(sUNO_PseudoSheet_Background);
            rValue <<= Reference< beans::XPropertySet >( xFamily->getByName( aStyleName ), UNO_QUERY_THROW );
        }
        else
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(SvxFmDrawPage::mpPage->GetModel());
            SfxStyleSheetBasePool* pSSPool = pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                OUString aLayoutName( static_cast< SdPage* >(SvxFmDrawPage::mpPage)->GetLayoutName() );
                aLayoutName = aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR)+4);
                aLayoutName += SD_RESSTR(STR_LAYOUT_BACKGROUND);
                SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aLayoutName, SD_STYLE_FAMILY_MASTERPAGE );

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
            const SfxItemSet& rFallbackItemSet(SvxFmDrawPage::mpPage->getSdrPageProperties().GetItemSet());

            if(drawing::FillStyle_NONE == static_cast<const XFillStyleItem&>(rFallbackItemSet.Get(XATTR_FILLSTYLE)).GetValue())
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
        rValue.clear();
        OSL_FAIL("sd::SdMasterPage::getBackground(), exception caught!");
    }
}

// XNamed
void SAL_CALL SdMasterPage::setName( const OUString& rName )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetPage()->GetPageKind() != PageKind::Notes)
    {
        SdDrawDocument* pDoc = GetModel()->GetDoc();
        bool bOutDummy;

        // Slide Name has to be unique
        if( pDoc && pDoc->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
            return; // throw Exception ?

        GetPage()->SetName( rName );

        if( pDoc )
            pDoc->RenameLayoutTemplate( GetPage()->GetLayoutName(), rName );

        // fake a mode change to repaint the page tab bar
        ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
        ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : nullptr;
        if( pViewSh && dynamic_cast< const ::sd::DrawViewShell* >(pViewSh) !=  nullptr )
        {
            ::sd::DrawViewShell* pDrawViewSh =
                  static_cast< ::sd::DrawViewShell*>(pViewSh);

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
}

OUString SAL_CALL SdMasterPage::getName(  )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage)
    {
        OUString aLayoutName( GetPage()->GetLayoutName() );
        return aLayoutName.copy(0, aLayoutName.indexOf(SD_LT_SEPARATOR));
    }

    return OUString();
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdMasterPage::getNotesPage()
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetModel()->GetDoc() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetMasterSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PageKind::Notes );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return nullptr;
}

// XShapes
void SAL_CALL SdMasterPage::add( const Reference< drawing::XShape >& xShape )
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdMasterPage::remove( const Reference< drawing::XShape >& xShape )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SvxShape* pShape = SvxShape::getImplementation( xShape );
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj )
        {
            if( GetPage()->IsPresObj( pObj ) )
                GetPage()->RemovePresObj(pObj);
        }
    }

    SdGenericDrawPage::remove( xShape );
}

Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage )
{
    Reference< uno::XInterface > xPage;

    if( pPage && pPage->GetModel() )
    {
        SdXImpressDocument* pModel = SdXImpressDocument::getImplementation( pPage->GetModel()->getUnoModel() );
        if( pModel )
        {
            if( pPage->IsMasterPage() )
            {
                xPage = static_cast<cppu::OWeakObject*>(new SdMasterPage( pModel, pPage ));
            }
            else
            {
                xPage = static_cast<cppu::OWeakObject*>(new SdDrawPage( pModel, pPage ));
            }
        }
    }

    return xPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

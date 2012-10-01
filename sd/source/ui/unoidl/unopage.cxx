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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/PresentationRange.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/metaact.hxx>
#include <toolkit/unohlp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
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
#include <svtools/wmf.hxx>
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
#include "unohelp.hxx"

using ::com::sun::star::animations::XAnimationNode;
using ::com::sun::star::animations::XAnimationNodeSupplier;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;

namespace sd {
extern Reference< XAnnotation > createAnnotation( const Reference< XComponentContext >& xContext, SdPage* );
extern Reference< XAnnotationEnumeration > createAnnotationEnumeration( const sd::AnnotationVector& );
}

/* this are the ids for page properties */
enum WID_PAGE
{
    WID_PAGE_LEFT, WID_PAGE_RIGHT, WID_PAGE_TOP, WID_PAGE_BOTTOM, WID_PAGE_WIDTH,
    WID_PAGE_HEIGHT, WID_PAGE_EFFECT, WID_PAGE_CHANGE, WID_PAGE_SPEED, WID_PAGE_NUMBER,
    WID_PAGE_ORIENT, WID_PAGE_LAYOUT, WID_PAGE_DURATION, WID_PAGE_LDNAME, WID_PAGE_LDBITMAP,
    WID_PAGE_BACK, WID_PAGE_PREVIEW, WID_PAGE_PREVIEWBITMAP, WID_PAGE_VISIBLE, WID_PAGE_SOUNDFILE, WID_PAGE_BACKFULL,
    WID_PAGE_BACKVIS, WID_PAGE_BACKOBJVIS, WID_PAGE_USERATTRIBS, WID_PAGE_BOOKMARK, WID_PAGE_ISDARK,
    WID_PAGE_HEADERVISIBLE, WID_PAGE_HEADERTEXT, WID_PAGE_FOOTERVISIBLE, WID_PAGE_FOOTERTEXT,
    WID_PAGE_PAGENUMBERVISIBLE, WID_PAGE_DATETIMEVISIBLE, WID_PAGE_DATETIMEFIXED,
    WID_PAGE_DATETIMETEXT, WID_PAGE_DATETIMEFORMAT, WID_TRANSITION_TYPE, WID_TRANSITION_SUBTYPE,
    WID_TRANSITION_DIRECTION, WID_TRANSITION_FADE_COLOR, WID_TRANSITION_DURATION, WID_LOOP_SOUND,
    WID_NAVORDER
};

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

static sal_Char sEmptyPageName[sizeof("page")] = "page";

/** this function stores the property maps for draw pages in impress and draw */
const SvxItemPropertySet* ImplGetDrawPagePropertySet( sal_Bool bImpress, PageKind ePageKind )
{
    static const SfxItemPropertyMapEntry aDrawPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE( beans::XPropertySet ),                  beans::PropertyAttribute::MAYBEVOID,0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_CHANGE),           WID_PAGE_CHANGE,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_DURATION),         WID_PAGE_DURATION,  &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_EFFECT),           WID_PAGE_EFFECT,    &::getCppuType((const presentation::FadeEffect*)0),     0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    &::getCppuType((const sal_Int16*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE( awt::XBitmap),                          beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_SPEED),            WID_PAGE_SPEED,     &::getCppuType((const presentation::AnimationSpeed*)0), 0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEWBITMAP),    WID_PAGE_PREVIEWBITMAP, SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_VISIBLE),          WID_PAGE_VISIBLE,   &::getBooleanCppuType(),                        0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_SOUNDFILE),         WID_PAGE_SOUNDFILE, &::getCppuType((const Any*)0),              0, 0},
        { MAP_CHAR_LEN(sUNO_Prop_IsBackgroundVisible),  WID_PAGE_BACKVIS,   &::getBooleanCppuType(),                        0, 0},
        { MAP_CHAR_LEN(sUNO_Prop_IsBackgroundObjectsVisible),   WID_PAGE_BACKOBJVIS,    &::getBooleanCppuType(),                        0, 0},
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, &::getCppuType((const Reference< ::com::sun::star::container::XNameContainer >*)0)  ,         0,     0},
        { MAP_CHAR_LEN(sUNO_Prop_BookmarkURL),          WID_PAGE_BOOKMARK,  &::getCppuType((const OUString*)0),             0,  0},
        { MAP_CHAR_LEN("IsBackgroundDark" ),            WID_PAGE_ISDARK,    &::getBooleanCppuType(),                        beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("FooterText"),                   WID_PAGE_FOOTERTEXT, &::getCppuType((const OUString*)0),                0,  0},
        { MAP_CHAR_LEN("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, &::getBooleanCppuType(),                  0, 0},
        { MAP_CHAR_LEN("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("DateTimeText"),                 WID_PAGE_DATETIMETEXT, &::getCppuType((const OUString*)0),              0,  0},
        { MAP_CHAR_LEN("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, &::getCppuType((const sal_Int32*)0),           0,  0},
        { MAP_CHAR_LEN("TransitionType"),               WID_TRANSITION_TYPE, &::getCppuType((const sal_Int16*)0),           0,  0},
        { MAP_CHAR_LEN("TransitionSubtype"),            WID_TRANSITION_SUBTYPE, &::getCppuType((const sal_Int16*)0),            0,  0},
        { MAP_CHAR_LEN("TransitionDirection"),          WID_TRANSITION_DIRECTION, &::getCppuType((const sal_Bool*)0),           0,  0},
        { MAP_CHAR_LEN("TransitionFadeColor"),          WID_TRANSITION_FADE_COLOR, &::getCppuType((const sal_Int32*)0),         0,  0},
        { MAP_CHAR_LEN("TransitionDuration"),           WID_TRANSITION_DURATION, &::getCppuType((const double*)0),          0,  0},
        { MAP_CHAR_LEN("LoopSound"),                    WID_LOOP_SOUND, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("NavigationOrder"),              WID_NAVORDER, &::com::sun::star::container::XIndexAccess::static_type(),0,  0},
        {0,0,0,0,0,0}
    };

#define DRAW_PAGE_NOTES_PROPERTIES \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    &::getCppuType((const sal_Int16*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE( awt::XBitmap),                          beans::PropertyAttribute::READONLY, 0},                                \
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},                                                                \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},                                                                \
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,      0,     0},\
        { MAP_CHAR_LEN("IsHeaderVisible"),              WID_PAGE_HEADERVISIBLE, &::getBooleanCppuType(),                    0, 0},                                                                 \
        { MAP_CHAR_LEN("HeaderText"),                   WID_PAGE_HEADERTEXT, &::getCppuType((const OUString*)0),                0,  0},                                                            \
        { MAP_CHAR_LEN("IsBackgroundDark" ),            WID_PAGE_ISDARK,    &::getBooleanCppuType(),                        beans::PropertyAttribute::READONLY, 0},                                \
        { MAP_CHAR_LEN("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, &::getBooleanCppuType(),                    0, 0},                                                                 \
        { MAP_CHAR_LEN("FooterText"),                   WID_PAGE_FOOTERTEXT, &::getCppuType((const OUString*)0),                0,  0},                                                            \
        { MAP_CHAR_LEN("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, &::getBooleanCppuType(),                    0, 0},                                                             \
        { MAP_CHAR_LEN("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, &::getBooleanCppuType(),                  0, 0},                                                                 \
        { MAP_CHAR_LEN("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, &::getBooleanCppuType(),                    0, 0},                                                                 \
        { MAP_CHAR_LEN("DateTimeText"),                 WID_PAGE_DATETIMETEXT, &::getCppuType((const OUString*)0),              0,  0},                                                            \
        { MAP_CHAR_LEN("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, &::getCppuType((const sal_Int32*)0),           0,  0},                                                            \
        { MAP_CHAR_LEN("NavigationOrder"),              WID_NAVORDER, &::com::sun::star::container::XIndexAccess::static_type(),0,  0},                                                            \
        {0,0,0,0,0,0}

    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyMap_Impl[] =
    {
        // this must be the first two entries so they can be excluded for PK_STANDARD
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE( beans::XPropertySet ),                  beans::PropertyAttribute::MAYBEVOID,0},
        DRAW_PAGE_NOTES_PROPERTIES
    };
    static const SfxItemPropertyMapEntry aDrawPageNotesHandoutPropertyNoBackMap_Impl[] =
    {
        DRAW_PAGE_NOTES_PROPERTIES
    };

#define GRAPHIC_PAGE_PROPERTIES \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE(awt::XBitmap),                           beans::PropertyAttribute::READONLY, 0},                                             \
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},                                                                             \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},    \
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEWBITMAP),    WID_PAGE_PREVIEWBITMAP, SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},\
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, &::getCppuType((const Reference< ::com::sun::star::container::XNameContainer >*)0)  ,         0,     0},                          \
        { MAP_CHAR_LEN(sUNO_Prop_BookmarkURL),          WID_PAGE_BOOKMARK,  &::getCppuType((const OUString*)0),             0,  0},                                                                             \
        { MAP_CHAR_LEN("IsBackgroundDark" ),            WID_PAGE_ISDARK,    &::getBooleanCppuType(),                        beans::PropertyAttribute::READONLY, 0},                                             \
        { MAP_CHAR_LEN("NavigationOrder"),              WID_NAVORDER, &::com::sun::star::container::XIndexAccess::static_type(),0,  0},                                                                         \
        {0,0,0,0,0,0}

    static const SfxItemPropertyMapEntry aGraphicPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE( beans::XPropertySet),                   beans::PropertyAttribute::MAYBEVOID,0},
        GRAPHIC_PAGE_PROPERTIES
    };
    static const SfxItemPropertyMapEntry aGraphicPagePropertyNoBackMap_Impl[] =
    {
        GRAPHIC_PAGE_PROPERTIES
    };

    bool bWithoutBackground = ePageKind != PK_STANDARD && ePageKind != PK_HANDOUT;
    const SvxItemPropertySet* pRet = 0;
    if( bImpress )
    {
        if( ePageKind == PK_STANDARD )
        {
            //PK_STANDARD always has a background property
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
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE(beans::XPropertySet),                    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE(awt::XBitmap),                           beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN("BackgroundFullSize"),           WID_PAGE_BACKFULL,  &::getBooleanCppuType(),                        0, 0},
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, &::getCppuType((const Reference< ::com::sun::star::container::XNameContainer >*)0)  ,         0,     0},
        { MAP_CHAR_LEN("IsBackgroundDark" ),            WID_PAGE_ISDARK,    &::getBooleanCppuType(),                        beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}
    };

    static const SfxItemPropertyMapEntry aHandoutMasterPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    &::getCppuType((const sal_Int16*)0),            0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),WID_PAGE_USERATTRIBS, &::getCppuType((const Reference< ::com::sun::star::container::XNameContainer >*)0)  ,         0,     0},
        { MAP_CHAR_LEN("IsBackgroundDark" ),            WID_PAGE_ISDARK,    &::getBooleanCppuType(),                        beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("IsHeaderVisible"),              WID_PAGE_HEADERVISIBLE, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("HeaderText"),                   WID_PAGE_HEADERTEXT, &::getCppuType((const OUString*)0),                0,  0},
        { MAP_CHAR_LEN("IsFooterVisible"),              WID_PAGE_FOOTERVISIBLE, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("FooterText"),                   WID_PAGE_FOOTERTEXT, &::getCppuType((const OUString*)0),                0,  0},
        { MAP_CHAR_LEN("IsPageNumberVisible"),          WID_PAGE_PAGENUMBERVISIBLE, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("IsDateTimeVisible"),            WID_PAGE_DATETIMEVISIBLE, &::getBooleanCppuType(),                  0, 0},
        { MAP_CHAR_LEN("IsDateTimeFixed"),              WID_PAGE_DATETIMEFIXED, &::getBooleanCppuType(),                    0, 0},
        { MAP_CHAR_LEN("DateTimeText"),                 WID_PAGE_DATETIMETEXT, &::getCppuType((const OUString*)0),              0,  0},
        { MAP_CHAR_LEN("DateTimeFormat"),               WID_PAGE_DATETIMEFORMAT, &::getCppuType((const sal_Int32*)0),           0,  0},
        {0,0,0,0,0,0}
    };

    const SvxItemPropertySet* pRet = 0;
    if( ePageKind == PK_HANDOUT )
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

const ::com::sun::star::uno::Sequence< sal_Int8 > & SdGenericDrawPage::getUnoTunnelId() throw()
{
    return theSdGenericDrawPageUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SdGenericDrawPage::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException)
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
:       SvxFmDrawPage( (SdrPage*) pInPage ),
        SdUnoSearchReplaceShape(this),
        mpModel     ( _pModel ),
        mpSdrModel(0),
        mnTempPageNumber(0),
        mpPropSet   ( _pSet ),
        mbIsImpressDocument(false)
{
    mpSdrModel = SvxFmDrawPage::mpModel;
    if( mpModel )
        mbIsImpressDocument = mpModel->IsImpressDocument() ? true : false;

}

SdGenericDrawPage::~SdGenericDrawPage() throw()
{
}

void SdGenericDrawPage::throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException )
{
    if( (SvxFmDrawPage::mpModel == 0) || (mpModel == 0) || (SvxFmDrawPage::mpPage == 0) )
        throw lang::DisposedException();
}

SdXImpressDocument* SdGenericDrawPage::GetModel() const
{
    if( mpSdrModel != SvxFmDrawPage::mpModel )
    {
        const_cast< SdGenericDrawPage* >(this)->mpSdrModel = SvxFmDrawPage::mpModel;
        if( mpSdrModel )
        {
            uno::Reference< uno::XInterface > xModel( SvxFmDrawPage::mpModel->getUnoModel() );
            const_cast< SdGenericDrawPage*>(this)->mpModel = SdXImpressDocument::getImplementation( xModel );
            if( mpModel )
                const_cast< SdGenericDrawPage*>(this)->mbIsImpressDocument = mpModel->IsImpressDocument() ? true : false;
        }
        else
        {
            const_cast< SdGenericDrawPage* >(this)->mpModel = 0;
        }
    }

    return mpModel;
}

// this is called whenever a SdrObject must be created for a empty api shape wrapper
SdrObject * SdGenericDrawPage::_CreateSdrObject( const Reference< drawing::XShape >& xShape ) throw()
{
    if( NULL == SvxFmDrawPage::mpPage || !xShape.is() )
        return NULL;

    String aType( xShape->getShapeType() );
    const String aPrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.") );
    if( aType.CompareTo( aPrefix, aPrefix.Len() ) != 0 )
    {
        SdrObject* pObj = SvxFmDrawPage::_CreateSdrObject( xShape );
        if( pObj && ( (pObj->GetObjInventor() != SdrInventor) || (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
            if( pDoc )
                pObj->NbcSetStyleSheet( pDoc->GetDefaultStyleSheet(), sal_True );
        }
        return pObj;
    }

    aType = aType.Copy( aPrefix.Len() );

    PresObjKind eObjKind = PRESOBJ_NONE;

    if( aType.EqualsAscii( "TitleTextShape" ) )
    {
        eObjKind = PRESOBJ_TITLE;
    }
    else if( aType.EqualsAscii( "OutlinerShape" ) )
    {
        eObjKind = PRESOBJ_OUTLINE;
    }
    else if( aType.EqualsAscii( "SubtitleShape" ) )
    {
        eObjKind = PRESOBJ_TEXT;
    }
    else if( aType.EqualsAscii( "OLE2Shape" ) )
    {
        eObjKind = PRESOBJ_OBJECT;
    }
    else if( aType.EqualsAscii( "ChartShape" ) )
    {
        eObjKind = PRESOBJ_CHART;
    }
    else if( aType.EqualsAscii( "CalcShape" ) )
    {
        eObjKind = PRESOBJ_CALC;
    }
    else if( aType.EqualsAscii( "TableShape" ) )
    {
        eObjKind = PRESOBJ_TABLE;
    }
    else if( aType.EqualsAscii( "GraphicObjectShape" ) )
    {
        eObjKind = PRESOBJ_GRAPHIC;
    }
    else if( aType.EqualsAscii( "OrgChartShape" ) )
    {
        eObjKind = PRESOBJ_ORGCHART;
    }
    else if( aType.EqualsAscii( "PageShape" ) )
    {
        if( GetPage()->GetPageKind() == PK_NOTES && GetPage()->IsMasterPage() )
            eObjKind = PRESOBJ_TITLE;
        else
            eObjKind = PRESOBJ_PAGE;
    }
    else if( aType.EqualsAscii( "NotesShape" ) )
    {
        eObjKind = PRESOBJ_NOTES;
    }
    else if( aType.EqualsAscii( "HandoutShape" ) )
    {
        eObjKind = PRESOBJ_HANDOUT;
    }
    else if( aType.EqualsAscii( "FooterShape" ) )
    {
        eObjKind = PRESOBJ_FOOTER;
    }
    else if( aType.EqualsAscii( "HeaderShape" ) )
    {
        eObjKind = PRESOBJ_HEADER;
    }
    else if( aType.EqualsAscii( "SlideNumberShape" ) )
    {
        eObjKind = PRESOBJ_SLIDENUMBER;
    }
    else if( aType.EqualsAscii( "DateTimeShape" ) )
    {
        eObjKind = PRESOBJ_DATETIME;
    }
    else if( aType.EqualsAscii( "MediaShape" ) )
    {
        eObjKind = PRESOBJ_MEDIA;
    }

    Rectangle aRect( eObjKind == PRESOBJ_TITLE ? GetPage()->GetTitleRect() : GetPage()->GetLayoutRect()  );

    const awt::Point aPos( aRect.Left(), aRect.Top() );
    xShape->setPosition( aPos );

    const awt::Size aSize( aRect.GetWidth(), aRect.GetHeight() );
    xShape->setSize( aSize );

    SdrObject *pPresObj = 0;
    if( (eObjKind == PRESOBJ_TABLE) || (eObjKind == PRESOBJ_MEDIA) )
    {
        pPresObj = SvxFmDrawPage::_CreateSdrObject( xShape );
        if( pPresObj )
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
            if( pDoc )
                pPresObj->NbcSetStyleSheet( pDoc->GetDefaultStyleSheet(), sal_True );
            GetPage()->InsertPresObj( pPresObj, eObjKind );
        }
    }
    else
    {
        pPresObj = GetPage()->CreatePresObj( eObjKind, sal_False, aRect, sal_True );
    }

    if( pPresObj )
        pPresObj->SetUserCall( GetPage() );

    return pPresObj;
}

// XInterface
Any SAL_CALL SdGenericDrawPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    Any aAny;

    QUERYINT( beans::XPropertySet );
    else QUERYINT( container::XNamed );
    else QUERYINT( util::XReplaceable );
    else QUERYINT( util::XSearchable );
    else QUERYINT( document::XLinkTargetSupplier );
    else QUERYINT( drawing::XShapeCombiner );
    else QUERYINT( drawing::XShapeBinder );
    else QUERYINT( beans::XMultiPropertySet );
    else if( rType == ITYPE( office::XAnnotationAccess ) )
    {
        return Any( Reference< office::XAnnotationAccess >( this ) );
    }
    else if( rType == ITYPE( XAnimationNodeSupplier ) )
    {
        if( mbIsImpressDocument )
        {
            const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PK_STANDARD;

            if( ePageKind == PK_STANDARD )
                return makeAny( Reference< XAnimationNodeSupplier >( this ) );
        }
    }
    else
        return SvxDrawPage::queryInterface( rType );

    return aAny;
}

// XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL SdGenericDrawPage::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;
    throwIfDisposed();
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdGenericDrawPage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
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
                GetPage()->SetAutoLayout( (AutoLayout)nValue, sal_True );
                break;
            case WID_PAGE_DURATION:
                GetPage()->SetTime((sal_uInt32)nValue);
                break;
            }
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

            Orientation eOri = (((view::PaperOrientation)nEnum) == view::PaperOrientation_PORTRAIT)?ORIENTATION_PORTRAIT:ORIENTATION_LANDSCAPE;

            if( eOri != GetPage()->GetOrientation() )
            {
                SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
            sal_Bool    bVisible = sal_False;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetExcluded( bVisible == sal_False );
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
                sal_Bool bStopSound = sal_False;
                if( aValue >>= bStopSound )
                {
                    GetPage()->SetStopSound( bStopSound ? true : false );
                    break;
                }
            }


            throw lang::IllegalArgumentException();
        }
        case WID_LOOP_SOUND:
        {
            sal_Bool bLoop = sal_False;
            if( ! (aValue >>= bLoop) )
                throw lang::IllegalArgumentException();

            GetPage()->SetLoopSound( bLoop ? true : false );
            break;
        }
        case WID_PAGE_BACKFULL:
        {
            sal_Bool    bFullSize = sal_False;
            if( ! ( aValue >>= bFullSize ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetBackgroundFullSize( bFullSize );
            break;
        }
        case WID_PAGE_BACKVIS:
        {
            sal_Bool bVisible = sal_False;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdrPage* pPage = GetPage();
            if( pPage )
            {
                SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
                if( pDoc->GetMasterPageCount() )
                {
                    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False), bVisible);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                }
            }
            break;
        }
        case WID_PAGE_BACKOBJVIS:
        {
            sal_Bool bVisible = sal_False;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();

            SdrPage* pPage = GetPage();
            if( pPage )
            {
                SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
                if( pDoc->GetMasterPageCount() )
                {
                    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                    SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                    aVisibleLayers.Set(rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False), bVisible);
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
                sal_Bool bVisible = sal_False;
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
                sal_Bool bVisible = sal_False;
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
                sal_Bool bVisible = sal_False;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbSlideNumberVisible = bVisible;
                break;
            }
            case WID_PAGE_DATETIMEVISIBLE:
            {
                sal_Bool bVisible = sal_False;
                if( ! ( aValue >>= bVisible ) )
                    throw lang::IllegalArgumentException();

                aHeaderFooterSettings.mbDateTimeVisible = bVisible;
                break;
            }
            case WID_PAGE_DATETIMEFIXED:
            {
                sal_Bool bVisible = sal_False;
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
            if( (GetPage()->GetPageKind() == PK_HANDOUT) && !GetPage()->IsMasterPage() )
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
            sal_Bool bValue = sal_False;
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
            throw beans::UnknownPropertyException();
    }

    GetModel()->SetModified();
}

Any SAL_CALL SdGenericDrawPage::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Any aAny;

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
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
        aAny = ::cppu::int2enum( (sal_Int32)((GetPage()->GetOrientation() == ORIENTATION_PORTRAIT)? view::PaperOrientation_PORTRAIT: view::PaperOrientation_LANDSCAPE), ::getCppuType((const view::PaperOrientation*)0) );
        break;
    case WID_PAGE_EFFECT:
        aAny = ::cppu::int2enum( (sal_Int32)GetPage()->GetFadeEffect(), ::getCppuType((const presentation::FadeEffect*)0) );
        break;
    case WID_PAGE_CHANGE:
        aAny <<= (sal_Int32)( GetPage()->GetPresChange() );
        break;
    case WID_PAGE_SPEED:
        {
            const double fDuration = GetPage()->getTransitionDuration();
            aAny = ::cppu::int2enum( fDuration < 2.0 ? 2 : (fDuration > 2.0 ? 0 : 1), ::getCppuType((const presentation::AnimationSpeed*)0) );
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
        aAny <<= (sal_Int32)(GetPage()->GetTime());
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
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
            if ( pDoc )
            {
                ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
                if ( pDocShell )
                {
                    sal_uInt16 nPgNum = 0;
                    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
                    sal_uInt16 nPageNumber = (sal_uInt16)( ( GetPage()->GetPageNum() - 1 ) >> 1 );
                    while( nPgNum < nPageCount )
                    {
                        pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PK_STANDARD ), nPgNum == nPageNumber );
                        nPgNum++;
                    }
                    ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                        pDocShell->GetPreviewMetaFile();
                    if ( pMetaFile )
                    {
                        Point   aPoint;
                        Size    aSize( GetPage()->GetSize() );
                        pMetaFile->AddAction( (MetaAction*) new MetaFillColorAction( COL_WHITE, sal_True ), 0 );
                        pMetaFile->AddAction( (MetaAction*) new MetaRectAction( Rectangle( aPoint, aSize ) ), 1 );
                        pMetaFile->SetPrefMapMode( MAP_100TH_MM );
                        pMetaFile->SetPrefSize( aSize );

                        SvMemoryStream aDestStrm( 65535, 65535 );
                        ConvertGDIMetaFileToWMF( *pMetaFile, aDestStrm, NULL, sal_False );
                        Sequence<sal_Int8> aSeq( (sal_Int8*)aDestStrm.GetData(), aDestStrm.Tell() );
                        aAny <<= aSeq;
                    }
                }
            }
        }
        break;

    case WID_PAGE_PREVIEWBITMAP :
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
            if ( pDoc )
            {
                ::sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
                if ( pDocShell )
                {
                    sal_uInt16 nPgNum = 0;
                    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
                    sal_uInt16 nPageNumber = (sal_uInt16)( ( GetPage()->GetPageNum() - 1 ) >> 1 );
                    while( nPgNum < nPageCount )
                    {
                        pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PK_STANDARD ), nPgNum == nPageNumber );
                        nPgNum++;
                    }
                    ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                        pDocShell->GetPreviewMetaFile();
                    BitmapEx aBitmap;
                    if ( pMetaFile && pMetaFile->CreateThumbnail( 160, /* magic value taken from GraphicHelper::getThumbnailFormatFromGDI_Impl() */
                                                                  aBitmap ) )
                    {
                        SvMemoryStream aMemStream;
                        aBitmap.GetBitmap().Write( aMemStream, sal_False, sal_False );
                        uno::Sequence<sal_Int8> aSeq( (sal_Int8*)aMemStream.GetData(), aMemStream.Tell() );
                        aAny <<= aSeq;
                    }
                }
            }
        }
        break;

    case WID_PAGE_VISIBLE :
    {
        sal_Bool bVisible = GetPage()->IsExcluded() == sal_False;
        aAny <<= Any( &bVisible, ::getBooleanCppuType() );
        break;
    }

    case WID_PAGE_SOUNDFILE :
    {
        if( GetPage()->IsStopSound() )
        {
            aAny <<= sal_True;
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
        aAny <<= (sal_Bool)GetPage()->IsLoopSound();
        break;
    }
    case WID_PAGE_BACKFULL:
    {
        sal_Bool bFullSize = GetPage()->IsBackgroundFullSize();
        aAny = Any( &bFullSize, ::getBooleanCppuType() );
        break;
    }
    case WID_PAGE_BACKVIS:
    {
        SdrPage* pPage = GetPage();
        if( pPage )
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
            if( pDoc->GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                aAny <<= (sal_Bool)aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False));
            }
            else
            {
                aAny <<= (sal_Bool)sal_False;
            }
        }
        break;
    }
    case WID_PAGE_BACKOBJVIS:
    {
        SdrPage* pPage = GetPage();
        if( pPage )
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
            if( pDoc->GetMasterPageCount() )
            {
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                aAny <<= (sal_Bool)aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False));
            }
            else
            {
                aAny <<= (sal_Bool)sal_False;
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
        aAny <<= (sal_Bool)GetPage()->GetPageBackgroundColor().IsDark();
        break;
    }
    case WID_PAGE_HEADERVISIBLE:
        aAny <<= (sal_Bool)GetPage()->getHeaderFooterSettings().mbHeaderVisible;
        break;
    case WID_PAGE_HEADERTEXT:
        {
            const OUString aText( GetPage()->getHeaderFooterSettings().maHeaderText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_FOOTERVISIBLE:
        aAny <<= (sal_Bool)GetPage()->getHeaderFooterSettings().mbFooterVisible;
        break;
    case WID_PAGE_FOOTERTEXT:
        {
            const OUString aText( GetPage()->getHeaderFooterSettings().maFooterText );
            aAny <<= aText;
        }
        break;
    case WID_PAGE_PAGENUMBERVISIBLE:
        aAny <<= (sal_Bool)GetPage()->getHeaderFooterSettings().mbSlideNumberVisible;
        break;
    case WID_PAGE_DATETIMEVISIBLE:
        aAny <<= (sal_Bool)GetPage()->getHeaderFooterSettings().mbDateTimeVisible;
        break;
    case WID_PAGE_DATETIMEFIXED:
        aAny <<= (sal_Bool)GetPage()->getHeaderFooterSettings().mbDateTimeIsFixed;
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
        throw beans::UnknownPropertyException();
    }
    return aAny;
}

void SAL_CALL SdGenericDrawPage::addPropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::removePropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// XMultiPropertySet
void SAL_CALL SdGenericDrawPage::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw (beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, RuntimeException )
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

Sequence< Any > SAL_CALL SdGenericDrawPage::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw (RuntimeException)
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

void SAL_CALL SdGenericDrawPage::addPropertiesChangeListener( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  ) throw (RuntimeException)
{
}

void SAL_CALL SdGenericDrawPage::removePropertiesChangeListener( const Reference< beans::XPropertiesChangeListener >&  ) throw (RuntimeException)
{
}

void SAL_CALL SdGenericDrawPage::firePropertiesChangeEvent( const Sequence< OUString >& , const Reference< beans::XPropertiesChangeListener >&  ) throw (RuntimeException)
{
}

Reference< drawing::XShape >  SdGenericDrawPage::_CreateShape( SdrObject *pObj ) const throw()
{
    DBG_ASSERT( GetPage(), "SdGenericDrawPage::_CreateShape(), can't create shape for disposed page!" );
    DBG_ASSERT( pObj, "SdGenericDrawPage::_CreateShape(), invalid call with pObj == 0!" );

    if( GetPage() && pObj )
    {
        PresObjKind eKind = GetPage()->GetPresObjKind(pObj);

        SvxShape* pShape = NULL;

        if(pObj->GetObjInventor() == SdrInventor)
        {
            sal_uInt32 nInventor = pObj->GetObjIdentifier();
            switch( nInventor )
            {
            case OBJ_TITLETEXT:
                pShape = new SvxShapeText( pObj );
                if( GetPage()->GetPageKind() == PK_NOTES && GetPage()->IsMasterPage() )
                {
                    // fake a empty PageShape if its a title shape on the master page
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
            xShape = SvxFmDrawPage::_CreateShape( pObj );


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

        // SdXShape aggregiert SvxShape
        new SdXShape( SvxShape::getImplementation( xShape ), GetModel() );
        return xShape;
    }
    else
    {
        return SvxFmDrawPage::_CreateShape( pObj );
    }

}

//----------------------------------------------------------------------

// XServiceInfo
Sequence< OUString > SAL_CALL SdGenericDrawPage::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    Sequence< OUString > aSeq( SvxFmDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, 3, "com.sun.star.drawing.GenericDrawPage",
                                                  "com.sun.star.document.LinkTarget",
                                                  "com.sun.star.document.LinkTargetSupplier");
    return aSeq;
}

//----------------------------------------------------------------------

// XLinkTargetSupplier
Reference< container::XNameAccess > SAL_CALL SdGenericDrawPage::getLinks(  )
    throw(uno::RuntimeException)
{
    return new SdPageLinkTargets( (SdGenericDrawPage*)this );
}

//----------------------------------------------------------------------

void SdGenericDrawPage::setBackground( const Any& ) throw(lang::IllegalArgumentException)
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

//----------------------------------------------------------------------

void SdGenericDrawPage::getBackground( Any& ) throw()
{
    OSL_FAIL( "Don't call me, I'm useless!" );
}

//----------------------------------------------------------------------

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
            aRet.append( (sal_Unicode)'#' );
            aRet.append( aBookmarkName );
        }
    }

    return aRet.makeStringAndClear();
}

//----------------------------------------------------------------------
void SdGenericDrawPage::setBookmarkURL( rtl::OUString& rURL )
{
    if( SvxFmDrawPage::mpPage )
    {
        sal_Int32 nIndex = rURL.indexOf( (sal_Unicode)'#' );
        if( nIndex != -1 )
        {
            const String aFileName( rURL.copy( 0, nIndex ) );
            const String aBookmarkName( SdDrawPage::getUiNameFromPageApiName( rURL.copy( nIndex+1 )  ) );

            if( aFileName.Len() && aBookmarkName.Len() )
            {
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->DisconnectLink();
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->SetFileName( aFileName );
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->SetBookmarkName( aBookmarkName );
                static_cast<SdPage*>(SvxFmDrawPage::mpPage)->ConnectLink();
            }
        }
    }
}

//----------------------------------------------------------------------
Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::combine( const Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    DBG_ASSERT(SvxFmDrawPage::mpPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(mpView, "SdrView ist NULL! [CL]");

    Reference< drawing::XShape > xShape;
    if(mpView==NULL||!xShapes.is()||GetPage()==NULL)
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );

    _SelectObjectsInView( xShapes, pPageView );

    mpView->CombineMarkedObjects( sal_False );

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
            xShape = Reference< drawing::XShape >::query( pObj->getUnoShape() );
    }

    mpView->HideSdrPage();

    GetModel()->SetModified();

    return xShape;
}

//----------------------------------------------------------------------
void SAL_CALL SdGenericDrawPage::split( const Reference< drawing::XShape >& xGroup )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==NULL||!xGroup.is()||GetPage()==NULL)
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );
    _SelectObjectInView( xGroup, pPageView );
    mpView->DismantleMarkedObjects( sal_False );
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

//----------------------------------------------------------------------
Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::bind( const Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Reference< drawing::XShape > xShape;
    if(mpView==NULL||!xShapes.is()||GetPage()==NULL)
        return xShape;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );

    _SelectObjectsInView( xShapes, pPageView );

    mpView->CombineMarkedObjects( sal_True );

    mpView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj )
            xShape = Reference< drawing::XShape >::query( pObj->getUnoShape() );
    }

    mpView->HideSdrPage();

    GetModel()->SetModified();

    return xShape;
}

//----------------------------------------------------------------------
void SAL_CALL SdGenericDrawPage::unbind( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(mpView==NULL||!xShape.is()||GetPage()==NULL)
        return;

    SdrPageView* pPageView = mpView->ShowSdrPage( GetPage() );
    _SelectObjectInView( xShape, pPageView );
    mpView->DismantleMarkedObjects( sal_True );
    mpView->HideSdrPage();

    GetModel()->SetModified();
}

void SdGenericDrawPage::SetLftBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetLftBorder() )
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
            if( pViewSh->ISA(::sd::DrawViewShell ) )
                static_cast< ::sd::DrawViewShell*>(pViewSh)->ResetActualPage();

            Size aPageSize = pDoc->GetSdPage(0, ePageKind)->GetSize();
            const long nWidth = aPageSize.Width();
            const long nHeight = aPageSize.Height();

            Point aPageOrg = Point(nWidth, nHeight / 2);
            Size aViewSize = Size(nWidth * 3, nHeight * 2);

            pDoc->SetMaxObjSize(aViewSize);

            pViewSh->InitWindows(aPageOrg, aViewSize, Point(-1, -1), sal_True);

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

        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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

        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
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
    mpModel = 0;
    SvxFmDrawPage::disposing();
}

// XAnimationNodeSupplier
Reference< XAnimationNode > SAL_CALL SdGenericDrawPage::getAnimationNode() throw (uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    SdPage *pSdPage = static_cast<SdPage*>(SvxFmDrawPage::mpPage);


    return pSdPage->getAnimationNode();
}

//========================================================================
// SdPageLinkTargets
//========================================================================

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
    throw(uno::RuntimeException)
{
    return ITYPE(beans::XPropertySet);
}

sal_Bool SAL_CALL SdPageLinkTargets::hasElements()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( !aStr.Len() && pObj->ISA( SdrOle2Obj ) )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
            if( aStr.Len() )
                return sal_True;
        }
    }

    return sal_False;
}

// container::XNameAccess

// XNameAccess
Any SAL_CALL SdPageLinkTargets::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
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
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    sal_uInt32 nObjCount = 0;

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( !aStr.Len() && pObj->ISA( SdrOle2Obj ) )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
            if( aStr.Len() )
                nObjCount++;
        }
    }

    Sequence< OUString > aSeq( nObjCount );
    if( nObjCount > 0 )
    {
        OUString* pStr = aSeq.getArray();

        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( !aStr.Len() && pObj->ISA( SdrOle2Obj ) )
                aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
            if( aStr.Len() )
                *pStr++ = aStr;
        }
    }

    return aSeq;
}

sal_Bool SAL_CALL SdPageLinkTargets::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    return FindObject( aName ) != NULL;
}

SdrObject* SdPageLinkTargets::FindObject( const String& rName ) const throw()
{
    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage == NULL )
        return NULL;

    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        String aStr( pObj->GetName() );
        if( !aStr.Len() && pObj->ISA( SdrOle2Obj ) )
            aStr = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
        if( aStr.Len() && (aStr == rName) )
            return pObj;
    }

    return NULL;
}

// XServiceInfo
OUString SAL_CALL SdPageLinkTargets::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( "SdPageLinkTargets" );
}

sal_Bool SAL_CALL SdPageLinkTargets::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

Sequence< OUString > SAL_CALL SdPageLinkTargets::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    const OUString aSN( "com.sun.star.document.LinkTargets" );
    Sequence< OUString > aSeq( &aSN, 1);
    return aSeq;
}

//========================================================================
// SdDrawPage
//========================================================================

SdDrawPage::SdDrawPage(  SdXImpressDocument* pModel, SdPage* pPage ) throw()
: SdGenericDrawPage( pModel, pPage, ImplGetDrawPagePropertySet( pModel->IsImpressDocument(), pPage->GetPageKind() ) )
{
}

SdDrawPage::~SdDrawPage() throw()
{
}

// XInterface
Any SAL_CALL SdDrawPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    if( rType == ITYPE( drawing::XMasterPageTarget ) )
    {
        return makeAny( Reference< drawing::XMasterPageTarget >( this ) );
    }
    else
    {
        if( mbIsImpressDocument )
        {
            const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PK_STANDARD;

            if( ePageKind != PK_HANDOUT && rType == ITYPE( presentation::XPresentationPage ) )
            {
                return makeAny( Reference< presentation::XPresentationPage >( this ) );
            }
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
Sequence< uno::Type > SAL_CALL SdDrawPage::getTypes() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( maTypeSequence.getLength() == 0 )
    {
        const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PK_STANDARD;
        sal_Bool bPresPage = mbIsImpressDocument && ePageKind != PK_HANDOUT;

        // Collect the types of this class.
        ::std::vector<uno::Type> aTypes;
        aTypes.reserve(13);
        aTypes.push_back(ITYPE(drawing::XDrawPage));
        aTypes.push_back(ITYPE(beans::XPropertySet));
        aTypes.push_back(ITYPE(container::XNamed));
        aTypes.push_back(ITYPE(drawing::XMasterPageTarget));
        aTypes.push_back(ITYPE(lang::XServiceInfo));
        aTypes.push_back(ITYPE(util::XReplaceable));
        aTypes.push_back(ITYPE(document::XLinkTargetSupplier));
        aTypes.push_back(ITYPE( drawing::XShapeCombiner ));
        aTypes.push_back(ITYPE( drawing::XShapeBinder ));
        aTypes.push_back(ITYPE( office::XAnnotationAccess ));
        aTypes.push_back(ITYPE( beans::XMultiPropertySet ));
        if( bPresPage )
            aTypes.push_back(ITYPE(presentation::XPresentationPage));
        if( bPresPage && ePageKind == PK_STANDARD )
            aTypes.push_back(ITYPE(XAnimationNodeSupplier));

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

namespace
{
    class theSdDrawPageImplementationId : public rtl::Static< UnoTunnelIdInit, theSdDrawPageImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SdDrawPage::getImplementationId() throw(uno::RuntimeException)
{
    return theSdDrawPageImplementationId::get().getSeq();
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
            sBuffer.appendAscii( sEmptyPageName );
            const sal_Int32 nPageNum = ( ( pPage->GetPageNum() - 1 ) >> 1 ) + 1;
            sBuffer.append( nPageNum );
            aPageName = sBuffer.makeStringAndClear();
        }
    }

    return aPageName;
}


OUString getPageApiNameFromUiName( const String& rUIName )
{
    OUString aApiName;

    String aDefPageName(SdResId(STR_PAGE));
    aDefPageName += sal_Unicode( ' ' );

    if( rUIName.Equals( aDefPageName, 0, aDefPageName.Len() ) )
    {
        aApiName = OUString( RTL_CONSTASCII_USTRINGPARAM( sEmptyPageName ) );
        aApiName += rUIName.Copy( aDefPageName.Len() );
    }
    else
    {
        aApiName = rUIName;
    }

    return aApiName;
}

OUString SdDrawPage::getPageApiNameFromUiName( const String& rUIName )
{
    return ::getPageApiNameFromUiName( rUIName );
}

String getUiNameFromPageApiNameImpl( const OUString& rApiName )
{
    const String aDefPageName(RTL_CONSTASCII_USTRINGPARAM( sEmptyPageName ));
    if( rApiName.compareTo( aDefPageName, aDefPageName.Len() ) == 0 )
    {
        OUString aNumber( rApiName.copy( sizeof( sEmptyPageName ) - 1 ) );

        // create the page number
        sal_Int32 nPageNumber = aNumber.toInt32();

        // check if there are non number characters in the number part
        const sal_Int32 nChars = aNumber.getLength();
        const sal_Unicode* pString = aNumber.getStr();
        sal_Int32 nChar;
        for( nChar = 0; nChar < nChars; nChar++, pString++ )
        {
            if((*pString < sal_Unicode('0')) || (*pString > sal_Unicode('9')))
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
            sBuffer.append( String(SdResId(STR_PAGE)) );
            sBuffer.append( sal_Unicode( ' ' ) );
            sBuffer.append( aNumber );
            return sBuffer.makeStringAndClear();
        }
    }

    return rApiName;
}

String SdDrawPage::getUiNameFromPageApiName( const OUString& rApiName )
{
    return getUiNameFromPageApiNameImpl( rApiName );
}

// XServiceInfo
OUString SAL_CALL SdDrawPage::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "SdDrawPage" );
}

Sequence< OUString > SAL_CALL SdDrawPage::getSupportedServiceNames() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.drawing.DrawPage" );

    if( mbIsImpressDocument )
        comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.DrawPage" );

    return aSeq;
}

sal_Bool SAL_CALL SdDrawPage::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::supportsService( ServiceName );
}

// XNamed
void SAL_CALL SdDrawPage::setName( const OUString& rName )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    DBG_ASSERT( GetPage() && !GetPage()->IsMasterPage(), "Don't call base implementation for masterpages!" );

    OUString aName( rName );

    if(GetPage() && GetPage()->GetPageKind() != PK_NOTES)
    {
        // check if this is the default 'page1234' name
        if(aName.compareToAscii( sEmptyPageName, sizeof( sEmptyPageName ) - 1 ) == 0)
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
                aName = OUString();
        }
        else
        {
            String aDefaultPageName( SdResId(STR_PAGE) );
            aDefaultPageName += sal_Unicode( ' ' );
            if( aName.compareTo( aDefaultPageName, aDefaultPageName.Len() ) == 0 )
                aName = OUString();
        }

        GetPage()->SetName( aName );

        sal_uInt16 nNotesPageNum = (GetPage()->GetPageNum()-1)>>1;
        if( GetModel()->GetDoc()->GetSdPageCount( PK_NOTES ) > nNotesPageNum )
        {
            SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( nNotesPageNum, PK_NOTES );
            if( pNotesPage )
                pNotesPage->SetName(aName);
        }

        // fake a mode change to repaint the page tab bar
        ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
        ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : NULL;
        if( pViewSh && pViewSh->ISA(::sd::DrawViewShell))
        {
            ::sd::DrawViewShell* pDrawViewSh = static_cast<
                  ::sd::DrawViewShell*>(pViewSh);

            EditMode eMode = pDrawViewSh->GetEditMode();
            if( eMode == EM_PAGE )
            {
                sal_Bool bLayer = pDrawViewSh->IsLayerModeActive();

                pDrawViewSh->ChangeEditMode( eMode, !bLayer );
                pDrawViewSh->ChangeEditMode( eMode, bLayer );
            }
        }

        GetModel()->SetModified();
    }
}

OUString SAL_CALL SdDrawPage::getName()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return getPageApiName( GetPage() );
}

// XMasterPageTarget
Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getMasterPage(  )
    throw(uno::RuntimeException)
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
            xPage = uno::Reference< drawing::XDrawPage >( rMasterPage.getUnoPage(), uno::UNO_QUERY );
        }

        return xPage;
    }
    return NULL;
}

void SAL_CALL SdDrawPage::setMasterPage( const Reference< drawing::XDrawPage >& xMasterPage )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage)
    {
        SdMasterPage* pMasterPage = SdMasterPage::getImplementation( xMasterPage );
        if( pMasterPage && pMasterPage->isValid() )
        {
            SvxFmDrawPage::mpPage->TRG_ClearMasterPage();

            SdPage* pSdPage = (SdPage*) pMasterPage->GetSdrPage();
            SvxFmDrawPage::mpPage->TRG_SetMasterPage(*pSdPage);

            SvxFmDrawPage::mpPage->SetBorder(pSdPage->GetLftBorder(),pSdPage->GetUppBorder(),
                              pSdPage->GetRgtBorder(),pSdPage->GetLwrBorder() );

            SvxFmDrawPage::mpPage->SetSize( pSdPage->GetSize() );
            SvxFmDrawPage::mpPage->SetOrientation( pSdPage->GetOrientation() );
            ((SdPage*)SvxFmDrawPage::mpPage)->SetLayoutName( ( (SdPage*)pSdPage )->GetLayoutName() );

            // set notes master also
            SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PK_NOTES );

            pNotesPage->TRG_ClearMasterPage();
            sal_uInt16 nNum = (SvxFmDrawPage::mpPage->TRG_GetMasterPage()).GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*SvxFmDrawPage::mpPage->GetModel()->GetMasterPage(nNum));
            pNotesPage->SetLayoutName( ( (SdPage*)pSdPage )->GetLayoutName() );

            GetModel()->SetModified();
        }

    }
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getNotesPage()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetModel()->GetDoc() && SvxFmDrawPage::mpPage->GetPageNum() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PK_NOTES );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return NULL;
}


// XIndexAccess
sal_Int32 SAL_CALL SdDrawPage::getCount()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getCount();
}

Any SAL_CALL SdDrawPage::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    return SdGenericDrawPage::getByIndex( Index );
}

// XElementAccess
uno::Type SAL_CALL SdDrawPage::getElementType()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getElementType();
}

sal_Bool SAL_CALL SdDrawPage::hasElements()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::hasElements();
}

// XShapes
void SAL_CALL SdDrawPage::add( const Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdDrawPage::remove( const Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
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
            pObj->SetUserCall(NULL);
        }
    }

    SdGenericDrawPage::remove( xShape );
}

void SdDrawPage::setBackground( const Any& rValue )
    throw( lang::IllegalArgumentException )
{
    Reference< beans::XPropertySet > xSet;

    if( !(rValue >>= xSet) && !rValue.hasValue() )
        throw lang::IllegalArgumentException();

    if( !xSet.is() )
    {
        // the easy case, no background set. Set XFILL_NONE to represent this
        GetPage()->getSdrPageProperties().PutItem(XFillStyleItem(XFILL_NONE));
        return;
    }

    // is it our own implementation?
    SdUnoPageBackground* pBack = SdUnoPageBackground::getImplementation( xSet );

    SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST );

    if( pBack )
    {
        pBack->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
    }
    else
    {
        SdUnoPageBackground* pBackground = new SdUnoPageBackground();

        Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
        Reference< beans::XPropertySet >  xDestSet( (beans::XPropertySet*)pBackground );
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

        pBackground->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
    }

    if( aSet.Count() == 0 )
    {
        // no background fill, represent by setting XFILL_NONE
        GetPage()->getSdrPageProperties().PutItem(XFillStyleItem(XFILL_NONE));
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
Reference< XAnnotation > SAL_CALL SdGenericDrawPage::createAndInsertAnnotation() throw (RuntimeException)
{
    if( !GetPage() )
        throw DisposedException();

    Reference< XAnnotation > xRet;
    GetPage()->createAnnotation(xRet);
    return xRet;
}

void SAL_CALL SdGenericDrawPage::removeAnnotation(const Reference< XAnnotation > & annotation) throw (RuntimeException, IllegalArgumentException)
{
    GetPage()->removeAnnotation(annotation);
}

Reference< XAnnotationEnumeration > SAL_CALL SdGenericDrawPage::createAnnotationEnumeration() throw (RuntimeException)
{
    return ::sd::createAnnotationEnumeration( GetPage()->getAnnotations() );
}

void SdDrawPage::getBackground( Any& rValue ) throw()
{
    const SfxItemSet& rFillAttributes = GetPage()->getSdrPageProperties().GetItemSet();

       if(XFILL_NONE == ((const XFillStyleItem&)rFillAttributes.Get(XATTR_FILLSTYLE)).GetValue())
    {
        // no fill set (switched off by XFILL_NONE), clear rValue to represent this
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
        else if( xIA->getCount() == static_cast< sal_Int32 >( GetPage()->GetObjCount() ) )
        {
            GetPage()->SetNavigationOrder(xIA);
            return;
        }
    }
    throw IllegalArgumentException();
}

class SdNavigationOrderAccess : public ::cppu::WeakImplHelper1< XIndexAccess >
{
public:
    SdNavigationOrderAccess( SdrPage* pPage );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

private:
    std::vector< Reference< XShape > > maShapes;
};

SdNavigationOrderAccess::SdNavigationOrderAccess( SdrPage* pPage )
: maShapes( static_cast< sal_uInt32 >( pPage ? pPage->GetObjCount() : 0 ) )
{
    if( pPage )
    {
        sal_uInt32 nIndex;
        const sal_uInt32 nCount = static_cast< sal_uInt32 >( pPage->GetObjCount() );
        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            SdrObject* pObj = pPage->GetObj( nIndex );
            sal_uInt32 nNavPos = pObj->GetNavigationPosition();
            DBG_ASSERT( !maShapes[nNavPos].is(), "sd::SdNavigationOrderAccess::SdNavigationOrderAccess(), duplicate navigation positions from core!" );
            maShapes[nNavPos] = Reference< XShape >( pObj->getUnoShape(), UNO_QUERY );
        }
    }
}

// XIndexAccess
sal_Int32 SAL_CALL SdNavigationOrderAccess::getCount(  ) throw (RuntimeException)
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

Any SAL_CALL SdNavigationOrderAccess::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if( (Index < 0) || (Index > getCount()) )
        throw IndexOutOfBoundsException();

    return Any( maShapes[Index] );
}

// XElementAccess
Type SAL_CALL SdNavigationOrderAccess::getElementType(  ) throw (RuntimeException)
{
    return XShape::static_type();
}

sal_Bool SAL_CALL SdNavigationOrderAccess::hasElements(  ) throw (RuntimeException)
{
    return maShapes.empty() ? sal_False : sal_True;
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

//========================================================================
// class SdMasterPage
//========================================================================

SdMasterPage::SdMasterPage( SdXImpressDocument* pModel, SdPage* pPage ) throw()
: SdGenericDrawPage( pModel, pPage, ImplGetMasterPagePropertySet( pPage ? pPage->GetPageKind() : PK_STANDARD ) )
{
}

SdMasterPage::~SdMasterPage() throw()
{
}

// XInterface
Any SAL_CALL SdMasterPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    uno::Any aAny;

    if( rType == ITYPE( container::XIndexAccess ) )
        aAny <<= Reference< container::XIndexAccess >((presentation::XPresentationPage*)(this));
    else if( rType == ITYPE( container::XElementAccess ) )
        aAny <<=  Reference< container::XElementAccess >((presentation::XPresentationPage*)(this));
    else if( rType == ITYPE( container::XNamed ) )
        aAny <<=  Reference< container::XNamed >(this);
    else if( rType == ITYPE( presentation::XPresentationPage ) &&
             ( mbIsImpressDocument &&
               GetPage()  && GetPage()->GetPageKind() != PK_HANDOUT) )
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
Sequence< uno::Type > SAL_CALL SdMasterPage::getTypes() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( maTypeSequence.getLength() == 0 )
    {
        const PageKind ePageKind = GetPage() ? GetPage()->GetPageKind() : PK_STANDARD;
        sal_Bool bPresPage = mbIsImpressDocument && SvxFmDrawPage::mpPage && ePageKind != PK_HANDOUT;

        // Collect the types of this class.
        ::std::vector<uno::Type> aTypes;
        aTypes.reserve(12);
        aTypes.push_back(ITYPE(drawing::XDrawPage));
        aTypes.push_back(ITYPE(beans::XPropertySet));
        aTypes.push_back(ITYPE(container::XNamed));
        aTypes.push_back(ITYPE(lang::XServiceInfo));
        aTypes.push_back(ITYPE(util::XReplaceable));
        aTypes.push_back(ITYPE(document::XLinkTargetSupplier));
        aTypes.push_back(ITYPE( drawing::XShapeCombiner ));
        aTypes.push_back(ITYPE( drawing::XShapeBinder ));
        aTypes.push_back(ITYPE( office::XAnnotationAccess ));
        aTypes.push_back(ITYPE( beans::XMultiPropertySet ));
        if( bPresPage )
            aTypes.push_back(ITYPE(presentation::XPresentationPage));
        if( bPresPage && ePageKind == PK_STANDARD )
            aTypes.push_back(ITYPE(XAnimationNodeSupplier));

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

namespace
{
    class theSdMasterPageImplementationId : public rtl::Static< UnoTunnelIdInit, theSdMasterPageImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SdMasterPage::getImplementationId() throw(uno::RuntimeException)
{
    return theSdMasterPageImplementationId::get().getSeq();
}

// XServiceInfo
OUString SAL_CALL SdMasterPage::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "SdMasterPage" );
}

Sequence< OUString > SAL_CALL SdMasterPage::getSupportedServiceNames() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.drawing.MasterPage" );

    if( SvxFmDrawPage::mpPage && ((SdPage*)SvxFmDrawPage::mpPage)->GetPageKind() == PK_HANDOUT )
        comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.HandoutMasterPage" );

    return aSeq;
}

sal_Bool SAL_CALL SdMasterPage::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::supportsService( ServiceName );
}

// XElementAccess
sal_Bool SAL_CALL SdMasterPage::hasElements() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if( SvxFmDrawPage::mpPage == NULL )
        return sal_False;

    return SvxFmDrawPage::mpPage->GetObjCount() > 0;
}

uno::Type SAL_CALL SdMasterPage::getElementType()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getElementType();
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPage::getCount()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdGenericDrawPage::getCount();
}

Any SAL_CALL SdMasterPage::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    return SdGenericDrawPage::getByIndex(Index);
}

// intern
void SdMasterPage::setBackground( const Any& rValue )
    throw( lang::IllegalArgumentException )
{
    // we need at least an beans::XPropertySet
    Reference< beans::XPropertySet > xInputSet( rValue, UNO_QUERY );
    if( !xInputSet.is() )
        throw lang::IllegalArgumentException();

    try
    {
        if( GetModel() && mbIsImpressDocument )
        {
            Reference< container::XNameAccess >  xFamilies( GetModel()->getStyleFamilies(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW ) ;
            if( xFamily.is() )
            {
                OUString aStyleName( OUString::createFromAscii(sUNO_PseudoSheet_Background) );

                Reference< beans::XPropertySet >  xStyleSet( xFamily->getByName( aStyleName ), UNO_QUERY_THROW );

                Reference< beans::XPropertySetInfo >  xSetInfo( xInputSet->getPropertySetInfo(), UNO_QUERY_THROW );
                Reference< beans::XPropertyState > xSetStates( xInputSet, UNO_QUERY );

                PropertyEntryVector_t aBackgroundProperties = ImplGetPageBackgroundPropertySet()->getPropertyMap()->getPropertyEntries();
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
                pBack->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
            }
            else
            {
                SdUnoPageBackground* pBackground = new SdUnoPageBackground();

                Reference< beans::XPropertySetInfo > xInputSetInfo( xInputSet->getPropertySetInfo(), UNO_QUERY_THROW );
                Reference< beans::XPropertySet > xDestSet( (beans::XPropertySet*)pBackground );
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

                pBackground->fillItemSet( (SdDrawDocument*)SvxFmDrawPage::mpPage->GetModel(), aSet );
            }

            // if we find the background style, copy the set to the background
            SdDrawDocument* pDoc = (SdDrawDocument*)SvxFmDrawPage::mpPage->GetModel();
            SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                String aLayoutName( static_cast< SdPage* >( SvxFmDrawPage::mpPage )->GetLayoutName() );
                aLayoutName.Erase(aLayoutName.Search(String(RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR)))+4);
                aLayoutName += String(SdResId(STR_LAYOUT_BACKGROUND));
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

void SdMasterPage::getBackground( Any& rValue ) throw()
{
    if( GetModel() ) try
    {
        if( mbIsImpressDocument )
        {
            Reference< container::XNameAccess > xFamilies( GetModel()->getStyleFamilies(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > xFamily( xFamilies->getByName( getName() ), UNO_QUERY_THROW );

            const OUString aStyleName( OUString::createFromAscii(sUNO_PseudoSheet_Background) );
            rValue <<= Reference< beans::XPropertySet >( xFamily->getByName( aStyleName ), UNO_QUERY_THROW );
        }
        else
        {
            SdDrawDocument* pDoc = (SdDrawDocument*)SvxFmDrawPage::mpPage->GetModel();
            SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
            if(pSSPool)
            {
                String aLayoutName( static_cast< SdPage* >(SvxFmDrawPage::mpPage)->GetLayoutName() );
                aLayoutName.Erase( aLayoutName.Search(String(RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR)))+4);
                aLayoutName += String(SdResId(STR_LAYOUT_BACKGROUND));
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

            if(XFILL_NONE == ((const XFillStyleItem&)rFallbackItemSet.Get(XATTR_FILLSTYLE)).GetValue())
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
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetPage()->GetPageKind() != PK_NOTES)
    {
        SdDrawDocument* pDoc = GetModel()->GetDoc();
        sal_Bool bOutDummy;
        String aNewName( rName );

        // Slide Name has to be unique
        if( pDoc && pDoc->GetPageByName( aNewName, bOutDummy ) != SDRPAGE_NOTFOUND )
            return; // throw Exception ?

        GetPage()->SetName( aNewName );

        if( pDoc )
            pDoc->RenameLayoutTemplate( GetPage()->GetLayoutName(), aNewName );

        // fake a mode change to repaint the page tab bar
        ::sd::DrawDocShell* pDocSh = GetModel()->GetDocShell();
        ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : NULL;
        if( pViewSh && pViewSh->ISA(::sd::DrawViewShell ) )
        {
            ::sd::DrawViewShell* pDrawViewSh =
                  static_cast< ::sd::DrawViewShell*>(pViewSh);

            EditMode eMode = pDrawViewSh->GetEditMode();
            if( eMode == EM_MASTERPAGE )
            {
                sal_Bool bLayer = pDrawViewSh->IsLayerModeActive();

                pDrawViewSh->ChangeEditMode( eMode, !bLayer );
                pDrawViewSh->ChangeEditMode( eMode, bLayer );
            }
        }

        GetModel()->SetModified();
    }
}

OUString SAL_CALL SdMasterPage::getName(  )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage)
    {
        String aLayoutName( GetPage()->GetLayoutName() );
        aLayoutName = aLayoutName.Erase(aLayoutName.Search( String( RTL_CONSTASCII_USTRINGPARAM((SD_LT_SEPARATOR)))));

        return aLayoutName;
    }

    return OUString();
}

// XPresentationPage
Reference< drawing::XDrawPage > SAL_CALL SdMasterPage::getNotesPage()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    throwIfDisposed();

    if(SvxFmDrawPage::mpPage && GetModel()->GetDoc() )
    {
        SdPage* pNotesPage = GetModel()->GetDoc()->GetMasterSdPage( (SvxFmDrawPage::mpPage->GetPageNum()-1)>>1, PK_NOTES );
        if( pNotesPage )
        {
            Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return NULL;
}

// XShapes
void SAL_CALL SdMasterPage::add( const Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdMasterPage::remove( const Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
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
                xPage = (::cppu::OWeakObject*)new SdMasterPage( pModel, pPage );
            }
            else
            {
                xPage = (::cppu::OWeakObject*)new SdDrawPage( pModel, pPage );
            }
        }
    }

    return xPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

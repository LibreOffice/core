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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


// HINTIDs must be on top; it is required for the macitem.hxx header
#include "hintids.hxx"
#include "unoevent.hxx"
#include "unoframe.hxx"
#include "unostyle.hxx"
#include "swevent.hxx"
#include "docstyle.hxx"
#include <svx/svxids.hrc>
#include "fmtinfmt.hxx"
#include <svl/macitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::PropertyValue;
using ::cppu::WeakImplHelper2;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


const sal_Char sAPI_ServiceName[] = "com.sun.star.container.XNameReplace";
const sal_Char sAPI_SwFrameEventDescriptor[] = "SwFrameEventDescriptor";
const sal_Char sAPI_SwFrameStyleEventDescriptor[] =
                                    "SwFrameStyleEventDescriptor";
const sal_Char sAPI_SwDetachedEventDescriptor[] = "SwDetachedEventDescriptor";
const sal_Char sAPI_SwHyperlinkEventDescriptor[] =
                                    "SwHyperlinkEventDescriptor";
const sal_Char sAPI_SwAutoTextEventDescriptor[] =
                                    "SwAutoTextEventDescriptor";

//
// tables of allowed events for specific objects
//

const struct SvEventDescription aGraphicEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { SVX_EVENT_IMAGE_LOAD,         "OnLoadDone" },
    { SVX_EVENT_IMAGE_ABORT,        "OnLoadCancel" },
    { SVX_EVENT_IMAGE_ERROR,        "OnLoadError" },
    { 0, NULL }
};

const struct SvEventDescription aFrameEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SW_EVENT_FRM_KEYINPUT_ALPHA,  "OnAlphaCharInput" },
    { SW_EVENT_FRM_KEYINPUT_NOALPHA,    "OnNonAlphaCharInput" },
    { SW_EVENT_FRM_RESIZE,          "OnResize" },
    { SW_EVENT_FRM_MOVE,            "OnMove" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aOLEEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aHyperlinkEvents[] =
{
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { 0, NULL }
};

const struct SvEventDescription aAutotextEvents[] =
{
    { SW_EVENT_START_INS_GLOSSARY,  "OnInsertStart" },
    { SW_EVENT_END_INS_GLOSSARY,    "OnInsertDone" },
    { 0, NULL }
};

const struct SvEventDescription aFrameStyleEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SW_EVENT_FRM_KEYINPUT_ALPHA,  "OnAlphaCharInput" },
    { SW_EVENT_FRM_KEYINPUT_NOALPHA,    "OnNonAlphaCharInput" },
    { SW_EVENT_FRM_RESIZE,          "OnResize" },
    { SW_EVENT_FRM_MOVE,            "OnMove" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { SVX_EVENT_IMAGE_LOAD,         "OnLoadDone" },
    { SVX_EVENT_IMAGE_ABORT,        "OnLoadCancel" },
    { SVX_EVENT_IMAGE_ERROR,        "OnLoadError" },
    { 0, NULL }
};

//
// SwHyperlinkEventDescriptor
//

SwHyperlinkEventDescriptor::SwHyperlinkEventDescriptor() :
    SvDetachedEventDescriptor(aHyperlinkEvents),
    sImplName(RTL_CONSTASCII_USTRINGPARAM(sAPI_SwHyperlinkEventDescriptor))
{
}

SwHyperlinkEventDescriptor::~SwHyperlinkEventDescriptor()
{
}

OUString SwHyperlinkEventDescriptor::getImplementationName(void)
    throw( RuntimeException )
{
    return sImplName;
}

void SwHyperlinkEventDescriptor::copyMacrosFromINetFmt(
    const SwFmtINetFmt& aFmt)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* aMacro = aFmt.GetMacro(nEvent);
        if (NULL != aMacro)
            replaceByName(nEvent, *aMacro);
    }
}

void SwHyperlinkEventDescriptor::copyMacrosIntoINetFmt(
    SwFmtINetFmt& aFmt)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasByName(nEvent))
        {
            SvxMacro aMacro(sEmpty, sEmpty);
            getByName(aMacro, nEvent);
            aFmt.SetMacro(nEvent, aMacro);
        }
    }
}


void SwHyperlinkEventDescriptor::copyMacrosFromNameReplace(
    uno::Reference<
        container::XNameReplace> & xReplace)
{
    // iterate over all names (all names that *we* support)
    Sequence<OUString> aNames = getElementNames();
    sal_Int32 nCount = aNames.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        // copy element for that name
        const OUString& rName = aNames[i];
        if (xReplace->hasByName(rName))
        {
            SvBaseEventDescriptor::replaceByName(rName,
                                                 xReplace->getByName(rName));
        }
    }
}


//
// SwFrameEventDescriptor
//

// use double cast in superclass constructor to avoid ambigous cast
SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextFrame& rFrameRef ) :
        SvEventDescriptor((text::XTextFrame&)rFrameRef, aFrameEvents),
        sSwFrameEventDescriptor(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_SwFrameEventDescriptor)),
        rFrame(rFrameRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextGraphicObject& rGraphicRef ) :
        SvEventDescriptor((text::XTextContent&)rGraphicRef, aGraphicEvents),
        rFrame((SwXFrame&)rGraphicRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextEmbeddedObject& rObjectRef ) :
        SvEventDescriptor((text::XTextContent&)rObjectRef, aOLEEvents),
        rFrame((SwXFrame&)rObjectRef)
{
}

SwFrameEventDescriptor::~SwFrameEventDescriptor()
{
}

void SwFrameEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    rFrame.GetFrmFmt()->SetFmtAttr(rItem);
}

const SvxMacroItem& SwFrameEventDescriptor::getMacroItem()
{
    return (const SvxMacroItem&)rFrame.GetFrmFmt()->GetFmtAttr(RES_FRMMACRO);
}

sal_uInt16 SwFrameEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

OUString SwFrameEventDescriptor::getImplementationName()
    throw( RuntimeException )
{
    return sSwFrameEventDescriptor;
}


//
// SwFrameStyleEventDescriptor
//

SwFrameStyleEventDescriptor::SwFrameStyleEventDescriptor(
    SwXFrameStyle& rStyleRef ) :
        SvEventDescriptor((document::XEventsSupplier&)rStyleRef,
                          aFrameStyleEvents),
        sSwFrameStyleEventDescriptor(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_SwFrameStyleEventDescriptor)),
        rStyle(rStyleRef)
{
}

SwFrameStyleEventDescriptor::~SwFrameStyleEventDescriptor()
{
}

void SwFrameStyleEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    // As I was told, for some entirely unobvious reason getting an
    // item from a style has to look as follows:
    SfxStyleSheetBasePool* pBasePool = rStyle.GetBasePool();
    if (pBasePool)
    {
        SfxStyleSheetBase* pBase = pBasePool->Find(rStyle.GetStyleName());
        if (pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
            SfxItemSet& rStyleSet = xStyle->GetItemSet();
            SfxItemSet aSet(*rStyleSet.GetPool(), RES_FRMMACRO, RES_FRMMACRO);
            aSet.Put(rItem);
            xStyle->SetItemSet(aSet);
        }
    }
}

static const SvxMacroItem aEmptyMacroItem(RES_FRMMACRO);

const SvxMacroItem& SwFrameStyleEventDescriptor::getMacroItem()
{
    // As I was told, for some entirely unobvious reason getting an
    // item from a style has to look as follows:
    SfxStyleSheetBasePool* pBasePool = rStyle.GetBasePool();
    if (pBasePool)
    {
        SfxStyleSheetBase* pBase = pBasePool->Find(rStyle.GetStyleName());
        if (pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase) );
            return (const SvxMacroItem&)xStyle->GetItemSet().Get(RES_FRMMACRO);
        }
        else
            return aEmptyMacroItem;
    }
    else
        return aEmptyMacroItem;
}

OUString SwFrameStyleEventDescriptor::getImplementationName()
    throw( RuntimeException )
{
    return sSwFrameStyleEventDescriptor;
}

sal_uInt16 SwFrameStyleEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

// tables of allowed events for specific objects

const struct SvEventDescription aGraphicEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { SVX_EVENT_IMAGE_LOAD,         "OnLoadDone" },
    { SVX_EVENT_IMAGE_ABORT,        "OnLoadCancel" },
    { SVX_EVENT_IMAGE_ERROR,        "OnLoadError" },
    { 0, nullptr }
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
    { 0, nullptr }
};

const struct SvEventDescription aOLEEvents[] =
{
    { SW_EVENT_OBJECT_SELECT,       "OnSelect" },
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { 0, nullptr }
};

const struct SvEventDescription aHyperlinkEvents[] =
{
    { SFX_EVENT_MOUSEOVER_OBJECT,   "OnMouseOver" },
    { SFX_EVENT_MOUSECLICK_OBJECT,  "OnClick" },
    { SFX_EVENT_MOUSEOUT_OBJECT,    "OnMouseOut" },
    { 0, nullptr }
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
    { 0, nullptr }
};

SwHyperlinkEventDescriptor::SwHyperlinkEventDescriptor() :
    SvDetachedEventDescriptor(aHyperlinkEvents),
    sImplName("SwHyperlinkEventDescriptor")
{
}

SwHyperlinkEventDescriptor::~SwHyperlinkEventDescriptor()
{
}

OUString SwHyperlinkEventDescriptor::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return sImplName;
}

void SwHyperlinkEventDescriptor::copyMacrosFromINetFormat(
    const SwFormatINetFormat& aFormat)
{
    for(sal_uInt16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; ++i)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* aMacro = aFormat.GetMacro(nEvent);
        if (nullptr != aMacro)
            replaceByName(nEvent, *aMacro);
    }
}

void SwHyperlinkEventDescriptor::copyMacrosIntoINetFormat(
    SwFormatINetFormat& aFormat)
{
    for(sal_uInt16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; ++i)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasById(nEvent))
        {
            SvxMacro aMacro("", "");
            getByName(aMacro, nEvent);
            aFormat.SetMacro(nEvent, aMacro);
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

// use double cast in superclass constructor to avoid ambigous cast
SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextFrame& rFrameRef ) :
        SvEventDescriptor((text::XTextFrame&)rFrameRef, aFrameEvents),
        sSwFrameEventDescriptor("SwFrameEventDescriptor"),
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
    rFrame.GetFrameFormat()->SetFormatAttr(rItem);
}

const SvxMacroItem& SwFrameEventDescriptor::getMacroItem()
{
    return static_cast<const SvxMacroItem&>(rFrame.GetFrameFormat()->GetFormatAttr(RES_FRMMACRO));
}

sal_uInt16 SwFrameEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

OUString SwFrameEventDescriptor::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return sSwFrameEventDescriptor;
}

SwFrameStyleEventDescriptor::SwFrameStyleEventDescriptor(
    SwXFrameStyle& rStyleRef ) :
        SvEventDescriptor((document::XEventsSupplier&)rStyleRef,
                          aFrameStyleEvents),
        sSwFrameStyleEventDescriptor("SwFrameStyleEventDescriptor"),
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
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
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
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase)) );
            return static_cast<const SvxMacroItem&>(xStyle->GetItemSet().Get(RES_FRMMACRO));
        }
        else
            return aEmptyMacroItem;
    }
    else
        return aEmptyMacroItem;
}

OUString SwFrameStyleEventDescriptor::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return sSwFrameStyleEventDescriptor;
}

sal_uInt16 SwFrameStyleEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

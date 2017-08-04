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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::XNameReplace;

// tables of allowed events for specific objects

const struct SvEventDescription aGraphicEvents[] =
{
    { SvMacroItemId::SwObjectSelect,    "OnSelect" },
    { SvMacroItemId::OnMouseOver,       "OnMouseOver" },
    { SvMacroItemId::OnClick,           "OnClick" },
    { SvMacroItemId::OnMouseOut,        "OnMouseOut" },
    { SvMacroItemId::OnImageLoadDone,   "OnLoadDone" },
    { SvMacroItemId::OnImageLoadCancel, "OnLoadCancel" },
    { SvMacroItemId::OnImageLoadError,  "OnLoadError" },
    { SvMacroItemId::NONE, nullptr }
};

const struct SvEventDescription aFrameEvents[] =
{
    { SvMacroItemId::SwObjectSelect,       "OnSelect" },
    { SvMacroItemId::SwFrmKeyInputAlpha,   "OnAlphaCharInput" },
    { SvMacroItemId::SwFrmKeyInputNoAlpha, "OnNonAlphaCharInput" },
    { SvMacroItemId::SwFrmResize,          "OnResize" },
    { SvMacroItemId::SwFrmMove,            "OnMove" },
    { SvMacroItemId::OnMouseOver,          "OnMouseOver" },
    { SvMacroItemId::OnClick,              "OnClick" },
    { SvMacroItemId::OnMouseOut,           "OnMouseOut" },
    { SvMacroItemId::NONE, nullptr }
};

const struct SvEventDescription aOLEEvents[] =
{
    { SvMacroItemId::SwObjectSelect, "OnSelect" },
    { SvMacroItemId::OnMouseOver,    "OnMouseOver" },
    { SvMacroItemId::OnClick,        "OnClick" },
    { SvMacroItemId::OnMouseOut,     "OnMouseOut" },
    { SvMacroItemId::NONE, nullptr }
};

const struct SvEventDescription aHyperlinkEvents[] =
{
    { SvMacroItemId::OnMouseOver, "OnMouseOver" },
    { SvMacroItemId::OnClick,     "OnClick" },
    { SvMacroItemId::OnMouseOut,  "OnMouseOut" },
    { SvMacroItemId::NONE, nullptr }
};

const struct SvEventDescription aFrameStyleEvents[] =
{
    { SvMacroItemId::SwObjectSelect,       "OnSelect" },
    { SvMacroItemId::SwFrmKeyInputAlpha,   "OnAlphaCharInput" },
    { SvMacroItemId::SwFrmKeyInputNoAlpha, "OnNonAlphaCharInput" },
    { SvMacroItemId::SwFrmResize,          "OnResize" },
    { SvMacroItemId::SwFrmMove,            "OnMove" },
    { SvMacroItemId::OnMouseOver,          "OnMouseOver" },
    { SvMacroItemId::OnClick,              "OnClick" },
    { SvMacroItemId::OnMouseOut,           "OnMouseOut" },
    { SvMacroItemId::OnImageLoadDone,      "OnLoadDone" },
    { SvMacroItemId::OnImageLoadCancel,    "OnLoadCancel" },
    { SvMacroItemId::OnImageLoadError,     "OnLoadError" },
    { SvMacroItemId::NONE, nullptr }
};

SwHyperlinkEventDescriptor::SwHyperlinkEventDescriptor() :
    SvDetachedEventDescriptor(aHyperlinkEvents)
{
}

SwHyperlinkEventDescriptor::~SwHyperlinkEventDescriptor()
{
}

OUString SwHyperlinkEventDescriptor::getImplementationName()
{
    return OUString("SwHyperlinkEventDescriptor");
}

void SwHyperlinkEventDescriptor::copyMacrosFromINetFormat(
    const SwFormatINetFormat& aFormat)
{
    for(sal_uInt16 i = 0; mpSupportedMacroItems[i].mnEvent != SvMacroItemId::NONE; ++i)
    {
        const SvMacroItemId nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* aMacro = aFormat.GetMacro(nEvent);
        if (nullptr != aMacro)
            replaceByName(nEvent, *aMacro);
    }
}

void SwHyperlinkEventDescriptor::copyMacrosIntoINetFormat(
    SwFormatINetFormat& aFormat)
{
    for(sal_uInt16 i = 0; mpSupportedMacroItems[i].mnEvent != SvMacroItemId::NONE; ++i)
    {
        const SvMacroItemId nEvent = mpSupportedMacroItems[i].mnEvent;
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
        container::XNameReplace> const & xReplace)
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

// use double cast in superclass constructor to avoid ambiguous cast
SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextFrame& rFrameRef ) :
        SvEventDescriptor(static_cast<text::XTextFrame&>(rFrameRef), aFrameEvents),
        sSwFrameEventDescriptor("SwFrameEventDescriptor"),
        rFrame(rFrameRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextGraphicObject& rGraphicRef ) :
        SvEventDescriptor(static_cast<text::XTextContent&>(rGraphicRef), aGraphicEvents),
        rFrame(static_cast<SwXFrame&>(rGraphicRef))
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextEmbeddedObject& rObjectRef ) :
        SvEventDescriptor(static_cast<text::XTextContent&>(rObjectRef), aOLEEvents),
        rFrame(static_cast<SwXFrame&>(rObjectRef))
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
{
    return sSwFrameEventDescriptor;
}

SwFrameStyleEventDescriptor::SwFrameStyleEventDescriptor(
    sw::ICoreFrameStyle& rStyle ) :
        SvEventDescriptor(rStyle.GetEventsSupplier(),
                          aFrameStyleEvents),
        sSwFrameStyleEventDescriptor("SwFrameStyleEventDescriptor"),
        m_rStyle(rStyle)
{
}

SwFrameStyleEventDescriptor::~SwFrameStyleEventDescriptor()
{
}

void SwFrameStyleEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    m_rStyle.SetItem(RES_FRMMACRO, rItem);
}

static const SvxMacroItem aEmptyMacroItem(RES_FRMMACRO);

const SvxMacroItem& SwFrameStyleEventDescriptor::getMacroItem()
{
    const SfxPoolItem* pItem(m_rStyle.GetItem(RES_FRMMACRO));
    return pItem ? static_cast<const SvxMacroItem&>(*pItem) : aEmptyMacroItem;
}

OUString SwFrameStyleEventDescriptor::getImplementationName()
{
    return sSwFrameStyleEventDescriptor;
}

sal_uInt16 SwFrameStyleEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

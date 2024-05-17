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
#include <hintids.hxx>
#include <unoevent.hxx>
#include <unoframe.hxx>
#include <unostyle.hxx>
#include <fmtinfmt.hxx>
#include <svl/macitem.hxx>
#include <sfx2/event.hxx>

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
    return u"SwHyperlinkEventDescriptor"_ustr;
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
            SvxMacro aMacro(u""_ustr, u""_ustr);
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
    const Sequence<OUString> aNames = getElementNames();
    for(const OUString& rName : aNames)
    {
        // copy element for that name
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
        m_rFrame(rFrameRef)
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextGraphicObject& rGraphicRef ) :
        SvEventDescriptor(static_cast<text::XTextContent&>(rGraphicRef), aGraphicEvents),
        m_rFrame(static_cast<SwXFrame&>(rGraphicRef))
{
}

SwFrameEventDescriptor::SwFrameEventDescriptor(
    SwXTextEmbeddedObject& rObjectRef ) :
        SvEventDescriptor(static_cast<text::XTextContent&>(rObjectRef), aOLEEvents),
        m_rFrame(static_cast<SwXFrame&>(rObjectRef))
{
}

SwFrameEventDescriptor::~SwFrameEventDescriptor()
{
}

void SwFrameEventDescriptor::setMacroItem(const SvxMacroItem& rItem)
{
    m_rFrame.GetFrameFormat()->SetFormatAttr(rItem);
}

const SvxMacroItem& SwFrameEventDescriptor::getMacroItem()
{
    return m_rFrame.GetFrameFormat()->GetFormatAttr(RES_FRMMACRO);
}

sal_uInt16 SwFrameEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

OUString SwFrameEventDescriptor::getImplementationName()
{
    return u"SwFrameEventDescriptor"_ustr;
}

SwFrameStyleEventDescriptor::SwFrameStyleEventDescriptor(
    sw::ICoreFrameStyle& rStyle ) :
        SvEventDescriptor(rStyle.GetEventsSupplier(),
                          aFrameStyleEvents),
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

const SvxMacroItem& SwFrameStyleEventDescriptor::getMacroItem()
{
    const SfxPoolItem* pItem(m_rStyle.GetItem(RES_FRMMACRO));
    return pItem ?
        static_cast<const SvxMacroItem&>(*pItem) :
        *GetDfltAttr(RES_FRMMACRO);
}

OUString SwFrameStyleEventDescriptor::getImplementationName()
{
    return u"SwFrameStyleEventDescriptor"_ustr;
}

sal_uInt16 SwFrameStyleEventDescriptor::getMacroItemWhich() const
{
    return RES_FRMMACRO;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

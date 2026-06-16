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

#include "htmlfly.hxx"
#include <svtools/htmlcfg.hxx>

constexpr sal_uInt16 MAX_FRMTYPES = HTML_FRMTYPE_END;
constexpr sal_uInt16 MAX_BROWSERS = HTML_CFG_MAX + 1;

constexpr AllHtmlFlags aHTMLOutFramePageFlyTable[][MAX_BROWSERS] =
{
    {
        // text frame with table
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE }       // Netscape 4!
    },
    {
        // text frame with table and headline
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // column frame
        { HtmlOut::GraphicFrame, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::MultiCol,     HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::MultiCol,     HtmlPosition::Prefix, HtmlContainerFlags::Div }        // Netscape 4
    },
    {
        // empty text frame
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other text frame
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Div, HtmlPosition::Prefix, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // graphic node
        { HtmlOut::GraphicNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicNode, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // plug-in
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // applet
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // floating frame
        { HtmlOut::OleNode,    HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode,    HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // other OLE objects
        { HtmlOut::OleGraphic, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleGraphic, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // marquee
        { HtmlOut::AMarquee,     HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::AMarquee,     HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // controls
        { HtmlOut::Control, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Control, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        // Netscape disables FROM at controls in absolute position span.
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other character objects
        { HtmlOut::GraphicFrame, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Prefix, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Prefix, HtmlContainerFlags::Span }       // Netscape 4
    }
};

const AllHtmlFlags & getHTMLOutFramePageFlyTable(SwHTMLFrameType eFrameType, sal_uInt16 nExportMode)
{
    static_assert(std::size(aHTMLOutFramePageFlyTable) == MAX_FRMTYPES);
    assert(eFrameType < HTML_FRMTYPE_END);
    assert(nExportMode <= HTML_CFG_MAX);

    return aHTMLOutFramePageFlyTable[eFrameType][nExportMode];
}

constexpr AllHtmlFlags aHTMLOutFrameParaFrameTable[][MAX_BROWSERS] =
{
    {
        // text frame with table
        { HtmlOut::TableNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::TableNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::TableNode, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // text frame with table and headline
        { HtmlOut::Div,       HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div,       HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::TableNode, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // column frame
        { HtmlOut::GraphicFrame, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::MultiCol,     HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::MultiCol,     HtmlPosition::Before, HtmlContainerFlags::Div }        // Netscape 4
    },
    {
        // empty text frame
        { HtmlOut::Div,    HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Spacer, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Spacer, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other text frame
        { HtmlOut::Div, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Div, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Div, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // graphic node
        { HtmlOut::GraphicNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicNode, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // plug-in
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // applet
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // floating frame
        { HtmlOut::OleNode,    HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode,    HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other OLE objects
        { HtmlOut::OleGraphic, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleGraphic, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // marquee (for Netscape 4 in container, so that
        // the marquee appears at the right spot)
        { HtmlOut::AMarquee,     HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::AMarquee,     HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // controls
        { HtmlOut::Control, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Control, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        // here you could make container out if it (import is missing)
        { HtmlOut::Control, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other character objects
        { HtmlOut::GraphicFrame, HtmlPosition::Before, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Before, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Before, HtmlContainerFlags::NONE }       // Netscape 4
    }
};

const AllHtmlFlags & getHTMLOutFrameParaFrameTable(SwHTMLFrameType eFrameType, sal_uInt16 nExportMode)
{
    static_assert(std::size(aHTMLOutFrameParaFrameTable) == MAX_FRMTYPES);
    assert(eFrameType < HTML_FRMTYPE_END);
    assert(nExportMode <= HTML_CFG_MAX);

    return aHTMLOutFrameParaFrameTable[eFrameType][nExportMode];
}

constexpr AllHtmlFlags aHTMLOutFrameParaPrtAreaTable[][MAX_BROWSERS] =
{
    {
        // text frame with table
        { HtmlOut::TableNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::TableNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::TableNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // text frame with table and headline
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // column frame
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // empty text frame
        { HtmlOut::Span,   HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Spacer, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Spacer, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other text frame
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // graphic node
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // plug-in
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // applet
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // floating frame
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other OLE objects
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // marquee
        { HtmlOut::AMarquee,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::AMarquee,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // controls
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        // here you could make container out if it (import is missing)
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other character objects
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    }
};

const AllHtmlFlags & getHTMLOutFrameParaPrtAreaTable(SwHTMLFrameType eFrameType, sal_uInt16 nExportMode)
{
    static_assert(std::size(aHTMLOutFrameParaPrtAreaTable) == MAX_FRMTYPES);
    assert(eFrameType < HTML_FRMTYPE_END);
    assert(nExportMode <= HTML_CFG_MAX);

    return aHTMLOutFrameParaPrtAreaTable[eFrameType][nExportMode];
}

constexpr AllHtmlFlags aHTMLOutFrameParaOtherTable[][MAX_BROWSERS] =
{
    {
        // text frame with table
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // text frame with table and headline
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // column frame
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // empty text frame
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other text frame
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Span, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // graphic node
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // plug-in
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // applet
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // floating frame
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // other OLE objects
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // marquee
        { HtmlOut::AMarquee,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::AMarquee,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    },
    {
        // controls
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        // Netscape disables FROM at controls in absolute position span.
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other character objects
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::Span }       // Netscape 4
    }
};

const AllHtmlFlags & getHTMLOutFrameParaOtherTable(SwHTMLFrameType eFrameType, sal_uInt16 nExportMode)
{
    static_assert(std::size(aHTMLOutFrameParaOtherTable) == MAX_FRMTYPES);
    assert(eFrameType < HTML_FRMTYPE_END);
    assert(nExportMode <= HTML_CFG_MAX);

    return aHTMLOutFrameParaOtherTable[eFrameType][nExportMode];
}

constexpr AllHtmlFlags aHTMLOutFrameAsCharTable[][MAX_BROWSERS] =
{
    {
        // text frame with table
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // text frame with table and headline
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // column frame
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::MultiCol,     HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // empty text frame
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Spacer,       HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Spacer,       HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other text frame
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // graphic node
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // plug-in
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // applet
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleNode, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // floating frame
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleNode,    HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other OLE objects
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::OleGraphic, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // marquee (can always exported as marquee, because
        // the content shows up at the right spot
        { HtmlOut::Marquee, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Marquee, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Marquee, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // controls
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::Control, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    },
    {
        // other character objects
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // IE 4
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE },      // SW
        { HtmlOut::GraphicFrame, HtmlPosition::Inside, HtmlContainerFlags::NONE }       // Netscape 4
    }
};

const AllHtmlFlags & getHTMLOutFrameAsCharTable(SwHTMLFrameType eFrameType, sal_uInt16 nExportMode)
{
    static_assert(std::size(aHTMLOutFrameAsCharTable) == MAX_FRMTYPES);
    assert(eFrameType < HTML_FRMTYPE_END);
    assert(nExportMode <= HTML_CFG_MAX);

    return aHTMLOutFrameAsCharTable[eFrameType][nExportMode];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

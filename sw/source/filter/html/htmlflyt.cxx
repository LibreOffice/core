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

#define TE(t,p,c) { HtmlOut::t, HtmlPosition::p, HtmlContainerFlags::c }

AllHtmlFlags aHTMLOutFramePageFlyTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // text frame with table
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(Div,       Prefix, NONE),      // IE 4
        TE(Div,       Prefix, NONE),      // SW
        TE(Div,       Prefix, NONE)       // Netscape 4!
    },
    {
        // text frame with table and headline
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(Div,       Prefix, NONE),      // IE 4
        TE(Div,       Prefix, NONE),      // SW
        TE(Div,       Prefix, NONE)       // Netscape 4
    },
    {
        // column frame
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Prefix, NONE),      // IE 4
        TE(MultiCol,      Prefix, NONE),      // SW
        TE(MultiCol,      Prefix, Div)        // Netscape 4
    },
    {
        // empty text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Div,          Prefix, NONE),      // IE 4
        TE(Div,          Prefix, NONE),      // SW
        TE(Div,          Prefix, NONE)       // Netscape 4
    },
    {
        // other text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Div,          Prefix, NONE),      // IE 4
        TE(Div,          Prefix, NONE),      // SW
        TE(Div,          Prefix, NONE)       // Netscape 4
    },
    {
        // graphic node
        TE(GraphicNode, Inside, NONE),      // HTML 3.2
        TE(GraphicNode, Prefix, NONE),      // IE 4
        TE(GraphicNode, Prefix, NONE),      // SW
        TE(GraphicNode, Prefix, Span)       // Netscape 4
    },
    {
        // plug-in
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Prefix, NONE),      // IE 4
        TE(OleNode, Prefix, NONE),      // SW
        TE(OleNode, Prefix, Span)       // Netscape 4
    },
    {
        // applet
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Prefix, NONE),      // IE 4
        TE(OleNode, Prefix, NONE),      // SW
        TE(OleNode, Prefix, Span)       // Netscape 4
    },
    {
        // floating frame
        TE(OleGraphic, Inside, NONE),      // HTML 3.2
        TE(OleNode,    Prefix, NONE),      // IE 4
        TE(OleNode,    Prefix, NONE),      // SW
        TE(OleGraphic, Prefix, Span)       // Netscape 4
    },
    {
        // other OLE objects
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleGraphic,  Prefix, NONE),      // IE 4
        TE(OleGraphic,  Prefix, NONE),      // SW
        TE(OleGraphic,  Prefix, Span)       // Netscape 4
    },
    {
        // marquee
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(AMarquee,     Prefix, NONE),      // IE 4
        TE(AMarquee,     Prefix, NONE),      // SW
        TE(GraphicFrame, Prefix, Span)       // Netscape 4
    },
    {
        // controls
        TE(Control, Inside, NONE),      // HTML 3.2
        TE(Control, Prefix, NONE),      // IE 4
        TE(Control, Prefix, NONE),      // SW
        // Netscape disables FROM at controls in absolute position span.
        TE(Control, Inside, NONE)       // Netscape 4
    },
    {
        // other character objects
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Prefix, NONE),      // IE 4
        TE(GraphicFrame,  Prefix, NONE),      // SW
        TE(GraphicFrame,  Prefix, Span)       // Netscape 4
    }
};

AllHtmlFlags aHTMLOutFrameParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // text frame with table
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(TableNode, Before, NONE),      // IE 4
        TE(TableNode, Before, NONE),      // SW
        TE(TableNode, Before, NONE)       // Netscape 4
    },
    {
        // text frame with table and headline
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(Div,       Before, NONE),      // IE 4
        TE(Div,       Before, NONE),      // SW
        TE(TableNode, Before, NONE)       // Netscape 4
    },
    {
        // column frame
        TE(GraphicFrame, Before, NONE),      // HTML 3.2
        TE(GraphicFrame, Before, NONE),      // IE 4
        TE(MultiCol,     Before, NONE),      // SW
        TE(MultiCol,     Before, Div)        // Netscape 4
    },
    {
        // empty text frame
        TE(GraphicFrame,  Before, NONE),      // HTML 3.2
        TE(Div,           Before, NONE),      // IE 4
        TE(Spacer,        Before, NONE),      // SW
        TE(Spacer,        Before, NONE)       // Netscape 4
    },
    {
        // other text frame
        TE(GraphicFrame, Before, NONE),      // HTML 3.2
        TE(Div,          Before, NONE),      // IE 4
        TE(Div,          Before, NONE),      // SW
        TE(Div,          Before, NONE)       // Netscape 4
    },
    {
        // graphic node
        TE(GraphicNode, Before, NONE),      // HTML 3.2
        TE(GraphicNode, Before, NONE),      // IE 4
        TE(GraphicNode, Before, NONE),      // SW
        TE(GraphicNode, Before, NONE)       // Netscape 4
    },
    {
        // plug-in
        TE(OleNode, Before, NONE),      // HTML 3.2
        TE(OleNode, Before, NONE),      // IE 4
        TE(OleNode, Before, NONE),      // SW
        TE(OleNode, Before, NONE)       // Netscape 4
    },
    {
        // applet
        TE(OleNode, Before, NONE),      // HTML 3.2
        TE(OleNode, Before, NONE),      // IE 4
        TE(OleNode, Before, NONE),      // SW
        TE(OleNode, Before, NONE)       // Netscape 4
    },
    {
        // floating frame
        TE(OleGraphic,  Before, NONE),      // HTML 3.2
        TE(OleNode,     Before, NONE),      // IE 4
        TE(OleNode,     Before, NONE),      // SW
        TE(OleGraphic,  Before, NONE)       // Netscape 4
    },
    {
        // other OLE objects
        TE(OleGraphic,  Before, NONE),      // HTML 3.2
        TE(OleGraphic,  Before, NONE),      // IE 4
        TE(OleGraphic,  Before, NONE),      // SW
        TE(OleGraphic,  Before, NONE)       // Netscape 4
    },
    {
        // marquee (for Netscape 4 in container, so that
        // the marquee appears at the right spot)
        TE(GraphicFrame, Before, NONE),      // HTML 3.2
        TE(AMarquee,     Before, NONE),      // IE 4
        TE(AMarquee,     Before, NONE),      // SW
        TE(GraphicFrame, Before, NONE)       // Netscape 4
    },
    {
        // controls
        TE(Control, Inside, NONE),      // HTML 3.2
        TE(Control, Before, NONE),      // IE 4
        TE(Control, Before, NONE),      // SW
        // here you could make container out if it (import is missing)
        TE(Control, Before, NONE)       // Netscape 4
    },
    {
        // other character objects
        TE(GraphicFrame,  Before, NONE),      // HTML 3.2
        TE(GraphicFrame,  Before, NONE),      // IE 4
        TE(GraphicFrame,  Before, NONE),      // SW
        TE(GraphicFrame,  Before, NONE)       // Netscape 4
    }
};

AllHtmlFlags aHTMLOutFrameParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // text frame with table
        TE(TableNode, Inside, NONE),      // HTML 3.2
        TE(TableNode, Inside, NONE),      // IE 4
        TE(TableNode, Inside, NONE),      // SW
        TE(TableNode, Inside, NONE)       // Netscape 4
    },
    {
        // text frame with table and headline
        TE(TableNode, Inside, NONE),      // HTML 3.2
        TE(Span,      Inside, NONE),      // IE 4
        TE(Span,      Inside, NONE),      // SW
        TE(Span,      Inside, NONE)       // Netscape 4
    },
    {
        // column frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(GraphicFrame, Inside, NONE),      // IE 4
        TE(MultiCol,     Inside, NONE),      // SW
        TE(MultiCol,     Inside, Span)       // Netscape 4
    },
    {
        // empty text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Span,         Inside, NONE),      // IE 4
        TE(Spacer,       Inside, NONE),      // SW
        TE(Spacer,       Inside, NONE)       // Netscape 4
    },
    {
        // other text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Span,         Inside, NONE),      // IE 4
        TE(Span,         Inside, NONE),      // SW
        TE(Span,         Inside, NONE)       // Netscape 4
    },
    {
        // graphic node
        TE(GraphicNode, Inside, NONE),      // HTML 3.2
        TE(GraphicNode, Inside, NONE),      // IE 4
        TE(GraphicNode, Inside, NONE),      // SW
        TE(GraphicNode, Inside, NONE)       // Netscape 4
    },
    {
        // plug-in
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Inside, NONE),      // IE 4
        TE(OleNode, Inside, NONE),      // SW
        TE(OleNode, Inside, NONE)       // Netscape 4
    },
    {
        // applet
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Inside, NONE),      // IE 4
        TE(OleNode, Inside, NONE),      // SW
        TE(OleNode, Inside, NONE)       // Netscape 4
    },
    {
        // floating frame
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleNode,     Inside, NONE),      // IE 4
        TE(OleNode,     Inside, NONE),      // SW
        TE(OleGraphic,  Inside, NONE)       // Netscape 4
    },
    {
        // other OLE objects
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleGraphic,  Inside, NONE),      // IE 4
        TE(OleGraphic,  Inside, NONE),      // SW
        TE(OleGraphic,  Inside, NONE)       // Netscape 4
    },
    {
        // marquee
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(AMarquee,     Inside, NONE),      // IE 4
        TE(AMarquee,     Inside, NONE),      // SW
        TE(GraphicFrame, Inside, NONE)       // Netscape 4
    },
    {
        // controls
        TE(Control, Inside, NONE),      // HTML 3.2
        TE(Control, Inside, NONE),      // IE 4
        TE(Control, Inside, NONE),      // SW
        // here you could make container out if it (import is missing)
        TE(Control, Inside, NONE)       // Netscape 4
    },
    {
        // other character objects
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, NONE)       // Netscape 4
    }
};

AllHtmlFlags aHTMLOutFrameParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // text frame with table
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(Span,      Inside, NONE),      // IE 4
        TE(Span,      Inside, NONE),      // SW
        TE(Span,      Inside, NONE)       // Netscape 4
    },
    {
        // text frame with table and headline
        TE(TableNode, Before, NONE),      // HTML 3.2
        TE(Span,      Inside, NONE),      // IE 4
        TE(Span,      Inside, NONE),      // SW
        TE(Span,      Inside, NONE)       // Netscape 4
    },
    {
        // column frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(GraphicFrame, Inside, NONE),      // IE 4
        TE(MultiCol,     Inside, NONE),      // SW
        TE(MultiCol,     Inside, Span)       // Netscape 4
    },
    {
        // empty text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Span,         Inside, NONE),      // IE 4
        TE(Span,         Inside, NONE),      // SW
        TE(Span,         Inside, NONE)       // Netscape 4
    },
    {
        // other text frame
        TE(GraphicFrame, Inside, NONE),      // HTML 3.2
        TE(Span,         Inside, NONE),      // IE 4
        TE(Span,         Inside, NONE),      // SW
        TE(Span,         Inside, NONE)       // Netscape 4
    },
    {
        // graphic node
        TE(GraphicNode, Inside, NONE),      // HTML 3.2
        TE(GraphicNode, Inside, NONE),      // IE 4
        TE(GraphicNode, Inside, NONE),      // SW
        TE(GraphicNode, Inside, Span)       // Netscape 4
    },
    {
        // plug-in
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Inside, NONE),      // IE 4
        TE(OleNode, Inside, NONE),      // SW
        TE(OleNode, Inside, Span)       // Netscape 4
    },
    {
        // applet
        TE(OleNode, Inside, NONE),      // HTML 3.2
        TE(OleNode, Inside, NONE),      // IE 4
        TE(OleNode, Inside, NONE),      // SW
        TE(OleNode, Inside, Span)       // Netscape 4
    },
    {
        // floating frame
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleNode,     Inside, NONE),      // IE 4
        TE(OleNode,     Inside, NONE),      // SW
        TE(OleGraphic,  Inside, Span)       // Netscape 4
    },
    {
        // other OLE objects
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleGraphic,  Inside, NONE),      // IE 4
        TE(OleGraphic,  Inside, NONE),      // SW
        TE(OleGraphic,  Inside, Span)       // Netscape 4
    },
    {
        // marquee
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(AMarquee,      Inside, NONE),      // IE 4
        TE(AMarquee,      Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, Span)       // Netscape 4
    },
    {
        // controls
        TE(Control, Inside, NONE),      // HTML 3.2
        TE(Control, Inside, NONE),      // IE 4
        TE(Control, Inside, NONE),      // SW
        // Netscape disables FROM at controls in absolute position span.
        TE(Control, Inside, NONE)       // Netscape 4
    },
    {
        // other character objects
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, Span)       // Netscape 4
    }
};

AllHtmlFlags aHTMLOutFrameAsCharTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // text frame with table
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, NONE)       // Netscape 4
    },
    {
        // text frame with table and headline
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, NONE)       // Netscape 4
    },
    {
        // column frame
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(MultiCol,      Inside, NONE),      // SW
        TE(MultiCol,      Inside, NONE)       // Netscape 4
    },
    {
        // empty text frame
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(Spacer,        Inside, NONE),      // SW
        TE(Spacer,        Inside, NONE)       // Netscape 4
    },
    {
        // other text frame
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, NONE)       // Netscape 4
    },
    {
        // graphic node
        TE(GraphicNode, Inside, NONE),      // HTML 3.2
        TE(GraphicNode, Inside, NONE),      // IE 4
        TE(GraphicNode, Inside, NONE),      // SW
        TE(GraphicNode, Inside, NONE)       // Netscape 4
    },
    {
        // plug-in
        TE(OleNode,     Inside, NONE),      // HTML 3.2
        TE(OleNode,     Inside, NONE),      // IE 4
        TE(OleNode,     Inside, NONE),      // SW
        TE(OleNode,     Inside, NONE)       // Netscape 4
    },
    {
        // applet
        TE(OleNode,     Inside, NONE),      // HTML 3.2
        TE(OleNode,     Inside, NONE),      // IE 4
        TE(OleNode,     Inside, NONE),      // SW
        TE(OleNode,     Inside, NONE)       // Netscape 4
    },
    {
        // floating frame
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleNode,     Inside, NONE),      // IE 4
        TE(OleNode,     Inside, NONE),      // SW
        TE(OleGraphic,  Inside, NONE)       // Netscape 4
    },
    {
        // other OLE objects
        TE(OleGraphic,  Inside, NONE),      // HTML 3.2
        TE(OleGraphic,  Inside, NONE),      // IE 4
        TE(OleGraphic,  Inside, NONE),      // SW
        TE(OleGraphic,  Inside, NONE)       // Netscape 4
    },
    {
        // marquee (can always exported as marquee, because
        // the content shows up at the right spot
        TE(Marquee, Inside, NONE),      // HTML 3.2
        TE(Marquee, Inside, NONE),      // IE 4
        TE(Marquee, Inside, NONE),      // SW
        TE(Marquee, Inside, NONE)       // Netscape 4
    },
    {
        // controls
        TE(Control, Inside, NONE),      // HTML 3.2
        TE(Control, Inside, NONE),      // IE 4
        TE(Control, Inside, NONE),      // SW
        TE(Control, Inside, NONE)       // Netscape 4
    },
    {
        // other character objects
        TE(GraphicFrame,  Inside, NONE),      // HTML 3.2
        TE(GraphicFrame,  Inside, NONE),      // IE 4
        TE(GraphicFrame,  Inside, NONE),      // SW
        TE(GraphicFrame,  Inside, NONE)       // Netscape 4
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#define TE(t,p,c) (sal_uInt8)( HTML_OUT_##t | HTML_POS_##p | HTML_CNTNR_##c )

sal_uInt8 aHTMLOutFramePageFlyTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4!
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  PREFIX, NONE),      // IE 4
        TE(MULTICOL,PREFIX, NONE),      // SW
        TE(MULTICOL,PREFIX, DIV)        // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, PREFIX, NONE),      // IE 4
        TE(GRFNODE, PREFIX, NONE),      // SW
        TE(GRFNODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLENODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLENODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLEGRF,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  PREFIX, NONE),      // IE 4
        TE(OLEGRF,  PREFIX, NONE),      // SW
        TE(OLEGRF,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,PREFIX, NONE),      // IE 4
        TE(AMARQUEE,PREFIX, NONE),      // SW
        TE(GRFFRM,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, PREFIX, NONE),      // IE 4
        TE(CONTROL, PREFIX, NONE),      // SW
        // Netscape schaltet FORM bei Controls in abs.-pos. SPAN aus.
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  PREFIX, NONE),      // IE 4
        TE(GRFFRM,  PREFIX, NONE),      // SW
        TE(GRFFRM,  PREFIX, SPAN)       // Netscape 4
    }
};

sal_uInt8 aHTMLOutFrameParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(TBLNODE, BEFORE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // SW
        TE(TBLNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(DIV,     BEFORE, NONE),      // SW
        TE(TBLNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(GRFFRM,  BEFORE, NONE),      // IE 4
        TE(MULTICOL,BEFORE, NONE),      // SW
        TE(MULTICOL,BEFORE, DIV)        // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(SPACER,  BEFORE, NONE),      // SW
        TE(SPACER,  BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(DIV,     BEFORE, NONE),      // SW
        TE(DIV,     BEFORE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, BEFORE, NONE),      // HTML 3.2
        TE(GRFNODE, BEFORE, NONE),      // IE 4
        TE(GRFNODE, BEFORE, NONE),      // SW
        TE(GRFNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLENODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLENODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLEGRF,  BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  BEFORE, NONE),      // HTML 3.2
        TE(OLEGRF,  BEFORE, NONE),      // IE 4
        TE(OLEGRF,  BEFORE, NONE),      // SW
        TE(OLEGRF,  BEFORE, NONE)       // Netscape 4
    },
    {
        // Laufschrift (fuer Netscape 4 im Container, damit
        // die LAufschrift an der richtigen Stelle erscheint
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(AMARQUEE,BEFORE, NONE),      // IE 4
        TE(AMARQUEE,BEFORE, NONE),      // SW
        TE(GRFFRM,  BEFORE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, BEFORE, NONE),      // IE 4
        TE(CONTROL, BEFORE, NONE),      // SW
        // hier koennte man einen Container draus machen (Import fehlt)
        TE(CONTROL, BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(GRFFRM,  BEFORE, NONE),      // IE 4
        TE(GRFFRM,  BEFORE, NONE),      // SW
        TE(GRFFRM,  BEFORE, NONE)       // Netscape 4
    }
};

sal_uInt8 aHTMLOutFrameParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, INSIDE, NONE),      // HTML 3.2
        TE(TBLNODE, INSIDE, NONE),      // IE 4
        TE(TBLNODE, INSIDE, NONE),      // SW
        TE(TBLNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, SPAN)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPACER,  INSIDE, NONE),      // SW
        TE(SPACER,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,INSIDE, NONE),      // IE 4
        TE(AMARQUEE,INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // SW
        // hier koennte man einen Container draus machen (Import fehlt)
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    }
};

sal_uInt8 aHTMLOutFrameParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, SPAN)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,INSIDE, NONE),      // IE 4
        TE(AMARQUEE,INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // SW
        // Netscape schaltet FORM bei Controls in abs.-pos. SPAN aus.
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, SPAN)       // Netscape 4
    }
};

sal_uInt8 aHTMLOutFrameAsCharTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, NONE)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(SPACER,  INSIDE, NONE),      // SW
        TE(SPACER,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Laufschrift (kann immer als MARQUEE exportiert werden, weil
        // der Inhalt an der richtigen Stelle erscheint
        TE(MARQUEE, INSIDE, NONE),      // HTML 3.2
        TE(MARQUEE, INSIDE, NONE),      // IE 4
        TE(MARQUEE, INSIDE, NONE),      // SW
        TE(MARQUEE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // SW
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

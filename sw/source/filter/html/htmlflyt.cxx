/*************************************************************************
 *
 *  $RCSfile: htmlflyt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "htmlfly.hxx"

#define TE(t,p,c) (BYTE)( HTML_OUT_##t | HTML_POS_##p | HTML_CNTNR_##c )

BYTE aHTMLOutFrmPageFlyTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4!
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  PREFIX, NONE),      // IE 4
        TE(MULTICOL,BEFORE, NONE),      // Netscape 3
        TE(MULTICOL,PREFIX, NONE),      // SW
        TE(MULTICOL,PREFIX, DIV)        // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(DIV,     PREFIX, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(DIV,     PREFIX, NONE),      // SW
        TE(DIV,     PREFIX, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, PREFIX, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // Netscape 3
        TE(GRFNODE, PREFIX, NONE),      // SW
        TE(GRFNODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLENODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLENODE, PREFIX, SPAN)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, PREFIX, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLENODE, PREFIX, NONE),      // SW
        TE(OLEGRF,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  PREFIX, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLEGRF,  PREFIX, NONE),      // SW
        TE(OLEGRF,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,PREFIX, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(AMARQUEE,PREFIX, NONE),      // SW
        TE(GRFFRM,  PREFIX, SPAN)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, PREFIX, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // Netscape 3
        TE(CONTROL, PREFIX, NONE),      // SW
        // Netscape schaltet FORM bei Controls in abs.-pos. SPAN aus.
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  PREFIX, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  PREFIX, NONE),      // SW
        TE(GRFFRM,  PREFIX, SPAN)       // Netscape 4
    }
};

BYTE aHTMLOutFrmParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(TBLNODE, BEFORE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(TBLNODE, BEFORE, NONE),      // SW
        TE(TBLNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(DIV,     BEFORE, NONE),      // SW
        TE(TBLNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(GRFFRM,  BEFORE, NONE),      // IE 4
        TE(MULTICOL,BEFORE, NONE),      // Netscape 3
        TE(MULTICOL,BEFORE, NONE),      // SW
        TE(MULTICOL,BEFORE, DIV)        // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(SPACER,  BEFORE, NONE),      // Netscape 3
        TE(SPACER,  BEFORE, NONE),      // SW
        TE(SPACER,  BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(GRFFRM,  BEFORE, NONE),      // Netscape 3
        TE(DIV,     BEFORE, NONE),      // SW
        TE(DIV,     BEFORE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, BEFORE, NONE),      // HTML 3.2
        TE(GRFNODE, BEFORE, NONE),      // IE 4
        TE(GRFNODE, BEFORE, NONE),      // Netscape 3
        TE(GRFNODE, BEFORE, NONE),      // SW
        TE(GRFNODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLENODE, BEFORE, NONE),      // Netscape 3
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLENODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLENODE, BEFORE, NONE),      // Netscape 3
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLENODE, BEFORE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  BEFORE, NONE),      // HTML 3.2
        TE(OLENODE, BEFORE, NONE),      // IE 4
        TE(OLEGRF,  BEFORE, NONE),      // Netscape 3
        TE(OLENODE, BEFORE, NONE),      // SW
        TE(OLEGRF,  BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  BEFORE, NONE),      // HTML 3.2
        TE(OLEGRF,  BEFORE, NONE),      // IE 4
        TE(OLEGRF,  BEFORE, NONE),      // Netscape 3
        TE(OLEGRF,  BEFORE, NONE),      // SW
        TE(OLEGRF,  BEFORE, NONE)       // Netscape 4
    },
    {
        // Laufschrift (fuer Netscape 4 im Container, damit
        // die LAufschrift an der richtigen Stelle erscheint
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(AMARQUEE,BEFORE, NONE),      // IE 4
        TE(GRFFRM,  BEFORE, NONE),      // Netscape 3
        TE(AMARQUEE,BEFORE, NONE),      // SW
        TE(GRFFRM,  BEFORE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, BEFORE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // Netscape 3
        TE(CONTROL, BEFORE, NONE),      // SW
        // hier koennte man einen Container draus machen (Import fehlt)
        TE(CONTROL, BEFORE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  BEFORE, NONE),      // HTML 3.2
        TE(GRFFRM,  BEFORE, NONE),      // IE 4
        TE(GRFFRM,  BEFORE, NONE),      // Netscape 3
        TE(GRFFRM,  BEFORE, NONE),      // SW
        TE(GRFFRM,  BEFORE, NONE)       // Netscape 4
    }
};

BYTE aHTMLOutFrmParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
#if 0
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(TBLNODE, BEFORE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(TBLNODE, BEFORE, NONE),      // SW
        TE(TBLNODE, BEFORE, NONE)       // Netscape 4
#endif
        TE(TBLNODE, INSIDE, NONE),      // HTML 3.2
        TE(TBLNODE, INSIDE, NONE),      // IE 4
        TE(TBLNODE, INSIDE, NONE),      // Netscape 3
        TE(TBLNODE, INSIDE, NONE),      // SW
        TE(TBLNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
#if 0
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(DIV,     BEFORE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(DIV,     BEFORE, NONE),      // SW
        TE(DIV,     BEFORE, NONE)       // Netscape 4
#endif
        TE(TBLNODE, INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(TBLNODE, INSIDE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,INSIDE, NONE),      // Netscape 3
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, SPAN)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(SPACER,  INSIDE, NONE),      // Netscape 3
        TE(SPACER,  INSIDE, NONE),      // SW
        TE(SPACER,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // Netscape 3
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(AMARQUEE,INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // Netscape 3
        TE(CONTROL, INSIDE, NONE),      // SW
        // hier koennte man einen Container draus machen (Import fehlt)
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    }
};

BYTE aHTMLOutFrmParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(TBLNODE, BEFORE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(TBLNODE, BEFORE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,BEFORE, NONE),      // Netscape 3
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, SPAN)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(SPAN,    INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(SPAN,    INSIDE, NONE),      // SW
        TE(SPAN,    INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // Netscape 3
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, SPAN)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // Laufschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(AMARQUEE,INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(AMARQUEE,INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, SPAN)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // Netscape 3
        TE(CONTROL, INSIDE, NONE),      // SW
        // Netscape schaltet FORM bei Controls in abs.-pos. SPAN aus.
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, SPAN)       // Netscape 4
    }
};

BYTE aHTMLOutFrmAsCharTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        // Textrahmen mit Tabelle
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Textrahmen mit Tabelle und Ueberschrift
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // spaltiger Rahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(MULTICOL,INSIDE, NONE),      // Netscape 3
        TE(MULTICOL,INSIDE, NONE),      // SW
        TE(MULTICOL,INSIDE, NONE)       // Netscape 4
    },
    {
        // leerer Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(SPACER,  INSIDE, NONE),      // Netscape 3
        TE(SPACER,  INSIDE, NONE),      // SW
        TE(SPACER,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstiger Textreahmen
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Grafik-Node
        TE(GRFNODE, INSIDE, NONE),      // HTML 3.2
        TE(GRFNODE, INSIDE, NONE),      // IE 4
        TE(GRFNODE, INSIDE, NONE),      // Netscape 3
        TE(GRFNODE, INSIDE, NONE),      // SW
        TE(GRFNODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Plugin
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Applet
        TE(OLENODE, INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLENODE, INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLENODE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Floating-Frame
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLENODE, INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLENODE, INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige OLE-Objekte
        TE(OLEGRF,  INSIDE, NONE),      // HTML 3.2
        TE(OLEGRF,  INSIDE, NONE),      // IE 4
        TE(OLEGRF,  INSIDE, NONE),      // Netscape 3
        TE(OLEGRF,  INSIDE, NONE),      // SW
        TE(OLEGRF,  INSIDE, NONE)       // Netscape 4
    },
    {
        // Laufschrift (kann immer als MARQUEE exportiert werden, weil
        // der Inhalt an der richtigen Stelle erscheint
        TE(MARQUEE, INSIDE, NONE),      // HTML 3.2
        TE(MARQUEE, INSIDE, NONE),      // IE 4
        TE(MARQUEE, INSIDE, NONE),      // Netscape 3
        TE(MARQUEE, INSIDE, NONE),      // SW
        TE(MARQUEE, INSIDE, NONE)       // Netscape 4
    },
    {
        // Controls
        TE(CONTROL, INSIDE, NONE),      // HTML 3.2
        TE(CONTROL, INSIDE, NONE),      // IE 4
        TE(CONTROL, INSIDE, NONE),      // Netscape 3
        TE(CONTROL, INSIDE, NONE),      // SW
        TE(CONTROL, INSIDE, NONE)       // Netscape 4
    },
    {
        // sonstige Zeichen-Objekte
        TE(GRFFRM,  INSIDE, NONE),      // HTML 3.2
        TE(GRFFRM,  INSIDE, NONE),      // IE 4
        TE(GRFFRM,  INSIDE, NONE),      // Netscape 3
        TE(GRFFRM,  INSIDE, NONE),      // SW
        TE(GRFFRM,  INSIDE, NONE)       // Netscape 4
    }
};
/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlflyt.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.9  2000/09/18 16:04:44  willem.vandorp
      OpenOffice header added.

      Revision 1.8  1999/11/17 16:53:54  mib
      #67938#: Export columned frames as images in MS-IE mode

      Revision 1.7  1998/04/20 07:46:02  MIB
      IE3-Export weg


      Rev 1.6   20 Apr 1998 09:46:02   MIB
   IE3-Export weg

      Rev 1.5   02 Apr 1998 19:48:30   MIB
   Spaltige Rahmen fuer SW ohne Container exportieren

      Rev 1.4   31 Mar 1998 11:23:36   MIB
   Keine Verschiebung von manchen am Zeichen geb. Rahmen und anderes

      Rev 1.3   27 Mar 1998 17:25:50   MIB
   direkte absolute Positionierung und Groessen-Export von Controls und Marquee

      Rev 1.2   20 Mar 1998 10:50:14   MIB
   Anpassungen an Netscape

      Rev 1.1   19 Mar 1998 09:18:58   MIB
   Rahmen-Export verbessert/aufgeraeumt

      Rev 1.0   14 Mar 1998 19:05:18   MIB
   Initial revision.


*************************************************************************/


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ide_pch.hxx,v $
 * $Revision: 1.4 $
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
#include <svheader.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/menu.hxx>

#include <svtools/svarray.hxx>
#include <svtools/itemset.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>

#define _BASIC_TEXTPORTIONS
#include <basic/sbdef.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/basmgr.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/event.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/progress.hxx>


#define ITEMID_FONT             0
#define ITEMID_POSTURE          0
#define ITEMID_WEIGHT           0
#define ITEMID_SHADOWED         0
#define ITEMID_CONTOUR          0
#define ITEMID_CROSSEDOUT       0
#define ITEMID_UNDERLINE        0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_COLOR            0
#define ITEMID_ADJUST           0
#define ITEMID_LINESPACING      0
#define ITEMID_TABSTOP          0
#define ITEMID_LRSPACE          0
#define ITEMID_ULSPACE          0
#define ITEMID_SEARCH           0
#define ITEMID_SEARCH           0
#define ITEMID_SIZE             0
#define ITEMID_FONTLIST     0
#define ITEMID_AUTOKERN     0
#define ITEMID_WORDLINEMODE 0
#define ITEMID_PROPSIZE     0
#define ITEMID_CHARSETCOLOR 0
#define ITEMID_KERNING      0
#define ITEMID_CASEMAP      0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_LANGUAGE     0
#define ITEMID_NOLINEBREAK  0
#define ITEMID_NOHYPHENHERE 0
#define ITEMID_BLINK        0
#define ITEMID_PAPERBIN     0
#define ITEMID_PRINT        0
#define ITEMID_OPAQUE       0
#define ITEMID_PROTECT      0
#define ITEMID_SHADOW       0
#define ITEMID_BOX          0
#define ITEMID_BOXINFO      0
#define ITEMID_FMTBREAK     0
#define ITEMID_FMTKEEP      0
#define ITEMID_LINE         0
#define ITEMID_BRUSH        0

#include <svx/srchitem.hxx>
#include <svx/flstitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/postitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fwdtitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/shdditem.hxx>
#include <svx/akrnitem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/prszitem.hxx>
#include <svx/colritem.hxx>
#include <svx/cscoitem.hxx>
#include <svx/kernitem.hxx>

#include <svx/cmapitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/langitem.hxx>
#include <svx/nlbkitem.hxx>
#include <svx/nhypitem.hxx>
#include <svx/lcolitem.hxx>
#include <svx/blnkitem.hxx>

#include <svx/pbinitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/prntitem.hxx>
#include <svx/opaqitem.hxx>
#include <svx/protitem.hxx>
#include <svx/shaditem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brkitem.hxx>
#include <svx/keepitem.hxx>
#include <svx/bolnitem.hxx>
#include <svx/brshitem.hxx>


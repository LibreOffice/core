/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:14:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#define ITEMID_WEIGHT       0
#define ITEMID_UNDERLINE    0
#define ITEMID_CROSSEDOUT   0
#define ITEMID_CONTOUR      0
#define ITEMID_SHADOWED     0
#define ITEMID_POSTURE      0
#define ITEMID_FONTHEIGHT   0
#define ITEMID_FONT         0
#define ITEMID_CHARRELIEF   0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_COLOR        0
#define ITEMID_WORDLINEMODE 0
#define ITEMID_CHARSCALE_W  0
#define ITEMID_AUTOKERN     0
#define ITEMID_ULSPACE      0
#define ITEMID_LRSPACE      0
#define ITEMID_LINESPACING  0
#define ITEMID_LANGUAGE     0
#define ITEMID_KERNING      0
#define ITEMID_TABSTOP      0
#define ITEMID_ADJUST       0
#define ITEMID_EMPHASISMARK 0
#ifndef _SVX_WGHTITEM_HXX
#include "wghtitem.hxx"
#endif
#ifndef _SVX_UDLNITEM_HXX
#include "udlnitem.hxx"
#endif
#ifndef _SVX_CRSDITEM_HXX
#include "crsditem.hxx"
#endif
#ifndef _SVX_ITEM_HXX
#include "cntritem.hxx"
#endif
#ifndef _SVX_SHDDITEM_HXX
#include "shdditem.hxx"
#endif
#ifndef _SVX_POSTITEM_HXX
#include "postitem.hxx"
#endif
#ifndef _SVX_CLIPBOARDCTL_HXX_
#include "clipfmtitem.hxx"
#endif
#ifndef _SVX_FHGTITEM_HXX
#include "fhgtitem.hxx"
#endif
#ifndef _SVX_FONTITEM_HXX
#include "fontitem.hxx"
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include "charreliefitem.hxx"
#endif
#ifndef _SVX_ESCPITEM_HXX
#include "escpitem.hxx"
#endif
#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif
#ifndef _SVX_WRLMITEM_HXX
#include "wrlmitem.hxx"
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include "charscaleitem.hxx"
#endif
#ifndef _SVX_AKRNITEM_HXX
#include "akrnitem.hxx"
#endif
#ifndef _SVX_ULSPITEM_HXX
#include "ulspitem.hxx"
#endif
#ifndef _SVX_LRSPITEM_HXX
#include "lrspitem.hxx"
#endif
#ifndef _SVX_LSPCITEM_HXX
#include "lspcitem.hxx"
#endif
#ifndef _SVX_LANGITEM_HXX
#include "langitem.hxx"
#endif
#ifndef _SVX_KERNITEM_HXX
#include "kernitem.hxx"
#endif
#ifndef _SVX_TSPTITEM_HXX
#include "tstpitem.hxx"
#endif
#ifndef _SVX_ADJITEM_HXX
#include "adjitem.hxx"
#endif
#ifndef _SVX_EMPHITEM_HXX
#include "emphitem.hxx"
#endif

#define SFX_TYPEMAP
#include "svxslots.hxx"


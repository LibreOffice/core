/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: slotadd.cxx,v $
 * $Revision: 1.30.124.1 $
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

#include "hintids.hxx"
#include "uiitems.hxx"
#include "cmdid.h"

#include <svtools/globalnameitem.hxx>


#include <svtools/imageitm.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/rectitem.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <svx/rulritem.hxx>
#include <svx/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/SmartTagItem.hxx>
#include <svtools/ptitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/srchitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/protitem.hxx>
#include <svx/opaqitem.hxx>
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#include <svx/akrnitem.hxx>
#include <svx/keepitem.hxx>
#include <svx/kernitem.hxx>
#include <svx/spltitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/shaditem.hxx>
#include <svx/pbinitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/prntitem.hxx>
#include <svx/orphitem.hxx>
#include <svx/widwitem.hxx>
#include <svx/bolnitem.hxx>
#include <svx/pmdlitem.hxx>
#include <svx/cmapitem.hxx>
#include <svx/langitem.hxx>
#include <svx/brkitem.hxx>
#include <svx/hyznitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/lspcitem.hxx>
#include <svx/adjitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/fontitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/postitem.hxx>
#include <svx/fhgtitem.hxx>
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#include <svx/colritem.hxx>
#include <svx/boxitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/xgrad.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <svx/xftsfit.hxx>
#include <svx/grafctrl.hxx>


#include <fmtornt.hxx>
#include <paratr.hxx>
#include <fmtinfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include "envimg.hxx"
#include "frmatr.hxx"
#include "cfgitems.hxx"
#include "grfatr.hxx"
#include "fmtline.hxx"
#include <svx/clipfmtitem.hxx>
#include <svx/blnkitem.hxx>
#include <svtools/slstitm.hxx>
#include <svx/paravertalignitem.hxx>
#include <svx/charreliefitem.hxx>
#include <svx/charrotateitem.hxx>
#include <svx/charscaleitem.hxx>
#include <svx/postattr.hxx>
#include <sfx2/frame.hxx>
#include <svx/chrtitem.hxx>
#include <svx/drawitem.hxx>
#include <avmedia/mediaitem.hxx>

#define SvxDrawToolItem  SfxAllEnumItem
#define SvxDrawAlignItem SfxAllEnumItem
#define SvxDrawBezierItem SfxAllEnumItem
#define avmedia_MediaItem ::avmedia::MediaItem

#define SFX_TYPEMAP
#include <sfx2/msg.hxx>
#include "swslots.hxx"




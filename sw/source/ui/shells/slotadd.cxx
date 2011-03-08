/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svl/globalnameitem.hxx>
#include <editeng/memberids.hrc>
#include <svl/imageitm.hxx>
#include <svl/aeitem.hxx>
#include <svl/rectitem.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <svx/rulritem.hxx>
#include <svx/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/SmartTagItem.hxx>
#include <svl/ptitem.hxx>
#include <svx/pageitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/lrspitem.hxx>
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
#include <editeng/blnkitem.hxx>
#include <svl/slstitm.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

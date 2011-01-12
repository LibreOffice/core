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
#include "precompiled_sc.hxx"


#include <svx/svxids.hrc>
#include "sc.hrc"
#include "scitems.hxx"
#include <editeng/memberids.hrc>


#define ITEMID_DBTYPE           0
























#include <sfx2/msg.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/objitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/srchitem.hxx>
#include <svx/postattr.hxx>
#include <editeng/postitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <svx/zoomitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <svl/ptitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/algitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnwtit.hxx>
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
#include <editeng/langitem.hxx>
#include <svx/grafctrl.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/drawitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/globalnameitem.hxx>
#include <svx/chrtitem.hxx>
#include <svx/zoomslideritem.hxx>

// #i25616#
#include <svx/sdshitm.hxx>

#include <svl/aeitem.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/frame.hxx>
#include "attrib.hxx"

#define SvxDrawToolItem             SfxAllEnumItem
#define SvxDrawAlignItem            SfxAllEnumItem
#define SvxChooseControlItem        SfxEnumItem
#define avmedia_MediaItem           ::avmedia::MediaItem

#define SFX_TYPEMAP
#include "scslots.hxx"

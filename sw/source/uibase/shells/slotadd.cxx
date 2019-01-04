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

#include <config_options.h>

#include <hintids.hxx>
#include <uiitems.hxx>
#include <cmdid.h>
#include <unomid.h>

#include <svl/globalnameitem.hxx>
#include <editeng/memberids.h>
#include <svl/imageitm.hxx>
#include <svl/aeitem.hxx>
#include <svl/rectitem.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <svx/rulritem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/SmartTagItem.hxx>
#include <svl/ptitem.hxx>
#include <svx/pageitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/watermarkitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/contouritem.hxx>
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
#include <svx/grafctrl.hxx>

#include <fmtornt.hxx>
#include <paratr.hxx>
#include <fmtinfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <envimg.hxx>
#include <frmatr.hxx>
#include <cfgitems.hxx>
#include <grfatr.hxx>
#include <fmtline.hxx>
#include <svx/clipfmtitem.hxx>
#include <editeng/blinkitem.hxx>
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

#define avmedia_MediaItem ::avmedia::MediaItem

#include <svx/xflftrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/AffineMatrixItem.hxx>
#include <svx/galleryitem.hxx>

#define SFX_TYPEMAP
#include <sfx2/msg.hxx>
#include <swslots.hxx>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

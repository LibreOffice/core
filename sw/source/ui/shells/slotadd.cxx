/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <editeng/tstpitem.hxx>
#endif
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
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <editeng/cntritem.hxx>
#endif
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




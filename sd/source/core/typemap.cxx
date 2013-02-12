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


#include "eetext.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fontitem.hxx>
#include <svl/poolitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <svx/chrtitem.hxx>
#include <sfx2/msg.hxx>
#include <svl/globalnameitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/postattr.hxx>
#include <editeng/editdata.hxx>
#include <svx/srchdlg.hxx>
#include <svx/rulritem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/algitem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/svdattr.hxx>
#include <svx/grafctrl.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include "sdattr.hxx"
#include <svx/zoomslideritem.hxx>
#include <editeng/memberids.hrc>
#include <svx/xftstit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <svx/xftsfit.hxx>
#include <avmedia/mediaitem.hxx>
#include <svx/drawitem.hxx>

// #UndoRedo#
#include <svl/slstitm.hxx>

#include <svl/lckbitem.hxx>


#define CharSetItem                 SfxUInt16Item
#define FontFamilyItem              SfxUInt16Item
#define FontPitchItem               SfxUInt16Item
#define FontAlignItem               SfxUInt16Item
#define FontWeightItem              SfxUInt16Item
#define FontUnderlineItem           SfxUInt16Item
#define FontStrikeoutItem           SfxUInt16Item
#define FontItalicItem              SfxUInt16Item
#define SvxDbTypeItem               SfxUInt16Item
#define SvxLineSpaceItem            SfxUInt16Item
#define SvxInterLineSpaceItem       SfxUInt16Item
#define SvxBreakItem                SfxUInt16Item
#define BrushStyleItem              SfxUInt16Item
#define SvxNumTypeItem              SfxUInt16Item
#define SvxShadowLocationItem       SfxUInt16Item
#define SvxDbTypeItem               SfxUInt16Item
#define SvxDrawToolEnumItem         SfxUInt16Item
#define SvxChooseControlItem        SfxEnumItem
#define SvxDrawToolItem             SfxUInt16Item
#define SvxCellHorJustifyEnumItem   SfxUInt16Item
#define SvxCellVerJustifyEnumItem   SfxUInt16Item
#define SvxCellOrientationEnumItem  SfxUInt16Item
#define SvxLanguage                 SfxUInt16Item
#define avmedia_MediaItem           ::avmedia::MediaItem
#include <sfx2/tplpitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/rectitem.hxx>

#include <sfx2/frame.hxx>

#define SFX_TYPEMAP
#include "sdslots.hxx"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

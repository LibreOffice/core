/*************************************************************************
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-25 14:34:49 $
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

#pragma hdrstop

#include <svx/svxids.hrc>
//#include <svx/hlnkitem.hxx>
#include "scitems.hxx"

#define ITEMID_ADJUST           0
#define ITEMID_CASEMAP          0
#define ITEMID_DBTYPE           0
#define ITEMID_LINESPACE        0
#define ITEMID_INTERLINESPACE   0
#define ITEMID_BREAK            0
#define ITEMID_NUMTYPE          0
#define ITEMID_SHADOWLOCATION   0
#define ITEMID_PAGE             0
#define ITEMID_PAGEMODEL        0
#define ITEMID_ORPHANS          0
#define ITEMID_KERNING          0
#define ITEMID_POSTITDATE       0
#define ITEMID_TEXT             0
#define ITEMID_AUTHOR           0
#define ITEMID_DATE             0
#define ITEMID_SIZE             0
#define ITEMID_HYPHENZONE       0
#define ITEMID_FMTSPLIT         0
#define ITEMID_FMTKEEP          0
#define ITEMID_FMTBREAK         0
#define ITEMID_AUTOKERN         0
#define ITEMID_LONGLRSPACE      0
#define ITEMID_LONGULSPACE      0
#define ITEMID_WIDOWS           0


#include <sfx2/msg.hxx>
#include <svtools/stritem.hxx>
#include <sfx2/objitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/postattr.hxx>
#include <svx/postitem.hxx>
#include <sfx2/tplpitem.hxx>
#include <svx/zoomitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/colritem.hxx>
#include <svx/boxitem.hxx>
#include <svx/bolnitem.hxx>
#include <svtools/ptitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/algitem.hxx>
#include <svx/udlnitem.hxx>
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
#include <svx/langitem.hxx>
#include <svx/grafctrl.hxx>

#include "attrib.hxx"

#define CharSetItem                 SfxUInt16Item
#define FontFamilyItem              SfxUInt16Item
#define FontPitchItem               SfxUInt16Item
#define FontAlignItem               SfxUInt16Item
#define FontWeightItem              SfxUInt16Item
#define FontUnderlineItem           SfxUInt16Item
#define FontStrikeoutItem           SfxUInt16Item
#define FontItalicItem              SfxUInt16Item
#define SvxShadowedItem             SfxUInt16Item
#define SvxContourItem              SfxUInt16Item
#define SvxCrossedOutItem           SfxUInt16Item
#define SvxDbTypeItem               SfxUInt16Item
#define SvxLineSpaceItem            SfxUInt16Item
#define SvxInterLineSpaceItem       SfxUInt16Item
#define SvxBreakItem                SfxUInt16Item
#define BrushStyleItem              SfxUInt16Item
#define SvxNumTypeItem              SfxUInt16Item
#define SvxShadowLocationItem       SfxUInt16Item
#define SvxDbTypeItem               SfxUInt16Item
#define SvxChooseControlEnumItem    SfxUInt16Item
#define SvxDrawToolEnumItem         SfxUInt16Item

//#define SvxChooseControlItem      SfxUInt16Item
//#define SvxDrawToolItem               SfxUInt16Item
#define SvxDrawToolItem             SfxEnumItem
#define SvxDrawAlignItem            SfxEnumItem
#define SvxChooseControlItem        SfxEnumItem

#define SvxCellHorJustifyEnumItem   SfxUInt16Item
#define SvxCellVerJustifyEnumItem   SfxUInt16Item
#define SvxCellOrientationEnumItem  SfxUInt16Item
#define SvxLanguage                 SfxUInt16Item

#define SFX_TYPEMAP
#include "scslots.hxx"



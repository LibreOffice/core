/*************************************************************************
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

#define ITEMID_CASEMAP          0
#define ITEMID_DBTYPE           0
#define ITEMID_LINESPACE        0
#define ITEMID_INTERLINESPACE   0
#define ITEMID_BREAK            0
#define ITEMID_NUMTYPE          0
#define ITEMID_SHADOWLOCATION   0
#define ITEMID_BRUSH            0
#define ITEMID_PAGE             0
#define ITEMID_PAGEMODEL        0
#define ITEMID_ORPHANS          0
#define ITEMID_KERNING          0
#define ITEMID_POSTITDATE       0
#define ITEMID_SEARCH           0
#define ITEMID_TEXT             0
#define ITEMID_AUTHOR           0
#define ITEMID_DATE             0
#define ITEMID_SIZE             0
#define ITEMID_LANGUAGE         0
#define ITEMID_HYPHENZONE       0
#define ITEMID_FMTSPLIT         0
#define ITEMID_FMTKEEP          0
#define ITEMID_FMTBREAK         0
#define ITEMID_AUTOKERN         0
#define ITEMID_LINE             0
#define ITEMID_LONGLRSPACE      0
#define ITEMID_LONGULSPACE      0
#define ITEMID_ZOOM             0
#define ITEMID_HORJUSTIFY       0
#define ITEMID_VERJUSTIFY       0
#define ITEMID_ORIENTATION      0
#define ITEMID_WIDOWS           0

#include "eetext.hxx"
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif


#include <svx/hlnkitem.hxx>
#include <svx/postattr.hxx>
#include <svx/editdata.hxx>
#include <svx/srchdlg.hxx>
#include <svx/rulritem.hxx>
#include <svx/srchitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/svxenum.hxx>
#include <svx/algitem.hxx>
#include <svx/zoomitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/svdattr.hxx>
#include "sdattr.hxx"
#include "tbxitem.hxx"

#ifndef _SVX_XFTSTIT_HXX //autogen
#include <svx/xftstit.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX //autogen
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX //autogen
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include <svx/xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX //autogen
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX__XGRADIENT_HXX //autogen
#include <svx/xgrad.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX //autogen
#include <svx/xflgrit.hxx>
#endif
#ifndef _SVX_XFLHTIT_HXX //autogen
#include <svx/xflhtit.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX //autogen
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_TEXTIT0_HXX //autogen
#include <svx/xtextit0.hxx>
#endif
#ifndef _SVX_XFTADIT_HXX //autogen
#include <svx/xftadit.hxx>
#endif
#ifndef _SVX_XFTDIIT_HXX //autogen
#include <svx/xftdiit.hxx>
#endif
#ifndef _SVX_XFTMRIT_HXX //autogen
#include <svx/xftmrit.hxx>
#endif
#ifndef _SVX_XFTOUIT_HXX //autogen
#include <svx/xftouit.hxx>
#endif
#ifndef _SVX_XFTSHIT_HXX //autogen
#include <svx/xftshit.hxx>
#endif
#ifndef _SVX_XFTSHCLIT_HXX //autogen
#include <svx/xftshcit.hxx>
#endif
#ifndef _SVX_XFTSHXY_HXX //autogen
#include <svx/xftshxy.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _SVX_TEXTIT0_HXX //autogen
#include <svx/xtextit0.hxx>
#endif

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
//#define SvxChooseControlEnumItem  SfxUInt16Item
#define SvxDrawToolEnumItem         SfxUInt16Item
#define SvxChooseControlItem        SfxEnumItem
#define SvxDrawToolItem             SfxUInt16Item
#define SvxCellHorJustifyEnumItem   SfxUInt16Item
#define SvxCellVerJustifyEnumItem   SfxUInt16Item
#define SvxCellOrientationEnumItem  SfxUInt16Item
#define SvxLanguage                 SfxUInt16Item
#define SfxLockBytesItem            SfxPoolItem
#define OfaStringListItem           SfxStringListItem

#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif

#define SFX_TYPEMAP
#include "sdslots.hxx"


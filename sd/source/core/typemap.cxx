/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:29:47 $
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
#include "precompiled_sd.hxx"

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
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif

#include <svtools/globalnameitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/postattr.hxx>
#include <svx/editdata.hxx>
#include <svx/srchdlg.hxx>
#include <svx/rulritem.hxx>
#include <svx/clipfmtitem.hxx>
#include <sfx2/srchitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/svxenum.hxx>
#include <svx/algitem.hxx>
#include <svx/zoomitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/svdattr.hxx>
#include <svx/grafctrl.hxx>
#include <svx/boxitem.hxx>
#include <svx/bolnitem.hxx>
#include "sdattr.hxx"

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
#ifndef _AVMEDIA_MEDIAITEM_HXX
#include <avmedia/mediaitem.hxx>
#endif

#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

// #UndoRedo#
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif

#include <svtools/lckbitem.hxx>


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
//#define SfxLockBytesItem            SfxPoolItem
#define OfaStringListItem           SfxStringListItem
#define avmedia_MediaItem           ::avmedia::MediaItem

#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif

#include <sfx2/frame.hxx>

#define SFX_TYPEMAP
#include "sdslots.hxx"


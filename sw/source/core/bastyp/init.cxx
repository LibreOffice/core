/*************************************************************************
 *
 *  $RCSfile: init.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:00:30 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif

#ifndef _SVX_XMLCNITM_HXX
#include <svx/xmlcnitm.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_NLBKITEM_HXX
#include <svx/nlbkitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_PRSZITEM_HXX
#include <svx/prszitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX
#include <svx/cscoitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_NHYPITEM_HXX
#include <svx/nhypitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_SCRIPSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVXSWAFOPT_HXX
#include <svx/swafopt.hxx>
#endif
#ifndef _MySVXACORR_HXX
#include <svx/svxacorr.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif
#ifndef _SVX_PARAVERTALIGNITEM_HXX
#include <svx/paravertalignitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <svx/pgrditem.hxx>
#endif
#ifndef _SVX_CHARHIDDENITEM_HXX
#include <svx/charhiddenitem.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <svx/acorrcfg.hxx>
#include <vcl/svapp.hxx>

#ifndef _FMTHBSH_HXX
#include <fmthbsh.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FMTRFMRK_HXX
#include <fmtrfmrk.hxx>
#endif
#ifndef _FMTTSPLT_HXX
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTROWSPLT_HXX //autogen
#include <fmtrowsplt.hxx>
#endif
#ifndef _FMTEIRO_HXX
#include <fmteiro.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX
#include <fmthdft.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FMTFORDR_HXX
#include <fmtfordr.hxx>
#endif
#ifndef _FMTFLCNT_HXX
#include <fmtflcnt.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#include <hfspacingitem.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CHECKIT_HXX
#include <checkit.hxx>
#endif

#ifndef _SWCALWRP_HXX
#include <swcalwrp.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

// OD 09.10.2003 #i18732#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
// OD 2004-05-05 #i28701#
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif


extern void _FrmFinit();
extern void ClearFEShellTabCols();

/*************************************************************************
|*  einige Bereiche fuer die Set in Collections / Nodes
|*************************************************************************/
    // AttrSet-Range fuer die 2 Break-Attribute
USHORT __FAR_DATA aBreakSetRange[] = {
    RES_PAGEDESC, RES_BREAK,
    0 };

    // AttrSet-Range fuer die TxtFmtColl
USHORT __FAR_DATA aTxtFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die GrfFmtColl
USHORT __FAR_DATA aGrfFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die TextNode
USHORT __FAR_DATA aTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die NoTxtNode
USHORT __FAR_DATA aNoTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_BREAK,
    RES_BACKGROUND,     RES_SHADOW,
    RES_HORI_ORIENT,    RES_HORI_ORIENT,
    RES_KEEP,           RES_KEEP,
    RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    // --> collapsing borders FME 2005-05-27 #i29550#
    RES_COLLAPSING_BORDERS, RES_COLLAPSING_BORDERS,
    // <-- collapsing
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableLineSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_ROW_SPLIT,      RES_ROW_SPLIT,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableBoxSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    RES_BOXATR_BEGIN,   RES_BOXATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwFrmFmt
USHORT __FAR_DATA aFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwCharFmt
USHORT __FAR_DATA aCharFmtSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwPageDescFmt
USHORT __FAR_DATA aPgFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

/******************************************************************************
 * lege eine Tabelle fuer einen Zugriff auf die
 * Default-Format-Attribute an
 ******************************************************************************/
SwDfltAttrTab __FAR_DATA aAttrTab;

SfxItemInfo __FAR_DATA aSlotTab[] =
{
    { SID_ATTR_CHAR_CASEMAP, SFX_ITEM_POOLABLE },       // RES_CHRATR_CASEMAP
    { SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },  // RES_CHRATR_CHARSETCOLOR
    { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },         // RES_CHRATR_COLOR
    { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },       // RES_CHRATR_CONTOUR
    { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },     // RES_CHRATR_CROSSEDOUT
    { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },    // RES_CHRATR_ESCAPEMENT
    { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },          // RES_CHRATR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_FONTSIZE
    { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },       // RES_CHRATR_KERNING
    { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },      // RES_CHRATR_LANGUAGE
    { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },       // RES_CHRATR_POSTURE
    { SID_ATTR_CHAR_PROPSIZE, SFX_ITEM_POOLABLE },      // RES_CHRATR_PROPORTIONALFONTSIZE
    { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },      // RES_CHRATR_SHADOWED
    { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },     // RES_CHRATR_UNDERLINE
    { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },        // RES_CHRATR_WEIGHT
    { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },  // RES_CHRATR_WORDLINEMODE
    { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },      // RES_CHRATR_AUTOKERN
    { SID_ATTR_FLASH, SFX_ITEM_POOLABLE },              // RES_CHRATR_BLINK
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_NOLINEBREAK
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_NOHYPHEN
    { SID_ATTR_BRUSH_CHAR, SFX_ITEM_POOLABLE },         // RES_CHRATR_BACKGROUND
    { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },      // RES_CHRATR_CJK_FONT
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CJK_FONTSIZE
    { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },  // RES_CHRATR_CJK_LANGUAGE
    { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },   // RES_CHRATR_CJK_POSTURE
    { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_CJK_WEIGHT
    { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },      // RES_CHRATR_CTL_FONT
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CTL_FONTSIZE
    { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },  // RES_CHRATR_CTL_LANGUAGE
    { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },   // RES_CHRATR_CTL_POSTURE
    { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_CTL_WEIGHT
    { SID_ATTR_CHAR_ROTATED, SFX_ITEM_POOLABLE },       // RES_CHRATR_ROTATE
    { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },  // RES_CHRATR_EMPHASIS_MARK
    { SID_ATTR_CHAR_TWO_LINES, SFX_ITEM_POOLABLE },     // RES_CHRATR_TWO_LINES
    { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },    // RES_CHRATR_SCALEW
    { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },        // RES_CHRATR_RELIEF
    { SID_ATTR_CHAR_HIDDEN, SFX_ITEM_POOLABLE },        // RES_CHRATR_HIDDEN

    { FN_TXTATR_INET, 0 },                              // RES_TXTATR_INETFMT
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY4
    { 0, 0 },                                           // RES_TXTATR_REFMARK
    { 0, 0 },                                           // RES_TXTATR_TOXMARK
    { 0, 0 },                                           // RES_TXTATR_CHARFMT
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY5,
    { SID_ATTR_CHAR_CJK_RUBY, 0 },                      // RES_TXTATR_CJK_RUBY,
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_UNKNOWN_CONTAINER,
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY6,
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY7,

    { 0, 0 },                                           // RES_TXTATR_FIELD
    { 0, 0 },                                           // RES_TXTATR_FLYCNT
    { 0, 0 },                                           // RES_TXTATR_FTN
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_SOFTHYPH
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_HARDBLANK
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY1
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY2

    { SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },     // RES_PARATR_LINESPACING
    { SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },        // RES_PARATR_ADJUST
    { SID_ATTR_PARA_SPLIT, SFX_ITEM_POOLABLE },         // RES_PARATR_SPLIT
    { SID_ATTR_PARA_ORPHANS, SFX_ITEM_POOLABLE },       // RES_PARATR_ORPHANS
    { SID_ATTR_PARA_WIDOWS, SFX_ITEM_POOLABLE },        // RES_PARATR_WIDOWS
    { SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },            // RES_PARATR_TABSTOP
    { SID_ATTR_PARA_HYPHENZONE, SFX_ITEM_POOLABLE },    // RES_PARATR_HYPHENZONE
    { FN_FORMAT_DROPCAPS, 0 },                          // RES_PARATR_DROP
    { SID_ATTR_PARA_REGISTER, SFX_ITEM_POOLABLE },      // RES_PARATR_REGISTER
    { SID_ATTR_PARA_NUMRULE, 0 },                       // RES_PARATR_NUMRULE
    { SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },   // RES_PARATR_SCRIPTSPACE
    { SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },// RES_PARATR_HANGINGPUNCTUATION

    { SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },// RES_PARATR_FORBIDDEN_RULES
    { SID_PARA_VERTALIGN, SFX_ITEM_POOLABLE },          // RES_PARATR_VERTALIGN
    { SID_ATTR_PARA_SNAPTOGRID, SFX_ITEM_POOLABLE },    // RES_PARATR_SNAPTOGRID
    { SID_ATTR_BORDER_CONNECT, SFX_ITEM_POOLABLE },     // RES_PARATR_CONNECT_BORDER
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_DUMMY5
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_DUMMY6
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_DUMMY7
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_DUMMY8

    { 0, SFX_ITEM_POOLABLE },                           // RES_FILL_ORDER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRM_SIZE
    { SID_ATTR_PAGE_PAPERBIN, SFX_ITEM_POOLABLE },      // RES_PAPER_BIN
    { SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },            // RES_LR_SPACE
    { SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },            // RES_UL_SPACE
    { 0, 0 },                                           // RES_PAGEDESC
    { SID_ATTR_PARA_PAGEBREAK, SFX_ITEM_POOLABLE },     // RES_BREAK
    { 0, 0 },                                           // RES_CNTNT
    { 0, SFX_ITEM_POOLABLE },                           // RES_HEADER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FOOTER
    { 0, SFX_ITEM_POOLABLE },                           // RES_PRINT
    { FN_OPAQUE, SFX_ITEM_POOLABLE },                   // RES_OPAQUE
    { FN_SET_PROTECT, SFX_ITEM_POOLABLE },              // RES_PROTECT
    { FN_SURROUND, SFX_ITEM_POOLABLE },                 // RES_SURROUND
    { FN_VERT_ORIENT, SFX_ITEM_POOLABLE },              // RES_VERT_ORIENT
    { FN_HORI_ORIENT, SFX_ITEM_POOLABLE },              // RES_HORI_ORIENT
    { 0, 0 },                                           // RES_ANCHOR
    { SID_ATTR_BRUSH, SFX_ITEM_POOLABLE },              // RES_BACKGROUND
    { SID_ATTR_BORDER_OUTER, SFX_ITEM_POOLABLE },       // RES_BOX
    { SID_ATTR_BORDER_SHADOW, SFX_ITEM_POOLABLE },      // RES_SHADOW
    { SID_ATTR_MACROITEM, SFX_ITEM_POOLABLE },          // RES_FRMMACRO
    { FN_ATTR_COLUMNS, SFX_ITEM_POOLABLE },             // RES_COL
    { SID_ATTR_PARA_KEEP, SFX_ITEM_POOLABLE },          // RES_KEEP
    { 0, SFX_ITEM_POOLABLE },                           // RES_URL
    { 0, SFX_ITEM_POOLABLE },                           // RES_EDIT_IN_READONLY

    { 0, SFX_ITEM_POOLABLE },                           // RES_LAYOUT_SPLIT
    { 0, 0 },                                           // RES_CHAIN
    { 0, SFX_ITEM_POOLABLE },                           // RES_TEXTGRID
    { FN_FORMAT_LINENUMBER, SFX_ITEM_POOLABLE },        // RES_LINENUMBER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FTN_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },                           // RES_END_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },                           // RES_COLUMNBALANCE

    { SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },     // RES_FRAMEDIR

    { SID_ATTR_HDFT_DYNAMIC_SPACING, SFX_ITEM_POOLABLE }, // RES_HEADER_FOOTER_EAT_SPACING
    { FN_TABLE_ROW_SPLIT, SFX_ITEM_POOLABLE },            // RES_ROW_SPLIT
    // DVO, OD 18.09.2003 #i18732# - use slot-id define in svx
    { SID_SW_FOLLOW_TEXT_FLOW, SFX_ITEM_POOLABLE },         // RES_FOLLOW_TEXT_FLOW
    // --> collapsing borders FME 2005-05-27 #i29550#
    { SID_SW_COLLAPSING_BORDERS, SFX_ITEM_POOLABLE },       // RES_COLLAPSING_BORDERS
    // <-- collapsing
    // OD 2004-05-04 #i28701#
    { SID_SW_WRAP_INFLUENCE_ON_OBJPOS, SFX_ITEM_POOLABLE }, // RES_WRAP_INFLUENCE_ON_OBJPOS
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRMATR_DUMMY3
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRMATR_DUMMY4
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRMATR_DUMMY5

    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_MIRRORGRF
    { SID_ATTR_GRAF_CROP, SFX_ITEM_POOLABLE },          // RES_GRFATR_CROPGRF
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_ROTATION,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_LUMINANCE,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CONTRAST,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELR,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELG,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELB,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_GAMMA,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_INVERT,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_TRANSPARENCY,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY1,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY2,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY3,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY4,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY5,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY6,

    { 0, SFX_ITEM_POOLABLE },                           // RES_BOXATR_FORMAT
    { 0, 0 },                                           // RES_BOXATR_FORMULA,
    { 0, SFX_ITEM_POOLABLE },                           // RES_BOXATR_VALUE

    { 0, SFX_ITEM_POOLABLE }                            // RES_UNKNOWNATR_CONTAINER
};


USHORT* SwAttrPool::pVersionMap1 = 0;
USHORT* SwAttrPool::pVersionMap2 = 0;
USHORT* SwAttrPool::pVersionMap3 = 0;
USHORT* SwAttrPool::pVersionMap4 = 0;
// OD 2004-01-21 #i18732#
USHORT* SwAttrPool::pVersionMap5 = 0;
SwIndexReg* SwIndexReg::pEmptyIndexArray = 0;

const sal_Char* __FAR_DATA pMarkToTable     = "table";
const sal_Char* __FAR_DATA pMarkToFrame     = "frame";
const sal_Char* __FAR_DATA pMarkToRegion    = "region";
const sal_Char* __FAR_DATA pMarkToText      = "text";
const sal_Char* __FAR_DATA pMarkToOutline   = "outline";
const sal_Char* __FAR_DATA pMarkToGraphic   = "graphic";
const sal_Char* __FAR_DATA pMarkToOLE       = "ole";

SvPtrarr *pGlobalOLEExcludeList = 0;

SwAutoCompleteWord* SwDoc::pACmpltWords = 0;

SwBreakIt* pBreakIt = 0;
//CHINA001 add for swui to access sw global variable.
SwBreakIt* GetBreakIt()
{
    return pBreakIt;
}
//CHINA001 end of add
SwCheckIt* pCheckIt = 0;
CharClass* pAppCharClass = 0;
SwCalendarWrapper* pCalendarWrapper = 0;
CollatorWrapper* pCollator = 0, *pCaseCollator = 0;
::utl::TransliterationWrapper* pTransWrp = 0;

/******************************************************************************
 *  void _InitCore()
 ******************************************************************************/
class SwDontWrite : public SfxBoolItem
{
public:
    SwDontWrite( USHORT nId ) : SfxBoolItem( nId ) {}

    virtual USHORT  GetVersion( USHORT nFFVer ) const;
};

USHORT SwDontWrite::GetVersion( USHORT nFFVer ) const
{ return USHRT_MAX; }




void _InitCore()
{
    SfxPoolItem* pItem;

    // erstmal alle Attribut-Pointer auf 0 setzen
    memset( aAttrTab, 0, (POOLATTR_END - POOLATTR_BEGIN) *
                            sizeof( SfxPoolItem* ) );

    aAttrTab[ RES_CHRATR_CASEMAP- POOLATTR_BEGIN ] = new SvxCaseMapItem;
    aAttrTab[ RES_CHRATR_CHARSETCOLOR- POOLATTR_BEGIN ] = new SvxCharSetColorItem;
    aAttrTab[ RES_CHRATR_COLOR- POOLATTR_BEGIN ] = new SvxColorItem;
    aAttrTab[ RES_CHRATR_CONTOUR- POOLATTR_BEGIN ] = new SvxContourItem;
    aAttrTab[ RES_CHRATR_CROSSEDOUT- POOLATTR_BEGIN ] = new SvxCrossedOutItem;
    aAttrTab[ RES_CHRATR_ESCAPEMENT- POOLATTR_BEGIN ] = new SvxEscapementItem;
    aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ] =
                                        new SvxFontItem( RES_CHRATR_FONT );

    aAttrTab[ RES_CHRATR_FONTSIZE- POOLATTR_BEGIN ] = new SvxFontHeightItem;
    aAttrTab[ RES_CHRATR_KERNING- POOLATTR_BEGIN ] = new SvxKerningItem;
    aAttrTab[ RES_CHRATR_LANGUAGE- POOLATTR_BEGIN ] = new SvxLanguageItem(LANGUAGE_DONTKNOW);
    aAttrTab[ RES_CHRATR_POSTURE- POOLATTR_BEGIN ] = new SvxPostureItem;
    aAttrTab[ RES_CHRATR_PROPORTIONALFONTSIZE- POOLATTR_BEGIN ] = new SvxPropSizeItem;
    aAttrTab[ RES_CHRATR_SHADOWED- POOLATTR_BEGIN ] = new SvxShadowedItem;
    aAttrTab[ RES_CHRATR_UNDERLINE- POOLATTR_BEGIN ] = new SvxUnderlineItem;
    aAttrTab[ RES_CHRATR_WEIGHT- POOLATTR_BEGIN ] = new SvxWeightItem;
    aAttrTab[ RES_CHRATR_WORDLINEMODE- POOLATTR_BEGIN ] = new SvxWordLineModeItem;
    aAttrTab[ RES_CHRATR_AUTOKERN- POOLATTR_BEGIN ] = new SvxAutoKernItem;
    aAttrTab[ RES_CHRATR_BLINK - POOLATTR_BEGIN ]
                = new SvxBlinkItem( FALSE, RES_CHRATR_BLINK );
    aAttrTab[ RES_CHRATR_NOHYPHEN - POOLATTR_BEGIN ]
                = new SvxNoHyphenItem( TRUE, RES_CHRATR_NOHYPHEN );
    aAttrTab[ RES_CHRATR_NOLINEBREAK- POOLATTR_BEGIN ]
                = new SvxNoLinebreakItem( TRUE, RES_CHRATR_NOLINEBREAK );
    aAttrTab[ RES_CHRATR_BACKGROUND - POOLATTR_BEGIN ]
                = new SvxBrushItem( RES_CHRATR_BACKGROUND );

    // CJK-Attributes
    aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ] =
                                    new SvxFontItem( RES_CHRATR_CJK_FONT );

    pItem = new SvxFontHeightItem;
    pItem->SetWhich( RES_CHRATR_CJK_FONTSIZE );
    aAttrTab[ RES_CHRATR_CJK_FONTSIZE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxLanguageItem(LANGUAGE_DONTKNOW);
    pItem->SetWhich( RES_CHRATR_CJK_LANGUAGE );
    aAttrTab[ RES_CHRATR_CJK_LANGUAGE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxPostureItem;
    pItem->SetWhich( RES_CHRATR_CJK_POSTURE );
    aAttrTab[ RES_CHRATR_CJK_POSTURE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxWeightItem;
    pItem->SetWhich( RES_CHRATR_CJK_WEIGHT );
    aAttrTab[ RES_CHRATR_CJK_WEIGHT - POOLATTR_BEGIN ] = pItem;

    // CTL-Attributes
    aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] =
                                    new SvxFontItem( RES_CHRATR_CTL_FONT );

    pItem = new SvxFontHeightItem;
    pItem->SetWhich( RES_CHRATR_CTL_FONTSIZE );
    aAttrTab[ RES_CHRATR_CTL_FONTSIZE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxLanguageItem(LANGUAGE_DONTKNOW);
    pItem->SetWhich( RES_CHRATR_CTL_LANGUAGE );
    aAttrTab[ RES_CHRATR_CTL_LANGUAGE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxPostureItem;
    pItem->SetWhich( RES_CHRATR_CTL_POSTURE );
    aAttrTab[ RES_CHRATR_CTL_POSTURE - POOLATTR_BEGIN ] = pItem;

    pItem = new SvxWeightItem;
    pItem->SetWhich( RES_CHRATR_CTL_WEIGHT );
    aAttrTab[ RES_CHRATR_CTL_WEIGHT - POOLATTR_BEGIN ] = pItem;

    aAttrTab[ RES_CHRATR_ROTATE - POOLATTR_BEGIN ] = new SvxCharRotateItem;
    aAttrTab[ RES_CHRATR_EMPHASIS_MARK - POOLATTR_BEGIN ] =
                new SvxEmphasisMarkItem;
    aAttrTab[ RES_CHRATR_TWO_LINES - POOLATTR_BEGIN ] = new SvxTwoLinesItem( FALSE );
    aAttrTab[ RES_CHRATR_SCALEW - POOLATTR_BEGIN ] = new SvxCharScaleWidthItem;
    aAttrTab[ RES_CHRATR_RELIEF - POOLATTR_BEGIN ] = new SvxCharReliefItem;
    aAttrTab[ RES_CHRATR_HIDDEN - POOLATTR_BEGIN ] = new SvxCharHiddenItem( FALSE, RES_CHRATR_HIDDEN );

// CharakterAttr - Dummies
    //no dummy available
// CharakterAttr - Dummies

// TextAttr Ende - Dummies
    aAttrTab[ RES_TXTATR_DUMMY4 - POOLATTR_BEGIN ]
                = new SfxBoolItem( RES_TXTATR_DUMMY4 );
// TextAttr Ende - Dummies

    aAttrTab[ RES_TXTATR_INETFMT - POOLATTR_BEGIN ]
                = new SwFmtINetFmt( aEmptyStr, aEmptyStr );
    aAttrTab[ RES_TXTATR_REFMARK - POOLATTR_BEGIN ] = new SwFmtRefMark( aEmptyStr );
    aAttrTab[ RES_TXTATR_TOXMARK - POOLATTR_BEGIN ] = new SwTOXMark;
    aAttrTab[ RES_TXTATR_CHARFMT- POOLATTR_BEGIN ] = new SwFmtCharFmt( 0 );

    aAttrTab[ RES_TXTATR_CJK_RUBY - POOLATTR_BEGIN ] = new SwFmtRuby( aEmptyStr );
    aAttrTab[ RES_TXTATR_UNKNOWN_CONTAINER - POOLATTR_BEGIN ] =
                new SvXMLAttrContainerItem( RES_TXTATR_UNKNOWN_CONTAINER );

    aAttrTab[ RES_TXTATR_FIELD- POOLATTR_BEGIN ] = new SwFmtFld;
    aAttrTab[ RES_TXTATR_FLYCNT - POOLATTR_BEGIN ] = new SwFmtFlyCnt( 0 );
    aAttrTab[ RES_TXTATR_FTN - POOLATTR_BEGIN ] = new SwFmtFtn;
    aAttrTab[ RES_TXTATR_SOFTHYPH- POOLATTR_BEGIN ] = new SwFmtSoftHyph;
    aAttrTab[ RES_TXTATR_HARDBLANK- POOLATTR_BEGIN ] = new SwFmtHardBlank( ' ', FALSE );

// TextAttr ohne Ende - Dummies
    aAttrTab[ RES_TXTATR_DUMMY1 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY1 );
    aAttrTab[ RES_TXTATR_DUMMY2 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY2 );
    aAttrTab[ RES_TXTATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY5 );
    aAttrTab[ RES_TXTATR_DUMMY6 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY6 );
    aAttrTab[ RES_TXTATR_DUMMY7 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY7 );
// TextAttr ohne Ende - Dummies

    aAttrTab[ RES_PARATR_LINESPACING- POOLATTR_BEGIN ] = new SvxLineSpacingItem;
    aAttrTab[ RES_PARATR_ADJUST- POOLATTR_BEGIN ] = new SvxAdjustItem;
    aAttrTab[ RES_PARATR_SPLIT- POOLATTR_BEGIN ] = new SvxFmtSplitItem;
    aAttrTab[ RES_PARATR_WIDOWS- POOLATTR_BEGIN ] = new SvxWidowsItem;
    aAttrTab[ RES_PARATR_ORPHANS- POOLATTR_BEGIN ] = new SvxOrphansItem;
    aAttrTab[ RES_PARATR_TABSTOP- POOLATTR_BEGIN ] = new
                            SvxTabStopItem( 1, SVX_TAB_DEFDIST );

    pItem = new SvxHyphenZoneItem;
    ((SvxHyphenZoneItem*)pItem)->GetMaxHyphens() = 0; // Default z.Z. auf 0
    aAttrTab[ RES_PARATR_HYPHENZONE- POOLATTR_BEGIN ] = pItem;

    aAttrTab[ RES_PARATR_DROP- POOLATTR_BEGIN ] = new SwFmtDrop;
    aAttrTab[ RES_PARATR_REGISTER - POOLATTR_BEGIN ] = new SwRegisterItem( FALSE );
    aAttrTab[ RES_PARATR_NUMRULE - POOLATTR_BEGIN ] = new SwNumRuleItem( aEmptyStr );

    aAttrTab[ RES_PARATR_SCRIPTSPACE - POOLATTR_BEGIN ] =
                                        new SvxScriptSpaceItem( TRUE );
    aAttrTab[ RES_PARATR_HANGINGPUNCTUATION - POOLATTR_BEGIN ] =
                                        new SvxHangingPunctuationItem( TRUE );
    aAttrTab[ RES_PARATR_FORBIDDEN_RULES - POOLATTR_BEGIN ] =
                                        new SvxForbiddenRuleItem( TRUE );
    aAttrTab[ RES_PARATR_VERTALIGN - POOLATTR_BEGIN ] =
                            new SvxParaVertAlignItem( 0 );
    aAttrTab[ RES_PARATR_SNAPTOGRID - POOLATTR_BEGIN ] = new SvxParaGridItem;
    aAttrTab[ RES_PARATR_CONNECT_BORDER - POOLATTR_BEGIN ] = new SwParaConnectBorderItem;
// ParaAttr - Dummies
    aAttrTab[ RES_PARATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY5 );
    aAttrTab[ RES_PARATR_DUMMY6 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY6 );
    aAttrTab[ RES_PARATR_DUMMY7 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY7 );
    aAttrTab[ RES_PARATR_DUMMY8 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY8 );
// ParatAttr - Dummies

    aAttrTab[ RES_FILL_ORDER- POOLATTR_BEGIN ] = new SwFmtFillOrder;
    aAttrTab[ RES_FRM_SIZE- POOLATTR_BEGIN ] = new SwFmtFrmSize;
    aAttrTab[ RES_PAPER_BIN- POOLATTR_BEGIN ] = new SvxPaperBinItem;
    aAttrTab[ RES_LR_SPACE- POOLATTR_BEGIN ] = new SvxLRSpaceItem;
    aAttrTab[ RES_UL_SPACE- POOLATTR_BEGIN ] = new SvxULSpaceItem;
    aAttrTab[ RES_PAGEDESC- POOLATTR_BEGIN ] = new SwFmtPageDesc;
    aAttrTab[ RES_BREAK- POOLATTR_BEGIN ] = new SvxFmtBreakItem;
    aAttrTab[ RES_CNTNT- POOLATTR_BEGIN ] = new SwFmtCntnt;
    aAttrTab[ RES_HEADER- POOLATTR_BEGIN ] = new SwFmtHeader;
    aAttrTab[ RES_FOOTER- POOLATTR_BEGIN ] = new SwFmtFooter;
    aAttrTab[ RES_PRINT- POOLATTR_BEGIN ] = new SvxPrintItem;
    aAttrTab[ RES_OPAQUE- POOLATTR_BEGIN ] = new SvxOpaqueItem;
    aAttrTab[ RES_PROTECT- POOLATTR_BEGIN ] = new SvxProtectItem;
    aAttrTab[ RES_SURROUND- POOLATTR_BEGIN ] = new SwFmtSurround;
    aAttrTab[ RES_VERT_ORIENT- POOLATTR_BEGIN ] = new SwFmtVertOrient;
    aAttrTab[ RES_HORI_ORIENT- POOLATTR_BEGIN ] = new SwFmtHoriOrient;
    aAttrTab[ RES_ANCHOR- POOLATTR_BEGIN ] = new SwFmtAnchor;
    aAttrTab[ RES_BACKGROUND- POOLATTR_BEGIN ] = new SvxBrushItem;
    aAttrTab[ RES_BOX- POOLATTR_BEGIN ] = new SvxBoxItem;
    aAttrTab[ RES_SHADOW- POOLATTR_BEGIN ] = new SvxShadowItem;
    aAttrTab[ RES_FRMMACRO- POOLATTR_BEGIN ] = new SvxMacroItem;
    aAttrTab[ RES_COL- POOLATTR_BEGIN ] = new SwFmtCol;
    aAttrTab[ RES_KEEP - POOLATTR_BEGIN ] = new SvxFmtKeepItem( FALSE );
    aAttrTab[ RES_URL - POOLATTR_BEGIN ] = new SwFmtURL();
    aAttrTab[ RES_EDIT_IN_READONLY - POOLATTR_BEGIN ] = new SwFmtEditInReadonly;
    aAttrTab[ RES_LAYOUT_SPLIT - POOLATTR_BEGIN ] = new SwFmtLayoutSplit;
    aAttrTab[ RES_CHAIN - POOLATTR_BEGIN ] = new SwFmtChain;
    aAttrTab[ RES_TEXTGRID - POOLATTR_BEGIN ] = new SwTextGridItem;
    aAttrTab[ RES_HEADER_FOOTER_EAT_SPACING - POOLATTR_BEGIN ] = new SwHeaderAndFooterEatSpacingItem;
    aAttrTab[ RES_LINENUMBER - POOLATTR_BEGIN ] = new SwFmtLineNumber;
    aAttrTab[ RES_FTN_AT_TXTEND - POOLATTR_BEGIN ] = new SwFmtFtnAtTxtEnd;
    aAttrTab[ RES_END_AT_TXTEND - POOLATTR_BEGIN ] = new SwFmtEndAtTxtEnd;
    aAttrTab[ RES_COLUMNBALANCE - POOLATTR_BEGIN ] = new SwFmtNoBalancedColumns;
    aAttrTab[ RES_FRAMEDIR - POOLATTR_BEGIN ] = new SvxFrameDirectionItem(FRMDIR_ENVIRONMENT);
    aAttrTab[ RES_ROW_SPLIT - POOLATTR_BEGIN ] = new SwFmtRowSplit;

    // OD 18.09.2003 #i18732#
    aAttrTab[ RES_FOLLOW_TEXT_FLOW - POOLATTR_BEGIN ] = new SwFmtFollowTextFlow( TRUE );
    // --> collapsing borders FME 2005-05-27 #i29550#
    aAttrTab[ RES_COLLAPSING_BORDERS - POOLATTR_BEGIN ] = new SfxBoolItem( RES_COLLAPSING_BORDERS, FALSE );
    // <-- collapsing
    // OD 2004-05-04 #i28701#
    aAttrTab[ RES_WRAP_INFLUENCE_ON_OBJPOS - POOLATTR_BEGIN ] =
            new SwFmtWrapInfluenceOnObjPos( text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED );
// FrmAttr-Dummies
    aAttrTab[ RES_FRMATR_DUMMY3 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_FRMATR_DUMMY3 );
    aAttrTab[ RES_FRMATR_DUMMY4 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_FRMATR_DUMMY4 );
    aAttrTab[ RES_FRMATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_FRMATR_DUMMY5 );
// FrmAttr-Dummies

    aAttrTab[ RES_GRFATR_MIRRORGRF- POOLATTR_BEGIN ] = new SwMirrorGrf;
    aAttrTab[ RES_GRFATR_CROPGRF- POOLATTR_BEGIN ] = new SwCropGrf;
    aAttrTab[ RES_GRFATR_ROTATION - POOLATTR_BEGIN ] = new SwRotationGrf;
    aAttrTab[ RES_GRFATR_LUMINANCE - POOLATTR_BEGIN ] = new SwLuminanceGrf;
    aAttrTab[ RES_GRFATR_CONTRAST - POOLATTR_BEGIN ] = new SwContrastGrf;
    aAttrTab[ RES_GRFATR_CHANNELR - POOLATTR_BEGIN ] = new SwChannelRGrf;
    aAttrTab[ RES_GRFATR_CHANNELG - POOLATTR_BEGIN ] = new SwChannelGGrf;
    aAttrTab[ RES_GRFATR_CHANNELB - POOLATTR_BEGIN ] = new SwChannelBGrf;
    aAttrTab[ RES_GRFATR_GAMMA - POOLATTR_BEGIN ] = new SwGammaGrf;
    aAttrTab[ RES_GRFATR_INVERT - POOLATTR_BEGIN ] = new SwInvertGrf;
    aAttrTab[ RES_GRFATR_TRANSPARENCY - POOLATTR_BEGIN ] = new SwTransparencyGrf;
    aAttrTab[ RES_GRFATR_DRAWMODE - POOLATTR_BEGIN ] = new SwDrawModeGrf;

// GraphicAttr - Dummies
    aAttrTab[ RES_GRFATR_DUMMY1 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY1 );
    aAttrTab[ RES_GRFATR_DUMMY2 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY2 );
    aAttrTab[ RES_GRFATR_DUMMY3 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY3 );
    aAttrTab[ RES_GRFATR_DUMMY4 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY4 );
    aAttrTab[ RES_GRFATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY5 );
// GraphicAttr - Dummies

    aAttrTab[ RES_BOXATR_FORMAT- POOLATTR_BEGIN ] = new SwTblBoxNumFormat;
    aAttrTab[ RES_BOXATR_FORMULA- POOLATTR_BEGIN ] = new SwTblBoxFormula( aEmptyStr );
    aAttrTab[ RES_BOXATR_VALUE- POOLATTR_BEGIN ] = new SwTblBoxValue;

    aAttrTab[ RES_UNKNOWNATR_CONTAINER- POOLATTR_BEGIN ] =
                new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER );

    // get the correct fonts:
    ::GetDefaultFonts( *(SvxFontItem*)aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ],
                       *(SvxFontItem*)aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ],
                       *(SvxFontItem*)aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] );

    // 1. Version - neue Attribute:
    //      - RES_CHRATR_BLINK
    //      - RES_CHRATR_NOHYPHEN
    //      - RES_CHRATR_NOLINEBREAK
    //      - RES_PARATR_REGISTER
    //      + 2 Dummies fuer die einzelnen "Bereiche"
    SwAttrPool::pVersionMap1 = new USHORT[ 60 ];
    USHORT i;
    for( i = 1; i <= 17; i++ )
        SwAttrPool::pVersionMap1[ i-1 ] = i;
    for ( i = 18; i <= 27; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 5;
    for ( i = 28; i <= 35; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 7;
    for ( i = 36; i <= 58; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 10;
    for ( i = 59; i <= 60; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 12;

    // 2. Version - neue Attribute:
    //      10 Dummies fuer den Frame "Bereich"
    SwAttrPool::pVersionMap2 = new USHORT[ 75 ];
    for( i = 1; i <= 70; i++ )
        SwAttrPool::pVersionMap2[ i-1 ] = i;
    for ( i = 71; i <= 75; ++i )
        SwAttrPool::pVersionMap2[ i-1 ] = i + 10;

    // 3. Version - neue Attribute:
    //      neue Attribute und Dummies fuer die CJK-Version
    //      und neue Grafik-Attribute
    SwAttrPool::pVersionMap3 = new USHORT[ 86 ];
    for( i = 1; i <= 21; i++ )
        SwAttrPool::pVersionMap3[ i-1 ] = i;
    for ( i = 22; i <= 27; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 15;
    for ( i = 28; i <= 82; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 20;
    for ( i = 83; i <= 86; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 35;

    // 4. Version - neue Paragraph Attribute fuer die CJK-Version
    SwAttrPool::pVersionMap4 = new USHORT[ 121 ];
    for( i = 1; i <= 65; i++ )
        SwAttrPool::pVersionMap4[ i-1 ] = i;
    for ( i = 66; i <= 121; ++i )
        SwAttrPool::pVersionMap4[ i-1 ] = i + 9;

    // OD 2004-01-21 #i18732# - setup new version map due to extension of
    // the frame attributes (RES_FRMATR_*) for binary filters.
    SwAttrPool::pVersionMap5 = new USHORT[ 130 ];
    for( i = 1; i <= 109; i++ )
        SwAttrPool::pVersionMap5[ i-1 ] = i;
    for ( i = 110; i <= 130; ++i )
        SwAttrPool::pVersionMap5[ i-1 ] = i + 6;

    pBreakIt = new SwBreakIt;
    const ::com::sun::star::lang::Locale& rLcl = pBreakIt->GetLocale(
                                            (LanguageType)GetAppLanguage() );
    pCheckIt = NULL;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();
    pAppCharClass = new CharClass( xMSF, rLcl );
    pCalendarWrapper = new SwCalendarWrapper( xMSF );

    _FrmInit();
    _TextInit();

    SwSelPaintRects::pMapMode = new MapMode;
    SwFntObj::pPixMap = new MapMode;

    SwIndexReg::pEmptyIndexArray = new SwIndexReg;

    pGlobalOLEExcludeList = new SvPtrarr;

    const SvxSwAutoFmtFlags& rAFlags = SvxAutoCorrCfg::Get()->GetAutoCorrect()->GetSwFlags();
    SwDoc::pACmpltWords = new SwAutoCompleteWord( rAFlags.nAutoCmpltListLen,
                                            rAFlags.nAutoCmpltWordLen );
}

/******************************************************************************
 *  void _FinitCore()
 ******************************************************************************/



void _FinitCore()
{
    _FrmFinit();
    _TextFinit();

    delete pBreakIt;
    delete pCheckIt;
    delete pAppCharClass;
    delete pCalendarWrapper;
    delete pCollator;
    delete pCaseCollator;

    // das default TableAutoFormat zerstoeren
    delete SwTableAutoFmt::pDfltBoxAutoFmt;

    delete SwSelPaintRects::pMapMode;
    delete SwFntObj::pPixMap;

    delete SwEditShell::pAutoFmtFlags;
    delete SwNumRule::pDefBulletFont;

#ifndef PRODUCT
    //Defaultattribut freigeben lassen um asserts zu vermeiden.
    if ( aAttrTab[0]->GetRefCount() )
        SfxItemPool::ReleaseDefaults( aAttrTab, POOLATTR_END-POOLATTR_BEGIN, FALSE);
#endif
    delete SwDoc::pACmpltWords;

    delete SwStyleNameMapper::pTextUINameArray;
    delete SwStyleNameMapper::pListsUINameArray;
    delete SwStyleNameMapper::pExtraUINameArray;
    delete SwStyleNameMapper::pRegisterUINameArray;
    delete SwStyleNameMapper::pDocUINameArray;
    delete SwStyleNameMapper::pHTMLUINameArray;
    delete SwStyleNameMapper::pFrmFmtUINameArray;
    delete SwStyleNameMapper::pChrFmtUINameArray;
    delete SwStyleNameMapper::pHTMLChrFmtUINameArray;
    delete SwStyleNameMapper::pPageDescUINameArray;
    delete SwStyleNameMapper::pNumRuleUINameArray;

    // Delete programmatic name arrays also
    delete SwStyleNameMapper::pTextProgNameArray;
    delete SwStyleNameMapper::pListsProgNameArray;
    delete SwStyleNameMapper::pExtraProgNameArray;
    delete SwStyleNameMapper::pRegisterProgNameArray;
    delete SwStyleNameMapper::pDocProgNameArray;
    delete SwStyleNameMapper::pHTMLProgNameArray;
    delete SwStyleNameMapper::pFrmFmtProgNameArray;
    delete SwStyleNameMapper::pChrFmtProgNameArray;
    delete SwStyleNameMapper::pHTMLChrFmtProgNameArray;
    delete SwStyleNameMapper::pPageDescProgNameArray;
    delete SwStyleNameMapper::pNumRuleProgNameArray;

    // And finally, any hash tables that we used
    delete SwStyleNameMapper::pParaUIMap;
    delete SwStyleNameMapper::pCharUIMap;
    delete SwStyleNameMapper::pPageUIMap;
    delete SwStyleNameMapper::pFrameUIMap;
    delete SwStyleNameMapper::pNumRuleUIMap;

    delete SwStyleNameMapper::pParaProgMap;
    delete SwStyleNameMapper::pCharProgMap;
    delete SwStyleNameMapper::pPageProgMap;
    delete SwStyleNameMapper::pFrameProgMap;
    delete SwStyleNameMapper::pNumRuleProgMap;


    // loesche alle default-Attribute
    SfxPoolItem* pHt;
    for( USHORT n = 0; n < POOLATTR_END - POOLATTR_BEGIN; n++ )
        if( 0 != ( pHt = aAttrTab[n] ))
            delete pHt;

    ::ClearFEShellTabCols();

    delete SwIndexReg::pEmptyIndexArray;
    delete[] SwAttrPool::pVersionMap1;
    delete[] SwAttrPool::pVersionMap2;
    delete[] SwAttrPool::pVersionMap3;
    delete[] SwAttrPool::pVersionMap4;
    // OD 2004-01-21 #i18732#
    delete[] SwAttrPool::pVersionMap5;

    for ( USHORT i = 0; i < pGlobalOLEExcludeList->Count(); ++i )
        delete (SvGlobalName*)(*pGlobalOLEExcludeList)[i];
    delete pGlobalOLEExcludeList;
}

// returns the APP - CharClass instance - used for all ToUpper/ToLower/...
CharClass& GetAppCharClass()
{
    return *pAppCharClass;
}

LocaleDataWrapper& GetAppLocaleData()
{
    SvtSysLocale aSysLocale;
    return (LocaleDataWrapper&)aSysLocale.GetLocaleData();
}


void SwCalendarWrapper::LoadDefaultCalendar( USHORT eLang )
{
    sUniqueId.Erase();
    if( eLang != nLang )
        loadDefaultCalendar( SvxCreateLocale( nLang = eLang ));
}

void SwCalendarWrapper::LoadCalendar( USHORT eLang, const String& rUniqueId )
{
    if( eLang != nLang || sUniqueId != rUniqueId )
        loadCalendar( sUniqueId = rUniqueId,SvxCreateLocale( nLang = eLang ));
}

ULONG GetAppLanguage()
{
    return Application::GetSettings().GetLanguage();
}

CollatorWrapper& GetAppCollator()
{
    if( !pCollator )
    {
        const ::com::sun::star::lang::Locale& rLcl = pBreakIt->GetLocale(
                                            (LanguageType)GetAppLanguage() );
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();

        pCollator = new CollatorWrapper( xMSF );
        pCollator->loadDefaultCollator( rLcl, SW_COLLATOR_IGNORES );
    }
    return *pCollator;
}
CollatorWrapper& GetAppCaseCollator()
{
    if( !pCaseCollator )
    {
        const ::com::sun::star::lang::Locale& rLcl = pBreakIt->GetLocale(
                                            (LanguageType)GetAppLanguage() );
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();

        pCaseCollator = new CollatorWrapper( xMSF );
        pCaseCollator->loadDefaultCollator( rLcl, 0 );
    }
    return *pCaseCollator;
}

const ::utl::TransliterationWrapper& GetAppCmpStrIgnore()
{
    if( !pTransWrp )
    {
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();

        pTransWrp = new ::utl::TransliterationWrapper( xMSF,
                ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE |
                ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA |
                ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );
        pTransWrp->loadModuleIfNeeded( GetAppLanguage() );
    }
    return *pTransWrp;
}


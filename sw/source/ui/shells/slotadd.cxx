/*************************************************************************
 *
 *  $RCSfile: slotadd.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: os $ $Date: 2002-12-05 12:56:52 $
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
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif
#pragma hdrstop

#include "hintids.hxx"
#include "uiitems.hxx"
#include "uiparam.hxx"
#include "cmdid.h"

#ifndef _SVT_IMAGEITM_HXX
#include <svtools/imageitm.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _SVX_RULRITEM_HXX //autogen
#include <svx/rulritem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX //autogen
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_BOLNITEM_HXX //autogen
#include <svx/bolnitem.hxx>
#endif
#ifndef _SVX_PMDLITEM_HXX //autogen
#include <svx/pmdlitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX__XGRADIENT_HXX //autogen
#include <svx/xgrad.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX //autogen
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include <svx/xlnedit.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
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
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX //autogen
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX //autogen
#include <svx/xlnclit.hxx>
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
#ifndef _SVX_XFTSTIT_HXX //autogen
#include <svx/xftstit.hxx>
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
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
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

#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#pragma code_seg( "SWSTATICS" )
#endif

#define SvxDrawToolItem  SfxEnumItem
#define SvxDrawAlignItem SfxEnumItem
#define SvxDrawBezierItem SfxEnumItem

#define SFX_TYPEMAP
#include "itemdef.hxx"
#include "swslots.hxx"

#ifndef PROFILE
#pragma code_seg()
#endif




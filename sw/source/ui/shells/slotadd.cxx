/*************************************************************************
 *
 *  $RCSfile: slotadd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-10-23 10:51:17 $
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

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _SFX_DBCOLL_HXX //autogen
#include <offmgr/dbcoll.hxx>
#endif
#ifndef _SVX_RULRITEM_HXX //autogen
#include <svx/rulritem.hxx>
#endif
//#ifndef _IDETEMP_HXX //autogen
//#include <basctl/idetemp.hxx>
//#endif
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
#include "frmatr.hxx"
#include "cfgitems.hxx"
#include "grfatr.hxx"


#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#pragma code_seg( "SWSTATICS" )
#endif

#define SwShadowItem  SvxShadowItem
#define SwPageSizeItem  SvxSizeItem
#define SwGraphicSizeItem  SvxSizeItem
#define SvxDrawToolItem  SfxEnumItem
#define SvxDrawAlignItem SfxEnumItem
#define SvxDrawBezierItem SfxEnumItem
#define SwPageLRSpaceItem SvxLRSpaceItem
#define SwPageULSpaceItem SvxULSpaceItem

#define SFX_TYPEMAP
#include "itemdef.hxx"
#include "swslots.hxx"

#ifndef PROFILE
#pragma code_seg()
#endif

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.36  2000/09/18 16:06:05  willem.vandorp
    OpenOffice header added.

    Revision 1.35  2000/09/04 11:44:07  tbe
    basicide, isetbrw, si, vcdlged moved from svx to basctl

    Revision 1.34  2000/08/02 08:04:34  jp
    changes for graphic attributes

    Revision 1.33  2000/07/31 19:25:39  jp
    new attributes for CJK/CTL and graphic

    Revision 1.32  1999/09/24 14:38:32  os
    hlnkitem.hxx now in SVX

    Revision 1.31  1998/04/23 10:30:04  OS
    PageLeft/Right/Top/BottomMargin #49761#


      Rev 1.30   23 Apr 1998 12:30:04   OS
   PageLeft/Right/Top/BottomMargin #49761#

      Rev 1.29   29 Nov 1997 15:52:54   MA
   includes

      Rev 1.28   24 Nov 1997 09:47:06   MA
   includes

      Rev 1.27   03 Nov 1997 13:55:42   MA
   precomp entfernt

      Rev 1.26   15 Aug 1997 11:48:38   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.25   11 Aug 1997 10:06:46   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.24   05 Aug 1997 16:29:08   TJ
   include svx/srchitem.hxx

      Rev 1.23   11 Apr 1997 10:34:48   TRI
   includes

      Rev 1.22   07 Apr 1997 16:45:24   OM
   HyperlinkItem recorden

      Rev 1.21   28 Feb 1997 12:47:12   MH
   Segmentierug mit WorkingSetTuner nun moeglich

      Rev 1.20   18 Feb 1997 16:56:40   OM
   dbcoll.hxx eingetragen

      Rev 1.19   11 Dec 1996 20:12:26   NF
   SvxDrawBezierItem dazu...

      Rev 1.18   19 Sep 1996 14:07:24   JP
   eigenes Segment fuer statische Objecte

      Rev 1.17   28 Aug 1996 15:55:04   OS
   includes

      Rev 1.16   15 Aug 1996 18:17:16   OS
   URL als Attribut - auch fuer die API

      Rev 1.15   26 Mar 1996 10:07:22   OS
   neu: SvxGraphicSizeItem-dummy

      Rev 1.14   25 Mar 1996 13:35:36   OS
   dummy SwPageSizeItem definiert

      Rev 1.13   21 Mar 1996 11:41:56   MA
   Umstellung SV311

      Rev 1.12   07 Mar 1996 08:08:54   OS
   jetzt auch mit PCH

      Rev 1.11   26 Jan 1996 11:20:40   OS
   +include sfxshobj.hxx

      Rev 1.10   16 Jan 1996 16:26:56   OS
   neu: SwShadowItem(nur ueberdefiniertes SvxShadowItem)

      Rev 1.9   08 Jan 1996 15:14:08   OS
   +include frmatr.hxx

      Rev 1.8   22 Dec 1995 15:09:20   OS
   +include adritem.hxx

      Rev 1.7   07 Dec 1995 18:25:56   OM
   Define fuer Enum-Slot

      Rev 1.6   04 Dec 1995 16:09:40   OS
   includes geaendert

 -------------------------------------------------------------------------*/



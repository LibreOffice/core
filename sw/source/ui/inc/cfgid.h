/*************************************************************************
 *
 *  $RCSfile: cfgid.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:38 $
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
#ifndef _CFGID_H
#define _CFGID_H

#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif

#define CFG_DBDESCMGR_ITEM      SFX_ITEMTYPE_SW_BEGIN
#define CFG_USERPREF_ITEM       (SFX_ITEMTYPE_SW_BEGIN + 1)

#define CFG_COLOR_ITEM          (SFX_ITEMTYPE_SW_BEGIN + 3)
#define CFG_PRINT_ITEM          (SFX_ITEMTYPE_SW_BEGIN + 4)
#define CFG_AUTOCORR_ITEM       (SFX_ITEMTYPE_SW_BEGIN + 5)
#define CFG_AUTOFMT_ITEM        (SFX_ITEMTYPE_SW_BEGIN + 6)
#define CFG_NUMRULE_ITEM        (SFX_ITEMTYPE_SW_BEGIN + 7)
#define CFG_ENV_ITEM            (SFX_ITEMTYPE_SW_BEGIN + 8)
#define CFG_LAB_ITEM            (SFX_ITEMTYPE_SW_BEGIN + 9)
#define CFG_CHAPTER_ITEM        (SFX_ITEMTYPE_SW_BEGIN +10)
//#define CFG_BROWSEPREF_ITEM   (SFX_ITEMTYPE_SW_BEGIN +11) //UsrPrefs fuer BrowseView
#define CFG_STDFONT_ITEM        (SFX_ITEMTYPE_SW_BEGIN +12)
#define CFG_STATUSBAR           (SFX_ITEMTYPE_SW_BEGIN +13)
#define CFG_NAVIPI              (SFX_ITEMTYPE_SW_BEGIN +14)
#define CFG_SRCVIEW             (SFX_ITEMTYPE_SW_BEGIN +15)
#define CFG_SW_MENU             (SFX_ITEMTYPE_SW_BEGIN +16)
#define CFG_SW_ACCEL            (SFX_ITEMTYPE_SW_BEGIN +17)
#define CFG_SW_MODULE           (SFX_ITEMTYPE_SW_BEGIN +18)
#define CFG_SWWEB_MENU          (SFX_ITEMTYPE_SW_BEGIN +19)
#define CFG_SWWEB_ACCEL         (SFX_ITEMTYPE_SW_BEGIN +20)
#define CFG_SWWEB_USRPREF       (SFX_ITEMTYPE_SW_BEGIN +21)
#define CFG_WEBPRINT_ITEM       (SFX_ITEMTYPE_SW_BEGIN +22)
#define CFG_TOOLBARCONFIG_ITEM      (SFX_ITEMTYPE_SW_BEGIN +23)
#define CFG_WEBTOOLBARCONFIG_ITEM   (SFX_ITEMTYPE_SW_BEGIN +24)
#define CFG_INSERT_DBCOLUMN_ITEM    (SFX_ITEMTYPE_SW_BEGIN +25)
#define CFG_INSERT_HEADFOOT_ITEM    (SFX_ITEMTYPE_SW_BEGIN +26)
#define CFG_SW_MENU_PORTAL          (SFX_ITEMTYPE_SW_BEGIN +27)
#define CFG_SWWEB_MENU_PORTAL       (SFX_ITEMTYPE_SW_BEGIN +28)

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.25  2000/09/18 16:05:37  willem.vandorp
    OpenOffice header added.

    Revision 1.24  2000/08/07 14:02:28  os
    portal menu added

    Revision 1.23  1998/05/25 08:06:20  JP
    neue ConfigId fuers einfuegen von Standard Kopf-/Fusszeilen


      Rev 1.22   25 May 1998 10:06:20   JP
   neue ConfigId fuers einfuegen von Standard Kopf-/Fusszeilen

      Rev 1.21   20 May 1998 21:24:56   JP
   neue ConfigId fuer Datenbankspalten einfuegen Autopilot

      Rev 1.20   02 Apr 1998 10:05:00   MA
   new: ToolbarCfg

      Rev 1.19   28 Nov 1997 08:26:12   MA
   includes

      Rev 1.18   05 Feb 1997 13:35:44   OS
   PrintOptins auch fuer Web

      Rev 1.17   30 Jan 1997 11:13:36   OS
   Id fuer BrowseUsrPref

      Rev 1.16   24 Jan 1997 14:57:20   OS
   defines fuer WebMenue und -Accel

      Rev 1.15   13 Jan 1997 16:54:00   OS
   neu: SwModuleOptions

      Rev 1.14   18 Nov 1996 09:27:14   OS
   neue ConfigIds

      Rev 1.13   15 Nov 1996 10:52:58   OS
   SFX_ITEMTYPE_USER_BEGIN -> SFX_ITEMTYPE_SW_BEGIN

      Rev 1.12   13 Nov 1996 14:33:08   OS
   neues ConfigItem: SwNavigationConfig

      Rev 1.11   07 Nov 1996 16:18:56   OS
   neue Id fuer StatusBar

      Rev 1.10   03 Sep 1996 16:55:08   OS
   neu: ConfigItem fuer Standardfonts

      Rev 1.9   24 Apr 1996 15:04:04   OS
   Umstellung UsrPref/ViewOption

      Rev 1.8   03 Jan 1996 10:33:28   OS
   neue Id fuer Kapitelnumerierung

      Rev 1.7   20 Feb 1995 19:38:26   PK
   neu: efg_(env|lab)_item

      Rev 1.6   10 Jan 1995 18:53:26   ER
   #endif ohne Kommentar

      Rev 1.5   31 Oct 1994 11:16:34   MS
   ifndef angepasst

      Rev 1.4   06 Oct 1994 17:15:52   VB
   cfgid.h -> T:/sw/source/ui/inc/cfgid.h_v
   Enter description of changes made
   NumRules wieder scharf geschaltet.

      Rev 1.3   28 Sep 1994 14:14:24   MS
   Umstellung auf SfxConfigItem

      Rev 1.2   27 Sep 1994 10:47:38   MS

      Rev 1.1   19 Sep 1994 15:28:32   MS
   Fix fuers Update

      Rev 1.0   15 Sep 1994 16:41:46   MS
   Initial revision.
 -------------------------------------------------------------------------*/
#endif

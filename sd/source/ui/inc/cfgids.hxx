/*************************************************************************
 *
 *  $RCSfile: cfgids.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:37 $
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

#ifndef _SD_CFGID_HXX
#define _SD_CFGID_HXX

#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif

// Item-Ids fuer Config-Items
#define SDCFG_IMPRESS           (SFX_ITEMTYPE_SD_BEGIN + 1)
#define SDCFG_SPELL             (SFX_ITEMTYPE_SD_BEGIN + 2)
#define SDCFG_DRAW              (SFX_ITEMTYPE_SD_BEGIN + 3)

// Acceleratoren, Menus, ...
#define RID_DRAW_DEFAULTACCEL       (SFX_ITEMTYPE_SD_BEGIN + 5)
#define RID_DRAW_DEFAULTMENU        (SFX_ITEMTYPE_SD_BEGIN + 6)
#define RID_DRAW_PORTALMENU         (SFX_ITEMTYPE_SD_BEGIN + 35)
#define RID_DRAW_STATUSBAR          (SFX_ITEMTYPE_SD_BEGIN + 7)
// Acceleratoren, Menus, ... (fuers Draw (Graphic))
#define RID_GRAPHIC_DEFAULTACCEL    (SFX_ITEMTYPE_SD_BEGIN + 8)
#define RID_GRAPHIC_DEFAULTMENU     (SFX_ITEMTYPE_SD_BEGIN + 9)
#define RID_GRAPHIC_PORTALMENU      (SFX_ITEMTYPE_SD_BEGIN + 36)
//#define RID_GRAPHIC_STATUSBAR     (SFX_ITEMTYPE_SD_BEGIN + 30)

// Toolbars
#define RID_DRAW_DEFAULTTOOLBOX     (SFX_ITEMTYPE_SD_BEGIN + 10)
#define RID_DRAW_TOOLBOX            (SFX_ITEMTYPE_SD_BEGIN + 11)
#define RID_SLIDE_TOOLBOX           (SFX_ITEMTYPE_SD_BEGIN + 12)
#define RID_DRAW_OBJ_TOOLBOX        (SFX_ITEMTYPE_SD_BEGIN + 13)
#define RID_SLIDE_OBJ_TOOLBOX       (SFX_ITEMTYPE_SD_BEGIN + 14)
#define RID_BEZIER_TOOLBOX          (SFX_ITEMTYPE_SD_BEGIN + 15)
#define RID_DRAW_TEXT_TOOLBOX       (SFX_ITEMTYPE_SD_BEGIN + 16)
#define RID_OUTLINE_TOOLBOX         (SFX_ITEMTYPE_SD_BEGIN + 17)
//#define RID_DRAW_POLYLINEOBJ_POPUP    (SFX_ITEMTYPE_SD_BEGIN + 18)
#define RID_GLUEPOINTS_TOOLBOX      (SFX_ITEMTYPE_SD_BEGIN + 19)
#define RID_DRAW_OPTIONS_TOOLBOX    (SFX_ITEMTYPE_SD_BEGIN + 20)
#define RID_DRAW_COMMONTASK_TOOLBOX (SFX_ITEMTYPE_SD_BEGIN + 21)
#define RID_FORMLAYER_TOOLBOX       (SFX_ITEMTYPE_SD_BEGIN + 22)

#define RID_GRAPHIC_TOOLBOX         (SFX_ITEMTYPE_SD_BEGIN + 25)
#define RID_GRAPHIC_OPTIONS_TOOLBOX (SFX_ITEMTYPE_SD_BEGIN + 26)
#define RID_GRAPHIC_OBJ_TOOLBOX     (SFX_ITEMTYPE_SD_BEGIN + 27)
#define RID_GRAPHIC_TEXT_TOOLBOX    (SFX_ITEMTYPE_SD_BEGIN + 28)

#define RID_DRAW_GRAF_TOOLBOX       (SFX_ITEMTYPE_SD_BEGIN + 30)


#endif

/*************************************************************************
 *
 *  $RCSfile: rscsfx.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscsfx.hxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

**************************************************************************/

#ifndef _RSCSFX_HXX
#define _RSCSFX_HXX

// StarView     (RSC_NOTYPE) bis (RSC_NOTYPE + 0x190)
// Sfx          (RSC_NOTYPE + 0x200) bis (RSC_NOTYPE + 0x20F)
#define RSC_SFX_STYLE_FAMILIES     (0x100 + 0x201)
#define RSC_SFX_STYLE_FAMILY_ITEM  (0x100 + 0x202)
#define RSC_SFX_SLOT_INFO          (0x100 + 0x203)
// StarMoney    (RSC_NOTYPE + 0x210) bis (RSC_NOTYPE + 0x22F)
// Public       (RSC_NOTYPE + 0x300) bis (RSC_NOTYPE + 0x3FF)

//========== S F X =======================================
enum SfxStyleFamily { SFX_STYLE_FAMILY_CHAR    = 1,
                      SFX_STYLE_FAMILY_PARA    = 2,
                      SFX_STYLE_FAMILY_FRAME   = 4,
                      SFX_STYLE_FAMILY_PAGE    = 8,
                      SFX_STYLE_FAMILY_PSEUDO  = 16,
                      SFX_STYLE_FAMILY_ALL   = 0x7fff
                      };


// SfxTemplateDialog
#define RSC_SFX_STYLE_ITEM_LIST         0x1
#define RSC_SFX_STYLE_ITEM_BITMAP       0x2
#define RSC_SFX_STYLE_ITEM_TEXT         0x4
#define RSC_SFX_STYLE_ITEM_HELPTEXT     0x8
#define RSC_SFX_STYLE_ITEM_STYLEFAMILY  0x10
#define RSC_SFX_STYLE_ITEM_IMAGE        0x20


// SfxSlotInfo
#define RSC_SFX_SLOT_INFO_SLOTNAME      0x1
#define RSC_SFX_SLOT_INFO_HELPTEXT      0x2


#endif

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscsfx.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:34:01 $
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

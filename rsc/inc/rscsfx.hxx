/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

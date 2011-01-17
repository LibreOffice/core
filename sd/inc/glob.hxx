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

#ifndef SD_GLOB_HXX
#define SD_GLOB_HXX

#include <tools/solar.h>

#include <rsc/rscsfx.hxx>

//------------------------------------------------------------------

#define DIA_SLOW    0
#define DIA_MEDIUM  1
#define DIA_FAST    2

#define SD_IF_SDAPP                     SFX_INTERFACE_SD_START + 0
#define SD_IF_SDDRAWDOCSHELL            SFX_INTERFACE_SD_START + 1
#define SD_IF_SDVIEWSHELL               SFX_INTERFACE_SD_START + 2
#define SD_IF_SDDRAWVIEWSHELL           SFX_INTERFACE_SD_START + 3
#define SD_IF_SDOUTLINEVIEWSHELL        SFX_INTERFACE_SD_START + 5
#define SD_IF_SDDRAWSTDOBJECTBAR        SFX_INTERFACE_SD_START + 6
#define SD_IF_SDDRAWTEXTOBJECTBAR       SFX_INTERFACE_SD_START + 7
#define SD_IF_SDDRAWBEZIEROBJECTBAR     SFX_INTERFACE_SD_START + 8
#define SD_IF_SDDRAWGLUEPOINTSOBJECTBAR SFX_INTERFACE_SD_START + 9
#define SD_IF_SDGRAPHICDOCSHELL         SFX_INTERFACE_SD_START + 10
#define SD_IF_SDGRAPHICVIEWSHELL        SFX_INTERFACE_SD_START + 11
#define SD_IF_SDGRAPHICSTDOBJECTBAR     SFX_INTERFACE_SD_START + 12
#define SD_IF_SDDRAWGRAFOBJECTBAR       SFX_INTERFACE_SD_START + 13
#define SD_IF_SDPRESVIEWSHELL           SFX_INTERFACE_SD_START + 14
#define SD_IF_SDPREVIEWVIEWSHELL        SFX_INTERFACE_SD_START + 15
#define SD_IF_SDVIEWSHELLBASE           SFX_INTERFACE_SD_START + 16
#define SD_IF_SD3DOBJECTBAR             SFX_INTERFACE_SD_START + 17
#define SD_IF_SDFONTWORKOBJECTBAR       SFX_INTERFACE_SD_START + 18
#define SD_IF_SDSLIDESORTERVIEWSHELL    SFX_INTERFACE_SD_START + 19
#define SD_IF_SDTASKPANEVIEWSHELL       SFX_INTERFACE_SD_START + 20
#define SD_IF_SDMASTERPAGESSELECTOR     SFX_INTERFACE_SD_START + 21
#define SD_IF_SDLAYOUTMENU              SFX_INTERFACE_SD_START + 22
#define SD_IF_SDDRAWMEDIAOBJECTBAR      SFX_INTERFACE_SD_START + 23
#define SD_IF_SDLEFTIMPRESSPANESHELL    SFX_INTERFACE_SD_START + 24
#define SD_IF_SDLEFTDRAWPANESHELL       SFX_INTERFACE_SD_START + 25
#define SD_IF_SDRIGHTPANESHELL          SFX_INTERFACE_SD_START + 26
#define SD_IF_SDDRAWTABLEOBJECTBAR      SFX_INTERFACE_SD_START + 27
#define SD_IF_SDANNOTATIONSHELL         SFX_INTERFACE_SD_START + 28
#define SD_IF_SDTOOLPANELPANESHELL      SFX_INTERFACE_SD_START + 29
#define SD_IF_SDTOOLPANELSHELL          SFX_INTERFACE_SD_START + 30

// Inventor-Id fuer StarDraw UserData
const sal_uInt32 SdUDInventor=sal_uInt32('S')*0x00000001+
                          sal_uInt32('D')*0x00000100+
                          sal_uInt32('U')*0x00010000+
                          sal_uInt32('D')*0x01000000;

// Object-Ids fuer StarDraw UserData
#define SD_ANIMATIONINFO_ID 1
#define SD_IMAPINFO_ID      2

// FamilyId der Praesentationsvorlagen
#define SD_STYLE_FAMILY_GRAPHICS        SFX_STYLE_FAMILY_PARA
#define SD_STYLE_FAMILY_PSEUDO          SFX_STYLE_FAMILY_PSEUDO
#define SD_STYLE_FAMILY_CELL            SFX_STYLE_FAMILY_FRAME
#define SD_STYLE_FAMILY_MASTERPAGE      SFX_STYLE_FAMILY_PAGE       // ex LT_FAMILY

// Trennzeichen zwischen Layoutname und Vorlagenname der Praesentationsvorlagen
#define SD_LT_SEPARATOR "~LT~"

// Optionsstream-Identifier
#define SD_OPTION_MORPHING  "Morph"
#define SD_OPTION_VECTORIZE "Vectorize"

//------------------------------------------------------------------

#endif // _SD_GLOB_HXX



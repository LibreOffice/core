#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=canvas
TARGET=directxcanvas
TARGET2=directx9canvas
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Nothing to do if we're compiling with --disable-directx -----------
.IF "$(ENABLE_DIRECTX)" == ""
@all:
    @echo "Building without DirectX support..."
.ENDIF


# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
CDEFS+= -DDX_DEBUG_IMAGES
.ENDIF

.IF "$(CCNUMVER)" <= "001499999999"
CDEFS += -DDIRECTX_VERSION=0x0500
.ELSE
CDEFS += -DDIRECTX_VERSION=0x0900
.ENDIF

# --- This is Windows only! { ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

SLOFILES = \
    $(SLO)$/dx_5rm.obj					    \
    $(SLO)$/dx_9rm.obj					    \
    $(SLO)$/dx_bitmap.obj				    \
    $(SLO)$/dx_canvasbitmap.obj 			\
    $(SLO)$/dx_canvascustomsprite.obj 		\
    $(SLO)$/dx_canvasfont.obj 				\
    $(SLO)$/dx_canvashelper.obj			    \
    $(SLO)$/dx_canvashelper_texturefill.obj \
    $(SLO)$/dx_config.obj					\
    $(SLO)$/dx_devicehelper.obj				\
    $(SLO)$/dx_gdiplususer.obj				\
    $(SLO)$/dx_impltools.obj 				\
    $(SLO)$/dx_linepolypolygon.obj			\
    $(SLO)$/dx_spritecanvas.obj 			\
    $(SLO)$/dx_spritecanvashelper.obj		\
    $(SLO)$/dx_spritehelper.obj 			\
    $(SLO)$/dx_surfacegraphics.obj			\
    $(SLO)$/dx_textlayout.obj				\
    $(SLO)$/dx_vcltools.obj					\
    $(SLO)$/dx_textlayout_drawhelper.obj    \

.IF "$(CCNUMVER)" <= "001499999999"
########################################################
# DX5
########################################################

# Indicates the filename of the shared library.
SHL1TARGET=$(TARGET).uno

# Links import libraries.
SHL1STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(VCLLIB) $(TOOLSLIB) $(UNOTOOLSLIB)

# Specifies an import library to create. For Win32 only.
SHL1IMPLIB=i$(TARGET)

# Specifies libraries from the same module to put into the shared library.
SHL1LIBS=$(SLB)$/$(TARGET).lib



SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1STDLIBS += $(GDI32LIB)
SHL1STDLIBS += $(DDRAWLIB)
SHL1STDLIBS += $(GDIPLUSLIB)

.IF "$(USE_DIRECTX5)"!=""
.IF "$(COM)" == "GCC"
SHL1STDLIBS += $(DIRECTXSDK_LIB)/d3dx.lib
.ELSE
SHL1STDLIBS += d3dx.lib
.ENDIF
.ELSE # "$(USE_DIRECTX5)
.IF "$(COM)" == "GCC"
SHL1STDLIBS += $(DIRECTXSDK_LIB)/d3dx9.lib
SHL1STDLIBS += $(DIRECTXSDK_LIB)/dxguid.lib
.ELSE
SHL1STDLIBS += d3dx9.lib     # directx 9
SHL1STDLIBS += dxguid.lib    # directx 9
.ENDIF
.ENDIF # "$(USE_DIRECTX5)

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL1STDLIBS += imdebug.lib
.ENDIF
.ENDIF
########################################################
# DX9
########################################################

SECOND_BUILD=DX9
DX9_SLOFILES=$(SLOFILES)
DX9CDEFS+=-DDIRECTX_VERSION=0x0900

LIB1TARGET= $(SLB)$/$(TARGET)dx9.lib
LIB1OBJFILES = $(REAL_DX9_SLOFILES)

# Indicates the filename of the shared library.
SHL2TARGET=$(TARGET2).uno

# Links import libraries.
SHL2STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(VCLLIB) $(TOOLSLIB) $(UNOTOOLSLIB)

# Specifies an import library to create. For Win32 only.
SHL2IMPLIB=$(TARGET)dx9.lib

# Specifies libraries from the same module to put into the shared library.
SHL2LIBS=$(SLB)$/$(TARGET)dx9.lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=exports.dxp

SHL2STDLIBS += $(GDI32LIB)
.IF "$(COM)" == "GCC"
SHL2STDLIBS += $(DIRECTXSDK_LIB)/d3d9.lib
.ELSE
SHL2STDLIBS += d3d9.lib
.ENDIF
SHL2STDLIBS += $(GDIPLUSLIB)

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL2STDLIBS += imdebug.lib
.ENDIF

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL1STDLIBS += imdebug.lib
.ENDIF

.ENDIF # IF "$(GUI)" == "WNT"

# --- This is Windows only! } ----------------------------------------------------------------

# ==========================================================================

.INCLUDE :	target.mk


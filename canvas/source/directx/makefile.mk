#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
TARGET=directx9canvas
TARGET2=directx5canvas
TARGET3=gdipluscanvas
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

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

# --- This is Windows only! { ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

SHARED_SLOFILES = \
    $(SLO)$/dx_bitmap.obj			        \
    $(SLO)$/dx_bitmapcanvashelper.obj		\
    $(SLO)$/dx_canvasbitmap.obj 			\
    $(SLO)$/dx_canvasfont.obj 				\
    $(SLO)$/dx_canvashelper.obj			    \
    $(SLO)$/dx_canvashelper_texturefill.obj \
    $(SLO)$/dx_devicehelper.obj				\
    $(SLO)$/dx_gdiplususer.obj				\
    $(SLO)$/dx_impltools.obj 				\
    $(SLO)$/dx_linepolypolygon.obj			\
    $(SLO)$/dx_textlayout.obj				\
    $(SLO)$/dx_textlayout_drawhelper.obj    \
    $(SLO)$/dx_vcltools.obj

DX_SLOFILES = \
    $(SLO)$/dx_5rm.obj					    \
    $(SLO)$/dx_9rm.obj					    \
    $(SLO)$/dx_canvascustomsprite.obj 		\
    $(SLO)$/dx_config.obj					\
    $(SLO)$/dx_spritecanvas.obj 			\
    $(SLO)$/dx_spritecanvashelper.obj		\
    $(SLO)$/dx_spritedevicehelper.obj		\
    $(SLO)$/dx_spritehelper.obj 			\
    $(SLO)$/dx_surfacebitmap.obj		    \
    $(SLO)$/dx_surfacegraphics.obj
DX_SLOFILES += $(SHARED_SLOFILES)

GDIPLUS_SLOFILES = \
    $(SLO)$/dx_canvas.obj
GDIPLUS_SLOFILES += $(SHARED_SLOFILES)

STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(VCLLIB) $(TOOLSLIB) $(UNOTOOLSLIB) $(I18NISOLANGLIB)


########################################################
# DX9
########################################################

# Indicates the source obj files for the dx5 lib
LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES = $(DX_SLOFILES)

# Indicates the filename of the shared library.
SHL1TARGET=$(TARGET).uno

# Links import libraries.
SHL1STDLIBS= $(STDLIBS)

# Specifies an import library to create. For Win32 only.
SHL1IMPLIB=i$(TARGET)

# Specifies libraries from the same module to put into the shared library.
SHL1LIBS=$(SLB)$/$(TARGET).lib

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

CDEFS+=-DDIRECTX_VERSION=0x0900

SHL1STDLIBS += $(GDI32LIB)
.IF "$(COM)" == "GCC"
SHL1STDLIBS += $(DIRECTXSDK_LIB)/d3d9.lib
.ELSE
SHL1STDLIBS += d3d9.lib
.ENDIF
SHL1STDLIBS += $(GDIPLUSLIB)

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL1STDLIBS += imdebug.lib
.ENDIF


########################################################
# DX5
########################################################

.IF "$(USE_DIRECTX5)" != ""
SECOND_BUILD=DX5
DX5_SLOFILES=$(DX_SLOFILES)
DX5CDEFS += -DDIRECTX_VERSION=0x0500

LIB2TARGET= $(SLB)$/$(TARGET2).lib
LIB2OBJFILES = $(REAL_DX5_SLOFILES)

# Indicates the filename of the shared library.
SHL2TARGET=$(TARGET2).uno

# Links import libraries.
SHL2STDLIBS= $(STDLIBS)

# Specifies an import library to create. For Win32 only.
SHL2IMPLIB=i$(TARGET2).lib

# Specifies libraries from the same module to put into the shared library.
SHL2LIBS=$(SLB)$/$(TARGET2).lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=exports.dxp

SHL2STDLIBS += $(GDI32LIB)
SHL2STDLIBS += $(DDRAWLIB)
SHL2STDLIBS += $(GDIPLUSLIB)

.IF "$(COM)" == "GCC"
SHL2STDLIBS += $(DIRECTXSDK_LIB)/d3dx.lib
.ELSE
SHL2STDLIBS += d3dx.lib
.ENDIF

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL2STDLIBS += imdebug.lib
.ENDIF
.ENDIF # IF "$(USE_DIRECTX5)" != ""


########################################################
# GDI+
########################################################

LIB3TARGET= $(SLB)$/$(TARGET3).lib
LIB3OBJFILES = $(GDIPLUS_SLOFILES)

# Indicates the filename of the shared library.
SHL3TARGET=$(TARGET3).uno

# Links import libraries.
SHL3STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(VCLLIB) $(TOOLSLIB) $(UNOTOOLSLIB) $(I18NISOLANGLIB)

# Specifies an import library to create. For Win32 only.
SHL3IMPLIB=i$(TARGET3).lib

# Specifies libraries from the same module to put into the shared library.
SHL3LIBS=$(SLB)$/$(TARGET3).lib
SHL3DEF=$(MISC)$/$(SHL3TARGET).def

DEF3NAME=$(SHL3TARGET)
DEF3EXPORTFILE=exports.dxp

SHL3STDLIBS += $(GDI32LIB)
SHL3STDLIBS += $(GDIPLUSLIB)

.IF "$(dx_debug_images)"!="" || "$(DX_DEBUG_IMAGES)"!=""
SHL3STDLIBS += imdebug.lib
.ENDIF

.ENDIF # IF "$(GUI)" == "WNT"

# --- This is Windows only! } ----------------------------------------------------------------

# ==========================================================================

.INCLUDE :	target.mk

ALLTAR : \
    $(MISC)/directx5canvas.component \
    $(MISC)/directx9canvas.component \
    $(MISC)/gdipluscanvas.component

$(MISC)/directx5canvas.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt directx5canvas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt directx5canvas.component

$(MISC)/directx9canvas.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt directx9canvas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt directx9canvas.component

$(MISC)/gdipluscanvas.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt gdipluscanvas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL3TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt gdipluscanvas.component

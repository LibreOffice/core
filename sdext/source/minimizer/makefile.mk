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
# $Revision: 1.13 $
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
PRJNAME=sdext
TARGET=SunPresentationMinimizer
GEN_HID=FALSE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE : settings.mk
.IF "$(L10N_framework)"==""
.INCLUDE :  $(PRJ)$/source$/minimizer$/minimizer.pmk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# set in minimizer.pmk
#EXTENSIONNAME:=SunPresentationMinimizer
EXTENSION_ZIPNAME:=sun-presentation-minimizer

.IF "$(ENABLE_MINIMIZER)" != "YES"
@all:
    @echo "Presentation Minimizer build disabled."
.ENDIF

DLLPRE=

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/unodialog.obj				\
            $(SLO)$/optimizerdialog.obj			\
            $(SLO)$/optimizerdialogcontrols.obj	\
            $(SLO)$/configurationaccess.obj		\
            $(SLO)$/impoptimizer.obj			\
            $(SLO)$/pppoptimizer.obj			\
            $(SLO)$/pppoptimizeruno.obj			\
            $(SLO)$/pppoptimizertoken.obj		\
            $(SLO)$/pppoptimizerdialog.obj		\
            $(SLO)$/fileopendialog.obj			\
            $(SLO)$/optimizationstats.obj		\
            $(SLO)$/aboutdialog.obj				\
            $(SLO)$/graphiccollector.obj		\
            $(SLO)$/pagecollector.obj			\
            $(SLO)$/informationdialog.obj

# --- Library -----------------------------------

SHL1TARGET=		$(TARGET).uno

SHL1STDLIBS=	$(CPPUHELPERLIB)	\
                $(CPPULIB)			\
                $(SALLIB)
SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=	exports.map
SHL1RPATH=      OXT
DEF1NAME=		$(SHL1TARGET)

COMPONENT_MERGED_XCU= \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Addons.xcu \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcu \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/ImpressWindowState.xcu

COMPONENT_XCU= \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ProtocolHandler.xcu

COMPONENT_FILES= \
    $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcs

# native libraries
COMPONENT_LIBRARIES= \
    $(EXTENSIONDIR)$/SunPresentationMinimizer.uno$(DLLPOST)

# rather freestyle or common to all?
COMPONENT_BITMAPS= \
    $(EXTENSIONDIR)$/bitmaps$/aboutlogo.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_16.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_26.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_16_h.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_26_h.png \
    $(EXTENSIONDIR)$/bitmaps$/minimizepresi_80.png \
    $(EXTENSIONDIR)$/bitmaps$/minimizepresi_80_h.png

# rather freestyle or common to all?
COMPONENT_HELP= \
    $(EXTENSIONDIR)$/help$/help_de.odt \
    $(EXTENSIONDIR)$/help$/help_en-US.odt \
    $(EXTENSIONDIR)$/help$/component.txt

# make sure to add your custom files here
EXTENSION_PACKDEPS=$(COMPONENT_BITMAPS) $(COMPONENT_HELP)

# --- Targets ----------------------------------

.INCLUDE : extension_pre.mk

.ENDIF # L10N_framework
.INCLUDE : target.mk

.IF "$(L10N_framework)"==""
.INCLUDE : extension_post.mk

$(COMPONENT_BITMAPS) : $(SOLARSRC)$/$(RSCDEFIMG)$/minimizer$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_HELP) : help$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.ENDIF # L10N_framework

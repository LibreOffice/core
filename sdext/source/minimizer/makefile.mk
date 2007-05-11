#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: sj $ $Date: 2007-05-11 13:56:50 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=customres
TARGET=SunPresentationMinimizer
GEN_HID=FALSE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

DLLPRE=
common_build_zip=

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
DEF1NAME=		$(SHL1TARGET)

COMPONENT_FILES= \
    $(MISC)$/SunPresentationMinimizer$/registration$/license_en-US.txt \
    $(MISC)$/SunPresentationMinimizer$/registration$/license_de-DE.txt \
    $(MISC)$/SunPresentationMinimizer$/configuration$/SunPresentationMinimizer.xcs \
    $(MISC)$/SunPresentationMinimizer$/configuration$/SunPresentationMinimizer.xcu \
    $(MISC)$/SunPresentationMinimizer$/configuration$/Addons.xcu \
    $(MISC)$/SunPresentationMinimizer$/configuration$/ImpressWindowState.xcu \
    $(MISC)$/SunPresentationMinimizer$/configuration$/ProtocolHandler.xcu \
    $(MISC)$/SunPresentationMinimizer$/description.xml

COMPONENT_BITMAPS= \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/aboutlogo.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_16.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_26.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_16_h.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_26_h.png

COMPONENT_HELP= \
    $(MISC)$/SunPresentationMinimizer$/help$/help_de.odt \
    $(MISC)$/SunPresentationMinimizer$/help$/help_en-us.odt
    
COMPONENT_MANIFEST= \
    $(MISC)$/SunPresentationMinimizer$/META-INF$/manifest.xml

COMPONENT_LIBRARY= \
    $(MISC)$/SunPresentationMinimizer$/SunPresentationMinimizer.uno$(DLLPOST)

ZIP1DEPN=		$(COMPONENT_MANIFEST) $(COMPONENT_FILES) $(COMPONENT_BITMAPS) $(COMPONENT_HELP) $(COMPONENT_LIBRARY)
ZIP1TARGET=		sun-presentation-minimizer
ZIP1DIR=		$(MISC)$/SunPresentationMinimizer
ZIP1EXT=		.oxt
ZIP1FLAGS=-r
ZIP1LIST=		*

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(COMPONENT_MANIFEST) : $$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(TYPE) $< | $(SED) "s/SHARED_EXTENSION/$(DLLPOST)/" > $@

$(COMPONENT_FILES) : $$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

$(COMPONENT_BITMAPS) : bitmaps$/$$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

$(COMPONENT_HELP) : help$/$$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@
 
$(COMPONENT_LIBRARY) : $(DLLDEST)$/$$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

# remove this once ZIPnDEPN exists
$(ZIP1TARGETN) : $(COMPONENT_MANIFEST) $(COMPONENT_FILES) $(COMPONENT_BITMAPS) $(COMPONENT_HELP) $(COMPONENT_LIBRARY)

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:54:03 $
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

PRJ=..$/..
PRJNAME=sdext
TARGET=SunPresentationMinimizer
GEN_HID=FALSE
EXTNAME=minimi

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_MINIMIZER)" == "NO"
@all:
    @echo "Presentation Minimizer build disabled."
.ELSE

DESCRIPTION:=$(MISC)$/SunPresentationMinimizer$/description.xml

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/SunPresentationMinimizer$/registry$/license_$i)
.ELSE
PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/SunPresentationMinimizer$/registry$/LICENSE_$i)
.ENDIF

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
SHL1RPATH=      OXT
DEF1NAME=		$(SHL1TARGET)

COMPONENT_MERGED_XCU= \
    $(MISC)$/SunPresentationMinimizer$/registry$/data$/org$/openoffice$/Office$/Addons.xcu \
    $(MISC)$/SunPresentationMinimizer$/registry$/data$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcu \
    $(MISC)$/SunPresentationMinimizer$/registry$/data$/org$/openoffice$/Office$/UI$/ImpressWindowState.xcu \

COMPONENT_FILES= \
    $(MISC)$/SunPresentationMinimizer$/registry$/schema$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcs \
    $(MISC)$/SunPresentationMinimizer$/registry$/data$/org$/openoffice$/Office$/ProtocolHandler.xcu

COMPONENT_BITMAPS= \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/aboutlogo.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_16.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_26.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_16_h.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/opt_26_h.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/minimizepresi_80.png \
    $(MISC)$/SunPresentationMinimizer$/bitmaps$/minimizepresi_80_h.png

COMPONENT_HELP= \
    $(MISC)$/SunPresentationMinimizer$/help$/help_de.odt \
    $(MISC)$/SunPresentationMinimizer$/help$/help_en-US.odt
    
COMPONENT_MANIFEST= \
    $(MISC)$/SunPresentationMinimizer$/META-INF$/manifest.xml

COMPONENT_LIBRARY= \
    $(MISC)$/SunPresentationMinimizer$/SunPresentationMinimizer.uno$(DLLPOST)

ZIP1DEPS=		$(PACKLICS) $(DESCRIPTION) $(COMPONENT_MANIFEST) $(COMPONENT_FILES) $(COMPONENT_BITMAPS) $(COMPONENT_HELP) $(COMPONENT_LIBRARY) $(COMPONENT_MERGED_XCU)
ZIP1TARGET=		sun-presentation-minimizer
ZIP1DIR=		$(MISC)$/SunPresentationMinimizer
ZIP1EXT=		.oxt
ZIP1FLAGS=-r
ZIP1LIST=		*

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(COMPONENT_MANIFEST) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(TYPE) $< | $(SED) "s/SHARED_EXTENSION/$(DLLPOST)/" > $@

$(COMPONENT_BITMAPS) : $(SOLARSRC)$/$(RSCDEFIMG)$/minimizer$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_HELP) : help$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_LIBRARY) : $(DLLDEST)$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
$(PACKLICS) : $(SOLARBINDIR)$/osl$/license$$(@:b:s/_/./:e:s/./_/)$$(@:e).txt
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ELSE
$(PACKLICS) : $(SOLARBINDIR)$/osl$/LICENSE$$(@:b:s/_/./:e:s/./_/)$$(@:e)
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ENDIF

$(MISC)$/SunPresentationMinimizer$/registry$/data$/%.xcu : $(MISC)$/$(EXTNAME)$/merge$/%.xcu
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@

$(MISC)$/SunPresentationMinimizer$/%.xcu : %.xcu
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@

$(MISC)$/SunPresentationMinimizer$/%.xcs : %.xcs
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@

.INCLUDE .IGNORE : $(MISC)$/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
$(DESCRIPTION) $(PHONYDESC) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(PERL) $(SOLARENV)$/bin$/licinserter.pl description.xml registry/LICENSE_xxx $@
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)$/$(TARGET)_lang_track.mk

.ENDIF #  "$(ENABLE_MINIMIZER)" != "YES"

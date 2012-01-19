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

PRJNAME=sdext
TARGET=pdfimport
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

MKDEPENDSOLVER:=
.INCLUDE: settings.mk

.IF "$(L10N_framework)"==""

.INCLUDE: pdfisettings.pmk
INCPRE=-I$(PRJ)$/source$/pdfimport$/inc

# --- Files --------------------------------------------------------

EXTENSIONNAME:=PDFImport
EXTENSION_ZIPNAME:=pdfimport

.IF "$(ENABLE_PDFIMPORT)" == "NO"
@all:
    @echo "PDF Import extension disabled."
.ENDIF

SLOFILES=\
    $(SLO)$/filterdet.obj   \
    $(SLO)$/pdfiadaptor.obj \
    $(SLO)$/services.obj

SHL1DLLPRE=
SHL1TARGET=$(ENFORCEDSHLPREFIX)$(TARGET).uno

SHL1LIBS=\
    $(SLB)$/pdfmisc.lib \
    $(SLB)$/pdfparse.lib \
    $(SLB)$/pdfsax.lib \
    $(SLB)$/pdfparsetree.lib \
    $(SLB)$/pdfodf.lib \
    $(SLB)$/xpdfwrapper.lib \
    $(SLB)$/$(TARGET).lib

SHL1STDLIBS=\
    $(CPPUHELPERLIB)	\
    $(ZLIB3RDLIB)	    \
    $(CPPULIB)			\
    $(SALLIB)

.IF "$(GUI)" == "UNX"
    SHL1STDLIBS+=-lbasegfx_s
.ELSE
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
    SHL1STDLIBS+=-lbasegfx_s
.ELSE
    SHL1LIBS+=$(SOLARLIBDIR)$/basegfx_s.lib
.ENDIF

.ENDIF # WNT
.ENDIF # UNX

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
.IF "$(GUI)"=="WNT"
SHL1VERSIONMAP=$(SOLARENV)/src/reg-component.map
.ELSE # just a quick hack for GCC fdo#42865
SHL1USE_EXPORTS = name
.ENDIF
SHL1RPATH=OXT

DEF1NAME=$(SHL1TARGET)

# --- Extension packaging ------------------------------------------

DESCRIPTION_SRC:=$(MISC)/$(EXTENSIONNAME)_in/description.xml
MANIFEST_SRC:=config$/manifest.xml
COMPONENT_CONFIGDIR:=config
COMPONENT_CONFIGDEST:=.
COMPONENT_XCU= \
    $(EXTENSIONDIR)$/pdf_import_filter.xcu \
    $(EXTENSIONDIR)$/pdf_types.xcu

CONVERTER_FILE= \
    $(EXTENSIONDIR)$/xpdfimport$(EXECPOST)

COMPONENT_DIALOGS= \
    $(EXTENSIONDIR)$/basic$/Module1.xba \
    $(EXTENSIONDIR)$/basic$/TargetChooser.xdl \
    $(EXTENSIONDIR)$/basic$/dialog.xlb \
    $(EXTENSIONDIR)$/basic$/impress.png \
    $(EXTENSIONDIR)$/basic$/script.xlb \
    $(EXTENSIONDIR)$/basic$/writer.png \
    $(EXTENSIONDIR)$/xpdfimport_err.pdf

COMPONENT_DESCRIPTION= \
    $(foreach,lang,$(alllangiso) $(EXTENSIONDIR)$/description-$(lang).txt)

# native libraries
COMPONENT_LIBRARIES= \
    $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)

COMPONENT_IMAGES=\
    $(EXTENSIONDIR)$/images$/extension_32.png

EXTENSION_PACKDEPS=$(CONVERTER_FILE) $(COMPONENT_DIALOGS) $(COMPONENT_DESCRIPTION) $(COMPONENT_IMAGES) makefile.mk

.INCLUDE : extension_pre.mk
.ENDIF # L10N_framework
.INCLUDE : target.mk
.IF "$(L10N_framework)"==""
.INCLUDE : extension_post.mk

$(DESCRIPTION_SRC) : description.xml
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)" != ""
    $(COMMAND_ECHO)$(XRMEX) -p $(PRJNAME) -i $< -o $@ -m $(LOCALIZESDF) -l all
.ELSE
    $(COPY) $< $@
.ENDIF

$(CONVERTER_FILE) : $(BIN)$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_DIALOGS) : dialogs$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_DESCRIPTION) : $(MISC)/descriptions

$(MISC)/descriptions : $(DESCRIPTION)
    $(COPY) description-en-US.txt $(EXTENSIONDIR)
.IF "$(WITH_LANG)" != ""
    $(COPY) $(MISC)/$(EXTENSIONNAME)_in/description-*.txt $(EXTENSIONDIR)
.ENDIF
    $(TOUCH) $@

$(COMPONENT_IMAGES) :  $(SRC_ROOT)/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ENDIF # L10N_framework


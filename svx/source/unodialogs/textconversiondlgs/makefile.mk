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
PRJ=..$/..$/..

PRJNAME=svx
TARGET=textconversiondlgs
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

LOCALIZE_ME=chinese_direction_tmpl.hrc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=           \
                    $(SLO)$/services.obj\
                    $(SLO)$/resid.obj\
                    $(SLO)$/chinese_dictionarydialog.obj\
                    $(SLO)$/chinese_translationdialog.obj\
                    $(SLO)$/chinese_translation_unodialog.obj

SRS1NAME=$(TARGET)
SRC1FILES=          \
                    chinese_translationdialog.src\
                    chinese_dictionarydialog.src

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

.IF "$(GUI)" == "OS2"
SHL1TARGET= txcnvdlg
.ELSE
SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
.ENDIF
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(CPPULIB)  \
        $(SALLIB)   \
        $(CPPUHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB)  \
        $(TKLIB)	 \
        $(SVTOOLLIB) \
        $(SVLLIB) \
        $(VCLLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)


# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk

$(SRS)$/$(TARGET).srs: $(INCCOM)$/chinese_direction.hrc
$(SLO)$/chinese_dictionarydialog.obj: $(INCCOM)$/chinese_direction.hrc
$(SLO)$/chinese_translationdialog.obj: $(INCCOM)$/chinese_direction.hrc


ALLTAR : $(MISC)/textconversiondlgs.component

$(MISC)/textconversiondlgs.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt textconversiondlgs.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt textconversiondlgs.component

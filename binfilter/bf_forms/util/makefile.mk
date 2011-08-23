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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=bf_frm

NO_HIDS=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk
INC+= -I$(PRJ)$/inc$/bf_forms
LDUMP=ldump2.exe

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/bf_forms.lib
LIB1FILES=\
        $(SLB)$/forms_common.lib \
        $(SLB)$/forms_resource.lib \
        $(SLB)$/forms_component.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(SALLIB) \
        $(OSLLIB) \
        $(ONELIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(VCLLIB) \
        $(BFSVTOOLLIB) \
        $(BFSVXLIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(DBTOOLSLIB) \
        $(TKLIB) \
        $(BFSO3LIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEPN=	$(LIB1TARGET)	\
        makefile.mk


SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

# === .res file ==========================================================

RES1FILELIST=\
    $(SRS)$/forms_resource.srs \

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE			> $@
    @echo OControl					>> $@
    @echo OBoundControl				>> $@
    @echo OCombo					>> $@
    @echo OList						>> $@
    @echo OFixedText				>> $@
    @echo OCheckBox					>> $@
    @echo OGroupBox					>> $@
    @echo RadioButton				>> $@
    @echo OHidden					>> $@
    @echo OEdit						>> $@
    @echo OEditBase					>> $@
    @echo ONumeric					>> $@
    @echo OPattern					>> $@
    @echo OCurrency					>> $@
    @echo ODate						>> $@
    @echo OTime						>> $@
    @echo OFile						>> $@
    @echo OFormatted				>> $@
    @echo OComponent				>> $@
    @echo OButton					>> $@
    @echo OImage					>> $@
    @echo OInterfaceContainer		>> $@
    @echo OFormsCollection			>> $@
    @echo OGroup					>> $@
    @echo HtmlSuccess				>> $@
    @echo OSuccess					>> $@
    @echo OParameter				>> $@
    @echo ODatabaseForm				>> $@
    @echo OFormComponents			>> $@
    @echo OFormSubmitResetThread	>> $@
    @echo OGrid						>> $@
    @echo FieldColumn				>> $@
    @echo BoxColumn					>> $@
    @echo StandardFormatsSupplier	>> $@
    @echo OGuard					>> $@
    @echo OPropertyChange			>> $@
    @echo OEnumeration				>> $@
    @echo Weak						>> $@
    @echo OUString					>> $@
    @echo Any@uno@star@sun@com		>> $@
    @echo _C						>> $@
    @echo _TI2						>> $@
    @echo _real						>> $@



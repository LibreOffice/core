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
PRJNAME=odk
TARGET=checkinc

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
.IF "$(L10N_framework)"==""
ODKCHECKFILE=$(MISC)$/$(TARGET).txt

all : $(ODKCHECKFILE)

$(ODKCHECKFILE) : $(SDK_CONTENT_CHECK_FILES)
    @echo "" > $(ODKCHECKFILE)
# THE PERL SCRIPT DELETES THE CHECK FILE, WHEN AN ERROR OCCURS
    -diff -br $(DESTDIRINC)$/osl   $(SOLARINCDIR)$/osl $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/rtl   $(SOLARINCDIR)$/rtl $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/sal   $(SOLARINCDIR)$/sal $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/store $(SOLARINCDIR)$/store $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)
.ELSE
pseudo:

.ENDIF

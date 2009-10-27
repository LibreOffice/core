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
# $Revision: 1.11 $
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
TARGET=checkinc2

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
    -diff -br $(DESTDIRINC)$/typelib  $(SOLARINCDIR)$/typelib 	$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/uno   	$(SOLARINCDIR)$/uno 		$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/bridges  $(SOLARINCDIR)$/bridges 	$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/cppu   	$(SOLARINCDIR)$/cppu 		$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/com$/sun$/star$/uno   	$(SOLARINCDIR)$/com$/sun$/star$/uno 	$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/cppuhelper 		$(SOLARINCDIR)$/cppuhelper 		$(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)
.ELSE
pseudo:

.ENDIF

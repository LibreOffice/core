#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:28:24 $
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
PRJNAME=odk
TARGET=checkinc

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=$(MISC)$/$(TARGET).txt

all : $(ODKCHECKFILE)

$(ODKCHECKFILE) : $(SDK_CONTENT_CHECK_FILES)
    @echo "" > $(ODKCHECKFILE)
# THE PERL SCRIPT DELETES THE CHECK FILE, WHEN AN ERROR OCCURS
    -diff -br $(DESTDIRINC)$/osl   $(SOLARINCDIR)$/osl $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/rtl   $(SOLARINCDIR)$/rtl $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/sal   $(SOLARINCDIR)$/sal $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/stl   $(SOLARINCDIR)$/stl $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/store $(SOLARINCDIR)$/store $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/vos   $(SOLARINCDIR)$/vos $(PIPEERROR) $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)

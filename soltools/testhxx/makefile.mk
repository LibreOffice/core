#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: ihi $ $Date: 2008-01-16 14:22:57 $
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
PRJ := ..
PRJNAME := soltools
TARGET := testhxx
LIBTARGET := NO
ENABLE_EXCEPTIONS := TRUE

CAPTURE_COMMAND = echo
CAPTURE_OUTPUT = > $(MISC)$/testhxx.output && $(TOUCH) $(SLO)$/testhxx.obj

.INCLUDE: $(PRJ)$/util$/makefile.pmk
.INCLUDE: settings.mk

SLOFILES = $(SLO)$/testhxx.obj

.IF "$(USE_SHELL)" == "4nt"
MY_SCRIPT_EXT := .btm
MY_EXECUTABLE := @ echo
.ELSE
MY_SCRIPT_EXT :=
MY_EXECUTABLE := chmod +x
.ENDIF

.INCLUDE: target.mk

ALLTAR: $(BIN)$/$(TARGET)$(MY_SCRIPT_EXT)

$(BIN)$/$(TARGET)$(MY_SCRIPT_EXT) .ERRREMOVE : $(MISC)$/testhxx.output create.pl
    $(PERL) -w create.pl < $(MISC)$/testhxx.output > $@
    $(MY_EXECUTABLE) $@

$(MISC)$/testhxx.output: $(SLO)$/testhxx.obj
    $(TOUCH) $<
    $(TOUCH) $@

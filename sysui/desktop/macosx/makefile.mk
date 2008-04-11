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
# $Revision: 1.3 $
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
TARGET=desktopmacosx

# --- Settings -------------------------------------------------

.INCLUDE :	settings.mk

# --- Files ----------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
        @echo "Nothing to build for OS $(OS)"

.ELSE           # "$(OS)"!="MACOSX"

ZIPFLAGS = -r
ZIP1TARGET = osxicons
ZIP1DIR = ../icons
ZIP1LIST := $(shell $(PERL) list_icons.pl < Info.plist)

.IF "$(GUIBASE)"!="aqua"
CREATOR_TYPE=OOo2
EXECUTABLE=droplet
.ELSE
CREATOR_TYPE=OOO2
EXECUTABLE=soffice.bin
.ENDIF

SOURCE=$(RSCREVISION)

.IF "$(CWS_WORK_STAMP)" != ""
CWS=[CWS:$(CWS_WORK_STAMP)]
.ENDIF

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

ZIP1TARGETN : Info.plist extract_icons_names.pl

ALLTAR : $(COMMONMISC)$/{PkgInfo Info.plist}

$(COMMONMISC)$/PkgInfo :
    echo "APPL$(CREATOR_TYPE)" > $@


$(COMMONMISC)$/Info.plist : $$(@:f)
     sed -e "s|\%EXECUTABLE|${EXECUTABLE}|g" -e "s|\%SOURCE|[$(SOURCE)$(CWS)]|g" $< > $@

.ENDIF		# "$(OS)"!="MACOSX"	


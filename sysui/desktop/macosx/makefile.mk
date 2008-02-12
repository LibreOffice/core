#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
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


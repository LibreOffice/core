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
ZIP1LIST := $(shell $(PERL) -w list_icons.pl < Info.plist)

CREATOR_TYPE=LIBO
EXECUTABLE=soffice

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/desktopshare
.ELSE # "$(WITH_LANG)"!=""
ULFDIR:=..$/share
.ENDIF # "$(WITH_LANG)"!=""

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

.IF "$(ZIP1TARGETN)"!=""
ZIP1TARGETN : Info.plist extract_icons_names.pl

.ENDIF          # "$(ZIP1TARGETN)"!=""

ALLTAR : $(COMMONMISC)$/{PkgInfo Info.plist} $(COMMONBIN)$/InfoPlist_{$(alllangiso)}.zip

$(COMMONMISC)$/PkgInfo :
    echo "APPL$(CREATOR_TYPE)" > $@


$(COMMONMISC)$/Info.plist : $$(@:f)
    sed -e "s|\%EXECUTABLE|${EXECUTABLE}|g" $< > $@

$(COMMONBIN)$/InfoPlist_{$(alllangiso)}.zip : $(COMMONMISC)$/$$(@:b)/InfoPlist.strings
    cd $(<:d) && zip ../$(@:f).$(INPATH) $(<:f)
    $(MV) -f $(COMMONMISC)$/$(@:f).$(INPATH) $@

$(COMMONMISC)$/InfoPlist_{$(alllangiso)}$/InfoPlist.strings : Info.plist $(ULFDIR)$/documents.ulf
    $(MKDIRHIER) $(@:d)
    $(PERL) -w gen_strings.pl -l $(@:d:d:b:s/InfoPlist_//) -p $< | iconv -f UTF-8 -t UTF-16 > $@.$(INPATH)
    $(MV) -f $@.$(INPATH) $@
    
.ENDIF		# "$(OS)"!="MACOSX"	


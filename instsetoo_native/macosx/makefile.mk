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
# $Revision: 1.8 $
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

PRJ=..
TARGET=osxbundle

# --- Settings -------------------------------------------------

.INCLUDE :	settings.mk

# --- Files ----------------------------------------------------

.IF "$(OS)"!="MACOSX" || "$(GUIBASE)"=="aqua"

dummy:
        @echo "Nothing to build for OS $(OS) and GUIBASE $(GUIBASE)"

.ELSE           # "$(OS)"!="MACOSX"

BUNDLE = $(MISC)$/OpenOffice.org.app
CONTENTS = $(BUNDLE)$/Contents
VERSIONED = $(MISC)$/versioned

scriptfiles = \
    $(CONTENTS)$/Resources$/Scripts$/main.scpt	\
    $(CONTENTS)$/Resources$/Scripts$/PostInstall.scpt

ZIPFLAGS = -r
ZIP1TARGET = osxbundle
ZIP1DIR = $(CONTENTS)
ZIP1LIST = * -x Info.plist -x PkgInfo

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

$(ZIP1TARGETN) : $(scriptfiles) 

# create application bundle from apple script source
$(CONTENTS)$/Resources$/Scripts$/main.scpt : application/main.applescript
    $(RM) -r $(BUNDLE)
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -o $(BUNDLE) -c OOo2 -s "$(VERSIONED)/$<"

$(CONTENTS)$/Resources$/Scripts$/%.scpt : application/%.applescript
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -d -o "$@" "$(VERSIONED)/$<"
    $(RM) "$(VERSIONED)/$<"

# The InfoPlist.strings file has to be in UTF-16, thus a back-and-forth conversion
# is needed for versioning
#
# THIS CODE IS PRESERVED FOR FUTURE LOCALIZATION EFFORTS
#$(CONTENTS)$/Resources$/InfoPlist.strings : application/InfoPlist.strings	
#	iconv -f UTF-16 -t UTF-8 "$<" > "$(VERSIONED)/tmp.utf8"
#	make_versioned.sh "$(VERSIONED)/tmp.utf8" "$(VERSIONED)/$<"
#	iconv -f UTF-8 -t UTF-16 "$(VERSIONED)/$<" > "$@"
#	$(RM) "$(VERSIONED)/tmp.utf8" "$(VERSIONED)/$<"
    
.ENDIF		# "$(OS)"!="MACOSX"


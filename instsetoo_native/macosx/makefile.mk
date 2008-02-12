#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2008-02-12 13:33:13 $
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


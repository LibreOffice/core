#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2007-10-09 15:05:49 $
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

.IF "$(OS)"!="MACOSX"

dummy:
        @echo "Nothing to build for OS $(OS)"

.ELSE           # "$(OS)"!="MACOSX"

BUNDLE = $(MISC)$/OpenOffice.org.app
CONTENTS = $(BUNDLE)$/Contents
VERSIONED = $(MISC)$/versioned

.IF "$(GUIBASE)"!="aqua"
scriptfiles = \
    $(CONTENTS)$/Resources$/Scripts$/main.scpt	\
    $(CONTENTS)$/Resources$/Scripts$/PostInstall.scpt
.ENDIF

plistfiles  = $(CONTENTS)$/Info.plist

nibfiles = \
    $(CONTENTS)$/Resources$/MainMenu.nib$/classes.nib   \
    $(CONTENTS)$/Resources$/MainMenu.nib$/info.nib   \
    $(CONTENTS)$/Resources$/MainMenu.nib$/keyedobjects.nib

ZIPFLAGS = -r
ZIP1TARGET = osxbundle
ZIP1DIR = $(CONTENTS)
ZIP1LIST = *

.IF "$(GUIBASE)"!="aqua"
CREATOR_TYPE=OOo2
BUNDLEEXECUTABLE=droplet
.ELSE
CREATOR_TYPE=OOO2
BUNDLEEXECUTABLE=soffice.bin
.ENDIF

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

$(ZIP1TARGETN) : $(scriptfiles) $(plistfiles) $(nibfiles)

$(plistfiles) : $(scriptfiles)

.IF "$(GUIBASE)"!="aqua"
# create application bundle from apple script source
# Info.plist will be deleted to trigger copy rule of our own Info.plist  
$(CONTENTS)$/Resources$/Scripts$/main.scpt : application/main.applescript
    $(RM) -r $(BUNDLE)
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -o $(BUNDLE) -c $(CREATOR_TYPE) -s "$(VERSIONED)/$<"
    echo "APPL$(CREATOR_TYPE)" > $(CONTENTS)$/PkgInfo
    $(RM) "$(CONTENTS)$/Info.plist" "$(VERSIONED)/$<"

$(CONTENTS)$/Resources$/Scripts$/%.scpt : application/%.applescript
    make_versioned.sh "$<" "$(VERSIONED)/$<"
    osacompile -d -o "$@" "$(VERSIONED)/$<"
    $(RM) "$(VERSIONED)/$<"
.ENDIF # "$(GUIBASE)"!="aqua"

# Info.plist is just versioned and copied into the bundle	
$(CONTENTS)$/%.plist : application/%.plist
    make_versioned.sh "$<" "$(VERSIONED)/$<"
        sed "s|\%BUNDLEEXECUTABLE|${BUNDLEEXECUTABLE}|g" "$(VERSIONED)/$<" > "$@"

# nibs are just copied
$(CONTENTS)$/Resources$/MainMenu.nib$/%.nib: application/MainMenu.nib$/%.nib
    $(MKDIRHIER) $(@:d)
    $(COPY) "$<" "$@"

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

